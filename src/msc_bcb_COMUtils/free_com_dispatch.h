#pragma once

#include <windows.h>
#include "os_utils.h"
#include "handle_array.h"




struct apartment_holder:thread_base<apartment_holder,2>
{


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
			RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
		}
		__except (EXCEPTION_CONTINUE_EXECUTION){}
	};


	DWORD m_appt;   
	apartment_holder( long atype=COINIT_MULTITHREADED):m_appt(atype)
	{
		m_handles[1]=CreateEvent(0,true,false,0);
		run();

	}
	 
	 long stop()
	 {
          return SignalObjectAndWait(m_handles[1],m_handles[0],2000,0);
	 }
	~apartment_holder()
	{
		stop();
	}


	inline int  loop()
	{
		while(loop_event(m_handles[1]))
		{
			MSG msg;
			while(PeekMessage(&msg,0,0,0,PM_REMOVE))
			{
				if(msg.message==WM_QUIT) return 0;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}


		}
	}
	inline int  thread_proc()
	{
		
        bool fmta=(COINIT_APARTMENTTHREADED==m_appt); 

		SetThreadName(GetCurrentThreadId(),(fmta)?"DiCh_STA_Factory":"DiCh_MTA_Factory");
        CoInitializeEx(0,m_appt);
		loop();
		//  else while(WAIT_IO_COMPLETION==WaitForSingleObjectEx(m_handles[1],INFINITE,true));
		CoUninitialize();
		return 0;
	}

};


struct com_free_factory
{
 enum
 {
   tm_both=0,
   tm_free=1
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


  //
typedef	HRESULT (__stdcall  *PDllGetClassObjectType)(const GUID &,const GUID &, void** ppv);


#define T_FREE L"{4B2E958D-0393-11D1-B1AB-00AA00BA3258}"//Mts.MtsGrp
#define T_FREE L"{F4EB28FB-829E-4BB2-9EFE-5295D488B1A3}"
#define T_FREE L"{5D59FCB2-B438-45BB-B784-917B85E87429}"
inline static HRESULT ApartmentInit(IUnknown** ppunk=0,LPOLESTR sclsid=T_FREE)
{
	HRESULT hr;
	CLSID clsid;
	hr=CLSIDFromString(sclsid,&clsid);
	if(FAILED(hr)) return hr;
	IUnknown* punk=0;
	if(!ppunk) ppunk=&punk;
	hr=::CoGetClassObject(clsid,CLSCTX_ALL,0,__uuidof(IClassFactory),(void**)ppunk);
	if(punk) punk->Release();
	return hr;
}



typedef apartment_holder apartment_holder_type;

template<class Intf>
inline static HRESULT CoGetClassObject(const CLSID & rclsid,Intf** ppCF,long tmflag=tm_both,HMODULE hm=HINSTANCE_IN_CODE)
{

	struct _creator:thread_base<_creator,3>
	{
		
		stub_holder m_stub;
		HRESULT m_hr;
		PDllGetClassObjectType DllGetClassObject;
        CLSID rclsid;
        inline stub_holder& stub()
		{
             HRESULT(*this);
            return  m_stub;
		}
		inline operator HRESULT()
		{
			WaitForSingleObject(m_handles[1],INFINITE);
			return m_hr;
		}
		_creator()
		{
            m_handles[1]=CreateEvent(0,true,false,0);
			m_handles[2]=CreateEvent(0,true,false,0);
		}

       HRESULT create()
	   {
		   HRESULT hr;
           IUnknown* punk=0;
		   IClassFactory* pcf=0;
           hr=DllGetClassObject(rclsid,__uuidof(IUnknown), (void**)&punk);
		   if(FAILED(hr))    return hr;
		   /*
		   

             hr=pcf->CreateInstance(0,__uuidof(IUnknown),(void**)&punk);
			 pcf->Release();
			 if(FAILED(hr))	 return hr;
			 */

           m_stub=stub_holder(punk,MSHCTX_INPROC,MSHLFLAGS_NORMAL); 
			punk->Release();
			return m_stub;
	   }
	  	

		inline int  thread_proc()
		{
			COMINIT_F;
			m_hr=create();
			SignalObjectAndWait(m_handles[1],m_handles[2],INFINITE,0);
			return 0;
		};
        inline void stop()
		{
			SetEvent(m_handles[2]);
		};
		~_creator()
		{
           stop();
		};

	} creator;



	HRESULT hr;
	//PDllGetClassObjectType DllGetClassObject;
	if(!ppCF) return E_POINTER;
    creator.DllGetClassObject=(PDllGetClassObjectType)GetProcAddress(hm,"DllGetClassObject");
	if(!creator.DllGetClassObject) return HRESULT_FROM_WIN32(GetLastError());
	creator.rclsid=rclsid;

    DWORD aptype;
		if(FAILED(hr=apartment_type(&aptype))) 
			return hr;
   bool f=(tmflag==tm_free)&&(aptype==COINIT_APARTMENTTHREADED);
    //ApartmentInit();
   //   ApartmentInit(0,L"{ecabafbf-7f19-11d2-978e-0000f8757e2a}");
   creator.run();
   //creator.run(!f);
   
   if(FAILED(hr=creator)) return hr;
   creator.stop();
   
    hr=creator.stub().unwrap(ppCF);   
	//creator.join(INFINITE);
   //creator.join(INFINITE);
   return hr;
	 //GUID  rclsid=__uuidof(Intf)
  //return E_POINTER;
}



};
