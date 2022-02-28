#pragma once

#include <windows.h>
#include <objbase.h>
#include <WinIoCtl.h>
#include <comutil.h>
#include <comdef.h>
#include <io.h>
#include <psapi.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <share.h>
//#include <dispcall/flipflop.h>
//#include <os_utils.h>
#define KB (1024)
#define MB (1024*1024)
#define GB ulong64(1024*MB)
#define TB (ulong64(1024)*GB)

typedef unsigned long long  ulong64;
typedef unsigned int       ulong32;
typedef   signed int       long32;
typedef   signed long long  long64;

//__declspec(selectany) void* (__stdcall * lsGetValue)(DWORD )=NULL;

template <bool fcloseh=false>
struct AC_Mutex
{
	HANDLE m_h;
	
	AC_Mutex(HANDLE h):m_h(h)
	{
		WaitForSingleObject(m_h,INFINITE);
	};
	~AC_Mutex()
	{
		ReleaseMutex(m_h);
		if(fcloseh) CloseHandle(m_h);
	}

};

inline SECURITY_ATTRIBUTES get_SA(bool inher=false)
{
	struct SECURITY_DESCRIPTOR_i:SECURITY_DESCRIPTOR
	{
		SECURITY_DESCRIPTOR_i()
		{
			InitializeSecurityDescriptor(this, SECURITY_DESCRIPTOR_REVISION);
			SetSecurityDescriptorDacl(this, TRUE, NULL, FALSE);
		};
	};
	static SECURITY_DESCRIPTOR_i sd; 
	SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),&sd,inher}; 
	return sa;
};

/*
Name  : CRC-32
Poly  : 0x04C11DB7	x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 
+ x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
Init  : 0xFFFFFFFF
Revert: true
XorOut: 0xFFFFFFFF
Check : 0xCBF43926 ("123456789")
MaxLen: 268 435 455 байт (2 147 483 647 бит) - обнаружение
одинарных, двойных, пакетных и всех нечетных ошибок
*/
unsigned long Crc32(unsigned char *buf, unsigned long len);

inline UUID new_guid()
{
	UUID  r;
	::CoCreateGuid(&r); 
	return r;
};


inline UUID new_uuid()
{
	UUID  r;
	::UuidCreateSequential(&r); 
	return r;
};

#define GLOBAL_SPACE_UUID get_global_space_id()

inline UUID get_global_space_id()
{
	UUID  uuid={ 0xfd7e0ba6, 0xc876, 0x4109, { 0x93, 0xb5, 0x9f, 0xa0, 0x89, 0x93, 0x9, 0x1a } };
	return uuid;
};



template <class Ch,int n=2*MAX_PATH>
struct ch_buf
{
	enum
	{
		size=n,
        size_b=n*sizeof(Ch)   
	};
	Ch p[n];

	inline 	operator Ch*()
	{
		return p;
	};


};
typedef ch_buf<char,128> striid;
typedef ch_buf<wchar_t> wcbuf;
typedef ch_buf<char> cbuf;


template <typename T,int N=1>
struct blank_ptr:ch_buf<T,N>
{
  blank_ptr(T t=T())
  {
	 for(int n=0;n<N) p[n]=t; 
  }
};

template <typename T>
struct blank_ptr<T,1>:ch_buf<T,1>
{
	blank_ptr(T t=T())
	{
		*p=t;
	};
};

inline wcbuf get_id_name(UUID& uuid=GLOBAL_SPACE_UUID,wchar_t* prfx=L"sfms")
{
	wcbuf r;
	struct u4_t
	{
      INT32 i[4];
	};
    u4_t& i=*((u4_t*)&uuid);
	wsprintfW(r,L"%s%08X%08X%08X%08X",prfx,i.i[0],i.i[1],i.i[2],i.i[3]);
	return r;
};

#pragma pack(push)
#pragma pack(1)
///*
struct page_segment
{
   ulong64 offset;
   ulong64 size;
};
//*/

struct system_info
{

inline static ulong32 os_granularity()
{
    static ulong32 g=get().dwAllocationGranularity;
	return g;
}
inline static ulong64& address_mask()
{
   static  ulong64 s_address_mask=~(ulong64(os_granularity())-1);
   return s_address_mask;
}

inline static ulong64 offset_align(ulong64 offset)
{
	return address_mask()&offset;
}

inline static ulong64 size_align(ulong64 size)
{
	static ulong64 ff=os_granularity()-1;
	return address_mask()&(size+ff);
}

/*
inline static void page_segment_align(page_segment& ps)
{
   ps.offset&=address_mask();
   ps.size=size_align(ps.size);
}
*/
inline static ulong64& page_address_mask()
{
	static  ulong64 s_address_mask=~(ulong64(page_size())-1);
	return s_address_mask;
}

inline static ulong32 page_size()
{
   static ulong32 ps=get().dwPageSize;
   return ps;
}


protected:
	
	inline static SYSTEM_INFO& get()
	{
		struct SYSTEM_INFO_c: SYSTEM_INFO
		{
			SYSTEM_INFO_c(){GetSystemInfo(this);}
		} ;
		static SYSTEM_INFO_c si;
		return si;
	}

};


inline ulong64 align_offset(ulong64 offset)
{
	return system_info::offset_align(offset);
}

inline ulong64 size_align(ulong64 size)
{
	return system_info::size_align(size);
}



/*
inline ulong32 os_granularity()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwAllocationGranularity;
}
*/

struct CLastError
{
	HRESULT err;
	CLastError():err(0){};
	CLastError(DWORD _err)
	{ SetLastError(err=_err);};
	~CLastError()
	{
		SetLastError(err);
	};
	inline CLastError& set(DWORD e=GetLastError())
	{
		err=e;
		return *this;
	}
	template <typename R>
	inline R& operator()(R const & r) 
	{
		set();
		return (R)r;
	}
inline HRESULT hr()
{
  return HRESULT_FROM_WIN32(err);
}
};

struct pair_mmn
{
	wchar_t filename[MAX_PATH*2];
	wchar_t* mapname;
	int fcompress;
	int ftemp;
	pair_mmn(wchar_t* fn,wchar_t* delim=L"::"):mapname(NULL),fcompress(0),ftemp(0)
	{
		if(fn) wcscpy(filename,fn);
		else 
			{
				filename[0]=0;
				return;
    	    }
		//wchar_t*
		wchar_t* p;
		if(p=wcsstr(filename,delim))
		{
			*p=0;
			mapname=p+wcslen(delim);
			if(p=wcsstr(mapname,delim))
			{
				int f=_wtoi(p+2);
              if(f&1) fcompress=1;
			  if(f&2) ftemp=1;

			  *p=0;
			}
		}

	};
	inline operator bool()
	{
		return filename[0];
	}
/*
static pair_mmn make_pair(wchar_t* fn,wchar_t* mapn=NULL,wchar_t* delim=L"::")
{
   if(!mapn) return 
}
*/
};

inline bool is_sparse(HANDLE hf)
{
	BY_HANDLE_FILE_INFORMATION bhfi;
	if(!GetFileInformationByHandle(hf, &bhfi)) return false;
	return bhfi.dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE;
}
inline bool is_compression(HANDLE hf)
{
	BY_HANDLE_FILE_INFORMATION bhfi;
	if(!GetFileInformationByHandle(hf, &bhfi)) return false;
	return bhfi.dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED;
}

inline bool set_compression(HANDLE hf,unsigned cf=COMPRESSION_FORMAT_DEFAULT)
{

	bool f=is_compression(hf);
	if(!f)
		f=DeviceIoControl(hf,FSCTL_SET_COMPRESSION,blank_ptr<short>(cf),sizeof(short),0,0,blank_ptr<DWORD>(), NULL);
	return f;
}

inline bool set_sparse(HANDLE hf)
{
 
 bool f=is_sparse(hf);
 if(!f)
  f=DeviceIoControl(hf, FSCTL_SET_SPARSE,NULL, 0, NULL, 0,blank_ptr<DWORD>(), NULL);
 return f;
}

inline bool fd_set_sparse(int fd)
{
	return set_sparse(HANDLE(_get_osfhandle(fd)));
}

inline bool fd_set_compression(int fd,unsigned cf=COMPRESSION_FORMAT_DEFAULT)
{
        return set_compression(HANDLE(_get_osfhandle(fd)),cf);
}
//S_IREAD


struct wild_handle
{
	void* m_h; 

	wild_handle():m_h(0){};

/*
	wild_handle(wild_handle const& a)
	{
		m_h=a.m_h;
	};
*/

template<class A>
	wild_handle(A  a)
	{
		m_h=(void*)(a);
	};
	template<class A>
	operator A()
	{
		return (A)m_h;
	}
///*
	template<class A>
	wild_handle& operator =(A a)
	{
		m_h=(void*)(a);
		return *this;
	}
//*/
	bool operator !()
	{
		return m_h==NULL;
	}
};


struct wild_handle_state: wild_handle
{
  wild_handle_state():state(0){};
  ulong64    state;
  /*
  wild_handle_state(wild_handle_state const& a)
  {
	  m_h=a.m_h;
       exists=a.exists;
  };
  */

 template<class A>
 wild_handle_state(A a,ulong64 fa=0)
 {
	 state=fa;
	m_h=(void*)(a);
 };
};



inline bool uuid_from_file_handle(wild_handle hf,uuid_t* puuid=NULL,ulong64 *psize=NULL)
{
	
	BY_HANDLE_FILE_INFORMATION fi;
	if(GetFileInformationByHandle(hf,&fi))
	{
      if(psize)
	  {
		  ULARGE_INTEGER ll;
		  ll.HighPart=fi.nFileSizeHigh;
		  ll.LowPart=fi.nFileSizeLow;
		  *psize=ll.QuadPart;
	  }
      if(puuid)
	  {
      fi.nNumberOfLinks=fi.dwVolumeSerialNumber;
	  fi.nFileSizeLow=Crc32((unsigned char*)&(fi.nNumberOfLinks),3*sizeof(DWORD));
	  *puuid=*((uuid_t*)&fi.nFileSizeLow);
	  }
	  return true;
	}
	return false;
}

inline bool uuid_from_file_name(wchar_t* pfn,uuid_t* puuid=NULL,ulong64 *psize=NULL)
{
	//	HANDLE hf=CreateFileW(pfn,FILE_READ_ATTRIBUTES,0,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	//		LastErrorSaver ler;
	CLastError ler;
	//HANDLE hf=CreateFileW(pfn,FILE_READ_ATTRIBUTES,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	HANDLE hf=CreateFileW(pfn,FILE_READ_ATTRIBUTES,0,0,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,0);
	if(hf==INVALID_HANDLE_VALUE) return ler.set(),false;
	bool f=uuid_from_file_handle(hf,puuid,psize);
	CloseHandle(hf);
	return f;
}
//inline wcbuf get_id_name(UUID& uuid=GLOBAL_SPACE_UUID,wchar_t* prfx=L"sfms")
inline wcbuf mapname_from_file_name(wchar_t* pfn,wchar_t* prfx=L"sfms")
{
uuid_t u;
  if(uuid_from_file_name(pfn,&u))
  {
    return get_id_name(u,prfx);
  }
  return wcbuf();
}

inline long64 get_file_size(wild_handle hf)
{
	LARGE_INTEGER lb;
	if(!GetFileSizeEx(hf,&lb)) return -1;
	return *((long64*)&lb);
}


inline bool close_handle(wild_handle h)
{
  return CloseHandle(h);
}

wild_handle create_sparse_file(wchar_t* pstr,int oflag=O_CREAT|O_RDWR,int shflag=SH_DENYNO,int pmode=S_IWRITE,int sparse_compress=1);// 1-sparse 2- compress
ulong32 set_zero_block(wild_handle hf,ulong64 offset,ulong64 size=64*KB);
ulong64 set_sparse_file_size(wild_handle hf,ulong64 space_size);

wild_handle get_enties(HANDLE hf,FILE_ALLOCATED_RANGE_BUFFER* pranges=NULL,ulong32 entries=0);
wild_handle create_file_map_space(wild_handle hf,wchar_t* pmapname=NULL,ulong64& space_size=*(blank_ptr<ulong64>()));
//wild_handle create_file_map_space(wild_handle hf,wchar_t* pmapname,ulong64& space_size);
wild_handle create_sparse_file_map_space(wchar_t* pstr,ulong64& space_size=*(blank_ptr<ulong64>()),int mode=O_CREAT|O_RDWR,int share_mode=SH_DENYNO,int pmode=S_IWRITE,int sparse_compress=1,void *phfile=0);

struct wild_ptr
{
	void*m_p; 
	wild_ptr(void* p)
	{ m_p=p;};
	template<class A>
inline 	operator A*()
	{
		return (A*)m_p;
	}
inline void** address()
{
	return &m_p;
}
};

//wild_handle create_file_map_space(wild_handle hf,wchar_t* pmapname=NULL,ulong64 size=0);

wild_ptr map_of_view(wild_handle hm,ulong64 offset,ulong32 size,ulong32 mode=FILE_MAP_READ|FILE_MAP_WRITE);
bool unmap_ptr(void* p);
inline wcbuf map_file_name(void* p)
{
   wcbuf r;  r[0]=0;
  GetMappedFileNameW(GetCurrentProcess(),p,r,r.size);
  return r;
}

inline long64 get_actual_size(wchar_t* tfn)
{
	LARGE_INTEGER ll=LARGE_INTEGER();
	ll.LowPart=GetCompressedFileSizeW(tfn,(DWORD*)&(ll.HighPart));
	return ll.QuadPart;
}

wcbuf GetFileNameFromPtr(void* pMem);

#pragma pop(push)

