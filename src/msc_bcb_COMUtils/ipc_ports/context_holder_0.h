#pragma once
//#include "context_holder.h"
#ifdef _WIN32
#include <windows.h>
#endif

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

		long  (*set_tss_lifetime)(void* handle);
		
		
};

#include <stdlib.h>
#ifndef _WIN32
typedef long HRESULT


#endif
#ifdef __cplusplus
#ifdef _WIN32
#include <windows.h>

#define CHECK_VOID(f) { if(!(f)) { hr=HRESULT_FROM_WIN32(GetLastError());return; } }
#define SET_PROC_ADDRESS(h,a)  context_helper_t::set_dll_proc(h,(void**)&a,#a)
#define CHECK_VOID_PROC_ADDRESS(h,a)  CHECK_VOID(SET_PROC_ADDRESS(h,a))
  

struct context_helper_t {
	context_holder_t* pch;
	HRESULT hr;

	context_holder_t* (*get_context_utils)();

	inline 	static void* set_dll_proc(HMODULE hlib, void** fp, char* name)	{
		return (hlib) ? (*fp = (void*)GetProcAddress(hlib, name)) : NULL;
	}

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
	context_helper_t(const wchar_t* lib=NULL)
		:hr(S_OK),pch(NULL), get_context_utils( NULL){		
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