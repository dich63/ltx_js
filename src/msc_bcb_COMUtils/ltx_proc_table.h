#pragma once
#include <windows.h>
#include <comutil.h>
#define DISPATCH_GETSF (DISPATCH_PROPERTYGET|DISPATCH_METHOD)
#define DISPATCH_PUTSF (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)


#define is_get(f) (f&DISPATCH_GETSF)
#define is_put(f) (f&DISPATCH_PUTSF)



struct ltx_proc_table_type
{

 typedef struct _OPEN_ARRAY_BASE
 {
   VARIANT* pargs;
    int arglast;

 } PASCAL_ARRAY_BASE;


 typedef void (__stdcall *COM_signal_type)(void*,long err);
 typedef void ( *exit_asyn_event_type)(void* pcntx,long err,VARIANT* pasynobj,VARIANT*pcookie,EXCEPINFO* pexi);
 typedef  LONG (  __cdecl *PEXTERNAL_CONTROLLER_PROC)(bool fconnect,long strong_lock_count); 

 typedef void (__stdcall * raise_error_proc)(wchar_t* smessage,wchar_t* ssource);
 
 typedef void (__cdecl * invoke_proc_type)
	 (
	 VARIANT* pResult,
	 void* pcntx,
	 raise_error_proc error,
	 int argc,
	 VARIANT* pvarg,
	 ...
	 );


 void* (__stdcall *ltx_on_exit_thread)(void* proc,void *ptr);
 void *rsv1,*rsv2;
 void* ( __stdcall *ltx_SetHardExternalControllerMonitor)(PEXTERNAL_CONTROLLER_PROC pp,LONG flags);
 ULONG ( __stdcall *ltx_StrongLockObjectsCount)();
 HRESULT (__stdcall *ltx_TlsArgs)(IDispatch** ppdisp);
 DWORD (__stdcall *ltx_TlsIndex)();
 HRESULT (__stdcall *ltx_external_data)(IDispatch** ppvObj,BOOL f_cliboard_moniker);
 HRESULT (__stdcall *ltx_external_data_info)(BSTR* pinfo,BOOL f_cliboard_moniker);
 HRESULT (__stdcall *ltx_external_data_link_attach)(VARIANT* pvalue);
 HRESULT (__stdcall *ltx_external_data_link_attach_asyn)(VARIANT* pvalue,/*COM_signal_type*/ void*  wildproc,void* pcntx);
 HRESULT (__stdcall *ltx_create_asyn_object)(VARIANT vobject,void* p_onexit_event,void* p_event_context,VARIANT* pvcookie,VARIANT* ppAsynObj);
 HRESULT (__stdcall *ltx_create_asyn_object_mon)(wchar_t* parse_str,/*exit_asyn_event_type*/ void* p_onexit_event,void* p_event_context,VARIANT* pvcookie,VARIANT* ppAsynObj);
 HRESULT (__stdcall *ltx_asyn_result)(VARIANT* pAsynObject,long ltimeout,VARIANT* presult);
 HRESULT (__stdcall *ltx_server_response_ex)(VARIANT* pServerHolder,long ltimeout,long loptions,VARIANT* pServerResult);
 HRESULT (__stdcall *ltx_server_response)(VARIANT* pServerHolder,long ltimeout,VARIANT* pServerResult);
 HRESULT (__stdcall *ltx_asyn_call_ex)(VARIANT* pvAsynObject,int argc,VARIANT** ppargv,VARIANT* pServerHolder);
 HRESULT (__stdcall *ltx_asyn_call)(VARIANT* pvAsynObject,int argc,VARIANT** ppargv);
 HRESULT (__stdcall *ltx_WrapAsynProc)(PROC proc,void* params,DWORD flag,DWORD threading_model,IDispatch** ppdisp);
 HRESULT (__stdcall *ltx_WrapAsynProcV)(PROC proc,void* params,DWORD flag,DWORD threading_model,VARIANT* pv);
 HRESULT (__stdcall *ltx_bind)(long flags,IDispatch** ppvObj);
 HRESULT (__stdcall *ltx_parse_bind)(BSTR parse_str,VARIANT* pres);
 HRESULT (__stdcall *ltx_unbind)(bool com_check);
 HRESULT (__stdcall *ltx_external_callback_attach_asyn)
 	 (/*invoke_proc_type*/void* callback_proc,void* pcntx,
	 /*COM_signal_type*/ void* onexit_proc,void* pcntx2);
/*
 HRESULT (__stdcall *ltx_asyn_call_ex_pascal)(VARIANT* pServerHolder,VARIANT* pvAsynObject,PASCAL_ARRAY_BASE oa);
 HRESULT (__stdcall *ltx_create_asyn_object_mon_pascal)(VARIANT* ppAsynObj,wchar_t* parse_str,void* p_onexit_event,void* p_event_context,VARIANT* pvcookie);
 HRESULT (__stdcall *ltx_srv_call_pascal)(VARIANT* pSrvResult,VARIANT* pServerHolder,VARIANT vAsynObject,PASCAL_ARRAY_BASE oa);
 */
 HRESULT (__stdcall *ltx_asyn_call_ex_pascal)(VARIANT* pServerHolder,VARIANT* pvAsynObject,VARIANT* pargv,int arglast);
 HRESULT (__stdcall *ltx_create_asyn_object_mon_pascal)(VARIANT* ppAsynObj,wchar_t* parse_str,void* p_onexit_event,void* p_event_context,VARIANT* pvcookie);
 HRESULT (__stdcall *ltx_srv_call_pascal)(VARIANT* pSrvResult,VARIANT* pServerHolder,VARIANT vAsynObject,VARIANT* pargv,int arglast);
 HRESULT (__stdcall *ltx_disp_wrapper_proc)(VARIANT* presult,void* callback_proc,void* pcntx,void* onexit_proc,void* pcntx2);
 HRESULT (__stdcall *ltx_register_CLSID)(CLSID* pclsid,void* pmodule_address);
 HRESULT (__stdcall *ltx_register_CLSID_str)(BSTR sclsid,BSTR libname);
 HRESULT (__stdcall *ltx_disp_wrapper_proc2)(VARIANT* presult,void* callback_proc,void* pcntx,void* onexit_proc,void* pcntx2);
 HRESULT (__stdcall *ltx_disp_wrapper_proc_ex)(VARIANT* pvout,void* callback_proc,void* pcntx,void* onexit_proc,void* pcntx2,unsigned flags);
 HRESULT (__stdcall *ltx_on_external_terminate)(void* onexit_proc,void* pcntx,bool fclibbrd);
};

typedef ltx_proc_table_type* pltx_proc_table;

struct process_modal_t
{
	BOOL ( __stdcall *CreateAssocProcessW)(
		LPCWSTR lpCommandLine,	
		LPCWSTR lpSearchPath,	
		LPSECURITY_ATTRIBUTES lpProcessAttributes,	// pointer to process security attributes 
		LPSECURITY_ATTRIBUTES lpThreadAttributes,	// pointer to thread security attributes 
		BOOL bInheritHandles,	// handle inheritance flag 
		DWORD dwCreationFlags,	// creation flags 
		LPVOID lpEnvironment,	// pointer to new environment block 
		LPCWSTR lpCurrentDirectory,	// pointer to current directory name 
		LPSTARTUPINFO lpStartupInfo,	// pointer to STARTUPINFO 
		LPPROCESS_INFORMATION lpProcessInformation,LPCWSTR phook_app);

	DWORD ( __stdcall *SetProcessAsModal)(HWND hwnParent,LPPROCESS_INFORMATION ppi,void** ppModalHookData);
	DWORD ( __stdcall *SetProcessAsModalEx)(HWND hwnParent,LPPROCESS_INFORMATION ppi,void** ppModalHookData,HANDLE hevent);


	BOOL  ( __stdcall  *CreateAssocProcessAsModalW)(
		HWND hwnParent,
		LPCWSTR lpCommandLine,	
		LPCWSTR lpSearchPath,	
		LPSECURITY_ATTRIBUTES lpProcessAttributes,	// pointer to process security attributes 
		LPSECURITY_ATTRIBUTES lpThreadAttributes,	// pointer to thread security attributes 
		BOOL bInheritHandles,	// handle inheritance flag 
		DWORD dwCreationFlags,	// creation flags 
		LPVOID lpEnvironment,	// pointer to new environment block 
		LPCWSTR lpCurrentDirectory,	// pointer to current directory name 
		LPSTARTUPINFO lpStartupInfo	// pointer to STARTUPINFO 
		,PDWORD pExitCode);

	BOOL ( __stdcall *CreateAssocProcessA)(
		LPCSTR lpCommandLine,	
		LPCSTR lpSearchPath,	
		LPSECURITY_ATTRIBUTES lpProcessAttributes,	// pointer to process security attributes 
		LPSECURITY_ATTRIBUTES lpThreadAttributes,	// pointer to thread security attributes 
		BOOL bInheritHandles,	// handle inheritance flag 
		DWORD dwCreationFlags,	// creation flags 
		LPVOID lpEnvironment,	// pointer to new environment block 
		LPCSTR lpCurrentDirectory,	// pointer to current directory name 
		LPSTARTUPINFO lpStartupInfo,	// pointer to STARTUPINFO 
		LPPROCESS_INFORMATION lpProcessInformation);

	BOOL   ( __stdcall *CreateAssocProcessAsModalA)(
		HWND hwnParent,
		LPCSTR lpCommandLine,	
		LPCSTR lpSearchPath,	
		LPSECURITY_ATTRIBUTES lpProcessAttributes,	// pointer to process security attributes 
		LPSECURITY_ATTRIBUTES lpThreadAttributes,	// pointer to thread security attributes 
		BOOL bInheritHandles,	// handle inheritance flag 
		DWORD dwCreationFlags,	// creation flags 
		LPVOID lpEnvironment,	// pointer to new environment block 
		LPCSTR lpCurrentDirectory,	// pointer to current directory name 
		LPSTARTUPINFO lpStartupInfo	// pointer to STARTUPINFO 
		,PDWORD pExitCode
		);
	BOOL ( __stdcall *RunAsModalW)(HWND hwnParent,
		LPCWSTR lpCommandLine,	
		LPCWSTR lpSearchPath,
		int nCmdShow,void* penvir,
		PDWORD PExitCode);

	BOOL ( __stdcall *RunAsModalA)(HWND hwnParent,
		LPCSTR lpCommandLine,	
		LPCSTR lpSearchPath,
		int nCmdShow,void* penvir,
		PDWORD PExitCode);
	//ULONG64 Crc64;

};

#include <vector>
#include <utility>
#include <algorithm>

template <class PT>
struct proc_lib_helper_t
{
	typedef typename PT proc_table_t;
	proc_table_t* m_pt;
	HMODULE m_hm;
	HRESULT hr;
	 proc_lib_helper_t():m_hm(0),m_pt(0),hr(E_FAIL){};
	~proc_lib_helper_t(){if(m_hm) FreeLibrary(m_hm);}

inline HRESULT	init(const wchar_t* libpathname)
{
	if(!libpathname) return hr=E_POINTER;
	m_hm=LoadLibraryW(libpathname);
	if(m_hm) return hr=S_OK;
	else  hr=HRESULT_FROM_WIN32(GetLastError());
}

inline HRESULT	init(const CLSID& CLSID_X)
{
	BSTR libn=0;
   hr=QueryPathOfRegTypeLib(CLSID_X,-1,-1,0,&libn);
 if(SUCCEEDED(hr))
 {
	m_hm=LoadLibraryW(libn);
	if(!m_hm) hr=HRESULT_FROM_WIN32(GetLastError());
	::SysFreeString(libn);
 }
  return hr;

}

inline bool stay_resident()
{
	HMODULE h;
	return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,(LPCWSTR)m_hm,&h);
}

inline	operator HMODULE()
{
	return m_hm;
}


};

#ifndef variant_t
#define variant_t _variant_t
#endif
struct ltx_proc_table_helper
{


	//template <class Variant_T=variant_t >
	template <class Variant_T>
	struct arguments_t
	{
		typedef Variant_T var_t;
		int argc;
		var_t* argv;
		arguments_t(int _argc,var_t* _argv,int offset=0):argc((_argv)?_argc-offset:0),argv(_argv+offset){};

		inline int length(){return argc;};


inline 	static var_t& verror()
		{
			static var_t sv(DISP_E_PARAMNOTFOUND,VT_ERROR);
			return sv;
		}


		struct arg_t
		{
			 const var_t& v;  
			arg_t(const  var_t& _v):v(_v){};

			operator  const  var_t&() const 
			{
				return v;
			}
           template <class N>
			operator N() 
			{
				return v;
			}
            
		inline 	bool is_empty()
			{
				VARTYPE t=v.vt;
				return (t==VT_NULL)||(t==VT_EMPTY)||(t==VT_ERROR);
			}


			template <class N>
			N def(N dv=N()) {
				try     {
					return (!is_empty())?v:dv;
				}
				catch(...){
				}
				return dv;
			}

			template<class Intf>
			HRESULT QueryInterface(Intf** ppObj)
			{
				HRESULT hr;
				return hr=QueryInterface(__uuidof(Intf),(void**)ppObj); 
			}

			HRESULT QueryInterface(REFIID riid, void** ppObj)
			{
				HRESULT hr;
				var_t tmp;
				if(SUCCEEDED(hr=VariantChangeType(&tmp,&v,0,VT_UNKNOWN))) 
				{ 
					hr=tmp.punkVal->QueryInterface(riid,ppObj);
				}
				return hr; 
			}

		};

		arg_t operator[](int n)
		{
			if((0<=n)&&(n<argc))
				return argv[n];         
			return  verror();       
		}

	};


	pltx_proc_table m_pt;
    HMODULE m_hm;

inline static void*  blocked()
{
	return (void*)256;
}
inline	operator HMODULE()
	{
		return m_hm;
	}

bool stay_resident()
{
	HMODULE h;
 
	typedef BOOL		(__stdcall *GetModuleHandleExW_t)(DWORD,LPCWSTR,HMODULE*);
		GetModuleHandleExW_t GetModuleHandleExW=(GetModuleHandleExW_t)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"),"GetModuleHandleExW");
/*
	#define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS        (0x00000004)
        #define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT  (0x00000002)
        #define GET_MODULE_HANDLE_EX_FLAG_PIN                 (0x00000001)
      //	return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,(LPCWSTR)m_hm,&h);
      */
      	return GetModuleHandleExW(5,(LPCWSTR)m_hm,&h);
}

	inline static CLSID ltx_clsid()
	{
         CLSID CLSID_X={0x9A7E6809,0x701C,0x45D0,{0x9D,0xA5,0xBF,0xB5,0xED,0x2C,0x4F,0xAA}};
         return   CLSID_X;
	}
	inline static CLSID lib_clsid()
	{
		//BF0D6B0F-B96E-4C12-9096-15AD47A28D79
		//MIDL_DEFINE_GUID(IID, LIBID_LifeTimeXLib,0xBF0D6B0F,0xB96E,0x4C12,0x90,0x96,0x15,0xAD,0x47,0xA2,0x8D,0x79);
		//CLSID CLSID_X={0x9A7E6809,0x701C,0x45D0,{0x9D,0xA5,0xBF,0xB5,0xED,0x2C,0x4F,0xAA}};
        CLSID CLSID_X={0xBF0D6B0F,0xB96E,0x4C12,{0x90,0x96,0x15,0xAD,0x47,0xA2,0x8D,0x79}};
		return   CLSID_X;
	}

	inline static HMODULE LoadLibraryX(wchar_t* plib_name=L"LifeTimeX.dll",CLSID& CLSID_X=ltx_clsid())
	{
		HMODULE hm;
		HRESULT hr0,hr;

         wchar_t buf[MAX_PATH];

    	 if(GetEnvironmentVariableW(L"LifeTimeX_PATH",buf,MAX_PATH))
		{
		   wchar_t buf1[MAX_PATH],*p;
          if(SearchPathW(buf,plib_name,NULL,MAX_PATH,buf1,&p))
		  {
			  if(hm=LoadLibraryW(buf1))
				  return hm;
		  }
		}

//	
		 BSTR libn=0;
    	 hr=QueryPathOfRegTypeLib(CLSID_X,-1,-1,0,&libn);
		 if(SUCCEEDED(hr))
		 {
             hm=LoadLibraryW(libn);
			 ::SysFreeString(libn);
			 return hm;
		 }

		 return 0;

/*
		IUnknown* pc;
        hr=CoInitializeEx(0,COINIT_MULTITHREADED);    
		hr0=CoGetClassObject(ltx_clsid(),CLSCTX_ALL,0,__uuidof(IUnknown),(void**)&(pc=0));
		
		hm=LoadLibraryW(plib_name);
		if(pc) pc->Release();
		if(SUCCEEDED(hr)) CoUninitialize();
		return hm;
*/
	};

	ltx_proc_table_helper(long flags=1,wchar_t* libpathname=L"LifeTimeX.dll")
   {
	     m_pt=0;
          m_hm=0;
		 if(flags) m_hm=LoadLibraryW(libpathname);
		
      
         if((!m_hm)&&((flags&2)==0)) m_hm=LoadLibraryX();
		// printf("Ok\n");
		 if(!m_hm) return;

		 void* p=GetProcAddress(m_hm,"ltx_proc_table");
		 //printf("GetProcAddress [%p] Ok \n",p);
		 char const * pc=*((char const**)p);
		// printf("const ltx_proc_table [%p] Ok \n",pc);
		 if(p) m_pt=(ltx_proc_table_type*)pc;
		// printf("ltx_proc_table [%p] Ok \n",m_pt);

   }




   ~ltx_proc_table_helper()
   {
	   if(m_hm) FreeLibrary(m_hm);
   }

operator pltx_proc_table()
{
  return m_pt;
}
pltx_proc_table   operator ->()
{
	return m_pt;
}
operator bool()
{
	return m_pt;
}

inline static VARIANT verr(HRESULT err=DISP_E_PARAMNOTFOUND)
{
	VARIANT v={VT_ERROR};
	v.scode=err;
	return v;
}


//inline HRESULT script(VARIANT* pres,wchar_t* script_str,VARIANT v0=verr(),VARIANT v1=verr(),VARIANT v2=verr(),VARIANT v3=verr(),VARIANT v4=verr(),VARIANT v5=verr(),VARIANT v6=verr())
//template <class VARIANT_T>
//inline HRESULT script(VARIANT* pres,wchar_t* script_str,VARIANT_T v0=verr(),VARIANT_T v1=verr(),VARIANT_T v2=verr(),VARIANT_T v3=verr(),VARIANT_T v4=verr(),VARIANT_T v5=verr(),VARIANT_T v6=verr())

inline HRESULT bind_call(VARIANT* pres,wchar_t* mon_str,int argc=0, VARIANT* pvarg=0,EXCEPINFO* pexceptinfo=0)
{

	HRESULT hr;
	VARIANT vres={},vdisp={};

	//if(FAILED(hr=m_pt->ltx_parse_bind(L"STA:script:dbg:1",&vdisp))) 
	if(FAILED(hr=m_pt->ltx_parse_bind(mon_str,&vdisp))) 
		return hr;

	
	std::vector<VARIANT> vv(argc);
	if(argc) 
		//std::copy_backward(pvarg,pvarg+argc,vv.begin());
		std::reverse_copy(pvarg,pvarg+argc,vv.begin());
	DISPPARAMS dp={(argc)?&vv[0]:0,NULL,argc,0};

	hr=vdisp.pdispVal->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dp,&vres, pexceptinfo, NULL);
	VariantClear(&vdisp);
	//VariantClear(&vv[argc]);
	if(pres) *pres=vres;
	else VariantClear(&vres);
	return hr;
};


inline HRESULT script(VARIANT* pres,wchar_t* script_str,VARIANT v0=verr(),VARIANT v1=verr(),VARIANT v2=verr(),VARIANT v3=verr(),VARIANT v4=verr(),VARIANT v5=verr(),VARIANT v6=verr())
{

	HRESULT hr;
	VARIANT vss={VT_BSTR},vres={},vdisp={};

	//if(FAILED(hr=m_pt->ltx_parse_bind(L"STA:script:dbg:1",&vdisp))) 
	if(FAILED(hr=m_pt->ltx_parse_bind(L"STA:script",&vdisp))) 
			return hr;

	vss.bstrVal=::SysAllocStringLen(script_str,wcslen(script_str)+1);

	VARIANT vv[8];//={v6,v5,v4,v3,v2,v1,v0,vss};
        vv[0]=v6; vv[1]=v5;vv[2]=v4;vv[3]=v3;vv[4]=v2;vv[5]=v1;vv[6]=v0;vv[7]=vss;
	int nc;


	for(nc=0;nc<8;nc++)
	{
		 if(vv[nc].vt!=VT_ERROR)
			   break;
	}

	DISPPARAMS dp={vv+nc,NULL,8-nc,0};
	hr=vdisp.pdispVal->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dp,&vres, NULL, NULL);
	VariantClear(&vdisp);
	VariantClear(&vss);
     if(pres) *pres=vres;
	 else VariantClear(&vres);
	return hr;
	
}


};

template <class T>
struct disp_wrap_base_t
{
	
	typedef   VARIANT com_variant_t;
	typedef   ltx_proc_table_type::raise_error_proc raise_error_proc;
	
	//typedef void (__stdcall * raise_error_proc_t)(wchar_t* smessage,wchar_t* ssource);


	inline T* thisT()
	{
		T* t = static_cast<T*>(this);
		return t;
	}

static	  void __cdecl  s_on_exit(T* p)
{
   delete p;
}

//inline  void  operator()(typename T::com_variant_t* presult,raise_error_proc error,int argc,typename T::com_variant_t* argv,int arglast,void* pei,HRESULT& hr,unsigned flags){};

static	  void __cdecl  s_invoke
		(VARIANT* presult, T* pt,raise_error_proc error,int argc,VARIANT* argv,int arglast,HRESULT& hr,void* pei,unsigned flags)
	{
		//AllocConsole();

		typedef typename T::com_variant_t* pvar_t;
		

		try
		{
          (*pt)(pvar_t(presult),error,argc,pvar_t(argv),arglast,hr,pei,flags);
			
		}
		catch(...){
			error(L"Unknown Exception !",0);}

	};


};

template <class T,class com_variant_t>
com_variant_t& wrap_dispacher_class( ltx_proc_table_type* pltx, T* pt,com_variant_t* pvout=&com_variant_t(),int apartment_flags=2)
{
	pltx->ltx_disp_wrapper_proc_ex(pvout,&T::s_invoke,pt,&T::s_on_exit,pt,apartment_flags);
	return *pvout;
}


