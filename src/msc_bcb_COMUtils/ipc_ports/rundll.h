#pragma once

#include <windows.h>
#include <string.h>

#define CHECK_ERROR(e)  if( (err=e) ) { return; };

struct rundll_t {


	inline static void* set_dll_proc(HINSTANCE h, void** fp, const char* name)
	{
		return (h) ? (*fp = (void*)GetProcAddress(h, name)) : NULL;
	};

	inline static wchar_t* lskip_space(wchar_t* p) {
		while ((*p) && (iswspace(*p))) ++p;
		return p;
	}

	inline static wchar_t* lskip_nospace(wchar_t* p) {
		while ((*p) && (!iswspace(*p))) ++p;
		return p;
	}

	rundll_t(const wchar_t* _cmdline):hlib(0),proc(0) {
		wchar_t*p;
		size_t len = wcslen(_cmdline);
		cmdline = (wchar_t*)::malloc(sizeof(wchar_t)*(len + 16));
		wcscpy(cmdline, _cmdline);
		CHECK_ERROR((p = wcspbrk(cmdline, L"\",")) ? 0 : 1);
		if (*p == L'"') {
			libname = ++p;
			CHECK_ERROR((p = wcspbrk(p, L"\"")) ? 0 : 1);
			*(p++) = 0;
			p = lskip_space(p);
			CHECK_ERROR((*p == L',') ? 0 : 1);
			++p;
		}
		else if (*p == L',') {
			libname = cmdline;
			*(p++) = 0;
		}
		
		procname = p = lskip_space(p);
		p = lskip_nospace(p);

		if (*p) {
			*(p++) = 0;
		}

		p = lskip_space(p);
		tail = p;

		init();
	}
	~rundll_t() {
		::free(cmdline);
	}

	inline void init() {

		

		CHECK_ERROR(err);
		CHECK_ERROR((hlib = ::LoadLibraryW(cmdline)) ? 0 : 1);
		//bstr_t pn = procname;
		{
			char pn[1024] = {};
			int cba2 = WideCharToMultiByte(CP_UTF8, 0, procname, -1, pn, 1024, 0, 0);
			CHECK_ERROR((cba2 > 1023) ? 1 : 0);
			strcat(pn, "W");
			CHECK_ERROR((set_dll_proc(hlib, (void**)&proc, pn)) ? 0 : 1);
		}

		
	}

	int operator()(HWND hwnd, HINSTANCE hinst, int nCmdShow= SW_SHOWDEFAULT) {

		if (err==0) {
			proc(hwnd, hinst,tail, nCmdShow);			
		}		
		return err;
	}



	wchar_t* cmdline;
	wchar_t* libname;
	wchar_t* procname;
	wchar_t* tail;


	void(__stdcall  *proc)	(HWND hwnd, HINSTANCE hinst, LPWSTR lpszCmdLine, int nCmdShow);
	HMODULE hlib;
	int err;
};

#undef CHECK_ERROR
