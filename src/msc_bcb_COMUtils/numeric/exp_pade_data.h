#pragma once
#include <windows.h>

struct exp_pade_helper_t
{
	typedef struct complex_t
	{
		double re;
		double im;

	} complex_t ,*pcomplex_t;

	 HRESULT hr;
	 HMODULE hlib;
     
	 long  (*poles_res_half)(int n,int m, void* p_poles,void* p_res );
	 long  (*poles_res)(int n,int m, void* p_poles,void* p_res );
	 exp_pade_helper_t(wchar_t* libpath=L"exp_pade_data.dll"):hr(0)
	 {

         if(hlib=LoadLibraryW(libpath)){
			 if(!set_dll_proc((void**)&poles_res_half, "pade_poles_res_half")) {set_error() ;return;} ;
			 if(!set_dll_proc((void**)&poles_res, "pade_poles_res")) {set_error() ;return;} ;			 
			 
		 }
		 else hr=GetLastError();
	 }

	inline operator bool(){
		return hr==S_OK; 
	}
  private:
	  inline void set_error(){hr=GetLastError() ;}
	 inline void* set_dll_proc(void** fp,char* name)
	 {
		 return (hlib)?(*fp=(void*)GetProcAddress(hlib,name)):NULL;
	 }

};
