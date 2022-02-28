// simple_server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "ltxjs.h"



#include <map>
#include <mutex>
#include <string>

typedef ipc_utils::com_scriptor_t javascript_t;
typedef ipc_utils::com_dispcaller_t caller_t;
typedef javascript_t::value_t jsvalue_t;
typedef i_mbv_buffer_ptr::int64_t int64_t;
typedef ltx_helper::arguments_t<jsvalue_t>  arguments_t;



struct ObjectMapExample_t :ltx_helper::dispatch_by_name_wrapper_t< ObjectMapExample_t, jsvalue_t, std::mutex>
{
	struct jsvalue_holder_t
	{
		jsvalue_holder_t() {}
		jsvalue_holder_t(jsvalue_t rv) { v = rv; }
		jsvalue_t v;
		inline operator jsvalue_t() { return v; }
	};

	typedef std::map<std::wstring, jsvalue_holder_t> jsmap_t;

	inline  void  on_get(DISPID id, const wchar_t* name, com_variant_t& result, arguments_t& arguments, i_marshal_helper_t* helper) {

		if (0==id) {
			std::cout << "clone ObjectMapExample_t ...\n";
			result = ipc_utils::wrap_ec<ObjectMapExample_t>();
			return;
		}
		if (!name)  arguments.raise_error(L"on_get е№џяж!!", L"ObjectMapExample_t");
		//_cwprintf(L"on_get %s\n", name);
		result = jsmap[name];

	};

	inline  void  on_put(DISPID id, const wchar_t* name, com_variant_t& value, arguments_t& arguments, i_marshal_helper_t* helper) {

		if (!name)  arguments.raise_error(L"on_put е№џяж!!", L"ObjectMapExample_t");
		//_cwprintf(L"on_put %s\n", name);
		jsmap[name] = jsvalue_holder_t(value);
	};

	ObjectMapExample_t() :dispatch_by_name_wrapper_t() {
		std::cout << "create ObjectMapExample_t ...\n";
	};

	~ObjectMapExample_t() {

		std::cout << "destroying ObjectMapExample_t ...\n";
	}

	jsmap_t jsmap;

};



int main()
{
	ipc_utils::COMInitF_t com_initialize;
	//ipc_utils::loader_ltx_t::get_instance().ltx_external_connector_wait(INFINITE, NULL);
	//caller_t mm = ipc_utils::wrap<ObjectMapExample_t>();
	//mm["AA"] = 11;

	jsvalue_t vm = ipc_utils::wrap_as_server<ObjectMapExample_t>();
	::Sleep(2000);
}

