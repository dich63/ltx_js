#pragma once

#ifdef _WIN32
#include <windows.h>
#else
typedef  long HRESULT;
#define E_NOTIMPL                        HRESULT(0x80004001L)
#define E_POINTER                        HRESULT(0x80004003L)
#define E_OUTOFMEMORY                    HRESULT(0x8007000EL)
#define S_OK HRESULT(0x0L)
#define S_FALSE HRESULT(0x1L)
#endif

#include "icontext.h"

#define CHECK_PTR(p) {if(!(p)) return E_POINTER;}
#define CHECK_PTR_ATTACH(p,v) {if(!(p)){ return E_POINTER;} else { *(p)=(v); return 0;}}
#define CHECK_PTR_ATTACH_CHECK_MEM(p,v) {if(!(p)){ return E_POINTER;} else { auto __t=v;if(__t){*(p)=(__t); return 0;}else { return E_OUTOFMEMORY;}}}
#define PTR_SET(p,v) {if(!(p)){*(p)=(v);}}
#define CHECK_PTR_SET CHECK_PTR_ATTACH
#define CHECK_PTR_SET_CHECK_MEM CHECK_PTR_ATTACH_CHECK_MEM


struct context_holder_t{
		
		long  (*addref)(void* handle);
		long  (*release)(void* handle);
		
		long  (*unwrap_context)(void* handle,void** pcontext);
		long  (*wrap_context)(void* context,void (* ondestroy )(void* context ),void** phandle);

		long  (*link)(void* handle,void* child_handle);
		long  (*clear_links)(void* handle);
		long  (*global_ref_count)();
		long (*unlink)(void* handle, void* child_handle);

		long  (*wrap_context_ex)(void* ptr, void(*ondestroy)(void* context), char* puuid, void** phandle);
		long  (*create_handle_ex)(void* ptr, void(*ondestroy)(void* context), char* puuid, void** phandle);
        

		long  (*get_context)(void* handle,void** pcontext);

		long  (*create_handle)(void* context,void (* ondestroy )(void* context ),void** phandle);
		
		long(*tss_link_context)(void* handle);
		long(*tss_unlink_context)(void* handle);
		long(*tss_clear_links)();

		long(*tss_onexit)(void(*ondestroy)(void* p), void* p);
		long(*load_lib_path)(wchar_t* plib, void** phandle);
		long(*load_lib_path_env)(wchar_t* penv, wchar_t* plib, void** phandle);
		

		//long  (*set_tss_lifetime)(void* handle);
		
		
};

#include <stdlib.h>
#ifndef _WIN32
typedef long HRESULT


#endif
#ifdef __cplusplus
#ifdef _WIN32
#include <windows.h>

#define CHECK_VOID(f) { if(!(f)) { hr=HRESULT_FROM_WIN32(GetLastError());return; } }
#define SET_PROC_ADDRESS(h,a)  libloader_t::set_dll_proc(h,(void**)&a,#a)
#define CHECK_VOID_PROC_ADDRESS(h,a)  CHECK_VOID(SET_PROC_ADDRESS(h,a))
#include <windows.h>
#include <string>


namespace inner_utils{

template <class T, class T2>
inline T make_detach(T& dst, T2 n) {
	T t = dst;
	dst = (T)n;
	return t;
};

template <class T>
inline T make_detach(T& dst) {
	T t = dst;
	dst = T();
	return t;
};

};

#define SET_PROC_ADDRESS(h,a)  libloader_t::set_dll_proc(h,(void**)&a,#a)

struct libloader_t {


	struct dll_dir_add_t {
		
		dll_dir_add_t()  {}

		HRESULT init(const wchar_t* n) {
			if (n) {
				std::wstring s = n;
				s += L"/../";
				
				if (!SetDllDirectoryW(s.c_str()))
					return GetLastError();
				return S_OK;

			}
			return E_POINTER;
		};

		~dll_dir_add_t() {
			SetDllDirectoryW(NULL);
		};
	};
	/*
	struct dll_dir_add_t {
		DLL_DIRECTORY_COOKIE cookie;
		dll_dir_add_t() :cookie(0) {}

		HRESULT init(const wchar_t* n)  {
			if (n) {
				std::wstring s = n;
				s += L"/../";
				::SetDefaultDllDirectories(0x00001e00);
				cookie = ::AddDllDirectory(s.c_str());
				if (!cookie)
					return GetLastError();
				return S_OK;

			}
			return E_POINTER;
		};

		~dll_dir_add_t() {
			if (cookie)
				::RemoveDllDirectory(cookie);
		};
	};
	*/

	libloader_t(const wchar_t* nenv, const wchar_t* name=NULL):hlib(NULL) {

		size_t s;
		wchar_t buffer[2048];// , buffer2[2048];
		wchar_t* n=(wchar_t*)name,*fn;

		if (nenv) {
			if ((0==_wgetenv_s(&s, buffer, 2047, nenv))&&s) {
				n = buffer;
			};
		}

		if (S_OK==(hr = dda.init(n))) {
			if (!(hlib = LoadLibraryW(n))) {
				hr = GetLastError();
			}
		};


	}

	
	inline operator HRESULT() {
		return hr;
	}

	inline operator HMODULE() {
		return hlib;
	}

	inline 	static void* set_dll_proc(HMODULE hlib, void** fp, char* name) {
		return (hlib) ? (*fp = (void*)GetProcAddress(hlib, name)) : NULL;
	}

	inline 	void* set_dll_proc( void** fp, char* name) {

		return set_dll_proc( hlib, fp, name);
	}
	
	dll_dir_add_t dda;
	HRESULT hr;
	HMODULE hlib;

};



struct context_helper_t {
	context_holder_t* pch;
	HRESULT hr;
	
	context_holder_t* (*get_context_utils)();

	

	inline operator bool() {
		return hr==0;
	}

	inline operator HRESULT() {
		return hr;
	}
	inline operator context_holder_t*() {
		return pch;
	}
	inline context_holder_t* operator->() {
		return pch;
	}
	context_helper_t(const wchar_t* env= L"context_wrapper", const wchar_t*  lib = NULL)
		:hr(S_OK),pch(NULL), get_context_utils( NULL){		
		//
		/*
		HMODULE hlib;
		wchar_t* n = (wchar_t*)lib,*p=0;
		wchar_t buffer[2048];

		if (!n) {
			//n = _wgetenv(L"context_wrapper");			
			size_t s;
			if (!_wgetenv_s(&s, buffer, 2047, L"context_wrapper"))
				n=buffer;

		}

		CHECK_VOID(hlib = LoadLibraryW(n));
		//*/

		///*
		libloader_t hlib(env, lib);

		if (hr = hlib)
			return;
		//*/

		CHECK_VOID_PROC_ADDRESS(hlib, get_context_utils);
		if (!(pch = get_context_utils()))
			hr = E_POINTER;
	}

};



template <class T>
struct context_base_T {

	static long s_onexit(void* p){
		if(p)
			delete (T*)p;
	}

	context_base_T(context_helper_t *_cp=NULL) :cp(_cp), hweak_ref(NULL){
		if (cp)
			cp->wrap_context((T*)this, &s_onexit, &hweak_ref);
	};

	long release() {

		return (cp)?cp->release(hweak_ref):-1;
	}

	template <class O>
	long link(O* p) {
		return (cp&&p) ? cp->link(hweak_ref,p->hweak_ref):-1;
	}

	context_helper_t* cp;
	void* hweak_ref;
};


template <class I,class T1, class T2>
inline bool cmp_as(const T1* t1, const T1* t2) {
	return *((const I*)t1) == *((const I*)t2);
 }

template <class I, class T1>
inline bool cmp_as(const T1* t1, const I& t2) {
	return *((const I*)t1) == t2;
}

template <class I, class T1>
inline bool cmp_as(const T1& t1, const I& t2) {
	return *((const I*)&t1) == t2;
}


#endif
#endif