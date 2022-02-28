#pragma once

#include "ltx_utils/ipc_utils_base.h"
#include "ltx_utils/dispatch_call_helpers.h"
#include "ltx_utils/typecase.h"
#include "ltx_utils/bindEx.h"
#include "utils/utils.h"

typedef bstr_t olestr_t;
namespace ipc_utils {

	inline bool is_external(smart_ptr_t<IDispatch>& sp) {
		HRESULT hr;
		return  SUCCEEDED(hr = ::CoGetObject(olestr_t("ltx.bind:external"), NULL, __uuidof(IDispatch), sp.ppQI())) ? true : false;
	};
	
	inline bool is_external() {
		smart_ptr_t<IDispatch> sp;
		return is_external(sp);
	
	}

	inline bool is_external(com_dispcaller_t& dc) {
		bool f;
		smart_ptr_t<IDispatch> sp;
		if (f = is_external(sp)) {
			if(FAILED(dc.reset(sp.p)))
				return false;
		}
		return f;
	};

	//
	template<class O, typename... Arguments>
	int wrap_as_server(const Arguments&... args) {


		variant_t ec=ipc_utils::bindObject("ltx.bind:external.connector:  global=3 ;")();
		variant_t r= wrap<O, com_exception, (FLAG_CBD_EC|FLAG_CBD_FTM) << 0>(args...);
		com_dispcaller_t external;
		if (is_external(external)) {


			external["result"] = r;

			external.reset();

			loader_ltx_t::get_instance().ltx_external_connector_wait(INFINITE, NULL);
		}
		return 0;

	}



}; //namespace ipc_utils
//#include "char_parsers/wchar_parsers.h"