#pragma once
#include <windows.h>

struct zlib_helper_t
{
	 HRESULT hr;
	 HMODULE hlib;
	 typedef UINT32 size_t;
     size_t  (*compressBound)(size_t size);
	 int  (*compress)(const void* pdest,size_t* maxdestSize,void* psrc, size_t inputSize,int level);
	 int  (*decompress)(const void* pdest,size_t* maxdestSize,void* psrc, size_t inputSize);

	 zlib_helper_t(wchar_t* libpath=L"zlib1.dll"):hr(0)
	 {

         if(hlib=LoadLibraryW(libpath)){
			 if(!set_dll_proc((void**)&compressBound, "compressBound")) {set_error() ;return;} ;
			 if(!set_dll_proc((void**)&compress, "compress2")) {set_error() ;return;} ;
			 if(!set_dll_proc((void**)&decompress, "uncompress")) {set_error() ;return;} ;
			 
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
