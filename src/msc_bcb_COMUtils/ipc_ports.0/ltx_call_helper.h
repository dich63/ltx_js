#pragma once
//ltx_call_helper.h

#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>

#include <windef.h>
#include <comdef.h>
#include <shellapi.h>
#include <dispex.h>
#include "ipc_utils_base.h"
#include "callback_context_arguments.h"


 #include "call_functor_reverse.h"



namespace ltx_helper {


	

	template <class Variant_T,bool FREVERSE_ARGS=true>
	struct arguments_t
	{


		
		typedef Variant_T var_t;
		bool reverse_args;
		int argc;
		var_t* argv;
		HRESULT hr;
		pcallback_context_arguments_t pcallback_context;

		struct V_C_t:VARIANT
		{
			
			V_C_t(VARIANT _v=VARIANT()){
				*((VARIANT*)this)=_v;
			};
			~V_C_t(){
				VariantClear(this);
			}

		};

		inline void init_context(void* p)
		{
			pcallback_context=pcallback_context_arguments_t(p);
			if(!p) return;
            argc=pcallback_context->argc;   
			argv=(var_t*)pcallback_context->argv;	
			reverse_args=!pcallback_context->fmaskreverse;

		}
		arguments_t(pcallback_context_arguments_t  p):pcallback_context(0),hr(0)
		{
           init_context(p);
		}
		;

		inline WORD	flags(){

			return (pcallback_context)?pcallback_context->flags:0;
		}

		pcallback_lib_arguments_t operator  ->() const
		{
			return pcallback_context->pclib;
		}


		arguments_t(int _argc,var_t* _argv,int offset=0):argc((_argv)?_argc-offset:0),argv(_argv+offset),reverse_args(FREVERSE_ARGS),pcallback_context(0){};
		arguments_t(DISPPARAMS* pdp):reverse_args(true),argc(0),pcallback_context(0){
			if(!pdp) return;
           argc=pdp->cArgs;
		   argv=(var_t*)pdp->rgvarg;
		};

		~arguments_t()
		{
			if(pcallback_context) *pcallback_context->phr=hr;
		}

		inline int length(){return argc;};


		inline 	static var_t& verror()
		{
			static var_t sv(DISP_E_PARAMNOTFOUND,VT_ERROR);
			return sv;
		}


		inline var_t& result()
		{
			return *( (var_t*)pcallback_context->result);
		}
		inline bool is_get_flag()
		{
			return is_get(pcallback_context->flags);
		}
		inline bool is_put_flag()
		{
			return is_put(pcallback_context->flags);
		}


		struct  arg_t
		{
			const var_t& v;  
			arg_t(const  var_t& _v):v(_v){};

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
				
				V_C_t tmp;
				if(SUCCEEDED(hr=VariantChangeType(&tmp,&v,0,VT_UNKNOWN))) 
				{ 
					hr=(tmp.punkVal)?tmp.punkVal->QueryInterface(riid,ppObj):E_POINTER;
				}
				return hr; 
			}

			HRESULT QueryInterface(REFIID riid)
			{
				HRESULT hr;
				smart_ptr_t<IUnknown> unk;
				return hr=QueryInterface(riid,unk._ppQI());
			}

		};

		arg_t operator[](int n)
		{
			if((0<=n)&&(n<argc))
				return (reverse_args)?argv[argc-n-1] :argv[n];         
			//return  verror();       
			return  var_t();       
		}

	};


	template <class T,class VariantType> 
	struct dispatch_wrapper_t
	{
		typedef VariantType com_variant_t;
		typedef typename arguments_t<com_variant_t> arguments_t;
		static		void  s_on_exit( T* pthis){
			delete pthis;
		}

		inline  void  on_get(com_variant_t& result,arguments_t& arguments){};
		inline  void  on_put(com_variant_t& value,arguments_t& arguments){};
		inline  void  oncallback(com_variant_t& result,arguments_t& arguments){
			   if(arguments.is_get_flag())
				   static_cast<T*>(this)->on_get(result, arguments);
			   	    else static_cast<T*>(this)->on_put(result, arguments);
		};


		inline  void  operator()
			( com_variant_t& result,pcallback_context_arguments_t pcca,int argc,com_variant_t* argv,int arglast,HRESULT& hr,void* pei,unsigned flags){
             
				  arguments_t arguments(pcca);
				static_cast<T*>(this)->oncallback(result,arguments);  
		}
		       

		static		void  s_callback
			( com_variant_t& result, T* pthis,pcallback_context_arguments_t pcca,int argc,com_variant_t* argv,int arglast,HRESULT& hr,void* pei,unsigned flags){

				(*pthis)(result,pcca,argc,argv,arglast,hr,pei,flags);

		}

	template <class Args>
     HRESULT wrap(const Args& args,VARIANT* pres,DWORD flags=FLAG_MASK_PUT)
	 {
		 HRESULT hr;
         return    hr= args->create_callback(flags,&dispatch_wrapper_t::s_callback,static_cast<T*>(this),&dispatch_wrapper_t::s_on_exit,pres);
	 }

	 
	 HRESULT wrap(VARIANT* pres,DWORD flags=FLAG_MASK_PUT)
	 {
		 HRESULT hr;
		 if(!pres) return E_POINTER;
		 smart_ptr_t<ICallbackDispatchFactory> callbackFactory;
		 //DWORD fl=MAKELONG()
		 hr=::CoGetObject(bstr_t("ltx.bind:callback.factory"),0,__uuidof(ICallbackDispatchFactory),callbackFactory.ppQI());		 
		 if(SUCCEEDED(hr))
		 		 hr=callbackFactory->CreateInstanceV(flags,&dispatch_wrapper_t::s_callback,static_cast<T*>(this),&dispatch_wrapper_t::s_on_exit,pres);			  		 
	 }

     template <class Calback_Aggregator>   
     HRESULT wrap(Calback_Aggregator* pfactory,VARIANT* pres,DWORD flags=FLAG_MASK_PUT)// =calback_aggregator_t<IDispatch> 
	 {
		       HRESULT hr;
              return hr=Calback_Aggregator::CreateInstanceV(flags,&dispatch_wrapper_t::s_callback,static_cast<T*>(this),&dispatch_wrapper_t::s_on_exit,pres);
	 }


	   static HINSTANCE get_hinstance(void* ptr=&s_callback)
	   {
		   HINSTANCE h=HINSTANCE(-1);
		   GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)ptr,&h);
		   return h;
	   }

	};

};
