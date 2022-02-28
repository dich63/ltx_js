// testdl.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"


#include "ipc_ports/ltx_js.h"
#include "ipc_ports/ltx_call_helper.h"
#include "dummy_dispatch_aggregator.h"

#include <map>
#include <string>
#include <sstream>

typedef ltx_helper::arguments_t<variant_t> arguments_t;

extern "C"  void __cdecl  wraptest
( variant_t& result, void* pltx,pcallback_context_arguments_t pcca,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
{
	//AllocConsole();
	try
	{


		bstr_t r=" {";

		for(int i=0;i<argc;i++)
		{
			r+=bstr_t(argv[i])+bstr_t(",");
		}
		r+="}";
		result=r;
	}
	catch(...)
	{
		pcca->raise_error(L"Exception ÕÐÓÌ ÕÐßÑÚ!!!",0);
	}


};



extern "C"  void  wraptestObj
( variant_t* presult, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
{


	

	

	const int flag_get=(DISPATCH_GETSF)<<16,flag_put=(DISPATCH_PUTSF)<<16,flag_STA=2;

	struct variant_holder_t
	{
		typedef variant_t com_variant_t;

		//variant_holder_t(const arguments_t& args){}


		static		void  s_on_exit( variant_holder_t* pthis){
                delete pthis;
		}

		static		void  s_callback
			( com_variant_t& result, variant_holder_t* pthis,pcallback_context_arguments_t pcca,int argc,com_variant_t* argv,int arglast,HRESULT& hr,void* pei,unsigned flags){

				(*pthis)(result,pcca,argc,argv,arglast,hr,pei,flags);

		}

		inline  void  operator()
			( com_variant_t& result,pcallback_context_arguments_t pcca,int argc,com_variant_t* argv,int arglast,HRESULT& hr,void* pei,unsigned flags)
		{



			arguments_t arg(pcca);


			if(arg.length()==0)
			{
				std::wstringstream stream;

				std::map<std::wstring,com_variant_t>::iterator e=m_map.end(),i=m_map.begin();


				stream<<L"[";
				if(i!=e) stream<<L"'"<<(*i++).first<<"'";
				for(;i!=e;++i) stream<<L",'"<<(*i).first<<"'";
				stream<<L"]";

				result=variant_t(stream.str().c_str());

				return;
			}




			bstr_t name=arg[0];

			if(name.length()==0)
				pcca->raise_error(L"Ups!!==0",0);


			if(is_get(arg.flags()))  result=m_map[(wchar_t*)name];
			else if(is_put(arg.flags())) m_map[(wchar_t*)name]=result;

		};

		std::map<std::wstring,com_variant_t> m_map;
	};




	try
	{
		arguments_t args(pcca);
		hr=args->create_callback(0,&variant_holder_t::s_callback,new variant_holder_t(),&variant_holder_t::s_on_exit,presult);
	

		//hr=plib->create_callback(0,&variant_holder_t::s_callback,new variant_holder_t(),&variant_holder_t::s_on_exit,presult);
	}
	catch(...)
	{
		pcca->raise_error(L"Exception ÕÐÓÌ ÕÐßÑÚ!!!",0);
	}

	
};


extern "C"  void  wraptestObj2
( variant_t* presult, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
{




	//typedef ltx_helper::arguments_t<variant_t> arguments_t;

	const int flag_get=(DISPATCH_GETSF)<<16,flag_put=(DISPATCH_PUTSF)<<16,flag_STA=2;

	struct variant_holder_t: ltx_helper::dispatch_wrapper_t<variant_holder_t,variant_t>
	{

		inline  void  operator()
			( com_variant_t& result,pcallback_context_arguments_t pcca,int argc,com_variant_t* argv,int arglast,HRESULT& hr,void* pei,unsigned flags)
		{



			arguments_t arg(pcca);


			if(arg.length()==0)
			{
				std::wstringstream stream;

				std::map<std::wstring,com_variant_t>::iterator e=m_map.end(),i=m_map.begin();


				stream<<L"[";
				if(i!=e) stream<<L"'"<<(*i++).first<<"'";
				for(;i!=e;++i) stream<<L",'"<<(*i).first<<"'";
				stream<<L"]";

				result=variant_t(stream.str().c_str());

				return;
			}




			bstr_t name=arg[0];

			if(name.length()==0)
				pcca->raise_error(L"Ups!!==0",0);


			if(is_get(arg.flags()))  result=m_map[(wchar_t*)name];
			else if(is_put(arg.flags())) m_map[(wchar_t*)name]=result;

		};

		std::map<std::wstring,com_variant_t> m_map;
	};




	try
	{
		arguments_t args(pcca);
		//hr=args->create_callback(0,&variant_holder_t::s_callback,new variant_holder_t(),&variant_holder_t::s_on_exit,presult);
		hr=(new variant_holder_t())->wrap(args,presult,0);
	
	}
	catch(...)
	{
		pcca->raise_error(L"Exception ÕÐÓÌ ÕÐßÑÚ!!!",0);
	}


};


extern "C"  void  wraptestObjQuick
( variant_t* presult, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
{




	//typedef ltx_helper::arguments_t<variant_t> arguments_t;

	

	struct dispQ_t:IDispatch_impl_base_t<dispQ_t,IUnknown_base_impl<dispQ_t,IDispatch> >
	{

		inline	HRESULT impl_Invoke(DISPID,REFIID,LCID,WORD,DISPPARAMS* disp,VARIANT* res,EXCEPINFO*, UINT*){ 
            
			VARIANT r={VT_I4};
            r.intVal=disp->cArgs;
			*res=r;
			return S_OK;
		}
	
	};




	try
	{
		//arguments_t args(pcca);
		//hr=args->create_callback(0,&variant_holder_t::s_callback,new variant_holder_t(),&variant_holder_t::s_on_exit,presult);
		//hr=(new variant_holder_t())->wrap(args,presult,0);
		VARIANT r={VT_DISPATCH};
         r.pdispVal=new dispQ_t;
		*presult=r;

	}
	catch(...)
	{
		pcca->raise_error(L"Exception ÕÐÓÌ ÕÐßÑÚ!!!",0);
	}


};


