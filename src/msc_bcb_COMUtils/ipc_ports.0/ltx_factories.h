#pragma once
//ltx_factories.h 
//#include "singleton_utils.h"
#include "com_marshal_templ.h"
#include "com_script_engine.h"
#include "com_apartments_asyn.h"
#include "dispatch_function.h"
#include "com_super_factory.h"

namespace ipc_utils {
	namespace com_apartments
	{


		namespace script_engine
		{



			//



			typedef _com_error com_exception;


			struct  __declspec(uuid("{DE459FE3-19A1-46a0-9896-8EAC2ECBF2CC}"))  asyn_factory_t
				:class_factory_base_aggregator_t<asyn_factory_t>
			{

				struct asyn_disp_t:IDispatch_impl_t<asyn_disp_t>					
				{
					typedef ltx_helper::arguments_t<variant_t> arguments_t;

					inline	HRESULT impl_Invoke(DISPID id,REFIID riid,LCID,WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi, UINT*){

						HRESULT hr=E_UNEXPECTED;
						if(id!=0) return hr=E_NOTIMPL;
						try
						{
							arguments_t arguments(dp);
							//int iii=arguments[0];

							smart_ptr_t<IDispatch> funct,asynobj,eventobj;
							variant_cast_t<VT_BSTR> vtext(VARIANT(arguments[0]),VARIANT_NOVALUEPROP);
							bool fscript=vtext;
							if(fscript)
							{
								OLE_CHECK_hr(create_apartment_script_engine(&funct.p,0,L"JScript",0));


							} 
							else OLE_CHECK_hr(arguments[0].QueryInterface(&funct.p));


							if(!arguments[1].is_empty())
								OLE_CHECK_hr(hr=arguments[1].QueryInterface(&eventobj.p));
							OLE_CHECK_hr(asyn_object_t::CreateInstance(funct,eventobj,&asynobj.p));

							if(fscript)
							{
								variant_t* pv=(variant_t*)((VARIANT*)vtext);
								variant_t r;
								//OLE_CHECK_hr(call_disp(asynobj,0,asyn_object_t::push_arg_id,*pv));
								DISPPARAMS dps={pv,0,1,0};
								hr=asynobj->Invoke(asyn_object_t::push_arg_id, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_GETSF,&dps,&r,0,0);
							}

							if(res)
							{
								VARIANT r={VT_DISPATCH};
								r.pdispVal=asynobj.detach();
								*res=r;
							}
							return hr=S_OK;



						}
						catch (com_exception& e)
						{
							//IErrorInfo* pei=e.ErrorInfo();
							hr=e.Error();


						}
						catch (...){								
						}
						return hr;




						return S_OK;
					}

				};


				inline HRESULT impl_CreateInstanceEx(	IUnknown * pUnkOuter,LPOLESTR pszDisplayName,IStream* ,REFIID riid,void ** ppvObject)	
				{
					HRESULT hr;

					aggregator_container_helper_FMT_EC<true,false> container;

					smart_ptr_t<iaggregator_helper> asyn=aggregator_helper_creator_t<asyn_disp_t>::New(new asyn_disp_t);

					mta_singleton_t::add_external_connector(container);

					hr=container.add(__uuidof(IDispatch),asyn).QueryInterface(riid,ppvObject);						

					return hr;

					;}

				static HRESULT CreateFactory(REFIID riid, void** ppcf)
				{
					HRESULT hr;
					ipc_utils::smart_ptr_t<IUnknown> unk;
					hr=(new asyn_factory_t)->GetFactory(&unk.p);
					if(SUCCEEDED(hr))
						hr=unk->QueryInterface(riid,ppcf);
					return hr;						  
				}


			};











			struct  __declspec(uuid("{02A0B5E1-529E-4b09-8B9F-97BBFA97CF66}"))  file_search_factory_t
				:simple_factory_t<file_search_factory_t>					
			{
				struct fs_disp_t:IDispatch_impl_t<fs_disp_t>					
				{

					inline	HRESULT impl_Invoke(DISPID id,REFIID riid,LCID,WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi, UINT*){


						OLE_CHECK_PTR(res);

						HRESULT hr=E_UNEXPECTED;
						if((id!=0)&&is_put(flags) )return hr=E_NOTIMPL;

						try
						{
							arguments_t arguments(dp);
							if(!arguments.length()) return hr=E_INVALIDARG;
							bstr_t fn=arguments[0].def(bstr_t());
							bstr_t search_path=arguments[1].def(bstr_t());
							int fc=arguments[2].def(1);
							VARIANT r={VT_BSTR};
							wchar_t *fulname;
							moniker_parser_t<wchar_t> mon;
							OLE_CHECK_hr(mon.expand_file_path(fn,&fulname,search_path,fc));

							r.bstrVal=bstr_t(fulname).Detach();
							*res=r;

						}
						catch (com_exception& e)
						{							
							hr=e.Error();
						}
						catch (...){								
						}
						return hr;




						return S_OK;
					}

				};


				inline HRESULT create_iaggregator(aggregator_container_helper_t& container,LPOLESTR pszDisplayName,IStream* ,GUID& uiid,smart_ptr_t<iaggregator_helper>& fs_disp )
				{
					fs_disp=aggregator_helper_creator_t<fs_disp_t>::New(new fs_disp_t);
					//
					mta_singleton_t::add_external_connector(container);
					//							container.add_EC();

					return (fs_disp)?S_OK:E_OUTOFMEMORY;

				}



			};






			struct  __declspec(uuid("{1D3EE9AB-27EC-418d-BD0F-99309FF488CC}")) external_connect_locker_factory_t
				:simple_factory_t<external_connect_locker_factory_t>					
			{

				struct handle_locker_t
				{
					event_local_t hwait;
					event_local_t hevent;

					handle_locker_t(HANDLE hw,HANDLE hev):hwait(hw),hevent(hev){}
					void operator()()
					{
						hevent.signal(1);
					}
					~handle_locker_t()
					{

					}


				};
				struct external_connect_locker_t:IDispatch_impl_t<external_connect_locker_t>					
				{
					inline HRESULT findname(LPOLESTR name,DISPID& dispid){

						HRESULT hr;
						static  BEGIN_STRUCT_NAMES(pn)										
							DISP_PAIR_IMPLC(lock,DISPID_VALUE)								
							DISP_PAIR_IMPLC(wait,1)								
							DISP_PAIR_IMPLC(signal,2)
							DISP_PAIR_IMPLC(handle,3)
							END_STRUCT_NAMES;
						hr=find_id(pn,name,&dispid);
						return hr;
					}

					inline	HRESULT impl_Invoke(DISPID id,REFIID riid,LCID lsid,WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi, UINT* pu){

						HRESULT hr;
						if(id==DISPID_VALUE)
						{
							if(!((dp->cArgs)&&(dp->rgvarg->vt==VT_BOOL))) return E_INVALIDARG;

							VARIANT_BOOL f=dp->rgvarg->boolVal;
							if(f)
							{ 
								
								/*
								int pid;


								arguments_t args(dp);

								if( pid=args[1].def<int>(0)){

									process_handle_t ph(pid);
									if(ph.get_handle())
									{								   
										HANDLE h=make_detach(ph.hp); 
										handle_locker_t* pl=new handle_locker_t(h,hevent);

										asyn_su::_pool<handle_locker_t,true> pool(pl);

										if(FAILED(hr=pool.register_kobject(h)))
										{
											delete pl;
											return HRESULT_FROM_WIN32(hr);
										}


									}
									else return hr=HRESULT_FROM_WIN32(ph.hr);

								}*/


								ec_counter.AddRef();

							}
							else if(ec_counter.Release()<=0)
								hevent.signal();




							return S_OK;
						}
						else
							if(id==1)
							{
								int to=INFINITE;
								if(dp->cArgs){
									variant_cast_t<VT_I4> vc(dp->rgvarg[0]);
									if(vc) to=vc->intVal;
								}

								VARIANT ws={VT_I4};
								ws.intVal=hevent.lock(to);
								if(res) *res=ws;

								return S_OK;
							} else 
								if(id==2)
								{
									hevent.signal();
                                   return S_OK;
								}
								else 
									if((id==3)&&res)
									{
										VARIANT v={VT_UI8};
										arguments_t args(dp);
										int pid=args[0].def<int>(0);
										process_handle_t ph(pid);
										v.ullVal=(ULONG64)dup_handle(hevent.hko,ph);
										if(!v.ullVal) return HRESULT_FROM_WIN32(GetLastError());
										*res=v;
										return S_OK;
									}

							return E_NOTIMPL;

					}

					RefCounterMT_t<0> ec_counter;
					event_local_t hevent;

				};

				inline HRESULT impl_CreateInstanceEx(	IUnknown * pUnkOuter,LPOLESTR pszDisplayName,IStream* stream,REFIID riid,void ** ppvObject)	
				{
					HRESULT hr;					
					OLE_CHECK_PTR(ppvObject);
					int global_mode=0;
					{						
						moniker_parser_t<wchar_t> mp(pszDisplayName);
						global_mode=mp.args[L"global"].def<int>(0);
						//wchar_t* mutex_name=mp.trim(mp.args[L"mutex"]);
						if(global_mode)
						{

							smart_GIT_ptr_t<IDispatch>& gec= mta_singleton_t::external_connector();
							if(gec)
								return  hr=gec.unwrap(riid,ppvObject);
						}


					}

					if(global_mode&2)
					{
						smart_ptr_t<IDispatch> disp;

						OLE_CHECK_hr(__super::impl_CreateInstanceEx(pUnkOuter,pszDisplayName,stream,__uuidof(IDispatch),disp._ppQI()));
						mta_singleton_t::external_connector().reset(disp);

						hr=disp->QueryInterface(riid,ppvObject);


					}
					else 	 hr=__super::impl_CreateInstanceEx(pUnkOuter,pszDisplayName,stream,riid,ppvObject);						 

					return hr;

				}



				inline HRESULT create_iaggregator(aggregator_container_helper_t& container,LPOLESTR pszDisplayName,IStream* ,GUID& uiid,smart_ptr_t<iaggregator_helper>& disp )
				{
					disp=aggregator_helper_creator_t<external_connect_locker_t>::New(new external_connect_locker_t);

					return (disp)?S_OK:E_OUTOFMEMORY;

				}



			};







			struct  __declspec(uuid("{0407A214-8257-4f58-8DDF-8861A67BF03A}"))  dll_bind_factory_t				
				:class_factory_base_aggregator_t<dll_bind_factory_t>
			{		



				inline HRESULT impl_CreateInstanceEx(	IUnknown * pUnkOuter,LPOLESTR pszDisplayName,IStream* ,REFIID riid,void ** ppvObject)	
				{
					HRESULT hr;

					OLE_CHECK_PTR(ppvObject);

					moniker_parser_t<wchar_t> mp(pszDisplayName);
					char* tail;
					wchar_t* wproc=mp.trim(mp.args[L"proc"]);
					wchar_t* lib=mp.trim(mp.args[L"lib"]);
					if(!lib) lib=mp.trim(mp.args[L"file"]);
					wchar_t* path=mp.trim(mp.args[L"path"]);
					DWORD flags=mp.args[L"flags"].def<int>(0);
					if((mp.args[L"flags.sta"].def<int>(0))) flags|=FLAG_CBD_APA;
					if((mp.args[L"flags.fmt"].def<int>(0))) flags|=FLAG_CBD_FTM;
					if((mp.args[L"flags.git"].def<int>(0))) flags|=FLAG_CBD_GIT;
					if((mp.args[L"flags.old"].def<int>(0))) flags|=FLAG_CBD_OLD;
					if((mp.args[L"flags.com"].def<int>(0))) flags|=FLAG_CBD_COM;
					if((mp.args[L"flags.rev"].def<int>(0))) flags|=FLAG_CBD_REVERSED;

					int fautodetect=mp.args[L"flags.autodetect"].def<int>(1);

					
					DWORD fm;

					fm=mp.args[L"flags.mask"].def<int>(DISPATCH_PUTSF);
					flags= (flags<<16)|fm;

					char_mutator<CP_THREAD_ACP> cm(wproc);

					char *proc=cm;



					if(!(proc&&lib)) return E_POINTER;

					wchar_t* fullibname=0;
					OLE_CHECK_hr(mp.expand_file_path(lib,&fullibname,path));
					HMODULE hm=LoadLibraryW(fullibname);
					if(!hm) return hr=HRESULT_FROM_WIN32(GetLastError());

					struct unloader_t{
						HMODULE hm;
						~unloader_t(){FreeLibrary(hm);}

					} unloader={hm};

                    void * pcalback;
					char *p;


					OLE_CHECK_hr(moniker_parser_t<char>::getProcAddress(hm,proc,&pcalback));

					/*

					if(p=moniker_parser_t<char>::_lskipi(proc,"::",2))
					{
						p[-2]=0;
						int l=safe_len(p);
						if(!l) 
							return hr=HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);  

						if(pcalback=GetProcAddress(hm,proc))
						{
							//external_callbacks_t** ppec=(external_callbacks_t**) make_detach(pcalback,0);
							//OLE_CHECK_PTR(ppec);
							//external_callbacks_t* pec=*ppec;

							external_callbacks_t* pec=(external_callbacks_t*) make_detach(pcalback,0);
							OLE_CHECK_PTR(pec);


							while(pec->name)
							{
								if(0==safe_cmpni(pec->name,p,l))
								{
									pcalback=pec->proc;
									break;
								}

								++pec;
							}
							if(!pcalback) 
								return hr=HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);  

						}
							//
						

					}
					else{
						pcalback=GetProcAddress(hm,proc);
						if(!pcalback) 
							return hr=HRESULT_FROM_WIN32(GetLastError());
					}
					*/

					//static HRESULT CreateInstanceV(void* pcalback,void* context,void* on_exit,VARIANT* pres)
					/*
					struct calback_context_t:callback_lib_arguments_t
					{
						static void on_exit(calback_context_t* p)
						{
							delete p;
						}
					};

					//HRESULT (*CreateInstanceV)(void* pcalback,void* context,void* on_exit,VARIANT* pres);
					calback_context_t* pcc=new calback_context_t;
					pcc->create_callback=&calback_aggregator_t<IDispatch>::CreateInstanceV;
					pcc->bind_object=&ltxGetObject;
					*/


					callback_lib_arguments_t* pcc=&process_com_t::get_instance().callback_lib_arguments;


					DWORD dw;

					//OLE_CHECK_hr(calback_aggregator_t<IDispatch>::CreateInstance(flags,pcalback,pcc,0,&dw));
					//hr=GIT_t::get_instance().unwrap(dw,riid,ppvObject,true);
					hr=calback_aggregator_t<IDispatch>::CreateInstance(flags,pcalback,pcc,0,riid,ppvObject);

					return hr;
				}

			};


			struct  __declspec(uuid("{9AE546C9-45A5-46fa-8137-EAD6FD0536B9}"))  callback_factory_factory_t
				:simple_factory_t<callback_factory_factory_t>					
			{


				//template <class T,class IntfA>ICallbackDispatchFactory
				//struct base_aggregator_t: base_component_t<T,IntfA>//:IntfA

				struct callback_dispatch_factory_t:base_aggregator_t<callback_dispatch_factory_t,ICallbackDispatchFactory>
				{
					virtual HRESULT STDMETHODCALLTYPE CreateInstance(DWORD exflags,void* pcalback,void* context,void* on_exit,REFIID riid,void** ppObj)
					{
						HRESULT hr;
						
						return hr=calback_aggregator_t<IDispatch>::CreateInstance(exflags,pcalback,context,on_exit,riid,ppObj);


					};
					virtual HRESULT STDMETHODCALLTYPE CreateInstanceV(DWORD exflags,void* pcalback,void* context,void* on_exit,VARIANT* pv)
					{
						HRESULT hr;						
						return hr=calback_aggregator_t<IDispatch>::CreateInstanceV(exflags,pcalback,context,on_exit,pv);
					};

				};




				inline HRESULT create_iaggregator(aggregator_container_helper_t& container,LPOLESTR pszDisplayName,IStream* ,GUID& uiid,smart_ptr_t<iaggregator_helper>& obj )
				{
					//fs_disp=aggregator_helper_creator_t<fs_disp_t>::New(new fs_disp_t);
					//
					//mta_singleton_t::add_external_connector(container);
					//							container.add_EC();
					uiid=__uuidof(ICallbackDispatchFactory);
                         obj=aggregator_helper_creator_t<callback_dispatch_factory_t>::New(new callback_dispatch_factory_t);

					return (obj)?S_OK:E_OUTOFMEMORY;

				}



			};




		}; // script_engine
	}; // com_apartments
}; //ipc_utils

