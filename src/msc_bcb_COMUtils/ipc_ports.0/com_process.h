#pragma once
//com_process.h
#include "com_script_engine.h"
#include "ltx_call_helper.h"
#include "ltx_factories.h" 

//#include "os_utils.h"

#define LTX_PROCESS_OBJREF L"ltx.{5097F904-F18E-4099-99E0-F8C194349E06}"
#define MONIKER_EXTERNAL_OBJECT_OLD L"DICH_EXTERNAL_OBJECT_REF_MONIKER"

namespace ipc_utils {
	namespace com_apartments
	{
		namespace script_engine
		{


			//com_process_t



			//	template <class T ,class IDispatchX=IDispatch,class RefCounter=RefCounterMT_t<1> >
			//	struct IDispatch_base_t:IDispatch_impl_base_t<T,IUnknown_base_impl<T,IDispatchX,RefCounter > >{};


			struct utils_creator_t
			{


			};






			struct __declspec(uuid("{68386F39-877D-4083-B727-F6843A239568}")) utils_factory_t:simple_factory_t<utils_factory_t>
			{


				inline HRESULT impl_CreateInstanceEx(	IUnknown * pUnkOuter,LPOLESTR pszDisplayName,IStream* stream,REFIID riid,void ** ppvObject)	
				{
					HRESULT hr=E_NOINTERFACE;	
					bstr_t tmp;
					wchar_t* p=tmp=(wchar_t*) pszDisplayName,*ptail;
					if(moniker_parser_t<wchar_t>::_lcheckni(p,L"external",6,&ptail))
					{
						wchar_t buf[1024];						
						if(GetEnvironmentVariableW(LTX_PROCESS_OBJREF,buf,1024))
							return hr=CoGetObject(bstr_t(buf),0,riid,ppvObject);						
					}
					else 
						if(moniker_parser_t<wchar_t>::_lcheckni(p,L"srv",3,&ptail)||moniker_parser_t<wchar_t>::_lcheckni(p,L"lpc",3,&ptail))
						{
                           /* 
							bstr_t mn;
							clsid_t clbind=__uuidof(CLSID_marshal_factory_t);
							bstr_t regstr=L"CLSID\\" +bstr_t(clbind) +bstr_t("\\InprocServer32\\");     
							if(moniker_parser_t<wchar_t>::_lcheckni(ptail,L":64",3,&ptail))
								mn=moniker_parser_t<wchar_t>::reg_value_ex<64>(HKEY_CLASSES_ROOT,bstr_t(),regstr,"");
							else 
								if(moniker_parser_t<wchar_t>::_lcheckni(ptail,L":32",3,&ptail))
									mn=moniker_parser_t<wchar_t>::reg_value_ex<32>(HKEY_CLASSES_ROOT,bstr_t(),regstr,"");								

							if(mn.length()==0) 
								mn=module_t<>::module_name();
								*/

							moniker_parser_t<wchar_t> mp;


                           wchar_t* mn=mp.get_version_module_path(ptail,&ptail);
                           wchar_t* Os[2]={L"win32",L"x64"};

							bstr_t bindstr=v_buf<wchar_t>().printf(L"ltx.bind:process: file=rundll32.exe;cmd=\"%s\",runsrv parent.platform=%s"
								,(wchar_t*)mn,Os[sizeof(void*)/8]);

							smart_ptr_t<IDispatch> process;
							//OLE_CHECK_hr(CoGetObject(bindstr,0,__uuidof(IDispatch),process._ppQI()));
							OLE_CHECK_hr(super_factory_t::CoGetObject(bindstr,__uuidof(IDispatch),process._ppQI()));
							variant_t res,res2;
							OLE_CHECK_hr(call_disp(process,&res,int(0)));


							if(!VARIANT_is_Object(res)) 
								 return E_UNEXPECTED;





							
								//ComClipboard()=res.punkVal;
								if(safe_len(ptail=moniker_parser_t<wchar_t>::_ltrim(ptail))&&(*ptail==L':'))
								{
									smart_ptr_t<IDispatch> binder;
									if(res.vt==VT_DISPATCH) binder.reset(res.pdispVal);
									else OLE_CHECK_hr(res.punkVal->QueryInterface(__uuidof(IDispatch),binder._ppQI()));

									OLE_CHECK_hr(call_disp(binder,res.GetAddress(),int(0),variant_t(ptail)));


									//if((res.vt!=VT_DISPATCH)&&(res.vt!=VT_UNKNOWN))
									//	return hr=E_UNEXPECTED;							
									if(!VARIANT_is_Object(res)) 
										return E_UNEXPECTED;


								}

								OLE_CHECK_hr(res.punkVal->QueryInterface(riid,ppvObject));   
								//ComClipboard()=(IUnknown*)*ppvObject;


								return hr;
							

							//rundll32.exe 


						}
						else 
							if(moniker_parser_t<wchar_t>::_lcheckni(p,L"clipbrd",6,&ptail))
							{							
								return 	hr=CoGetObject(bstr_t(win_ver_6_t<>::ClipboardText()),0,riid,ppvObject);
							}
							else if(moniker_parser_t<wchar_t>::_lcheckni(p,L"old_style.external",-1,&ptail))
							{
								moniker_parser_t<wchar_t> mn(ptail);

								int fdebug=mn.args[L"debug"].def<int>(0);
								//int fenviron=mn.args[L"env"].def<int>(0);

								int cb;
								wchar_t* pbuf= mn.get_cache(1024);
								if(cb=GetEnvironmentVariableW(MONIKER_EXTERNAL_OBJECT_OLD,pbuf,1024))
								{
									OLE_CHECK_hr(CoGetObject(bstr_t(pbuf),0,riid,ppvObject));
								}
								else {


									if(fdebug&1)
									{
										wchar_t* hookfn = mn.get_cache().cat((wchar_t*)module_t<>::module_name((void*)0)).cat(L".LifeTimeX_hook");
										buf_fm_T<HOOK_PROCESS_INFO,OPEN_EXISTING>  hook_buf(hookfn);
										if(hook_buf&&moniker_parser_t<wchar_t>::_spin(&hook_buf->fready,100,100)) {							 
											wchar_t* pobjref=hook_buf->objref;
											if(SUCCEEDED(hr=CoGetObject(bstr_t(pobjref),0,riid,ppvObject)))
											{
												SetEnvironmentVariableW(MONIKER_EXTERNAL_OBJECT_OLD,pobjref);
												return hr;
											}
										}
									}
									if(fdebug&2)
									{
                                           bstr_t  bcstr=win_ver_6_t<>::ClipboardText();
										   if(SUCCEEDED(hr=CoGetObject(bcstr,0,riid,ppvObject)))
										   {
											   SetEnvironmentVariableW(MONIKER_EXTERNAL_OBJECT_OLD,bcstr);
											   return hr;
										   }

									}


									return hr=E_NOINTERFACE;

								}						 



							}
							else if(moniker_parser_t<wchar_t>::_lcheckni(p,L"progid:",7,&ptail))
							{
								clsid_t clsid(ptail);
								OLE_CHECK_hr(clsid.hr);
								return hr=::CoCreateInstance((CLSID)clsid,0,CLSCTX_ALL,riid, ppvObject);
								
							}





							/*	else 	if(moniker_parser_t<wchar_t>::_lcheckni(p,L"combin",-1,&ptail))
							{
							//_Check_return_ HRESULT CoCreateInstance(_In_ REFCLSID rclsid, _In_opt_ LPUNKNOWN pUnkOuter = NULL, _In_ DWORD dwClsContext = CLSCTX_ALL) throw()
							//
							clsid_t clsid(L"LifeTimeX.ltxmoniker");
							//clsid_t clsid(L"combin.clipbrd");

							return hr=::CoCreateInstance((CLSID)clsid,0,CLSCTX_ALL,riid, ppvObject);
							}
							*/

							return hr;

				}
			};



			struct stub_holder_t
			{

				stub_holder_t(IUnknown* unk=0):hstub(0),hr(E_POINTER)
				{
					if(unk)
						reset(unk);
				}
				stub_holder_t(const wchar_t* base64_str ):hstub(0),hr(E_POINTER)
				{

					reset(base64_str);
				}

				stub_holder_t& reset(const wchar_t* base64_str)
				{
					reset();
					hr=(hstub=moniker_parser_t<wchar_t>::base64_decode(moniker_parser_t<wchar_t>::_trim((wchar_t*)base64_str)))?S_OK:E_INVALIDARG;
					return *this;
				}


				~stub_holder_t(){
					if(hstub) reset();
				};

				stub_holder_t& reset(IUnknown* unk=0)
				{
					HGLOBAL t=make_detach(hstub);
					if (unk) 
						hr=ipc_utils::CoMarshalInterfaceToHGlobal(unk,&hstub);
					if(t) ipc_utils::CoReleaseCOMStub(t,true);
					return *this;
				}

				inline HRESULT detachInterface(){

					if(hstub&&(  (hstub=GlobalFree(hstub))!=NULL)) 
						return HRESULT_FROM_WIN32(GetLastError());
					else
						return S_OK;
				}

				inline operator bool()
				{
					return SUCCEEDED(hr);
				}
				inline bstr_t objref(bool fdetach=false)
				{
					bstr_t r;  
					if(SUCCEEDED(hr))
					{
						r = moniker_parser_t<wchar_t>::base64_encode(hstub,true);
						if((fdetach)&&(FAILED(hr=detachInterface()))) 
							return bstr_t();
					}
					return r;

				}

				inline HRESULT unwrap(REFIID iid,void**ppObj)
				{
					HRESULT hr0;
					OLE_CHECK_hr(hr);
					hr0=ipc_utils::CoUnmarshalInterfaceFromHGlobal(hstub,iid,ppObj);
					return hr0;

				}

				HGLOBAL hstub;
				HRESULT hr;
			};



			struct process_args_t:args_base_t<process_args_t,RefCounterMT_t<1>,mutex_cs_t >
			{
				typedef _com_error com_exception;	
				enum {
					id_result=-5,
					id_wait=-7,
					id_pid=-8
				};




				inline HRESULT findname(LPOLESTR name,DISPID& dispid){
					HRESULT hr;
					int i;

					static  BEGIN_STRUCT_NAMES(pn)										
						DISP_PAIR_IMPLC_LEN(result,id_result,3)
						DISP_PAIR_IMPLC_LEN(wait,id_result,4)
						DISP_PAIR_IMPLC(pid,id_pid)
						END_STRUCT_NAMES;
					if(SUCCEEDED(hr=find_id(pn,name,&dispid)))
						return hr;
					return hr=__super::findname(name,dispid);
				}


				inline	HRESULT impl_Result(WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi)
				{
					HRESULT hr;
					try
					{
						{				
							locker_t<mutex_t> lock(mutex);
							if(dp->cArgs!=1) return E_INVALIDARG;
							hr=VariantCopyInd(result.GetAddress(),dp->rgvarg);
						}

						hevent.signal();

					}
					catch (com_exception& e)
					{							
						hr=e.Error();
					}
					catch (...){								
					}
					return hr;




				}
				inline	HRESULT impl_Wait(WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi)
				{
					return E_NOTIMPL;
				}


				inline DWORD 	wait_result(VARIANT* res)
				{
					DWORD ws;

				}



				inline	HRESULT impl_Invoke(DISPID id,REFIID riid,LCID lsid,WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi, UINT* pu){

					HRESULT hr;

					if(id==id_result)
						return hr=impl_Result(flags,dp,res,pexi);
					else if(id==id_wait)
						return hr=impl_Wait(flags,dp,res,pexi);
					if((id==id_pid)&&is_get(flags)&&res)
					{
						VARIANT v={VT_I4};
						v.intVal=GetCurrentProcessId();
						*res=v;
						return S_OK;

					}
					return __super::impl_Invoke(id,riid,lsid,flags,dp,res,pexi,pu);

				}


				variant_t result;
				event_local_t hevent;				
				process_args_t(HANDLE h_event,DISPPARAMS* pdp ):args_base_t<process_args_t,RefCounterMT_t<1>,mutex_cs_t >(pdp),hevent(h_event){}		


			};


			struct  __declspec(uuid("{55DA116A-AEA5-4d68-88F8-8DBF19FFFE88}"))  process_factory_t:simple_factory_t<process_factory_t,true,false>
			{
				enum{
					id_env=1,
					id_exitcode=2,
					id_terminate=3
				};

				struct process_disp_t:IDispatch_base_t<process_disp_t> // IDispatch_impl_t<process_disp_t>					
				{
					inline HRESULT findname(LPOLESTR name,DISPID& dispid){

						HRESULT hr;
						static  BEGIN_STRUCT_NAMES(pn)										
							DISP_PAIR_IMPLC(call,DISPID_VALUE)								
							DISP_PAIR_IMPLC(run,DISPID_VALUE)								
							DISP_PAIR_IMPLC_LEN(env,id_env,3)
							DISP_PAIR_IMPLC_LEN(error,id_exitcode,3)
							DISP_PAIR_IMPLC(exitcode,id_exitcode)
							DISP_PAIR_IMPLC_LEN(terminate,id_terminate,4)
							END_STRUCT_NAMES;
						hr=find_id(pn,name,&dispid);
						return hr;
					}



					inline DWORD get_pid()    
					{
						return InterlockedCompareExchange(&pid,0,0);
					};


					inline	HRESULT impl_Invoke(DISPID id,REFIID riid,LCID,WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi, UINT*){



						arguments_t args(dp);

						HRESULT hr=E_UNEXPECTED;




						try
						{

							arguments_t args(dp);
							if((id==id_terminate)&&(DISPATCH_METHOD&flags))
							{
								process_handle_t ph(get_pid());
								HANDLE h;
								return (((h=ph.get_handle(PROCESS_TERMINATE))&&TerminateProcess(h,-1)))?S_OK:HRESULT_FROM_WIN32(GetLastError());

							}

							if((id==id_exitcode)&&is_get(flags)&&res)
							{
								VARIANT v={VT_I4};

								//locker_t<mutex_cs_t> lock(mutex);
								process_handle_t ph(get_pid());
								DWORD exitcode=-1;


								if(!GetExitCodeProcess(ph,&exitcode))
									return HRESULT_FROM_WIN32(GetLastError());

								int fmt=args[0].def<int>(0);
								if(fmt)
								{
									v.vt=VT_BSTR;
									bstr_t msg;
									if(fmt==16) msg=v_buf<wchar_t>().printf(L"0x%08x",exitcode);
									else msg=error_msg(exitcode);
									v.bstrVal=msg.Detach();

								}
								else 							  					  
									v.intVal=exitcode;




								*res=v;
								return S_OK;
							}




							if(id==id_env)
							{
								locker_t<mutex_cs_t> lock(mutex);
								int flagApp;
								bstr_t e=args[0].def(bstr_t());
								if(e.length()){								  
									argv_zzs<wchar_t> argsE;
									argsE= argv_zz<wchar_t>((wchar_t*)env); 							     
									env=argsE.flat_str();	
								}

								hr=toVARIANT(static_cast<IUnknown*>(this),res);
								return hr;
							}





							if((id!=0)||is_put(flags) )return hr=E_NOTIMPL;
							//OLE_CHECK_PTR(res);

							//inline int start_process_desktop(bstr_t desktop,bstr_t lpCmdLine,DWORD waittime=INFINITE,PROCESS_INFORMATION* ppi=NULL,DWORD crflags=0,void* penv=0,bool finher=0)

							bstr_t cmd;
							argv_zzs<wchar_t> argsE;
							DWORD  CF;

							{

								locker_t<mutex_cs_t> lock(mutex);

								//exitcode=-1;			
								if(filename.length())
								{
									moniker_parser_t<wchar_t> mon;
									wchar_t* pfn=filename;
									pfn=mon.trim(pfn);
									pfn=mon.undecorate(pfn);
									filename=bstr_t(L"\"")+pfn+bstr_t(L"\" ");
									cmd=filename+cmdline;

								}
								else cmd=cmdline;


								if(env.length())
									argsE= argv_zz<wchar_t>((wchar_t*)env); 							     
								else argsE= argv_env<wchar_t>(); 




								CF=cf|CREATE_UNICODE_ENVIRONMENT;
							}

							processinfo_t processinfo;
							event_local_t hevent;
							smart_ptr_t <process_args_t> process_args(new process_args_t(hevent,dp));
							stub_holder_t stub_holder(process_args);
							argsE[LTX_PROCESS_OBJREF]=stub_holder.objref();

							InterlockedExchange(&pid,0);
							wchar_t* pent=( wchar_t*)argsE.flat_str();
							if(!start_process_desktop(desktop,cmd,0,&processinfo,CF,pent))
								return HRESULT_FROM_WIN32(GetLastError());

							InterlockedExchange(&pid,processinfo.dwProcessId);

							HANDLE hh[2]={hevent.hko,processinfo.hProcess};							
							DWORD ws;
							ws=WaitForMultipleObjects(2,hh,false,INFINITE);
							/*
							if(ws!=WAIT_OBJECT_0)
							{
							locker_t<mutex_cs_t> lock(mutex);
							DWORD ec=-1;
							if(GetExitCodeProcess(processinfo.hProcess,&ec));
							exitcode=ec;

							//if(ec) return hr=HRESULT_FROM_WIN32(ec);
							}
							*/

							if(res)
							{
								*res=process_args->result.Detach();
							}

							process_args->clear();
							return S_OK;



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

					process_disp_t(bstr_t _desktop,bstr_t _filename,bstr_t _cmdline,bstr_t _env,int _wshow,DWORD _cf)
						:wshow(_wshow),cf(_cf),xexitcode(-1),pid(0)
					{
						desktop=_desktop;
						filename=_filename;
						cmdline=_cmdline;
						env=_env;

					}


					mutex_cs_t mutex;
					bstr_t desktop,filename,cmdline,env;
					int wshow;
					DWORD cf;
					volatile LONG pid;
					LONG xexitcode;


				};


				struct process_git_t: IGITDispatch_t<process_git_t>
				{
					process_git_t(DWORD dw):IGITDispatch_t<process_git_t>(dw){};
					process_git_t(IDispatch* pd):IGITDispatch_t<process_git_t>(pd){};

				};

				struct creator_process_t
				{
					HRESULT hr;
					DWORD dwCookie;
					smart_ptr_t<process_disp_t> pd;

					creator_process_t(process_disp_t *p):pd(p,false),hr(E_FAIL),dwCookie(0){}
					inline HRESULT	operator()(DWORD t)
					{
						return hr=GIT_t::get_instance().wrap((IDispatch*)pd.p,&dwCookie);
					}

				};


				inline HRESULT create_iaggregator(aggregator_container_helper_t& container,LPOLESTR pszDisplayName,IStream* ,GUID& uiid,smart_ptr_t<iaggregator_helper>& disp )
				{
					HRESULT hr;   
					moniker_parser_t<wchar_t> mp(pszDisplayName);
					wchar_t* desk=mp.trim(mp.args[L"desk"]);
					wchar_t* filename=mp.trim(mp.args[L"file"]);
					wchar_t* cmdline=mp.trim(mp.args[L"cmd"]);

					DWORD cf=mp.args[L"cf"].def<int>(0);
					DWORD sw=mp.args[L"sw"].def<int>(SW_SHOW);

					//process_disp_t* process=;


					process_git_t* process_git;


					creator_process_t cp(new process_disp_t(desk,filename,cmdline,bstr_t(),sw,cf));


					DWORD tt=0;   
					if(COMInit_t::is_MTA())	hr=cp(tt);
					else hr=mta_singleton_t::apartment().call_sync(cp,hr,E_UNEXPECTED);						


					if(FAILED(hr)) return hr;

					process_git=new process_git_t(cp.dwCookie);

					disp=aggregator_helper_creator_t<process_git_t>::New(process_git);

					mta_singleton_t::add_external_connector(container);
					return (disp)?S_OK:E_OUTOFMEMORY;

				}






			};


		}; // script_engine
	}; // com_apartments
}; //ipc_utils
