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

	typedef struct {} fake_exception;


	template <class _Exception>
	struct com_exception_helper_t
	{
		typedef _Exception ex_t;
		static HRESULT check_error(HRESULT hr){ return HRESULT_FROM_WIN32(hr);}

	};

	template <>
	struct com_exception_helper_t<com_exception>
	{
		typedef com_exception ex_t;
		static HRESULT check_error(HRESULT hr){ 
			  if(FAILED(hr)) _com_issue_error(hr);			
			;return hr;
		}


	};


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

	template< class ExcetionClass=fake_exception>
	struct dispcaller_t:invoker_base_t<dispcaller_t<ExcetionClass>,variant_t,variant_t,VARIANT>
	{
		typedef com_exception_helper_t<ExcetionClass> exception_helper_t;

		struct getter_setter_t
		{
			
          getter_setter_t(IDispatch* _disp,DISPID _id,HRESULT _hr):disp(_disp),id(_id),hr(_hr){};


		  variant_t & prop_get()
		  {
              if(SUCCEEDED(exception_helper_t::check_error(hr)))
			  {
				  DISPPARAMS dps={};
				  WORD wFlags=DISPATCH_PROPERTYGET;
				   hr=disp->Invoke(id,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,&vget,0,0);
				  exception_helper_t::check_error(hr);				  
			  }
			  return vget;

		  }

		   HRESULT  prop_put(variant_t & v)
		  {
			  if(SUCCEEDED(exception_helper_t::check_error(hr)))
			  {
				  DISPID did=DISPID_PROPERTYPUT;
				  DISPPARAMS dps={&v,&did,1,1};
				  WORD wFlags=DISPATCH_PROPERTYPUT;
				  hr=disp->Invoke(id,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,&vget,0,0);
				  exception_helper_t::check_error(hr);
			  }
			  return hr;
		  }




		  template <class N>
		  operator N() 
		  {		  				    
			  return prop_get();
		  }

		  inline static 	bool is_empty(const variant_t& v)
		  {
			  VARTYPE t=v.vt;
			  return (t==VT_NULL)||(t==VT_EMPTY)||(t==VT_ERROR);
		  }


 inline   HRESULT operator =(variant_t v)
		  {
			  return prop_put(v);
		  }



		  template <class N>
		  N def(N dv=N()) {
			  try     {

				   prop_get();
				  return (!is_empty(vget))?vget:dv;
			  }
			  catch(...){
			  }
			  return dv;
		  }


          IDispatch* disp;
		  DISPID id;
		  HRESULT hr;
		  variant_t vget;
		};
		

		HRESULT hr;
		dispcaller_t(IUnknown* punk,DISPID id=DISPID_VALUE):disp(0),dispid(id),hr(E_POINTER){

			(punk)&&(hr=punk->QueryInterface(__uuidof(IDispatch),disp._ppQI())); 
			exception_helper_t::check_error(hr);

		};
		//::new((void*)shared_heap<t>().allocate(sizeof(t))) t
		dispcaller_t(VARIANT v,DISPID id=DISPID_VALUE):disp(0),dispid(id),hr(E_POINTER){
			if((v.vt==VT_DISPATCH)||(v.vt==VT_UNKNOWN))
				::new(this) dispcaller_t(v.punkVal,id);
		}

		dispcaller_t(bstr_t moniker_string,DISPID id=DISPID_VALUE):disp(0),dispid(id),hr(E_POINTER){
			
			     hr=disp.CoGetObject(moniker_string);
                 exception_helper_t::check_error(hr);
				
		}

		inline 	res_t invoke_proc(int argc,argh_t* argv)
		{
			DISPPARAMS dps={argv,0,argc,0};
			WORD wFlags=(argc)? DISPATCH_METHOD:DISPATCH_METHOD|DISPATCH_PROPERTYGET;
			res_t r;
			hr=disp->Invoke(dispid,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,&r,0,0);
			exception_helper_t::check_error(hr);
			return r;

		} 


       getter_setter_t operator[](int i)
	   {
		    wchar_t s[128];
		   return this->operator [](bstr_t(_itow(i,s,10)));
	   }

		getter_setter_t operator[](bstr_t name)
		{
            smart_ptr_t<IDispatchEx> diex;
			HRESULT hr0;
		   DISPID id=0;
		   BSTR b=name;
		   if(SUCCEEDED(hr=disp->QueryInterface(__uuidof(IDispatchEx),diex._ppQI())))
		   {
			   hr=diex->GetDispID(name,fdexNameEnsure,&id);

		   }
		   else   hr=disp->GetIDsOfNames(IID_NULL,&b,1, LOCALE_USER_DEFAULT,&id);
		   return getter_setter_t(disp,id,hr);

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
			 hr=disp->Invoke(dispid,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,result.GetAddress(),exceptinfo.address(),0);
			}			
			return hr;

		} 
		res_t hr0;
        variant_t result;
		smart_ptr_t<IDispatch> disp;
		exceptinfo_t exceptinfo;
		DISPID dispid;
	};



	template< class ExcetionClass=fake_exception>
	struct dispcaller_plus_t:invoker_base_t<dispcaller_plus_t<ExcetionClass>,variant_t,variant_t,VARIANT>
	{
		typedef com_exception_helper_t<ExcetionClass> exception_helper_t;

		dispcaller_plus_t(IDispatch* pdisp,BSTR txt,HRESULT* _phr):disp(pdisp),script_text(txt),fcomplete(false),phr(_phr){};
		inline 	res_t invoke_proc(int argc,argh_t* argv)
		{
			HRESULT& hr=*phr;
			fcomplete=true;
			argv[argc].vt=VT_BSTR;
			argv[argc].bstrVal=script_text;
			DISPPARAMS dps={argv,0,argc+1,0};
			WORD wFlags=(argc)? DISPATCH_METHOD:DISPATCH_METHOD|DISPATCH_PROPERTYGET;
			res_t r;
			hr=disp->Invoke(DISPID_VALUE,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,&r,0,0);
			exception_helper_t::check_error(hr);
			return r;

		} 

		inline dispcaller_plus_t& operator<<(bstr_t txt) 
		{
			//if(!buf.length()) buf=script_text;
			//buf+=L"\n"+txt;
			//script_text=buf;
			script_text+=L"\n"+txt;
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
			script_text=d.script_text;
			fcomplete=d.fcomplete;
			phr=d.phr;
		//	buf=d.buf;
			d.fcomplete=true;
		}
		~dispcaller_plus_t()
		{

			if(!fcomplete) invoke_proc();
		}

		IDispatch* disp;
		bstr_t script_text;
		bool fcomplete;
		HRESULT* phr;
		//bstr_t buf;

	};

	template< class ExcetionClass=fake_exception>
	struct delay_t
	{ 

            typedef com_exception_helper_t<ExcetionClass> exception_helper_t;

		inline variant_t operator()(bstr_t stxt) 
		{
			
			argv[argc].bstrVal=stxt;
			argv[argc].vt=VT_BSTR;
			DISPPARAMS dps={argv,0,argc+1,0};
			WORD wFlags=(argc)? DISPATCH_METHOD:DISPATCH_METHOD|DISPATCH_PROPERTYGET;
			variant_t r;
			*phr=disp->Invoke(DISPID_VALUE,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,&r,0,0);
			exception_helper_t::check_error(*phr);
			
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

	template< class ExcetionClass=fake_exception>
	struct dispcaller_delay_t:invoker_base_t<dispcaller_delay_t<ExcetionClass>,variant_t,delay_t<ExcetionClass>>
	{

		dispcaller_delay_t(IDispatch* pdisp,HRESULT* _phr):disp(pdisp),phr(_phr){};
		inline 	delay_t<ExcetionClass> invoke_proc(int argc,argh_t* argv)
		{
			//VARIANT vargv[32];
			//for(int k=0;k<argc;k++) vargv[k]=argv[k];
			//delay_t d={argc,vargv,disp,phr};
			return delay_t<ExcetionClass>(argc,argv,disp,phr);
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

    template< class ExcetionClass=fake_exception,DWORD _COINIT_DEFAULT=COINIT_MULTITHREADED>
	struct scriptcaller_t
	{
		  typedef variant_t value_t;
		  typedef ExcetionClass exception_t;
          typedef com_exception_helper_t<ExcetionClass> exception_helper_t;
		  typedef dispcaller_plus_t<ExcetionClass> dispcaller_plus_t;
		  typedef dispcaller_delay_t<ExcetionClass> dispcaller_delay_t;
		  typedef dispcaller_t<ExcetionClass> dispcaller_t;

		HRESULT hr;


		scriptcaller_t(const scriptcaller_t& sc):disp(0),arguments(0,0),hr(E_POINTER){
			disp=sc.disp;                
			arguments.disp=disp;
			arguments.phr=&hr;
		}

		scriptcaller_t(scriptcaller_t& sc):disp(0),arguments(0,0),hr(E_POINTER){
            disp=sc.disp;                
			arguments.disp=disp;
			arguments.phr=&hr;
		}

		scriptcaller_t(IUnknown* punk):disp(0),arguments(0,0),hr(E_POINTER){

			(punk)&&(hr=punk->QueryInterface(__uuidof(IDispatch),disp._ppQI())); 
			exception_helper_t::check_error(hr);
			arguments.disp=disp;
			arguments.phr=&hr;

		};

		scriptcaller_t(bstr_t moniker_url=L"ltx.bind:script: imports=[sm_buffer_helper]"):disp(0),arguments(0,0){
               
			comdef.init(_COINIT_DEFAULT);
			hr=CoGetObject(moniker_url,0,__uuidof(IDispatch),disp._ppQI()); 
			exception_helper_t::check_error(hr);
			//hr=disp.CoGetObject(moniker_url);
            arguments.disp=disp;
			arguments.phr=&hr;
		};


		//inline 
			dispcaller_plus_t operator()(bstr_t parse_script)
		{

			//dispcaller_plus_t d(disp.p,parse_script,&hr);
			//return d;
			return dispcaller_plus_t(disp.p,parse_script,&hr);

		}

		inline dispcaller_plus_t operator<<(bstr_t parse_script) 
		{
			return dispcaller_plus_t(disp.p,parse_script,&hr);
			
		}

		/*
		template <class V>
		variant_t to_array(std::vector<V>& v)
		{
			int l=v.size();
			 variant_t r=to_array(l,(l)?(variant_t*)&v[0]:0);
			   return r;
		}
		variant_t to_array(int length=0,variant_t* pv=0)
		{
			if(to_array_func.vt==VT_EMPTY)
			{
				to_array_func=arguments()<<js_text(
					( function ( ){
						   return function to_JSArray(){
							 return  Array.apply(null ,arguments);                           
						   }

				         }
					)()
					);

			}

			return dispcaller_t<exception_t>(to_array_func).invoke(length,pv);

		}
		*/


		//template <class O>
		//HRESULT set_dispatch_wrapper(bstr_t name,O* o,flags)

        COMInit_t comdef;
		dispcaller_delay_t arguments;
		smart_ptr_t<IDispatch> disp;
		//variant_t to_array_func;
		
	};

    
	typedef scriptcaller_t<com_exception>  com_scriptor_t;
	typedef dispcaller_t<com_exception>  com_dispcaller_t;
	

}//ipc_utils






