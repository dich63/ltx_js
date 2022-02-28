// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <iostream>
#include "ltxjs.h"



#include <map>
#include <mutex>
#include <string>
#include <vector>

typedef ipc_utils::com_scriptor_t javascript_t;
//typedef ipc_utils::dispcaller_t<javascript_t::exception_t> caller_t;
typedef ipc_utils::com_dispcaller_t caller_t;
typedef javascript_t::value_t jsvalue_t;
typedef i_mbv_buffer_ptr::int64_t int64_t;
typedef ltx_helper::arguments_t<jsvalue_t>  arguments_t;

void *gpo;



struct ObjectMapExample_t :ltx_helper::dispatch_by_name_wrapper_t< ObjectMapExample_t, jsvalue_t, std::mutex>
{
	struct jsvalue_holder_t
	{
		jsvalue_holder_t() {}
		jsvalue_holder_t(jsvalue_t rv) { v = rv; }
		jsvalue_t v;
	};

	typedef std::map<std::wstring, jsvalue_holder_t> jsmap_t;

	inline  void  on_get(DISPID id, const wchar_t* name, com_variant_t& result, arguments_t& arguments, i_marshal_helper_t* helper) {

		//std::lock_guard<std::mutex> lock(mtx);

		if (!name)  arguments.raise_error(L"on_get ÍåÕðeí!!", L"ObjectMapExample_t");
		_cwprintf(L"on_get %s\n", name);
		result = jsmap[name].v;


	};
	inline  void  on_put(DISPID id, const wchar_t* name, com_variant_t& value, arguments_t& arguments, i_marshal_helper_t* helper) {
		//std::lock_guard<std::mutex> lock(mtx);
		if (!name)  arguments.raise_error(L"on_put ÍåÕðeí!!", L"ObjectMapExample_t");
		_cwprintf(L"on_put %s\n", name);
		jsmap[name] = jsvalue_holder_t(value);

	};
	ObjectMapExample_t() {
		gpo = (void*)this;
	};
	~ObjectMapExample_t() {
		
		jsmap.clear();
	}

	jsmap_t jsmap;
	//std::mutex mtx;

};




void child(caller_t& parent) {

	HRESULT hr;

	typedef ipc_utils::com_dispcaller_t caller_t;
	

	
	//caller_t parent = olestr_t("ltx.bind:external");

	

	int len = parent["length"];
	std::cout <<"len="<<len<< "\n";

	ipc_utils::smart_ptr_t< i_mm_region_ptr> sm;

	//hr = sm.reset(VARIANT(parent[1]));
	
	i_mm_region_ptr::ptr_t<double> pm(parent[1], -1);
	//i_mm_region_ptr::ptr_t<double> pm(parent[1].get(),-1);

	double* pf = pm;

	pf[0] = 7.777777e11;

	double r = 0;

	caller_t mo = VARIANT(parent[0]);

	jsvalue_t vsm = parent[1];

	mo["BB"] = L"AAA";
	mo["AA"] = vsm;

	for (auto k = 2; k < len; k++) {
		std::cout << "a[" << k<<"]="<< double(parent[k]) << "\n";
		r += double(parent[k]);
	}
	std::cout << "result=" << r << "\n";
	parent["result"] = r;
	
}

int test(int a, int b, int b3, int b4,const char* b5) {
	return a;
}

template<class G,typename... Arguments>
size_t tfunc(const Arguments&... args)
{

	constexpr auto numargs{ sizeof...(Arguments) };

	return test(args...);

}

//  Parent Process



void main(int argn, char** argv) {

	ipc_utils::COMInitF_t ci0;
	caller_t js= olestr_t("ltx.bind:srv:script");
	//js("require('utils').AllocConsole()");
	caller_t jj = ipc_utils::asyn_interact_js(js);
	/*
	caller_t jj("ltx.bind:script:");
	auto r=jj("aaaa=111");
	jj("r=require('fso')");
	jj("r.AllocConsole()");
	//
	*/
	ipc_utils::bstr_c_t<> bst(L"ABCDEFGERATATA");
	auto r = jj("aaa");
	bst = int(11002);
	r = jj("aaa");
	std::vector<variant_t> vvv;
	
	std::shared_ptr<std::mutex> smu(new std::mutex);

	std::shared_ptr<std::mutex> s2mu = smu;
	
	auto cuu=smu.use_count();

	size_t na = tfunc<int>(1, 2, 3, 4.44, "AA");

	HRESULT hr;

	ipc_utils::COMInitF_t ci;
	
	auto vsm0 = ipc_utils::bindObject("ltx.bind:mm_buffer:len=#1;type=#2")(1111, "double");
	auto sm= ipc_utils::bindObject<i_mm_region_ptr>("ltx.bind:mm_buffer:len=#1;type=#2")(1111, "double");

	bool ffork;
	/*
	try {		
		caller_t parent = olestr_t("ltx.bind:external");
		ffork = 1;
		
	}
	catch (...) {
		ffork = 0;
	}
	*/

	caller_t parent;

	ffork = ipc_utils::is_external(parent);

	if (ffork) {

		child(parent);
		return;

	}

	

	//	caller_t process = olestr_t("ltx.bind:process: file=notepad.exe");
	//	caller_t process = olestr_t("ltx.bind:process: file=")+ olestr_t(argv[0]);// dbg.hook=1");
	caller_t process = olestr_t("ltx.bind:process: file=*;CF=16;dbghook=1");// dbg.hook=1");

	jsvalue_t vsm = sm.toVARIANT(),o;

	{
		i_mm_region_ptr::ptr_t<double> lm(vsm);

		
		lm[0] = -13131313.0;
		

	}

	//jsvalue_t o = ipc_utils::wrap<ObjectMapExample_t>();
	//ObjectMapExample_t* mo=NULL,*mp=(ObjectMapExample_t*)gpo;

	auto  mo = new ObjectMapExample_t();

	//hr = ltx_helper::unwrapObject(o, &mo);

	hr = mo->wrap(&o);

	ObjectMapExample_t* mow = 0;

	hr = ObjectMapExample_t::unwrap(o, &mow);

	ipc_utils::smart_ptr_t<i_context> co;



	hr = co.reset(o);

	

	hr = co->GetContext((void**)&mow);

	//hr = ltx_helper::unwrapObject(o, &mow);



	caller_t cmo(o);

	variant_t result = process(o,vsm,1.01, 2.01, 3.01, 4.01);

	jsvalue_t a = cmo["AA"];
	i_mm_region_ptr::ptr_t<double> pm(sm.p, -1);

	auto fv = pm[0];
	std::cout << "pm[0] = "<< fv << "\n";
	std::cout << "result = " << double(result) << "\n";

	
	

	
}
	/*
int main(int argn, char** argv){
	HRESULT hr;
	//caller_t jc("ltx.bind:script:");
	ipc_utils::COMInitF_t ci;
	ipc_utils::smart_ptr_t<i_mm_region_ptr> sm;
	//CoGetObject

	hr = ::CoGetObject(olestr_t("ltx.bind:mm_buffer:len=7777;type=double"), NULL ,__uuidof(i_mm_region_ptr), (void**)&sm.p);
	//hr = ipc_utils::bindObject("ltx.bind:mm_buffer:len=#1;type=#2", sm)(1111,"double");

	caller_t smo(sm);

	int len = smo["length"];
	
	i_mm_region_ptr::ptr_t<double> pm(sm.p, -1);
	int at = ipc_utils::COMInit_t::apartment_type();
	caller_t jj = ipc_utils::interact_js();
    std::cout << "Hell...\n"; 

}
	*/
