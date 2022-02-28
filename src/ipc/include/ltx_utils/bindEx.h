#pragma once

#include "dispatch_call_helpers.h"

namespace ipc_utils {


	struct loader_ltx_t {


		//typedef HRESULT(*bindObjectWithArgsEx_t)(const wchar_t* moniker_str, REFIID riid, void** ppObj, bool freverse, int argc, VARIANT* argv);
		
		loader_ltx_t(){

			com_dispcaller_t tmp("ltx.bind:");

			HMODULE hm = ::LoadLibraryW(L"ltx_js.dll");
			bool f=hm;			

			f = f && SET_PROC_ADDRESS(hm, bindObjectWithArgsEx);
			f = f && SET_PROC_ADDRESS(hm, ltx_process_callback_loop);
			f = f && SET_PROC_ADDRESS(hm, ltx_external_connector_wait);

			hr=(f)?S_OK: HRESULT_FROM_WIN32(GetLastError());

		}
		//bindObjectWithArgsEx_t bind;

		HRESULT  (*bindObjectWithArgsEx)(const wchar_t* moniker_str, REFIID riid, void** ppObj, bool freverse, int argc, VARIANT* argv);
		HRESULT  (*ltx_process_callback_loop)(void* pcalback, void* context, void* on_exit);
		HRESULT  (*ltx_external_connector_wait)(DWORD tio, DWORD* pwaitresult);

		inline static loader_ltx_t& get_instance() {
			static loader_ltx_t ltx;
			return ltx;
		}
		
		HRESULT hr;
	};



	struct bindObject_t :invoker_base_t<bindObject_t, variant_t, HRESULT, VARIANT>
	{
		bstr_t moniker_str_;
		REFIID riid_;
		void** ppObj_;
		bindObject_t(bstr_t moniker_str, REFIID riid, void** ppObj)
			:moniker_str_(moniker_str), riid_(riid), ppObj_(ppObj) {};

		inline 	HRESULT invoke_proc(int argc, argh_t* argv) {
			static loader_ltx_t ltx;
			HRESULT hr = ltx.hr;
			return hr =(SUCCEEDED(hr))? ltx.bindObjectWithArgsEx(moniker_str_, riid_, ppObj_, true, argc, argv):hr;
		}

		inline operator HRESULT() { return invoke_proc(0, 0); };

		
	};

	template <class I,class  ExcetionClass = com_exception>
	struct bindObjectExc_t :invoker_base_t< bindObjectExc_t<I, ExcetionClass>, variant_t, smart_ptr_t<I>, VARIANT>
	{
		//typedef com_exception_helper_t<ExcetionClass> exception_helper_t;
		typedef typename VARIANT argh_t;
		bstr_t moniker_str_;
		REFIID riid_;
		

		bindObjectExc_t(bstr_t moniker_str, REFIID riid)
			:moniker_str_(moniker_str), riid_(riid) {};

		inline 	smart_ptr_t<I> invoke_proc(int argc, argh_t* argv) {

			smart_ptr_t<I> r;

			loader_ltx_t&  ltx= loader_ltx_t::get_instance();

			HRESULT hr = ltx.hr;
			hr = (SUCCEEDED(hr)) ? ltx.bindObjectWithArgsEx(moniker_str_, riid_,r._ppQI(), true, argc, argv) : hr;

			com_exception_helper_t<ExcetionClass>::check_error(hr);
			return r;
		}

		inline operator smart_ptr_t<I>() { return invoke_proc(0, 0); };

		//bindObjectWithArgsEx(const wchar_t* moniker_str,REFIID riid,void** ppObj,bool freverse=true,int argc=0,VARIANT* argv=0);
	};



	template <class  ExcetionClass = com_exception>
	struct bindObjectExcV_t :invoker_base_t< bindObjectExcV_t<ExcetionClass>, variant_t, variant_t, VARIANT>
	{
		//typedef com_exception_helper_t<ExcetionClass> exception_helper_t;
		typedef typename VARIANT argh_t;
		bstr_t moniker_str_;
		


		bindObjectExcV_t(bstr_t moniker_str)
			:moniker_str_(moniker_str) {};

		inline 	variant_t invoke_proc(int argc, argh_t* argv) {

			VARIANT  r = {VT_UNKNOWN};

			loader_ltx_t&  ltx = loader_ltx_t::get_instance();

			HRESULT hr = ltx.hr;
			hr = (SUCCEEDED(hr)) ? ltx.bindObjectWithArgsEx(moniker_str_,__uuidof(IUnknown), &r.byref, true, argc, argv) : hr;

			com_exception_helper_t<ExcetionClass>::check_error(hr);
			return variant_t(r,false);
		}

		inline operator variant_t() { return invoke_proc(0, 0); };

		
	};




	inline static bindObject_t bindObject(bstr_t moniker_str, REFIID riid, void** ppObj){
		return bindObject_t(moniker_str, riid, ppObj);
	}
	template <class I>
	inline static bindObject_t bindObject(bstr_t moniker_str, I** ppObj) {
		return bindObject(moniker_str, __uuidof(I), (void**)ppObj);
	}

	template <class I>
	inline static bindObject_t bindObject(bstr_t moniker_str, smart_ptr_t<I>& sm ) {
		return bindObject(moniker_str, __uuidof(I),sm.ppQI());
	}

	template <class I>
	inline static bindObjectExc_t<I> bindObject(bstr_t moniker_str) {
		return bindObjectExc_t<I>(moniker_str,__uuidof(I));
	}

	
	inline static bindObjectExcV_t<> bindObject(bstr_t moniker_str) {		
		
		return bindObjectExcV_t<>(moniker_str);
	};


	template<class O,class ExcetionClass,DWORD flags, typename... Arguments>
	variant_t wrap(const Arguments&... args)	{
		typedef com_exception_helper_t<ExcetionClass> exception_helper_t;

		HRESULT hr;
		variant_t r;
		auto p = new O(args...);

		ipc_utils::com_dispcaller_t::exception_helper_t::check_error_if(p, hr = E_POINTER);
		ipc_utils::com_dispcaller_t::exception_helper_t::check_error(hr = p->wrap(&r, flags));
		return r;

	};

	template<class O, typename... Arguments>
	variant_t wrap(const Arguments&... args) {
		return wrap<O, com_exception,0>(args...);
	}

	template<class O, typename... Arguments>
	variant_t wrap_ec(const Arguments&... args) {
		return wrap<O, com_exception, (FLAG_CBD_EC | FLAG_CBD_FTM)>(args...);
	}


};// end namespace ipc_utils