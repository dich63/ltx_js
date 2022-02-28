// ltx_js.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"


//#include "ipc_ports/com_super_factory.h"

#include "ipc_ports/hook_coregister.h"
#include "ipc_ports/com_process.h" 
#include "ipc_ports/ltx_js_console.h"
#include "ipc_ports/ltx_js.h"
#include "ipc_ports/com_marshal_disp_by_val.h"
#include "ipc_ports/dispatch_by_name_aggregator.h"
//#include "ipc_ports/mbv_buffer.h"
//#include "ipc_ports/mbv_buffer2.h"
#include "ipc_ports/mbv_mm.h"
//
#include "ipc_ports/mbv_socket.h"
#include "ipc_ports/mbv_koh.h"
#include "ipc_ports/mbv_container.h"
#include "ipc_ports/windesktops.h"

#include "singleton_utils.h"
#include <Commdlg.h>
//

#include "web_silk2.h"
#include <mshtmhst.h>
#include <Mshtml.h>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>





//#include "os_utils.h"


#include "afxres.h"
#define  RSET_ID L"739F0D1F-C63B-44d2-9976-208C6A3F95FE"
#define  EVENT_ID L"EVENT_" RSET_ID 
#define  OBJREF_ID L"OBJREF_" RSET_ID 

//argv_ini
#pragma comment(lib ,"urlmon.lib")



#include "ipc_ports/ltx_utils_list.h"

//int g_inits=ltx_global_ref_count(),g_init_f;

volatile LONG* p_GRefCount=&RefCounterGlobal_t<>::get().refcount;



extern "C" HRESULT ltx_register_classes(const class_triplet_t* pct,int * pbadindex)
{
	class_triplet_t* p=(class_triplet_t*)pct;
	HRESULT hr=S_OK;
	int t;
	int& indx=(pbadindex)? *pbadindex:t;
	indx=-1;  
	while(p->clsid!=GUID_NULL)
	{
		++indx;
		OLE_CHECK_hr(register_class_map_t<>::get_instance().register_moniker(p->clsid,p->GetClassObject,p->alias,0,0));		
		++p;
	}
	return hr;	
}






struct registration_setup_t
{
	HRESULT hr;
	int index;
	smart_ptr_t<IUnknown> uncf;
	registration_setup_t()
	{
		hr=E_FAIL;
		//REG_FACTORY_BEGIN(triplet)
		LTX_REG_FACTORY_BEGIN2(&hr,&index)
        //LTX_REG_FACTORY_MARSHAL(CTestFactory2_t,testmarshal)		
        LTX_REG_FACTORY(script_engine_factory_t,script)
		LTX_REG_FACTORY2(asyn_factory_t,asyn,asyncall)
		//LTX_REG_FACTORY(asyn_factory_t,asyncall)
		LTX_REG_FACTORY(file_search_factory_t,fileSearch)
		LTX_REG_FACTORY3(dll_bind_factory_t,dll,module,lib)
		//LTX_REG_FACTORY(dll_bind_factory_t,module)
		//LTX_REG_FACTORY(dll_bind_factory_t,lib)
		LTX_REG_FACTORY2(external_connect_locker_factory_t,ec_locker, external.connector)
		//LTX_REG_FACTORY(external_connect_locker_factory_t,external.connector)
		LTX_REG_FACTORY(binder_factory_t,*)
		LTX_REG_FACTORY(binder_factory_t,)
		LTX_REG_FACTORY(process_factory_t,process)
		LTX_REG_FACTORY(callback_factory_factory_t,callback.factory)
		LTX_REG_FACTORY2(ec_wrapper_factory_t,ec_wrapper, external.wrapper)
		//LTX_REG_FACTORY(ec_wrapper_factory_t,external.wrapper)
		LTX_REG_FACTORY(mbv_dispatch_register_t,mbv.register)		
		LTX_REG_FACTORY2(stub_holder_factory_t,stub, stub_holder)		

		
			

    	//LTX_REG_FACTORY8(utils_factory_t,external,clipbrd,progid,srv,lpc,.net,dot-net,old_style.external)
		//LTX_REG_FACTORY3(utils_factory_t, js.factory,global,host.script)
		
		LTX_REG_FACTORY(utils_factory_t,external)
		LTX_REG_FACTORY(utils_factory_t,clipbrd)
		LTX_REG_FACTORY(utils_factory_t,progid)		
		LTX_REG_FACTORY2(utils_factory_t,srv,lpc)		
		LTX_REG_FACTORY(utils_factory_t,old_style.external)						
		LTX_REG_FACTORY2(utils_factory_t,.net,dot-net)

		LTX_REG_FACTORY(utils_factory_t, js.factory)
		LTX_REG_FACTORY(utils_factory_t, global)
		LTX_REG_FACTORY(utils_factory_t, host.script)
		
		LTX_REG_FACTORY(utils_factory_t, ref)

			
		LTX_REG_FACTORY_END 

		//return;
        //hr=ltx_register_classes(triplet,&index);

		//

		//void *p=0;
		//hr=super_factory_t::CreateFactory(__uuidof(IStream),&p);


    	//CLSID cc = com_register_t<>::getUUID();

		//




		COMInitF_t ci;

		bool fnoinstall = 1;

		smart_ptr_t<IMarshalByValueDispatchRegister> mbvdr;
		if(SUCCEEDED(hr=super_factory_t::CoGetObject(L"ltx.bind:mbv.register",__uuidof(IMarshalByValueDispatchRegister),mbvdr._ppQI())))
		{
			//hr=mbvdr->Register(L"CoDispatch_mbv",&CoDispatch_mbv::getMCD());
			if (fnoinstall) {
				hr = mbvdr->Register(L"mm_buffer", &mbv_mm::mm_context_t::getMCD());
				hr = mbvdr->Register(L"sm_buffer", &mbv_mm::mm_context_t::getMCD());
				hr = mbvdr->Register(L"sh_socket", &mbv_socket::sh_socket_context_t::getMCD());
				hr = mbvdr->Register(L"socket", &mbv_socket::sh_socket_context_t::getMCD());
				//hr=mbvdr->Register(L"ko_socket",&mbv_socket::sh_socket_context_t::getMCD());
				hr = mbvdr->Register(L"mbv_map", &mbv_container::mbv_container_t::getMCD());
				hr = mbvdr->Register(L"mbv.map", &mbv_container::mbv_container_t::getMCD());
			}

			hr=mbvdr->Register(L"kernel_object",&mbv_kernel_object::sh_kernel_object_t::getMCD());
			hr = mbvdr->Register(L"kernel.object", &mbv_kernel_object::sh_kernel_object_t::getMCD());
			

			//hr=mbvdr->Register(L"sm_buffer",&mbv2::mbv_buffer_t<>::getMCD());

			//hr=mbvdr->Register(L"sh_buffer",&mbv::mbv_buffer_t<>::getMCD());
			//hr=mbvdr->Register(L"sh_buffer_float64",&mbv::mbv_buffer_t<mbv::float64_class_t>::getMCD());

		}

		mbvdr.Release();
		RefCounterGlobal_t<>::get().AddRefEx(-RefCounterGlobal_t<>::get().RefCount());


	
		

		
	}

} registration_setup;


//hook_ltx_corerister_t hook_ltx_corerister;

// test
/*
struct registration_setup_t
{
	HRESULT hr;
	smart_ptr_t<IUnknown> uncf;
  registaration_setup_t()

  {
	  //1376092800 2013 8 10 
	  silki_t<1376092800>::update();  
	  hr=ltx_register_class(script_engine::script_engine_factory_t::GetCLSID(),&script_engine::script_engine_factory_t::GetClassObject,L"script",uncf.address());
	  
	  hr=ltx_register_class(asyn_factory_t::GetCLSID(),&asyn_factory_t::GetClassObject,L"asyn",uncf.address());
	  hr=ltx_register_class(file_search_factory_t::GetCLSID(),&file_search_factory_t::GetClassObject,L"fileSearch",uncf.address());
	  hr=ltx_register_class(dll_bind_factory_t::GetCLSID(),&dll_bind_factory_t::GetClassObject,L"dll",uncf.address());
	  hr=ltx_register_class(external_connect_locker_factory_t::GetCLSID(),&external_connect_locker_factory_t::GetClassObject,L"ec_locker",uncf.address()); 
	  //hr=ltx_register_class(binder_factory_t::GetCLSID(),&binder_factory_t::GetClassObject,L"*",uncf.address());
	  hr=ltx_register_class(binder_factory_t::GetCLSID(),&binder_factory_t::GetClassObject,L"",uncf.address());

	  hr=ltx_register_class(process_factory_t::GetCLSID(),&process_factory_t::GetClassObject,L"process",uncf.address());

 		  hr=ltx_register_class(utils_factory_t::GetCLSID(),&utils_factory_t::GetClassObject,L"external",uncf.address());
		  hr=ltx_register_class(utils_factory_t::GetCLSID(),&utils_factory_t::GetClassObject,L"clipbrd",uncf.address());
  		  hr=ltx_register_class(utils_factory_t::GetCLSID(),&utils_factory_t::GetClassObject,L"srv",uncf.address());
   		  hr=ltx_register_class(utils_factory_t::GetCLSID(),&utils_factory_t::GetClassObject,L"lpc",uncf.address());
		  hr=ltx_register_class(utils_factory_t::GetCLSID(),&utils_factory_t::GetClassObject,L"old_style.external",uncf.address());


         hr=ltx_register_class(callback_factory_factory_t::GetCLSID(),&callback_factory_factory_t::GetClassObject,L"callback.factory",uncf.address());


				  hr=ltx_register_class(utils_factory_t::GetCLSID(),&utils_factory_t::GetClassObject,L"combin",uncf.address());

	  
	  
	  hr=ltx_register_class(CTestFactory2_t::GetCLSID(),&CTestFactory2_t::GetClassObject,L"CTestFactory",uncf.address());
	  hr=ltx_register_class(CTestFactory2_t::GetCLSID(),&CTestFactory2_t::GetClassObject,L"CoDispatch_test_t",uncf.address());
	  
	  hr=ltx_register_class(__uuidof(CoDispatch_test_t),0);

  }
} registration_setup;
*/


// ... test



extern "C" HRESULT ltx_register_class(CLSID clsid,class_object_pair_t::GetClassObject_t pClassObject,const wchar_t* alias,IUnknown** ppUnk,class_object_pair_t** pp_pair)
{
	HRESULT hr;
	hr=register_class_map_t<>::get_instance().register_moniker(clsid,pClassObject,alias,ppUnk, pp_pair);
	return hr;	
}



extern "C" HRESULT ltx_get_class(const wchar_t* moniker_str,IUnknown** ppUnk,class_object_pair_t** pp_pair)
{
	HRESULT hr;
		OLE_CHECK_hr(register_class_map_t<>::get_instance().get_moniker(moniker_str,ppUnk,false));
		if(pp_pair&&ppUnk) hr=(*ppUnk)->QueryInterface(GUID_NULL,(void**)pp_pair);
	return hr;	
}


extern "C" HRESULT ltxGetObjectEx(const wchar_t* moniker_str,REFIID riid,void** ppObj,IBindCtx *bctx)
{
	HRESULT hr;
	OLE_CHECK_PTR(ppObj);
	//if(SUCCEEDED(hr=super_factory_t::CoGetObject(bstr_t(moniker_str),riid,ppObj))||(REGDB_E_CLASSNOTREG!=hr))
	if(SUCCEEDED(hr=super_factory_t::CoGetObject(bstr_t(moniker_str),riid,ppObj,bctx)) )
		return hr;
	else return hr=::CoGetObject(bstr_t(moniker_str),0,riid,ppObj);

	/*
	smart_ptr_t<IUnknown> unk;
	class_object_pair_t*  pcop=0;
	hr=ltx_get_class(moniker_str,&unk.p,&pcop);
	
	if(FAILED(hr))
			return hr=::CoGetObject(bstr_t(moniker_str),0,riid,ppObj);
    smart_ptr_t<IParseDisplayName> pdn;
    OLE_CHECK_hr(pcop->GetClassObject(pcop->clsid,__uuidof(IParseDisplayName),pdn._ppQI()));
	smart_ptr_t<IMoniker> moniker;
	smart_ptr_t<IBindCtx> bctx;
	OLE_CHECK_hr(CreateBindCtx(0,&bctx.p));	
	ULONG  lp=0;
	OLE_CHECK_hr(pdn->ParseDisplayName(bctx,bstr_t(moniker_str),&lp,&moniker.p));	
	hr=moniker->BindToObject(bctx,0,riid,ppObj);
	return hr;	
*/

	
}

extern "C" HRESULT ltxGetObject(const wchar_t* moniker_str,REFIID riid,void** ppObj)
{
	return ltxGetObjectEx(moniker_str, riid, ppObj);
}


extern "C" HRESULT bindObjectWithArgsEx(const wchar_t* moniker_str,REFIID riid,void** ppObj,bool freverse,int argc,VARIANT* argv)
{
	HRESULT hr;
	if(argc)
	{
		args_stack_t args(argc,argv,freverse);
		args.leftbound=1;
		smart_ptr_t<IBindCtx> bctx;
		OLE_CHECK_hr(CreateBindCtx(0,&bctx.p));
		ipc_utils::bstr_c_t<64> bstrc( LTX_BINDCONTEXT_PARAMS);
         OLE_CHECK_hr(bctx->RegisterObjectParam(bstrc,&args));
		 return  hr=ltxGetObjectEx(moniker_str,riid, ppObj,bctx);
	}
	else return  hr=ltxGetObject(moniker_str,riid, ppObj);
	
}
extern "C" HRESULT bindObjectWithArgs(const wchar_t* moniker_str,REFIID riid,void** ppObj,int argc,VARIANT* argv)
{
	//return bindObjectWithArgsEx(moniker_str,riid,ppObj,true, argc,argv);
	return bindObjectWithArgsEx(moniker_str,riid,ppObj,false, argc,argv);
}


//inline HRESULT get_moniker(const wchar_t* moniker_name,IUnknown** ppUnk=0,bool frelease=false)

STDAPI DllCanUnloadNow(void)
{
	return 0;
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	HRESULT hr;

	
    if(rclsid==__uuidof(CLSID_marshal_factory_t))	
		return hr=super_factory_t::CreateFactory(riid,ppv);



	if(rclsid==binder_factory_t::GetCLSID())
		return hr=binder_factory_t::CreateFactory(riid,ppv);

	if(rclsid==__uuidof(script_engine_factory_t))
		return hr=script_engine_factory_t::GetClassObject(rclsid,riid,ppv);

	hr=register_class_map_t<>::GetClassObject(rclsid,riid,ppv);
	
	return hr;
}


// DllRegisterServer - Adds entries to the system registry
inline HRESULT dll_register_server()
{
	HRESULT hr;

	   OLE_CHECK_hr(com_register_t<>::dll_class_register(__uuidof(script_engine::script_engine_factory_t),L"both",L"ltx.script"))
	   OLE_CHECK_hr(com_register_t<>::dll_class_register(__uuidof(CLSID_marshal_factory_t),L"both",L"ltx.bind"))
	   OLE_CHECK_hr(com_register_t<>::dll_class_register(binder_factory_t::GetCLSID(),L"both",L"ltx.moniker"));
	 //if(sizeof(void*)<8)  		 OLE_CHECK_hr(com_register_t<>::set_root_evironment_variable());

	return hr;
}

inline HRESULT dll_unregister_server()
{
	HRESULT hr=S_OK,t;	
	
	SUCCEEDED(t = com_register_t<>::dll_class_unregister(__uuidof(script_engine::script_engine_factory_t))) || (hr = t);
	SUCCEEDED(t = com_register_t<>::dll_class_unregister(__uuidof(CLSID_marshal_factory_t))) || (hr = t);
	SUCCEEDED(t = com_register_t<>::dll_class_unregister(__uuidof(script_engine::script_engine_factory_t))) || (hr = t);

	return hr;
}

extern "C" void __stdcall  set_root_evironment_variableW
(HWND hwnd, HINSTANCE hinst,LPWSTR lpszCmdLine,int nCmdShow)
{
   //
	com_register_t<>::set_root_evironment_variable(lpszCmdLine);
}

STDAPI DllRegisterServer(void)
{
	return dll_register_server();
}

// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	return dll_unregister_server();
}

// DllInstall - Adds/Removes entries to the system registry per user
//              per machine.	
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	HRESULT hr = E_FAIL;
	
	return hr;
}

extern "C" int ltx_stop_all_apartment(DWORD timeout)
{
	return apartment_list_t<>::get_instance().stop_all(timeout);
}


inline HRESULT run_server(HWND hwnd,LPWSTR lpszCmdLine,int nCmdShow)
{
	HRESULT hr;
	COMInitF_t ci;

	smart_ptr_t<IDispatch> external_obj;
	OLE_CHECK_hr(super_factory_t::CoGetObject(bstr_t(L"ltx.bind:external"),__uuidof(IDispatch),external_obj._ppQI()));

	smart_ptr_t<IDispatch> ec_locker,binder;

	{
		argv_cmdline<wchar_t> argscmd(lpszCmdLine);
	  HWND hconswin=(HWND) argscmd[L"console.hwnd"].def<int>(0);
	  process_com_t::get_instance().console_hwin(1,hconswin);
	}


	OLE_CHECK_hr(super_factory_t::CoGetObject(bstr_t(L"ltx.bind:ec_locker:  global=3 ;"),__uuidof(IDispatch),ec_locker._ppQI()));
	OLE_CHECK_hr(super_factory_t::CoGetObject(bstr_t(L"ltx.bind:"),__uuidof(IDispatch),binder._ppQI()));

	{
	 variant_t ve;		 		 
	            disp_call_t external_args(external_obj);
				OLE_CHECK_hr(external_args(int(0)));
				bstr_t parent_external_obj_ref=external_args.result;
				SetEnvironmentVariableW(LTX_PROCESS_OBJREF,(parent_external_obj_ref.length())?(wchar_t*)parent_external_obj_ref:(wchar_t*)0);          
				SetEnvironmentVariableW(L"_LTX_", (parent_external_obj_ref.length()) ? (wchar_t*)parent_external_obj_ref : (wchar_t*)0);
				disp_call_t external_pid(external_obj,"pid");
				OLE_CHECK_hr(external_pid());
				bstr_t pidstr=external_pid.result;
                SetEnvironmentVariableW(L"ltx.initiator.pid",pidstr); 

				//int ihwndc=(int) 






			//	 OLE_CHECK_hr(toVARIANT(binder.p,&ve));
	        //	 OLE_CHECK_hr(call_disp(external_obj,L"result",0,ve));
	   //MessageBoxW(0,L"??",L"Waiting Debugger",MB_OK|MB_SYSTEMMODAL|MB_ICONINFORMATION);

	     //    disp_call_t disp_call(external_obj,L"result");
	 //		 OLE_CHECK_hr(disp_call(binder.p));
	 		 OLE_CHECK_hr(disp_call_t(external_obj,L"result")(binder.p));

	  external_obj.Release();	
	}
	//
	int sw;
	for (;;)
	{
		disp_call_t dc(ec_locker, L"wait");
		OLE_CHECK_hr(dc(int(1000)));
		sw = dc.result;
		if (sw == WAIT_OBJECT_0)
			break;
	}
//	mta_singleton_t::stop_and_wait(5000);

	apartment_list_t<>::get_instance().stop_all(5000);
	

	//while(!loop()){};
    return hr;
}


LONG WINAPI jsUnhandledExceptionFilter( PEXCEPTION_POINTERS ExceptionInfo){

	TerminateProcess(GetCurrentProcess(),-1); 
	return 0;
};

extern "C" void __stdcall  runsrvW
(HWND hwnd, HINSTANCE hinst,LPWSTR lpszCmdLine,int nCmdShow)
{
	HRESULT hr;
	SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
	SetUnhandledExceptionFilter(&jsUnhandledExceptionFilter);
	hr=run_server(hwnd,lpszCmdLine,nCmdShow);
   ExitProcess(hr);

}


BOOL WINAPI HandlerRoutine(
						    DWORD dwCtrlType
						   )
{
	 FreeConsole();
	 return TRUE;
};


extern "C" void __stdcall  js_console_holderW
(HWND hwnd, HINSTANCE hinst,LPWSTR lpszCmdLine,int nCmdShow)
{
	HRESULT hr;
	//ShowWindow(hwnd,SW_SHOW);
      
    //SetConsoleCtrlHandler(&HandlerRoutine,TRUE);
	SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
	SetUnhandledExceptionFilter(&jsUnhandledExceptionFilter);

	hr=ltx_js_server_t<>::console_holder(lpszCmdLine);
	//if(hr) FatalAppExit(0,error_msg(hr));
	ExitProcess(hr);
	//ExitProcess(run_server(lpszCmdLine,nCmdShow));

}


extern "C" void __stdcall  jsW
(HWND hwnd, HINSTANCE hinst,LPWSTR lpszCmdLine,int nCmdShow)
{
	HRESULT hr=S_OK;
	//ShowWindow(hwnd,SW_SHOW);
	DWORD em=GetErrorMode();
	int mdbg=0;	
    wchar_t* pdbg;
	if(pdbg=_wgetenv(L"ltx.script.mode"))
		       mdbg=_wtoi(pdbg);

	//	AttachConsole(ATTACH_PARENT_PROCESS);
	//
	startup_console_attr(L"console.GUI",L"services.console.",1);

	int nc;
	wchar_t** ppv=CommandLineToArgvW(lpszCmdLine,&nc);
	
	if(nc>0)
	{
		
		//moniker_parser_t<wchar_t> mp;
		//bstr_t scf=L"load_string('"+bstr_t(ppv[0])+L"')";        

		//ipc_utils::com_apartments::script_engine::js_t js(L"global.require.add_to_module_path(arguments[0]+'/../');load_string(arguments[0])",0x3&mdbg);
		ipc_utils::com_apartments::script_engine::js_t js(
			js_text(
			   require;
			   global.__script_triplet__=os.fs_triplet(arguments[0]);
               global.require.add_to_module_path(global.__script_triplet__[0])
			   ;load_string(arguments[0]);
			),0x3&mdbg);
		if(SUCCEEDED(hr=js(ppv[0])))
		{
            js.script_text.Attach(js.result.Detach().bstrVal);
			std::vector<VARIANT> vv(nc);
			VARIANT *pv=&vv[0];
			for(int n=0;n<nc;n++)
			{
				pv[n].vt=VT_BSTR;
				pv[n].bstrVal=bstr_t(ppv[n]).Detach();
			}
			hr=js.invoke(nc,(variant_t*)pv);

		};
		if(FAILED(hr))
		{
			 
			 //s_parser_t<wchar_t> sp;
			 //wchar_t* p=sp.printf(L"");
			
			EXCEPINFO& exinfo=js.engine.exceptinfo;
			s_parser_t<wchar_t> sp; 
			wchar_t* p;
			 if(exinfo.scode)
			 {  
				 
				 p=sp.printf(L"error=%s[%08x]\nSource: %s\nDescription: %s\n",(wchar_t*)error_msg(exinfo.scode),exinfo.scode
					 ,(wchar_t*)exinfo.bstrSource
					 ,(wchar_t*)exinfo.bstrDescription);						   
				 //sp.printf(0x04,L"Source: %s\n",(wchar_t*)exinfo.bstrSource);
				 //sp.printf(0x04,L"Description: %s\n",(wchar_t*)exinfo.bstrDescription);				   

			 }
			 else 
			 {
				 p=sp.printf(L"api error=%s[%08x]\n",(wchar_t*)error_msg(hr),hr);						   
			 }

			 //0x3&mdbg
			   if(mdbg&0x4)
				    FatalAppExitW(0,p);
               console_shared_data_t<wchar_t,0>().printf(0x04,p);
			   
			 
			
		}
		
	}
	ExitProcess(hr);

}

//com_UUID_t<> ccoo;

extern "C" void __stdcall  js_consoleW
(HWND hwnd, HINSTANCE hinst,LPWSTR lpszCmdLine,int nCmdShow)
{
	//HRESULT hr;
	//hr=js_t("alert($$[0]+':'+$$[1])")(1,2);

	//com_UUID_t<> ccoo;
	//HINSTANCE h = ipc_utils::get_hinstance();
	//HINSTANCE h2 = ipc_utils::get_hinstance(GetModuleHandle(NULL));
   ltx_js_console_t<> js_console(lpszCmdLine);
	ExitProcess(js_console.hr);
	//ExitProcess(run_server(lpszCmdLine,nCmdShow));

}

extern "C" void __stdcall  SwitchDesktopW
(HWND hwnd, HINSTANCE hinst,LPWSTR lpszCmdLine,int nCmdShow)
{
	HRESULT hr;
	
	wchar_t *pw;

	moniker_parser_t<wchar_t> mp;

	//pw=mp.get_filemap_name(&SwitchDesktopW,&hr);
	//return;

	console_input_t<> ci;
	console_output_t<> co;


	console_info_t::simple_set_alpha(85);
	ltx_js_console_t<>::set_icon2();

	co.attr(0x70);




	
    desktop_switch_t<> dsw;
	SetConsoleTitleW(v_buf<wchar_t>().printf(L"Active Desktop: \"%s\"",dsw.name));
	for(;;){

	DWORD a;
	system("cls");
	int cb=dsw.desktops.update();
	for(int i=1;i<=cb;i++){
		wchar_t *p=dsw.desktops[i];
		if(dsw.is_active(p))
		{
			a=co.attr(0x72);
			co.printf(L"[%d] %s\n",i,p);
			co.attr(a);

		}
		else co.printf(L"[%d] %s\n",i,p);

	}
     
	 co.printf(L"\n"); 
	 a=co.attr(0x73);
	 co.printf(L"q[uit] or select desktop \nnumber [%d-%d]:>",1,cb);
	 co.attr(0x07);
	 //co.printf(L">"); 
	 pw=ci.getline<wchar_t>(1);
	 co.attr(a);
	 if(pw) {

		 if((*pw==L'q')||(*pw==L'Q')||(*pw==L'é')||(*pw==L'É'))
			 return ;
		 int i=_wtoi(pw);
		 wchar_t *pn=dsw.desktops[i];
		 if(pn){
		   if(hr=dsw.switch_to(pn))
			 return ;
		 }
	 }

	}




	

	 //desktop_switch_t<>::desktop_keep_t dk;

	

//	win_desktops_t<> wd;
	
//	pw=ci.getline<wchar_t>(1);







/*
	HANDLE   hin=CreateFileW(L"CONIN$",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	//HANDLE hin=GetStdHandle(STD_INPUT_HANDLE);
	if(GetConsoleMode(hin,&dwm))
	{
		SetConsoleMode(hin,dwm|ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT);
	    f=ReadConsoleA(hin,buf,80,&(cbout=0),NULL);
		if(f) buf[cbout]=0;
		f=ReadConsoleW(hin,wbuf,80,&(cbout=0),NULL);
		if(f) wbuf[cbout]=0;
	}
	*/


	//int p=getchar();
	//p=getch();

}