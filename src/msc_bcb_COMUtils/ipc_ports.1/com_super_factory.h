#pragma once
//#include "ipc_ports/com_super_factory.h"
#include "com_script_engine.h"
#include "reg_scripts.h"
#include "ltx_js.h"
#include "class_factory_factory.h"
#include "com_marshal_templ.h"
//#include "com_process.h" 
//#include "ltx_factories.h" 

namespace ipc_utils {
	namespace com_apartments
	{
		namespace script_engine
		{

			template <class T,bool _FTM=true,bool _EC=false>    
			struct  simple_factory_t
				:class_factory_base_aggregator_t<T>
			{
				typedef ltx_helper::arguments_t<variant_t> arguments_t;

				typedef typename aggregator_container_helper_FMT_EC<_FTM,_EC> aggregator_container_helper_t;


				inline HRESULT create_iaggregator(aggregator_container_helper_t& container,LPOLESTR pszDisplayName,IStream* ,GUID& uiid,smart_ptr_t<iaggregator_helper>& disp,IBindCtx * pbc )
				{
					return E_NOTIMPL;
				}

				inline HRESULT impl_CreateInstanceEx(	IUnknown * pUnkOuter,LPOLESTR pszDisplayName,IStream* stream,REFIID riid,void ** ppvObject,IBindCtx * pbc)	
				{
					HRESULT hr;					

					smart_ptr_t<iaggregator_helper> helper;//=aggregator_helper_creator_t<fs_disp_t>::New(new fs_disp_t);
					IID uuid=__uuidof(IDispatch);
					aggregator_container_helper_FMT_EC<_FTM,_EC> container;
					OLE_CHECK_hr(static_cast<T*>(this)->create_iaggregator(container,pszDisplayName,stream,uuid,helper,pbc));												
					hr=container.add(uuid,helper).QueryInterface(riid,ppvObject);						

					return hr;

				}


				static HRESULT CreateFactory(REFIID riid, void** ppcf)
				{
					HRESULT hr;
					ipc_utils::smart_ptr_t<IUnknown> unk;
					hr=(new T)->GetFactory(&unk.p);
					if(SUCCEEDED(hr))
						hr=unk->QueryInterface(riid,ppcf);
					return hr;						  
				}

			};




			struct super_factory_t:class_factory_factory_t<super_factory_t>
			{
				static HRESULT CreateFactory(REFIID riid,void** ppv){
					HRESULT hr=	(new super_factory_t)->GetFactory(riid,ppv);
					
					return hr;
				};

				static HRESULT CoGetObject(bstr_t parse_string,REFIID riid,void** ppv,IBindCtx* pbctx=0)
				{
					OLE_CHECK_PTR(ppv);					

					HRESULT hr;
					
					smart_ptr_t<IParseDisplayName> pdn;
					OLE_CHECK_hr(super_factory_t::CreateFactory(__uuidof(IParseDisplayName),pdn._ppQI()));
					smart_ptr_t<IMoniker> moniker;
					smart_ptr_t<IBindCtx> bctx;
					if(!pbctx){
					    OLE_CHECK_hr(CreateBindCtx(0,&bctx.p));
						pbctx=bctx.p;
					}
					ULONG ec=0;
					OLE_CHECK_hr(pdn->ParseDisplayName(pbctx,parse_string,&ec,&moniker.p));
					//smart_ptr_t<IUnknown> unk;
					OLE_CHECK_hr(moniker->BindToObject(pbctx,0,riid,ppv));
					return hr;
				}


				

			};

			struct  __declspec(uuid("{9FA7F2EE-B022-4b92-BF8C-91429E61B381}"))  binder_factory_t
				:simple_factory_t<binder_factory_t>					
			{





				struct binder_t:IDispatch_impl_t<binder_t>					
				{
					typedef _com_error com_exception;

					inline HRESULT findname(LPOLESTR name,DISPID& dispid){

						HRESULT hr;
						
						static  BEGIN_STRUCT_NAMES(pn)										
     							DISP_PAIR_IMPLC(bind,DISPID_VALUE)							
								DISP_PAIR_IMPLC(call,DISPID_VALUE)							
								END_STRUCT_NAMES;
						hr=find_id(pn,name,&dispid);
						return hr;
					}


					inline	HRESULT impl_Invoke(DISPID id,REFIID riid,LCID,WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi, UINT*){

						HRESULT hr=E_UNEXPECTED;
						OLE_CHECK_PTR(res);
						if(!((id==0)&&(dp->cArgs)&&(is_get(flags)))) return hr=E_NOTIMPL;
						try
						{
							arguments_t arguments(dp);
							bstr_t ps=arguments[0];
							if(!ps.length()) return E_INVALIDARG;
							wchar_t* p=ps;
							if((p[0]!=L':')&&(!moniker_parser_t<wchar_t>::_lcheckni(p,L"ltx.bind:"))) ps=bstr_t(L":")+p;
							//OLE_CHECK_hr(owner->impl_ParseDisplayName())

							smart_ptr_t<IUnknown> unk;
							//
							//OLE_CHECK_hr(super_factory_t::CoGetObject(ps,__uuidof(IUnknown),unk._ppQI()));
							 args_stack_t args(dp);
							 smart_ptr_t<IBindCtx> bctx;
							 if(dp->cArgs>1)
							 {
							 			 OLE_CHECK_hr(CreateBindCtx(0,&bctx.p));
										 ipc_utils::bstr_c_t<64> bstrc( LTX_BINDCONTEXT_PARAMS);
										 bstr_t bb=(LTX_BINDCONTEXT_PARAMS);
										 wchar_t* p=bb;
										 int* pcb=(int*)(((char*)p)-4);

										 //hr=bctx->RegisterObjectParam(bstr_t(LTX_BINDCONTEXT_PARAMS),&args);
										 hr=bctx->RegisterObjectParam(bstrc,&args);
							 }
							 


							if( FAILED( hr=super_factory_t::CoGetObject(ps,__uuidof(IUnknown),unk._ppQI(),bctx.p) ) )
							{
								OLE_CHECK_hr(::CoGetObject(ps,0,__uuidof(IUnknown),unk._ppQI()));

							}



							



							//
							/*

							smart_ptr_t<IParseDisplayName> pdn;
							//OLE_CHECK_hr(QueryInterface(__uuidof(IParseDisplayName),pdn._ppQI()));

							OLE_CHECK_hr(super_factory_t::CreateFactory(__uuidof(IParseDisplayName),pdn._ppQI()));

							smart_ptr_t<IMoniker> moniker;
							smart_ptr_t<IBindCtx> bctx;
							OLE_CHECK_hr(CreateBindCtx(0,&bctx.p));
							ULONG ec;
							OLE_CHECK_hr(pdn->ParseDisplayName(bctx,ps,&ec,&moniker.p));
							
							OLE_CHECK_hr(moniker->BindToObject(bctx,0,__uuidof(IUnknown),unk._ppQI()));
							//*/
							hr=toVARIANT(unk.p,res);							



						}
						catch (com_exception& e){			
							hr=e.Error();
						}
						catch (...){		
						}
						return hr;

					}

					binder_t()
					{

					}

				};

				 binder_factory_t()
				 {

				 }


				inline HRESULT create_iaggregator(aggregator_container_helper_t& container,LPOLESTR pszDisplayName,IStream* ,GUID& uiid,smart_ptr_t<iaggregator_helper>& disp ,IBindCtx * pbc)
				{

					disp=aggregator_helper_creator_t<binder_t>::New(new binder_t);
					mta_singleton_t::add_external_connector(container);
					return (disp)?S_OK:E_OUTOFMEMORY;

				}



			};


		}//script_engine
	};//com_apartments
};//ipc_utils
