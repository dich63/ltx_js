#pragma once

//#define WINBASE_DECLARE_GET_MODULE_HANDLE_EX
#include <windows.h>
#include <windef.h>
//#include <atldef.h>
#include <comdef.h>
#include <shellapi.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atldef.h>
#include <comdef.h>
#include <vector>
#include <atlenc.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <conio.h>
#include "free_com_factory.h"
//
#include "dispid_find.h"

#define DICH_KEY "Software\\_Dich_Heap"
#define DICH_KEYW L"Software\\_Dich_Heap"

#define DISPATCH_GETSF (DISPATCH_PROPERTYGET|DISPATCH_METHOD)
#define DISPATCH_PUTSF (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)



#define RESET_FLAGS(s,f) ( s&=(~(f)) )
#define   SET_FLAGS(s,f) ( s|=( (f)) )



#define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT  (0x00000002)
#define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS        (0x00000004)

 
#define __MULTLOCAL02(a,b) a##b
#define __MULTLOCAL01(a,b) __MULTLOCAL02(a,b)
#define __MULTLOCAL00(prefix) __MULTLOCAL01(prefix,__COUNTER__)

#define LOCAL_DECLARE(_TYPE) _TYPE __MULTLOCAL00(_00_local__)
#define THIS_LOCAL_LOCK  LOCAL_DECLARE(ObjectLock)(this)
#define LOCK_METHOD  THIS_LOCAL_LOCK
#define LOCK_METHOD_R  LOCAL_DECLARE(ObjectLock)(*this)

#define CS_AUTOLOCK(cs) LOCAL_DECLARE(CComCritSecLock<CComAutoCriticalSection>)(cs)

//ObjectLock __MULTLOCAL00(zzz)(this);
#define pQI(x) ((void**)&(x))

#define COMINIT LOCAL_DECLARE(COMInit)
#define COMINIT_F COMINIT(COINIT_MULTITHREADED)
#define COMINIT_A COMINIT(COINIT_APARTMENTTHREADED)

#define safe_in_bstr(p) _bstr_t(p)
//g____xxx_00001

inline _bstr_t _ExpandEnvironmentStrings(_bstr_t str);
inline bool  _wlcheckni(wchar_t* p,wchar_t* ps,int cb=-1,wchar_t** pptail=NULL);

class COMInit
{
  public:
	HRESULT m_hr;
   COMInit(DWORD dw= COINIT_MULTITHREADED /*COINIT_APARTMENTTHREADED*/ )
   {
     m_hr=CoInitializeEx(NULL,dw); 
   }
   ~COMInit()
   {
	   //	   Beep(500,500);
	   if(SUCCEEDED(m_hr)) CoUninitialize();
   }
 inline operator HRESULT() { return m_hr;}
};


//#define ADDRESS_NEXT address_next<void>((void**)GetTickCount())
#define ADDRESS_NEXT s_address_next<void*>()
#define HINSTANCE_IN_CODE ( __getHinstance(ADDRESS_NEXT))
#define MODULE_PATH_FROM_ADDRESS(p)  get_module_path_W(__getHinstance(p))

inline _bstr_t  trim(wchar_t* p);

template <class T>
inline HRESULT VARIANT_OUT(const T& t, VARIANT* pout)
{
  if(!pout) return E_POINTER;
  

}

inline bool is_optional(const VARIANT& v,bool fex=false)
{

	if(fex)
	{
		VARTYPE vt=v.vt&VT_TYPEMASK;
		if(vt==(VT_VARIANT)&&(v.vt&VT_BYREF))
		{
			if(!v.byref) return true;
			vt= v.pvarVal->vt;
		}
       fex=(vt==VT_EMPTY)||(vt==VT_NULL)||(vt==VT_ERROR);

	   return fex;
	}


	return (v.vt==VT_ERROR)&&(v.scode==DISP_E_PARAMNOTFOUND);
}
inline int VINT_def(VARIANT&v ,int id=0,bool fex=false)
{
	VARIANT t=VARIANT();
	

	if(is_optional(v,fex)) return id;
	if(FAILED(VariantChangeType(&t,&v,0,VT_I4))) return id;
	return t.intVal;
}

inline double VDOUBLE_def(VARIANT&v ,double id=0,bool fex=false)
{
	VARIANT t=VARIANT();


	if(is_optional(v,fex)) return id;
	if(FAILED(VariantChangeType(&t,&v,0,VT_R8))) return id;
	return t.dblVal;
}

#define VNOPARAM VarError(DISP_E_PARAMNOTFOUND)
inline VARIANT VarError(HRESULT hr=DISP_E_PARAMNOTFOUND)
{
	VARIANT v={VT_ERROR,0,0,0,hr};
	return v;
}



template <class T>
struct s_address_next
{

static __declspec(noinline) void* __stdcall _next(void** p)
{
	void* res=(void*)(*(&p-1));
	return res;
};
inline operator T()
{
	return T(_next((void**)this));
}
};


inline _bstr_t bstr_alloc(size_t cb,wchar_t* pstr=NULL)
{
	
	return _bstr_t(::SysAllocStringLen(pstr,cb+1),false);
}



template <class T>
struct enum_windows
{
    
	static BOOL __stdcall s_enum_proc(HWND hw,LPARAM lp)
	{
	    return ((T*)lp	)->enum_proc(hw);
            
	};

	bool start()
	{
       return EnumWindows(&s_enum_proc,LPARAM(((T*)this))); 
	};
};

struct FindProcessWindow:public enum_windows<FindProcessWindow>
{
  HWND m_hwin;
  DWORD m_pid;
  bstr_t m_wc,m_wt;
  bstr_t m_buf;
  int lwc,lwt;

  FindProcessWindow(wchar_t* wc,wchar_t* wt=NULL,DWORD pid=GetCurrentProcessId())
	  :m_wc(wc),m_wt(wt),m_pid(pid)
  {
	  lwc=m_wc.length();
	  lwt=m_wt.length();
  };


inline bool check_pid(HWND hw)
{
	DWORD dwpid=0;
	GetWindowThreadProcessId(hw,&dwpid);
 return  (dwpid==m_pid);
}

inline BOOL enum_proc(HWND hw)
{
   if(!check_pid(hw)) return true;
   wchar_t* p=m_buf;
      *p=0;
	  bool fc=true,ft=true;
   if((lwc)&&GetClassNameW(hw,p,1024))
   {
	  fc=bool(StrStrIW(p,m_wc));
   }
   *p=0;
   if((lwt)&&GetWindowTextW(hw,p,1024))
   {
	   ft=bool(StrStrIW(p,m_wt));
   }
    if(fc&&ft) 
	{
      m_hwin=hw;
	  return false;
	}
    return  true;

}

inline   operator HWND()
  {
	  if(lwc||lwt)
	  {
      if(!(m_hwin=FindWindow(m_wc,m_wt))) return m_hwin; 
      if(m_pid==0) return m_hwin; 
      if(check_pid(m_hwin)) return m_hwin;
	  }

      m_hwin=0;
	  m_buf=bstr_alloc(256*256);
      start();
     return m_hwin; 

  }



};






template <class T>
__declspec(noinline) T* __stdcall address_next(T** p)
{
       T* res=(T*)(*(&p-1));
	return res;
};

inline HMODULE __stdcall __getHinstance(void* p)
{
	HMODULE h=0;
	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)p,&h);
	return h;
};

#define get_module_path get_module_path_W
inline _bstr_t get_module_filename(HMODULE h=HINSTANCE_IN_CODE)
{
        wchar_t wbuf[2048];
		GetModuleFileNameW(h,wbuf,2048);
		return wbuf;
}
inline _bstr_t get_module_path_W(HMODULE h=HINSTANCE_IN_CODE,bool erasebs=false)
{
	wchar_t wbuf[4096],*p=0;
	{
		wchar_t wbuf2[4096];
		GetModuleFileNameW(h,wbuf2,4096);
		GetFullPathNameW(wbuf2,4096,wbuf,&p);
	};
	if(p)
	{
     if(erasebs&&(ULONG_PTR(p)>ULONG_PTR(wbuf))) *(p-1)=0;
	 else *p=0;
	}
	return _bstr_t(wbuf);
};
inline void* _set_dll_proc(HMODULE h,void** fp,char* name)
{
	return (h)?(*fp=(void*)GetProcAddress(h,name)):NULL;
}

#define SET_PROC_ADDRESS_prfx(h,a,prfx) _set_dll_proc(h,(void**)&prfx##a,#a)
#define SET_PROC_ADDRESS(h,a) _set_dll_proc(h,(void**)&a,#a)
#define SET_PROC_ADDRESS_NS(h,ns,a) _set_dll_proc(h,(void**)&ns##::##a,#a)



template<class B>
inline B* InterlockedAttach(ATL::CComPtr<B>& sp,B* p)
{
	if(sp.p==p) return p;
	B* tmp=sp.p;
	void* pp=InterlockedExchangePointer((void**)&(sp.p),(void*)p);
	if(tmp) tmp->Release();
	return (B*)pp;
}

template <class LIST>
inline _bstr_t named_value(wchar_t* name,int argc, LIST const& argv,LPWSTR sep=L"=#",int *pind=0,DWORD flag=0)
 {
   int sl=wcslen(name);
   if(sl) 
   for(int n=0;n<argc;n++)
   { 
	   wchar_t *pn,*ptail=0;
	   _bstr_t tmp=argv[n];
	   pn=wcstok_s(tmp,sep,&ptail);
	   if(flag==1) sl=max(sl,wcslen(pn));
      if(0==StrCmpNIW(name,pn,sl)) 
	  {
        if(pind) *pind=n; 
		 return  ptail;
	  } 
   }

   if(pind) *pind=-1;
   return (wchar_t*)(NULL);
 }


class wcharlist
{
   public:
class v_adapter
{
  public:
  wcharlist &m_owner;
  _bstr_t m_name;
  v_adapter(wcharlist &o,_bstr_t name):m_owner(o)
  {m_name=name;};
  template<class T>
  operator T()
  {
	return _variant_t(m_owner.get_var(m_name));
  }
 template<class D>
  D def(D dflt,bstr_t prfx=bstr_t())
  {
	  try
	  {
		  int ind=-1,l;
		  _bstr_t b=m_owner.get_var(m_name,&ind);
		  if(ind>=0)
		  {
			  wchar_t* p=b;
			  if(l=prfx.length())
				  _wlcheckni(p,prfx,l,&p);
			  return _variant_t(p) ;
		  }
		  
	  } catch (...){}
	  return dflt;
	  
  }
  v_adapter& operator =(_variant_t v)
  {
      m_owner.set_var(m_name,_bstr_t(v)); 
	  return *this;
  }

};

   std::vector<LPWSTR> vpwc;
   union{
   LPWSTR pwbuf;
   LPSTR pcbuf;
   };
long tot_len;

inline _bstr_t print(_bstr_t delim=L"\n")
{
	_bstr_t s;
   for(int i=0;i<vpwc.size()-1;i++)
       s+=vpwc[i]+delim;
   for(int i=vpwc.size()-1;i<vpwc.size();i++)
	   s+=vpwc[i];
  return s;
}

inline  wcharlist& append(wcharlist& cl)
{
	_bstr_t buf;
	wchar_t* ptail=0,*ptoc;
	long cb=cl.vpwc.size();
  for(int n=0;n<cb;n++)
  {
    buf=cl.vpwc[n];
	if(ptoc=wcstok_s(buf,L"=",&ptail))
	 set_var(ptoc,ptail);	
  };

	return *this;
}

inline void clear()
{

for(std::vector<LPWSTR>::iterator it=vpwc.begin();it!=vpwc.end();it++)
    free(*it);
  vpwc.clear();
  free(pwbuf);
  pwbuf=NULL;
}
inline wcharlist& init(int argc,LPWSTR* argv)
{
	//vpwc.clear();
	clear();
  for(int n=0;n<argc;n++)
  {
	vpwc.push_back(_wcsdup(argv[n]));
  }
  return *this;
}
wcharlist():pwbuf(NULL){};

inline wcharlist& init(wchar_t* p)
{
	clear();
   
	while((p)&&(*p))
	{
		vpwc.push_back(_wcsdup(p));
		p+=wcslen(p)+1;
	}
	
	return *this;
}


wcharlist(wchar_t* pp ):pwbuf(NULL),tot_len(0)
{
  //wchar_t* p=pp;
	init(pp);
}


 //_bstr_t named_value(wchar_t* name,int argc, LIST const& argv,LPWSTR sep=L"=#",int *pind=0)
inline _bstr_t get_var(_bstr_t name,int* pind=NULL)
{
 return named_value(name,vpwc.size(),vpwc,L"=",pind,1);
}

inline void set_var(_bstr_t name,_bstr_t value)
{
	int ind;

	_bstr_t tmp=name+L"="+value;
	LPWSTR psnew=_wcsdup(tmp);  
	if(!named_value(name,vpwc.size(),vpwc,L"=",&ind,1))
		  vpwc.push_back(psnew);
	else
	{
       LPWSTR& rs=vpwc.at(ind);
	   free(rs);
	   rs=psnew;
	}  
	;
}

inline PVOID flat_str(bool funicode=true)
{
tot_len=0;
size_t cb=0,off=0;
free(pwbuf);pwbuf=0;
for(std::vector<LPWSTR>::iterator it=vpwc.begin();it!=vpwc.end();it++)
{
  pwbuf=(LPWSTR)realloc(pwbuf,((cb+=wcslen(*it)+1)+2)*sizeof(wchar_t));
  if(funicode) wcscpy(pwbuf+off,*it);
  else 
      strcpy(pcbuf+off,CW2A(*it));
  
  off=cb;
}
if(pwbuf)
{
if(funicode) pwbuf[off]=0;
else pcbuf[off]=0;
tot_len=off+1;
}

return pwbuf;
}



~wcharlist(){clear();};

v_adapter operator[](_bstr_t n)
{
	return v_adapter(*this,n);
}

inline wchar_t* item(int n)
{
	return vpwc[n];
};

inline int count()
{
  return vpwc.size();
}
inline  bool is_empty()
{
	return  count()==0;
}
};





inline _bstr_t reg_value(HKEY hk,_bstr_t value, _bstr_t subkey,_bstr_t dfltval)
{
	
	HKEY the_hk=NULL;
    
	if(subkey.length()!=0)
	{
        if(ERROR_SUCCESS!=RegOpenKeyW(hk, subkey,&the_hk))
			return  (wchar_t*)dfltval;
		hk=the_hk;
	} 
	unsigned long cb=2048,ct=0;
	_bstr_t res=bstr_alloc(cb);
	 wchar_t* buf=res; 
      cb*=sizeof(wchar_t); 
	if(ERROR_SUCCESS!=RegQueryValueExW(hk,value,NULL,&ct,(LPBYTE)buf,&cb))
		buf=dfltval;

     if(the_hk) RegCloseKey(the_hk);

	 return buf;

};



inline _bstr_t get_module_ininame()
{
     wchar_t buf[2048];
	if(!GetModuleFileNameW(HINSTANCE_IN_CODE,buf,2048)) return BSTR(NULL);
	return bstr_t(buf)+bstr_t(L".ini");
}

inline _bstr_t get_private_profile_string(_bstr_t sec,_bstr_t nam,_bstr_t def="",_bstr_t filen=BSTR(NULL))
{
    
   if(filen.length()==0)  filen=get_module_ininame();

   wchar_t buf[2048];

   if(!GetPrivateProfileStringW(sec,nam,def,buf,2048,filen)) return BSTR(NULL);
  
   return buf;

}
inline int get_private_profile_int(_bstr_t sec,_bstr_t nam,int def=0,_bstr_t filen=BSTR(NULL))
{
	if(filen.length()==0)  filen=get_module_ininame();
	return GetPrivateProfileIntW(sec,nam,def,filen);
}

inline bool get_private_profile_section(wcharlist& section, bstr_t secnam,bstr_t filen=BSTR(NULL))
{

	if(filen.length()==0)  filen=get_module_ininame();

	wchar_t buf[2048];
	if(!GetPrivateProfileSectionW(secnam,buf,2048,filen)) return false;
     section.init(buf);
	return true;
}

class cmd_split
{
 public:
	 //_bstr_t arg0,tail;
     int argc;
	 LPWSTR* pargv,*pargv0;
	 wchar_t* strex;
   //  _bstr_t str;
   inline void init(LPCWSTR _str=GetCommandLineW(),int offset=0)
   {

	 //	   str=_str;
	   clear();
	   int cb;

	   if(cb=ExpandEnvironmentStringsW(_str,0,0))
	   {
		   strex=new wchar_t[cb]; 
		   ExpandEnvironmentStringsW(_str,strex,cb);
		  pargv0=pargv=CommandLineToArgvW(strex,&(argc=0));   

          pargv+=offset;
          argc-=offset; 

	   }


   }
   inline void clear()
   {
	   if(pargv0) LocalFree(pargv0);
	   if(strex) delete[] strex;
       pargv=0;strex=0;

   }
inline    cmd_split():argc(0),pargv0(0),pargv(0),strex(0)
	{

	};
///*
inline    cmd_split( cmd_split & src):argc(0),pargv0(0),pargv(0),strex(0)
	 {
		 init(src.strex);
	 }
//*/
inline	 cmd_split(LPCWSTR _str,int offset=0):argc(0),pargv0(0),pargv(0),strex(0)
	{
        if(_str&&(*_str))   init(_str,offset);
	};
	~cmd_split()
	{
		clear();
	}



inline cmd_split & operator =(cmd_split  src)
{
 init(src.strex);
 return *this;
}

inline LPWSTR  argv(int n=-1)
{
  if(n<0) return strex;
  if(n<argc) return pargv[n];
  return NULL;
};
inline  _bstr_t  tail(int n=1,_bstr_t qu="")
{
    _bstr_t t,sp=L" ";
int i;
  for( i=n;i<argc-1;i++) 
 	             t+=qu+pargv[i]+qu+sp;
  for(;i<argc;i++)
	         t+=qu+pargv[i]+qu; 
  return t;
};

inline _bstr_t operator[](int ind)
{
	return argv(ind);
};

inline _bstr_t operator[](const _bstr_t name)
{
	return named_value(name,argc,pargv,L"=#");
};

inline _bstr_t named_argv(_bstr_t name)
{
	return named_value(name,argc,pargv,L"=#");
};

inline bool is(_bstr_t name)
{  
	int ind=-1;
    named_value(name,argc,pargv,L"=#",&ind).length();
	return (ind>=0);
}
inline _variant_t value(_bstr_t name,_variant_t dflt=_variant_t())
{
	int ind=-1;
	_bstr_t v=named_value(name,argc,pargv,L"=#",&ind);
	return (ind>=0)?  _variant_t(v):dflt;
};


inline bool operator !()
{
	return argc==0;
}

inline bstr_t cppstr(wchar_t* s)
{
	int cb=(s)?wcslen(s):0;
	bstr_t r;
	for(int n=0;n<cb;n++)
	{
		wchar_t cc[2]={0,0};
		 *cc=s[n];
		if(*cc==L'\\') r+=L"\\\\";
		else r+=cc;

	}
   return r;
}
inline bstr_t to_JSON()
{
       bstr_t b=L"[", bq=L"\"",bqc=L"\",";

   int n=0;
   for(;n<argc-1;n++)
   	   b+=bq+cppstr(argv(n))+bqc;
   for(;n<argc;n++) b+=bq+cppstr(argv(n))+bq;
   b+=L"]";
  return b;
}
};





inline cmd_split assoc_module(_bstr_t fn)
{
	wchar_t* pext=PathFindExtensionW(fn);
	wchar_t buf[1024];
	DWORD cb=1024;
    if(AssocQueryStringW(0,ASSOCSTR_COMMAND,pext,0,buf,&cb)) return cmd_split();
	return buf;
}

class environ_str
{
	LPWSTR m_enw;
 public:
inline 	 environ_str()
	 {
       m_enw=GetEnvironmentStringsW();
	 }
	 ~environ_str()
	 {
		 FreeEnvironmentStringsW(m_enw);
	 }
inline operator LPWSTR()
{
	return m_enw;
}
   
};

inline _bstr_t get_env_var(_bstr_t name)
{
	wchar_t buf[4096];
	if(GetEnvironmentVariableW(name,buf,4096))
		return buf;
	else return BSTR(NULL);
}
inline  _bstr_t set_env_var(_bstr_t name,_bstr_t value=BSTR(NULL),bool f_if_exists=0 )
{
	wchar_t buf[4096]=L"";
	if(GetEnvironmentVariableW(name,buf,4096)&&f_if_exists) return buf;
	SetEnvironmentVariableW(name,value);
	return buf;
	
}

inline bstr_t msg_error(DWORD err=GetLastError())
{
	wchar_t lpMsgBuf[1024]=L"";
   const DWORD lang=  MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US);  //MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
	FormatMessageW( 
		//FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		lang, // Default language
		(LPWSTR) lpMsgBuf,
		1024,
		NULL 
		);

	// Display the string.
       bstr_t ret=lpMsgBuf;       

	// Free the buffer.
	//LocalFree( lpMsgBuf );
  return ret;
}

inline HWND wnd_alloc(char* winclass,char* winname=NULL,HWND hwinParent=0)
{
	WNDCLASSA wc;
	HINSTANCE HInstance=GetModuleHandleA(NULL);
	if(!GetClassInfoA(HInstance,winclass,&wc) )
	{
		memset(&wc,0,sizeof(wc));
		wc.hInstance = HInstance;
		wc.lpszClassName=winclass;
		wc.lpfnWndProc=DefWindowProc;
		if ( ! RegisterClassA( &wc ) ) return NULL;
	};
	HWND hwnd=CreateWindowA(winclass,winname,0,0,0,0,0,hwinParent,NULL,HInstance,0);
return hwnd;
}
inline HWND wnd_lite(char* winclass)
{
	return wnd_alloc(winclass,0,HWND_MESSAGE);
};

class ComClipboard
{
public:
	CComPtr<IDispatch> m_cc;
	HRESULT m_hr;
	_variant_t m_v;

inline HRESULT init()
{
  return	m_hr=m_cc.CoCreateInstance(L"combin.clipbrd");
}

  ComClipboard(IUnknown* punk=NULL)
  {
   if(SUCCEEDED( init())&&punk)
   {
	   VARIANT V={VT_UNKNOWN};
	   V.punkVal=punk;
        m_hr= m_cc.PutProperty(DISPID_VALUE,&V);
   }
  };
  ComClipboard(VARIANT V)
  {
	 if(SUCCEEDED( init()))
     m_hr= m_cc.PutProperty(DISPID_VALUE,&V);

  };

inline HRESULT getV()
{
	if(FAILED(m_hr)) return m_hr; 
	return m_cc.GetProperty(DISPID_VALUE,m_v.GetAddress());
};
inline operator IDispatch*()
{
	HRESULT hr;
  if(SUCCEEDED(getV())
	  &&SUCCEEDED(hr=VariantChangeType(&m_v,&m_v,0,VT_DISPATCH))) 
  return m_v.pdispVal;
   return NULL; 
  
}
inline operator IUnknown*()
{
	HRESULT hr;
	if(SUCCEEDED(getV())
		&&SUCCEEDED(hr=VariantChangeType(&m_v,&m_v,0,VT_UNKNOWN))) 
		return m_v.punkVal;
	return NULL; 
}
inline operator variant_t& () 
{
	HRESULT hr;
	if(SUCCEEDED(hr=getV())) return m_v; 
	else return  *(m_v.GetAddress())=VarError(hr),m_v;
}

};

class ClipboardText
{
	HWND hwin;
 public:
 ClipboardText()
 {
	 hwin=wnd_lite("Clip00");
	 OpenClipboard(hwin);
 };
  ClipboardText(_bstr_t str)
  {
         hwin=wnd_lite("Clip00");
    OpenClipboard(hwin);
	EmptyClipboard();
	long cb;

		cb=(wcslen(str)<<1)+2;
	HANDLE hm=GlobalAlloc(GMEM_FIXED,cb);
	StrCpyW((wchar_t*)hm,str);
	SetClipboardData(CF_UNICODETEXT,hm);

	cb>>=1;
	hm=GlobalAlloc(GMEM_FIXED,cb);
	StrCpyA((char*)hm,str);
	SetClipboardData(CF_TEXT,hm);

  }
  inline operator _bstr_t()
  {

   if(IsClipboardFormatAvailable(CF_UNICODETEXT)) 
   {
    HANDLE hm=GetClipboardData(CF_UNICODETEXT);
    return (wchar_t*)hm;
   } 
   else if(IsClipboardFormatAvailable(CF_TEXT)) 
   {
	   HANDLE hm=GetClipboardData(CF_TEXT);
	   return (char*)hm;
   }
	return BSTR(0);

  };
~ClipboardText()
{
 CloseClipboard();
 DestroyWindow(hwin);
}
};


class CAddPath
{
public:
	bstr_t old;
	bool flocal;
	CAddPath(bstr_t path,bool _local=false):flocal(_local)
  {
	  //bstr_t path=
	  path=_ExpandEnvironmentStrings(path);
     old=get_env_var(L"PATH");
	 path=path+";";
	 LPWSTR pfindstr=StrStrIW(old,path);
	 if(!pfindstr)
	 {
		 bstr_t new_path=path+old;
          set_env_var(L"PATH",new_path);
	 }

  };
~CAddPath()
{
	if(flocal) set_env_var(L"PATH",old);
}

inline operator bstr_t()
{
	return get_env_var("PATH");
}

};


inline _bstr_t _ExpandEnvironmentStrings(_bstr_t str)
{
	_bstr_t strex;
	int cb=ExpandEnvironmentStringsW(str,NULL,0);
	if(cb) 
	{
		BSTR bstr=::SysAllocStringLen(NULL,cb+16);
		ExpandEnvironmentStringsW(str,bstr,cb+16);
		strex=(wchar_t*)(bstr);
	}
	return strex;
};

inline _bstr_t current_dir(bool flag=0)
{
         _bstr_t bstr=bstr_alloc(1024);
    GetCurrentDirectoryW(4096,bstr);
  if(flag) return bstr;
  else  return (wchar_t*)(bstr);   
}

class localCDir
{
public:
	wchar_t wbuf[4096];
	localCDir(wchar_t* newdir)
	{
		GetCurrentDirectoryW(4096,wbuf);
		SetCurrentDirectoryW(newdir);
	}
	~localCDir(){SetCurrentDirectoryW(wbuf);}
};



#define _FILE_ACCESS_RET(file) if( _file_access(file)) return;

inline bool file_exists(wchar_t* path)
{
 //
	DWORD attr = GetFileAttributesW(path);
  return (attr != INVALID_FILE_ATTRIBUTES);
   
//	wchar_t buf[2],*p;
//DWORD nn=SearchPath(NULL,path,NULL,1,buf,&p); 
 //return nn;
}

class FileSearcher
{
protected:

/*
inline bool _file_access(_bstr_t& file)
{
	wchar_t buf[4096];
	wchar_t *p=0;
	if(fok=!(nerr=_waccess(file,0)))
	{
		if(GetFullPathNameW(file,4096,buf,&p))
			fullpath=buf;
		else fullpath=file;
		
	};  

	return fok;
}
*/

 public:
	 bool fok;
	 errno_t nerr;
   _bstr_t fullpath;
	  FileSearcher():fok(0){};





	  FileSearcher(_bstr_t file,_bstr_t path=_bstr_t(),_bstr_t separator=L";"):fok(0)
	  {

		  wchar_t* seps=L"\0";
		  _bstr_t bbuf,ffile;
		   wchar_t buf[4096];
		   wchar_t* ppath=path;  
		   wchar_t *p=0,*pfile;
		   bool ff;

		   if(separator.length()) 
			    seps=separator;

		    if(file.length()==0) return;
            file=trim(_ExpandEnvironmentStrings(file));
            if(file.length()==0) return;

			 pfile=file;
            if(0==StrCmpIW(pfile,L"*"))
			{
				fullpath=get_module_filename(NULL); 
				fok=1;nerr=0;
				return;
			}

			if((StrCmpNIW(pfile,L"*/",2)==0)||(StrCmpNIW(pfile,L"*\\",2)==0))
			{
				bbuf=get_module_path_W(NULL)+(pfile+2);
				file=bbuf;
			}
			if((StrCmpNIW(pfile,L"**/",3)==0)||(StrCmpNIW(pfile,L"**\\",3)==0))
			{
				file=bbuf=get_module_path_W(HINSTANCE_IN_CODE)+(pfile+3);

			}  

		   if(!PathIsRelativeW(file))
		   {
			   if(fok=file_exists(file))
			   {
				   if(GetFullPathNameW(file,4096,buf,&p))
					   fullpath=buf;
				   else fullpath=file;
				   // 	   EACCES
				  
			   };           	 
                return;
		   }

		   {
			   //if(!(ppath&&(*ppath)) )
			   if((path.length()==0))
			   {
				   //wchar_t buf[4096],*p=0;
				   if(fok=SearchPathW(NULL,file,NULL,4096,buf,&p))
					   fullpath=buf;
				   else nerr=GetLastError();

				   return ;				   
			   }
		   }
/*		             
		 // if(!(ppath&&(*ppath)))
          if(path.length()==0)
		  {
			  if(fok=!(nerr=_waccess(file,0)))
				 {
					 if(GetFullPathNameW(file,4096,buf,&p))
						 fullpath=buf;
					 else fullpath=file;
					 return;
			  };           	 
		  }
		  else
*/
  		  {
             if(*ppath==seps[0])
			 {
				 if(fok=SearchPathW(NULL,file,NULL,4096,buf,&p))
				 { 
					 fullpath=buf;
                     return;
				 }
				 else nerr=GetLastError();
              _bstr_t tmp=ppath+1;
			   path=tmp;
			 }

			 path=_ExpandEnvironmentStrings(path);
			 
             
			 wchar_t* token = wcstok_s(path, seps,&p );
             _bstr_t bsbuf; 
			 while( token != NULL )
			 {
				 if(StrCmpNIW(token,L"*/",2)==0)
				 {
					bsbuf=get_module_path_W(NULL)+(token+2);
            	     token=bsbuf;
				 }
				 else if(StrCmpNIW(token,L"**/",3)==0)
				 {
					 token=bsbuf=get_module_path_W(HINSTANCE_IN_CODE)+(token+3);

				 }  
				 else if(StrCmpW(token,L".")==0)
				 {
					 GetCurrentDirectoryW(4096,buf);
					 token=bsbuf=buf;
				 }  

				 /* While there are tokens in "string" */
				 int sl=wcslen(token);
				 ffile=token;
				 if((sl)&&((token[sl-1]!=L'\\')||(token[sl-1]!=L'/')) )	 
					  ffile+=L"/";
    		     ffile+=file;
//				 if( ( fok=GetFullPathNameW(ffile,4096,buf,&p))
//					 &&( fok=!(nerr=_waccess(ffile,0)) )				 )
				 if(fok=file_exists(ffile))
				 {
                  if(GetFullPathNameW(ffile,4096,buf,&p))
                  fullpath=buf;
				  else fullpath=ffile;
				  return;
				 };           	 
				 /* Get next token: */
				 token =wcstok_s( NULL, seps,&p );
			 }
            return;
		  }
		  
		 			
	  }

  inline HRESULT Detach(BSTR* pbs,HRESULT fbad_ptr=E_POINTER)
  {
	  if(!pbs) return fbad_ptr;
	  if(!fok) return S_FALSE;
	  try
	  {
		  *pbs=fullpath.Detach();
		  return S_OK;
	  }
	  catch (...){};

	  *pbs=fullpath.copy();
	  return S_OK;
	  
  }
  inline  _bstr_t FileName()
  {
	  return (wchar_t*)fullpath;
  }
  inline operator const _bstr_t&()
  {
       return fullpath;
  }
  inline operator bool()
  {
          return fok;
  }
  inline operator HRESULT()
  {
	  return (fok)?S_OK:HRESULT_FROM_WIN32(nerr);
  }
};

inline  _bstr_t computer_name()
  {
	  wchar_t  buf2[1024];
	  DWORD pid=1024;
	  GetComputerNameW(buf2,&pid);
    return buf2;
  };

inline  _bstr_t process_info(void* proc=0)
{
	 wchar_t  buf[4096];
_bstr_t cn,pn,clib,cproc;
DWORD pid=1024;
GetComputerNameW(buf,&pid);
cn=buf;
/*
if(proc)
{
  HMODULE hm=__getHinstance(proc);
  if(hm!=GetModuleHandle(NULL))
  {

  }

}
else
*/
{
GetModuleFileNameW(NULL,buf,4096);
pn=buf;
pid=GetCurrentProcessId();
swprintf_s(buf,4096,L"\\\\%s\\\"%s\" pid=%d",(wchar_t*)cn,(wchar_t*)pn,pid);
return buf;
}
};

 inline HRESULT weak_RegisterCOMObject(IUnknown* unk,_bstr_t display_name,PDWORD pdwReg=0)
 {
	 DWORD dwReg;
	 HRESULT hr;
	 CComPtr<IUnknown> unkunk;
	 CComPtr<IMoniker> mon;
	 CComPtr<IRunningObjectTable> ROT;

	 if(FAILED(hr=unk->QueryInterface(IID_IUnknown,(void**)&(unkunk)))) return hr;
	 if(FAILED(hr=CreateFileMoniker(display_name,&mon))) return hr;
	 if(FAILED(hr=GetRunningObjectTable(0,&ROT))) return hr; 
	 if(FAILED(hr=ROT->Register(ROTFLAGS_ALLOWANYCLIENT,unkunk,mon,&dwReg)))  return hr;
	 if(pdwReg) *pdwReg=dwReg;
	 return S_OK;
 };


inline _bstr_t make_filemoniker_name(_bstr_t prefix,DWORD pid_or_tid=GetCurrentProcessId(),_bstr_t host=computer_name())
{
	wchar_t  buf2[1024];
	//wsprintf(buf2,L"%s.%s.[%08X]",(wchar_t*)prefix,(wchar_t*)host,pid_or_tid);
	wsprintfW(buf2,L"%s.%s.pid=%d",(wchar_t*)prefix,(wchar_t*)host,pid_or_tid);
	return buf2;
}

inline HRESULT xGetObject(const IID& riid,BSTR name, VARIANT* vobj)
 {
	 HRESULT hr;
	 if(!vobj) return E_POINTER;
	 VARIANT v={VT_UNKNOWN};
	 if(FAILED(hr=CoGetObject(name,0,riid,(void**)&(v.punkVal)))) return hr;
	 //VariantChangeType(&v,&v,0,VT_DISPATCH);
	 *vobj=v;
	 return S_OK;
 }

inline HRESULT GetObjectRefString(IUnknown* pUnk,_bstr_t& RefStr)
{
     HRESULT hr;
	CComPtr<IMoniker> mon;
	if(FAILED(hr=CreateObjrefMoniker(pUnk,&mon))) return hr ;

	BSTR bss=NULL;
	CComPtr<IBindCtx> BindCtx;
	if(FAILED(hr=CreateBindCtx(0,&BindCtx))) return hr ;
	if(FAILED(hr=mon->GetDisplayName(BindCtx,0,&bss))) return hr ;
	 RefStr=bss;
	CoTaskMemFree(bss);
    return S_OK;
}



inline HRESULT strong_RegisterCOMObject(IUnknown* unk,bstr_t display_name,PDWORD pdwReg=0)
{
	DWORD dwReg;
	HRESULT hr;
	CComPtr<IMoniker> mon;
	CComPtr<IRunningObjectTable> ROT;

	if(FAILED(hr=CreateFileMoniker(display_name,&mon))) return hr;
	if(FAILED(hr=GetRunningObjectTable(0,&ROT))) return hr; 
	if(FAILED(hr=ROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE,unk,mon,&dwReg)))  return hr;
	if(pdwReg) *pdwReg=dwReg;
	return S_OK;
};

template<class T>
class CHGlobal
{
public:
	HGLOBAL m_h;
	T* p;
	CHGlobal(HGLOBAL h):m_h(h),p(0)
	{
      if(m_h) p=(T*) GlobalLock(m_h);
	}


inline HGLOBAL detach()
{
    if(m_h) GlobalUnlock(m_h);
	HGLOBAL tmp=m_h;
	return tmp=m_h,m_h=NULL,tmp;
}
    ~CHGlobal()
	{
	 if(m_h) GlobalUnlock(m_h);
	}
inline operator T*()
{
	return p;
}
inline size_t size()
{
    return   GlobalSize(m_h);
};

};

 
 inline  HRESULT CoReleaseMarshalDataFromHGlobal(HGLOBAL hmem,bool free_hmem=1)
 {
	 if(!hmem) return E_POINTER;
	 HRESULT hr;
	 CComPtr<IStream> stream;
	 if(FAILED(hr=CreateStreamOnHGlobal(hmem,free_hmem,&stream))) return hr;
	 /*
	 LARGE_INTEGER l1;
	 ULARGE_INTEGER l2;
	 l1.QuadPart=0;
	 hr=stream->Seek(l1,STREAM_SEEK_CUR,&l2);
	 hr=stream->Seek(l1,STREAM_SEEK_SET,&l2);
	 */
 	 return hr=CoReleaseMarshalData(stream);

 };

#define MON_PREFIX "objref:"
#define MON_PREFIX_LN  (strlen(MON_PREFIX))




 inline bstr_t ObjRefFromHGlobal(HGLOBAL hmem)
{
  bstr_t res;
  CHGlobal<BYTE> ch(hmem);
  int cbs=ch.size();
  if(!cbs) return "";
  int cbd=Base64EncodeGetRequiredLength(cbs,ATL_BASE64_FLAG_NOCRLF);
  char* pref=(char*)calloc(1,cbd+MON_PREFIX_LN+15);
  strcpy_s(pref,MON_PREFIX_LN+1,MON_PREFIX);
  if(Base64Encode(ch,cbs,pref+MON_PREFIX_LN,&cbd,ATL_BASE64_FLAG_NOCRLF))
  {
	  pref[MON_PREFIX_LN+cbd]=':';
      res=pref;
  }
    free(pref);
	return res;
}


inline  HRESULT StubFromObjref(char* objref,HGLOBAL* phmem)
{
	if(!objref) return E_POINTER;
	int off=strlen("objref:");
	if(StrCmpNIA("objref:",objref,off)) return E_FAIL;
	objref+=off;
	int cb=strlen(objref);
	if(!cb)  return E_FAIL;
	BYTE*ph=(BYTE*)GlobalAlloc(GMEM_FIXED,cb--);
    if(!ph) return  E_OUTOFMEMORY;
	if(!Base64Decode(objref,cb,ph,&cb))  return GlobalFree(HGLOBAL(ph)),E_FAIL;

	if(phmem) *phmem=HGLOBAL(ph);
	else GlobalFree(HGLOBAL(ph));
   return  S_OK;
	  
}


//#define MON_PREFIX "objref:"
//#define MON_PREFIX_LN  (strlen(MON_PREFIX))



 inline  HRESULT CoReleaseMarshalDataFromObjRefString(char* objref)
 {
	 if(!objref) return E_POINTER;

	 //if(s)
	 int off=strlen("objref:");
	 if(StrCmpNIA("objref:",objref,off)) return E_FAIL;
	 objref+=off;
	 int cb=strlen(objref);
	 if(!cb)  return E_FAIL;
	 BYTE*ph=(BYTE*)GlobalAlloc(GMEM_FIXED,cb--);

	 if(!Base64Decode(objref,cb,ph,&cb))  return E_FAIL;
	 return CoReleaseMarshalDataFromHGlobal(HGLOBAL(ph));
 };

// inline  HRESULT CoMarshalInterfaceToHGlobal(IUnknown *pUnk,DWORD ctx=MSHCTX_DIFFERENTMACHINE,DWORD mshflags=MSHLFLAGS_TABLESTRONG ,/* out*/HGLOBAL* phmem)




 inline  HRESULT CoMarshalInterfaceToHGlobal(IUnknown *pUnk,DWORD ctx,DWORD mshflags,/* out*/HGLOBAL* phmem)
 {
	 if(!phmem) return E_POINTER;
	 if(!pUnk) return E_POINTER;
	 HRESULT hr,hr2;
	 HGLOBAL hmem;
	 CComPtr<IStream> stream;
	 if(FAILED(hr=CreateStreamOnHGlobal(0,false,&stream))) return hr;

	 hr2=CoMarshalInterface(stream,IID_IUnknown,pUnk,ctx,0,mshflags) ;

	 if(FAILED(hr=GetHGlobalFromStream(stream,&hmem))) return hr;
	 if(FAILED(hr2)) return GlobalFree(hmem),hr2;
	 *phmem=hmem;
	 return S_OK;
 };

 inline  HRESULT CoUnmarshalInterfaceFromHGlobal(HGLOBAL hmem,REFIID riid,void** ppv,bool frelease=false)
 {
	 if(!(hmem&&ppv)) return E_POINTER;
	 
	 HRESULT hr;
	 CComPtr<IStream> stream;
	 if(FAILED(hr=CreateStreamOnHGlobal(hmem,frelease,&stream))) return hr;
	 hr=CoUnmarshalInterface(stream,riid,ppv);
	 return hr;
 };





 inline  HRESULT CoUnmarshalInterfaceFromBuff(int cb,void* p,REFIID riid,void** ppv)
 {
	  HRESULT hr;
	 CComPtr<IStream> stream;
	 if(FAILED(hr=CreateStreamOnHGlobal(NULL,true,&stream))) return hr;
	 DWORD cwb;
	 if(FAILED(hr=stream->Write(p,cb,&cwb))) return hr ;
	  LARGE_INTEGER ll={};
   
     if(FAILED(stream->Seek(ll,STREAM_SEEK_SET,(ULARGE_INTEGER*)&ll))) return hr ;

	 hr=CoUnmarshalInterface(stream,riid,ppv);
	 return hr;
 };
 


template<class Intf>
  inline  HRESULT CoUnmarshalInterfaceFromHGlobal(HGLOBAL hmem,Intf** ppi,bool frelease=false)
  {
	  return CoUnmarshalInterfaceFromHGlobal(hmem,__uuidof(Intf),(void**)ppi,frelease);
  }

inline  HRESULT CoDuplicateInterface(HGLOBAL hin,HGLOBAL* phout,DWORD ctx=MSHCTX_LOCAL,DWORD mshflags=MSHLFLAGS_TABLESTRONG)
{
	 HRESULT hr;
	if(!phout) return E_POINTER;

	
	
     COMINIT_F;
	 IUnknown *punk=0;

	 if(FAILED(hr=CoUnmarshalInterfaceFromHGlobal(hin,&punk)))
		 return hr;

	  hr=CoMarshalInterfaceToHGlobal(punk,ctx,mshflags,phout);
      punk->Release(); 
	 return hr;	   

}

inline  HRESULT CoDuplicateInterfaceFromBuff(int cb,void* buf,HGLOBAL* phout,DWORD ctx=MSHCTX_LOCAL,DWORD mshflags=MSHLFLAGS_TABLESTRONG)
{
	HRESULT hr;
	if(!phout) return E_POINTER;



	COMINIT_F;
	IUnknown *punk=0;

	if(FAILED(hr=CoUnmarshalInterfaceFromBuff(cb,buf,__uuidof(IUnknown),(void**)&punk)))
		return hr;

	hr=CoMarshalInterfaceToHGlobal(punk,ctx,mshflags,phout);
	punk->Release(); 
	return hr;	   

}


class stub_holder
{
	public:
   HGLOBAL m_hmem;
   HRESULT m_hr;
   


   

   stub_holder(IUnknown*punk,DWORD ctx=MSHCTX_DIFFERENTMACHINE,DWORD mshflags=MSHLFLAGS_TABLESTRONG):m_hmem(NULL)
  {
      m_hr=CoMarshalInterfaceToHGlobal(punk,ctx,mshflags,&m_hmem);
  }




stub_holder():m_hmem(0),m_hr(E_POINTER){};

void operator =(stub_holder& sh)
{
  clear();
  m_hr=sh.m_hr;
  m_hmem=sh.m_hmem;
  sh.m_hr=E_FAIL;
}

void clear()
{
  if(SUCCEEDED(m_hr)) CoReleaseMarshalDataFromHGlobal(m_hmem);
  m_hr=E_FAIL;
}
~stub_holder()
{
   clear();
}



bstr_t objref()
{
	bstr_t r=ObjRefFromHGlobal(m_hmem);

    return r;
}

template<class Intf>
inline HRESULT unwrap(Intf** ppi)
{
	HRESULT hr;
    if(FAILED(m_hr)) return m_hr;
	if(!ppi) return E_POINTER;
    return hr=CoUnmarshalInterfaceFromHGlobal(m_hmem,__uuidof(Intf),(void**)ppi,false); 
}
inline HRESULT unwrap(VARIANT& v)
{
	//v.vt=VT_UNKNOWN;
	
	VARIANT t={VT_UNKNOWN};
    HRESULT hr=unwrap(&t.punkVal),hr0;
	if(SUCCEEDED(hr))
	{
		VARIANT td=VARIANT();
     if(SUCCEEDED(hr0=VariantChangeType(&td,&t,0,VT_DISPATCH)))
	 {
		 v=td;
		 VariantClear(&t);
	 }
	 else 
	 {
        v=t;  
	 }

	}
	return hr;
}
operator HRESULT()
{
	return m_hr;
}
};


inline _bstr_t vbprintf(_bstr_t fmt, va_list argptr)
{
	_bstr_t res;
	int cb=_vscwprintf(fmt,argptr);
	BSTR pbuf=SysAllocStringLen(NULL,cb+1);
	res.Attach(pbuf);
	vswprintf_s(pbuf,cb+1, fmt, argptr);
	return (wchar_t*)res; 
}
inline _bstr_t bprintf(_bstr_t fmt, ...)
{
    va_list argptr;
	va_start(argptr, fmt);
	return vbprintf(fmt,argptr); 
}

 inline void dbg_wprintf(_bstr_t fmt, ...)
 {
	 wchar_t buffer[4096];
	 va_list argptr;
	 va_start(argptr, fmt);
	 vswprintf_s(buffer,4096, fmt, argptr);
	 va_end(argptr);
	 OutputDebugStringW(buffer);
 } ;


 inline void dbg_printf(_bstr_t fmt, ...)
 {
	 char buffer[4096];
	 va_list argptr;
	 va_start(argptr, fmt);
	 vsprintf_s(buffer,4096, fmt, argptr);
	 va_end(argptr);
	 OutputDebugStringA(buffer);
 } ;


 template <class T>
 class detached 
 {
 public:
	 T& r;
	 detached(T& bb):r(bb){} ;
	 ~detached()
	 {  
		 r.Detach();
	 }
 };

//
 /*
 typedef struct _PAIR_NAME
 {
	 const wchar_t* name;
	 const DISPID id;
	 const size_t cb;
	 const int len;
 } PAIR_NAME;

#define _MLL_ int(256) 
#define BEGIN_STRUCT_NAMES(pn)  PAIR_NAME pn[]={
#define PAIR_SHIFT(b,s) {L#b,offsetof(s,b),sizeof(((s *)0)->b),_MLL_},
#define PAIR_SHIFT_2(b,s,off) {L#b,offsetof(s,b)+off,sizeof(((s *)0)->b),_MLL_},
#define DISP_PAIR(b,id)  {L#b,id,-1,_MLL_},
#define DISP_PAIR_IMPLC(b,id)  {L#b,id,-1,wcslen(L#b)},
#define END_STRUCT_NAMES {NULL,-1,-1,_MLL_} };
#define DISP_ONE(b) DISP_PAIR(b,b)

 inline const wchar_t* find_name_def(PAIR_NAME* pn,DISPID id,wchar_t* pdef=0)
 {
     while(pn->name)
	 {
           if(pn->id==id) return pn->name; 
		 ++pn;
	 }

     return pdef;    
 }
 inline HRESULT find_id(PAIR_NAME* pn,const wchar_t* name,DISPID* pid,DISPID log_add=0)
 {

	 if(!pid) return E_POINTER;
	 int ln0=(name)?wcslen(name):0;
	 for(int i=0;pn[i].name;i++)
	 {
    	 //
		 //int  l=max(ln0,pn->len);
         //
		 int  l=pn[i].len;
		 if(StrCmpNIW(pn[i].name,name,l)==0) return (*pid=(pn[i].id)|log_add),S_OK; 
    	 //if(StrCmpNIW(pn[i].name,name,l)==0) return (*pid=(pn[i].id)|log_add),S_OK; 
	 }

	 return DISP_E_UNKNOWNNAME;
 }
 // */

#define COM_INTERFACE_ENTRY_THIS() COM_INTERFACE_ENTRY_FUNC(IID_NULL,0,_This_Ptr<false>)
#define COM_INTERFACE_ENTRY_THIS_MEMBER(x) COM_INTERFACE_ENTRY_FUNC(IID_NULL,offsetof(_ComMapClass,x),_This_Ptr<false>)
#define COM_INTERFACE_ENTRY_THIS_MEMBER2(iid,x) COM_INTERFACE_ENTRY_FUNC(iid,offsetof(_ComMapClass,x),_This_Ptr<true>)
#define COM_INTERFACE_ENTRY_THIS_MEMBER_NOREF(iid,x) COM_INTERFACE_ENTRY_FUNC(iid,offsetof(_ComMapClass,x),_This_Ptr<false>)

 template <bool faddref>
 inline HRESULT WINAPI _This_Ptr(void* pv,REFIID iid,void** ppvObject, DWORD_PTR  dw)
 {
	 //ATLASSERT(iid==IID_NULL);
	     char* pc= ((char*)pv)+dw;
		 if(faddref) 
			((IUnknown*)pc)->AddRef();
		 (void*)(*ppvObject)=pc;
	 return S_OK;
 }


//#define COM_INTERFACE_ENTRY_THIS_MEMBER(x) COM_INTERFACE_ENTRY_FUNC(IID_NULL,offsetofclass(x,_ComMapClass),_MEMBER_Ptr<_ComMapClass>)

/*
 template <class _baseclass>
 inline HRESULT WINAPI _MEMBER_Ptr(void* pv,REFIID iid,void** ppvObject, DWORD dw)
 {
	 ATLASSERT(iid==IID_NULL);
	 char* pc= ((char*)pv)+dw;
	 (void*)(*ppvObject)=pc;
	 return S_OK;
 }
*/



 inline HRESULT WINAPI _Cache_if(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
 {
	 if(dw==DWORD_PTR(-1))  return E_NOINTERFACE;
	 else  return CComObjectRootBase::_Cache(pv,riid,ppv,dw);
	 
 };



#define COM_INTERFACE_ENTRY_AUTOAGGREGATE_BLIND_IF(punk, clsid,flag)\
 {NULL,\
 (flag)?((DWORD_PTR)&ATL::_CComCacheData<\
 ATL::CComAggregateCreator<_ComMapClass, &clsid>,\
 (DWORD_PTR)offsetof(_ComMapClass, punk)\
 >::data):DWORD_PTR(-1),\
 &_Cache_if},

#define COM_INTERFACE_ENTRY_AUTOAGGREGATE_BLIND_IF_IN_DLL(punk, clsid,flag)\
 {NULL,\
 (flag)?((DWORD_PTR)&ATL::_CComCacheData<\
 free_com_factory::_aggregate_creator_in_DLL_code<_ComMapClass, &clsid>,\
 (DWORD_PTR)offsetof(_ComMapClass, punk)\
 >::data):DWORD_PTR(-1),\
 &_Cache_if},




 ///*
 inline IDispatch* VARIANT_to_dispatch(VARIANT& V)
 {
	 if(SUCCEEDED(VariantChangeType(&V,&V,0,VT_DISPATCH)))
		 return  V.pdispVal;
	 else return 0;
 }
//*/

 inline IDispatch * VARIANT_to_dispatch(VARIANT& V,CComPtr<IDispatch>& disp )
 {
	 VARIANT VV={VT_EMPTY};
	 if(SUCCEEDED(VariantChangeType(&VV,&V,0,VT_DISPATCH)))
		 disp.Attach(VV.pdispVal);
	 else disp.Release();
	 return disp.p;
 }



#define BNS (1000000000)
#define  NS 1./double(BNS)

 class CStopwatch
 {
 public:
	 CStopwatch() {
		 QueryPerformanceFrequency(&m_liPerfFreq), Start(); 
	 }

inline	 void Start() {
	QueryPerformanceCounter(&m_liPerfStart); 
}

	 inline long double Sec()
	 {
		 LARGE_INTEGER liPerfNow;
		 QueryPerformanceCounter(&liPerfNow);
		 return (long double)( liPerfNow.QuadPart - m_liPerfStart.QuadPart ) / (long double)(m_liPerfFreq.QuadPart);
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
 private:
	 LARGE_INTEGER m_liPerfFreq;
	 LARGE_INTEGER m_liPerfStart;
 };

inline bstr_t unique_str()
{
	union{;
	GUID g;
	struct {
		DWORD d[4];
			};
	} u;
		CoCreateGuid(&u.g);
    return bprintf(L"%04x%04x%04x%04x",u.d[0],u.d[1],u.d[2],u.d[3]);

};

inline bstr_t new_tmp_file(bstr_t prfx=L"~za",HRESULT* phr=0)
{
	
	if(phr) *phr=0;
	wchar_t buf1[MAX_PATH],  buf2[MAX_PATH];
    if(GetTempPathW(MAX_PATH,buf1)&&GetTempFileNameW(buf1,prfx,0,buf2))
		return bstr_t(buf2);
	else
	{
	
		if(phr) *phr=HRESULT_FROM_WIN32(GetLastError());
		return bstr_t();
	}
}

inline wchar_t*  _wltrim(wchar_t* p)
{
	if(p)  while(iswspace( static_cast<unsigned short>(*p)))p++;  
	return p;
};
inline wchar_t*  _wrtrim(wchar_t* p)
{
	if(p)
	{
		wchar_t* pr=p+wcslen(p); 
		while((pr!=p)&&iswspace( static_cast<unsigned short>(*(--pr)))) *pr=0;  
	}
	return p;
};

inline wchar_t*  _wtrim(wchar_t* p)
{
	return _wrtrim(_wltrim(p));
};

//inline bool  _wlcheckni(wchar_t* p,wchar_t* ps,int cb=-1,wchar_t** pptail=NULL)
inline wchar_t*   _wlskip(wchar_t* p,wchar_t* ps,int cb=-1)
{
	if(p)
	{
		p=StrStrIW(p,ps);  
		if(!p) return 0;
		if(cb<0) {cb=wcslen(ps);}
		p+=cb;
	}
	return p;
}
inline bool  _wlcheckni(wchar_t* p,wchar_t* ps,int cb,wchar_t** pptail)
{
  if(!p) return 0;
  p=_wltrim(p);
  if(cb<0) {cb=wcslen(ps);}
  bool f=(p==StrStrNIW(p,ps,cb));
  if(f&&(pptail)) *pptail=p+cb;
  return f;
}

 inline bool lcheckni(char* p,char* ps,int cb,char** pptail=0)
{
	{
		if(!p) return 0;
		while(isspace(*p)) ++p;		
		if(cb<0) {cb=strlen(ps);}
		//bool f=(p==StrStrNIA(p,ps,cb)); strstrni
		bool f=StrCmpNIA(p,ps,cb)==0;
		if(f&&(pptail)) *pptail=p+cb;
		return f;
	}

}


inline _bstr_t  trim(wchar_t* p)
{
	//_bstr_t  buf;
	return _wtrim(_bstr_t(p));
};


inline wchar_t* _wrembk(wchar_t** pp,wchar_t l=L'{',wchar_t r=L'}')
{

	wchar_t* pr;
	wchar_t* p=*pp;

	p=_wtrim(p);
	if(!p) return 0;
	if(*p==l)
	{
		++p;
		if(pr=wcsrchr(p,r))
			*pr=0;
		else   return 0;


	}
	*pp=p;
	return p;
}


inline wchar_t* get_cmd_line1(wchar_t* pcmd=GetCommandLineW())
{
	bool f;
	int c;
	//cmd_split cmd=pcmd;
	wchar_t** ppcmd=CommandLineToArgvW(pcmd,&c);
	f=_wlcheckni(pcmd,L"\"",1,&pcmd);
	//wchar_t* p0=*ppcmd;
	int l=wcslen(*ppcmd);
	LocalFree(ppcmd);
	pcmd+=l;
	if(f) _wlcheckni(pcmd,L"\"",1,&pcmd);
	return _wltrim(pcmd);
}


inline long GetVariantElemsizeSize(VARTYPE vt)
{
typedef struct{PVOID pvRecord;IRecordInfo *pRecInfo;} __RI; 
	SAFEARRAY* psa;
	if(vt&VT_ARRAY) return sizeof(void*);
	if(vt==VT_RECORD) return sizeof( __RI); 

	SAFEARRAYBOUND sb={1,0};
	if(!(psa=SafeArrayCreate(vt,1,&sb) ) ) return 0;
	//long e=psa->cbElements;
	long e=SafeArrayGetElemsize(psa);
		SafeArrayDestroy(psa);
	return e;
}

struct LastErrorSaver
{
	DWORD err;
	LastErrorSaver():err(0){};
   ~LastErrorSaver(){SetLastError(err);};
inline DWORD set(DWORD e=GetLastError())
{
 return err=e;
 }
 operator HRESULT()
 {
	 return HRESULT_FROM_WIN32(err);
 }
};

//	VARTYPE vt=(V.vt)&(~VT_BYREF);
     
#define WILD_FLAGS_CAST(f,ff) if(f&(ff)) f|=(ff);
#define DISPATCH_GET_ALL (DISPATCH_METHOD|DISPATCH_PROPERTYGET)
#define DISPATCH_PUT_ALL (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)

template <class T>
HINSTANCE LoadLibraryX(_bstr_t libname,T id,bool fnoComReg=false)
{
	HMODULE hm=0;
  if(fnoComReg) hm=LoadLibraryW(libname);
	if(!hm)
 {
	COMINIT;
	CComPtr<IUnknown> unk;
	unk.CoCreateInstance(id);
	hm=LoadLibraryW(libname);
 }
	return hm;
}
inline HRESULT COM_check()
{
   HRESULT hr = CoInitialize(NULL);
   CoUninitialize();
   return hr;
}

#define _CLSID  L"clsid:"
#define _CLSID_LEN 6
#define _PROGID  L"progid:"
#define _PROGID_LEN 7
#define _CLIPBRD L"clipbrd:"
#define _CLIPBRD_LEN 8
#define _ENVIRON L"environ:"
#define _ENVIRON_LEN 8



template<class Intf > 
HRESULT parse_get_object(CLSID& clsid,Intf** ppunk,DWORD clsctx=CLSCTX_ALL)
{
	HRESULT hr;
	if(!ppunk) return E_POINTER;
    return 	hr=::CoCreateInstance(clsid,NULL,clsctx,__uuidof(Intf),(void**)ppunk);
};

template<class Intf > 
 HRESULT parse_get_object(BSTR name,Intf** ppunk,DWORD clsctx=CLSCTX_ALL)
{
	HRESULT hr;
	if(!ppunk) return E_POINTER;
	CLSID clsid;
	LPWSTR p;

       if(_wlcheckni(name,_ENVIRON,_ENVIRON_LEN,&p))
	   {
		   _bstr_t bn=bstr_alloc(1024);
		   if(GetEnvironmentVariableW(_wltrim(p),bn,1024))
		          return hr=::CoGetObject(bn,0,__uuidof(Intf),(void**)ppunk);
				  //parse_get_object(bn,ppunk,clsctx);
		   		   else   return HRESULT_FROM_WIN32(GetLastError());
	   }
	//inline bool  _wlcheckni(wchar_t* p,wchar_t* ps,int cb=-1,wchar_t** pptail=NULL)
	if(_wlcheckni(name,_CLSID,_CLSID_LEN,&p))
	{
		p=_wltrim(p);
		if(FAILED(hr=CLSIDFromString(p,&clsid)))   return hr;
		return hr=::CoCreateInstance(clsid,NULL,clsctx,__uuidof(Intf),(void**)ppunk);
	}
	if(_wlcheckni(name,_PROGID,_PROGID_LEN,&p))
	{
			p=_wltrim(p);
			if(FAILED(hr=CLSIDFromProgID(p,&clsid))) return hr;
        	return hr=::CoCreateInstance(clsid,NULL,clsctx,__uuidof(Intf),(void**)ppunk);
	}
	
	_bstr_t buf;	
			if(_wlcheckni(name,_CLIPBRD,_CLIPBRD_LEN))
			{
				name=buf=ClipboardText();
			}
			else name=buf=name;

			//hr=::CoGetObject(name,0,__uuidof(Intf),(void**)ppunk);
			hr=free_com_factory::CoGetObject(name,NULL,ppunk);   

			if(FAILED(hr)) // Fucking Vista!!! syphilis  preservation
			{
				int tid=free_com_factory::MTA_tid();
				hr=free_com_factory::CoGetObject(name,NULL,ppunk,tid);   
			}
				

    return hr;
}

inline bool msg_loop()
{
   MSG msg;
   //while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
   while(GetMessage(&msg,NULL,NULL,NULL))
   {
	   TranslateMessage(&msg);
	   DispatchMessage(&msg);
     if(msg.message==WM_QUIT) return false;
   }
   return true;
};

inline int dump_implement_interfaces(IUnknown* punk)
{
	//AllocConsole();
	if(!punk) return 0;
	CRegKey rk(HKEY_CLASSES_ROOT);
	HRESULT hr;
	if(FAILED(hr=rk.Open(HKEY_CLASSES_ROOT,TEXT("Interface")))) return 0;

	int i=0,count=0;
	//wchar_t buf[1024],buf2[1024];
	TCHAR buf[1024],buf2[1024];
	memset(buf,0,sizeof(buf));
	memset(buf2,0,sizeof(buf2));
	ULONG cbb=1024;
	while(S_OK==rk.EnumKey(i++,buf,&(cbb=1024)))
	{   
		CLSID   clsid;
		CComPtr<IUnknown> ptmp;
		_bstr_t bb(buf);
		if(FAILED( hr=CLSIDFromString(bb,&clsid))) continue ; 
		if(FAILED(hr=punk->QueryInterface(clsid,(void**)&ptmp))) continue; 
		CRegKey rl;
		//
		wprintf(L"%s",buf);


		if(rl.Open(rk,buf,KEY_READ)) 
		{
			printf("=???\n");
			continue;
		}
		ULONG cb=1024;
		 memset(buf2,0,sizeof(buf2));
		if(FAILED(hr=rl.QueryStringValue(CA2T(""),buf2,&cb))) continue; 
		wprintf(L" :%s\n",buf2);
		count++;
		memset(buf,0,sizeof(buf));
		
	}

	return count;
}

inline long refcount(IUnknown *punk)
{
	if(punk)
	try{
	punk->AddRef();
	return punk->Release();
	}catch(...){}
    return 0;
}


struct safeVARIANT:VARIANT
{
	safeVARIANT(){vt=VT_EMPTY;};
    ~safeVARIANT()
	{
		if((vt==VT_EMPTY)||(vt==VT_ERROR)) return ;
       try
		{
			VariantClear(this);
		}
		catch(...){};
	}
};

inline bstr_t ANSItoOEM(bstr_t b)
{
  bstr_t s=b.copy(true);
  char *pc=s;
  CharToOemA(pc,pc);
  return pc;
};
inline bstr_t OEMtoANSI(bstr_t b)
{
	bstr_t s=b.copy(true);
	char *pc=s;
	OemToCharA(pc,pc);
	return pc;
};

struct CProcessInfo:PROCESS_INFORMATION
{
	CProcessInfo()
	{
		PROCESS_INFORMATION* pp=this;
		*pp=PROCESS_INFORMATION();
	}
	~CProcessInfo()
	{
		CloseHandle(hProcess);CloseHandle(hThread);
	}
inline PROCESS_INFORMATION	Detach()
{
   PROCESS_INFORMATION tmp=*this;
    hProcess=hThread=0;
   return tmp;
}
};
inline HRESULT runasadmin(bstr_t ApplicationName,bstr_t CommandLine,PROCESS_INFORMATION* ppi=NULL,DWORD nShow = SW_SHOWNORMAL)
{

	SHELLEXECUTEINFOW Shex;
	ZeroMemory( &Shex, sizeof( SHELLEXECUTEINFOW ) );
	Shex.cbSize = sizeof( SHELLEXECUTEINFOW );
	Shex.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
	Shex.lpVerb = L"runas";
	Shex.lpFile = ApplicationName;
	Shex.lpParameters = CommandLine;
	Shex.nShow = nShow;

	HRESULT hr;
	if ( ! ShellExecuteExW( &Shex ) )
		return	hr =  HRESULT_FROM_WIN32(GetLastError());
	

	if (!  ppi )
	{
		WaitForSingleObject( Shex.hProcess, INFINITE );
			CloseHandle( Shex.hProcess );
			return S_OK;
	}
	 ppi->dwProcessId=GetProcessId(ppi->hProcess=Shex.hProcess);


	
	return  S_OK;
}

inline int startprocess(bstr_t lpCmdLine,DWORD waittime=INFINITE,PROCESS_INFORMATION* ppi=NULL,DWORD crflags=0,bool finher=0)
{
	LastErrorSaver lerr;
	STARTUPINFOW si={sizeof(si),0,0,0,0,0,0,0,0,0,0,STARTF_USESHOWWINDOW,SW_SHOW}; 
	CProcessInfo pi;
	if(!lpCmdLine.length()) return false;
	bool f=
		CreateProcessW(NULL,lpCmdLine,
		NULL,	// pointer to process security attributes 
		NULL,	// pointer to thread security attributes 
		finher,	// handle inheritance flag 
		crflags,	// creation flags 
		0,	// pointer to new environment block 
		0,	// pointer to current directory name 
		&si,	// pointer to STARTUPINFO 
		&pi);

	 lerr.set();

	if(f)
	{
		WaitForSingleObject(pi.hProcess,waittime);

		if(ppi)
			*ppi=pi.Detach();

				
		return pi.dwProcessId;
	}
	return 0;

};

inline
int flag_options_def(VARIANT& options,int flags=0)
{
	HRESULT hr;

	if(!is_optional(options))
	{

		VARIANT t={0};
		if(SUCCEEDED(hr=VariantChangeType(&t,&options,0,VT_I4))) 
			flags=t.intVal;
	}
	return flags;
}

inline
double double_options_def(VARIANT& options,double dfl=0)
{
	HRESULT hr;

	if(!is_optional(options))
	{

		VARIANT t={0};
		if(SUCCEEDED(hr=VariantChangeType(&t,&options,0,VT_R8))) 
			dfl=t.dblVal;
	}
	return dfl;
}

inline bstr_t ms_resource_gavno_resolver(char* s)
{
	bstr_t bs=s;
	wchar_t*  ps=bs;
	wchar_t*  p;
	if(p=StrStrIW(ps,L"//__MS_RESOURCE_POLNOE_GAVNO_NULL"))
	{
		//*p=0;
		wsprintfW(p,L"\n/*eof-res*/\n\0\0");
		return ps;
	}
	else
	if(p=StrStrIW(ps,L"<?__MS_RESOURCE_POLNOE_GAVNO_NULL"))
	{
		wsprintfW(p,L"\n<?eof-res?>\0\0");
		return ps;
		//*p=0;
		//return ps+bstr_t("<?eof?>");
	}
	return bs;
}


inline  void safe_wild_call(void *proc,void* param0=0,void* param1=0,void* param2=0,void* param3=0,void* param4=0,void* param5=0,void* param6=0,void* param7=0)
{

	if(proc)
	{
		__try
		{

#if defined(_WIN64)

typedef void (*x64__safe_wild_call_t)(void* param0,void* param1,void* param2,void* param3,void* param4,void* param5,void* param6,void* param7);

       ((x64__safe_wild_call_t)proc)(param0,param1,param2,param3,param4,param5,param6,param7);

#else


			DWORD ustack;

			__asm {
				mov ustack,ESP;
				push param7;
				push param6;
				push param5;
				push param4;
				push param3;
				push param2;
				mov  EDX,param1;
                push EDX;
				mov  ECX,param0;
				mov  EAX,ECX;
				push ECX;
				//mov  EAX,proc;
				//call EAX;
				call DWORD PTR proc;

				mov ESP,ustack;
			}; 
#endif

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}

	}
}

template <class M>
inline  void safe_wild_call(M method,void* param0=0,void* param1=0,void* param2=0,void* param3=0,void* param4=0,void* param5=0,void* param6=0,void* param7=0)
{
	union
	{ struct{ M m;};
	  struct{ void* p;};
	} U;
	U.m=method;
   return safe_wild_call(U.p,param0,param1,param2,param3,param4,param5,param6,param7);
}

inline CLSID CLSID_s(bstr_t sclsid)
{
	CLSID clsid=CLSID();
  CLSIDFromString(sclsid,&clsid);
  return clsid;
}

//#define CREATE_INSTANCE_hr( )



template <class Intf>
inline HRESULT _create_instance_in_dll(REFCLSID rclsid,Intf** pp,long tm=free_com_factory::tm_both,free_com_factory::_in_apartment_installer* pi=0)
{
  HRESULT hr;
  return hr=free_com_factory::CreateInstance(rclsid,pp,tm,HINSTANCE_IN_CODE,NULL,pi);
};


