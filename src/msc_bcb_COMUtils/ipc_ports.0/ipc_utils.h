#pragma once


#include "ipc_utils_base.h"
#include "static_constructors_0.h"
#include "singleton_utils.h"

/*
#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>
#include "static_constructors_0.h"
#include "singleton_utils.h"
#include "ipc_ports/ltx_call_helper.h"
*/

//

//#include "static_com.h"


//#pragma comment(lib ,"Ws2_32.lib")



#define HINSTANCE_IN_CODE (ipc_utils::module_t<>::__getHinstance())

namespace ipc_utils {









	template <int VV=0>
	struct win_ver_6_t
	{

	static	 HWND wnd_alloc(char* winclass,char* winname=NULL,HWND hwinParent=0)
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
    static	 HWND wnd_lite(char* winclass)
		{
			return wnd_alloc(winclass,0,HWND_MESSAGE);
		};

		static int thread_id(HANDLE h)
		{
			 DWORD (__stdcall *GetThreadId)(HANDLE h);
			if(SET_PROC_ADDRESS(GetModuleHandleW(L"kernel32.dll"),GetThreadId))
			return GetThreadId(h);
			 return 0;         
           
		}


		class ClipboardText
		{
			HWND hwin;
		public:
			ClipboardText()
			{
				hwin=wnd_lite("Clip00");
				OpenClipboard(hwin);
			};
			ClipboardText(bstr_t str)
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
			inline operator bstr_t()
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



	};


/*



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


	struct thread_handle_t
	{
		HRESULT hr;
		HANDLE hp;
		int tid;
		thread_handle_t(int _tid=0):tid(_tid),hp(0),hr(0)
		{
		}
		thread_handle_t(HANDLE h):tid(win_ver_6_t<>::thread_id(h)),hp(h)
		{
			hr=((tid==0)&&(h))?GetLastError():0;
		}
		thread_handle_t(int _tid,HANDLE h):tid(_tid),hp(h),hr(0){}

		~thread_handle_t()
		{
			if(hp) CloseHandle(hp);
		}

		inline HANDLE get_handle()
		{
			if((tid)&&(hp==0)) 
			{
				hp=OpenThread(THREAD_ALL_ACCESS,0,tid);
				hr=GetLastError();
			}
			return hp;

		}

		inline operator HANDLE() const
		{

			thread_handle_t* t=const_cast<thread_handle_t*>(this);
			return t->get_handle();
		}
		inline operator bool()
		{
			return HANDLE(*this)!=0;
		}



	};


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
			
			wchar_t buf[256];
			StringFromGUID2(clsid,buf,256);
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


		template<class Intf>
		HRESULT QueryInterface(Intf** ppvObject)
		{
			HRESULT hr;
			if(p) return hr=p->QueryInterface(__uuidof(Intf),(void**)ppvObject);
			else return hr=E_POINTER;
		}

		HRESULT CoCreateInstance( wchar_t* bclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) 
		{
			
			HRESULT hr;
			CLSID rclsid;
			if(SUCCEEDED(hr=CLSIDFromString(bclsid,&rclsid)))
				return hr=CoCreateInstance(rclsid, pUnkOuter, dwClsContext);
		}

		HRESULT CoCreateInstance( REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) 
		{
			HRESULT hr;
			reset();
			return hr=::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(N), (void**)&p);
		}

	};


	struct COMInit_t
	{
	
		HRESULT hr;
		COMInit_t():hr(CO_E_NOTINITIALIZED){};
		COMInit_t(DWORD mode):hr(CO_E_NOTINITIALIZED){
			init(mode);  
		};

		void init(DWORD mode= COINIT_MULTITHREADED  )
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


*/

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

/*

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

*/

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
	

	struct process_handle_t
	{
		HRESULT hr;
		HANDLE hp;
		int tid;
		process_handle_t(int _tid=0):tid(_tid),hp(0),hr(0){
		}
		process_handle_t(HANDLE h):tid(GetProcessId(h)),hp(h){
			hr=((tid==0)&&(h))?GetLastError():0;
		}
		process_handle_t(int _tid,HANDLE h):tid(_tid),hp(h),hr(0){}

		~process_handle_t()
		{
			if(hp) CloseHandle(hp);
		}

		inline HANDLE get_handle(DWORD dwDesiredAccess=PROCESS_DUP_HANDLE|PROCESS_QUERY_INFORMATION)
		{
			if((tid)&&(hp==0)) 
			{
				hp=OpenProcess(dwDesiredAccess,0,tid);
				hr=GetLastError();
			}
			return hp;

		}

		inline operator HANDLE() const
		{

			process_handle_t* t=const_cast<process_handle_t*>(this);
			return t->get_handle();
		}
		inline operator bool()
		{
			return HANDLE(*this)!=0;
		}



	};



	template<class T>
	class CHGlobal_t
	{
	public:
		HGLOBAL m_h;
		T* p;
		bool ffree;
		CHGlobal_t(HGLOBAL h=0,bool _ffree=false):m_h(h),p(0),ffree(_ffree)
		{
			//if(m_h) p=(T*) GlobalLock(m_h);
		}

		CHGlobal_t(int cb,void* pd=0,bool _ffree=true):m_h(0),p(0),ffree(_ffree)
		{
			m_h=GlobalAlloc(GHND,cb); 
			if(m_h) p=(T*) GlobalLock(m_h);
			if(p&&pd)
				memcpy(p,pd,cb);
		}

		inline HGLOBAL detach()
		{
			if(m_h&&p) GlobalUnlock(m_h);
			//HGLOBAL tmp=m_h;
			//return tmp=m_h,m_h=NULL,tmp;
			return make_detach(m_h);
		}
		inline void clear()
		{
			HGLOBAL h=detach();
			if(ffree&&h) GlobalFree(h);
		}

		~CHGlobal_t()
		{
			clear();
		}

		inline HGLOBAL 	handle()
		{
			return m_h;
		}

		inline operator T*()
		{
			if(m_h&&(p==0)) p=(T*)GlobalLock(m_h);
			return p;
		}
		inline size_t sizeb()
		{
			return   GlobalSize(m_h);
		};
		inline size_t size()
		{
			return   sizeb()/sizeof(T);
		};

	};







	inline  HRESULT CoMarshalInterfaceToHGlobal(IUnknown *pUnk,/* out*/HGLOBAL* phmem,DWORD ctx=MSHCTX_LOCAL,DWORD mshflags=MSHLFLAGS_TABLESTRONG)
	{
		if(!phmem) return E_POINTER;
		if(!pUnk) return E_POINTER;
		HRESULT hr,hr2;
		HGLOBAL hmem;
		smart_ptr_t<IStream> stream;
		if(FAILED(hr=CreateStreamOnHGlobal(0,false,stream.address()))) return hr;

		hr2=CoMarshalInterface(stream,IID_IUnknown,pUnk,ctx,0,mshflags) ;

		if(FAILED(hr=GetHGlobalFromStream(stream,&hmem))) return hr;
		if(FAILED(hr2)) return GlobalFree(hmem),hr2;
		*phmem=hmem;
		return S_OK;
	};

	inline  HRESULT CoUnmarshalInterfaceFromHGlobal(HGLOBAL hmem,REFIID riid,void** ppv,int release_flags=0)
	{
		if(!(hmem&&ppv)) return E_POINTER;

		HRESULT hr,hr0;
		smart_ptr_t<IStream> stream;
		if(FAILED(hr=CreateStreamOnHGlobal(hmem,release_flags&2,stream.address()))) return hr;
		hr=CoUnmarshalInterface(stream,riid,ppv);
		if(release_flags) {
			stream->Seek(LARGE_INTEGER(),STREAM_SEEK_SET,&ULARGE_INTEGER());
			hr0=CoReleaseMarshalData(stream);
		}

		return hr;
	};


	inline  HRESULT CoDuplicateCOMStub(HGLOBAL hmem,HGLOBAL* phout,bool frelease,DWORD ctx=MSHCTX_LOCAL,DWORD mshflags=MSHLFLAGS_TABLESTRONG)
	{
		HRESULT hr=S_OK;

		COMInit_t ci(COINIT_MULTITHREADED);

		smart_ptr_t<IUnknown> unk;
		smart_ptr_t<IStream> stream;

		if(FAILED(hr=CreateStreamOnHGlobal(hmem,false,stream.address()))) return hr;
		if(phout)
		{
			if(FAILED(hr=CoUnmarshalInterface(stream,__uuidof(IUnknown),unk.ppQI()))) return hr;
		}

		if(frelease) {
			stream->Seek(LARGE_INTEGER(),STREAM_SEEK_SET,&ULARGE_INTEGER());
			hr=CoReleaseMarshalData(stream);
		}

		if(!phout) return hr;


		if(FAILED(hr=CreateStreamOnHGlobal(NULL,false,stream.address()))) return hr;
		if(FAILED(hr=CoMarshalInterface(stream,__uuidof(IUnknown),unk,ctx,0,mshflags))) return hr;
		hr=GetHGlobalFromStream(stream,phout);

		return hr;  
	}

	inline  HRESULT CoReleaseCOMStub(HGLOBAL hmem,bool fdestroy=false)
	{
		HRESULT hr;
		if(GMEM_INVALID_HANDLE==GlobalFlags(hmem))
			return hr=HRESULT_FROM_WIN32(GetLastError());
		hr=CoDuplicateCOMStub(hmem,NULL,true);
		if(fdestroy) 
			GlobalFree(hmem);
		return hr;
	}


	inline 	HANDLE dup_handle(HANDLE hsrc,HANDLE htp=GetCurrentProcess())
	{
		HANDLE ht=0;
		DuplicateHandle(GetCurrentProcess(),hsrc,htp,&ht,0,FALSE,DUPLICATE_SAME_ACCESS);
		return ht;
	};


	struct WA_socket_utils
	{
		inline static SOCKET socket(WSAPROTOCOL_INFOW* pwsapi)
		{
		   WSA_initerfiniter::get_instance();

			return WSASocketW(FROM_PROTOCOL_INFO,FROM_PROTOCOL_INFO,FROM_PROTOCOL_INFO,pwsapi, 0, FROM_PROTOCOL_INFO);
		}
		inline static SOCKET socket(WSAPROTOCOL_INFOA* pwsapi)
		{
			WSA_initerfiniter::get_instance();
			return WSASocketA(FROM_PROTOCOL_INFO,FROM_PROTOCOL_INFO,FROM_PROTOCOL_INFO,pwsapi, 0, FROM_PROTOCOL_INFO);
		}

		template<class WSAInfo>
		inline static HRESULT socket(WSAInfo& wsapi,SOCKET* ps)
		{
			SOCKET s=WA_socket_utils::socket(&wsapi);
			if(INVALID_SOCKET==s) return WSAGetLastError();
			*ps=s;
			return S_OK;
		}
		


		inline static HRESULT get_info(SOCKET s,WSAPROTOCOL_INFOW* pwsapi)
		{
			WSA_initerfiniter::get_instance();
			int sz=sizeof(WSAPROTOCOL_INFOW);
			return 	(0==getsockopt(s,SOL_SOCKET,SO_PROTOCOL_INFOW,(char*)pwsapi,&sz))?S_OK:WSAGetLastError();
		}

		inline static HRESULT get_info(SOCKET s,WSAPROTOCOL_INFOA* pwsapi)
		{
			WSA_initerfiniter::get_instance();
			int sz=sizeof(WSAPROTOCOL_INFOA);
			return 	(0==getsockopt(s,SOL_SOCKET,SO_PROTOCOL_INFOA,(char*)pwsapi,&sz))?S_OK:WSAGetLastError();
		}

	};

	template  <class WSAInfo>
	HRESULT WSASocketUnmarshal(
		__in        HANDLE hSourceProcessHandle,
		__in        WSAInfo* pSourceInfo,
		__in        HANDLE hTargetProcessHandle,
		__deref_out SOCKET* pTargetSocket)

	{
		if((!pTargetSocket)||(!pSourceInfo)) return E_POINTER;

		HRESULT hr;
		HANDLE hs,hst=HANDLE(pSourceInfo->dwProviderReserved);
		BOOL f;
		f=DuplicateHandle(hSourceProcessHandle,hst,hTargetProcessHandle,&hs,0,FALSE,DUPLICATE_SAME_ACCESS);
		if(!f) return hr=GetLastError();
		WSAInfo tmp=*pSourceInfo;
		tmp.dwProviderReserved=(DWORD)hs;
		if(FAILED(hr=WA_socket_utils::socket(tmp,pTargetSocket)))
			CloseHandle(hs);
		return hr;

	}


	template  <class WSAInfo>
	HRESULT WSASocketMarshal(SOCKET SourceSocket,WSAInfo* pInfo)
	{
		HRESULT hr;
		if(!pInfo) return E_POINTER;
		if(SUCCEEDED(hr=WA_socket_utils::get_info(SourceSocket,pInfo)))
			pInfo->dwProviderReserved=(DWORD)SourceSocket;
		return hr;
	}

	template  <class WSAInfo>
	HRESULT WSADuplicateSocket(SOCKET s,const process_handle_t& target_process,WSAInfo* pInfo)
	{
		BOOL f;
		HRESULT hr;
		if(FAILED(hr=WA_socket_utils::get_info(s,pInfo)))
			return hr;
		HANDLE htp=(HANDLE)target_process,hc=GetCurrentProcess(),hts;
		if(!htp) return target_process.hr;

		f=DuplicateHandle(hc,(HANDLE)s,htp,&hts,0,FALSE,DUPLICATE_SAME_ACCESS);
		if(!f) return hr=GetLastError();
		pInfo->dwProviderReserved=(DWORD)hts;
		return hr;

	}

inline	SOCKET dup_socket(SOCKET s)
	{
		WSA_initerfiniter::get_instance();
		WSAPROTOCOL_INFOW info;
		if(S_OK==WSADuplicateSocket(s,GetCurrentProcess(),&info))
			return WA_socket_utils::socket(&info);
		else 
			return INVALID_SOCKET;
	}


	inline 	  int CheckCOMStub(HANDLE h,HRESULT& hr)
	{
		CHGlobal_t<DWORD> gl=HGLOBAL(h);
		try{
			SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOALIGNMENTFAULTEXCEPT);    
		if(GlobalFlags(HGLOBAL(h))==GMEM_INVALID_HANDLE)  hr=GetLastError();
		else
			if(gl.size()>3)
			{
				//if(strncmp((char*)gl,"MEOW",4)==0)
				if(((DWORD*)gl)[0]==((DWORD*)"MEOW")[0])
					return 8|4;
				return 4;
			}
			return 0;
		}catch(...){

		};
	}

	inline bool check_socket(HANDLE h)
	{
		DWORD dw;
		int ii=sizeof(dw);
        return !(SOCKET_ERROR==getsockopt(SOCKET(h),SOL_SOCKET,SO_TYPE,(char*)&dw,&ii));
	}
    
	inline 	  int GetHandleType(HANDLE h,HRESULT* phr=0)
	{
		HRESULT t;
		HRESULT&  hr=(phr)?*phr:t;

		hr=S_OK;

		BOOL f;
		DWORD dw;
		if(!(f=GetHandleInformation(h,&dw)))
		{
			hr=GetLastError();
			int k=CheckCOMStub(h,hr);
			return k;

		}
		else
		{
			int ii=sizeof(ii);
			if(SOCKET_ERROR==getsockopt(SOCKET(h),SOL_SOCKET,SO_TYPE,(char*)&dw,&ii))
			{
				hr=WSAGetLastError();
				int k=CheckCOMStub(h,hr);
				return (k)?k:1;

			}
			else return 2;   


			return 1;

		}

		return 0;
	}



	struct weak_destructor_t
	{
		static void destroy_socket(void* p,void* pctx)
		{
			if(p) closesocket(SOCKET(p));
		}
		static void destroy_handle(void* p,void* pctx)
		{
			DWORD dw;
			HANDLE h=HANDLE(ULONG_PTR(p)&(~ULONG_PTR(1)));
			if(p&&GetHandleInformation(h,&dw))
				CloseHandle(h);
		}
		static void destroy_hglobal(void* p,void* pctx)
		{
			DWORD dw;
			if(p&&(GMEM_INVALID_HANDLE!=GlobalFlags(HGLOBAL(p))))
				GlobalFree(HGLOBAL(p));			
		}
		static void destroy_comstub(void* p,void* pctx)
		{
			if(p)
				ipc_utils::CoReleaseCOMStub(HGLOBAL(p),true);
		}
	};

	template <int Vers=0>
	struct module_t
	{
        struct wbuf_t
		{
			enum {
				size=2*MAX_PATH
			};
			wchar_t buf[size];

			wbuf_t(){buf[0]=0;buf[1]=0;}


			operator wchar_t*()
			{
				return buf;
			}

		};
		
		 static HRESULT DLL_AddRef(void* p)
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

		inline static HMODULE  __getHinstance(void* p=&DLL_AddRef,bool faddref=false,HRESULT* phr=0)
		{
			HMODULE h=0;
			DWORD flags=(faddref)?GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
				:GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;

			BOOL f=GetModuleHandleExW(flags,(LPCWSTR)p,&h);
			if(phr) *phr=(f)?S_OK:GetLastError();
			return h;
		};

		inline static wbuf_t  module_path(void* p=&DLL_AddRef,HRESULT* phr=0)
		{
			HRESULT hr;
			wbuf_t buf,buf2;
			wchar_t* pt=0;
			buf=module_name(p, &hr);
			GetFullPathNameW(buf,buf2.size,buf2,&pt);		
			if(pt) pt[0]=0;
			if(phr) *phr=hr;
			return buf2;
		}

		inline static wbuf_t  module_name(void* p=&DLL_AddRef,HRESULT* phr=0)
		{
             wbuf_t buf;
			 HRESULT hr;
			 HMODULE h=__getHinstance(p,false,&hr);
			 if(SUCCEEDED(hr))
			 {
				 
				 
				 bool f=GetModuleFileNameW(h,buf.buf,buf.size);            

			 }

			 if(phr) *phr=hr;
			 
			 
			 return buf;
			 
		}
       inline static wbuf_t  module_name(IUnknown* punk,HRESULT* phr=0)
	   {
           void*** ppp=(void***)punk;
		   if(ppp&&ppp[0]&&ppp[0][0])
			   return module_name(ppp[0][0],phr);
               else if(phr) *phr=E_POINTER;
			    return wbuf_t();
	   }
		
		

	};
/*
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

	};

	*/

};