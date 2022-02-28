#pragma once

#include "ltx_proc_table.h"
#include "os_utils.h"
#import "progid:WScript.Shell"  named_guids no_namespace raw_interfaces_only

template <class comvariant,class comstring>//=variant_t,=bstr_t>
struct test_process
{

    HRESULT (__stdcall *ltx_external_data_info)(BSTR* pinfo,BOOL f_cliboard_moniker);
	template<class N> 
	static	void* wild_cast(N n)
	{
		union
		{  
			N pn;
			void* p ;
		};
		pn=n;
		return p;
	};


	class COMInit
	{
	public:
		HRESULT m_hr;
		COMInit(DWORD dw= COINIT_MULTITHREADED /*COINIT_APARTMENTTHREADED*/ )
		{			m_hr=CoInitializeEx(NULL,dw); 	}
		~COMInit()
		{		if(SUCCEEDED(m_hr)) CoUninitialize();	}
		inline operator HRESULT() { return m_hr;}
	};

#define  VNOPARAM blank()
	inline static VARIANT blank()
	{
		return comvariant(DISP_E_PARAMNOTFOUND,VT_ERROR);
	}

	typedef test_process<comvariant,comstring> self_type;
	ltx_proc_table_helper ltx_tab;

  #define EXIT_CODE_FROM_HRESULT(hr) short(hr)

	void __stdcall on_exit_asyn_event(long err,VARIANT* pasynobj,VARIANT*pcookie,EXCEPINFO* pexi)
	{
		HRESULT hr;
		comvariant v;
		hr=ltx_tab->ltx_asyn_result(pasynobj,0,&v);
		wchar_t*p=(wchar_t*)"?";
		if(SUCCEEDED(hr))
		{ 

			hr=VariantChangeType(&v,&v,0,VT_BSTR);
          if(SUCCEEDED(hr))
			  p=v.bstrVal;
              _cwprintf(L"exit_result=%s\n",p);
		}
		_cwprintf(L"exit_asyn_event...exit_code=%d \n",EXIT_CODE_FROM_HRESULT(err));
	};

#define CALL_CONV __stdcall
#define CALL_CONV __cdecl

	void CALL_CONV on_sign(HRESULT hr )
	{

		_cwprintf(L"signal...errcode= %x \n",hr);
		Sleep(3000);
		::_exit(7);
	}


	inline void exitm(HRESULT hr=0)
	{
		printf("terminated..exitcode=%p\n press any key..\n",hr);
		getchar();
		::exit(0);
	}

	void run(int argc, wchar_t* argv[])
	{
		HRESULT hr;
		comstring binfo,exename;
		if(!ltx_tab)
		{
			printf("LifeTimeX not found!!!!!!!!");
			exitm(-1);

		}
/*
		void* pp=0;
		printf("ltx_external_data INTO\n");
            hr=ltx_tab->ltx_external_data((IDispatch**)&pp,0);
			pp=ltx_tab->ltx_external_data_info;
		printf("ltx_external_data_info=%p\n",pp);
      
		pp=GetProcAddress(ltx_tab.m_hm,"ltx_external_data_info");
        pp= SET_PROC_ADDRESS(ltx_tab.m_hm,ltx_external_data_info);
		printf("ltx_external_data_GPA=%p\n",&ltx_external_data_info);
        hr=ltx_external_data_info(NULL,0);
		printf("ltx_external_data_GPA into\n");
*/
		hr=ltx_tab->ltx_external_data_info(binfo.GetAddress(),0);
		if(FAILED(hr))
		{
			printf("start process id=%d as client\n press any key..",GetCurrentProcessId());
			getchar();


			wchar_t buf[1024];
			//GetModuleFileNameW(0,buf,1024);
			//exename=buf;


			COMInit cc;


			comvariant asynobject;
			//wsprintfW(buf,L"process:'%s'",argv[argc-1]);
			//
			wsprintfW(buf,L"process:{app:'%s',flags:CREATE_NEW_CONSOLE}",argv[argc-1]);

			//wsprintfW(buf,L"process:{app:'%s',flags:CREATE_NEW_CONSOLE,fullscreen:1}",argv[argc-1]);
			//  for debugging SERVER PART 
			//wsprintfW(buf,L"process:{app:'**/jmhook.exe',cmd:'%s',flags:CREATE_NEW_CONSOLE}",argv[argc-1]);
			comstring strmoniker= buf;
			void *proc=wild_cast(&self_type::on_exit_asyn_event);
			IDispatch* pD1=0,*pD2=0;



			hr=ltx_tab->ltx_create_asyn_object_mon(strmoniker,proc,this,&comvariant(strmoniker),&asynobject);
			if(FAILED(hr)) exitm(hr);  

			comvariant serverholder;
			if(1)
			{
				comvariant arg1=L"ARG1";
				comvariant arg2=777.333;
				VARIANT* pargs[2]={&arg1,&arg2};
				hr=ltx_tab->ltx_asyn_call_ex(&asynobject,2,pargs,&serverholder);
			}
			else
			{
				hr=ltx_tab->ltx_asyn_call_ex(&asynobject,0,0,&serverholder);
			}

			if(FAILED(hr)) exitm(hr);  
			comvariant srvres;
			printf("wait server\n");

			while(hr=ltx_tab->ltx_server_response(&serverholder,5000,&srvres))
			{
				if(FAILED(hr)) exitm(hr);  
				printf("."); 
			}

			IWshShell* wsh=0;
			if(SUCCEEDED(hr=VariantChangeType(&srvres,&srvres,0,VT_DISPATCH)))
			{
				hr=srvres.pdispVal->QueryInterface(__uuidof(IWshShell),(void**)&wsh);
				if(SUCCEEDED(hr))
				{
					int ii;
					hr=wsh->Popup(comstring(L" MessageBox in Server Side"),&VNOPARAM,&VNOPARAM,&VNOPARAM,&ii);

					wsh->Release();
				}

			}
			else       	 wprintf(L"\nres...%s\n",(wchar_t*)comstring(srvres));

			printf("press any key and server disconnected.. \n");
			getchar();
			VariantClear(&serverholder);

			exitm(0); 
		}
		else
		{
			printf("start process id=%d as server\n client info=%s\n",GetCurrentProcessId(),(char*)binfo);  

			comvariant res=L"My Response STRING";
			printf("press any key and server send Response .. \n");
			getchar();

			//hr=ltx_tab->ltx_external_data_link_attach(&res);
			//printf("server terminated. exitcode=%x\n",hr);
			//  exitm();
			//COMInit cc(COINIT_APARTMENTTHREADED);
			COMInit cc(COINIT_MULTITHREADED);
			if(1)
			{

			
			IDispatch *pdisp=0;
			hr=::CoCreateInstance(__uuidof(WshShell),NULL,CLSCTX_ALL,__uuidof(IDispatch),(void**)&pdisp) ;

			if(SUCCEEDED(hr))
			{
				res.Clear();
				res.vt=VT_DISPATCH;
				res.pdispVal=pdisp;
				;
			}
			}
			hr=ltx_tab->ltx_external_data_link_attach_asyn(&res,wild_cast(&self_type::on_sign),this);
			printf("wait disconect signal ... or   press any key\n");
			getchar(); 
			::exit(0);
		}

	}
};








/*
// test.exe
// or
// test.exe test.js
#include "stdafx.h"

#include "ltx_proc_table_test.hpp"
#include "ltx_script.h"
#include <comdef.h>

int wmain(int argc, wchar_t* argv[])
{

test_process<variant_t,bstr_t>().run(argc,argv);
return 0;
}

*/


/*

// test.js
// jscript server:

ltx_extern=function()
{
	try
	{
		return GetObject('ipc-ltx:extern');
	}
	catch(e)
	{
		// if not manifest or register!!
		var getenv=function (s){ return  (new ActiveXObject('WScript.Shell')).Environment("Process")(s);}
		
			return  GetObject(getenv(getenv("LTX_MONIKER_EXTERNAL_OBJECT")));
	}
}

ex=ltx_extern();
n=ex.arg.count;
s='from client argc='+n+' args:';
for(i=0;i<n;i++) s+=ex.arg[i]+" : ";
wsh=new ActiveXObject('WScript.Shell');
wsh.Popup(s,15,ex.info);

/// =====================
ex.link().attach=new ActiveXObject('WScript.Shell');
///=======================
ex.result='postmortem report : puk ';
*/
