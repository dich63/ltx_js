#pragma once



#define BYTESPERLINE(Width, BPP) ((WORD)((((DWORD)(Width) * (DWORD)(BPP) + 31) >> 5)) << 2)

#include "static_constructors_0.h" 

#include "singleton_utils.h"
#include <limits>
struct _non_copyable
{

	__declspec(align(32)) class CStopwatch
	{
	public:
		
		CStopwatch() {  Start(); }

		inline	 CStopwatch& Start() {

			QueryPerformanceFrequency(&m_liPerfFreq);
			QueryPerformanceCounter(&m_liPerfStart); 
			return *this;
		}

		inline long double Sec(bool fstart=false)
		{
			LARGE_INTEGER liPerfNow;
			QueryPerformanceCounter(&liPerfNow);
			long double r= (long double)( liPerfNow.QuadPart - m_liPerfStart.QuadPart ) / (long double)(m_liPerfFreq.QuadPart);
			if(fstart) Start();
			return r;
		}
		inline LONGLONG Tic()
		{
			LARGE_INTEGER liPerfNow;
			QueryPerformanceCounter(&liPerfNow);
			return (LONGLONG)( liPerfNow.QuadPart - m_liPerfStart.QuadPart );
		}
		inline LONGLONG Frec()
		{
			QueryPerformanceFrequency(&m_liPerfFreq);
			return  m_liPerfStart.QuadPart;
		}

		//
	static	inline double fps(double t )
		{
			if(t>0) return double(1)/t;
			else return  std::numeric_limits<double>::infinity();
		}

      inline double fps()
	  {
		  return fps(Sec());
	  }

	private:
		LARGE_INTEGER m_liPerfFreq;
		LARGE_INTEGER m_liPerfStart;
	};




	template <class MUTEX>
	struct locker_t
	{
		MUTEX* r;
		locker_t(MUTEX* pc,bool flock=true):r(pc)
		{
			if(flock) r->lock();
		}
		locker_t(MUTEX& c,bool flock=true):r(&c)
		{
			if(flock) r->lock();
		}
		~locker_t()
		{
			r->unlock();
		}
	};

	_non_copyable(){};
private:
	_non_copyable(_non_copyable&){};
	_non_copyable(const _non_copyable&){};
	void operator=(_non_copyable&){};
	void operator=(const _non_copyable&){};

};


template <class T>
unsigned long Crc32(T* p,size_t c=1)
{

	struct __s_crt__
	{
		unsigned long crc_table[256];
		__s_crt__()
		{
			unsigned long crc;
			for (int i = 0; i < 256; i++)
			{
				crc = i;
				for (int j = 0; j < 8; j++)
					crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;

				crc_table[i] = crc;
			};
		};

		inline unsigned long make(void *p, unsigned long len)
		{
			unsigned char *buf=(unsigned char *)p;
			unsigned long crc = 0xFFFFFFFFUL;
			while (len--) 
				crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);

			return crc ^ 0xFFFFFFFFUL;

		};

	};


	return	class_initializer_T<__s_crt__> ::get().make(p,sizeof(T)*c);
}		


inline DWORD PALSIZE(const BITMAPINFO& bmi)
{
	UINT BitCount=bmi.bmiHeader.biBitCount;
	UINT clUsed=bmi.bmiHeader.biClrUsed;
	switch(BitCount)
	{
	case 1 :return ((clUsed)?clUsed:2)*sizeof(RGBQUAD);
	case 4 :return ((clUsed)?clUsed:16)*sizeof(RGBQUAD);
	case 8 :return ((clUsed)?clUsed:256)*sizeof(RGBQUAD);
	default:return 0;
	};
}   ;



inline BYTE* pBMP_bits(void* ptr,int* psizb=0)
{
	
	if(!ptr) return 0;
		
	char* p=(char*)ptr;
	BITMAPFILEHEADER* pbfh=(BITMAPFILEHEADER*)ptr;
	if(psizb)
	{
	 BITMAPINFOHEADER* pbih=(BITMAPINFOHEADER*)(p+sizeof(BITMAPFILEHEADER));
	 *psizb=BYTESPERLINE(pbih->biWidth,pbih->biBitCount)*pbih->biHeight;
	}
//
	return (BYTE*)( p+pbfh->bfOffBits);
}



struct DC_holder:_non_copyable
{
   HDC dc;
   HWND hwin;
   
   DC_holder(HWND hw=HWND(-1))
   {
	   hwin=hw;
	   if(hwin!=HWND(-1))
	     dc=GetDC(hwin);
	   else dc=0;
   }
	operator HDC()
	{
      return dc; 
	}
DC_holder&  attach(HDC d,HWND hw=HWND(-1))
	{
      release();
      dc=d;
	  hwin=hw;
	  return *this;
	}
DC_holder&  attach(HWND hw)
{
	release();
	dc=GetDC(hw);
	hwin=hw;
	return *this;
}
DC_holder&  operator=(HDC d)
{
	return attach(d);
};

 void release()
 {
	 if(dc)
	 {
		 if(hwin==HWND(-1)) DeleteDC(dc);
		 else ReleaseDC(hwin,dc);
	 }
	 dc=0;
    hwin=HWND(-1);
	 
 }
 ~DC_holder(){release();}
};

struct BMPMap:_non_copyable
{


	enum{
		CRDIS=OPEN_ALWAYS,
		OPMOD=GENERIC_READ|GENERIC_WRITE,
		SHAREMOD=FILE_SHARE_READ|FILE_SHARE_WRITE,
		FILEATTR=0
	};

    
	struct remote_control_data_t
	{
		ULONG32 crc32;
        POINT cursor; 
	};

	struct frame_info_base_t
	{
	  
	  
		GUID sign;
		LONG32 last_num;
        ULONG32 offset_next_frame_info;
		ULONG32 time;
		ULONG32 crc32;		

		inline  bool check(bool fset=false) 
		{
			const GUID _sign = { 0x1c85a22a, 0xea4c, 0x4eed, { 0xaf, 0x2c, 0xa5, 0x25, 0x6, 0xba, 0xae, 0x31 } };
			if(fset) sign=_sign; 
			return _sign==sign;             
		}
	};
	struct frame_info_t:frame_info_base_t
	{
		BYTE pbits[1];
	};

	HANDLE hmap,hmutex,habortevent;
	std::vector<mutex_ref_t> vframe_mutexes;
	std::vector<frame_info_t*> vframe_info;
	BYTE* ptr;
	DWORD offset;
	DWORD size;
	DWORD size_bits;
	ULONG32 crc32;
    ULONG32 crc32_old;
	int nframes;
	v_buf<wchar_t> filename;
	v_buf<wchar_t> mapname_prfx,mapname,mutexname;
	DWORD bbpc;


	BMPMap():hmap(0),ptr(0),size(0),hmutex(0),crc32(0),crc32_old(1),nframes(0),habortevent(0),bbpc(0){};

	~BMPMap(){
        
		close();
	}
inline void	close(bool faborted=false)
{

	UnmapViewOfFile(make_detach(ptr));
	if(hmap) CloseHandle(make_detach(hmap));
	if(hmutex) CloseHandle(make_detach(hmutex));
	if(habortevent) 
	{
		if(faborted) SetEvent(habortevent);
		CloseHandle(make_detach(habortevent));
	}
	
	//if (vframe_mutexes.size()) 
		reset_frame_mutexes();
	 //ptr=0; 
     //hmutex=hmap=0;
}

inline int lock(int tio=INFINITE)
{    
	DWORD ws=WaitForSingleObject(hmutex,tio);
	return (ws==WAIT_OBJECT_0);
}
inline int unlock()
{
	return ReleaseMutex(hmutex);
}
inline bool try_lock()
{
	//DWORD ws=WaitForSingleObject(hmutex,0);
	//return ws!=WAIT_TIMEOUT;
	return lock(0);
}

inline BYTE* pBits()
{
	 
	return (ptr)?ptr+offset:0;
}
inline BYTE* pAddData()
{
    return (ptr)?ptr+((BITMAPFILEHEADER*)ptr)->bfSize:0;
}

inline remote_control_data_t* remote_control_data()
{
    return (remote_control_data_t*)( (ptr)?ptr+((BITMAPFILEHEADER*)ptr)->bfSize+sizeof(BITMAPFILEHEADER):0);
}

inline bool _open_mutex(wchar_t* name=0)
{
	
	v_buf<wchar_t> mn,en;

	if((name)&&(*name))
	{
		mutexname.printf(L"%s_mutex",name);
		en.printf(L"%s_abort",name);
		
	}
	  hmutex=CreateMutexW(0,0,mutexname);
	  habortevent=CreateEventW(0,1,0,en);


	  return hmutex&&habortevent;

};

inline void reset_frame_mutexes(int new_nframes=0)
{
	v_buf<wchar_t> buf;
	wchar_t* pn=mapname_prfx.get();
	if(!pn) pn=L"xx";
	for(int n=0;n<make_detach(nframes,new_nframes);n++)  CloseHandle(vframe_mutexes[n]);
	if(nframes)
	{
     vframe_mutexes.resize(nframes);
   	 for(int n=0;n<nframes;n++)
     vframe_mutexes[n]=CreateMutexW(0,0,buf.printf(L"%s_%d",mapname_prfx.get(),n));   
	}
	//else if(!vframe_mutexes.empty()) 	vframe_mutexes.resize(0);
}

template <class BMPI>
inline BMPI& header()
{
	char* p=(char*)ptr;
	//BITMAPFILEHEADER* pbfh=(BITMAPFILEHEADER*)ptr;

    return  *((BMPI*)(p+sizeof(BITMAPFILEHEADER)));
}

inline int line_sizeb()
{
      BITMAPINFOHEADER& bi=header<BITMAPINFOHEADER>();
	return BYTESPERLINE(bi.biWidth,bi.biBitCount);
}

inline int bytes_sizeb()
{
	BITMAPINFOHEADER& bi=header<BITMAPINFOHEADER>();
	return BYTESPERLINE(bi.biWidth,bi.biBitCount)*bi.biHeight;
}

inline long image_size_b()
{
	BITMAPINFOHEADER& bi=header<BITMAPINFOHEADER>();
   return line_sizeb()*bi.biHeight+PALSIZE(*pInfo())+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
}
inline long image_file_size_b()
{
   return ((BITMAPFILEHEADER*)ptr)->bfOffBits+image_size_b();
}

template <class T>
inline T* pBitsT()
{
       char* p=(char*)ptr;
	   BITMAPFILEHEADER* pbfh=(BITMAPFILEHEADER*)ptr;
   return (T*)( p+pbfh->bfOffBits);
}

inline frame_info_t* pframe_info(int n=0)
{
      char* p=pBitsT<char>();
	  if(!p) return 0;
	  p-=sizeof(frame_info_base_t);
	  frame_info_t* pf=(frame_info_t*)p;
	  
	  while((n>0))
	  {
		  if(pf->offset_next_frame_info==0) return 0;
		  p+=pf->offset_next_frame_info;
		  pf=(frame_info_t*)p;
		  --n;
	  }
	  return pf;
};


inline BITMAPFILEHEADER* get_BITMAPFILEHEADER()
{
	return (BITMAPFILEHEADER*)ptr;
}

inline BITMAPINFO* pInfo()
{
	char* p=(char*)ptr;
	//BITMAPFILEHEADER* pbfh=(BITMAPFILEHEADER*)ptr;
	return (BITMAPINFO*)( (p)?( p+sizeof(BITMAPFILEHEADER)):0 );
}

inline long get_last_index(long l=0){
    
     frame_info_t* pi=pframe_info();
	 if(pi==0) return -1;
  	long ind=InterlockedExchangeAdd((volatile long*)&pi->last_num,l);
	return ind;
}

inline long set_last_index(long l){

	frame_info_t* pi=pframe_info();
	if(pi==0) return -1;
	long ind=InterlockedExchange((volatile long*)&pi->last_num,l); 
	return ind;
}

void set_map_name_file(const wchar_t* _name,const wchar_t* pfilename)
{
	v_buf<wchar_t> tmp;
   if(safe_len(pfilename))
   {
	   pfilename=tmp.clear().cat(pfilename);
	   filename.printf(L"%s",pfilename);
   }
   else  filename.clear();

  if(safe_len(_name))
  {
	  mapname_prfx.printf(L"sh_bmpmap_%s",_name);
	  mapname.printf(L"%s",_name);
  } else
  {
      mapname_prfx.clear();
	  mapname.clear();
  }


   
}

static bool check_resize(const BITMAPINFO* pbmi1,const BITMAPINFO* pbmi2)
{
	
		if(!pbmi1) 
			  return false;
		
		  int headsize=PALSIZE(*pbmi1)+sizeof(BITMAPINFOHEADER);
		  return (memcmp(pbmi1,pbmi2,headsize))==0;
			  return false;

}


inline bool check_resize(BITMAPINFO* pbmi=0)
{
		__try	{
	     if(pbmi&&(!check_resize(pInfo(),pbmi)))
			 return false;
	     frame_info_t* fi=pframe_info();
	     return (fi)&&fi->check();
			}	__except(EXCEPTION_EXECUTE_HANDLER)	{};
		return false;
}


inline bool check_frames_exists()
{
      int headsize=PALSIZE(*pInfo())+sizeof(BITMAPINFOHEADER);  	   
      int ind=((BITMAPFILEHEADER*)ptr)->bfOffBits-headsize-sizeof(frame_info_base_t);

     if(ind>=0)
	 {
		 __try
		 {
           frame_info_t* pfi=pframe_info();
		   return (pfi)&&pfi->check();

		 }
		 __except(EXCEPTION_EXECUTE_HANDLER){}

	 }
	 return false;
}


inline BITMAPFILEHEADER*  open(const wchar_t* name,const wchar_t* filename=0)
{
	close();

	v_buf<wchar_t> tmp;
//	
     
    
	HANDLE hfile=INVALID_HANDLE_VALUE;
	if(safe_len(filename))
	{
		hfile=CreateFileW(filename,OPMOD,SHAREMOD,NULL,CRDIS,FILEATTR,NULL);
		if(hfile==INVALID_HANDLE_VALUE) return 0;   
		//
		if(safe_len(name)==0)			name=file_uuid(hfile,tmp);
	}
    

	set_map_name_file(name,filename);

    if(!_open_mutex(mapname_prfx)) return 0;	

	locker_t<BMPMap> lock(this);
  
  //hmap=OpenFileMappingW(FILE_MAP_READ,0,name);
  hmap=CreateFileMappingW(hfile,0,PAGE_READWRITE,0,0,mapname_prfx);
  if(hfile!=INVALID_HANDLE_VALUE)
	  CloseHandle(hfile);
  ptr=(BYTE*)MapViewOfFile(hmap,FILE_MAP_ALL_ACCESS,0,0,0);
  if(ptr)
  {
   BITMAPFILEHEADER* pfh=(BITMAPFILEHEADER*)ptr;
   size=pfh->bfSize+sizeof(BITMAPFILEHEADER);
   offset=pfh->bfOffBits;

    //if(vframe_info.size())
		vframe_info.clear();
   int _nframes=0;

if(check_frames_exists())
{
   frame_info_t *pfp;
   // 
   //vframe_mutexes.resize(0);
   while(pfp=pframe_info(_nframes))     _nframes++;
   
    vframe_info.resize(_nframes);
    for(int n=0;n<_nframes;n++)
   {
	   vframe_info[n]=pframe_info(n);
	
   }
   
}
      reset_frame_mutexes(_nframes);
  }
    return (BITMAPFILEHEADER*)ptr;

};


BITMAPFILEHEADER* open_key( const wchar_t* _mapname,const wchar_t* pathname=0)
{

	bool f;
	int cbm=safe_len(_mapname);
	if(cbm==0)
		return false;
	v_buf<wchar_t> tmp,fn;
	if((safe_len(pathname)==0))
		pathname=file_make_dir(L"%APPDATA%\\shared_images",0,tmp);
	bool fnonul=1;
	if(safe_len(pathname)&&(fnonul=_wcsicmp(pathname,L"nul")))
	{
		
       v_buf<char> encname(cbm*3+16);

	   
	   char* penc=url_escape(char_mutator<CP_UTF8>(_mapname),cbm,encname.get());

		fn.printf(L"%s\\%s.bmp",pathname,(wchar_t*)char_mutator<CP_UTF8>(penc));
	}
	else
	{
		//if(StrStrNW(pathname,L"nul",3))
		if(fnonul)
			return 0;
	}

	// f= open_mapping(const wchar_t* filename,const wchar_t* mapname,fbcount);    

	BITMAPFILEHEADER* pbifh=open(_mapname,fn);//(wchar_t*)
	
	  return pbifh;
}

inline BYTE*  reset(BITMAPINFO& bi,int _nframes=1,DWORD capacity=(16*1024*1024))
{
	//if(!ptr) 		return 0;
     return init(bi,mapname,filename,_nframes,capacity);
}

inline BYTE*  reset(DWORD capacity=(16*1024*1024),int _nframes=0)
{
	//if(!ptr) 		return 0;
	return init(*((BITMAPINFO*)0),mapname,filename,_nframes,capacity);
}


BITMAPFILEHEADER* clone_to(BMPMap& bm)
{
  return	bm.open_key(mapname,filename);
}

inline BYTE*  init(BITMAPINFO& bi,wchar_t* name=0,wchar_t* pfilename=0,int _nframes=1,DWORD capacity=(16*1024*1024))
  {
	  close();
      
	  v_buf<wchar_t> tmp;
	  
       HANDLE hfile=INVALID_HANDLE_VALUE;
	   
/*
	   if(filename&&(*filename))
	   {
		   hfile=CreateFileW(filename,OPMOD,SHAREMOD,NULL,CRDIS,FILEATTR,NULL);
		   if(hfile==INVALID_HANDLE_VALUE) 
		   {
				   return 0;   }
		   
	   }
*/

	   if(safe_len(pfilename))
	   {
		   hfile=CreateFileW(pfilename,OPMOD,SHAREMOD,NULL,CRDIS,FILEATTR,NULL);
		   if(hfile==INVALID_HANDLE_VALUE) return 0;   
		   // 
		   //
		   if(safe_len(name)==0)			   name=file_uuid(hfile,tmp);
	   }

	   set_map_name_file(name,filename);

	   if(!_open_mutex(mapname_prfx)) return 0;	
	   
      locker_t<BMPMap> lock(this);

	   reset_frame_mutexes(_nframes);

	   
    if(&bi==0)
	{

		hmap=CreateFileMappingW(hfile,0,PAGE_READWRITE,0,capacity,mapname_prfx);
		if(hfile!=INVALID_HANDLE_VALUE)
			CloseHandle(hfile);
		if(hmap==0) return 0;
		ptr=(BYTE*)MapViewOfFile(hmap,FILE_MAP_ALL_ACCESS,0,0,capacity);
		return ptr;
	}
	   //filename.clear().cat(pfilename);
	   //mapname_prfx.clear().cat(name);
     
	  WORD  wbm=*((WORD*)"BM");
	  DWORD palsz=PALSIZE(bi);
	  DWORD bitsoffset=sizeof(BITMAPINFOHEADER)+palsz;
	  DWORD cb= BYTESPERLINE(bi.bmiHeader.biWidth,bi.bmiHeader.biBitCount)*bi.bmiHeader.biHeight;
	  size_bits=cb;
	  offset=bitsoffset+sizeof(BITMAPFILEHEADER);
	  offset=((offset+7)/8)*8;
	  offset+=sizeof(frame_info_base_t);
	  DWORD imsize=cb+offset-sizeof(BITMAPFILEHEADER);
	  BITMAPFILEHEADER fi={wbm,imsize,0,0,offset};
	  
	  size=imsize+sizeof(BITMAPFILEHEADER);

	  int framesize=size+sizeof(frame_info_base_t);
      int mapsize=size+(nframes-1)* framesize;
	 
	  

	  hmap=CreateFileMappingW(hfile,0,PAGE_READWRITE,0,max(mapsize,capacity),mapname_prfx);

	  if(hfile!=INVALID_HANDLE_VALUE)
		     CloseHandle(hfile);

	  if(hmap==0) return 0;
	  
	  ptr=(BYTE*)MapViewOfFile(hmap,FILE_MAP_ALL_ACCESS,0,0,mapsize+16);

	  

	  if(ptr)
	  {
		  cb=sizeof(BITMAPFILEHEADER);
		  memcpy(ptr,&fi,cb);
          memcpy(ptr+cb,&bi,bitsoffset);

          

		  frame_info_base_t* pfr=pframe_info();
		  (*pfr)=frame_info_base_t();

          //vframe_info.clear();
		  vframe_info.resize(nframes);
		  pfr->check(true);
		  for(int n=1;n<nframes;++n)
		  {
			  pfr->offset_next_frame_info=framesize;
			  pfr=pframe_info(n);
			  pfr->check(true);
			  vframe_info[n]=(frame_info_t*)pfr;
			  (*pfr)=frame_info_base_t();
		  }


		  
	  }
	  return ptr;
  }
     
inline    bool check_crc32()
	{
		return crc32_old==crc32;
	}
inline    ULONG32 get_crc32()
	{
		crc32_old=crc32;
		return crc32=Crc32((char*)ptr,size);
	}
inline operator bool()
{
	return ptr;
}
 inline int decrease_toRGB24(bool foffset=false)
 {
	 BITMAPINFOHEADER& bmih=header<BITMAPINFOHEADER>();
	 BITMAPFILEHEADER* pfh=(BITMAPFILEHEADER*)ptr;

	 if(bbpc==0) bbpc=bmih.biBitCount;

	 if(bbpc!=32) return 0;

    int line24=BYTESPERLINE(bmih.biWidth,24);
	int line32=BYTESPERLINE(bmih.biWidth,32);

	char *ps=((char*)pBits());
	char *pd=ps;

	int h=bmih.biHeight;
    int wb=3*(bmih.biWidth-1);
	
	for(int y=0;y<h;y++)
	{  

        char * p=pd,*pp=ps;
        for(int k=0;k<bmih.biWidth;k++)
		{
           
			p[0]=pp[0];
			p[1]=pp[1];
			p[2]=pp[2];
			p+=3;
			pp+=4;
		   

		}

		pd+=line24;
		ps+=line32;
	}
   
     
       //bipr.bmiHeader=bip.bmiHeader;
       bmih.biBitCount=24;
	   //bmp.pBits=pBits;
/*
	   if(foffset)
	   {
		   int offs=(pfh->bfOffBits-(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)));
		   if(offs>0)
		   {             
			   memmove();

		   }

	   }
*/
       //int offs=(pfh->bfOffBits-(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)));

	   pfh->bfSize=image_file_size_b();
	   
	   //bmih.biSizeImage=line24*bmih.biHeight;
	   
	   
       

  return 1;
 }

};


struct HGDI_holder:_non_copyable
{
	
	HGDIOBJ hobj;

	HGDI_holder(HGDIOBJ h=0 ):hobj(h){};
	operator HGDIOBJ()
	{
		return hobj; 
	}
	HGDI_holder&  attach(HGDIOBJ ho)
	{
		release();
		hobj=ho;		
		return *this;
	}

inline HGDI_holder&  operator=(HGDIOBJ ho)
{
         return attach(ho);
}
	void release()
	{
		if(hobj) DeleteObject(hobj);
		hobj=0;
		

	}
	~HGDI_holder(){release();}
};




struct Capture_base:_non_copyable
{




	struct CS
	{
		CRITICAL_SECTION m_cs;
		CS(){

			InitializeCriticalSection(&m_cs);
		}
		~CS()
		{
			DeleteCriticalSection(&m_cs);
		} // on  	  DLL_PROCESS_DETACH
		inline void lock(){ EnterCriticalSection(&m_cs);}
		inline void unlock(){ LeaveCriticalSection(&m_cs);}

	};


	struct BIHPAL:BITMAPINFO
	{
		//
		RGBQUAD rr[255];
		BIHPAL()
		{
			memset(this,0,sizeof(BIHPAL));
			bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		}
		BIHPAL* init(int w,int h, int nBPP)
		{
			bmiHeader.biWidth=w;
			bmiHeader.biHeight=h;
			bmiHeader.biBitCount = USHORT( nBPP );
			bmiHeader.biPlanes = 1;
			bmiHeader.biCompression = BI_RGB;
			return this;

		}
		inline int line_sizeb()
		{
			return  BYTESPERLINE(bmiHeader.biWidth,bmiHeader.biBitCount);
		}
		
		inline size_t sizeb()
		{
		  return	line_sizeb()*bmiHeader.biHeight;
		}
	};

	static   inline HWND wnd_alloc(char* winclass,char* winname=NULL,HWND hwinParent=0,int style=WS_POPUPWINDOW,HBRUSH hbk=0)
	{
		WNDCLASSA wc;
		HINSTANCE HInstance=GetModuleHandleA(NULL);
		if(!GetClassInfoA(HInstance,winclass,&wc) )
		{
			memset(&wc,0,sizeof(wc));
			wc.hInstance = HInstance;
			wc.hbrBackground=hbk;
			wc.lpszClassName=winclass;
			wc.lpfnWndProc=DefWindowProc;
			if ( ! RegisterClassA( &wc ) ) return NULL;
		};
		//HWND hwnd=CreateWindowA(winclass,winname,style,0,0,0,0,hwinParent,NULL,HInstance,0);
		HWND hwnd=CreateWindowExA(WS_EX_TOOLWINDOW,winclass,winname,style,0,0,0,0,hwinParent,NULL,HInstance,0);
		return hwnd;
	}


	HRESULT hr;
	BIHPAL bip;//,bipr;
    //BIHPAL raw_bip;
	
	BMPMap bmpmap;
	HGDI_holder hBitmap;
	DC_holder MemDC;
	void* pBits;
	//	CS mutex;
		//__s_crt__ s_crc;
	int linesizeb;
	
	ULONG32 crc;
	ULONG32 crcold;
    ULONG32 state_masks;
	union
	{
		struct{ char _fourcc[8];};
		struct{ DWORD _fourdw;};
		struct{ long long _fourdw8;};

	};
	
	
	char* get_fourcc()
	{
		return _fourcc;
	}


inline operator BITMAPINFO*()
{
	 return &bip;
}
	
	//Capture_base():hr(0),MemDC(0),hBitmap(0),pBits(0),crc(0),crcold(-1),linesizeb(0){};
   Capture_base():hr(0),pBits(0),crc(0),crcold(-1),linesizeb(0),state_masks(0)
   {
         _fourdw8=0;
   };

	~Capture_base() {
	 //if(MemDC) DeleteDC(MemDC);
	 //if(hBitmap) DeleteObject(hBitmap);
   }


	HRESULT _update(HDC dc,RECT& r)
	{
		DWORD ff;
		if(hr==S_OK)
		{

			int w=r.right-r.left;
			int h=r.bottom-r.top;

			ff=::BitBlt(MemDC, 0,0, w,h, dc,r.left,r.top, SRCCOPY);
			if(!ff) hr=GetLastError();
		}

		return hr;
	}

	inline HRESULT pre_init(HDC dc)
	{
        ;
		if(!bmpmap.init(bip)) return HRESULT_FROM_WIN32(GetLastError());

		//hBitmap.attach(::CreateDIBSection( 0,&bip, DIB_RGB_COLORS, &pBits, NULL,0));
		hBitmap.attach(::CreateDIBSection( 0,&bip, DIB_RGB_COLORS, &pBits, bmpmap.hmap,bmpmap.offset));
		if(!hBitmap) 
		{
			hr=HRESULT_FROM_WIN32(GetLastError());
			if(!hr) return hr=E_FAIL;
		}
		MemDC.attach(CreateCompatibleDC(dc));
		if(!MemDC) 
		{
			hr=HRESULT_FROM_WIN32(GetLastError());
			if(!hr) return hr=E_FAIL;
		}

		linesizeb=_line_sizeb();
         HGDI_holder hold;
		 //hold.hobj=
			 SelectObject(MemDC,hBitmap);
		//HGDIOBJ hold=SelectObject(MemDC,hBitmap);
		//if(hold) DeleteObject(hold);
		return S_OK;
	}

 inline WORD BPP() {
   return bip.bmiHeader.biBitCount;
 }

 inline DWORD compresion() {
	 return bip.bmiHeader.biCompression;
 }

 inline int width()
 {
	 return bip.bmiHeader.biWidth;
 };

 inline int height()
 {
	 return bip.bmiHeader.biHeight;
 };

inline RECT cam_rect()
{
	RECT r={0,0,width(),height()};
	return r;
}

 inline int line_sizeb()
 {
	 return linesizeb;
 }
 //inline int line_sizebr() {	 return BYTESPERLINE(bipr.bmiHeader.biWidth,bipr.bmiHeader.biBitCount);; }

 inline int _line_sizeb()
 {
	return  linesizeb=BYTESPERLINE(bip.bmiHeader.biWidth,bip.bmiHeader.biBitCount);
 }
 
 char* pline(int n)
 {
	 
      return ((char*)pBits)+linesizeb*n;
 }


 ULONG32 get_crc32()
 {
	 int siz=line_sizeb()*height();
	 return Crc32((char*)pBits,siz);
		 //s_crc.make(pBits,siz);
 }

 bool crc32_check()
 {
    ULONG32 s=get_crc32();
	crcold=crc;
	if(s==crc) return true;
	    else crc=s;
      return false;
 }


 

inline  size_t get_file_size()
 {
	 DWORD palsz=PALSIZE(bip);
	 DWORD bitsoffset=sizeof(BITMAPINFOHEADER)+palsz;
	 DWORD imsize=bip.sizeb()+bitsoffset;
	 return imsize+sizeof(BITMAPFILEHEADER);
 }

 template <class Setter>
 int write_to(const Setter& gs)
 {
   if(!pBits)  return -1;
   
    WORD  wbm=*((WORD*)"BM");
    DWORD palsz=PALSIZE(bip);
    DWORD bitsoffset=sizeof(BITMAPINFOHEADER)+palsz;
    DWORD imsize=bip.sizeb()+bitsoffset;

   //
	BITMAPFILEHEADER fi={wbm,imsize,0,0,bitsoffset+sizeof(fi)};
	//BITMAPFILEHEADER fi={wbm,imsize,0,0,sizeof(BITMAPINFOHEADER)+sizeof(fi)};

   size_t lb=line_sizeb(),cbtot=0;
   int cb;
   cb=sizeof(fi);
    //gs(&fi,cb);
   if(cb!=gs((unsigned char*)&fi,cb)) return -1;
   else cbtot+=cb;
   //cb=sizeof(BITMAPINFOHEADER);
   cb=bitsoffset;
   if(cb!=gs(( char*)&bip.bmiHeader,cb)) return -1;
   else cbtot+=cb;

   cb=lb;
   for(int y=0;y<height();y++)
   {
           char* p=pline(y);
	   if(cb!=gs(p,cb)) return -1;
	   else cbtot+=cb;
   }
      return cbtot;
 };

 int write_to_file(FILE* hf)
 {
    struct  Setter
    {
        FILE* hf;
		inline int	operator()(void* p,int cb) const
		{
			return fwrite(p,1,cb,hf);
		}
      
	} setter={hf};

     if(!hf) return -1;

   return write_to(setter);

 };

int write_to_file(char* fn)
{
	if(!fn) return -1;
	int cb=-1;
	FILE* hf=fopen(fn,"wb");
	try	{
       cb=write_to_file(hf);
	}
	catch (...){}
	if(hf) fclose(hf);
	return cb;
}
int write_to_file(wchar_t* fn)
{
	if(!fn) return -1;
	int cb=-1;
	FILE* hf=_wfopen(fn,L"wb");
	try	{
		cb=write_to_file(hf);
	}
	catch (...){}
	if(hf) fclose(hf);
	return cb;
}


 /*
 bool decrease_toRGB24()
 {
	 if(24==bip.bmiHeader.biBitCount) 
		 return true;
       if(32!=bip.bmiHeader.biBitCount) 
		   return false;
    int line24=BYTESPERLINE(bip.bmiHeader.biWidth,24);

	char *ps=((char*)pBits)+3;
	char *pd=ps;

	int h=height();
    int wb=3*(width()-1);
	
	for(int y=0;y<h;y++)
	{
         char * p=ps;
 

        for(char * i=pd;i<pd+wb;i+=3) 
		{
           *((unsigned*)i)=* ((unsigned*)(++p));
		}
		pd+=line24;
		ps+=linesizeb;
	}
       bipr.bmiHeader=bip.bmiHeader;
       bipr.bmiHeader.biBitCount=24;
  return true;
 }
 */



};