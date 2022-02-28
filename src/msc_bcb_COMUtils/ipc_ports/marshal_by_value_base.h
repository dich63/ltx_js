#pragma once 
//marshal_by_value_base
//#include "ipc_ports/ipc_utils.h"
//#include "ipc_ports/imarshal_aggregator.h"
#include "ipc_ports/dispatch_by_name_aggregator.h"


namespace ipc_utils {
	
			namespace marshal_by_value_base{

				template<class T,bool fDBN=true,class com_variant_T=variant_t>
				struct mbv_context_base0_t
				{
					typedef com_variant_T com_variant_t;

					typedef ltx_helper::arguments_t<com_variant_t> arguments_t;

					static 		void OnExit(void *context){

						delete (T*)context;						
					};


					HRESULT Init(BOOL fUnmarshal,IBindCtx* bctx,LPOLESTR pszDisplayName,DWORD& flags){
						    return S_OK;
					}

					HRESULT marshal_to_process(i_marshal_helper_t* helper,DWORD pid_target,IStream* pStm){
						return E_NOTIMPL;
					}
					HRESULT unmarshal(i_marshal_helper_t* helper,IStream* pStm){
						return E_NOTIMPL;
					}
					HRESULT set_aggregator_helpers(iaggregator_container* container,pcallback_lib_arguments_t plib)
					{
						
                            return S_OK;
					}



					static 		HRESULT CreateContext(BOOL fUnmarshal,IBindCtx* bctx,LPOLESTR pszDisplayName,DWORD* pflags,void ** ppcontext){

						     if(!ppcontext) return E_POINTER;
							 HRESULT hr;
						     local_ptr_t<T> obj=new T;	
							 DWORD t=0;
							 DWORD &flags=(pflags)? *pflags:t;

                              hr=obj->Init(fUnmarshal,bctx,pszDisplayName,flags);
                                 
							 return obj.detach(hr,ppcontext);
					};
					static 		HRESULT MarshalToProcess(void* context,i_marshal_helper_t* helper,DWORD pid_target,IStream* pStm){
						HRESULT hr;

						hr=static_cast<T*>(context)->marshal_to_process(helper,pid_target,pStm);
						return hr;						
					};
					static 		HRESULT Unmarshal(void* context,i_marshal_helper_t* helper,IStream* pStm){

						HRESULT hr;
						hr=static_cast<T*>(context)->unmarshal(helper,pStm);
						return hr;

					}

					static HRESULT SetAggregatorHelpers(void *context,iaggregator_container* container,pcallback_lib_arguments_t plib)
					{
						HRESULT hr;
						hr=static_cast<T*>(context)->set_aggregator_helpers(container,plib);
						return hr;
					};



					inline  HRESULT  on_get(DISPID id,const wchar_t* name,com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper){
						return E_NOTIMPL;
					};
					inline   HRESULT  on_put(DISPID id,const wchar_t* name,com_variant_t& value,arguments_t& arguments,i_marshal_helper_t* helper){
						return E_NOTIMPL;
					};
					inline  void  oncallback(com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper){

						wchar_t* name=arguments.pm_name();
						DISPID id=arguments.id();
						HRESULT& hr=arguments.hr;

						if(arguments.is_get_flag())
						 hr=	static_cast<T*>(this)->on_get(id,name,result, arguments,helper);
						else hr=static_cast<T*>(this)->on_put(id,name,result, arguments,helper);
					};


					inline  void  invoke_by_name(pcallback_context_arguments_t pcca,i_marshal_helper_t* helper){

						arguments_t arguments(pcca);
						static_cast<T*>(this)->oncallback(arguments.result(),arguments,helper);  
					}




					static 		void InvokeByName(void* context,pcallback_context_arguments_t pcca,i_marshal_helper_t* helper){
						static_cast<T*>(context)->invoke_by_name(pcca,helper);
					}


				};

				template<class T, const CLSID& _clsid, bool fDBN = true, class com_variant_T = variant_t>
				struct mbv_context_base_t:mbv_context_base0_t<T,fDBN, com_variant_T> {

					static 		HRESULT GetClassID(CLSID* pclsid) {

						*pclsid = _clsid;
						return S_OK;
					}
					static MarshalCallbackData_t getMCD()
					{
						MarshalCallbackData_t m = { &GetClassID,&CreateContext,&OnExit,fDBN ? &InvokeByName : 0,&MarshalToProcess,&Unmarshal,&SetAggregatorHelpers };
						return m;
					}
				};

				template<class T, class coT, bool fDBN = true, class com_variant_T = variant_t>
				struct mbv_context_base_c_t :mbv_context_base0_t<T, fDBN, com_variant_T> {

					static 		HRESULT GetClassID(CLSID* pclsid) {

						*pclsid = __uuidof(coT);
						return S_OK;
					}
					static MarshalCallbackData_t getMCD()
					{
						MarshalCallbackData_t m = { &GetClassID,&CreateContext,&OnExit,fDBN ? &InvokeByName : 0,&MarshalToProcess,&Unmarshal,&SetAggregatorHelpers };
						return m;
					}
				};












			}//marshal_by_value_base 
}; //ipc_utils

