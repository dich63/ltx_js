#pragma once
#include <windows.h>
/*
#include "./lib/lz4frame_static.h"

extern "C" size_t  lz4_compressBound(size_t isize);
extern "C" size_t  lz4_compress(const void* psrc,void* pdest, size_t inputSize, size_t maxOutputSize);
extern "C" size_t  lz4_decompress(const void* psrc,void* pdest, size_t inputSize, size_t maxOutputSize);
*/

struct lz4_helper_t
{
	 HRESULT hr;
	 HMODULE hlib;
     size_t  (*compressBound)(size_t isize);
	 size_t  (*compress)(const void* psrc,void* pdest, size_t inputSize, size_t maxOutputSize);
	 size_t  (*decompress)(const void* psrc,void* pdest, size_t inputSize, size_t maxOutputSize);
	 lz4_helper_t(wchar_t* libpath=L"lz4_fast.dll"):hr(0)
	 {

         if(hlib=LoadLibraryW(libpath)){
			 if(!set_dll_proc((void**)&compressBound, "lz4_compressBound")) {set_error() ;return;} ;
			 if(!set_dll_proc((void**)&compress, "lz4_compress")) {set_error() ;return;} ;
			 if(!set_dll_proc((void**)&decompress, "lz4_decompress")) {set_error() ;return;} ;
			 
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
