#pragma once
//

#include "windows.h"
#include "autocloser.h"
#include "singleton_utils.h"
#include "tls_dll.h"
//#include "os_utils.h"
#include "static_com.h"
#include <strstream>

#define KB (1024)
#define MB (KB*KB)
#define GB ((long long)(KB*MB))
template <class H,class T=H>
struct _auto_nop :_auto_close_P<H,noop_type<H>,T> 
{
	_auto_nop(H h=NULL):_auto_close_P(h){};
};





template <class T>
struct buf_T
{

	typedef tls_LastErrorSaver LastErrorSaver;
	T* m_p;
	long m_count;
    
	buf_T(T* p=NULL,long count=0): m_p(p),m_count(count){};

	inline operator T* ()
	{ 
	   return m_p;
	}
inline T* detach()
{
   T* pt=m_p;
   m_p=NULL;
   return pt;
}
inline  operator bool()
{
	return (m_p!=NULL);
}

T* operator->() const throw()
{
	return  m_p;
}

template<class A>
inline  A& ref(long nn=0)
{
    return  *(((A*)m_p)+nn);
}
inline  long count()
{
	return m_count;
}
inline  void zero()
{
  if(m_p) ZeroMemory(m_p,sizeof(T)*m_count);
}
virtual ~buf_T(){};
};

template <class T>
struct buf_res_T: public buf_T<T>
{
    //long m_cb;
	buf_res_T(){};
	buf_res_T(HMODULE hmod,const wchar_t* res,const wchar_t* type)
	{
     
		open(hmod,res,type);
	}

inline T*	open(HMODULE hmod,const wchar_t* res,const wchar_t* type)
	{
		HRSRC hrs=FindResourceW(hmod,res,type);
		if(hrs) 
		{
			HGLOBAL hg= LoadResource(hmod,hrs);
			m_count=SizeofResource(hmod,hrs)/sizeof(T);
            // m_cb=LocalSize(hg);
			if(hg)
				m_p=(T*)LockResource(hg);
		}
		return m_p;
	}
  int save_to_file(wchar_t* fn)
  {
	  if(!m_p) return -2;
	  FILE* hf=_wfopen(fn,L"wb");
	  if(!hf) return -1;
	  int cb=fwrite(m_p,sizeof(T),m_count,hf);
	  fclose(hf);

  }

  /*
  HRESULT copy_to_BSTR(BSTR* pb)
  {
	  if((!pb)&&(m_p)) return E_POINTER;
	  int siz=sizeof(T)*m_count;
      BSTR b=SysAllocStringByteLen(0,siz+4);
	  char* p=(char*)b;
	  if(!p) return E_OUTOFMEMORY;
	  memcpy(p,m_p,siz);
	  
	  ULONG* l=(ULONG*)(p+siz);
	  l=0;
	  *pb=b;
	  return S_OK;
  }
  */
  template<class Stream>
  Stream& operator >>(Stream& s)
  {
	  if(m_p) s.write((T*)m_p,m_count);
	  return s;
  }

  operator bstr_t()
  {
	  if(m_p)
	  {
		  std::strstream s;
		  (*this)>>s<<'\0'<<'\0';
		  return bstr_t(s.str());
	  }
	  return bstr_t();
  }
};


template <class T,
long CRDIS=OPEN_ALWAYS,
long OPMOD=GENERIC_READ|GENERIC_WRITE,
long SHAREMOD=FILE_SHARE_READ|FILE_SHARE_WRITE>
struct buf_fm_T : public buf_T<T>
{
  
//_auto_stdcall< LPCVOID,&::UnmapViewOfFile,T*> m_p;
	//T* m_p;
  //long m_count;
	HANDLE m_htmp;
	bool m_lock;  
	buf_fm_T():buf_T<T>(),m_lock(false),m_htmp(NULL){};

buf_fm_T(wchar_t* fn,long long  count=0,DWORD dwmode=OPMOD
	 ,DWORD dwShareMode=SHAREMOD,DWORD dwCreationDisposition=CRDIS):buf_T<T>()
 {
	 m_lock=false;
	 m_htmp=NULL;
	 open(fn,count,dwmode,dwShareMode,dwCreationDisposition);
 };

struct pair_mmn
{
  wchar_t filename[MAX_PATH*2];
  wchar_t* mapname;
  pair_mmn(wchar_t* fn):mapname(NULL)
  {
	  *filename=0;
    if(!fn) return ;
		wcscpy(filename,fn);
    //wchar_t*
    wchar_t* p;
	if(p=wcsstr(filename,L"::"))
	{
    	*p=0;
       mapname=p+2;
   	}
	
  };
  inline operator bool()
  {
	  return filename[0];
  }
};


 inline T* open(wchar_t* fn,long long count=0,DWORD dwmode=OPMOD,DWORD dwShareMode=SHAREMOD
	 ,DWORD dwCreationDisposition=CRDIS,DWORD fileattr=0)
 {
	 HRESULT hr;
	 LastErrorSaver lerr;
	 unmap();
	 HANDLE h=0;
	 
	 DWORD siz;
   HANDLE hf=INVALID_HANDLE_VALUE,hm;
      struct pair_mmn mmn(fn);
	   siz=count*sizeof(T);
	  if(mmn)
	  {
       hf=CreateFileW(mmn.filename,dwmode,dwShareMode,NULL,dwCreationDisposition,fileattr,NULL);
	   lerr.set();
	   hr=lerr;
       if(INVALID_HANDLE_VALUE==hf) return NULL;

	    if(count<0) 
		{
			 LARGE_INTEGER ll;
			if(GetFileSizeEx(hf,&ll))
			{
				siz=ll.QuadPart;
				count=siz/sizeof(T);
			}
			else count=0;
		}
		//else    
	  }  ;
	 

	 //{
		 
         m_count=count;

	 	 DWORD flProtect=(dwmode&GENERIC_WRITE)? PAGE_READWRITE:PAGE_READONLY;

		 if(hm=CreateFileMappingW(hf,NULL,flProtect|SEC_COMMIT,0,siz,mmn.mapname)) 
	 {
		 
     	 DWORD dwDesiredAccess=(dwmode&GENERIC_WRITE)? FILE_MAP_ALL_ACCESS:FILE_MAP_READ;
    	  if(m_p=(T*)MapViewOfFile(hm,dwDesiredAccess,0,0,siz)) 
			  lerr.set();
			  m_count=count;
          CloseHandle(hm);
	 } else lerr.set();
		 OVERLAPPED o=OVERLAPPED(); 
		 //		 if(m_lock)	m_lock=LockFileEx(hf,LOCKFILE_FAIL_IMMEDIATELY|LOCKFILE_EXCLUSIVE_LOCK,NULL,count,0,&o);
		 if((fileattr&FILE_FLAG_DELETE_ON_CLOSE)||m_lock)m_htmp=hf;
		 else   CloseHandle(hf);
	  
	 //}
	 return m_p;
 }
 
 inline bool flush()
 {
		 return (m_p)&&(FlushViewOfFile(m_p,m_count));
 }
 inline bool unmap()
 {
    if(!m_p) return false;
	 T* tmp=m_p;
	 m_p=NULL;
	 CloseHandle(m_htmp);
    return UnmapViewOfFile(tmp);
 }
 virtual ~buf_fm_T()
 {
	 unmap();
 }
//inline operator T* (){ 	return m_p;}

};




template <class IniFini>
wchar_t* tmp_file_string_object(VARIANT* vo,wchar_t* txt,wchar_t* ext=L"%tmp%\\~%~u~%.TMP",bool flocked=true)
{
	if(!vo) return 0;

	struct fileholder:CDispatchFunction<fileholder,CDynamicUnknown<IDispatch,&__uuidof(IDispatch)> >
	{
		IniFini inifini;
		//wchar_t fn[MAX_PATH];  
		bstr_t fn;
		buf_fm_T<char> buf;
		fileholder(wchar_t* txt,wchar_t* ext,bool flocked)
		{ 
			wchar_t bb[MAX_PATH];
			bstr_t ctxt=txt;
			
			int cb=ctxt.length();
			  SetEnvironmentVariableW(L"~u~",unique_str());
			  //bstr_t bext
				  fn=_ExpandEnvironmentStrings(ext);
                 buf.open(fn,cb+1);
			  //wsprintf(fn,bext,(wchar_t*)unique_str());

/*
           if(GetTempPathW(MAX_PATH,fn))
		   {
			   StrCatW(fn,unique_str());
			   StrCatW(fn,ext);
               buf.open(fn,cb+1);
			   
		   }
*/
			/*
			if(GetTempPathW(MAX_PATH,bb)&&GetTempFileNameW(bb,prfx,0,fn))
			{
               buf.open(fn,cb+1);
			}
			*/
		   if(cb>0) lstrcpynA(buf,ctxt,cb+1);
		   if(!flocked) buf.unmap();
		}
		inline HRESULT call(int argc,VARIANT* pargv,VARIANT* presult,EXCEPINFO* pexcepinfo)
		{
			if(presult)
			{
			 if(argc>0) *presult=variant_t((char*)buf).Detach();
			 else *presult=variant_t(fn).Detach();
			}
			return S_OK;
		};
    ~fileholder()
	{
		buf.unmap();
		DeleteFileW(fn);
	}


	} ;


	V_VT(vo)=VT_DISPATCH;
       fileholder *pfh=new fileholder(txt,ext,flocked);
    V_DISPATCH(vo)=pfh->get_Dispatch(1);
	return pfh->fn;
	//	template <class T,class Unk=CStaticUnknown<IDispatch,&__uuidof(IDispatch)> >
	//  class CDispatchFunction: public CDispatchT<T,Unk> 

	//wchar_t buf1[MAX_PATH],  buf2[MAX_PATH];

}

#pragma pack(push)
#pragma pack(1)
typedef struct _HOOK_PROCESS_INFO
{  
	union{
		
		struct
		{
			LONG unicode_prefix;
			LONG fready;
//#ifdef _XA64
    		LONG hwndPP;
//#else
	//		HWND hwndPP;
//#endif
			DWORD wshow;
			BOOL fullscreen;
			DWORD flags;
			wchar_t pp_info[1024];
			wchar_t appname[1024];
			wchar_t cmdline[1024];
			wchar_t currdir[1024];
			wchar_t objref[1024];
			wchar_t environ[32*1024];
		};
		struct{ wchar_t _resrvd[64*1024];};
	};
	
   } HOOK_PROCESS_INFO,*PHOOK_PROCESS_INFO;
#pragma pack(pop)

inline PHOOK_PROCESS_INFO inline_ltx_GetDEBUGProcessInfo(wchar_t* pexename)
{
	buf_fm_T<HOOK_PROCESS_INFO>::pair_mmn pmap=pexename;
	bstr_t  fn;
    if(pmap.mapname)
	{
		fn=pmap.mapname;
	}
	else{
		fn=(pexename)?pexename:(wchar_t*)argv_ini<wchar_t>::get_module_name(0);
	    fn+=L".LifeTimeX_hook";
	};
	return buf_fm_T<HOOK_PROCESS_INFO,OPEN_EXISTING,GENERIC_READ>(fn,1).detach();
}
inline   bool inline_ltx_FreeDEBUGProcessInfo(PHOOK_PROCESS_INFO ppi)
{
	if(ppi) return UnmapViewOfFile(ppi);
	return 0;
}