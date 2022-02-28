#pragma once
//dispatch_by_name_aggregator.h
//#include "com_global.h" 
//#include "com_script_engine.h"
#include "com_apartments.h"
//typedef   ipc_utils::com_apartments::process_com_t p_c_t;
//typedef   ipc_utils::com_apartments::script_engine::script_engine_t::process_com_t p_c_t;


namespace ipc_utils {
	//namespace com_apartments {
		//namespace script_engine		{

			 ;


     //    base_callsback_marshal_t<IDispatchMBV_t,IDispatch,creator_class_base_t >
                
           template< class GlobalCOM_T > 
			struct dispatch_by_name_aggregator_t
				:IDispatch_impl_t<dispatch_by_name_aggregator_t<GlobalCOM_T> >
			{

				typedef void (*InvokeByName_callback_t)(void* context,pcallback_context_arguments_t pcca,i_marshal_helper_t* helper);
                
				dispatch_by_name_aggregator_t(IUnknown* Outer,void* InvokeByName_callback,void* context):InvokeByName(InvokeByName_callback_t(InvokeByName_callback)),pcontext(context)
				{
					pOuter=Outer;
				}


				inline HRESULT findname(LPOLESTR name,DISPID& dispid){

					
					int l=sizeof(wchar_t)*safe_len(name);
					tls_com_t<>::get_instance().set_buffer(l,name);
					dispid=callback_context_arguments_t::disp_id_t::TSSid;
					return S_OK;
				}

				static void s_call_disp( VARIANT* presult,dispatch_by_name_aggregator_t* _this,pcallback_context_arguments_t pcca,int argc,VARIANT* argv,int arglast,HRESULT& hr,void*,unsigned flags)
				{

					 
					_this->InvokeByName(_this->pcontext,pcca,&GlobalCOM_T::get_instance().marshal_helper);							  

				} 

				inline	HRESULT impl_Invoke(DISPID did,REFIID,LCID,WORD flags,DISPPARAMS* dp,VARIANT*res,EXCEPINFO* pexi, UINT*){

					//						return S_OK;

					if(!InvokeByName) 
						return E_NOTIMPL;

					safe_caller_t<>::V_C_t tmp;

					if(!res) res=&tmp;


					HRESULT hr;
					int c=dp->cArgs;
					VARIANT* pdv=dp->rgvarg;
					if(is_put(flags))
					{
						bool f=(dp->cNamedArgs==1)&&(dp->rgdispidNamedArgs[0]==DISPID_PROPERTYPUT);				   
						if(!f)
							return E_NOTIMPL;
						--c;
						res=pdv++;
					}

					void* ptr=this;       
					hr=safe_caller_t<>::call_prop(&s_call_disp,ptr,flags,c,pdv,res,pexi,did,this);						
					return hr;
				}

				/*
				inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ) 
				{   
					if(riid==GUID_NULL)
					{
						if(!ppObj)
						{
							hr=E_POINTER;
							return true;

						}
						*ppObj=pcontext;
						hr=S_OK;
						return true;
					}

					return __super::inner_QI(Outer,riid,ppObj,hr);
				}
				*/



				

				InvokeByName_callback_t InvokeByName;
				void* pcontext;
				//marshal_helper_t helper;
			};




//		};// namespace script_engine
//	}; //namespace com_apartments
};//namespace ipc_utils