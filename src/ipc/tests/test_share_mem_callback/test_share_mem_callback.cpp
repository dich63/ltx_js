// test_share_mem_callback.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

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

		if (!name)  arguments.raise_error(L"on_get е№џяж!!", L"ObjectMapExample_t");
		//_cwprintf(L"on_get %s\n", name);
		result = jsmap[name];

	};

	inline  void  on_put(DISPID id, const wchar_t* name, com_variant_t& value, arguments_t& arguments, i_marshal_helper_t* helper) {
	
		if (!name)  arguments.raise_error(L"on_put е№џяж!!", L"ObjectMapExample_t");
		//_cwprintf(L"on_put %s\n", name);
		jsmap[name] = jsvalue_holder_t(value);
	};
	
	~ObjectMapExample_t() {
		
		std::cout << "destroying ObjectMapExample_t ...\n";
	}

	jsmap_t jsmap;

};




struct callbackExample_t :ltx_helper::dispatch_wrapper_t< callbackExample_t, jsvalue_t> {
	inline  void  oncallback(com_variant_t& result, arguments_t& arguments) {
		
		for (int k = 0; k < arguments.length(); k++) {
			
			std::cout << arguments[k].def<olestr_t>("??") << " <-callback \n";
		}

		result = -7777.3333;

	}
	~callbackExample_t() {

		std::cout << "destroying callbackExample_t....\n";
	}
};




void main(int argn, char** argv) {

	HRESULT hr;	

	ipc_utils::COMInitF_t com_initialize;
	
	
	
	caller_t parent;   
	

	if (false==ipc_utils::is_external(parent)) {

		//  PARENT PROCESS  EXECUTING.....

		// create  dictionary object

		ObjectMapExample_t * mp;

		jsvalue_t vm = ipc_utils::wrap<ObjectMapExample_t>();	
		


		hr = ObjectMapExample_t::unwrap(vm, &mp);

		auto& map = mp->jsmap;	

		
		

		caller_t cmap(vm);
		cmap["Key"] = "Ok";
		jsvalue_t s = cmap["Key"];

		


		// create  memmaping buffer
		jsvalue_t mmbuffer = ipc_utils::bindObject("ltx.bind:mm_buffer:len=#1;type=#2")(1111, "double");

		i_mm_region_ptr::ptr_t<double> mm_region(mmbuffer);

		double* pf = mm_region;

		// pv for debug watch list view 16 elements
		auto  pv = utils::pN_t<16>::ptr(pf);

		// fill pf
		for (auto k = 0; k < 11; k++) pf[k] = (k + 1) / 3.;


		jsvalue_t cb = ipc_utils::wrap<callbackExample_t>();
        
		

		caller_t process = olestr_t("ltx.bind:process: file=*;CF=16;dbghook=1");// dbg.hook=0");
		
		

		auto result= [&]() {
		
			i_mm_region_ptr::unlocker_t  unlock(mm_region.intf());		

			return process(vm, mmbuffer,cb,11.01, 22.01, 33.01, 44.01);
		}();

		

		

		double r = result;
		std::cout << "result = " << r << "\n";
		std::cout << "map.size()= " << map.size() << "\n";

	}
	else {

		
		//  CHILD PROCESS EXECUTING.....

		caller_t mo = (jsvalue_t)parent[0];
		jsvalue_t mmbuf = parent[1];

		caller_t callback= parent[2];

		
		int len = parent["length"];

		auto key = mo["Key"];
		
		for (auto k = 1; k < len; k++) {
			mo[k] = (jsvalue_t)parent[k];
		}

		
		
		i_mm_region_ptr::ptr_t<double> rgn(mmbuf);
		double* pf = rgn;
		auto  pvc = utils::pN_t<16>::ptr(pf);		

		for (auto k = 0; k < rgn.size(); k++) {
			pf[k] = 3 * pf[k] + 0.01;
		}

		auto r= callback("arg1", "arg2", "arg3", "arg4");
		parent["result"] = r;

	}

}
