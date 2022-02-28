#pragma once
//ltx_call_helper.h

#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>

#include <windef.h>
#include <comdef.h>
#include <shellapi.h>
#include <dispex.h>

#include "callback_context_arguments.h"


#include "call_functor_reverse.h"

#include "ipc_utils_base.h"

#define OLE_CHECK_ZERO(a) if(FAILED((a))) return 0;

#define OLE_CHECK_PTR(p) if(!p) return E_POINTER;
#define OLE_CHECK_VOID(a) if(FAILED((a))) return;
#define OLE_CHECK_hr(a) if(FAILED( hr=(a) ) ) return hr;
#define OLE_CHECK(a) ;{ HRESULT hr__0000;if(FAILED( hr__0000=(a) ) ) return hr__0000;};


namespace ltx_helper {


	
	
	struct exceptinfo_t: EXCEPINFO
	{
		exceptinfo_t():EXCEPINFO(){};
		//{ memset(this,0,sizeof(exceptinfo_t));}
		~exceptinfo_t()	{
			_free();
			_free_errinfo();
		};



		inline exceptinfo_t* clear()
		{
			 _free();
			 _free_errinfo();
			*((EXCEPINFO*)this)=EXCEPINFO();			 
			return this;
		}
	
      IErrorInfo* ErrorInfo(HRESULT hr) {
		  
		  if(hr!=DISP_E_EXCEPTION)
			  return 0;

		  _free_errinfo();

		  ICreateErrorInfo* cei=0;
	
		  bool f;

		  if(FAILED(CreateErrorInfo(&cei)))
			  return 0;

		  f=   SUCCEEDED(cei->SetSource(bstrSource));
		  f=f&&SUCCEEDED(cei->SetDescription(bstrDescription));
		  f=f&&SUCCEEDED(cei->SetHelpFile(bstrHelpFile));
		  f=f&&SUCCEEDED(cei->SetHelpContext(dwHelpContext));
		  f=f&&SUCCEEDED(cei->QueryInterface(__uuidof(IErrorInfo),(void**)&perrinfo));

		  cei->Release();
		  return perrinfo;			   
	  };


	protected:
		inline void _free()
		{
			SysFreeString( bstrSource);
			SysFreeString(bstrDescription);
			SysFreeString(bstrHelpFile);
		}

		inline void _free_errinfo()
		{
			if(perrinfo){
			 perrinfo->Release();
			 perrinfo=0;
			}		
		}

	  IErrorInfo* perrinfo;

	};




	template <class Variant_T,bool FREVERSE_ARGS=true>
	struct arguments_t
	{


		
		typedef Variant_T var_t;
		bool reverse_args;
		int argc;
		var_t* argv;
		HRESULT hr;
		pcallback_context_arguments_t pcallback_context;
		var_t vempty;

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
        	vempty=var_t();
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


		arguments_t(int _argc,var_t* _argv,int offset=0):argc((_argv)?_argc-offset:0),argv(_argv+offset),reverse_args(FREVERSE_ARGS),pcallback_context(0){
           vempty=var_t();
		};
		arguments_t(DISPPARAMS* pdp):reverse_args(true),argc(0),pcallback_context(0){
			vempty=var_t();
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

		inline void raise_error(const wchar_t* message,const wchar_t* source=0)
		{
			if(pcallback_context) pcallback_context->raise_error((wchar_t*)message,(wchar_t*)source);
 
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

		inline DISPID id()
		{
            return pcallback_context->tss_id.id;

		}
		inline BSTR pm_name()
		{
			callback_context_arguments_t::disp_id_t& tid=pcallback_context->tss_id;
			return (tid.id==tid.TSSid)?tid.tssName:0;		

		}

		inline bool check_index(INT64* pint=0)
		{
			callback_context_arguments_t::disp_id_t& tid=pcallback_context->tss_id;
			int l;

			if((tid.id==tid.TSSid)&&(l=SysStringLen(tid.tssName)))
			{				
                  INT64 r;
				  wchar_t* ptail=0;
				  r=_wcstoi64(tid.tssName,&ptail,0);				  
				 if(ptail==(tid.tssName+l)) 
				 {
					 if(pint) *pint=r;
					 return true;
				 }
			}
			 return false;
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
			return  vempty;       
		}



	};


	
	template <DWORD Flags>
	inline variant_t wrap_callback(const void* callback,const void* context=0,const void* onexit=0)
	{
		   HRESULT hr;
		   
            variant_t r;		
			ICallbackDispatchFactory* callbackFactory=0;
			
			hr=::CoGetObject(bstr_t(L"ltx.bind:callback.factory"),0,__uuidof(ICallbackDispatchFactory),(void**)&callbackFactory);		 
			if(SUCCEEDED(hr))
			{
				hr=callbackFactory->CreateInstanceV(FLAG_MASK_PUT|(Flags<<16),(void*)callback,(void*)context,(void*)onexit,&r);			  		 
				callbackFactory->Release();

			}
			if(FAILED(hr)) return variant_t(hr,VT_ERROR);
			
			return r;

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
		 ipc_utils::smart_ptr_t<ICallbackDispatchFactory> callbackFactory;
		 hr=::CoGetObject(bstr_t(L"ltx.bind:callback.factory"),0,__uuidof(ICallbackDispatchFactory),callbackFactory.ppQI());		 
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

	template <class F>
	struct i_mbv_buffer_helper_t
	{
		typedef i_mbv_buffer_ptr::int64_t int64_t;

		F* ptr;
		int64_t count;
		int64_t byteLength;
		shared_mem_data_t* smd;
		HRESULT hr;

		i_mbv_buffer_helper_t(VARIANT vbuffer):buffer(0),hr(E_POINTER)
		{
			VARIANT t={};
			OLE_CHECK_VOID(hr=VariantChangeType(&t,&vbuffer,VARIANT_NOVALUEPROP,VT_UNKNOWN));
			new (this) i_mbv_buffer_helper_t(t.punkVal);
			VariantClear(&t);
		}

		i_mbv_buffer_helper_t(IUnknown* unk):buffer(0),hr(E_POINTER)
		{
			if(unk)
			{
				//buffer->AddRef();
				OLE_CHECK_VOID(hr=unk->QueryInterface(__uuidof(i_mbv_buffer_ptr),(void**)&buffer));


				OLE_CHECK_VOID(hr=buffer->GetPtr((void**)&ptr,&(byteLength=0)));		 
				count=byteLength/sizeof(F);
				if(SUCCEEDED(hr=buffer->GetContext((void**)&smd))&&(smd->element_size!=sizeof(F)))
				hr=E_INVALIDARG;
				if(SUCCEEDED(hr))
					  buffer->Lock();

			}
		};

		~i_mbv_buffer_helper_t(){
			if(buffer)
			{
				buffer->Unlock();
				buffer->Release();
			}
		}
		inline F& operator[](int64_t k)
		{
			return ptr[k];    
		}

		//i_mbv_buffer_ptr::locker_t lock;
		i_mbv_buffer_ptr* buffer;
	protected:

		i_mbv_buffer_helper_t(i_mbv_buffer_helper_t& p){};
		i_mbv_buffer_helper_t(const i_mbv_buffer_helper_t& p){};
	};




	template <class O>
	HRESULT wrapObject(O* o,VARIANT* pvunk)
	{
		if(!pvunk) return E_POINTER;
		   HRESULT hr;
		   VARIANT v={VT_UNKNOWN};
		   if(SUCCEEDED(hr=wrapObject(o,&v.punkVal)))
			   *pvunk=v;
			   return hr;
	}     

	template <class O>
	HRESULT wrapObject(O* o,IUnknown** ppOut,DWORD flags=FLAG_CBD_FTM|FLAG_CBD_EC)
	{
		   
		struct deleter_t
		{
			static		void  s_on_exit( O* po)
			{
				delete po;
			}
		};

		if(!ppOut) return E_POINTER;
		HRESULT hr;


		ipc_utils::smart_ptr_t<ICallbackDispatchFactory> callbackFactory;
		hr=::CoGetObject(bstr_t(L"ltx.bind:callback.factory"),0,__uuidof(ICallbackDispatchFactory),callbackFactory.ppQI());		 
		if(SUCCEEDED(hr))
			hr=callbackFactory->CreateInstance(flags<<16,0,(void*)o,&deleter_t::s_on_exit,__uuidof(IUnknown),(void**)ppOut);			  		 

	};         
     


  template <class O>
   HRESULT unwrapObject(IUnknown* punk,O **ppo)
   {   

	   HRESULT hr=E_POINTER;
	   punk&&ppo&&(hr=punk->QueryInterface(GUID_NULL,(void**)ppo));        	   
	   return hr;

   }

   template <class O>
   HRESULT unwrapObject(VARIANT v,O **ppo)
   {   

	   HRESULT hr=E_INVALIDARG;
	   if((v.vt==VT_UNKNOWN)||(v.vt==VT_DISPATCH))
	   hr=unwrapObject(v.punkVal,ppo);        	   
	   return hr;
   }


   


};//ltx_helper - namespace end