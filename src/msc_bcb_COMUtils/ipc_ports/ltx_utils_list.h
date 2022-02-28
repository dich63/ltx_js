#pragma once
// #include "ltx_utils_list.h"


#include "context_holder_0.h"

using namespace ipc_utils;
using namespace ipc_utils::com_apartments;
using namespace ipc_utils::com_apartments::script_engine;
using namespace ltx_helper;

#include "lz4_fast.h"
#include "zlib_helper.h"
#include "ltx_js_console.h"
#include "ltx_job_utils.h"
#include "ipc_utils.h"
#include "ltx_factories.h"
#include "rm_call.h"
#include "window_tools.h"
#include "dispid_find.h"
#include <intrin.h>
#include <stdlib.h>
#include <psapi.h> 

#include "ipc_ports/com_script_engine.h" 

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
void* ppcu=addme(argv_ini<wchar_t>::get_hinstance());
*/



lz4_helper_t lz4_helper(moniker_parser_t<wchar_t>().safe_full_filename(L"**/lz4_fast.dll"));
zlib_helper_t zlib_helper(moniker_parser_t<wchar_t>().safe_full_filename(L"**/zlib1.dll"));


struct _mm_helper_t {

	HRESULT hr;

	ipc_utils::smart_ptr_t<i_mm_region_ptr> mm;
	_mm_helper_t(IUnknown* punk) :hr(E_POINTER)
	{
		if (punk)
			hr = punk->QueryInterface(__uuidof(i_mm_region_ptr), mm._ppQI());

	}
	operator HRESULT() { return hr; }
	i_mm_region_ptr* operator ->()
	{
		return mm.p;
	}
};


inline DWORD get_protect_memory(void* p) {

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	bool;
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(p, &mbi, si.dwPageSize))
		return mbi.Protect;
	else
		return -1;
}

extern "C"  HRESULT __cdecl  ltx_get_register_objref(const wchar_t* name, VARIANT* pmoniker) {
	HRESULT hr;

	ipc_utils::com_apartments::script_engine::js_t js(L"require('register').getROM(arguments[0])");

	OLE_CHECK_hr(js(bstr_t(name)));

	variant_t& result = js.result;

	if ((result.vt == VT_BSTR) && SysStringLen(result.bstrVal)) {
		if (pmoniker) {
			*pmoniker = result.Detach();
		}
		hr = S_OK;
	}
	else {
		hr = MK_E_SYNTAX;
	}



	return hr;
}

extern "C"  HRESULT __cdecl  ltx_create_resource_handle_ex(void* pcontext, void* info_proc, void* ondestroy, IUnknown** ppunk)
{

	HRESULT hr;
	OLE_CHECK_PTR(ppunk);
	DWORD pmm, flags = FLAG_CBD_FTM;
	const DWORD pmexec = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

	//ipc_utils::com_apartments::script_engine::callback_dispatch_factory_t::;
	//callback_factory_factory_t::create_iaggregator()
	if (info_proc && ((pmm = get_protect_memory(info_proc)) >= 0) && ((pmm&pmexec) == 0)) {
		flags |= FLAG_CBD_INFO;
	}

	VARIANT res = {};
	ipc_utils::smart_ptr_t<ICallbackDispatchFactory> CDF;
	ipc_utils::smart_ptr_t<IClassFactory> CF;
	//super_factory_t::CreateFactory()
	OLE_CHECK_hr(callback_factory_factory_t::CreateFactory(__uuidof(IClassFactory), CF._ppQI()));
	OLE_CHECK_hr(CF->CreateInstance(NULL, __uuidof(ICallbackDispatchFactory), CDF._ppQI()));
	//OLE_CHECK_hr(CF.QueryInterface(CDF._address()));


	//OLE_CHECK_hr(ltxGetObject(L"ltx.bind:callback.factory",__uuidof(ICallbackDispatchFactory),CDF._ppQI()));
	if (SUCCEEDED(hr))
	{


	}
	OLE_CHECK_hr(CDF->CreateInstanceV(FLAG_MASK_PUT | (flags << 16), info_proc, pcontext, (void*)ondestroy, &res));
	//RefCounterGlobal_t<>::get().AddRef();

	*ppunk = res.punkVal;
	return hr;
}

extern "C"  HRESULT __cdecl  ltx_create_resource_handle(void* pcontext, void* ondestroy, IUnknown** ppunk)
{
	return ltx_create_resource_handle_ex(pcontext, NULL, ondestroy, ppunk);
}



extern "C"  long __cdecl  ltx_release(IUnknown* punk)
{
	if (!punk) return -1;
	//RefCounterGlobal_t<>::get().Release();
	return  punk->Release();
}

extern "C"  long __cdecl  ltx_addref(IUnknown* punk)
{
	if (!punk) return -1;
	//RefCounterGlobal_t<>::get().AddRef();
	return  punk->AddRef();
}

extern "C"  long __cdecl  ltx_global_ref_count() {

	return RefCounterGlobal_t<>::get().RefCount();
}
extern "C"  HRESULT __cdecl  ltx_get_context(IUnknown* punk, void** ppctx)
{
	HRESULT hr;
	OLE_CHECK_PTR(punk&&ppctx);
	ipc_utils::smart_ptr_t<i_context> context;
	OLE_CHECK_hr(punk->QueryInterface(__uuidof(i_context), context._ppQI()));
	return hr = context->GetContext(ppctx);

}

extern "C"  HRESULT __cdecl  ltx_push_cache(IUnknown* punk, IUnknown* pvalue)
{
	HRESULT hr;
	OLE_CHECK_PTR(punk&&pvalue);
	VARIANT v = { VT_UNKNOWN };
	v.punkVal = pvalue;
	ipc_utils::smart_ptr_t<i_context_with_cache> context;
	OLE_CHECK_hr(punk->QueryInterface(__uuidof(i_context_with_cache), context._ppQI()));
	return hr = context->PushCacheValue(v);

}

extern "C"  HRESULT __cdecl  ltx_clear_cache(IUnknown* punk)
{
	HRESULT hr;
	OLE_CHECK_PTR(punk);
	ipc_utils::smart_ptr_t<i_context_with_cache> context;
	OLE_CHECK_hr(punk->QueryInterface(__uuidof(i_context_with_cache), context._ppQI()));
	return hr = context->ClearCache();

}



extern "C"  HRESULT __cdecl  ltx_mm_Lock(IUnknown* punk)
{
	HRESULT hr;
	_mm_helper_t mm(punk);
	OLE_CHECK_hr(mm);
	return hr = mm->Lock();

};

extern "C"  HRESULT __cdecl  ltx_mm_Unlock(IUnknown* punk)
{
	HRESULT hr;
	_mm_helper_t mm(punk);
	OLE_CHECK_hr(mm);
	return hr = mm->Unlock();

};



extern "C"  HRESULT __cdecl  ltx_mm_DataInfo(IUnknown* punk, VARTYPE* pvt, INT64* pCount = 0, long* pElementSize = 0)
{

	HRESULT hr;
	_mm_helper_t mm(punk);
	OLE_CHECK_hr(mm);
	return hr = mm->GetElementInfo(pvt, pCount, pElementSize);

};

extern "C"  HRESULT __cdecl  ltx_mm_ByteLength(IUnknown* punk, INT64* pByteLength = 0)
{
	HRESULT hr;
	_mm_helper_t mm(punk);
	OLE_CHECK_hr(mm);
	return hr = mm->GetTotalSize(pByteLength);

};


extern "C"  HRESULT __cdecl  ltx_mm_GetPtr(IUnknown* punk, void** pptr, INT64* pByteLength)
{
	HRESULT hr;
	_mm_helper_t mm(punk);
	OLE_CHECK_hr(mm);
	return hr = mm->GetPtr(pptr, pByteLength);
};


extern "C"  HRESULT __cdecl  ltx_mm_CommitRegionPtr(IUnknown* punk, INT64 ByteOffset, INT64 ByteLength, void** pptr)
{
	HRESULT hr;
	_mm_helper_t mm(punk);
	OLE_CHECK_hr(mm);
	return hr = mm->CommitRegionPtr(ByteOffset, ByteLength, pptr);

};

extern "C"  HRESULT __cdecl  ltx_mm_DecommitRegionPtr(IUnknown* punk, void* ptr)
{
	HRESULT hr;
	_mm_helper_t mm(punk);
	OLE_CHECK_hr(mm);
	return hr = mm->DecommitRegionPtr(ptr);

};

extern "C"  int __cdecl  ltx_apartment_type()
{
	return ipc_utils::COMInit_t::apartment_type();
}


extern "C"  HRESULT __cdecl  ltx_process_callback(void* pcalback, void* context, void* on_exit)
{
	HRESULT hr;
	smart_ptr_t<IDispatch> ec, external_obj, callback;
	DWORD exflags = (FLAG_CBD_EC) << 16;
	OLE_CHECK_hr(super_factory_t::CoGetObject(bstr_t(L"ltx.bind:external.connector:  global=3 ;"), __uuidof(IDispatch), ec._ppQI()));
	OLE_CHECK_hr(super_factory_t::CoGetObject(bstr_t(L"ltx.bind:external"), __uuidof(IDispatch), external_obj._ppQI()));
	OLE_CHECK_hr(calback_aggregator_t<IDispatch>::CreateInstance(exflags, pcalback, context, on_exit, __uuidof(IDispatch), callback._ppQI()));
	return hr = disp_call_t(external_obj, L"result")(callback.p);
}

extern "C"  HRESULT __cdecl  ltx_callback(IDispatch** ppdisp, void* pcalback, void* context, void* on_exit)
{
	HRESULT hr;
	DWORD exflags = (FLAG_CBD_EC) << 16;
	hr = calback_aggregator_t<IDispatch>::CreateInstance(exflags, pcalback, context, on_exit, __uuidof(IDispatch), (void**)ppdisp);
	return hr;
}


extern "C"  HRESULT __cdecl  ltx_script(IDispatch** ppdisp) {
	HRESULT hr;
	return hr = super_factory_t::CoGetObject(bstr_t(L"ltx.bind:script"), __uuidof(IDispatch), (void**)ppdisp);
}

extern "C"  HRESULT __cdecl  ltx_external_connector_wait(DWORD tio = INFINITE, DWORD* pwaitresult = 0)
{
	HRESULT hr;
	ipc_utils::smart_ptr_t<IDispatch> ec;
	OLE_CHECK_hr(super_factory_t::CoGetObject(bstr_t(L"ltx.bind:external.connector:  global=3 ;"), __uuidof(IDispatch), ec._ppQI()));
	if (tio != INFINITE)
	{
		disp_call_t locker(ec, L"wait");
		OLE_CHECK_hr(locker(int(tio)));
		if (pwaitresult) {
			VARIANT r = {};
			OLE_CHECK_hr(VariantChangeType(&r, &locker.result, 0, VT_I4));
			*pwaitresult = r.intVal;
		}
	}
	else
		OLE_CHECK_hr(disp_call_t(ec, L"msgwait")());

	return S_OK;
}

extern "C"  HRESULT __cdecl  ltx_process_callback_loop(void* pcalback, void* context, void* on_exit)
{
	HRESULT hr;
	OLE_CHECK_hr(ltx_process_callback(pcalback, context, on_exit));
	hr = ltx_external_connector_wait();
	return hr;
}

extern "C"  void __cdecl  ltx_raise_exception(wchar_t* smessage, wchar_t* ssource = 0)
{
	ipc_utils::safe_caller_t<>::__raise_exception(smessage, ssource);
}

extern "C"  void __cdecl  ltx_fill_exception(pcallback_context_arguments_t pcca, wchar_t* smessage, wchar_t* ssource = 0)
{
	ipc_utils::safe_caller_t<>::_fill_ExceptInfo(pcca->pei, smessage, ssource);
	*(pcca->phr) = DISP_E_EXCEPTION;
}



//



extern "C"  void __cdecl  win_console
(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags)
{
	bool ff;
	arguments_t<variant_t> args(pcca);
	int cmd = args[0].def(-1);
	if (cmd == -1) { hr = E_INVALIDARG; return; }
	if (!(ff = is_console()))
		if (!AttachConsole(process_com_t::get_instance().console_pid()))
		{
			//hr = HRESULT_FROM_WIN32(GetLastError());
			//return;
		}

	switch (cmd)
	{
	case 0:
	{
		variant_cast_t<VT_BSTR> vc(args[1]);
		if (!vc) hr = E_INVALIDARG;
		else {
			int attr = args[2].def<int>(M_CC);

			//g_console.puts(vc->bstrVal,attr);
			class_initializer_T<console_shared_data_t<wchar_t>>().get().puts(vc->bstrVal, attr);


		}
	}
	break;
	case 1:
		result = int(is_console());
		break;
	case 3:	{
		//int f = args[1].def<int>(0);
		HWND h = (HWND)args[1].def<int>(0);
		result = int(process_com_t::get_instance().console_hwin(!!h, h));
	     }
		break;
	case 4:
		result = int(GetConsoleWindow());
		break;
	case 5:
		result = int(process_com_t::get_instance().console_pid());
		break;
	case 8:
		clrscr(true);
		break;
	case 16:
	{


		console_info_t ci;
		ci.set_alpha(args[1].def<double>(100));
		break;
	}
	case 31:
		ltx_js_console_t<>::set_icon();
		break;
	case 32:
		ltx_js_console_t<>::set_icon2();
		break;

	case 48: {

		console_input_t<> ci;
		result = ci.getline<wchar_t>(true);
	}
			 break;
	case 50: {
		bstr_t buf = args[1].def<bstr_t>(L"");
		if (buf.length()) {
			console_input_t<> ci;
			result=(int)ci.send_console_input(buf);
		}
	}
			 break;
	case 64: {
		bstr_t wcmd = args[1].def<bstr_t>(L"");
		if (wcmd.length()) {
			system((char*)wcmd);
		}
	}
			 break;
	case 128: {
		bstr_t w = args[1].def<bstr_t>();
		bool force = args[2].def <int>(0);

		if ((w.length()) && (force || IsDebuggerPresent())) {
			OutputDebugStringW((wchar_t*)w);
		}

	}
			  break;
	case 129: {
		bstr_t w = args[1].def<bstr_t>();


		if (w.length()) {
			OutputDebugStringW((wchar_t*)w);
		}

	}
			  break;


	case 444:
	{
		int sw = args[1].def<int>(1);
		HWND h = GetConsoleWindow();
		result = int(ShowWindow(h, sw));
	}
	break;
	case 888:
	{
		bstr_t wtext = args[1].def<bstr_t>(L"");
		HWND h = GetConsoleWindow();
		result = int(SendMessageW(h, WM_SETTEXT, 0, LPARAM((wchar_t*)wtext)));
	}
	break;

	}

	if (!ff) FreeConsole();


}

extern "C"  void __cdecl  ShowHTML
(variant_t* presult, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags)
{

	DWORD at = ipc_utils::COMInit_t::apartment_type();
	arguments_t<variant_t> arg(pcca);
	bstr_t url = arg[0].def<bstr_t>(L"about:blank");
	bstr_t opts = arg[1].def<bstr_t>(L"");
	VARIANT argin = arg[2];
	DWORD dwflags = arg[4].def<int>(HTMLDLG_MODAL | HTMLDLG_VERIFY | HTMLDLG_ALLOW_UNKNOWN_THREAD);
	SHOWHTMLDIALOGEXFN* pfnShowHTMLDialogEx;
	VARIANT hw = arg[3];
	variant_cast_t<VT_I8> vc(hw);
	HWND hwin = (vc) ? (HWND)vc->llVal : 0;
	HINSTANCE hinstMSHTML = LoadLibraryA("MSHTML.DLL");
	if (hinstMSHTML == NULL) {
		hr = HRESULT_FROM_WIN32(GetLastError());
		return;
	}


	if (!(pfnShowHTMLDialogEx = (SHOWHTMLDIALOGEXFN*)GetProcAddress(hinstMSHTML, "ShowHTMLDialogEx")))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return;
	}

	smart_ptr_t<IMoniker> URLMoniker;

	OLE_CHECK_VOID(hr = CreateURLMoniker(NULL, url, &URLMoniker.p));

	//hr=(*pfnShowHTMLDialogEx)(hwin,URLMoniker,dwflags,&argin,opts,presult);		
	hr = pfnShowHTMLDialogEx(hwin, URLMoniker, dwflags, &argin, opts, presult);

}



extern "C"  void __cdecl  GetDispId
(variant_t* presult, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags)
{


	arguments_t<variant_t> arg(pcca);

	smart_ptr_t<IDispatch> disp;

	if (FAILED(hr = arg[0].QueryInterface(&disp.p))) return;
	DISPID did;
	bstr_t bs = bstr_t(arg[1]);
	wchar_t* lpsz = bs;
	disp->GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&lpsz), 1, LOCALE_USER_DEFAULT, &did);
	if (SUCCEEDED(hr) && presult)
		*presult = variant_t(int(did));

}

template <int _VERSION = 0>
struct ltx_utils_t
{

	typedef arguments_t<variant_t> arguments_t;

	typedef i_mm_region_ptr::int64_t int64_t;

	/*
	struct handle_holder_t{
	handle_holder_t(HANDLE _h=0):h(_h){}
	~handle_holder_t(){
	ipc_utils::check_handle(h)&&CloseHandle(h);
	}


	inline operator HANDLE()
	{
	return h;
	}

	HANDLE h;
	};
	*/

	struct safearray_holder_t
	{
		SAFEARRAY* psa;
		VARTYPE vt;
		safearray_holder_t(int size, VARTYPE _vt = VT_VARIANT) :vt(_vt) {
			//psa=SafeArrayCreateVector(vt,0,size);

			SAFEARRAYBOUND sb[2] = { {size,0} };
			psa = SafeArrayCreate(vt, 1, sb);

		}
		~safearray_holder_t() { if (psa) SafeArrayDestroy(psa); }

		inline VARIANT detach()
		{
			VARIANT r = { vt | VT_ARRAY };
			r.parray = ipc_utils::make_detach(psa);
			return r;
		}


	};

	static	void __cdecl  sleepmsg
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {

		arguments_t a(pcca);

		int tio = a[0].def<int>(0);
		int mode = a[1].def<int>(0);
		if (mode == 1) Sleep(tio);
		else	ipc_utils::sleep_msg(tio);

	}

	static	void __cdecl  checkQueryInterface
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {

		result = false;
		try {
			HRESULT hrQI;
			arguments_t arg(pcca);
			bstr_t b = bstr_t(arg[1]);
			clsid_t clsid(b);
			if (SUCCEEDED(clsid.hr)) {
				if (SUCCEEDED(hrQI = arg[0].QueryInterface(clsid)))
					result = true;
			}
		}
		catch (...) {};


	}

	static	void __cdecl  variant_type
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {

		result = false;
		try {
			arguments_t arg(pcca);

			result = int(VARIANT(arg[0]).vt);

		}
		catch (...) {};


	}



	struct on_terminated_t
	{
		HRESULT hr;
		process_handle_t ph;
		process_handle_t child_ph;
		int ec;



		on_terminated_t(int pid, int child_pid, int excode = 0)
			:child_ph((child_pid == -1) ? GetCurrentProcessId() : child_pid)
			, ph((pid == -1) ? GetCurrentProcessId() : pid), ec(excode)
		{
			HANDLE h = ph.get_handle(SYNCHRONIZE);
			HANDLE h2 = child_ph.get_handle(PROCESS_TERMINATE);
			if (SUCCEEDED(hr = HRESULT_FROM_WIN32(ph.hr)))
				hr = HRESULT_FROM_WIN32(child_ph.hr);

		}

		inline void operator()()
		{
			HRESULT hr0;

			bool f = TerminateProcess(child_ph, ec);
			hr0 = GetLastError();

		}

		inline HRESULT start()
		{
			HRESULT hrt = hr;
			if (SUCCEEDED(hrt))
				hrt = pool_on_ko_signaled(ph, this);
			else delete this;
			return hrt;

		}

	};


	static	void __cdecl  kill_process_after_process
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {



		arguments_t arg(pcca);
		int pid = arg[0].def<int>(0), child_pid = arg[1].def<int>(0);
		int ec = arg[2].def<int>(0);
		if (!(pid&&child_pid)) arg.raise_error(L"invalid pid", L"ltx_js");

		on_terminated_t* onpt = new on_terminated_t(pid, child_pid);

		variant_t vcpid = (int)onpt->child_ph.tid;
		OLE_CHECK_VOID(arg.hr = onpt->start());
		result = vcpid;

	}

	static	void __cdecl  kill_process_by_id
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {



		arguments_t arg(pcca);
		int pid = arg[0].def<int>(0);
		int ec = arg[1].def<int>(0);
		if (!pid) arg.raise_error(L"invalid pid", L"ltx_js");

		process_handle_t ph((pid == -1) ? GetCurrentProcessId() : pid);
		arg.hr = HRESULT_FROM_WIN32(ph.terminate(ec));


		//result=int(VARIANT(arg[0]).vt);							


	}

	static	void __cdecl  find_process_window
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {


		HWND hwin;

		arguments_t arg(pcca);



		bstr_t wc = arg[0].def<bstr_t>();
		bstr_t wt = arg[1].def<bstr_t>();

		int pid = arg[2].def<int>(0);

		pid = (pid == -1) ? GetCurrentProcessId() : pid;

		FindProcessWindow fpw(wc, wt, pid);
		fpw.start();

		hwin = fpw.m_hwin;

		result = (int)hwin;
		hr = S_OK;

	}

	static	void __cdecl  find_process_top_window
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {


		HWND hwin;

		arguments_t arg(pcca);

		int pid = arg[0].def<int>(-1);

		hwin = window_tools::FindTopWindow((pid == -1) ? GetCurrentProcessId() : pid);

		result = (int)hwin;
		hr = S_OK;

	}


	static	void __cdecl  process_image_filename_by_id
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {



		arguments_t arg(pcca);
		int pid = arg[0].def<int>(-1);
		int fparent = arg[1].def<int>(0);
		DWORD dwflags = arg[2].def<int>(0);

		if (pid == -1)
			pid = GetCurrentProcessId();

		if (fparent) {
			pid = GetParentProcessId(pid);
		}


		process_handle_t ph(pid);

		OLE_CHECK_VOID_hr(ph.hr);

		wchar_t buf[2048];
		DWORD sz = 2047;
		//OLE_CHECK_VOID_hr_if(!::GetProcessImageFileNameW(ph,buf,2048),GetLastError());
		OLE_CHECK_VOID_hr_if(!::QueryFullProcessImageNameW(ph, dwflags, buf, &sz), GetLastError());
		result = bstr_t(buf);


		//result=int(VARIANT(arg[0]).vt);							


	};

	static	void __cdecl  parent_process_id
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {



		arguments_t arg(pcca);
		int pid = arg[0].def<int>(-1);
		pid = (pid == -1) ? GetCurrentProcessId() : pid;

		result = int(GetParentProcessId(pid));




	};

	static	void __cdecl  is_debug_present
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {



		arguments_t arg(pcca);

		int pid = arg[0].def<int>(-1);
		pid = (pid == -1) ? GetCurrentProcessId() : pid;

		remote_call::rp_IsDebugPresent_t is_debug(pid);

		OLE_CHECK_VOID_hr(is_debug.hr);

		result = (bool)is_debug();

	};


	struct on_process_terminated_t
	{
		HRESULT hr;
		process_handle_t ph;
		ipc_utils::smart_GIT_ptr_t<IDispatch> disp_cookie;

		on_process_terminated_t(int pid, IDispatch* pdisp) :ph(pid), disp_cookie(pdisp)
		{
			HANDLE h = ph.get_handle(SYNCHRONIZE);
			if (SUCCEEDED(hr = HRESULT_FROM_WIN32(ph.hr)) && (!disp_cookie))
				hr = E_FAIL;


		}

		inline void operator()()
		{
			HRESULT hr0;
			ipc_utils::COMInitF_t com;
			ipc_utils::smart_ptr_t<IDispatch> disp;
			hr0 = disp_cookie.unwrap(disp);
			VARIANT vpid = { VT_I4 };
			vpid.intVal = ph.tid;
			DISPPARAMS dp = { &vpid,0,1,0 };

			if (SUCCEEDED(hr0))
				//hr0=disp->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_METHOD,&dp,0,0,0);		
				//
				hr0 = disp->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &dp, 0, 0, 0);

		}

		inline HRESULT start()
		{
			HRESULT hrt = hr;
			if (SUCCEEDED(hrt))
				hrt = pool_on_ko_signaled(ph, this);
			else delete this;
			return hrt;

		}

	};


	static	void __cdecl  on_process_terminated
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {





		arguments_t arg(pcca);
		int pid = arg[0].def<int>(0);
		if (!pid) arg.raise_error(L"invalid pid", L"ltx_js");
		ipc_utils::smart_ptr_t<IDispatch> disp;
		OLE_CHECK_VOID(arg.hr = arg[1].QueryInterface(disp._address()));

		/*
		HRESULT hr0;
		DISPPARAMS dp={};
		hr0=disp->Invoke(DISPID_VALID, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_METHOD,&dp,0,0,0);
		*/

		on_process_terminated_t* onpt = new on_process_terminated_t(pid, disp);

		arg.hr = onpt->start();

	}


	//


	static	void __cdecl  safearray_sizes
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags)
	{
		result = false;
		arguments_t arg(pcca);
		VARIANT v = arg[0];
		variant_resolver vr(&v, 1);
		VARTYPE vt = vr.vt();
		if (vt&VT_ARRAY)
		{
			SAFEARRAY * psa = vr->parray;
			SAFEARRAYBOUND* rgsabound = psa->rgsabound;
			int d = psa->cDims;
			safearray_holder_t sah(d);
			sa_utils::safe_array_ptr_t sap(sah.psa);
			VARIANT *pdest = sap;
			pdest += d;
			VARIANT t = { VT_I4 };
			for (int k = 0; k < d; k++)
			{
				t.intVal = psa->rgsabound[k].cElements;
				*(--pdest) = t;
			}
			result.Attach(sah.detach());
		}
	}






	static	void __cdecl  safearray_from_jsarray
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {

		result = false;
		try {
			arguments_t arg(pcca);
			HRESULT &hr = arg.hr;

			smart_ptr_t<IDispatchEx> dispex;
			OLE_CHECK_VOID(hr = arg[0].QueryInterface(dispex._address()));
			disp_call_t dc(dispex, L"length");
			OLE_CHECK_VOID(arg.hr = dc());
			int len = dc.result;
			DISPID dispid;
			double d = 0;

			safearray_holder_t sah(len);
			sa_utils::safe_array_ptr_t sap(sah.psa);
			VARIANT *pdest = sap;
			DISPPARAMS dp = { 0,0,0,0 };


			hr = dispex->GetNextDispID(fdexEnumAll, DISPID_STARTENUM, &dispid);
			while (hr == NOERROR)
			{
				BSTR bstrName = 0;
				wchar_t* pend;
				hr = dispex->GetMemberName(dispid, &bstrName);
				int l = SysStringLen(bstrName);
				int i = wcstol(bstrName, &pend, 10);
				if (pend == bstrName + l)
				{
					OLE_CHECK_VOID(hr = dispex->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dp, &pdest[i], pcca->pei, 0));
				}
				SysFreeString(bstrName);
				OLE_CHECK_VOID(hr = dispex->GetNextDispID(fdexEnumAll, dispid, &dispid));
			}

			//arg.hr=hr;


			result.Attach(sah.detach());
			hr = S_OK;


		}
		catch (...) {};


	}


	static	void __cdecl  safearray_from_jsarray_safe
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {

		result = false;
		try {
			arguments_t arg(pcca);
			HRESULT &hr = arg.hr;

			/*
			int ll;
			{
				smart_ptr_t<IDispatch> disp;
				OLE_CHECK_VOID(hr=arg[0].QueryInterface(disp._address()));
				disp_call_t dc(disp,L"length");
				OLE_CHECK_VOID(arg.hr=dc());
				ll=dc.result;
			}
			*/



			smart_ptr_t<IDispatchEx> dispex;
			OLE_CHECK_VOID(hr = arg[0].QueryInterface(dispex._address()));
			disp_call_t dc(dispex, L"length");
			OLE_CHECK_VOID(arg.hr = dc());
			int len = dc.result;
			DISPID dispid;
			double d = 0;

			safearray_holder_t sah(len);
			sa_utils::safe_array_ptr_t sap(sah.psa);
			OLE_CHECK_VOID(hr = sap);



			VARIANT *pdest = sap;
			DISPPARAMS dp = { 0,0,0,0 };

			//		_itow  
			bstr_c_t<16> sname;
			wchar_t* pname = sname;
			VARIANT vempty = { VT_EMPTY };
			for (int i = 0; i < len; i++)
			{
				_itow(i, pname, 10);

				if (SUCCEEDED(dispex->GetIDsOfNames(IID_NULL, &pname, 1, LOCALE_USER_DEFAULT, &dispid))) {
					variant_t r;
					OLE_CHECK_VOID(hr = dispex->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dp, &r, pcca->pei, 0));
					OLE_CHECK_VOID(hr = VariantCopyInd(&pdest[i], &r));
				}
				else pdest[i] = vempty;

			}

			result.Attach(sah.detach());
			hr = S_OK;


		}
		catch (...) {};


	}




	static	void __cdecl  varray_call
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT& hr, void*, unsigned flags) {

		result = false;
		try {
			arguments_t arg(pcca);

			smart_ptr_t<IDispatch> disp;
			OLE_CHECK_VOID(arg.hr = arg[0].QueryInterface(disp._address()));
			std::vector<VARIANT> vd;

			DISPPARAMS dp = { 0,0,0,0 };

			VARIANT*&pd = dp.rgvarg;
			UINT& ld = dp.cArgs;


			if (!arg[1].is_empty())
			{

				VARIANT va = arg[1];



				if (va.vt != (VT_ARRAY | VT_VARIANT))
					OLE_CHECK_VOID(arg.hr = DISP_E_TYPEMISMATCH);

				sa_utils::safe_array_ptr_t sap(va.parray);


				OLE_CHECK_VOID(arg.hr = sap);
				ld = sap.length;
				vd.resize(ld);
				pd = &vd[0];
				VARIANT*ps = (VARIANT*)sap;
				std::reverse_copy(ps, ps + ld, pd);

			}

			arg.hr = disp->Invoke(arg.id(), IID_NULL, LOCALE_USER_DEFAULT, arg.flags(), &dp, &result, pcca->pei, 0);

		}
		catch (...) {};


	}



	static	void __cdecl  uuidgen
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT&, void*, unsigned flags) {

		arguments_t args(pcca);
		CLSID g = uuid_generate();

		if (args[0].def<int>(0))
		{
			result = (wchar_t*)wstr_uuid(g);
		}
		else
		{
			ipc_utils::clsid_t cg(g);
			result = bstr_t(cg);
		}

	}

	static	void __cdecl  fillmem
	(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca, int argc, variant_t* argv, int arglast, HRESULT&, void*, unsigned flags) {



		arguments_t arg(pcca);
		HRESULT& hr = arg.hr;
		smart_ptr_t<i_mbv_buffer_ptr> buffer0;
		hr = arg[0].QueryInterface(buffer0._address());

		smart_ptr_t<i_mbv_typed_buffer_ptr> buffer;
		OLE_CHECK_VOID(hr = arg[0].QueryInterface(buffer._address()));
		char *pc;
		long long l, cb;

		OLE_CHECK_VOID(hr = buffer->GetPtr((void**)&pc, &l));
		VARIANT val = arg[1];

		OLE_CHECK_VOID(hr = buffer->GetCount(&cb));
		for (long long k = 0; k < cb; k++)
		{
			OLE_CHECK_VOID(hr = buffer->SetElement(k, val));
		}


		//memset(pc,-1,l);




		/*bstr_t b=bstr_t(arg[1]);
		clsid_t clsid(b);
		if(SUCCEEDED(clsid.hr)){
		if(SUCCEEDED(hrQI=arg[0].QueryInterface(clsid)))
		result=true;
		}
		}
		catch(...){};*/


	}

	static inline wchar_t* z_bstr(bstr_t& s)
	{
		return (s.length()) ? (wchar_t*)s : 0;
	}


	static void __cdecl  message_box
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca)
	{
		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		HWND hwin = (HWND)arguments[0].def<int>(0);
		bstr_t text = arguments[1].def<bstr_t>(L"");
		bstr_t caption = arguments[2].def<bstr_t>(L"ltx");
		int utype = arguments[3].def<int>(MB_OK | MB_ICONINFORMATION);
		WORD wli = arguments[4].def<int>(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_UK));
		int action = ::MessageBoxExW(hwin, text, caption, utype, wli);
		OLE_CHECK_VOID_hr_cond(action, ::GetLastError());
		result = action;

	}
	static void __cdecl  browse_for_folder
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca) {

		arguments_t arguments(pcca);
		int c = 0;
		HWND hwin = (HWND)arguments[c++].def<int>(0);
		bstr_t title = arguments[c++];
		bstr_t initialPath = arguments[c++];
		int flags = arguments[c++].def<int>(-1);

		result = moniker_parser_t<>::BrowseForFolder(hwin, title, initialPath, flags);

	}


	static void __cdecl  open_save_file_dialog
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca)
	{
#ifdef _WIN64
#pragma pack( push )
#pragma pack( 16 ) 
		typedef struct tagOFNW {
			DWORD        lStructSize;
			HWND         hwndOwner;
			HINSTANCE    hInstance;
			LPCWSTR      lpstrFilter;
			LPWSTR       lpstrCustomFilter;
			DWORD        nMaxCustFilter;
			DWORD        nFilterIndex;
			LPWSTR       lpstrFile;
			DWORD        nMaxFile;
			LPWSTR       lpstrFileTitle;
			DWORD        nMaxFileTitle;
			LPCWSTR      lpstrInitialDir;
			LPCWSTR      lpstrTitle;
			DWORD        Flags;
			WORD         nFileOffset;
			WORD         nFileExtension;
			LPCWSTR      lpstrDefExt;
			LPARAM       lCustData;
			LPOFNHOOKPROC lpfnHook;
			LPCWSTR      lpTemplateName;
#ifdef _MAC
			LPEDITMENU   lpEditInfo;
			LPCSTR       lpstrPrompt;
#endif
#if (_WIN32_WINNT >= 0x0500)
			void *        pvReserved;
			DWORD        dwReserved;
			DWORD        FlagsEx;
#endif // (_WIN32_WINNT >= 0x0500)
		} OPENFILENAME_t;

#pragma pack( pop )
#else
		typedef OPENFILENAMEW OPENFILENAME_t;
#endif   // _WIN64




		arguments_t arguments(pcca);
		int fos = arguments[0];
		bstr_t file = arguments[1];
		variant_t filter = arguments[2];//.def<bstr_t>(L"All Files(*.*)\0*.*\0\0");
		HWND hwin = (HWND)arguments[3].def<int>(0);
		bstr_t title = arguments[4];
		int flags = arguments[5].def<int>(0);
		bstr_t inidir = arguments[6];
		int fl;
		//int fl=filter.length();

		const int bufsize = 8 * 1024 * 1024;
		//std::vector<wchar_t> buf(bufsize);
		bstr_t buf;
		BSTR bs = SysAllocStringLen(0, bufsize);
		fl = SysStringLen(bs);
		memset(bs, 0, 2 * bufsize);
		buf.Attach(bs);
		fl = buf.length();
		wchar_t* pbuf = buf;
		if (file.length()) wcscpy(pbuf, file);

		OPENFILENAME_t ofn = { sizeof(OPENFILENAME_t) };
		ofn.hwndOwner = hwin;
		ofn.lpstrFile = pbuf;
		ofn.nMaxFile = bufsize;
		ofn.lpstrFilter = ((filter.vt == VT_BSTR) && (SysStringLen(filter.bstrVal))) ? filter.bstrVal : 0;
		ofn.lpstrTitle = z_bstr(title);
		ofn.Flags = flags;
		ofn.lpstrInitialDir = z_bstr(inidir);


		BOOL f;
		f = (fos) ? GetSaveFileNameW((OPENFILENAMEW*)&ofn) : GetOpenFileNameW((OPENFILENAMEW*)&ofn);
		// f=GetSaveFileNameW(&ofn);
		//argv_zzs<wchar_t> agrs(ofn.lpstrFile);
		if (!f) arguments.hr = HRESULT_FROM_WIN32(GetLastError());
		else
		{
			if (ofn.Flags | OFN_ALLOWMULTISELECT)
			{
				result.vt = VT_BSTR;
				result.bstrVal = buf.Detach();
				int l = safe_len_zerozero(result.bstrVal);
				SysReAllocStringLen(&result.bstrVal, result.bstrVal, l);

			}
			else result = ofn.lpstrFile;

		}
	};

	static void __cdecl  buffer2buffer
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca) {

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		int64_t count, count_d;
		long el_d, el_s;
		VARIANT vs = {}, vr = {};
		VARTYPE vt;

		ipc_utils::smart_ptr_t<i_mm_cache_buffer_ptr> src, dest;
		OLE_CHECK_VOID(hr = arguments[0].QueryInterface(src._address()));
		OLE_CHECK_VOID(hr = arguments[1].QueryInterface(dest._address()));

		OLE_CHECK_VOID(hr = src->GetElementInfo(&vs.vt, &count, &el_s));
		OLE_CHECK_VOID(hr = dest->GetElementInfo(&vt, &count_d, &el_d));
		if (count > count_d) {
			hr = E_INVALIDARG;
			return;
		}

		vs.vt |= VT_BYREF;

		i_mm_cache_buffer_ptr::locker_t lock1(src);
		i_mm_cache_buffer_ptr::locker_t lock2(dest);

		for (int64_t i = 0; i < count; i++) {
			char*p;
			OLE_CHECK_VOID(hr = src->GetPtrOfIndex(i, &vs.byref));
			OLE_CHECK_VOID(hr = dest->GetPtrOfIndex(i, (void**)&p));
			OLE_CHECK_VOID(hr = VariantChangeType(&vr, &vs, 0, vt));
			memcpy(p, &vr.intVal, el_d);
		}

		result = double(count);

	}

	static void __cdecl  varray2buffer
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca) {

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		int64_t count, count_d;
		long el_d, el_s;
		VARIANT vr = {};
		VARTYPE vt;

		VARIANT va = arguments[0];

		if (va.vt != (VT_ARRAY | VT_VARIANT))
			OLE_CHECK_VOID(arguments.hr = DISP_E_TYPEMISMATCH);

		sa_utils::safe_array_ptr_t sap(va.parray);
		ipc_utils::smart_ptr_t<i_mm_cache_buffer_ptr>  dest;


		OLE_CHECK_VOID(arguments.hr = sap);
		count = sap.length;
		VARIANT*pvs = (VARIANT*)sap;



		OLE_CHECK_VOID(hr = arguments[1].QueryInterface(dest._address()));

		OLE_CHECK_VOID(hr = dest->GetElementInfo(&vt, &count_d, &el_d));
		if (count != count_d) {
			hr = E_INVALIDARG;
			return;
		}




		i_mm_cache_buffer_ptr::locker_t lock2(dest);

		for (int64_t i = 0; i < count; i++) {
			char*p;

			OLE_CHECK_VOID(hr = dest->GetPtrOfIndex(i, (void**)&p));
			OLE_CHECK_VOID(hr = VariantChangeType(&vr, pvs + i, 0, vt));
			memcpy(p, &vr.intVal, el_d);
		}

		result = double(count);

	}

	static void __cdecl  buffer_from_varray
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca) {

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		int64_t count, count_d;
		long el_d, el_s;
		VARIANT vr = {};



		VARIANT va = arguments[0];



		VARTYPE vt;

		bstr_t bt = arguments[1].def<bstr_t>(L"float64");

		OLE_CHECK_VOID(hr = VARTYPE_finder_t<>::type_by_name(bt, &vt));

		if (va.vt != (VT_ARRAY | VT_VARIANT))
			OLE_CHECK_VOID(hr = DISP_E_TYPEMISMATCH);

		sa_utils::safe_array_ptr_t sap(va.parray);
		ipc_utils::smart_ptr_t<i_mm_cache_buffer_ptr>  dest;


		OLE_CHECK_VOID(hr = sap);
		count = sap.length;
		VARIANT*pvs = (VARIANT*)sap;

		hr = moniker_parser_t<wchar_t>::bindObject(L"ltx.bind:mm_buffer:type=#1;length=#2;flags.sparse=0", dest._address())(vt, count);
		if (FAILED(hr))
			hr = moniker_parser_t<wchar_t>::bindObject(L"ltx.bind:mm_buffer:type=#1;length=#2;flags.sparse=1", dest._address())(vt, count);

		OLE_CHECK_VOID(hr);

		//OLE_CHECK_VOID(hr = arguments[1].QueryInterface(dest._address()));

		OLE_CHECK_VOID(hr = dest->GetElementInfo(&vt, &count_d, &el_d));

		if (count != count_d) {
			hr = E_INVALIDARG;
			return;
		}




		i_mm_cache_buffer_ptr::locker_t lock2(dest);

		for (int64_t i = 0; i < count; i++) {
			char*p;

			OLE_CHECK_VOID(hr = dest->GetPtrOfIndex(i, (void**)&p));
			OLE_CHECK_VOID(hr = VariantChangeType(&vr, pvs + i, 0, vt));
			memcpy(p, &vr.intVal, el_d);
		}

		//result = double(count);
		hr = dest.detach(&result);

	}

	static void __cdecl  base64_encode_buffer
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca)
	{
		v_buf<wchar_t> buf;
		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		unsigned char* plz4 = 0;
		struct ptr_deleter_t
		{
			void** pp;
			~ptr_deleter_t() {
				free(*pp);
			}
		} ptr_deleter = { (void**)&plz4 };


		ipc_utils::smart_ptr_t<i_mbv_buffer_ptr> buffer;
		OLE_CHECK_VOID(hr = arguments[0].QueryInterface(buffer._address()));

		int flz4;
		flz4 = (lz4_helper) && arguments[2].def<int>(0);
		flz4 = (zlib_helper) && arguments[2].def<int>(0);
		unsigned char* p;
		i_mbv_buffer_ptr::int64_t len;


		i_mbv_buffer_ptr::locker_t lock(buffer);
		OLE_CHECK_VOID(hr = buffer->GetPtr((void**)&p, &len));
		if (flz4)
		{
			if (0) {
				int cb = lz4_helper.compressBound(len);
				plz4 = (unsigned char*)malloc(cb);
				len = lz4_helper.compress(p, plz4, len, cb);
				p = plz4;

			}
			else {
				zlib_helper_t::size_t cb = zlib_helper.compressBound(len);
				plz4 = (unsigned char*)malloc(cb);

				int level = arguments[3].def<int>(1);
				if (zlib_helper.compress(plz4, &cb, p, len, level))
					OLE_CHECK_VOID(hr = E_FAIL);
				len = cb;
				p = plz4;
			}

			lock.detach();
		}

		bstr_t bstr;
		bstr_t bprefix = arguments[1].def(bstr_t());
		if (len)
		{
			char_mutator<CP_UTF8> cm((wchar_t*)bprefix);
			char* prefix = cm;
			bstr = moniker_parser_t<wchar_t>::base64_encodeW(p, len, 0, (unsigned char*)prefix);
		}
		else bstr = (wchar_t*)bprefix;

		result.vt = VT_BSTR;
		result.bstrVal = bstr.Detach();
	}

	static void __cdecl  base64_decode_buffer
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca)
	{

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		VARIANT v = arguments[0];
		variant_resolver vr(&v);

		if (vr.vt() != VT_BSTR)
			OLE_CHECK_VOID(hr = E_INVALIDARG);

		v_buf<unsigned char> buf;
		unsigned char* pbin;
		int buf_length;

		bstr_t  buf_type = arguments[1].def<bstr_t>(L"uint8");
		ipc_utils::smart_ptr_t<i_mbv_buffer_ptr> buffer_ptr;

		if (0) {
			char_mutator<CP_THREAD_ACP> cm(vr->bstrVal);
			char* pbase64 = cm;
			buf_length = safe_len(pbase64);
			buf.resize(buf_length);
			pbin = buf.get();

			if (!moniker_parser_t<wchar_t>::Base64Decode(pbase64, buf_length, pbin, &buf_length))
				OLE_CHECK_VOID(hr = E_INVALIDARG);






			long els;
			OLE_CHECK_VOID(hr = VARTYPE_finder_t<>::element_size(buf_type, &els));
			long len = buf_length / els;
			if (len*els != buf_length)
				OLE_CHECK_VOID(hr = E_INVALIDARG);
			v_buf<wchar_t> tmp;
			wchar_t* monstr = tmp.printf(L"ltx.bind:sm_buffer: length=%d; type=%s;", len, (wchar_t*)buf_type);

			OLE_CHECK_VOID(hr = ltxGetObject(monstr, __uuidof(i_mbv_buffer_ptr), buffer_ptr._ppQI()));
			unsigned char* ptr;
			OLE_CHECK_VOID(hr = buffer_ptr->GetPtr((void**)&ptr, 0));
			memcpy(ptr, pbin, buf_length);
		}
		else
		{
			moniker_parser_t<wchar_t> parser;
			bool flz4 = false;
			long uncompress_count = 0;
			wchar_t* pwbase64 = vr->bstrVal, *pw, *pparse;
			if (pw = wcschr(pwbase64, L','))
			{
				wchar_t *ptail, *p, *pb64, *pp;
				pparse = parser.get_cache(pwbase64, pw + 1);
				pparse[std::distance(pwbase64, pw)] = 0;

				if (parser._lcheckni(pparse, L"data:", 5, &ptail))
				{

					//pp=parser.get_cache(p=ptail);			 

					if (p = parser._lskipi(ptail, L"_", 1))
					{
						p[-1] = 0;
						buf_type = ptail;
						pp = parser.between(p, L'[', L']', &ptail);
						uncompress_count = get_double(pp);

					}

					pb64 = parser._lskipi(ptail, L":base64", 7, true);
					flz4 = parser._lcheckni(pb64, L":lz4", 4);

					//flz4=flz4&&lz4_helper;
					flz4 = flz4 && zlib_helper;

				}
				pwbase64 = pw + 1;

			}





			long src_len = safe_len(pwbase64);
			buf_length = moniker_parser_t<wchar_t>::Base64DecodeLength(pwbase64);
			long els, ierr;
			OLE_CHECK_VOID(hr = VARTYPE_finder_t<>::element_size(buf_type, &els));


			if (!((uncompress_count > 0) && (buf_length > 0))) {

				wchar_t* monstr = parser.printf(L"ltx.bind:sm_buffer: length=0; type=%s;", (wchar_t*)buf_type);
				OLE_CHECK_VOID(hr = ltxGetObject(monstr, __uuidof(i_mbv_buffer_ptr), buffer_ptr._ppQI()));
				result = buffer_ptr.toVARIANT();
				return;
			}



			if (flz4)
			{
				long len = els * uncompress_count;

				unsigned char* plz4 = 0;
				struct ptr_deleter_t
				{
					void** pp;
					~ptr_deleter_t() {
						free(*pp);
					}
				} ptr_deleter = { (void**)&plz4 };

				plz4 = (unsigned char*)malloc(buf_length);
				if (!moniker_parser_t<wchar_t>::Base64Decode(pwbase64, src_len, plz4, &buf_length))
					OLE_CHECK_VOID(hr = E_INVALIDARG);

				wchar_t* monstr = parser.printf(L"ltx.bind:sm_buffer: length=%d; type=%s;", uncompress_count, (wchar_t*)buf_type);
				OLE_CHECK_VOID(hr = ltxGetObject(monstr, __uuidof(i_mbv_buffer_ptr), buffer_ptr._ppQI()));

				i_mbv_buffer_ptr::locker_t lock(buffer_ptr);
				OLE_CHECK_VOID(hr = buffer_ptr->GetPtr((void**)&pbin, 0));
				//ierr=lz4_helper.decompress(plz4,pbin,buf_length,len);

				long l = len;
				ierr = zlib_helper.decompress(pbin, (UINT32*)&l, plz4, buf_length);
				if (ierr || (l != len))
					OLE_CHECK_VOID(hr = E_FAIL);


			}
			else {

				long len = buf_length / els;
				if (len*els != buf_length)
					OLE_CHECK_VOID(hr = E_INVALIDARG);
				v_buf<wchar_t> tmp;
				wchar_t* monstr = tmp.printf(L"ltx.bind:sm_buffer: length=%d; type=%s;", len, (wchar_t*)buf_type);
				OLE_CHECK_VOID(hr = ltxGetObject(monstr, __uuidof(i_mbv_buffer_ptr), buffer_ptr._ppQI()));
				unsigned char* pbin;
				i_mbv_buffer_ptr::locker_t lock(buffer_ptr);
				OLE_CHECK_VOID(hr = buffer_ptr->GetPtr((void**)&pbin, 0));
				if (!moniker_parser_t<wchar_t>::Base64Decode(pwbase64, src_len, pbin, &buf_length))
					OLE_CHECK_VOID(hr = E_INVALIDARG);
			}

		}

		result = buffer_ptr.toVARIANT();
	}

	static void __cdecl  utf8_buffer_to_string
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca)
	{
		v_buf<wchar_t> buf;
		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		ipc_utils::smart_ptr_t<i_mbv_buffer_ptr> buffer;
		OLE_CHECK_VOID(hr = arguments[0].QueryInterface(buffer._address()));

		int fzz = arguments[1].def<int>(0);

		char* putf8;
		i_mbv_buffer_ptr::int64_t len;

		OLE_CHECK_VOID(hr = buffer->GetPtr((void**)&putf8, &len));

		if (fzz) {
			len = (fzz == 1) ? safe_len(putf8, true) : safe_len_zerozero(putf8, true);
		}

		int cb = MultiByteToWideChar(CP_UTF8, 0, putf8, len, 0, 0);



		BSTR bstr = SysAllocStringLen(0, cb);
		//bstr[cb]=0;


		int cb2 = MultiByteToWideChar(CP_UTF8, 0, putf8, len, bstr, cb);
		if (cb2 != cb) {
			SysFreeString(bstr);
			OLE_CHECK_VOID(hr = E_INVALIDARG);
		}
		result.vt = VT_BSTR;
		result.bstrVal = bstr;
	}





	static void __cdecl  string_to_utf8_buffer
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca)
	{
		v_buf<wchar_t> buf;
		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		VARIANT v = arguments[0];
		variant_resolver vr(&v);
		if (vr.vt() != VT_BSTR)
			OLE_CHECK_VOID(hr = E_INVALIDARG);
		bstr_t name = arguments[1].def<bstr_t>();

		BSTR pw = vr->bstrVal;
		int cb = SysStringLen(pw);
		int cba = WideCharToMultiByte(CP_UTF8, 0, pw, cb, 0, 0, 0, 0);
		wchar_t* monstr;
		if (name.length())
			monstr = buf.printf(L"ltx.bind:sm_buffer: length=%d;flags.temp=1; type=uint8;name=\"%s\"", cba, (wchar_t*)name);
		else
			monstr = buf.printf(L"ltx.bind:sm_buffer: length=%d;flags.temp=1; type=uint8;", cba);

		ipc_utils::smart_ptr_t<i_mbv_buffer_ptr> buffer_ptr;
		OLE_CHECK_VOID(hr = ltxGetObject(monstr, __uuidof(i_mbv_buffer_ptr), buffer_ptr._ppQI()));
		char* ptr;
		OLE_CHECK_VOID(hr = buffer_ptr->GetPtr((void**)&ptr, 0));
		int cba2 = WideCharToMultiByte(CP_UTF8, 0, pw, cb, ptr, cba, 0, 0);
		if (cba != cba2) OLE_CHECK_VOID(hr = E_INVALIDARG);
		result = buffer_ptr.toVARIANT();
	}



	static	void __cdecl  buffer_jsarray(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca)
	{

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		int cc = 0;




		int fgp = arguments[cc++].def<int>(0);
		int64_t offset = arguments[cc++].def<double>(0);


		ipc_utils::smart_ptr_t<i_mm_cache_buffer_ptr> buffer;

		OLE_CHECK_VOID_hr(arguments[cc++].QueryInterface(buffer._address()));

		smart_ptr_t<IDispatchEx> disp;
		OLE_CHECK_VOID_hr(arguments[cc++].QueryInterface(disp._address()));

		disp_call_t dc(disp, L"length");

		OLE_CHECK_VOID_hr(dc());

		int len = dc.result;
		DISPID dispid;
		double d = 0;

		VARTYPE vt;
		int64_t count;
		long elsize;


		i_mm_cache_buffer_ptr::locker_t lock(buffer);
		OLE_CHECK_VOID_hr(buffer->GetElementInfo(&vt, &count, &elsize));
		VARIANT vel = { vt };

		bstr_c_t<16> sname;
		wchar_t* pname = sname;




		i_mm_cache_buffer_ptr::ptr_t<char> ptr(buffer, offset*elsize, len*elsize);
		char* p = ptr;

		if (fgp)
		{
			DISPID dput = DISPID_PROPERTYPUT, ddd;
			//VARIANT vv[2]={{vt},{VT_DISPATCH}};
			//vv[1].pdispVal=disp;
			DISPPARAMS dp = { &vel,&dput,1,1 };
			for (int i = 0; i < len; i++)
			{
				_itow(i, pname, 10);

				//OLE_CHECK_VOID_hr(disp->GetDispID(pname,fdexNameEnsure, &ddd));

				OLE_CHECK_VOID_hr(disp->GetIDsOfNames(IID_NULL, &pname, 1, LOCALE_USER_DEFAULT, &dispid));
				//

				variant_t r;
				memcpy(&vel.intVal, p, elsize);
				p += elsize;
				//
				OLE_CHECK_VOID_hr(disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &dp, &r, 0, 0));
				//OLE_CHECK_VOID_hr(disp->InvokeEx(dispid, LOCALE_USER_DEFAULT,DISPATCH_PROPERTYPUT,&dp,0,0,0));				

				//OLE_CHECK_VOID(hr=VariantCopyInd(&pdest[i],&r));
			}


		}
		else {
			DISPPARAMS dp = { 0,0,0,0 };

			for (int i = 0; i < len; i++)
			{
				_itow(i, pname, 10);

				//
				OLE_CHECK_VOID_hr(disp->GetIDsOfNames(IID_NULL, &pname, 1, LOCALE_USER_DEFAULT, &dispid));
				//OLE_CHECK_VOID_hr(disp->GetDispID(pname,fdexNameEnsure, &dispid));
				variant_t r;
				OLE_CHECK_VOID_hr(disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dp, &r, 0, 0));
				OLE_CHECK_VOID_hr(VariantChangeType(&vel, &r, 0, vt));
				memcpy(p, &vel.intVal, elsize);
				p += elsize;

				//OLE_CHECK_VOID(hr=VariantCopyInd(&pdest[i],&r));
			}


		}
		//arg.hr=hr;
		//result.Attach(sah.detach());




		hr = S_OK;



	}

	static	void __cdecl  get_ini_section(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca) {

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		DWORD sz;
		moniker_parser_t<wchar_t> mp;
		bstr_t appname = arguments[0].def<bstr_t>();
		bstr_t filename = arguments[1].def<bstr_t>();
		wchar_t* buf = mp.get_cache(32 * 1024);

		VARIANT r = { VT_BSTR };
		;
		if ((sz = GetPrivateProfileSectionW(appname, buf, 32 * 1024 - 1, filename)) > 0) {
			r.bstrVal = SysAllocStringLen(buf, sz);
		}
		result.Attach(r);
	}

	/*
	static	void __cdecl  buffer_to_file( variant_t& result, pcallback_lib_arguments_t pltx,pcallback_context_arguments_t pcca)
	{

	arguments_t arguments(pcca);
	HRESULT &hr=arguments.hr;
	int cc=0;

	ipc_utils::smart_ptr_t<i_mm_cache_buffer_ptr> buffer;

	OLE_CHECK_VOID_hr(arguments[cc++].QueryInterface(buffer._address()));

	bstr_t filename=arguments[cc++].def<bstr_t>(0);

	OLE_CHECK_VOID_hr_cond(filename.length(),E_INVALIDARG);





	}
	*/

	static	void __cdecl  empty_working_set(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca) {
		result = bool(EmptyWorkingSet(GetCurrentProcess()));
	}

	static	void __cdecl  command_line(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca) {
		result = GetCommandLineW();
	}

	static	void __cdecl  alloc_console(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca) {
		AllocConsole();
	}
	static	void __cdecl  free_console(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca) {
		FreeConsole();
	}
	static	void __cdecl  attach_console(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca) {
		arguments_t arguments(pcca);
		int pid = arguments[0].def<int>(ATTACH_PARENT_PROCESS);
		arguments.hr = AttachConsole(pid) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}


	static	void __cdecl  NumberOfProcessors(variant_t& result, pcallback_lib_arguments_t pltx, pcallback_context_arguments_t pcca) {
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		result = int(si.dwNumberOfProcessors);
	}

	static void __cdecl  Beep
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca)
	{

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		DWORD fr = arguments[0].def<int>(1000);
		DWORD du = arguments[1].def<int>(200);
		BOOL f = ::Beep(fr, du);
		OLE_CHECK_VOID_hr_cond(f, GetLastError());
	}


	static void __cdecl  CurrentDirectory
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca)
	{

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		moniker_parser_t<wchar_t> mp;
		bool f;
		wchar_t* p;

		if (arguments.length())
		{
			bstr_t dn;
			dn = arguments[0].def<bstr_t>();

			OLE_CHECK_VOID_hr(mp.expand_file_path(dn, &p));
			f = ::SetCurrentDirectoryW(p);
			OLE_CHECK_VOID_hr_cond(f, GetLastError());
		}

		p = mp.get_cache(4096);
		f = GetCurrentDirectory(4096, p);
		OLE_CHECK_VOID_hr_cond(f, GetLastError());
		result = p;

	}

	static HRESULT _get_ip_address(const char* hn, char** pp)
	{
		HRESULT hr;
		hostent * record;

		OLE_CHECK_hr_cond(pp, E_POINTER);
		OLE_CHECK_hr_cond(hn, E_POINTER);
		OLE_CHECK_hr_cond(record = gethostbyname(hn), WSAGetLastError());
		in_addr * address = (in_addr *)record->h_addr;
		*pp = inet_ntoa(*address);

		return S_OK;
	}



	static void __cdecl  get_ip_address
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca)
	{

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		char* ip;
		bstr_t host = arguments[0].def<bstr_t>();
		OLE_CHECK_VOID_hr(_get_ip_address((char*)host, &ip));


		result = (wchar_t*)char_mutator<>(ip);


	}

	static bool _getnameInfo(bstr_t ipaddr, bstr_t& name) {
		char* pipaddr = ipaddr;
		DWORD dwRetval;

		struct sockaddr_in saGNI;
		char hostname[NI_MAXHOST];
		char servInfo[NI_MAXSERV];
		u_short port = 80;
		saGNI.sin_family = AF_INET;
		saGNI.sin_addr.s_addr = inet_addr(pipaddr);
		saGNI.sin_port = htons(port);

		dwRetval = getnameinfo((struct sockaddr *) &saGNI,
			sizeof(struct sockaddr),
			hostname,
			NI_MAXHOST, servInfo,
			NI_MAXSERV, NI_NUMERICSERV);
		if (dwRetval)
			return false;
		name = hostname;
		return true;

	}

	static void __cdecl  getnameInfo
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca)
	{

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		bstr_t  ipname;
		bstr_t host = arguments[0].def<bstr_t>();
		OLE_CHECK_VOID_hr_cond(host.length(), E_INVALIDARG);
		OLE_CHECK_VOID_hr(_getnameInfo(host, ipname));


		result = (wchar_t*)ipname;


	}


	static void __cdecl  cmp_file_id
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca)
	{

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;


		bstr_t f1 = arguments[0].def<bstr_t>();
		bstr_t f2 = arguments[1].def<bstr_t>();

		moniker_parser_t<wchar_t> mn;
		result = mn.cmp_file_id((wchar_t*)f1, (wchar_t*)f2, &(hr = S_OK));

	}



	static void __cdecl  create_pipe
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca) {

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		int c = 0;

		ipc_utils::smart_ptr_t<i_mbv_kernel_object> hko_read, hko_write;
		kernel_object_t* pko_read, *pko_write;

		OLE_CHECK_VOID_hr(arguments[c++].QueryInterface(hko_read._address()));
		OLE_CHECK_VOID_hr(hko_read->GetContext((void**)&(pko_read = NULL)));
		OLE_CHECK_VOID_hr(arguments[c++].QueryInterface(hko_write._address()));
		OLE_CHECK_VOID_hr(hko_write->GetContext((void**)&(pko_write = NULL)));

		int nSize = arguments[c++].def<int>(0);

		OLE_CHECK_VOID_hr_cond(::CreatePipe(&pko_read->hko, &pko_write->hko, NULL, nSize), GetLastError());

	}

	static void __cdecl  event_op
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca) {

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		int c = 0;

		ipc_utils::smart_ptr_t<i_mbv_kernel_object> hko_event;
		kernel_object_t* pko_event;

		OLE_CHECK_VOID_hr(arguments[c++].QueryInterface(hko_event._address()));
		OLE_CHECK_VOID_hr(hko_event->GetContext((void**)&(pko_event = NULL)));

		int op = arguments[c++].def<int>(0);
		//DWORD tio = arguments[c++].def<int>(INFINITE);

		hr = E_INVALIDARG;
		HANDLE h = pko_event->hko;
		DWORD ws;
		if (op == 0) {
			//DWORD tio = arguments[c++].def<int>(INFINITE);						
			//ws = ::WaitForSingleObject(h, INFINITE);
			//OLE_CHECK_VOID_hr_cond(ws = WAIT_FAILED, GetLastError());
			//
			result = WAIT_OBJECT_0 == ipc_utils::wait_loop(h);
			hr = S_OK;
		}
		else if (op == 1) {
			OLE_CHECK_VOID_hr_cond(SetEvent(h), GetLastError());
			result = true;
			hr = S_OK;
		}
		else if (op == 2) {
			OLE_CHECK_VOID_hr_cond(ResetEvent(h), GetLastError());
			result = true;
			hr = S_OK;
		}
		else if (op == 3) {
			OLE_CHECK_VOID_hr_cond(PulseEvent(h), GetLastError());
			result = true;
			hr = S_OK;
		}


	}

	static void __cdecl  mutex_op
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca) {

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		int c = 0;

		ipc_utils::smart_ptr_t<i_mbv_kernel_object> hko_event;
		kernel_object_t* pko_event;

		OLE_CHECK_VOID_hr(arguments[c++].QueryInterface(hko_event._address()));
		OLE_CHECK_VOID_hr(hko_event->GetContext((void**)&(pko_event = NULL)));

		int op = arguments[c++].def<int>(0);
		//DWORD tio = arguments[c++].def<int>(INFINITE);

		hr = E_INVALIDARG;
		HANDLE h = pko_event->hko;
		DWORD ws;
		if (op == 0) {
			result = WAIT_OBJECT_0 == ipc_utils::wait_loop(h);
			hr = S_OK;
		}
		else {
			OLE_CHECK_VOID_hr_cond(ReleaseMutex(h), GetLastError());
			result = true;
			hr = S_OK;
		}


	}

	static void __cdecl  wait_for
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca) {

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		int count = arguments.argc;
		HANDLE ph[64];
		OLE_CHECK_VOID_hr_cond(count <= 64, E_INVALIDARG);

		for (int c = 0; c < count; c++) {
			kernel_object_t* pko;
			ipc_utils::smart_ptr_t<i_mbv_kernel_object> hko;
			OLE_CHECK_VOID_hr(arguments[c].QueryInterface(hko._address()));
			OLE_CHECK_VOID_hr(hko->GetContext((void**)&(pko = NULL)));
			ph[c] = pko->hko;
		}

		result = ipc_utils::wait_loop(ph, count);
		hr = S_OK;

	}

	static void __cdecl  wait_for_ex
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca) {


		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;



		ipc_utils::smart_ptr_t<i_mbv_container> mm;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(mm._address()));

		VARIANT* names;
		long count;

		OLE_CHECK_VOID_hr(mm->GetNames(&(names = 0), &(count = 0)));
		OLE_CHECK_VOID_hr_cond(count <= 64, E_INVALIDARG);


		DWORD tio = arguments[1].def<int>(-1);
		BOOL fwaitall = arguments[2].def<int>(1);

		//DWORD dwFlags = fwaitall ? (MWMO_WAITALL|MWMO_INPUTAVAILABLE | MWMO_ALERTABLE  ) : (MWMO_INPUTAVAILABLE | MWMO_ALERTABLE);  


		HANDLE ph[64];

		for (int c = 0; c < count; c++) {
			kernel_object_t* pko;
			ipc_utils::smart_ptr_t<i_mbv_kernel_object> hko;
			variant_t r;
			OLE_CHECK_VOID_hr(mm->GetItem(names[c], &r));
			OLE_CHECK_VOID_hr(hko.reset(r));
			OLE_CHECK_VOID_hr(hko->GetContext((void**)&(pko = NULL)));
			ph[c] = pko->hko;
		}

		result = (int) ::WaitForMultipleObjects(count, ph, fwaitall, tio);
		hr = S_OK;

	}

	static void __cdecl  copy_file_io
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca) {

		const size_t bufsize = 0x10000;
		char* buffer;

		struct bufholder_t {
			void* p;
			~bufholder_t() { ::free(p); };

		}	bufholder = { buffer = (char*) ::malloc(bufsize) };


		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		int c = 0;
		ipc_utils::smart_ptr_t<i_mbv_kernel_object> hko_w, hko_r;
		kernel_object_t* pko_w, *pko_r;
		OLE_CHECK_VOID_hr(arguments[c++].QueryInterface(hko_w._address()));
		OLE_CHECK_VOID_hr(hko_w->GetContext((void**)&(pko_w = NULL)));

		OLE_CHECK_VOID_hr(arguments[c++].QueryInterface(hko_r._address()));
		OLE_CHECK_VOID_hr(hko_r->GetContext((void**)&(pko_r = NULL)));

		HANDLE hf_w = pko_w->hko, hf_r = pko_r->hko;

		ipc_utils::file_seek_t fs_w(hf_w), fs_r(hf_r);

		bool  f, ffirst = true;

		//SetEndOfFile(hf_w);
		DWORD cbout = -1, cboutw;
		double cbtotal = 0;
		while (1) {

			f = ReadFile(hf_r, buffer, bufsize, &cbout, NULL);
			OLE_CHECK_VOID_hr_cond(f, HRESULT_FROM_WIN32(GetLastError()));

			if (!cbout)	break;


			cbtotal += cbout;

			if (ffirst) {
				SetEndOfFile(hf_w);
			}

			f = WriteFile(hf_w, buffer, cbout, &cboutw, NULL);
			OLE_CHECK_VOID_hr_cond(f, HRESULT_FROM_WIN32(GetLastError()));

			if (cbout < bufsize)	break;
		}

		result = cbtotal;


	}

	static void __cdecl  string_io
	(variant_t& result, void* pctx, pcallback_context_arguments_t pcca)
	{

		typedef moniker_parser_t<wchar_t> parser_t;
		/*
		struct file_seek{
			HANDLE hfile;
			LARGE_INTEGER ll;
			HRESULT hr;
			file_seek(HANDLE hf):hfile(hf),hr(S_OK){
				LARGE_INTEGER t={};
				if(hfile){
					OLE_CHECK_VOID_hr_cond(SetFilePointerEx(hfile,t,&ll,FILE_CURRENT),HRESULT_FROM_WIN32(GetLastError()));
					OLE_CHECK_VOID_hr_cond(SetFilePointerEx(hfile,t,&t,FILE_BEGIN),HRESULT_FROM_WIN32(GetLastError()));
				}
			}

			~file_seek(){
				if(hfile&&SUCCEEDED(hr))
					SetFilePointerEx(hfile,ll,&ll,FILE_BEGIN);
			}

		};
		*/




		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		int c = 0;
		int fwmode = arguments[c++].def<int>(0);
		int fw = fwmode & 0x01;
		ipc_utils::smart_ptr_t<i_mbv_kernel_object> hko;
		kernel_object_t* pko;

		OLE_CHECK_VOID_hr(arguments[c++].QueryInterface(hko._address()));
		OLE_CHECK_VOID_hr(hko->GetContext((void**)&(pko = NULL)));
		//OLE_CHECK_VOID_hr_cond(parser_t::str_cmp_ni(pko->type,L"file",4)==0,E_INVALIDARG);

		HANDLE hfile = pko->hko;

		parser_t mpc;



		BOOL f;
		LARGE_INTEGER ll = {};

		ipc_utils::file_seek_t fs((fwmode & 64) ? 0 : hfile);

		bool fzi = (fwmode & 32);

		OLE_CHECK_VOID_hr(fs.hr);



		const size_t bufsize = 4096;
		char *ptr, *p;

		if (fw == 0) {

			OLE_CHECK_VOID_hr_cond(GetFileSizeEx(hfile, &ll), HRESULT_FROM_WIN32(GetLastError()));
			int cp_xx = arguments[c++].def<int>(CP_UTF8);


			size_t cb = ll.QuadPart, c = cb, nc = cb / bufsize, cr = cb - (nc*bufsize);
			p = ptr = (char*)mpc.get_cache((cb + 1) / 2).get();

			DWORD cbout;

			for (size_t n = 0; n < nc; n++) {

				f = ReadFile(hfile, p, bufsize, &cbout, NULL);
				OLE_CHECK_VOID_hr_cond(f, HRESULT_FROM_WIN32(GetLastError()));
				p += bufsize;

			}

			if (cr > 0) {
				f = ReadFile(hfile, p, cr, &cbout, NULL);
				OLE_CHECK_VOID_hr_cond(f, HRESULT_FROM_WIN32(GetLastError()));
			}
			//char_mutator<>

			//if((!fzi)&&(p[cb-1]==0)&&(cb>0))
			//	--cb;


			variant_t &r = mpc.to_unicode(ptr, cb, cp_xx, &hr);
			OLE_CHECK_VOID_hr(hr);
			result.Attach(r.Detach());

			//ReadFile(hfile,p,



		}
		else {

			wchar_t* pw;
			VARIANT v0 = arguments[c++], vtmp;
			if (v0.vt == VT_BSTR)
				pw = v0.bstrVal;
			else {
				OLE_CHECK_VOID_hr(VariantChangeType(&vtmp, &v0, 0, VT_BSTR));
				pw = vtmp.bstrVal;

			}

			int cp_xx = arguments[c++].def<int>(CP_UTF8);
			size_t cb, cbw;

			if (cp_xx == -1) {

				cb = SysStringByteLen(pw);
				p = ptr = (char*)pw;
			}
			else
				p = ptr = mpc.from_unicode(pw, -1, &cb, cp_xx, &hr);

			OLE_CHECK_VOID_hr(hr);
			if (!p)
				return;

			//if((!fzi)&&(p[cb-1]==0)&&(cb>0))
			//	--cb;

			size_t c = cb, nc = cb / bufsize, cr = cb - (nc*bufsize);
			DWORD cbout;


			SetEndOfFile(hfile);

			for (size_t n = 0; n < nc; n++) {

				f = WriteFile(hfile, p, bufsize, &cbout, NULL);
				OLE_CHECK_VOID_hr_cond(f, HRESULT_FROM_WIN32(GetLastError()));
				p += bufsize;

			}

			if (cr > 0) {
				f = WriteFile(hfile, p, cr, &cbout, NULL);
				OLE_CHECK_VOID_hr_cond(f, HRESULT_FROM_WIN32(GetLastError()));
			}





			result = double(cb);

		}










	}







	inline static HRESULT attempt_buffer_alloc(VARIANT v_in, VARTYPE vt, int64_t n, pcallback_lib_arguments_t lib, i_mm_region_ptr** ppdata)
	{
		HRESULT hr;
		ipc_utils::smart_ptr_t<i_mm_region_ptr> mmptr;
		if (SUCCEEDED(hr = mmptr.reset(v_in)))
		{
			VARTYPE vt0;
			i_mm_region_ptr::int64_t n0;
			long es;

			OLE_CHECK_hr(mmptr->GetElementInfo(&vt0, &n0, &es));
			OLE_CHECK_hr_cond(vt == vt0, E_INVALIDARG);
			OLE_CHECK_hr_cond(n0 <= n, E_INVALIDARG);
			*ppdata = mmptr.detach();
			return hr;
		}
		else {
			VARIANT vv[] = { {VT_I8},{VT_I4} };
			vv[0].llVal = n;
			vv[1].intVal = vt;
			hr = lib->bind_object_args(L"ltx.bind:mm_buffer:length=#1; type=#2", __uuidof(i_mm_region_ptr), (void**)ppdata, 2, vv);
			return hr;
		}
	}

	struct uint64_triplet_t {
		UINT64 v[3];
	};

	static void __cdecl  GetSystemTimes
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{

		FILETIME idle, kernel, user;
		BOOL ff;
		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		ipc_utils::smart_ptr_t<i_mm_region_ptr> mmptr;
		OLE_CHECK_VOID_hr(attempt_buffer_alloc(arguments[0], VT_UI8, 3, plib, mmptr._address()));
		i_mm_region_ptr::ptr_t<uint64_triplet_t> ptrip(mmptr);

		ff = ::GetSystemTimes(&idle, &kernel, &user);
		OLE_CHECK_VOID_hr_cond(ff, GetLastError());

		memcpy(ptrip->v + 0, &idle, sizeof(UINT64));
		memcpy(ptrip->v + 1, &kernel, sizeof(UINT64));
		memcpy(ptrip->v + 2, &user, sizeof(UINT64));

		hr = mmptr.detach(&result);
	}


	static void __cdecl  SetJobAffinity
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{


		BOOL ff;
		DWORD len, affinity_mask;
		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		DWORD_PTR pap, pas;
		ff = GetProcessAffinityMask(GetCurrentProcess(), &pap, &pas);
		OLE_CHECK_VOID_hr_cond(ff, GetLastError());
		affinity_mask = arguments[1].def<int>(pas);
		ipc_utils::smart_ptr_t<i_mbv_kernel_object> ko;
		kernel_object_t* pko;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(ko._address()));
		OLE_CHECK_VOID_hr(ko->GetContext((void**)&(pko = 0)));
		HANDLE hjob = pko->hko;
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION jel = {};
		ff = QueryInformationJobObject(hjob, JobObjectExtendedLimitInformation, &jel, sizeof(jel), &len);
		OLE_CHECK_VOID_hr_cond(ff, GetLastError());
		jel.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_AFFINITY;
		jel.BasicLimitInformation.Affinity = affinity_mask;

		ff = SetInformationJobObject(hjob, JobObjectExtendedLimitInformation, &jel, sizeof(jel));


	}

	static void __cdecl  SetJobMemory
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{

		BOOL ff;
		DWORD len, affinity_mask;
		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		double process_memory, job_memory;

		job_memory = arguments[1].def<double>(-1);
		process_memory = arguments[2].def<double>(-1);




		ipc_utils::smart_ptr_t<i_mbv_kernel_object> ko;
		kernel_object_t* pko;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(ko._address()));
		OLE_CHECK_VOID_hr(ko->GetContext((void**)&(pko = 0)));
		HANDLE hjob = pko->hko;
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION jel = {};
		ff = QueryInformationJobObject(hjob, JobObjectExtendedLimitInformation, &jel, sizeof(jel), &len);
		OLE_CHECK_VOID_hr_cond(ff, GetLastError());

		if (job_memory >= 0)
		{
			jel.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
			jel.JobMemoryLimit = job_memory;

		}
		if (process_memory >= 0)
		{
			jel.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;
			jel.ProcessMemoryLimit = process_memory;
		}



		//jel.BasicLimitInformation.LimitFlags|=JOB_OBJECT_LIMIT_AFFINITY;
		//jel.BasicLimitInformation.Affinity=affinity_mask;

		ff = SetInformationJobObject(hjob, JobObjectExtendedLimitInformation, &jel, sizeof(jel));


	}

	static void __cdecl  GetObjectAndRegisterOnce
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{

		typedef ltx_helper::mbv_map_writer_t<variant_t>  mbv_map_writer_t;
		BOOL ff;

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		DWORD dwReg = 0;
		ipc_utils::smart_ptr_t<IUnknown> obj;
		ipc_utils::smart_ptr_t<IDispatch> callback;
		ipc_utils::smart_ptr_t<IMoniker> mon;
		ipc_utils::smart_ptr_t<IRunningObjectTable> ROT;



		bstr_t display_name = arguments[0].def<bstr_t>();
		OLE_CHECK_VOID_hr_cond(display_name.length(), E_INVALIDARG);


		locker_t<process_com_t::mutex_host_t> lock(process_com_t::get_instance().host_mutex);

		hr = ::CoGetObject(display_name, 0, __uuidof(IUnknown), obj._ppQI());

		if (FAILED(hr))
		{




			OLE_CHECK_VOID_hr(arguments[1].QueryInterface(callback.address()));


			disp_call_t dcall(callback);

			OLE_CHECK_VOID_hr(dcall());

			OLE_CHECK_VOID_hr(obj.reset(dcall.result));





			DWORD flags = arguments[2].def<int>(ROTFLAGS_REGISTRATIONKEEPSALIVE);


			OLE_CHECK_VOID_hr(CreateFileMoniker(display_name, mon._address()));
			OLE_CHECK_VOID_hr(GetRunningObjectTable(0, ROT._address()));
			HRESULT hr0 = ROT->Register(flags, obj, mon, &(dwReg = 0));
			if (MK_S_MONIKERALREADYREGISTERED == hr0)
			{
				ROT->Revoke(dwReg);
				//OLE_CHECK_VOID_hr(HRESULT_FROM_WIN32(hr0));
				//OLE_CHECK_VOID_hr(0x80000000|hr0);
				arguments.raise_error(L"Moniker is already registered in running object table", L"ltx_js");

			}
			OLE_CHECK_VOID_hr(hr0);


		}



		//
		if (1)
		{
			result = int(dwReg);
			result.vt = VT_UNKNOWN;
			result.punkVal = obj.detach();
		}
		else {

			mbv_map_writer_t mw;
			OLE_CHECK_VOID_hr(mw);
			OLE_CHECK_VOID_hr(mw.set_value(L"obj", obj.p));
			OLE_CHECK_VOID_hr(mw.set_value(L"cookie", int(dwReg)));
			hr = mw.detach(&result);
		}
	}



	static void __cdecl  RegisterCOMObject
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{


		BOOL ff;

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		DWORD dwReg;
		ipc_utils::smart_ptr_t<IUnknown> obj;
		ipc_utils::smart_ptr_t<IMoniker> mon;
		ipc_utils::smart_ptr_t<IRunningObjectTable> ROT;



		bstr_t display_name = arguments[0].def<bstr_t>();
		OLE_CHECK_VOID_hr_cond(display_name.length(), E_INVALIDARG);

		OLE_CHECK_VOID_hr(arguments[1].QueryInterface(obj._address()));

		DWORD flags = arguments[2].def<int>(ROTFLAGS_REGISTRATIONKEEPSALIVE);


		OLE_CHECK_VOID_hr(CreateFileMoniker(display_name, mon._address()));
		OLE_CHECK_VOID_hr(GetRunningObjectTable(0, ROT._address()));
		HRESULT hr0 = ROT->Register(flags, obj, mon, &(dwReg = 0));
		if (MK_S_MONIKERALREADYREGISTERED == hr0)
		{
			ROT->Revoke(dwReg);
			//OLE_CHECK_VOID_hr(HRESULT_FROM_WIN32(hr0));
			//OLE_CHECK_VOID_hr(0x80000000|hr0);
			arguments.raise_error(L"Moniker is already registered in running object table", L"ltx_js");

		}



		//

		result = int(dwReg);


	}
	static void __cdecl  UnregisterCOMObject
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{


		BOOL ff;

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		DWORD dwReg;
		ipc_utils::smart_ptr_t<IRunningObjectTable> ROT;

		DWORD dwcookie = arguments[0].def<int>(0);

		OLE_CHECK_VOID_hr(GetRunningObjectTable(0, ROT._address()));
		OLE_CHECK_VOID_hr(ROT->Revoke(dwcookie));
		result = int(dwcookie);


	}



	static void __cdecl  TerminateJob
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{


		BOOL ff;

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		ipc_utils::smart_ptr_t<i_mbv_kernel_object> ko;
		kernel_object_t* pko;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(ko._address()));
		OLE_CHECK_VOID_hr(ko->GetContext((void**)&(pko = 0)));
		HANDLE hjob = pko->hko;
		DWORD ec = arguments[1].def<int>(-1);
		ff = ::TerminateJobObject(hjob, ec);
		OLE_CHECK_VOID_hr_cond(ff, GetLastError());
	}

	static void __cdecl  CallNamedPipeZZ
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{


		BOOL ff;

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		s_parser_t<wchar_t> parser;



		int icc = 0;
		int szout, szin, l;
		bstr_t pipename;
		wchar_t* pmsgin;

		variant_t vin;
		//VARIANT v=arguments[1];
		//OLE_CHECK_VOID_hr_cond(v.vt==VT_BSTR,E_INVALIDARG);		 
		OLE_CHECK_VOID_hr_cond(l = (pipename = arguments[icc++].def<bstr_t>()).length(), E_INVALIDARG);


		{
			wchar_t* p = pipename;
			for (int k = 0; k < l; k++)
				if (p[k] == L'/') p[k] = L'\\';


			//msgin=arguments[icc++].def<bstr_t>();

			VARIANT vr = VARIANT(arguments[icc++]);

			if (vr.vt != VT_BSTR)
			{
				OLE_CHECK_VOID_hr(VariantChangeType(&vin, &VARIANT(arguments[icc++]), 0, VT_BSTR));
				p = vin.bstrVal;
			}
			else p = vr.bstrVal;
			l = SysStringLen(p);

			pmsgin = parser.get_cache(p, p + l);

		}



		szout = arguments[icc++].def<double>(4096);

		int tio = arguments[icc++].def<int>(0);

		char_mutator<CP_UTF8, true> cm_in(pmsgin);

		szin = cm_in.char_count(true);


		v_buf<char> bufout(szout);
		char* pout = bufout, *pin = cm_in;


		ff = ::CallNamedPipeW(pipename, pin, szin, pout, szout, (DWORD*)&szout, tio);
		OLE_CHECK_VOID_hr_cond(ff, GetLastError());
		char_mutator<CP_UTF8, true> cm_out(pout);

		VARIANT r = { VT_BSTR };
		int szoutw = sizeof(wchar_t)*cm_out.wchar_count();
		wchar_t* poutw = cm_out;

		r.bstrVal = ::SysAllocStringByteLen(0, szoutw);
		memcpy(r.bstrVal, poutw, szoutw);
		result.Attach(r);

	}


	static void __cdecl  dll_attr
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca) {


		BOOL ff;

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		bstr_t fn = arguments[0].def<bstr_t>();
		ipc_utils::module_helper_t mh(fn);
		OLE_CHECK_VOID_hr(mh.hr);

		int flags = 0;
		mh.is_dll() && (flags |= 1);
		mh.is_win32() && (flags |= 2);

		result = flags;
	}

	static void __cdecl  clipbrd
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{


		BOOL ff;

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		VARIANT v = { VT_BSTR };

		bool f = arguments.length();
		bstr_t b;

		if (f) {

			b = arguments[0].def<bstr_t>();
			ipc_utils::win_ver_6_t<>::ClipboardText cl(b);

			OLE_CHECK_VOID_hr(cl.hr);



		}

		b = ipc_utils::win_ver_6_t<>::ClipboardText();

		v.bstrVal = b.Detach();

		result.Attach(v);

	};


	static void __cdecl  check_member
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{


		BOOL ff;

		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;
		HRESULT hr0;
		ipc_utils::smart_ptr_t<IDispatch> disp;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(disp._address()));
		bstr_t name = arguments[1].def<bstr_t>();
		variant_t rfailed = L"Not Found!";
		DISPID dispid = -1;
		if (name.length())
		{
			wchar_t* pname = name;

			hr0 = disp->GetIDsOfNames(IID_NULL, &pname, 1, LOCALE_USER_DEFAULT, &dispid);
			if (SUCCEEDED(hr0))
			{
				result = int(dispid);
				return;
			}

		}


		result = rfailed;
	}

	static void __cdecl  is_hyperthreading
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{
		int cpuinfo[4];
		__cpuid(cpuinfo, 1);
		bool hasHT = (cpuinfo[3] & (1 << 28)) > 0;
		result = hasHT;

	}
	static void __cdecl  stream2file
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{
		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr, hr0;
		ipc_utils::smart_ptr_t<IStream> src, dest;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(src._address()));
		bstr_t fn = arguments[1].def<bstr_t>(L"");
		OLE_CHECK_VOID_hr(SHCreateStreamOnFileEx(fn, STGM_WRITE | STGM_CREATE, FILE_ATTRIBUTE_NORMAL, true, NULL, dest._address()));
		ULARGE_INTEGER ul, cb, cbr, cbw;
		LARGE_INTEGER pos = LARGE_INTEGER();
		//cb.QuadPart=ULONGLONG(-1);
		OLE_CHECK_VOID_hr(src->Seek(pos, STREAM_SEEK_END, &cb));
		OLE_CHECK_VOID_hr(src->Seek(pos, STREAM_SEEK_SET, &ul));
		OLE_CHECK_VOID_hr(dest->Seek(pos, STREAM_SEEK_SET, &ul));
		OLE_CHECK_VOID_hr(src->CopyTo(dest, cb, &cbr, &cbw));
		if (cbr.QuadPart == cbw.QuadPart) {
			result = double(cbw.QuadPart);
		}
		else hr = E_FAIL;
	}

	static void __cdecl  filestream
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{
		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr, hr0;
		ipc_utils::smart_ptr_t<IStream> stream;
		bstr_t fn = arguments[0].def<bstr_t>(L"");
		OLE_CHECK_VOID_hr(SHCreateStreamOnFileEx(fn, STGM_READ, FILE_ATTRIBUTE_NORMAL, false, NULL, stream._address()));
		hr = stream.detach(&result);
	}
	//set_window_alpha

	static void __cdecl  set_window_alpha
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{
		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr;

		HWND hWnd = HWND(arguments[0].def<uint64_t>(0));
		double alpha = arguments[1].def<double>(100.0);
		BYTE balpha = (255.0 * alpha) / 100.0;
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hWnd, 0, balpha, LWA_ALPHA);
		hr = S_OK;
	}
	static void __cdecl  set_icon
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca)
	{
		arguments_t arguments(pcca);
		HRESULT &hr = arguments.hr, hr0;

		HWND hw = HWND(arguments[0].def<uint64_t>(0));
		bstr_t fn = arguments[1].def<bstr_t>();
		HMODULE hkl = module_t<>::__getHinstance();
		HICON hico = (HICON)LoadImageW(NULL, (wchar_t*)fn, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
		OLE_CHECK_VOID_hr_cond(hico, GetLastError());
		int lr;
		lr = SendMessage(hw, WM_SETICON, ICON_BIG, LPARAM(hico));
		lr = SendMessage(hw, WM_SETICON, ICON_SMALL, LPARAM(hico));
		result = lr;
	}

	static void __cdecl  object_id
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca) {
		typedef ULONGLONG uint64_t;

		arguments_t arguments(pcca);

		HRESULT &hr = arguments.hr;
		ipc_utils::smart_ptr_t<IUnknown> unk;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(unk._address()));
		int fmt = arguments[1].def<int>(64);

		uint64_t i = uint64_t((void*)unk.p);
		if (fmt == 64) {

			//result = (wchar_t*)mp.base64_encodeW((unsigned char*)&i, sizeof(unk.p));
			hr = moniker_parser_t<wchar_t>::base64_encodeV(&result, (unsigned char*)&i, sizeof(unk.p));
		}
		else if (fmt == 16) {
			moniker_parser_t<wchar_t> mp;
			result = (wchar_t*)mp.printf(L"%016llx", i);

		}
		else {

			result = double(i);
		}

	}
	
	static void __cdecl  module_name
	(variant_t& result, pcallback_lib_arguments_t plib, pcallback_context_arguments_t pcca) {
		typedef ULONGLONG uint64_t;

		arguments_t arguments(pcca);

		HRESULT &hr = arguments.hr;
		ipc_utils::smart_ptr_t<IUnknown> unk;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(unk._address()));		

		result = (wchar_t*)ipc_utils::module_t<>::module_name(unk.p, &hr);

	}
};



PROC_LIST_BEGIN(ltx_context_utils)
PROC_ITEM_NAME_POSTFIX(ltx_, addref)
PROC_ITEM_NAME_POSTFIX(ltx_, release)
PROC_ITEM_NAME_POSTFIX(ltx_, create_resource_handle)
PROC_ITEM_NAME_POSTFIX(ltx_, get_context)
PROC_ITEM_NAME_POSTFIX(ltx_, tss_onexit)
PROC_LIST_END;


static struct h_t :context_holder_t
{
	inline void mp(void** d, void* s)
	{
		*d = s;
	}
	static void*  create_s(void* context, void(*ondestroy)(void* context))
	{
		IUnknown* pu = 0;
		ltx_create_resource_handle(context, ondestroy, &pu);
		return pu;

	};
	static void*  context_s(void* handle) {

		void *ctx = 0;
		ltx_get_context((IUnknown*)handle, &ctx);
		return ctx;

	};

	static long  set_tss_lifetime_s(void* handle) {

		IUnknown* punk = (IUnknown*)handle;
		ltx_tss_onexit(punk, &ltx_release);
		return 1;
	};

	h_t() {

		mp((void**)&addref, (void*)&ltx_addref);
		mp((void**)&release, (void*)&ltx_release);



		//mp((void**)&wrap_context,(void*)&create_s);
		//mp((void**)&unwrap_context,(void*)&context_s);
		mp((void**)&set_tss_lifetime, (void*)&set_tss_lifetime_s);

		//mp((void**)&handle_push_cache_handle,(void*)&ltx_push_cache);
		//mp((void**)&handle_clear_cache,(void*)&ltx_clear_cache);

		mp((void**)&link, (void*)&ltx_push_cache);
		mp((void**)&clear_links, (void*)&ltx_clear_cache);
		mp((void**)&global_ref_count, (void*)&ltx_global_ref_count);




		mp((void**)&wrap_context, (void*)&ltx_create_resource_handle);
		mp((void**)&unwrap_context, (void*)&ltx_get_context);


		mp((void**)&create_handle, (void*)&ltx_create_resource_handle);
		mp((void**)&get_context, (void*)&ltx_get_context);
		mp((void**)&tss_onexit, (void*)&ltx_tss_onexit);


	}

};


extern "C"  void* __cdecl  get_context_utils() {


	return (context_holder_t*)&class_initializer_T<h_t>().get();
}

PROC_LIST_BEGIN(ltx_callback_list)
PROC_ITEM(win_console)
PROC_ITEM(GetDispId)
PROC_ITEM_STRUCT(ltx_utils_t<>, checkQueryInterface)
PROC_ITEM_STRUCT(ltx_utils_t<>, variant_type)
PROC_ITEM_STRUCT(ltx_utils_t<>, varray_call)
PROC_ITEM_STRUCT(ltx_utils_t<>, set_window_alpha)
PROC_ITEM_STRUCT(ltx_utils_t<>, find_process_window)
PROC_ITEM_STRUCT(ltx_utils_t<>, find_process_top_window)
PROC_ITEM_STRUCT(ltx_utils_t<>, kill_process_after_process)
PROC_ITEM_STRUCT(ltx_utils_t<>, kill_process_by_id)
PROC_ITEM_STRUCT(ltx_utils_t<>, on_process_terminated)
PROC_ITEM_STRUCT(ltx_utils_t<>, safearray_from_jsarray)
PROC_ITEM_STRUCT(ltx_utils_t<>, safearray_from_jsarray_safe)
PROC_ITEM_STRUCT(ltx_utils_t<>, safearray_sizes)
PROC_ITEM_STRUCT(ltx_utils_t<>, fillmem)
PROC_ITEM_STRUCT(ltx_utils_t<>, uuidgen)
PROC_ITEM_STRUCT(ltx_utils_t<>, open_save_file_dialog)
PROC_ITEM_STRUCT(ltx_utils_t<>, browse_for_folder)
PROC_ITEM_STRUCT(ltx_utils_t<>, message_box)
PROC_ITEM_STRUCT(ltx_utils_t<>, string_to_utf8_buffer)
PROC_ITEM_STRUCT(ltx_utils_t<>, utf8_buffer_to_string)
PROC_ITEM_STRUCT(ltx_utils_t<>, base64_encode_buffer)
PROC_ITEM_STRUCT(ltx_utils_t<>, base64_decode_buffer)
PROC_ITEM_STRUCT(ltx_utils_t<>, sleepmsg)
PROC_ITEM_STRUCT(ltx_utils_t<>, buffer_jsarray)
PROC_ITEM_STRUCT(ltx_utils_t<>, empty_working_set)
PROC_ITEM_STRUCT(ltx_utils_t<>, alloc_console)
PROC_ITEM_STRUCT(ltx_utils_t<>, command_line)
PROC_ITEM_STRUCT(ltx_utils_t<>, free_console)
PROC_ITEM_STRUCT(ltx_utils_t<>, attach_console)
PROC_ITEM_STRUCT(ltx_utils_t<>, NumberOfProcessors)
PROC_ITEM_STRUCT(ltx_utils_t<>, Beep)
PROC_ITEM_STRUCT(ltx_utils_t<>, CurrentDirectory)
PROC_ITEM_STRUCT(ltx_utils_t<>, get_ip_address)
PROC_ITEM_STRUCT(ltx_utils_t<>, getnameInfo)
PROC_ITEM_STRUCT(ltx_utils_t<>, GetSystemTimes)
PROC_ITEM_STRUCT(ltx_utils_t<>, SetJobAffinity)
PROC_ITEM_STRUCT(ltx_utils_t<>, SetJobMemory)
PROC_ITEM_STRUCT(ltx_utils_t<>, TerminateJob)
PROC_ITEM_STRUCT(ltx_utils_t<>, GetObjectAndRegisterOnce)
PROC_ITEM_STRUCT(ltx_utils_t<>, RegisterCOMObject)
PROC_ITEM_STRUCT(ltx_utils_t<>, UnregisterCOMObject)
PROC_ITEM_STRUCT(ltx_utils_t<>, CallNamedPipeZZ)
PROC_ITEM_STRUCT(ltx_utils_t<>, clipbrd)
PROC_ITEM_STRUCT(ltx_utils_t<>, dll_attr)
PROC_ITEM_STRUCT(ltx_utils_t<>, check_member)
PROC_ITEM_STRUCT(ltx_utils_t<>, stream2file)
PROC_ITEM_STRUCT(ltx_utils_t<>, filestream)
PROC_ITEM_STRUCT(ltx_utils_t<>, cmp_file_id)
PROC_ITEM_STRUCT(ltx_utils_t<>, string_io)
PROC_ITEM_STRUCT(ltx_utils_t<>, copy_file_io)

PROC_ITEM_STRUCT(ltx_utils_t<>, create_pipe)
PROC_ITEM_STRUCT(ltx_utils_t<>, event_op)
PROC_ITEM_STRUCT(ltx_utils_t<>, mutex_op)
PROC_ITEM_STRUCT(ltx_utils_t<>, wait_for)
PROC_ITEM_STRUCT(ltx_utils_t<>, wait_for_ex)
PROC_ITEM_STRUCT(ltx_utils_t<>, buffer2buffer)
PROC_ITEM_STRUCT(ltx_utils_t<>, varray2buffer)
PROC_ITEM_STRUCT(ltx_utils_t<>, buffer_from_varray)
PROC_ITEM_STRUCT(ltx_utils_t<>, object_id)
PROC_ITEM_STRUCT(ltx_utils_t<>, set_icon)
PROC_ITEM_STRUCT(ltx_utils_t<>, process_image_filename_by_id)
PROC_ITEM_STRUCT(ltx_utils_t<>, parent_process_id)
PROC_ITEM_STRUCT(ltx_utils_t<>, is_debug_present)
PROC_ITEM_STRUCT(ltx_utils_t<>, get_ini_section)
PROC_ITEM_STRUCT(ltx_utils_t<>, module_name)


//PROC_ITEM_STRUCT(ltx_utils_t<>,is_hyperthreading)
PROC_STRUCT_STRUCT(ltx_job_list, JOB)
PROC_ITEM(ShowHTML)
PROC_LIST_END;
