// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ipc_ports/ltx_js.h"
#include "ipc_ports/ipc_utils_base.h"
#include "tls_dll.h"

//void* on_exit_thread(void* proc,void *ptr)
extern "C"  void* ltx_tss_onexit(void* proc,void *ptr)
{
	return tss_cleaner::on_exit_thread(proc,ptr);
}

/*
void*  (  __stdcall *AddDllDirectory)(  PCWSTR NewDirectory);
void* addme(HMODULE hModule)
{
	wchar_t buf[4096],*p;
	void* pcc=0;
	if(SET_PROC_ADDRESS(GetModuleHandleA("kernel32.dll"),AddDllDirectory))
	{
		if(!GetModuleFileNameW(hModule,buf,4096))
			return 0;
		p=wcsrchr(buf,L'\\');
		if(!p) return 0;
		*p=0;
		pcc=AddDllDirectory(buf);


	};
	return pcc;
}
*/

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	tss_cleaner::clean_tss_if_reason(ul_reason_for_call);
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:		  
		//addme(hModule);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
			//	ltx_stop_all_apartment(5000);
		break;
	}
	return TRUE;
}

