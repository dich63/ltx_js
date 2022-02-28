#pragma once


//#include "char_parsers/wchar_parsers.h"



#include "ltx_utils/ipc_utils_base.h"
#include "ltx_utils/dispatch_call_helpers.h"
#include "ltx_utils/typecase.h"
#include "ltx_utils/ltx_call_helper.h"

#include <vector>

namespace ltx {
	typedef ipc_utils::com_dispcaller_t caller_t;
	typedef  caller_t::exception_t exception_t;
	typedef caller_t::exception_helper_t exception_helper_t;
	typedef variant_t value_t;
	typedef bstr_t string_t;
	constexpr char *js_script_srv = "ltx.bind:srv:[job=*]:64:script";
	constexpr char *js_script = "ltx.bind:script";

	template <class WC>
	inline WC* callback_creator(WC* pw, value_t* pres) {
		HRESULT hr;
		exception_helper_t::check_error(hr = pw->wrap(pres));
		return pw;
	}

	template <class WC>
	inline value_t bind_callback(WC* p) {
		HRESULT hr;
		value_t r;
		exception_helper_t::check_error_if(p,hr =E_POINTER);
		exception_helper_t::check_error(hr = p->wrap(&r));
		return r;
	}



	inline string_t join(std::vector<const char*> v, string_t dlm= "\n") {
		string_t s;
		auto c = v.size();
		for (auto k = 0; k < c - 1; ++k) {
			s += string_t(v[k]) + dlm;
		}
		for (auto k = c - 1; k < c; ++k) {
			s += string_t(v[k]);
		}
		return s;
	}


};
