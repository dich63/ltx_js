#pragma once

#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>


#include "ltx_call_helper.h"
//

//#include "static_com.h"


#pragma comment(lib ,"Ws2_32.lib")





namespace ipc_utils {




    template<int MAX=256>
    struct bstr_c_t
	{
        int lenb;
		wchar_t buf[MAX];
		bstr_c_t(const wchar_t* str){             
			(lenb=(str)? sizeof(wchar_t)*wcslen(str) : 0 )&& memcpy(buf,str,(lenb+sizeof(wchar_t)));  	  
		}
		inline operator BSTR()
		{
			return (lenb)?BSTR(buf):BSTR(0);
		}

	};

	template <class T, class T2>
	inline T make_detach(T& dst, T2 n)
	{  
		T t=dst;
		dst=(T)n;
		return t;
	};

	template <class T>
	inline T make_detach(T& dst)
	{  
		T t=dst;
		dst=T(); 
		return t;
	};



	inline long ref_count(IUnknown* p)
	{
		if(p)
		{
			p->AddRef();
			return p->Release();
		}
		else return 0;
	}
	




	struct clsid_t
	{
		HRESULT hr;
		CLSID clsid;
		clsid_t():clsid(GUID_NULL),hr(0){};
		clsid_t(CLSID _clsid):clsid(_clsid),hr(0){};

		clsid_t(bstr_t str){
			if(FAILED(hr=CLSIDFromString(str,&clsid)))
				clsid=GUID_NULL;
		};

		clsid_t(const wchar_t* str){

			if(FAILED(hr=CLSIDFromString(bstr_t(str),&clsid)))
				clsid=GUID_NULL;
		};

		inline operator bstr_t()
		{
			
			wchar_t buf[48];
			StringFromGUID2(clsid,buf,48);
			return buf;
		}
		  inline  bstr_t str_copy_n(int n=1,wchar_t* p=0)
		  { 
			    BSTR bstr=  SysAllocStringLen(p,n);
				memset(bstr,0,n*sizeof(wchar_t));
                return bstr_t(bstr,false);
		  }
          inline  bstr_t parse_string(bstr_t addstr=bstr_t(),int n=2)
		  {
			  bstr_t b= bstr_t(*this)+L":"+ addstr+ str_copy_n(n);
			  return b;
		  }

		inline operator CLSID&()
		{
			return clsid;
		}
		inline bstr_t prog_id()
		{
			bstr_t b;
            BSTR pbst=0;
			if(SUCCEEDED(hr=ProgIDFromCLSID(clsid,&pbst))){
				 b=pbst;
				 CoTaskMemFree(pbst);
			}
					


				return  b;
		}

	};




	template <class N>
	struct   smart_ptr_t
	{

		N* p;
		smart_ptr_t():p(0){};

		smart_ptr_t(N* _p,bool faddref=1):p(_p)
		{
			if((p)&&faddref) p->AddRef(); 
		}
		~smart_ptr_t()
		{
			if(p) p->Release();
		}
		
		smart_ptr_t(const smart_ptr_t& sp):p(0)
		{
			reset(sp.p);
		}
        
		smart_ptr_t& operator =(const smart_ptr_t& sp)
		{
			return reset(sp.p);
		}

		
		template<class DerivedN>
		smart_ptr_t& operator =( smart_ptr_t<DerivedN> sp)
		{
			return reset(sp.p);
		}


		inline N* detach()
		{
			return ipc_utils::make_detach(p,(N*)0);
		}
		inline smart_ptr_t& reset(N* np=0,bool faddref=true)
		{
			if(np!=p)
			{
				N* t=ipc_utils::make_detach(p,np);
				if(p&&faddref) p->AddRef();
				if(t)
					t->Release();
			}
			return *this;
		}

		inline void Release() { reset();};

		inline  N* operator->()
		{
			return p;
		}
		inline   operator N*()
		{
			return p;
		}

		N** address()
		{
			reset();
			return &p;
		}
		N** _address()
		{
			
			return &p;
		}
		void** ppQI()
		{
			reset();
			return (void**)&p;
		}

		void** _ppQI()
		{			
			return (void**)&p;
		}

		long refcount()
		{
			return ref_count(p);
		}

		inline HRESULT CopyTo(N** pp)
		{
			if(!pp) return E_POINTER;
			*pp=p;
			if(p) p->AddRef();
			return S_OK;
		};

        inline VARIANT toVARIANT()
		{
			VARIANT t={(__uuidof(N)==__uuidof(IDispatch))?VT_DISPATCH:VT_UNKNOWN};
			t.byref=p;
			return t;
		}

		template<class Intf>
		HRESULT QueryInterface(Intf** ppvObject)
		{
			HRESULT hr;
			if(p) return hr=p->QueryInterface(__uuidof(Intf),(void**)ppvObject);
			else return hr=E_POINTER;
		}

		
		HRESULT QueryInterface(REFIID riid,void** ppvObject)
		{
			HRESULT hr;
			if(p) return hr=p->QueryInterface(riid,ppvObject);
			else return hr=E_POINTER;
		}


		HRESULT CoGetObject( bstr_t parseStr,BIND_OPTS* pbo=0)
		{
              HRESULT hr;
			  
			  N* t=0;
			  if(SUCCEEDED(hr=::CoGetObject(parseStr,pbo, __uuidof(N), (void**)&t)))
				  reset(t,false);
			  return hr;
		}

		HRESULT CoCreateInstance( wchar_t* bclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) 
		{
			
			HRESULT hr;
			CLSID rclsid;
			if(SUCCEEDED(hr=CLSIDFromString(bclsid,&rclsid)))
				hr=CoCreateInstance(rclsid, pUnkOuter, dwClsContext);
			return hr;
		}

		HRESULT CoCreateInstance( REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) 
		{
			HRESULT hr;
			
			N* t=0;
			if(SUCCEEDED(hr=::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(N), (void**)&t)))
				reset(t,false);
			return hr;
		}

	};


	struct COMInit_t
	{
	
		HRESULT hr;
		COMInit_t():hr(CO_E_NOTINITIALIZED){};
		COMInit_t(DWORD mode):hr(CO_E_NOTINITIALIZED){
			init(mode);  
		};

		void init(DWORD mode= COINIT_MULTITHREADED /*COINIT_APARTMENTTHREADED*/ )
		{
			if(hr==CO_E_NOTINITIALIZED) 
				hr=CoInitializeEx(NULL,mode); 
		}
		~COMInit_t()
		{
			//	   Beep(500,500);
			if(SUCCEEDED(hr)) CoUninitialize();
		}
		inline operator HRESULT() { return hr;}

		inline static DWORD apartment_type()
		{
			HRESULT hr;
             COMInit_t ci(COINIT_MULTITHREADED);
			 if((hr=ci.hr))
			 {
				 if(hr==RPC_E_CHANGED_MODE) return COINIT_APARTMENTTHREADED;
				 else if(hr==S_FALSE) return COINIT_MULTITHREADED;
			 }		 
            return -1;
		}
		inline static bool is_MTA() { return  apartment_type()==COINIT_MULTITHREADED;  }
	};

	struct COMInitA_t:COMInit_t{  COMInitA_t():COMInit_t(COINIT_APARTMENTTHREADED){} ;  };
	struct COMInitF_t:COMInit_t{  COMInitF_t():COMInit_t(COINIT_MULTITHREADED){};   };



	inline IUnknown* VARIANT_to_IUnknown(const VARIANT& v)
	{
		VARTYPE t=v.vt;
		return ((t==VT_DISPATCH)||(t==VT_UNKNOWN))?v.punkVal:(IUnknown*)0;

	}

	template<class I>
	inline HRESULT toVARIANT(I* pintf,VARIANT* pv,bool frelease=false)
	{
		if(!pintf)return E_POINTER;
		VARIANT v={VT_UNKNOWN};
		HRESULT hr_di,hr;
		if(pv)	hr=pintf->QueryInterface(__uuidof(IUnknown),&v.byref);

		if(frelease) pintf->Release();

		if(!pv)return E_POINTER;
		if(SUCCEEDED(hr))
		{
			hr_di=VariantChangeType(&v,&v,0,VT_DISPATCH);
			*pv=v;
		}
		return hr;
	}

	inline VARIANT* clearV(VARIANT* pv)
	{
		if(pv) VariantClear(pv);
		return pv;            
	}

/*
	struct GIT_t
	{
		HRESULT hr;
		DWORD tid,MTA_tid;
		smart_ptr_t<IGlobalInterfaceTable> GIT;
		static DWORD __stdcall s_tp(void* he)
		{
           COMInit_t ci(COINIT_MULTITHREADED);
		   SetEvent(HANDLE(he));
		   SleepEx(-1,1);
		   return 0;
		}

		void initMTA()
		{
		
			HANDLE hevent=CreateEvent(0,1,0,0);
			HANDLE hthread=CreateThread(0,0,&s_tp,hevent,0,&MTA_tid);
			//if(hthread) WaitForSingleObject(hevent,INFINITE);
			CloseHandle(hthread);
			CloseHandle(hevent);
		}
		GIT_t():tid(GetCurrentThreadId())
		{
			
              //			initMTA();
			COMInit_t ci(COINIT_MULTITHREADED);
			hr=GIT.CoCreateInstance(CLSID_StdGlobalInterfaceTable);	  

		}
		inline operator IGlobalInterfaceTable*(){ return GIT.p;}
		inline IGlobalInterfaceTable* operator  ->(){ return GIT.p;}

		template<class Intf>
		inline     HRESULT wrap(Intf* p,DWORD* pdw,bool frelease=false)
		{
			HRESULT hr;
			if(!p) return E_POINTER;
			hr=GIT->RegisterInterfaceInGlobal(p,__uuidof(Intf),pdw);
			if(frelease&&SUCCEEDED(hr)) p->Release();

			return hr;

		};    

		template<class Intf>
		inline   HRESULT unwrap(DWORD dw,Intf** pp,bool frevoke=true)
		{
			 return unwrap(dw,__uuidof(Intf),(void**)pp,frevoke);
		}

		template<class Intf>
		inline   HRESULT unwrap(DWORD dw,smart_ptr_t<Intf>& i,bool frevoke=true)
		{
			return unwrap(dw,__uuidof(Intf),i.ppQI(),frevoke);
		}

        inline   HRESULT unwrap(DWORD dw,REFIID riid, void** pp,bool frevoke=true)
		{
			HRESULT hr,hr0;
			hr=(pp)?GIT->GetInterfaceFromGlobal(dw,riid,(void**)pp):S_FALSE;

			if(frevoke&&SUCCEEDED(hr))
				hr0=GIT->RevokeInterfaceFromGlobal(dw);
			return hr;
		}

         inline   HRESULT unwrap(DWORD dw,VARIANT* pv,bool frevoke=true)
		 {
			 HRESULT hr;
			 if(!pv) return E_POINTER;
			 smart_ptr_t<IUnknown> unk;
			 if(SUCCEEDED(hr=unwrap(dw,&unk.p,frevoke)))
			 {
				 hr=toVARIANT(unk.p,pv);
			 }
			 return hr;			 

		 }

		inline   HRESULT unwrap(DWORD dw)
		{
			COMInit_t ci(COINIT_MULTITHREADED);
			HRESULT hr0;
			return hr0=GIT->RevokeInterfaceFromGlobal(dw);
		}

		inline static GIT_t& get_instance()
		{
			return class_initializer_T<GIT_t>().get();
		}

       


	};


	template <class N>
	struct smart_GIT_ptr_t
	{
		smart_GIT_ptr_t(N* p=0):dwcookie(0)
		 {
           reset(p);

		 }

		inline smart_GIT_ptr_t& reset(N* p=0)
		{
			DWORD t=make_detach(dwcookie);
			if(p)
				GIT_t::get_instance().wrap(p,&dwcookie);
			if(t) 
				GIT_t::get_instance().unwrap(t);

			return *this;
		}
		inline DWORD detach(DWORD dw=0)
		{
			return make_detach(dwcookie,dw);
		}

		~smart_GIT_ptr_t()
		{
		         reset();
		}


		 DWORD dwcookie;
		 template<class I>
		 HRESULT unwrap(smart_ptr_t<I>& sp )
		 {
           HRESULT hr;
		   hr=GIT_t::get_instance().unwrap(dwcookie,sp.address(),false);
		   return hr;
		 }

		 template<class I>
		 HRESULT unwrap(I**pp )
		 {
			 HRESULT hr;
			 hr=GIT_t::get_instance().unwrap(dwcookie,pp,false);
			 return hr;
		 }

		 
		 HRESULT unwrap(REFIID riid,  void**pp )
		 {
			 HRESULT hr;
			 hr=GIT_t::get_instance().unwrap(dwcookie,riid,pp,false);
			 return hr;
		 }

		 HRESULT unwrap(VARIANT* pv )
		 {
			 HRESULT hr;
			 hr=GIT_t::get_instance().unwrap(dwcookie,pv,false);
			 return hr;
		 }
		 inline operator bool(){
			 return (dwcookie);
		 }

	};

*/

  static 	bool loop(HWND hwnd=0)
	{
		MSG msg;
		 while(PeekMessage(&msg,hwnd,0,0,PM_REMOVE))
		{
			if((msg.message==WM_QUIT)) return true;
			
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			
		}
		return false;
	} ;

	static 	bool wait_alertable(HANDLE* ph,int nh=1)
	{
		DWORD ws;
		ws=MsgWaitForMultipleObjectsEx(1,ph,INFINITE,QS_ALLINPUT,MWMO_INPUTAVAILABLE|MWMO_ALERTABLE);
		return ((ws==WAIT_OBJECT_0)||(ws==WAIT_ABANDONED));
	};




	class stopwatch_t
	{
	public:
		stopwatch_t() {
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


/*
	struct WSA_initerfiniter
	{
		HRESULT err;
		LPFN_CONNECTEX ConnectEx;
		LPFN_TRANSMITPACKETS TransmitPackets; 

		WSA_initerfiniter()
		{

			ConnectEx=0;
			TransmitPackets=0;
			WORD wVersionRequested;
			WSADATA wsaData;
			wVersionRequested = MAKEWORD( 2, 2 );
			err = WSAStartup( wVersionRequested, &wsaData );
			if(!err)
			{
				SOCKET s=::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 


				struct soh
				{
					SOCKET s;
					~soh(){::closesocket(s);}
				} __soh={s};



				GUID wiid=WSAID_CONNECTEX;
				DWORD cbr;
				if(WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,&wiid,sizeof(wiid),&ConnectEx,sizeof(ConnectEx),&cbr,0,0))
				{	
					err=WSAGetLastError();
					return;
				}

				GUID wiid2=WSAID_TRANSMITPACKETS;

				if(WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,&wiid2,sizeof(wiid),&TransmitPackets,sizeof(TransmitPackets),&cbr,0,0))
					err=WSAGetLastError();				

			}

		}
		~WSA_initerfiniter()
		{
			if(err==0) WSACleanup();

		}
		inline operator bool(){ return err==0;}

		inline static WSA_initerfiniter& get_instance()
		{
			return class_initializer_T<WSA_initerfiniter>().get();
		}
	};

*/

	inline 	bool is_VARIANT_empty(VARIANT&v )
	{
		VARTYPE t=v.vt;
		return (t==VT_NULL)||(t==VT_EMPTY)||(t==VT_ERROR);
	}

	template <VARTYPE VT >
	struct variant_cast_t
	{
		VARIANT s;
		VARIANT* pv;
		HRESULT hr;
		variant_cast_t(const VARIANT& src,unsigned short flags=0):pv((VARIANT*)&src),hr(0)
		{
			{
				VARIANT t={};
				s=t;
			}
			if(src.vt!=VT)
			{
				hr=VariantChangeType(pv=&s,&src,flags,VT);
			}	   

		}
		inline  operator VARIANT*()
		{
			return pv;
		}

		inline  VARIANT* operator->()
		{
			return pv;
		}
		inline  operator bool()
		{
			return SUCCEEDED(hr);
		}

		~variant_cast_t(){ if(s.vt) VariantClear(&s); };

	};

	struct exceptinfo_t:EXCEPINFO
	{
		exceptinfo_t(const EXCEPINFO& ei=EXCEPINFO() ):EXCEPINFO(ei){}
		inline EXCEPINFO* clear(int f=1)
		{
			SysFreeString(bstrSource);
			SysFreeString(bstrDescription);
			SysFreeString(bstrHelpFile);
			if(f) *this=exceptinfo_t();
			return this;
		}
		~exceptinfo_t()
		{
			clear(0);
		}

		inline EXCEPINFO detach()
		{
			EXCEPINFO t=*this;
			*this=exceptinfo_t();
			return t;
		}
		inline HRESULT detach(EXCEPINFO* pei)
		{
			if(!pei) return E_POINTER;
			*pei=*this;
			*this=exceptinfo_t();
			return S_OK;
		}

		inline EXCEPINFO* address(){ return clear();}    

		inline static bstr_t error_msg(HRESULT hr=GetLastError())
		{

			const DWORD lang=  MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US);  //MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
			wchar_t buf[512];
			int n=	FormatMessageW( 
				//FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				hr,
				lang, // Default language
				(LPWSTR) buf,
				512,
				NULL 
				);
			if(n)
			{


				buf[n--]=0;
				for(;n>=0;n--) 
					if(iswspace(buf[n])) buf[n]=0;
					else break;  
			}
			return buf;
		}


	};

};