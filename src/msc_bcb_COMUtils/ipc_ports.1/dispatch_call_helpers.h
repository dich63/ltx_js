#pragma once
//#include "dispatch_call_helpers.h"


#include "call_functor_reverse.h"
//#include "ltx_call_helper.h"
#include "ipc_utils_base.h"

#define __Lpn(a) L###a
#define _Lpn(...) __Lpn(__VA_ARGS__)

#define js_text(...) __Lpn(__VA_ARGS__)
//#define js_script(...) operator()(__Lpn(__VA_ARGS__))



namespace ipc_utils {


	typedef _com_error com_exception;


	struct res_str_t
	{
		  bstr_t str;
		  inline static HINSTANCE get_hinst()
		  {
			  struct s_t: ltx_helper::dispatch_wrapper_t<s_t,VARIANT>{};
			  return s_t::get_hinstance();
		  }
		  res_str_t(wchar_t* resid,HINSTANCE hinst=get_hinst(),wchar_t* rtype=MAKEINTRESOURCEW(23))   
		  {
              init(resid,hinst,rtype);
		  }

     res_str_t& init(wchar_t* resid,HINSTANCE hinst=get_hinst(),wchar_t* rtype=MAKEINTRESOURCEW(23))   
		 {
			 char* p,*pz;
             HRSRC hrs=FindResourceW(hinst,resid,rtype);  
			 if(hrs) 
			 {
				 HGLOBAL hg= LoadResource(hinst,hrs);
				 int cb=SizeofResource(hinst,hrs);
				 // m_cb=LocalSize(hg);
				 if(hg)
					p=(char*)LockResource(hg);
				 if(p)
				 {
				  pz=(char*)calloc(cb+1,1);
				  memcpy(pz,p,cb);
				  str=pz;
				  free(pz);
				  UnlockResource(hg);
				 }

			 }		 
			 return *this;

		 }

		inline  operator bstr_t(){return str;}

	};

	struct dispcaller_t:invoker_base_t<dispcaller_t,variant_t,variant_t,VARIANT>
	{
		HRESULT hr;
		dispcaller_t(IDispatch* pdisp,DISPID id=DISPID_VALUE):disp(pdisp),dispid(id){};
		inline 	res_t invoke_proc(int argc,argh_t* argv)
		{
			DISPPARAMS dps={argv,0,argc,0};
			WORD wFlags=(argc)? DISPATCH_METHOD:DISPATCH_METHOD|DISPATCH_PROPERTYGET;
			res_t r;
			hr=disp->Invoke(dispid,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,&r,0,0);
			return r;

		} 

		smart_ptr_t<IDispatch> disp;
		DISPID dispid;
	};


	struct disp_call_t:invoker_base_t<disp_call_t,variant_t,HRESULT,VARIANT>
	{
		
		disp_call_t(IDispatch* pdisp,DISPID id=DISPID_VALUE):disp(pdisp),dispid(id){
			
			hr0=(disp)?S_OK:E_POINTER;
		};

        disp_call_t(IDispatch* pdisp,bstr_t name):disp(pdisp),dispid(-1){

            hr0=(disp)?S_OK:E_POINTER;
		   if(SUCCEEDED(hr0))  {
			     LPOLESTR lpsz=(LPOLESTR)name;			   
				   hr0=disp->GetIDsOfNames(IID_NULL, &lpsz, 1, LOCALE_USER_DEFAULT,&dispid);
		   }


		};


		inline 	res_t invoke_proc(int argc,argh_t* argv)
		{
			HRESULT hr=hr0;
			if(SUCCEEDED(hr))
			{			
			 DISPPARAMS dps={argv,0,argc,0};
			 WORD wFlags=(argc)? DISPATCH_METHOD:DISPATCH_METHOD|DISPATCH_PROPERTYGET;
			 res_t hr;
			 hr=disp->Invoke(dispid,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,result.GetAddress(),0,0);
			}			
			return hr;

		} 
		res_t hr0;
        variant_t result;
		smart_ptr_t<IDispatch> disp;
		DISPID dispid;
	};



	struct dispcaller_plus_t:invoker_base_t<dispcaller_plus_t,variant_t,variant_t,VARIANT>
	{

		dispcaller_plus_t(IDispatch* pdisp,BSTR txt,HRESULT* _phr):disp(pdisp),stxt(txt),fcomplete(false),phr(_phr){};
		inline 	res_t invoke_proc(int argc,argh_t* argv)
		{
			fcomplete=true;
			argv[argc].vt=VT_BSTR;
			argv[argc].bstrVal=stxt;
			DISPPARAMS dps={argv,0,argc+1,0};
			WORD wFlags=(argc)? DISPATCH_METHOD:DISPATCH_METHOD|DISPATCH_PROPERTYGET;
			res_t r;
			*phr=disp->Invoke(DISPID_VALUE,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,&r,0,0);
			return r;

		} 

		inline dispcaller_plus_t& operator<<(bstr_t txt) 
		{
			if(!buf.length()) buf=stxt;
			buf+=L"\n"+txt;
			stxt=buf;
			return *this;
		}


		inline variant_t invoke_proc()
		{
			VARIANT tmp={}; 
			return  invoke_proc(0,(argh_t*)&tmp);
		}

		inline operator variant_t(){ 
			return invoke_proc();
		}
		template <class N>
		inline operator N(){ 
			return (N)invoke_proc();
		}

		dispcaller_plus_t(dispcaller_plus_t& d)
		{
			disp=d.disp;
			stxt=d.stxt;
			fcomplete=d.fcomplete;
			phr=d.phr;
			buf=d.buf;
			d.fcomplete=true;
		}
		~dispcaller_plus_t()
		{

			if(!fcomplete) invoke_proc();
		}

		IDispatch* disp;
		BSTR stxt;
		bool fcomplete;
		HRESULT* phr;
		bstr_t buf;

	};


	struct delay_t
	{ 



		inline variant_t operator()(bstr_t stxt) 
		{
			
			argv[argc].bstrVal=stxt;
			argv[argc].vt=VT_BSTR;
			DISPPARAMS dps={argv,0,argc+1,0};
			WORD wFlags=(argc)? DISPATCH_METHOD:DISPATCH_METHOD|DISPATCH_PROPERTYGET;
			variant_t r;
			*phr=disp->Invoke(DISPID_VALUE,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,&r,0,0);
			return r;

		}

		inline variant_t operator<<(bstr_t stxt) 
		{
			return this->operator()(stxt);
		}

		delay_t(){};
		delay_t(int c,VARIANT* pv,IDispatch* p,HRESULT* _phr):argc(c),disp(p),phr(_phr)
		{
			for(int k=0;k<argc;k++) argv[k]=pv[k];
		};
		int argc;
		VARIANT argv[32];
		IDispatch* disp;
		HRESULT* phr;
	};

	struct dispcaller_delay_t:invoker_base_t<dispcaller_delay_t,variant_t,delay_t>
	{

		dispcaller_delay_t(IDispatch* pdisp,HRESULT* _phr):disp(pdisp),phr(_phr){};
		inline 	delay_t invoke_proc(int argc,argh_t* argv)
		{
			//VARIANT vargv[32];
			//for(int k=0;k<argc;k++) vargv[k]=argv[k];
			//delay_t d={argc,vargv,disp,phr};
			return delay_t(argc,argv,disp,phr);
		} 



		IDispatch* disp;	
		HRESULT* phr;

	};




	template <class _Variant>
	struct result_t
	{
		typedef result_t<_Variant> self_t;
		typedef _Variant var_t;
		HRESULT hri;  
		const var_t& v;  
		result_t(const  var_t& _v,HRESULT hr=0):v(_v),hri(hr){};

		operator  const  var_t&() const 
		{
			return v;
		}
		template <class N>
		operator N() 
		{
			return v;
		}

		inline 	bool is_empty()
		{
			VARTYPE t=v.vt;
			return (t==VT_NULL)||(t==VT_EMPTY)||(t==VT_ERROR);
		}


		template <class N>
		N def(N dv=N()) {
			try     {
				return (!is_empty())?v:dv;
			}
			catch(...){
			}
			return dv;
		}

		template<class Intf>
		HRESULT QueryInterface(Intf** ppObj)
		{
			HRESULT hr;
			return hr=QueryInterface(__uuidof(Intf),(void**)ppObj); 
		}

		HRESULT QueryInterface(REFIID riid, void** ppObj)
		{
			HRESULT hr;
			var_t tmp;
			if(SUCCEEDED(hr=VariantChangeType(&tmp,&v,0,VT_UNKNOWN))) 
			{ 
				hr=tmp.punkVal->QueryInterface(riid,ppObj);
			}
			return hr; 
		}

	};


	struct scriptcaller_t
	{
		HRESULT hr;
		scriptcaller_t(IDispatch* pdisp):disp(pdisp),arguments(pdisp,0){
			arguments.phr=&hr;
		};

		scriptcaller_t(bstr_t moniker_url=L"ltx.bind:script:"):disp(0),arguments(0,0){
              
            
			hr=CoGetObject(moniker_url,0,__uuidof(IDispatch),disp._ppQI()); 
			//hr=disp.CoGetObject(moniker_url);
            arguments.disp=disp;
			arguments.phr=&hr;
		};


		inline dispcaller_plus_t operator()(bstr_t parse_script)
		{

			return dispcaller_plus_t(disp.p,parse_script,&hr);

		}

		inline dispcaller_plus_t operator<<(bstr_t parse_script) 
		{
			return dispcaller_plus_t(disp.p,parse_script,&hr);
		}


		dispcaller_delay_t arguments;
		smart_ptr_t<IDispatch> disp;
	};



}//ipc_utils






