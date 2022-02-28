// ltx_hssh_w32.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "ipc_ports/ltx_js_hssh_w32.h"




static void set_icon(HINSTANCE hkl)
{
	HMODULE hke=GetModuleHandleW(L"kernel32.dll");
	HICON hico;
	hkl=ipc_utils::module_t<>::__getHinstance();

	BOOL (WINAPI *SetConsoleIcon)(HICON);
	if(SET_PROC_ADDRESS(hke,SetConsoleIcon))
	{

		if(hico=LoadIconW(hkl,L"#103"))
			SetConsoleIcon(hico);
	}	  


}

static void set_icon2()
{

	
	HICON hico;
	HWND hwin=GetConsoleWindow();
	LRESULT lr;
    HINSTANCE hkl=ipc_utils::module_t<>::__getHinstance();

	if( (hico=LoadIconW(hkl,L"#103"))){
		lr = SendMessage(hwin, WM_SETICON, ICON_BIG, LPARAM(hico));
		lr = SendMessage(hwin, WM_SETICON, ICON_SMALL,LPARAM(hico));
	}					  


}



BOOL WINAPI HandlerRoutine(
						                    DWORD dwCtrlType
						   )
{
	return TRUE;
	if(dwCtrlType==CTRL_SHUTDOWN_EVENT)
		return false;
	else return true;
}




extern "C" void __stdcall  startW
(HWND hwnd, HINSTANCE hinst,LPWSTR lpszCmdLine,int nCmdShow)
{
	HRESULT hr;



	argv_cmdline<wchar_t> argv(lpszCmdLine);
	//wchar_t* nsport=argv[L"port"];
	startup_console_attr();		 
	//set_icon(hinst);
	set_icon2();

	SetConsoleTitleW(v_buf<wchar_t>().printf(L"ltx_json_server_w32[%d] %s",GetCurrentProcessId(),lpszCmdLine));
	if(argv[L"forever"].def<bool>(0))
	{
      SetConsoleCtrlHandler( &HandlerRoutine,TRUE);
	}
	


	//void * hport=hssh.create_socket_portW(nsport,HSF_SRD_AUTO,&on_hss_js_accept,0);

	//hssh.wait_signaled(hport,-1);
	//ltx_hss_json_rpc_t<>::filelog().cat(argv[L"log"]);

	//ltx_hss_json_rpc_t<>(nsport,argv[L"oppc"].def<bool>(false)).wait();
	ltx_hss_json_rpc_t<>(argv).wait();

}

