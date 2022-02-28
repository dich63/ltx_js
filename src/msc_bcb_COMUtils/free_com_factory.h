#pragma once

#include <windows.h>
//#include "os_utils.h"
#include "handle_array.h"
#include <utility>




struct free_com_factory
{
 enum
 {
   tm_both=0,
   tm_free=1,
   tm_apart=2,
   tm_current=3
 };

 
 struct initer_finiter_DLL
 {
	 void* m_p;

	 initer_finiter_DLL():m_p(0){}
	 ~initer_finiter_DLL()
	 {  
		 reset();
	 }

	 inline void reset(void* p=0)
	 {
		 if(p) free_com_factory::DLL_AddRef(p);
		 if(m_p) free_com_factory::DLL_Release(m_p);
		 m_p=p;
	 }

	 inline void   operator =(void * p)
	 {
		 reset(p);
	 }

 };

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
     	 if(SUCCEEDED(m_hr)) CoUninitialize();
	 }
	 inline operator HRESULT() { return m_hr;}
 };

 template<DWORD AT>
 struct COMInitT:COMInit
 {
	 COMInitT(void*p=0):COMInit(AT){};
 };

inline static HRESULT apartment_type(DWORD* ptype)
	{
		if(NULL==ptype) return E_POINTER;
         COMInit com(COINIT_APARTMENTTHREADED);
        if(com==S_OK) return CO_E_NOTINITIALIZED;
		else if(com==RPC_E_CHANGED_MODE) *ptype=COINIT_MULTITHREADED;
		else if(com==S_FALSE) *ptype=COINIT_APARTMENTTHREADED;
		else return E_UNEXPECTED;
		return S_OK;
	};


inline static bool is_MTA()
{
       DWORD at;
	   HRESULT hr;
   if(SUCCEEDED(hr=apartment_type(&at)))
   {
      return at==COINIT_MULTITHREADED;  
   }
    SetLastError(hr);
    return false;
}

struct iapartment
{
	virtual	long stop()=0;
};

template< int ThreadingModel> 
static int GlobalApartmentThreadId(void (cdecl * proc)(void*)=0,void* p=0,iapartment** ppap=0)
{

	struct s_tm
	{
		apartment_holder<ThreadingModel> holder;
         void* m_p;  
		 
		 s_tm(void (cdecl * proc)(void*),void* p):holder(proc,p)
		{
          //holder.m_proc=proc;
          //holder.m_param=p;
		};
		


	};
	initializator_singleton::locker lock;

	{
		static s_tm tm(proc,p);
		if(ppap)
			*ppap=static_cast<iapartment*>(&tm.holder);

		return tm.holder.id();
	}

};


inline static int MTA_tid(void (cdecl * proc)(void*)=0,void* p=0 ,iapartment** ppap=0)
{
    return GlobalApartmentThreadId<COINIT_MULTITHREADED>(proc,p,ppap);
}



typedef	HRESULT (__stdcall  *PDllGetClassObjectType)(const GUID &,const GUID &, void** ppv);

#define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT  (0x00000002)
#define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS        (0x00000004)


inline static HMODULE  __getHinstance(void* p,bool faddref=false)
{
	HMODULE h=0;
	DWORD flags=(faddref)?GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
	:GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;

	GetModuleHandleExW(flags,(LPCWSTR)p,&h);
	return h;
};



inline static HRESULT DLL_AddRef(void* p)
{
         HRESULT hr=S_OK;
         if(!__getHinstance(p,true))
		   hr=HRESULT_FROM_WIN32(GetLastError());
		 return hr;
}

inline static HRESULT DLL_Release(void* p)
{
	HRESULT hr;
	HMODULE h;
	if(!(h=__getHinstance(p,false)))
		return hr=HRESULT_FROM_WIN32(GetLastError());
    if(!FreeLibrary(h))
		return hr=HRESULT_FROM_WIN32(GetLastError());
	return S_OK;
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

#define ADDRESS_NEXT free_com_factory::s_address_next<void*>()
#define HINSTANCE_IN_CODE ( free_com_factory::__getHinstance(ADDRESS_NEXT))


struct _in_apartment_installer
{
	virtual HRESULT __stdcall init(IUnknown** ppunk)=0;
};

template <class T>
struct in_apartment_installer_T:_in_apartment_installer
{
	void* p;
	in_apartment_installer_T(void* pcontext=0):p(pcontext){};
	virtual HRESULT __stdcall init(IUnknown** ppunk)
	{
		return T::init(*ppunk,p);
	};
};

struct _creator
{
	typedef	HRESULT (__stdcall  *PDllGetClassObject)(const CLSID &,const IID &, void** ppv);
	DWORD m_cookie;
	HRESULT m_hr;
	PDllGetClassObject m_DllGetClassObject;
	IGlobalInterfaceTable *pGIT;
	_in_apartment_installer* m_pinstaller;


	_creator(HMODULE hm=HINSTANCE_IN_CODE,bool fdirect=true,_in_apartment_installer* pinstaller=0)
		:pGIT(0),m_hr(E_POINTER),m_cookie(0),m_DllGetClassObject(0),m_pinstaller(pinstaller)
	{

		if(hm!=HMODULE(-1))
		{
           void* p=::GetProcAddress(hm,"DllGetClassObject");
		   if(!p)
		   {
			   m_hr=HRESULT_FROM_WIN32(GetLastError());
			   return;
		   }

		    m_DllGetClassObject=(PDllGetClassObject)p;

		}
		
			if(fdirect==false)
			m_hr=::CoCreateInstance(CLSID_StdGlobalInterfaceTable,NULL,CLSCTX_INPROC_SERVER
				,__uuidof(IGlobalInterfaceTable), (void**)&pGIT);
			else m_hr=S_OK;
		
		//Sleep(0);
	};


	operator HRESULT()
	{
		return m_hr;
	}

template<class Intf>
	HRESULT unwrap(Intf **ppObj)
	{
		HRESULT hr;
		return hr=unwrap(__uuidof(Intf),(void**)ppObj);
	}

inline	HRESULT unwrap(const IID& iid,void** ppObj)
	{
		//HRESULT hr;
		if(FAILED(m_hr)) return m_hr;

		m_hr=pGIT->GetInterfaceFromGlobal(m_cookie,iid,ppObj);

		return m_hr;

	}


//
inline HRESULT  RegisterCLSID(const wchar_t* sclsid,DWORD* pcookie=0)
{  
   CLSID clsid;
  if(SUCCEEDED(m_hr=CLSIDFromString(LPOLESTR(sclsid),&clsid)))
	  m_hr=RegisterCLSID(clsid,pcookie);
   return m_hr;
}

inline HRESULT  RegisterCLSID(REFCLSID rclsid,DWORD* pcookie=0)
{
   if(FAILED(m_hr)) return m_hr;	
     IUnknown *punk=0;
	 DWORD dw;
  if(FAILED(m_hr= GetClassObject(rclsid,&punk))) return m_hr;
  m_hr= CoRegisterClassObject(rclsid,punk,CLSCTX_INPROC_SERVER,REGCLS_MULTIPLEUSE,&dw);
  punk->Release();
  if(pcookie) *pcookie=dw;
  return m_hr;
}


template <class Intf>
  HRESULT GetClassObject(REFCLSID rclsid,Intf** ppObj)
  {
    return GetClassObject(rclsid, __uuidof(Intf),(void**)ppObj);
  }

inline  HRESULT GetClassObject(REFCLSID rclsid,REFIID riid,void** ppObj)
  {
	  if(!ppObj) return m_hr=E_POINTER;
	  if(FAILED(m_hr)) return m_hr;
	  m_hr=m_DllGetClassObject(rclsid,__uuidof(IClassFactory),ppObj);
      return m_hr;
  }

inline  HRESULT CreateInstance(REFCLSID rclsid,IUnknown*  pUnkOuter,REFIID riid,void** ppObj)
  {
	  if(!ppObj) return m_hr=E_POINTER;
	  if(FAILED(m_hr)) return m_hr;
	  IClassFactory* pcf=0;
	  //void* pp=m_DllGetClassObject;
	  //m_hr=m_DllGetClassObject(rclsid,__uuidof(IClassFactory), (void**)&pcf);
	  if(FAILED(GetClassObject(rclsid,&pcf))) return m_hr;
	  m_hr=pcf->CreateInstance(pUnkOuter,riid,ppObj);
	  pcf->Release();
	  if((m_pinstaller)&&(SUCCEEDED(m_hr)))
		  m_hr=m_pinstaller->init((IUnknown**)ppObj);
	  return m_hr;
  };

inline	HRESULT operator()(CLSID rclsid)
	{
		if(!pGIT) return m_hr=E_UNEXPECTED;
		IUnknown* punk=0;
		
		if(FAILED(CreateInstance(rclsid,NULL,__uuidof(IUnknown),(void**)&punk))) 
			return m_hr;

		m_hr=pGIT->RegisterInterfaceInGlobal(punk,__uuidof(IUnknown),&m_cookie);

		punk->Release();

		return m_hr;   
	}

template< class Creator> 
inline	HRESULT operator()(Creator& c)
{
	if(!pGIT) return m_hr=E_UNEXPECTED;
	IUnknown* punk=0;

	if(FAILED(m_hr=c.CreateInstance(&punk))) 
		return m_hr;

	m_hr=pGIT->RegisterInterfaceInGlobal(punk,__uuidof(IUnknown),&m_cookie);

	punk->Release();

	return m_hr;   
}

template< class ClassId> 
	HRESULT operator()(std::pair<ClassId,long*>& p)
	{

   		return m_hr=RegisterCLSID(p.first,(DWORD*)p.second);
	};


	HRESULT operator()(DWORD dwcookie)
	{
		return m_hr=CoRevokeClassObject(dwcookie);
	};

	~_creator()
	{
		if(m_cookie)
			pGIT->RevokeInterfaceFromGlobal(m_cookie);
	}
	
};



template <class T, const CLSID* pclsid>
class _aggregate_creator_in_DLL_code
{
public:
	static HRESULT WINAPI CreateInstance(void* pv, REFIID/*riid*/, LPVOID* ppv) throw()
	{
		HRESULT hr;
		if (pv == NULL)
			return E_INVALIDARG;

		T* p =static_cast<T*>(pv);
		// Add the following line to your object if you get a message about
		// GetControllingUnknown() being undefined
		// DECLARE_GET_CONTROLLING_UNKNOWN()
		return hr=_creator(HINSTANCE_IN_CODE,true).CreateInstance(*pclsid,p->GetControllingUnknown(), __uuidof(IUnknown), ppv);
			//CreateInstance(*pclsid, p->GetControllingUnknown(), CLSCTX_INPROC, __uuidof(IUnknown), ppv);
	}
};




struct registrator_once_functor
{
   long m_tls_index;
    HMODULE m_hlib;
    DWORD dwcookie;
	registrator_once_functor(long tls_index=-1,HMODULE hlib=HINSTANCE_IN_CODE)
		:m_hlib(hlib),m_tls_index(tls_index){};

inline HRESULT operator()(CLSID clsid)
{
	HRESULT hr;
	
	 if(m_tls_index>=0)
	 {
		 dwcookie=(DWORD)TlsGetValue(m_tls_index);
		 if(dwcookie) return S_FALSE;
		 hr=GetLastError();
		 if(hr!=ERROR_SUCCESS)
			 return hr=HRESULT_FROM_WIN32(hr);
		 
	 }
     IUnknown* punk=0;
     hr=_creator(m_hlib,true).GetClassObject(clsid,&punk);
	 if(FAILED(hr))  return hr;
     hr=CoRegisterClassObject(clsid,punk,CLSCTX_INPROC_SERVER,REGCLS_MULTIPLEUSE,&dwcookie);
     punk->Release(); 
     if(SUCCEEDED(hr)) 
		     TlsSetValue(m_tls_index,(void*)dwcookie);
		 return hr;

}

};

template< class ClassId> 
inline static HRESULT RegisterCLSID(HMODULE hm,ClassId clsid,long* pcookie,int tid=GetCurrentThreadId())
{
	HRESULT hr;

     _creator creator(hm,0);

	 if(FAILED(hr=creator)) return hr;

	if( tid==GetCurrentThreadId())
	  return hr=creator.RegisterCLSID(clsid,(DWORD*)pcookie);
	 std::pair<ClassId,long*> arg=std::make_pair(clsid,(long*)pcookie);

    return hr=call_in_thread_context(creator,arg,tid,E_UNEXPECTED);
	
}



template <class Intf>
inline static HRESULT CoGetObject(const wchar_t* parseStr,BIND_OPTS* pbo,Intf** ppintf,int tid=GetCurrentThreadId())
{
	HRESULT hr;

	struct safe_BSTR
	{
		BSTR pstr;
		~safe_BSTR(){::SysFreeString(pstr);}
		inline operator BSTR&(){return pstr;}

	} sparseStr={::SysAllocString(parseStr)};

	if( tid==GetCurrentThreadId())
	{
		return hr=::CoGetObject(sparseStr,pbo,__uuidof(Intf),(void**)ppintf);
	}

    struct _mon_creator
	{
		BSTR pstr;
		BIND_OPTS *pbo;
     inline	HRESULT CreateInstance(IUnknown** ppunk)
		{
			HRESULT hr;
            return hr=::CoGetObject(pstr,pbo,__uuidof(IUnknown),(void**)ppunk);
		}


	} monic={sparseStr,pbo};

    _creator creator(HMODULE(-1),false);

    hr=call_in_thread_context(creator,monic,tid,E_UNEXPECTED);
	if(SUCCEEDED(hr))
	{
		hr=creator.unwrap(ppintf);
	}

	return hr;
};





template<class Intf>
inline static HRESULT CreateInstance(REFCLSID rclsid,Intf** ppObj,long tmflag=tm_both,HMODULE hm=HINSTANCE_IN_CODE,IUnknown* punkOuter=0,_in_apartment_installer* pinstaller=0)
{
   return CreateInstance(rclsid,__uuidof(Intf),(void**)ppObj,tmflag,hm,punkOuter,pinstaller);
}

inline static HRESULT CreateInstance(REFCLSID rclsid,REFIID riid,void** ppObj,long tmflag=tm_both,HMODULE hm=HINSTANCE_IN_CODE,IUnknown* punkOuter=0,_in_apartment_installer* pinstaller=0)
{

	    
     HRESULT hr;
     DWORD aptype;
    if(FAILED(hr=apartment_type(&aptype)))
		  return hr;

	
   int MTA_id=GlobalApartmentThreadId<COINIT_MULTITHREADED>();
   int tid=MTA_id;
   
   bool fdirect=(tmflag==tm_both)&&(aptype!=COINIT_MULTITHREADED);

    fdirect|=(tmflag==tm_current);

   if((!fdirect)&&( punkOuter))
	   return hr=CLASS_E_NOAGGREGATION;

   if((tmflag==tm_apart)) tid=GlobalApartmentThreadId<COINIT_APARTMENTTHREADED>();

   
   _creator creator(hm,fdirect,pinstaller);

   if(fdirect)
   	   return hr=creator.CreateInstance(rclsid,punkOuter,riid,ppObj);
   


    if(FAILED(hr=creator)) return hr;


    hr=call_in_thread_context(creator,rclsid,tid,E_UNEXPECTED);

    if(FAILED(hr)) return hr;

	return hr=creator.unwrap(riid,ppObj);

}
     
 
protected:

  template< DWORD ThreadingModel,class IniterFiniter=COMInitT<ThreadingModel> >
	struct apartment_holder:iapartment,thread_base<apartment_holder<ThreadingModel>,3,IniterFiniter >
	{

        void (cdecl * m_proc)(void* p);
		void * m_param;

		inline static void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName)
		{

			struct{
				DWORD dwType; // must be 0x1000
				LPCSTR szName; // pointer to name (in user addr space)
				DWORD dwThreadID; // thread ID (-1=caller thread)
				DWORD dwFlags; // reserved for future use, must be zero
			} 	info={0x1000,szThreadName,dwThreadID,0};



			__try
			{
				RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (ULONG_PTR*)&info );
			}
			__except (EXCEPTION_CONTINUE_EXECUTION){}
		};



		apartment_holder(void (cdecl * _proc)(void*)=0,void* _p=0)
		{
			m_proc=_proc;
			m_param=_p;
			m_handles[1]=CreateEvent(0,true,false,0);
			m_handles[2]=CreateEvent(0,true,false,0);
			run();
			m_handles.wait();

		}

	virtual	long stop()
		{
			DWORD ws;
			return ws=SignalObjectAndWait(m_handles[1],m_handles[0],5000,0);
		}
		~apartment_holder()
		{
			stop();
		}


		inline int  loop()
		{
			DWORD ws;
			try
			{

				while(ws=loop_event(m_handles[1]))
				{
					MSG msg;
					//if(ws<0)
					while(PeekMessage(&msg,0,0,0,PM_REMOVE))
					{
						if(msg.message==WM_QUIT) return 0;
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}


				}
			}
			catch (...){}

			return 0;
		}
		inline int  thread_proc()
		{

			const bool fmta=(COINIT_APARTMENTTHREADED==ThreadingModel); 

			SetThreadName(GetCurrentThreadId(),(fmta)?"DiCh_STA_Factory":"DiCh_MTA_Factory");
			//CoInitializeEx(0,ThreadingModel);
			//COMInit com(ThreadingModel);
			SetEvent(m_handles[2]);
			if(1||fmta)
			loop();
			else while(WAIT_IO_COMPLETION==WaitForSingleObjectEx(m_handles[1],INFINITE,1)){};
			if(m_proc) m_proc(m_param);
			
			//CoUninitialize();
			return 0;
		}

	};


 free_com_factory(){};
~free_com_factory(){};
};
