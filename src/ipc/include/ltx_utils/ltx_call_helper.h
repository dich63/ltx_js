#pragma once
//ltx_call_helper.h

#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>

#include <windef.h>
#include <comdef.h>
#include <shellapi.h>
#include <dispex.h>
#include <list>
#include <memory>

#include "callback_context_arguments.h"


#include "call_functor_reverse.h"

#include "ipc_utils_base.h"



//#include <mutex>

#define OLE_CHECK_ZERO(a) if(FAILED((a))) return 0;

#define OLE_CHECK_PTR(p) if(!(p)) return E_POINTER;
#define OLE_CHECK_VOID(a) if(FAILED((a))) return;
#define OLE_CHECK_VOID_hr(a) if(FAILED(hr=(a))) return;
#define OLE_CHECK_VOID_hr_if(f,h) if(f){ hr=HRESULT_FROM_WIN32(h);    return;};
#define OLE_CHECK_VOID_hr_cond(f,h) if(!(f)){ hr=HRESULT_FROM_WIN32(h);    return;};
#define OLE_CHECK_hr_cond(f,h) if(!(f)){ hr=HRESULT_FROM_WIN32(h);    return hr;};
#define OLE_CHECK_hr(a) if(FAILED( hr=(a) ) ) return hr;
#define OLE_CHECK_WIN32_hr(a) if(FAILED( hr=HRESULT_FROM_WIN32(a) ) ) return hr;
#define OLE_CHECK(a) ;{ HRESULT hr__0000;if(FAILED( hr__0000=(a) ) ) return hr__0000;};


namespace ltx_helper {

	template <class I=IUnknown>
	struct i_unwrapper_t {
		HRESULT hr;

		template <class Intf>
		i_unwrapper_t(Intf* o) :hr(E_POINTER) {
			if (o) {
				hr = o->QueryInterface(__uuidof(I),ptr._ppQI());
			}
		}

		template <class Intf>
		i_unwrapper_t(ipc_utils::smart_ptr_t <Intf>& o) {

			new(this) i_unwrapper_t(o.p);
		}

		template <class O>
		i_unwrapper_t(O& o) {
			new(this) i_unwrapper_t(o.disp.p);
		}

		i_unwrapper_t(VARIANT v) :hr(E_INVALIDARG) {			
			if ((v.vt == VT_DISPATCH) || (v.vt == VT_UNKNOWN))
				::new(this) i_unwrapper_t(v.punkVal);
		}

		i_unwrapper_t(variant_t& v) {			
				::new(this) i_unwrapper_t(VARIANT(v));		}

		inline  I* operator->()
		{
			return ptr.p;
		}
		inline   operator I*()
		{
			return ptr.p;
		}

		inline   operator HRESULT() {
			return hr;
		}

		inline   operator bool() {
			return SUCCEEDED(hr);
		}

		ipc_utils::smart_ptr_t<I> ptr;
	};
	
	
	struct exceptinfo_t: EXCEPINFO
	{
		exceptinfo_t()
		{ memset(this,0,sizeof(exceptinfo_t));}

		~exceptinfo_t()	{
			_free();
			_free_errinfo();
		};

		exceptinfo_t(const exceptinfo_t&) :exceptinfo_t() {

		};

		inline exceptinfo_t* clear()
		{
			 _free();
			 _free_errinfo();
			*((EXCEPINFO*)this)=EXCEPINFO();			 
			return this;
		}
		inline EXCEPINFO* address() { return clear(); }
	
      IErrorInfo* ErrorInfo(HRESULT hr) {
		  
		  
		  if(this&&(hr==DISP_E_EXCEPTION))
		  {

		  

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
		  }
		  else return 0;

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

		inline IDispatch* self()
		{
			return pcallback_context->pthis;
		}

		inline long refcount()
		{
			self()->AddRef();
			return self()->Release();
		}
		inline WORD	flags(){

			return (pcallback_context)?pcallback_context->flags:0;
		}

		pcallback_lib_arguments_t operator  ->() const
		{
			return pcallback_context->pclib;
		}

		pcallback_lib_arguments_t plib() const
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
		inline bool is_method()
		{
			return pcallback_context->flags|DISPATCH_METHOD;
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
				return (N)v;
			}

			inline 	bool is_empty()
			{
				VARTYPE t=v.vt;
				return (t==VT_NULL)||(t==VT_EMPTY)||(t==VT_ERROR);
			}

			bool is_object() {
				VARTYPE t = (v.vt)&(VT_TYPEMASK);
				return (t == VT_DISPATCH) || (t == VT_UNKNOWN);
			}

			template <class N>
			N def(N dv=N()) {
				try     {
					return (!is_empty())?(N)v:dv;
				}
				catch(...){
				}
				return dv;
			}

			template <class N>
			N& update(N& rv) {

				try     {

					if(!is_empty())	rv=v;
				}
				catch(...){
				}
				return rv;

			}

			/*
			HRESULT operator =(var_t v)
			{
				if(pwr) return pwr->set_value(v);
				else return E_ACCESSDENIED;

			}
			*/






/*
			template<class O>
			HRESULT QueryInterface( O& Obj)
			{
				HRESULT hr;
				return hr=QueryInterface(__uuidof(O::intf_t),Obj._ppQI()); 
			}
			*/


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
				ipc_utils::smart_ptr_t<IUnknown> unk;
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



	

	template <DWORD Flags>
	inline HRESULT wrap_callback(const void* callback,const void* context,const void* onexit,VARIANT* res)
	{
		HRESULT hr;
		if(!res) return E_POINTER;

		
		ICallbackDispatchFactory* callbackFactory=0;

		hr=::CoGetObject(bstr_t(L"ltx.bind:callback.factory"),0,__uuidof(ICallbackDispatchFactory),(void**)&callbackFactory);		 
		if(SUCCEEDED(hr))
		{
			hr=callbackFactory->CreateInstanceV(FLAG_MASK_PUT|(Flags<<16),(void*)callback,(void*)context,(void*)onexit,res);			  		 
			callbackFactory->Release();
			
		}
		//if(FAILED(hr)) return variant_t(hr,VT_ERROR);

		return hr;

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

	struct fake_mutex_t {
		inline void lock() {};
		inline void unlock() {};
	};

	struct mutex_cs_t
	{
		CRITICAL_SECTION m_cs;
		mutex_cs_t(unsigned int sc = 0) {

			if (!sc) ::InitializeCriticalSection(&m_cs);
			else ::InitializeCriticalSectionAndSpinCount(&m_cs, sc);
		}
		~mutex_cs_t()
		{
			::DeleteCriticalSection(&m_cs);
		} // on  	  DLL_PROCESS_DETACH
		inline void lock() { ::EnterCriticalSection(&m_cs); }
		inline void unlock() { ::LeaveCriticalSection(&m_cs); }


		inline bool try_lock(int t = 0)
		{
			return  ::TryEnterCriticalSection(&m_cs);
		}

		inline unsigned int set_spin_count(unsigned int sc)
		{
			return ::SetCriticalSectionSpinCount(&m_cs, sc);
		}

	};


	template <class T,class VariantType,class MUTEX= mutex_cs_t>
	struct dispatch_by_name_wrapper_t
	{
		typedef dispatch_by_name_wrapper_t<T,VariantType,MUTEX> base_t;
		typedef VariantType com_variant_t;
		typedef typename arguments_t<com_variant_t> arguments_t;

		struct locker_t {
			locker_t(MUTEX* pm) :m(pm) {
				m->lock();
			}
			~locker_t() {
				m->unlock();
			}

			MUTEX* m;
		};


		static		void  s_on_exit( T* pthis){

			std::shared_ptr<MUTEX> m = pthis->get_base()->mutex;
			locker_t lock(m.get());
			delete pthis;
			
		}

		inline base_t* get_base() {
			return this;
		}

		inline  void  on_get(DISPID id,const wchar_t* name,com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper){};
		inline  void  on_put(DISPID id,const wchar_t* name,com_variant_t& value,arguments_t& arguments,i_marshal_helper_t* helper){};

		inline  void  oncallback(com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper){

			 wchar_t* name=arguments.pm_name();
			 DISPID id=arguments.id();

			if(arguments.is_get_flag())
				static_cast<T*>(this)->on_get(id,name,result, arguments,helper);
			else static_cast<T*>(this)->on_put(id,name,result, arguments,helper);
		};


		inline  void  invoke_by_name(pcallback_context_arguments_t pcca,i_marshal_helper_t* helper){

			

			locker_t lock(mutex.get());
			//locker_t lock(&mutex);
			//std::lock_guard<MUTEX> guard(mutex);



				arguments_t arguments(pcca);
				static_cast<T*>(this)->oncallback(arguments.result(),arguments,helper);  
		}


		static		void  s_callback
			(  T* pthis,pcallback_context_arguments_t pcca,i_marshal_helper_t* helper){

				pthis->invoke_by_name(pcca,helper);

		}

		template <class Args>
		HRESULT wrap(const Args& args,VARIANT* pres,DWORD flags)
		{
			HRESULT hr;
			flags|=FLAG_CBD_DBN;
			return    hr= args->create_callback(flags<<16,&base_t::s_callback,static_cast<T*>(this),&base_t::s_on_exit,pres);
		}


		HRESULT wrap(VARIANT* pres,DWORD flags=0)
		{
			HRESULT hr;
			if(!pres) return E_POINTER;
			flags|=FLAG_CBD_DBN;
			ipc_utils::smart_ptr_t<ICallbackDispatchFactory> callbackFactory;
			hr=::CoGetObject(bstr_t(L"ltx.bind:callback.factory"),0,__uuidof(ICallbackDispatchFactory),callbackFactory.ppQI());		 
			
			if(SUCCEEDED(hr))
				hr=callbackFactory->CreateInstanceV(flags<<16,&base_t::s_callback,static_cast<T*>(this),&base_t::s_on_exit,pres);			  		 
		}

		template <class Calback_Aggregator>   
		HRESULT wrap(Calback_Aggregator* pfactory,VARIANT* pres,DWORD flags)// =calback_aggregator_t<IDispatch> 
		{
			HRESULT hr;
			flags|=FLAG_CBD_DBN;
			return hr=Calback_Aggregator::CreateInstanceV(flags<<16,&base_t::s_callback,static_cast<T*>(this),&base_t::s_on_exit,pres);
		}


		static HINSTANCE get_hinstance(void* ptr=&s_callback)
		{
			HINSTANCE h=HINSTANCE(-1);
			GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)ptr,&h);
			return h;
		}

		template <class O,class F>
		static HRESULT unwrap(O o, F** ppf) {
			HRESULT hr;

			if (!ppf)
				return E_POINTER;

			i_unwrapper_t<i_context> ic(o);

			if (SUCCEEDED(hr=ic.hr)) {
				hr=ic->GetContext((void**)ppf);
			}
			return hr;
		}

		dispatch_by_name_wrapper_t() :mutex(new MUTEX) {};
		/*
		dispatch_by_name_wrapper_t() {
			//mutex.reset(new MUTEX() );
			//locker_t lock(mutex.get());
		};
		*/
		std::shared_ptr <MUTEX> mutex;
		//MUTEX mutex;
	};


	


	template <class F>
	struct i_mbv_buffer_helper_t
	{
		typedef i_mbv_buffer_ptr::int64_t int64_t;

		F* ptr;
		int64_t count;
		int64_t byteLength;
		shared_mem_data_t* smd;
		long elementSize;
		HRESULT hr;
		VARTYPE vt;

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


				//OLE_CHECK_VOID(hr=buffer->GetPtr((void**)&ptr,&(byteLength=0)));		 
				//count=byteLength/sizeof(F);
				//virtual HRESULT STDMETHODCALLTYPE GetElementInfo(VARTYPE* pvt,int64_t* pCount=0,long* pElementSize=0)=0;

				if(SUCCEEDED(hr=buffer->GetElementInfo(&vt,&count,&elementSize))&&(elementSize!=sizeof(F)))
				   hr=E_INVALIDARG;
				if(SUCCEEDED(hr))
					  buffer->Lock();

			}
		};

		~i_mbv_buffer_helper_t(){
			if(buffer)
			{
				if(SUCCEEDED(hr))
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



   template <class Variant_T>
   struct mbv_map_reader_t
   {
	   typedef Variant_T var_t;

	   typedef typename arguments_t<var_t>::arg_t arg_t;
	   //typedef a_t::V_C_t arg_t;
	   VARIANT vempty;
	   //var_t r; 
	   long length;
	   

	   


	   

	   struct variant_holder_t
	   {
		   VARIANT  v;
		   variant_holder_t():v(){}
		   ~variant_holder_t(){ VariantClear(&v);}

	   };

	   
	   HRESULT hr;
	   i_mbv_container* mbv;

	   operator HRESULT(){ return hr;}

	   mbv_map_reader_t(VARIANT v):mbv(0)
	   {
		   VARIANT t={VT_EMPTY};
		   vempty=t;
		  OLE_CHECK_VOID_hr(VariantChangeType(&t,&v,0,VT_UNKNOWN));
		  OLE_CHECK_VOID_hr(t.punkVal->QueryInterface(__uuidof(i_mbv_container),(void**)&mbv));
		  t.punkVal->Release();		  	
		  OLE_CHECK_VOID_hr(mbv->GetLength(&length));	

	   }
	   ~mbv_map_reader_t(){
		   if(mbv) mbv->Release();
	   }


	    bool is(bstr_t n)
		{
			return (mbv->IsItem(n)==S_OK);
		}

	template<class N>
	   arg_t operator[](N name)
	   {

		   var_t vn=name;

		   cache_v.push_front(variant_holder_t());	 		
		   VARIANT* pr=&cache_v.front().v;
		   if(SUCCEEDED(hr)&&SUCCEEDED(mbv->GetItem(vn,pr)))
		   {
			   return *((var_t*)pr);
		   }

		   //return  verror();       
		   return  vempty;       
	   }

	   template <class N>
	   N& update(const var_t key,N& n) 
	   {
		   return (*this)[key].update(n);
	   }

	   void clear_cache()
	   {
            cache_v.clear();
	   }

	   
	   std::list<variant_holder_t> cache_v;   

   };

   template <class Variant_T>
   struct mbv_map_writer_t
   {
	   typedef Variant_T var_t;
	   HRESULT hr;
	   i_mbv_container* mbv;

	   mbv_map_writer_t(IUnknown* punk=0):mbv(0){

		   if(punk)
		   {			   
			   hr=punk->QueryInterface(__uuidof(i_mbv_container),(void**)&mbv);
		   }
		   else
		   {
             hr=::CoGetObject(bstr_t(L"ltx.bind:mbv_map"),0,__uuidof(i_mbv_container),(void**)&mbv);
		   }
		   
	   };
	   ~mbv_map_writer_t(){
		   if(mbv) mbv->Release();
	   }

	   HRESULT set_value(var_t key,var_t val)
	   {
           HRESULT hr0;
		   if(FAILED(hr)) return hr;
		   hr0=mbv->SetItem(key,val);
		   return hr0;
	   }

	   HRESULT set_value(const wchar_t* key,var_t val)
	   {
		   HRESULT hr0;
		    return  hr0=set_value(var_t(key),val);
	   }


	   HRESULT detach(VARIANT* pv)
	   {
		   if(!pv) return E_POINTER;
		   if(SUCCEEDED(hr))
		   {
		   VARIANT v={VT_UNKNOWN};
		   v.punkVal=mbv;
		   mbv=0;
		   *pv=v;
		   }
		   return hr;
	   }
	   operator HRESULT(){ return hr;}


   };

   


};//ltx_helper - namespace end
