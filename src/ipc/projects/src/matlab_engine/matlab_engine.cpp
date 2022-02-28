// matlab_engine.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"


#include "matlab_engine.h"
#include "ipc_ports/ltx_js.h"
#include "ipc_ports/ltx_call_helper.h"
#include "ipc_ports/ipc_utils_base.h"

typedef ltx_helper::arguments_t<variant_t> arguments_t;


bool gf=matlab_engine_t<>::init_once();

///*
engine_mutex_t g_mutex;

struct matlab_init_t{
	matlab_init_t(){
_MCLCONVERSION_FLAGS flags={};
InitConversionFlags(&flags);
	}
} g_matlab_init;
//*/

struct matlab_engine_dispatch_t: ltx_helper::dispatch_wrapper_t<matlab_engine_dispatch_t,variant_t>
{

	inline  void  operator()
		( com_variant_t& result,pcallback_context_arguments_t pcca,int argc,com_variant_t* argv,int arglast,HRESULT& hr,void* pei,unsigned flags)
	{



		arguments_t arg(pcca);
		
        int l=arg.length();
		int flag=arg[1].def<int>(1);

		if(l)
		{
         bstr_t text=arg[0].def<bstr_t>(L"");
         OLE_CHECK_VOID(arg.hr= engine.execute((wchar_t*)text));   
		}
       
        

		if(flag)
		{

	              arg.hr= engine.get_msg_buffer(&result);			

			return;
		}
		




	};

	//	matlab_engine_dispatch_t():engine(g_mutex,L"ooo"){}
	//
	matlab_engine_dispatch_t(bstr_t objref=bstr_t()){
		engine.init(objref);
	}

	matlab_engine_t<> engine;
};





extern "C"  void  engine_factory
( variant_t* presult, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
{
	//ipc_utils::smart_ptr_t<i_stub_holder> stub;
	//OLE_CHECK_VOID(hr=plib->bind_object(L"stub_holder:",__uuidof(i_stub_holder),stub._ppQI()));
	arguments_t args(pcca);
	bstr_t objref=args[0];
	hr=(new matlab_engine_dispatch_t(objref))->wrap(args,presult,MAKELONG(FLAG_MASK_PUT,FLAG_CBD_EC));



}


