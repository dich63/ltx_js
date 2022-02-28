#pragma once


#include "ltx_js.h"
#include "call_functor_reverse.h"
#include "com_apartments.h"


#include "com_global.h" 



#include <activscp.h>
#include <comdef.h>
#include <dispex.h>

#define __RPC__inout_xcount(size) 
#define __RPC__in_xcount(size) 

#include "activdbg.h"
#include <shlwapi.h>
#include "resource.h"

#include "moniker_parsers.h"
#include "dispatch_function.h"

#include "com_args.h"
//#include "static_com.h"
#include "buf_fm.h"

#pragma comment(lib, "shlwapi.lib") 

#define __Lpn(a) L###a

#define _Lpn(...) __Lpn(__VA_ARGS__)

#define js_text(...) __Lpn(__VA_ARGS__)

namespace ipc_utils {
	namespace com_apartments
	{
		namespace script_engine
		{


			struct ExternalDebugger_t{

				ExternalDebugger_t():dwAppCookie(0),hr(E_POINTER){ };

				void init()
				{
					dwAppCookie=0;
					//FatalAppExit(0,L"AAAAAAAAAAAAAAA");
					OLE_CHECK_VOID(hr=pdm.CoCreateInstance(__uuidof(ProcessDebugManager)));
					OLE_CHECK_VOID(hr=pdm->CreateApplication(&DebugApp.p));
					DebugApp->SetName(L"Scripting Application");
					OLE_CHECK_VOID(hr=pdm->AddApplication(DebugApp.p, &dwAppCookie));
					OLE_CHECK_VOID(hr=pdm->CreateDebugDocumentHelper(NULL, &DebugDocHelper.p));
					//hr=DebugDocHelper->Init(DebugApp,bstr_t("AAA"),bstr_t("AAA/AAA"),TEXT_DOC_ATTR_READONLY);
					//
					hr=DebugDocHelper->Init(DebugApp,NULL,NULL,NULL);
					hr=DebugDocHelper->Attach(NULL);
				}

				~ExternalDebugger_t()
				{
					if(!pdm.p) return;
					DebugDocHelper.Release();
					DebugApp.Release();
					pdm->RemoveApplication(dwAppCookie);

				}


				inline IProcessDebugManager* get_ProcessDebugManager()
				{
					return pdm.p;
				}

				inline IDebugDocumentHelper* get_DebugDocumentHelper()
				{
					return DebugDocHelper.p;
				}
				inline smart_ptr_t<IDebugApplication>& get_DebugApplication()
				{
					return DebugApp;
				}

				inline operator bool(){ 
					return SUCCEEDED(hr);
				}
				HRESULT  hr;
				DWORD dwAppCookie;
				smart_ptr_t<IProcessDebugManager> pdm;
				smart_ptr_t<IDebugApplication>    DebugApp;
				smart_ptr_t<IDebugDocumentHelper>  DebugDocHelper;

			};

			template <class T>
			struct __declspec(novtable) IActiveScriptSite_impl:
				public IActiveScriptSite, 
				public IActiveScriptSiteWindow,
				public IActiveScriptSiteDebug
			{

				inline T* this_T()
				{
					return static_cast<T*>(this);
				}

				template<class Intf,class T>
				inline bool   _CheckQI(T* _this,REFIID iid,void ** ppObj)
				{
					bool f=InlineIsEqualGUID(iid,__uuidof(Intf));
					if(f)  *ppObj= (void*)static_cast<Intf*>(_this);
					return f;
				}

				inline    IActiveScriptSite* ScriptSite()
				{
					return static_cast<IActiveScriptSite*>(this);
				}

				inline T& init(bool _fdebug=false)
				{
					if(_fdebug) dbg.init();

					return *this_T();
				}

				IActiveScriptSite_impl(bool _fdebug=false):fdebug(_fdebug),refcount(1){
					//init(_fdebug);
				};



				STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject)
				{
					if(!ppvObject) return E_POINTER;

					if(!_CheckQI<IActiveScriptSite>(this,iid,ppvObject))
						if(!_CheckQI<IActiveScriptSiteWindow>(this,iid,ppvObject)) 
							if(!_CheckQI<IActiveScriptSiteDebug>(this,iid,ppvObject))
								//if(!(dbg&&_CheckQI<IActiveScriptSiteDebug>(this,iid,ppvObject)))
								return E_NOINTERFACE;

					//this->AddRef();   
					return S_OK;

				}


				STDMETHOD_(ULONG,AddRef)()
				{
					//
					return 1;
					//return InterlockedIncrement(&refcount);
				}
				STDMETHOD_(ULONG,Release)()
				{
					return 1;
					ULONG l = InterlockedDecrement(&refcount);
					if (l == 0)
						delete this_T();
					return l;
				}


				// IActiveScriptSite
				STDMETHOD(GetLCID)(LCID __RPC_FAR *plcid)
				{return E_NOTIMPL;}


				inline HRESULT OnItemInfo(  LPCOLESTR pstrName,DWORD dwReturnMask,
					IUnknown **ppiunkItem,ITypeInfo **ppti)
				{
					return E_NOTIMPL;
				}
				STDMETHOD(GetItemInfo)(  LPCOLESTR pstrName,DWORD dwReturnMask,
					IUnknown **ppiunkItem,ITypeInfo **ppti)
				{ 		
					return  this_T()->OnItemInfo(pstrName,dwReturnMask,ppiunkItem,ppti);			
				} 

				STDMETHOD(GetDocVersionString)( 
					BSTR*pbstrVersion)
				{ 
					return E_NOTIMPL;
				} 

				STDMETHOD(OnScriptTerminate)( 
					const VARIANT  *pvarResult,
					const EXCEPINFO  *pexcepinfo)
				{ 
					return E_NOTIMPL;
				} // end OnScriptTerminate

				STDMETHOD(OnStateChange)( 
					SCRIPTSTATE ssScriptState)
				{ 
					return S_OK;
				} 

				inline void OnError(IActiveScriptError  *pscripterror){};

				STDMETHOD(OnScriptError)( 
					IActiveScriptError  *pscripterror)
				{ 

					this_T()->OnError(pscripterror);

					return S_OK;
				} // end OnScriptError

				STDMETHOD(OnEnterScript)( void)
				{ 
					return S_OK;
				} // end OnEnterScript

				STDMETHOD(OnLeaveScript)( void)
				{ 
					return S_OK;
				} 
				////*
				STDMETHOD(GetWindow)( 
					HWND __RPC_FAR *phwnd)
				{
					if(phwnd)  	*phwnd = NULL;

					return S_OK;
				} // end GetWindow

				STDMETHOD(EnableModeless)( 
					BOOL fEnable)
				{
					return S_OK;
				} // end EnableModeless

				STDMETHOD(GetDocumentContextFromPosition)
					(
					DWORD_PTR dwSourceContext,// As provided to ParseScriptText 
					// or AddScriptlet 
					ULONG uCharacterOffset,// character offset relative 
					// to start of script block or scriptlet 
					ULONG uNumChars,// Number of characters in context 
					// Returns the document context corresponding to this character-position range. 
					IDebugDocumentContext **ppsc)
				{


					ULONG ulStartPos = 0;
					HRESULT hr= E_NOTIMPL;
					return hr;
				}

				STDMETHOD(GetApplication)(IDebugApplication** ppda)
				{
					//		return E_NOTIMPL;
					//if(!dbg) return E_NOTIMPL;
					HRESULT hr; 
					if(!ppda) return E_POINTER;
					if(dbg)
						return	hr=dbg.get_DebugApplication().CopyTo(ppda);
					else	return E_NOTIMPL;
				};


				STDMETHOD(GetRootApplicationNode)(IDebugApplicationNode **ppdanRoot)
				{
					//		return E_NOTIMPL;
					//if(!dbg) return E_NOTIMPL;
					HRESULT hr,hrN; 	
					if(!dbg) return E_NOTIMPL;
					/*
					APPBREAKFLAGS appbf;
					DWORD count;
					com_ptr_t<IRemoteDebugApplicationThread> rdat;
					if(SUCCEEDED(dbg.DebugApp->GetBreakFlags(&appbf,&rdat.p)))
					{
					rdat->Resume(&count);
					}
					//*/





					//hrN=dbg.get_DebugDocumentHelper()->SetShortName(L"AAAAAAJ");

					if (!ppdanRoot) return E_INVALIDARG;								
					if(SUCCEEDED(hr= dbg.get_DebugDocumentHelper()->GetDebugApplicationNode(ppdanRoot)))
					{
						//  

					}

					return hr;

				}

				inline bool OnScriptErrorWhenContinuing(){ return true;}
				inline bool EnterDebugger(){ return true;}

				STDMETHOD(OnScriptErrorDebug)( IActiveScriptErrorDebug *pErrorDebug, 
					// whether to pass the error to the debugger to do JIT debugging 
					BOOL*pfEnterDebugger, 
					// whether to call IActiveScriptSite::OnScriptError() when the user 
					// decides to continue without debugging 
					BOOL *pfCallOnScriptErrorWhenContinuing)
				{
					//return E_NOTIMPL;
					(pfEnterDebugger)&&(*pfEnterDebugger =this_T()->EnterDebugger());		
					(pfCallOnScriptErrorWhenContinuing)&&(*pfCallOnScriptErrorWhenContinuing =this_T()->OnScriptErrorWhenContinuing());

					return S_OK;

				};

				bool fdebug;
				volatile LONG refcount;
				ExternalDebugger_t dbg;
			};

			/*
			struct ArgsDispatch_t:CDispatchT<ArgsDispatch_t>
			{
			bool fcopy;
			enum {
			offset_id=17
			};
			std::vector<VARIANT> vargs;

			inline int check(int i)
			{
			return (0<=i)&&(i<vargs.size());
			}




			inline HRESULT GetDispID(BSTR bstrName,DWORD grfdex,DISPID *pid	)
			{ 
			HRESULT hr;
			if(!pid) return E_POINTER;
			if(bstrName)
			{
			int i;
			if(StrStrNIW(bstrName,L"len",3)) { *pid=-2; return S_OK;}
			if(StrStrNIW(bstrName,L"def",3)) { *pid=-4; return S_OK;}
			else if(StrToIntExW(bstrName,STIF_SUPPORT_HEX,&i))
			{
			if(check(i))
			*pid=i+offset_id;
			else *pid=-3;

			return S_OK;
			}

			}

			return E_INVALIDARG; 
			};


			void do_invoke(int wFlags,int dispid,int argc,VARIANT* argv,VARIANT& res,HRESULT &hr)
			{ 
			hr=S_OK;
			VARIANT vl={VT_I4},ve={VT_EMPTY};
			int indx;
			if(is_get(wFlags))
			{

			if((dispid==DISPID_VALUE)||(dispid==-4)) {

			if(argc>0)
			{
			VARIANT vi={};
			OLE_CHECK_VOID(hr=VariantChangeType(&vi,&argv[argc-1],0,VT_I4));
			if(check(vi.intVal))
			{
			res=variant_t(vargs[vi.intVal]).Detach();

			}
			else 
			if(argc>1)
			{
			res=variant_t(argv[argc-2]).Detach();
			}
			else res=ve;								
			}
			return;

			}





			if(dispid==-2) { vl.intVal=vargs.size();res=vl;return;}

			indx=dispid-offset_id;
			if(check(indx))
			res=variant_t(vargs[indx]).Detach();
			else res=ve;
			}


			};

			inline void clear(){ 

			vargs.clear(); 
			};

			inline HRESULT fill(int argc,VARIANT* argv,bool freverse=false)
			{
			HRESULT hr=S_OK;
			if(argc>0)
			{
			vargs.resize(argc);
			if(freverse) std::reverse_copy(argv,argv+argc,vargs.begin());
			else std::copy(argv,argv+argc,vargs.begin());
			}
			return hr;
			}

			};
			*/


			struct script_engine_t
			{
				//CComInit _cini;
				//com_ptr_t<IActiveScriptSite> site;// dangerouss!!!!!!!!!!! selflock

				typedef   global_com_t<script_engine_t> process_com_t;



				struct Site_t:IActiveScriptSite_impl<Site_t>
				{
					//Site_t(script_engine_t* po,bool fdebug):IActiveScriptSite_impl<Site_t>(fdebug),owner(po){};
					struct weak_ref_t
					{

						typedef void (* proc_t)( variant_t* result, script_engine_t & engine,pcallback_context_arguments_t pcc,int argc,variant_t* argv,int arglast,HRESULT& hr,EXCEPINFO* pei,unsigned flags);

						static   void __cdecl  call
							( variant_t* result, weak_ref_t * pweak,pcallback_context_arguments_t pcc,int argc,variant_t* argv,int arglast,HRESULT& hr,EXCEPINFO* pei,unsigned flags)
						{

							if(!(pweak->owner&&pweak->proc)) hr=RPC_E_CALL_REJECTED;
							else pweak->proc(result,*(pweak->owner),pcc,argc,argv,arglast,hr,pei,flags);

						}
						weak_ref_t(script_engine_t *o,void* s_proc):owner(o),proc(proc_t (s_proc)){};

						static   void __cdecl  on_exit(weak_ref_t * pweak)
						{
							delete pweak;
						}

						script_engine_t *owner;
						proc_t proc;

					};
					std::list< smart_ptr_t<IUnknown>  > ulist;
					std::list< weak_ref_t*> weak_list;

					//weak_ref_t* pweak_engine,*pweak_engine2;

					//Site_t():pweak_engine(0),pweak_engine2(0){};
					Site_t(){};

					inline void close()
					{
						//if(pweak_engine) pweak_engine->owner=0;
						//if(pweak_engine2) pweak_engine2->owner=0;
						std::list< weak_ref_t*>::iterator i;
						for(i=weak_list.begin();i!=weak_list.end();++i)
							(*i)->owner=0;


					}



					inline void setup(script_engine_t* po,bool fdebug)
					{ 
						owner=po;
						init(fdebug); 
					}

					inline bool OnScriptErrorWhenContinuing(){ return true;}
					inline bool EnterDebugger(){ return owner->ferrorbreak;}
					inline void OnError(IActiveScriptError  *pscripterror){
						//	owner->error=pscripterror;
						//	if(pscripterror) pscripterror->GetExceptionInfo(owner->exceptinfo.address());
						owner->on_error(pscripterror);

					};
					script_engine_t *owner;


					inline HRESULT attach_item(IUnknown *ppiunkItem,weak_ref_t* pwr=0)
					{
						smart_ptr_t<IUnknown> unk(ppiunkItem,1);
						ulist.push_front(unk);
						if(pwr) weak_list.push_front(pwr);
						return S_OK;
					}


					inline HRESULT OnItemInfo(  LPCOLESTR pstrName,DWORD dwReturnMask,
						IUnknown **ppiunkItem,ITypeInfo **ppti)
					{
						HRESULT hr;
						if(SCRIPTINFO_IUNKNOWN&dwReturnMask)
						{
							if(pstrName)
							{

								if(!ppiunkItem) return E_POINTER;

								if(wcscmp(pstrName,L"require")==0){
									//
									//buf_res_T<char> bres(module_t<>::__getHinstance(),MAKEINTRESOURCEW(IDR_REQUIRE_JS),RT_HTML);
									//
									buf_res_T<char> bres(module_t<>::__getHinstance(),L"requirejs",RT_HTML);
									//bstr_t s=bstr_t(L"[")+bstr_t(bres)+bstr_t(L"][0]");
									VARIANT r={};
									OLE_CHECK_hr(owner->__call(bstr_t(bres),&r));
									*ppiunkItem=r.punkVal;
									return S_OK;

								}



								if((wcscmp(pstrName,L"$$")==0)||(wcscmp(pstrName,L"arguments")==0))
								{
									*ppiunkItem=owner->args.get_Dispatch();	
									return S_OK;
								}

								if ((wcscmp(pstrName, L"call_apply") == 0))
								{
									*ppiunkItem = function_apply_t::get_ptr();
									return S_OK;
								}

								if(wcscmp(pstrName,L"globalEval")==0)
								{
									// owner->AddRef();
									//create_dispatch_wrapper(&script_engine_t::global_eval,owner,&script_engine_t::s_on_exit,owner,(IDispatch**)ppiunkItem);


									//create_dispatch_wrapper(&weak_ref_t::call,pweak_engine,&weak_ref_t::on_exit,pweak_engine,(IDispatch**)ppiunkItem);
									//smart_ptr_t<IUnknown> unk(new IDispatch_empty_RC_t<1>,false);
									//ulist.push_back(unk);
									//*ppiunkItem =unk;
									//*ppiunkItem =new IDispatch_empty_RC_t<1>;
									weak_ref_t* pweak_engine =new weak_ref_t(owner,&script_engine_t::global_eval);
									*ppiunkItem=new calback_impl_t<>(&weak_ref_t::call,pweak_engine,&weak_ref_t::on_exit);
									return attach_item(*ppiunkItem,pweak_engine);
								}
								if((wcscmp(pstrName,L"self")==0)||(wcscmp(pstrName,L"global")==0)) 
								{
									(*ppiunkItem=owner->dispex.p)->AddRef();
									return S_OK;
								}



								if(wcscmp(pstrName,L"process")==0){


									OLE_CHECK_hr(GIT_t::get_instance().unwrap(process_com_t::get_instance().js_cookie(true),ppiunkItem,false));
									return hr;
									//return attach_item(*ppiunkItem); 

								}
								if(wcscmp(pstrName,L"bindObject")==0){
									return hr=::ltxGetObject(L"ltx.bind:",__uuidof(IUnknown),(void**)ppiunkItem);																			 

								}
								if(wcscmp(pstrName,L"externRef")==0)
								{
									weak_ref_t* pweak_engine =new weak_ref_t(owner,&script_engine_t::extern_ref);
									*ppiunkItem=new calback_impl_t<>(&weak_ref_t::call,pweak_engine,&weak_ref_t::on_exit);
									return attach_item(*ppiunkItem,pweak_engine);

								}








								return S_OK; 

							}

						}
						return TYPE_E_ELEMENTNOTFOUND;
					}



				} site;


				static   void __cdecl  extern_ref
					( variant_t* result, script_engine_t & engine,void* error,int argc,variant_t* argv,int arglast,HRESULT& hr,EXCEPINFO* pei,unsigned flags)
				{
					hr=E_NOTIMPL;
					if(flags&DISPATCH_METHOD)
					{
						IDispatch* pdisp=(IDispatch*)InterlockedCompareExchangePointer((void**)&(engine.pweak_ref),0,0);
						if(pdisp) 
						{
							hr=toVARIANT(pdisp,result);
						}
						else hr=E_POINTER;

					}			


				}




				static   void __cdecl  global_eval
					( variant_t* result, script_engine_t & engine,pcallback_context_arguments_t pcca,int argc,variant_t* argv,int arglast,HRESULT& hr,EXCEPINFO* pei,unsigned flags)
				{
					//script_engine_t & engine=*((script_engine_t *)pctx);
					if(argc>0)
					{
						//variant_t txt;  	  
						//OLE_CHECK_VOID(hr=VariantChangeType(&txt,argv,0,VT_BSTR));   
						variant_cast_t<VT_BSTR> vc(argv[arglast]);
						OLE_CHECK_VOID(hr=vc.hr);
						engine.set_args(argc-1,argv+0,true);
						hr=engine._call(vc->bstrVal,result);
						if(FAILED(hr)&&engine.error&&pei)
						{
							//*pei=make_detach((*(EXCEPINFO*)(&)),EXCEPINFO());  
							engine.exceptinfo.detach(pei);
							hr=DISP_E_EXCEPTION;

						}
						engine.gc();

					}
					//*presult=variant_t(int(apt));

				};

				static   void __cdecl  global_eval_ex
					( variant_t* result, script_engine_t & engine,pcallback_context_arguments_t pcca,int argc,variant_t* argv,int arglast,HRESULT& hr,EXCEPINFO* pp,unsigned flags)
				{
					//script_engine_t & engine=*((script_engine_t *)pctx);

					if(argc>0)
					{
						//variant_t txt;  	  
						//OLE_CHECK_VOID(hr=VariantChangeType(&txt,argv,0,VT_BSTR));   
						if(is_VARIANT_empty(argv[argv[arglast]]))
						{
							global_eval(result,engine,pcca,argc-1,argv+1,arglast-1,hr,pp,flags);

						}
						else {



						}






					}
					//*presult=variant_t(int(apt));

				};




				static	  void __cdecl  s_on_exit(script_engine_t* pengine)
				{
					//	   delete pengine;
					bstr_c_t<64> ft=L"try{ for ( var k in this) delete this[k];} catch(e){};";
					//pengine->call(L"try{ for ( k in this) delete this[k];} catch(e){};");
					pengine->__call(ft);
					pengine->gc(SCRIPTGCTYPE_EXHAUSTIVE);
					pengine->Release();

				}



				~script_engine_t()
				{
					//gc();

					site.close();
					site.ulist.clear();




					dispex.Release();
					disp.Release();
					error.Release();
					parser.Release();								       	  
					//
					if(engine)  
					{
						EXCEPINFO e={};
						//hr=engine->InterruptScriptThread(SCRIPTTHREADID_ALL,&e,SCRIPTINTERRUPT_RAISEEXCEPTION);
						//
						hr=engine->Close();
					}
					engine.Release();

				}

				script_engine_t(bstr_t lang=L"JScript",int fdebug=0*3,bool fwith_process=true)
				{
					//site.Attach(new Site_t(this,fdebug));	
					hr_ex=E_FAIL;
					pweak_ref=0;
					ferrorbreak=2&fdebug;
					site.setup(this,fdebug);
					OLE_CHECK_VOID(hr=engine.CoCreateInstance(lang));
					OLE_CHECK_VOID( hr=engine->SetScriptSite(site.ScriptSite()) );
					OLE_CHECK_VOID( hr=engine.QueryInterface(&(parser.p)) );
					hr=engine.QueryInterface(&(GarbageCollector.p));


					OLE_CHECK_VOID( hr=parser->InitNew()    );
					OLE_CHECK_VOID(	hr=engine->GetScriptDispatch(bstr_t(L"\0\0"),&disp.p)); 
					hr_ex=disp.QueryInterface(&dispex.p);

					DWORD dwFlags=SCRIPTITEM_ISVISIBLE|SCRIPTITEM_ISSOURCE;

					if(SUCCEEDED(hr_ex)) OLE_CHECK_VOID( hr=engine->AddNamedItem(bstr_t(L"self"),dwFlags));
					if(SUCCEEDED(hr_ex)) OLE_CHECK_VOID( hr=engine->AddNamedItem(bstr_t(L"global"),dwFlags));
					OLE_CHECK_VOID( hr=engine->AddNamedItem(bstr_t(L"$$"),dwFlags));
					OLE_CHECK_VOID( hr=engine->AddNamedItem(bstr_t(L"arguments"),dwFlags));
					//
					//		
					OLE_CHECK_VOID( hr=engine->AddNamedItem(bstr_t(L"globalEval"),dwFlags));
					OLE_CHECK_VOID( hr=engine->AddNamedItem(bstr_t(L"bindObject"),dwFlags));
					OLE_CHECK_VOID( hr=engine->AddNamedItem(bstr_t(L"externRef"),dwFlags));
					OLE_CHECK_VOID( hr=engine->AddNamedItem(bstr_t(L"require"),dwFlags));
					OLE_CHECK_VOID(hr = engine->AddNamedItem(bstr_t(L"call_apply"), dwFlags));




					if(fwith_process)
					{
						//						process_com_t::get_instance().js_cookie(true);

						OLE_CHECK_VOID( hr=engine->AddNamedItem(bstr_t(L"process"),SCRIPTITEM_ISSOURCE|SCRIPTITEM_ISVISIBLE));
					}




					OLE_CHECK_VOID(hr=engine->SetScriptState(SCRIPTSTATE_INITIALIZED));
					OLE_CHECK_VOID(hr=engine->SetScriptState(SCRIPTSTATE_STARTED));

					/*
					if(0)
					{

					VARIANT yy={VT_DISPATCH}  ;
					site.pweak_engine =new Site_t::weak_ref_t(site.owner,&script_engine_t::global_eval);


					//create_dispatch_wrapper(&Site_t::weak_ref_t::call,site.pweak_engine,&Site_t::weak_ref_t::on_exit,site.pweak_engine,&yy.pdispVal);
					yy.pdispVal=new calback_impl_t<>(Site_t::weak_ref_t::call,site.pweak_engine,&Site_t::weak_ref_t::on_exit);
					set_item(L"globalEval",yy);
					VariantClear(&yy);


					yy.vt=VT_DISPATCH; 					 
					yy.pdispVal=new IDispatch_empty_RC_t<1>;
					set_item(L"dummy",yy);
					VariantClear(&yy);
					}//*/

					// init

					//  buf_res_T<char> bres(module_t<>::__getHinstance(),MAKEINTRESOURCEW(101),RT_HTML);
					// if(!fwith_process)
					HRESULT hrr;
					if(0){


						//buf_res_T<char> bres0(module_t<>::__getHinstance(),MAKEINTRESOURCEW(IDR_REQUIRE_JS),RT_HTML);
						//if(bres0)	  								hrr= _call(bstr_t(bres0));




						buf_res_T<char> bres(module_t<>::__getHinstance(),MAKEINTRESOURCEW(IDR_JSON_JS),RT_HTML);

						//return ;	  


						if(bres)						  	
							hrr= _call(bstr_t(bres));

						buf_res_T<char> bres2(module_t<>::__getHinstance(),MAKEINTRESOURCEW(IDR_JSUTILS_JS),RT_HTML);
						if(bres2)						  	
							hrr= _call(bstr_t(bres2));
					}
					else {

						//buf_res_T<char> bres0(module_t<>::__getHinstance(),MAKEINTRESOURCEW(IDR_REQUIRE_JS),RT_HTML);
						buf_res_T<char> bres1(module_t<>::__getHinstance(),MAKEINTRESOURCEW(IDR_JSON_JS),RT_HTML);
						buf_res_T<char> bres2(module_t<>::__getHinstance(),MAKEINTRESOURCEW(IDR_JSUTILS_JS),RT_HTML);
						//bstr_t bscript=bstr_t(bres0)+bstr_t(L"\n")+bstr_t(bres1)+bstr_t(L"\n")+bstr_t(bres2)+bstr_t(L"\n");
						bstr_t bscript=bstr_t(L"\n")+bstr_t(bres1)+bstr_t(L"\n")+bstr_t(bres2)+bstr_t(L"\n");
						hrr= _call(bscript);

					}





				}




				inline HRESULT gc(SCRIPTGCTYPE st=SCRIPTGCTYPE::SCRIPTGCTYPE_NORMAL){

					HRESULT hr=E_NOTIMPL;
					GarbageCollector&&(hr=GarbageCollector->CollectGarbage(st));
					return hr;
				}

				HRESULT call(bstr_t txt,VARIANT* pres=0,int argc=0,VARIANT* argv=0,EXCEPINFO* pei=0)
				{
					if(FAILED(this->hr)) return this->hr;
					HRESULT hr;
					set_args(argc,argv);
					hr=_call(txt,pres,pei);
					gc();
					return hr;
				}
				HRESULT _call(BSTR txt,VARIANT* pres=0,EXCEPINFO* pei=0)
				{

					HRESULT hr0=__call(txt,pres,pei);					
					args.clear();
					return hr0;

				}


				inline HRESULT parse_script_text(BSTR& txt,variant_t& buf)
				{
					HRESULT hr;
					if(txt&&(txt[0]==L':'))
					{

						wchar_t* tail,*p=txt+1;
						moniker_parser_t<wchar_t> mp;
						if(mp._lcheckni(p,L"file:",5,&tail))
						{
							/*script_engine_t eng;
							tail= mp.undecorateQQ(tail);
							variant_t a=tail;							
							if(SUCCEEDED(hr=eng.call(L"load_string($$[0])",&buf,1,&a)))
							*/

							bool fr=mp._lcheckni(tail,L":",1,&tail)||mp._lcheckni(tail,L"r:",2,&tail);							
							bstr_t b;
							OLE_CHECK_hr(mp.load_string(tail,b));
							if(fr&&tail)
							{
								variant_t vs=tail;
								//OLE_CHECK_hr(call(L"global.require.add_to_module_path(arguments[0]+'/../');\n\n",0,1,&vs));
								const wchar_t* p=js_text(
									require;
								global.__script_triplet__=os.fs_triplet(arguments[0]);
								global.require.add_to_module_path(global.__script_triplet__[0]);

								);
								OLE_CHECK_hr(call(p,0,1,&vs));										

							}



							buf.vt=VT_BSTR;
							txt=buf.bstrVal=b.Detach();						


						}
						else if(mp._lcheckni(p,L"clipbrd:",5,&(tail=0)))
						{
							bstr_t t=ClipboardText();
							buf.vt=VT_BSTR;
							txt=buf.bstrVal=t.Detach();
						}
					}

					return S_OK;
				}




				HRESULT __call(BSTR txt,VARIANT* pres=0,EXCEPINFO* pei=0)
				{
					//EXCEPINFO ei={};
					//pei||(pei=&ei);
					if(!safe_len(txt)) return S_FALSE;

					variant_t tmp,tmp2;

					pres||(pres=&tmp);
					HRESULT hr0;
					DWORD_PTR dw=DWORD_PTR(L"KKKAAA");
					error.Release();
					//args.fill(argc,argv);
					if(SUCCEEDED(hr0=parse_script_text(txt,tmp2)))					
						hr0=parser->ParseScriptText(txt,NULL,NULL,NULL,dw,0,SCRIPTTEXT_ISEXPRESSION|SCRIPTTEXT_ISVISIBLE,pres,pei);
					//hr0=parser->ParseScriptText(txt,NULL,NULL,NULL,dw,0,SCRIPTTEXT_ISVISIBLE,pres,pei);
					//args.clear();
					return hr0;

				}

				void set_args(int argc=0,VARIANT* argv=0,bool freverse=false)
				{   
					//args.fill(argc,argv,freverse);
					args.freverse=freverse;
					args.reset(argc,argv); 

				}



				HRESULT set_item(bstr_t  name,VARIANT value)
				{
					if(FAILED(hr_ex)) return hr_ex;
					HRESULT h;
					DISPID did,dput;
					if(SUCCEEDED(h=dispex->GetDispID(name,fdexNameEnsure,&did)))
					{
						dput=DISPID_PROPERTYPUT;
						variant_t r;
						DISPPARAMS dp={&value,&dput,1,1};
						//
						const DWORD dwf=DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF;
						h=dispex->Invoke(did, IID_NULL, LOCALE_USER_DEFAULT, dwf,&dp,&r, NULL, NULL);
						//h=dispex->Invoke(did, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT,&dp,&r, NULL, NULL);
					}
					return h;
				}
				HRESULT get_item(bstr_t name,VARIANT* pvalue)
				{
					if(FAILED(hr)) return hr;

					HRESULT h;
					DISPID did;
					if(SUCCEEDED(h=disp->GetIDsOfNames(IID_NULL, &name.GetBSTR(), 1, LOCALE_USER_DEFAULT, &did)))
					{
						DISPPARAMS dp={};
						h=disp->Invoke(did, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,&dp,pvalue, NULL, NULL);
					}

					return h;
				}

				inline void on_error(IActiveScriptError* pscripterror)
				{
					HRESULT hr;
					DWORD dwSc=-1;
					ULONG ln=-1;
					LONG cp=-1;

					error=pscripterror;
					if(pscripterror)
					{
						OLE_CHECK_VOID(hr=error->GetExceptionInfo(exceptinfo.address()));
						hr=error->GetSourcePosition(&dwSc,&ln,&cp);
						bstr_t bline,bc=L"\n";
						bstr_t bdsc(make_detach<BSTR>(exceptinfo.bstrDescription),0);
						if(SUCCEEDED(hr=error->GetSourcePosition(&dwSc,&ln,&cp)))
						{
							wchar_t buf[128];
							wsprintf(buf,L"\n[%d,%d] ",ln+1,cp);
							bdsc+=bstr_t(buf);
							bc=L"";

						};

						if(SUCCEEDED(hr=error->GetSourceLineText(bline.GetAddress())))
						{						 
							bdsc+=bc+bline;						 

						}

						exceptinfo.bstrDescription=bdsc.Detach();

					}




				};



				RefCounterMT_blocked_t<1,false> refcounter;
				inline int Release()
				{
					return refcounter.Release(this);

				};
				inline int AddRef()
				{
					return refcounter.AddRef();

				};


				inline void enable_counter(bool f=true)  
				{
					refcounter.f=f;
				}








				HRESULT hr,hr_ex;
				//com_ptr_t<Site_t> site;
				exceptinfo_t exceptinfo;

				smart_ptr_t<IActiveScript> engine;
				smart_ptr_t<IActiveScriptParse> parser;
				smart_ptr_t<IActiveScriptError> error;
				smart_ptr_t<IDispatch> disp;
				smart_ptr_t<IDispatchEx> dispex;
				IDispatch* pweak_ref;

				smart_ptr_t<IActiveScriptGarbageCollector> GarbageCollector;
				//ArgsDispatch_t args;
				struct args_script_t:args_base_t<args_script_t,RefCounterFake_t<1> >
				{
					args_script_t():args_base_t(0,0,true){};

					IDispatch* get_Dispatch()	{

						return static_cast<IDispatch*>(this);
					};


				} args;

				bool ferrorbreak;

				/*
				static HRESULT CreateInstance(const wchar_t* lang,int fdebug,IDispatch**ppdisp,bool fwith_process=true)
				{
				if(!ppdisp) return E_POINTER;
				smart_ptr_t<IDispatch> engine;
				script_engine_t* pse=new script_engine_t(lang,fdebug,fwith_process);
				pse->enable_counter();
				create_dispatch_wrapper(&script_engine_t::global_eval,pse,&script_engine_t::s_on_exit,pse,&engine.p);
				*ppdisp=engine.detach();
				return S_OK;

				}

				static HRESULT CreateInstance(const wchar_t* lang,int fdebug,DWORD* pcookie,bool fwith_process=true)
				{
				HRESULT hr;
				if(!pcookie) return E_POINTER;
				smart_ptr_t<IDispatch> engine;
				OLE_CHECK_hr( CreateInstance(lang,fdebug,&engine.p,fwith_process));
				hr=GIT_t::get_instance().wrap(engine.p,pcookie);
				return hr;
				}
				*/

			};





			struct creator_t
			{
				wchar_t* lang;
				int fdebug;
				int with_process;
				IDispatch** pp_weak_ref;
				script_engine_t* pse;



				HRESULT operator()(DWORD* pcookie) 
				{
					HRESULT hr;
					smart_ptr_t<IDispatch> engine;
					pse=new script_engine_t(lang,fdebug,with_process);

					if(FAILED(hr=pse->hr)) {
						delete pse;
						return hr; 
					}
					pse->enable_counter();

					pp_weak_ref=&pse->pweak_ref;

					//create_dispatch_wrapper(&script_engine_t::global_eval,pse,&script_engine_t::s_on_exit,pse,&engine.p);
					//engine.reset(new calback_impl_t<>(&script_engine_t::global_eval,pse,&script_engine_t::s_on_exit),false);
					//return hr=ipc_utils::GIT_t::get_instance().wrap(engine.p,pcookie);
					return hr=ipc_utils::GIT_t::get_instance().wrap(new calback_impl_t<>(&script_engine_t::global_eval,pse,&script_engine_t::s_on_exit),pcookie,true);
				}


			}; 


			template <class Intf>
			inline  static   HRESULT create_apartment_script_engine(Intf** ppdisp ,IDispatch* exc_event=0,wchar_t* lang=L"JScript",int fdebug=3,bool fnosafe=0,int with_process=1,script_engine_t** ppse=0)
			{
				HRESULT hr;
				DWORD dw_engine=0,*pdw=&dw_engine;

				typedef apartment_t<COMInit_t,COMInitF_t> apartment_t;
				//typedef apartment_t<> apartment_t;



				creator_t creator={lang,fdebug,with_process};

				if(fnosafe)
				{  
					if(SUCCEEDED(hr=creator(&dw_engine)))
						OLE_CHECK_hr(ipc_utils::GIT_t::get_instance().unwrap(dw_engine,ppdisp,true));
					//*(creator.pp_weak_ref)=*ppdisp;
					InterlockedExchangePointer((void**)creator.pp_weak_ref,*ppdisp);
					if(ppse) *ppse=creator.pse;				  
					return hr;
				}




				apartment_t * apartment=new apartment_t(COINIT_APARTMENTTHREADED);		
				aggregator_container_helper  container(apartment,NULL,&apartment_t::on_exit);



				smart_ptr_t<IUnknown> git_engine;
				OLE_CHECK_hr(apartment->call_sync(creator,pdw,E_FAIL));
				OLE_CHECK_hr(create_GITDispatch_aggregator_helper(dw_engine,&git_engine.p));

				if(exc_event) container.add_EC(exc_event);
				else mta_singleton_t::add_external_connector(container);

				OLE_CHECK_hr(container.add_FMT().add(__uuidof(IDispatch),git_engine).QueryInterface(ppdisp));
				InterlockedExchangePointer((void**)creator.pp_weak_ref,*ppdisp);




				/*

				smart_ptr_t<iaggregator_container> container;

				smart_ptr_t<IUnknown> git_engine,free_marshal,external_connector;







				hr=create_iaggregator_container(&container.p,apartment,NULL,&apartment_t::on_exit);
				if(!container.p)  {
				delete apartment;
				return hr;
				};



				OLE_CHECK_hr(apartment->call_sync(creator,pdw,E_FAIL));

				SetThreadName(apartment->tid,bstr_t(L"DICH apartment:")+bstr_t(lang));


				//int at=COMInit_t::apartment_type();
				//smart_ptr_t<IDispatch>  disptest;
				//hr=ipc_utils::GIT_t::get_instance().unwrap(dw_engine,&disptest.p,0);

				OLE_CHECK_hr(create_GITDispatch_aggregator_helper(dw_engine,&git_engine.p));
				OLE_CHECK_hr(container->SetAggregator(__uuidof(IDispatch),git_engine));


				OLE_CHECK_hr(create_FTM_aggregator_helper(&free_marshal.p));
				OLE_CHECK_hr(container->SetAggregator(__uuidof(IMarshal),free_marshal));



				if(exc_event)
				{
				OLE_CHECK_hr(create_ExternalConnection_aggregator_helper(exc_event,&external_connector.p));
				OLE_CHECK_hr(container->SetAggregator(__uuidof(IExternalConnection),external_connector));

				}

				hr=container.QueryInterface(ppdisp);*/
				return hr;
			}




			struct creator2_t
			{

				struct param_t
				{
					int debug;
					wchar_t* lang;
					int fnosafe;
					int fprocess;
					wchar_t* import;
				};

				/*	wchar_t* lang;
				int fdebug;
				int with_process;
				*/
				param_t opts;
				IDispatch** pp_weak_ref;
				script_engine_t* pse;



				HRESULT operator()(DWORD* pcookie) 
				{
					HRESULT hr;
					smart_ptr_t<IDispatch> engine;
					pse=new script_engine_t(opts.lang,opts.debug,opts.fprocess);
					hr=pse->hr;



					if(SUCCEEDED(hr)&&opts.import)
					{
						variant_t v=opts.import;

						bstr_t sctext=js_text(
							if(0) debugger;
						var arg=arguments[0];			
						if(typeof(arg)=='string')  arg=arg.split(',');
						var l=arg.length;
						if(l)
						{
							for(var k=0;k<l;k++)
								require(arg[k],this);
						}
						);

						//			hr=pse->call(L"require(arguments[0],this)",0,1,&v);
						hr=pse->call(sctext,0,1,&v);
						//hr=pse->call(L"debugger;",0,1,&v);
					}


					if(FAILED(hr)) {
						delete pse;
						return hr; 
					}

					pse->enable_counter();
					pp_weak_ref=&pse->pweak_ref;

					//create_dispatch_wrapper(&script_engine_t::global_eval,pse,&script_engine_t::s_on_exit,pse,&engine.p);
					//engine.reset(new calback_impl_t<>(&script_engine_t::global_eval,pse,&script_engine_t::s_on_exit),false);
					//return hr=ipc_utils::GIT_t::get_instance().wrap(engine.p,pcookie);
					return hr=ipc_utils::GIT_t::get_instance().wrap(new calback_impl_t<>(&script_engine_t::global_eval,pse,&script_engine_t::s_on_exit),pcookie,true);
				}


			}; 

			template <class Intf>
			inline  static   HRESULT create_apartment_script_engine2(Intf** ppdisp,creator2_t::param_t& opts,IDispatch* exc_event=0,script_engine_t** ppse=0)
			{
				HRESULT hr;
				DWORD dw_engine=0,*pdw=&dw_engine;

				typedef apartment_t<COMInit_t,COMInitF_t> apartment_t;
				//typedef apartment_t<> apartment_t;



				creator2_t creator={};
				creator.opts=opts;


				if(opts.fnosafe)
				{  
					if(SUCCEEDED(hr=creator(&dw_engine)))
						OLE_CHECK_hr(ipc_utils::GIT_t::get_instance().unwrap(dw_engine,ppdisp,true));
					//*(creator.pp_weak_ref)=*ppdisp;
					InterlockedExchangePointer((void**)creator.pp_weak_ref,*ppdisp);
					if(ppse) *ppse=creator.pse;				  
					return hr;
				}




				apartment_t * apartment=new apartment_t(COINIT_APARTMENTTHREADED);		
				aggregator_container_helper  container(apartment,NULL,&apartment_t::on_exit);



				smart_ptr_t<IUnknown> git_engine;
				OLE_CHECK_hr(apartment->call_sync(creator,pdw,E_FAIL));
				OLE_CHECK_hr(create_GITDispatch_aggregator_helper(dw_engine,&git_engine.p));

				if(exc_event) container.add_EC(exc_event);
				else mta_singleton_t::add_external_connector(container);

				OLE_CHECK_hr(container.add_FMT().add(__uuidof(IDispatch),git_engine).QueryInterface(ppdisp));
				InterlockedExchangePointer((void**)creator.pp_weak_ref,*ppdisp);




				return hr;
			}







			//struct  __declspec(uuid("{61F8A798-2A81-4d98-B3C5-543E558B1468}")) CLSID_script_engine_factory_t{};            
			///*
			struct  __declspec(uuid("{61F8A798-2A81-4d98-B3C5-543E558B1468}"))  script_engine_factory_t
				:class_factory_base_aggregator_t<script_engine_factory_t>
			{

				/*struct param_t
				{
				int debug;
				wchar_t* lang;
				int fnosafe;
				int fprocess;
				wchar_t* import;
				};*/

				inline static HRESULT make_imports(IDispatch* disp,wchar_t* pimports)
				{


					HRESULT hr=S_OK;
					if( pimports)
					{
						wchar_t* sctext=js_text(
							if(0) debugger;
						var arg=arguments[0];			
						if(typeof(arg)=='string')  arg=arg.split(',');
						var l=arg.length;
						if(l)
						{
							for(var k=0;k<l;k++)
								require(arg[k],this);
						}
						);

						hr=disp_call_t(disp)(sctext,pimports);

					}
					return hr;

				}

				inline HRESULT impl_CreateInstanceEx(	IUnknown * pUnkOuter,LPOLESTR pszDisplayName,IStream* ,REFIID riid,void ** ppvObject,IBindCtx * pbc)	
				{
					HRESULT hr;
					//param_t dflt={0,L"JScript"};
					//param_t& params=(pparams)?*((param_t*)pparams):dflt;
					creator2_t::param_t params={0,L"JScript",0,1};
					wchar_t* plang;

					if((plang=_wgetenv(L"JScript_CLSID")))
						params.lang=plang;

					moniker_parser_t<wchar_t> mp(pszDisplayName,pbc);
					if(mp){
						int fforked=mp.args[L"__forked__"].def(0);
						if(!fforked)
						{


							int platform=mp.bind_args(L"platform").def(0);
							int fsrv=0;
							fsrv=mp.bind_args(L"isolated").def(fsrv);
							fsrv=mp.bind_args(L"srv").def(fsrv);
							if(fsrv&&(platform!=32)&&((platform!=64)))
								platform=8*sizeof(void*);							  

							if((platform)&&(fsrv||(platform!=(8*sizeof(void*)))))
							{
								if((platform!=32)&&((platform!=64)))
									return E_INVALIDARG;
								//wchar_t* ptail= mp._lskipi(pszDisplayName,L":",1,0);
								wchar_t* parsestring=mp.printf(L"ltx.bind:srv:%d:%s;__forked__=1;",platform,pszDisplayName);
								hr=ltxGetObjectEx(parsestring,riid,ppvObject,pbc);
								return hr;


							}
						}
						params.debug=mp.bind_args(L"debug").def(params.debug);
						params.lang=mp.undecorate(mp.bind_args(L"lang").def<bstr_t>(params.lang));
						params.fnosafe=mp.bind_args(L"appartment").def(params.fnosafe);
						params.fprocess=mp.bind_args(L"process").def(params.fprocess);
						params.import=mp.undecorate(mp.bind_args(L"imports").def<bstr_t>(params.import),L'[',L']');
					}


					wchar_t* pimports =ipc_utils::make_detach(params.import); 

					smart_ptr_t<IDispatch> disp;
					//if(SUCCEEDED(hr=create_apartment_script_engine(unk.address(),0,params.lang,params.debug,params.fnosafe,params.fprocess)))
					if(SUCCEEDED(hr=create_apartment_script_engine2(disp.address(),params)))
						if(SUCCEEDED(hr=make_imports(disp,pimports)))
							hr=disp->QueryInterface(riid,ppvObject);									  

					return hr;
					;}
				/*
				inline HRESULT impl_ParseDisplayName(IBindCtx * pbc,LPOLESTR pszDisplayName,ULONG * pchEaten,IUnknown ** ppUnk)	{

				HRESULT hr;

				param_t params={0,L"JScript"};



				moniker_parser_t<wchar_t> mp(pszDisplayName);
				if(mp){
				params.debug=mp.args[L"debug"].def(params.debug);
				params.lang=mp.args[L"lang"].def(params.lang);
				params.fnosafe=mp.args[L"appartment"].def(params.fnosafe);															  
				}





				return impl_CreateInstance(0,__uuidof(IUnknown),(void**)ppUnk,&params);
				}*/

				static HRESULT CreateFactory(REFIID riid, void** ppcf)
				{
					HRESULT hr;
					ipc_utils::smart_ptr_t<IUnknown> unk;
					hr=(new script_engine_factory_t)->GetFactory(&unk.p);
					if(SUCCEEDED(hr))
						hr=unk->QueryInterface(riid,ppcf);
					return hr;						  
				}


			};

			//*/



			inline VARIANT vnop(){ VARIANT v={VT_ERROR};v.scode=DISP_E_PARAMNOTFOUND;return v;  };


			HRESULT call_disp(IDispatch* disp,VARIANT* pres=0,int did=DISPID_VALUE,variant_t v0=vnop(),variant_t v1=vnop(),variant_t v2=vnop(),variant_t v3=vnop(),variant_t v4=vnop(),variant_t v5=vnop(),variant_t v6=vnop(),variant_t v7=vnop())
			{
				VARIANT vv[]={v7,v6,v5,v4,v3,v2,v1,v0};
				HRESULT hr;
				if(!disp) return E_POINTER;

				VARIANT *pvb=&vv[0], *pve=&vv[7],*pv,*pp=0;
				variant_t r;
				if(!pres) pres=&r;

				int cb=8;
				for(int k=0;k<8;k++)
				{
					if(vv[k].vt!=VT_ERROR) break;
					pvb++;
					cb--;
				}

				DISPPARAMS dps={pvb,0,cb,0};
				WORD wFlags=DISPATCH_METHOD;

				if(cb)
				{
					wFlags|=DISPATCH_PROPERTYGET;

				}

				hr=disp->Invoke(did, IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,pres,0,0);

				return hr;			   

			};

			HRESULT call_disp(IDispatch* disp,bstr_t sdid,VARIANT* pres=0,variant_t v0=vnop(),variant_t v1=vnop(),variant_t v2=vnop(),variant_t v3=vnop(),variant_t v4=vnop(),variant_t v5=vnop(),variant_t v6=vnop(),variant_t v7=vnop())
			{
				HRESULT hr;
				if(!disp) return E_POINTER;
				DISPID did=DISPID_VALUE;

				if(sdid.length())
				{				
					LPOLESTR lpsz=sdid;
					OLE_CHECK_hr(disp->GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&lpsz), 1, LOCALE_USER_DEFAULT,&did));

				}
				hr= call_disp(disp,pres,did,v0,v1,v2,v3,v4,v5,v6,v7);
				return hr;

			}


			inline	HRESULT script_execute(bstr_t text,VARIANT* presult=&VARIANT(),variant_t v0=variant_t(),variant_t v1=variant_t(),variant_t v2=variant_t(),variant_t v3=variant_t()
				,variant_t v4=variant_t(),variant_t v5=variant_t(),variant_t v6=variant_t(),variant_t v7=variant_t(),variant_t v8=variant_t())
			{
				VARIANT vv[]={v0,v1,v2,v3,v4,v5,v6,v7,v8};
				COMInit_t ci(0);
				script_engine_t engine(L"JScript",0);							          
				HRESULT hr;
				hr=engine.call(text,presult,9,vv);
				if(FAILED(hr)&&engine.error)
					hr=engine.exceptinfo.scode;
				return hr;
			}

			//struct			js_script{}
			struct js_t:invoker_base_t<js_t,variant_t,variant_t,VARIANT>{
				COMInitA_t ci;
				bstr_t script_text;
				script_engine_t engine;
				HRESULT hr;
				variant_t result;
				operator bool(){ return SUCCEEDED(hr);};

				inline HRESULT result2QueryInterface( REFIID riid, void** ppObj) {

					return SUCCEEDED(hr) ? VARIANT2QueryInterface(result, riid, ppObj) : hr;
				}



				inline HRESULT invoke_proc(int argc,VARIANT* argv){
					engine.set_args(argc,argv,true);  						   
					hr=engine._call(script_text,result.GetAddress());
					if(FAILED(hr)&&engine.error)
						hr=engine.exceptinfo.scode;
					return hr;
				} 

				js_t(bstr_t _script_text,int mdbg=0):script_text(_script_text),engine(L"JScript",mdbg){

				}

			};



		}; // script_engine

		typedef   script_engine::script_engine_t::process_com_t process_com_t;
	}; // com_apartments
}; //ipc_utils

