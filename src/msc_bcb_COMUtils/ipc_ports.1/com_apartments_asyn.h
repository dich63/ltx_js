#pragma once
//#include "ipc_ports/com_apartments_asyn.h"
#include "com_script_engine.h"

namespace ipc_utils {
	namespace com_apartments
	{

		

		struct asyn_object_t:IDispatch_impl_t<asyn_object_t>
		{

            typedef  asyn_object_t self_t;

			struct creator_asyn_t
			{
				


				HRESULT hr;
				smart_GIT_ptr_t<IDispatch> dispobj,eventobj;
				smart_GIT_ptr_t<IUnknown> asynobj;

				creator_asyn_t(IDispatch* pdispobj,IDispatch* pevent):dispobj(pdispobj),eventobj(pevent){};

				inline HRESULT operator()(HRESULT& hr)
				{

					//dispobj,eventobj

					struct F:aggregator_helper_base_t<F>
					{

						inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ){

							return asyn(Outer,riid,ppObj,hr);
						}
						F(IDispatch* pdisp=0,IDispatch* pevent=0):asyn(pdisp,pevent){};
						self_t asyn;
					};

					ipc_utils::smart_ptr_t<IDispatch>  d,e;
					//ipc_utils::smart_ptr_t<IUnknown> eca; 

					dispobj.unwrap(d);
					eventobj.unwrap(e);

					ipc_utils::smart_ptr_t<F> asyn(new F(d,e),0);
					aggregator_container_helper container;
					OLE_CHECK_hr(container.set_aggregator(__uuidof(IDispatch),asyn));
					//OLE_CHECK_hr(container.set_ExternalConnect());

					/*
					OLE_CHECK_hr(create_ExternalConnection_aggregator_helper(0,&eca.p));
					OLE_CHECK_hr(container.set_aggregator(__uuidof(IExternalConnection),eca));
					OLE_CHECK_hr(container.set_aggregator(__uuidof(IObjectWithSite),eca));
					*/

					//if(SUCCEEDED(hr))
						asynobj.reset(container);
					return hr;
				}



			};





			struct thread_deferred_t
			{
				smart_ptr_t<asyn_object_t> deferred;        		  
				HRESULT hr;

				thread_deferred_t(asyn_object_t*p):deferred(p){ }

				inline void operator() (){
					

					COMInit_t ci(COINIT_MULTITHREADED);					

					hr=deferred->thread_proc();

				}


			};




			enum {
				push_arg_id=1,
				res_id=2,
				disp_id=3,
				wait_id=4,
				complete_id=5
			};
			//DISPPARAMS dps;

			typedef mutex_cs_t mutex_t;
			smart_ptr_t<IDispatch> disp,site,dispevent;
			DISPID DISP_id;
			std::vector<VARIANT> vargs;
			std::list<VARIANT> vlist;
			variant_t result;
			event_local_t evnt_complete;
			event_local_t evnt_init;
			mutex_t mutex;
			unsigned state;
			exceptinfo_t ei;
			int count0;


            
			asyn_object_t(IDispatch* pdisp=0,IDispatch* pevent=0):IDispatch_impl_t<asyn_object_t>(),disp(pdisp),dispevent(pevent),state(0),count0(0),DISP_id(DISPID_VALUE){}
			
			~asyn_object_t()
			{
				std::list<VARIANT>::iterator b=vlist.begin(),e=vlist.end(),i;
				for(i=b;i!=e;++i)
					VariantClear(&(*(i)));
			}


			inline bool set_check_state(unsigned f)
			{
				if(state&f) return true;
				else state|=f;
				return false;
			}


			HRESULT thread_proc()
			{
				HRESULT hr=E_FAIL;
				
				try{				

					int cb=vargs.size();
					VARIANT *pv=(cb>0)?&vargs[0]:0;
					WORD wFlags= (cb>0)?DISPATCH_METHOD|DISPATCH_PROPERTYGET:DISPATCH_METHOD;
					DISPPARAMS dps={pv,0,cb,0};
					unsigned int uArgError=-1;

					

				struct complete_signal_t
				{
					event_local_t* ev;
					~complete_signal_t(){  ev->signal(1); };
				} complete_signal={&evnt_complete};


				evnt_init.signal(1);
				if(disp)
					hr=disp->Invoke(DISP_id,IID_NULL,LOCALE_USER_DEFAULT,wFlags,&dps,&result,&ei,&uArgError);
				else hr=S_FALSE;

				}
				catch(...){};

				//smart_ptr_t<IObjectWithSite> site;
				//hr=result.punkVal->QueryInterface(__uuidof(IObjectWithSite),(void**)&site.p);

				try{
					 if(dispevent){


						 variant_t tmp,verror,vhr=int(hr);
						 
						 if(FAILED(hr))
						 {
                             result.Clear();
							 result=variant_t(hr,VT_ERROR);

							 if((DISP_E_EXCEPTION ==hr)&&(SysStringLen(ei.bstrDescription)))
								 verror=ei.bstrDescription;
							   else verror=(wchar_t*)error_msg();

						 }
						 else verror.vt=VT_NULL;
						 VARIANT vparams[3]={vhr,verror,result};
						 HRESULT hr0;

						DISPPARAMS dps={vparams,0,3,0};
						hr0=dispevent->Invoke(0,IID_NULL,LOCALE_USER_DEFAULT,DISPATCH_METHOD|DISPATCH_PROPERTYGET,&dps,&tmp,0,0);
					  } 


					}		catch(...){};            

				//evnt_complete.signal(1);
				return hr;

			}



			HRESULT save_args(int count,VARIANT* pv)
			{

				locker_t<mutex_cs_t> lock(mutex);

				if(set_check_state(2))
					return E_ACCESSDENIED;


				HRESULT hr=S_OK;                   
				if(vargs.size()) 
					return E_ACCESSDENIED;
				
				//int c=count+cl;
				//vargs.resize(c);
				


				for(int k=0;k<count;k++){
					VARIANT v={};
					if(FAILED(hr=VariantCopyInd(&v,pv+k)))
						return hr;				
					vlist.push_back(v);
				}

				std::list<VARIANT>::iterator it=vlist.end();       
				int cl=vlist.size();
				vargs.resize(cl);
				for(int k=0;k<cl;k++){
                    vargs[k]=*(--it);
				}

				
				return hr;
			}

			inline HRESULT findname(LPOLESTR name,DISPID& dispid){
				HRESULT hr;
				static  BEGIN_STRUCT_NAMES(pn)		
					DISP_PAIR_IMPLC(result,res_id)
					DISP_PAIR_IMPLC(func,disp_id)
					DISP_PAIR_IMPLC(push,push_arg_id)
					DISP_PAIR_IMPLC(wait,wait_id)                    
					DISP_PAIR_IMPLC(call,DISPID_VALUE)					 
					END_STRUCT_NAMES;
				hr=find_id(pn,name,&dispid);
				return hr;
			}
			inline	HRESULT impl_Invoke(DISPID id,REFIID,LCID,WORD flags,DISPPARAMS* pdp,VARIANT*res,EXCEPINFO*ei, UINT*){

				HRESULT hr=E_NOTIMPL;
				switch(id)
				{		 
				case DISPID_VALUE:
					if(is_get(flags))
					{
						OLE_CHECK_hr(hr=save_args(pdp->cArgs,pdp->rgvarg));

						asyn_su::_pool<thread_deferred_t,true>(new thread_deferred_t(this)).run(4);

						hr=toVARIANT(this,res);

						return hr;
					}
					break;					
				
				case res_id:
					if(DISPATCH_PROPERTYGET==flags)
					{
                        if(evnt_complete.try_lock())
						{
                          if(res)
							  hr=VariantCopyInd(res,&result);
						  else hr=S_FALSE;
						}
						else  hr=E_ACCESSDENIED;

					}

					break;
				case push_arg_id:
					if(is_get(flags)&&(pdp->cArgs))
					{
                      locker_t<mutex_cs_t> lock(mutex);
					  VARIANT V={};
					  if(SUCCEEDED(hr=VariantCopyInd(&V,pdp->rgvarg)))
        			     vlist.push_back(V);
					}
					else hr=E_INVALIDARG;

					break;


				case wait_id:
					if(is_get(flags)&&(pdp->cArgs))
					{
						
						evnt_complete.try_lock();

					}
					else hr=E_INVALIDARG;

					break;


					
				
				}
				return hr;
			}


			


           static HRESULT CreateInstance(IDispatch* pdispobj,IDispatch* pevent,IDispatch** ppasyn)
		   {

			   HRESULT hr;
			 //  process_com_t::get_instance().MTA();
			   mta_singleton_t::apartment();

			   apartment_t<> apartment(COINIT_MULTITHREADED);
			   creator_asyn_t creator(pdispobj,pevent);

			   
			   OLE_CHECK_hr(apartment.call_sync(creator,hr,E_FAIL,3));
			   hr=creator.asynobj.unwrap(ppasyn);              
			   return hr;
		   }

		};




	};//com_apartments
};//ipc_utils