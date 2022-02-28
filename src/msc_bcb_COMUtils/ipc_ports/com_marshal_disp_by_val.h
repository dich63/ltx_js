#pragma once
#include "com_marshal_templ.h"
#include "ltx_factories.h"


namespace ipc_utils {
	namespace com_apartments
	{
		namespace script_engine
		{
			//		template <class T ,class I,class RefCounter=RefCounterMT_t<1> >
			//		struct IUnknown_base_impl:I

			//template <int _Vers=0>  


			template <int _Vers = 0>
			struct  mbv_callback_class_factory_t :callback_class_factory_base_t<mbv_callback_class_factory_t<_Vers>>
			{

				struct   MBV_t :IUnknown_this_t<MBV_t>
				{
					MarshalCallbackData_t mcd;
					MBV_t(MarshalCallbackData_t& _mcd) {
						mcd = _mcd;
					}
				};


				struct IDispatchMBV_t
					:IDispatch_impl_t<IDispatchMBV_t >
					, base_callsback_marshal_t<IDispatchMBV_t, IDispatch, creator_class_base_t >
				{

					typedef IDispatchMBV_t owner_t;

					struct marshal_helper_t :i_marshal_helper_t
					{

						virtual  HRESULT marshal_hko(IStream* pStm, HANDLE hko = 0) {
							HRESULT hr;
							return hr = owner_t::marshal_hko_to_process(hko, pid, pStm);
						};
						virtual  HRESULT marshal_interface(IStream* pStm, REFIID riid, IUnknown* pObj) {
							HRESULT hr;
							return hr = CoMarshalInterface(pStm, riid, pObj, MSHCTX_LOCAL, 0, MSHLFLAGS_NORMAL);
						};
						virtual  HRESULT unmarshal_hko(IStream* pStm, HANDLE* phko = 0) {
							HRESULT hr;
							return hr = owner_t::unmarshal_hko(pStm, phko);

						};
						virtual HRESULT unmarshal_and_release(IStream *pStm, REFIID riid, void** ppObj) {
							HRESULT hr;
							return hr = base_marshal_utils_t<>::unmarshal_and_release(pStm, riid, ppObj);
						};
						virtual HRESULT marshal_BSTR(BSTR bstr, IStream* pStm)
						{
							HRESULT hr;
							return hr = owner_t::marshal_BSTR(bstr, pStm);
						}

						virtual HRESULT unmarshal_BSTR(IStream* pStm, BSTR* pbstr)
						{
							HRESULT hr;
							return hr = owner_t::unmarshal_BSTR(pStm, pbstr);
						}

						marshal_helper_t(DWORD _pid = GetCurrentProcessId()) :pid(_pid) {};
						DWORD pid;
					};


					inline HRESULT GetClassID(CLSID* pclsid)
					{
						HRESULT hr;
						return hr = mcd->GetClassID(pclsid);
					}


					inline HRESULT marshal_to_process(DWORD pid_target, IStream* pStm)
					{

						HRESULT hr;
						if (!&(mcd->MarshalToProcess)) return S_FALSE;

						marshal_helper_t helper(pid_target);
						return hr = mcd->MarshalToProcess(context, &helper, pid_target, pStm);
					};

					inline HRESULT unmarshal(IStream* pStm)
					{
						HRESULT hr;
						if (!&(mcd->Unmarshal)) return S_FALSE;
						marshal_helper_t helper;
						return hr = mcd->Unmarshal(context, &helper, pStm);
					};



					inline static HRESULT CreateInstance(IUnknown* punkMD, IBindCtx* bctx, LPOLESTR pszDisplayName, REFIID riid, void ** ppvObject, DWORD mcxmask = 0)
					{
						HRESULT hr;

						//return (new IDispatch_test2_t)->getInstance(riid,ppvObject,0,mcxmask,FMT_enable);
						//if(FMT_enable) mcxmask=MAKELONG(mcxmask,1);
						OLE_CHECK_PTR(ppvObject);

						moniker_parser_t<wchar_t> parser(pszDisplayName);

						int m = parser.args[L"__unmarshal__"];

						local_ptr_t<IDispatchMBV_t> mbv = new IDispatchMBV_t(punkMD);
						OLE_CHECK_hr(mbv->init(bctx, pszDisplayName, m, &mcxmask));

						bool f_addDispatch = mbv->mcd->InvokeByName;

						OLE_CHECK_hr(creator_t::CreateInstance(mbv.p, pszDisplayName, riid, ppvObject, mcxmask, f_addDispatch));

						if (mbv->mcd->SetAggregatorHelpers)
						{
							smart_ptr_t<iaggregator_container> ac;
							//

							OLE_CHECK_hr(static_cast<IUnknown*>(*ppvObject)->QueryInterface(__uuidof(iaggregator_container), ac._ppQI()));
							void*p;				HRESULT hhr = ac->GetContext(&(p = 0));
							callback_lib_arguments_t cla = { &calback_aggregator_t<IDispatch>::CreateInstanceV,&ltxGetObject,&bindObjectWithArgs };
							OLE_CHECK_hr(mbv->mcd->SetAggregatorHelpers(mbv->context, ac, &cla));
						}


						//HRESULT (*SetAggregatorHelpers)(void *context,IUnknown* aggregator_container)
						//IDispatchMBV_t*  

						//if(FAILED(hr=mbv->init(bctx,pszDisplayName,m,&mcxmask)))
							//delete mbv;
						//else   OLE_CHECK_hr(creator_t::CreateInstance(mbv,pszDisplayName,riid,ppvObject,mcxmask));




						mbv.detach();
						return hr;

					}

					
					inline bool _chk_js_cnsr_proto(const wchar_t* name, int len) {

						const wchar_t cr[] = L"constructor";
						const wchar_t pr[] = L"prototype";

						if ((len == sizeof(cr) - 2) && (wcscmp(name, cr)==0)) {
							return true;
						}
						else if ((len == sizeof(pr) - 2) && (wcscmp(name, pr)==0)) {
							return true;
						}
						return false;
					}

					inline HRESULT findname(LPOLESTR name, DISPID& dispid) {
						int l = 2 * safe_len(name);

						if (_chk_js_cnsr_proto(name, l)) {
							return DISP_E_UNKNOWNNAME;
						}
													
						tls_com_t<>::get_instance().set_buffer(l, name);
						
						dispid = callback_context_arguments_t::disp_id_t::TSSid;
						return S_OK;
					}

					static void s_call_disp(VARIANT* presult, IDispatchMBV_t* _this, pcallback_context_arguments_t pcca, int argc, VARIANT* argv, int arglast, HRESULT& hr, void*, unsigned flags)
					{

						marshal_helper_t helper;
						_this->mcd->InvokeByName(_this->context, pcca, &helper);

					}

					inline	HRESULT impl_Invoke(DISPID did, REFIID, LCID, WORD flags, DISPPARAMS* dp, VARIANT*res, EXCEPINFO* pexi, UINT*) {

						//						return S_OK;

						safe_caller_t<>::V_C_t tmp;

						if (!res) res = &tmp;


						HRESULT hr;
						int c = dp->cArgs;
						VARIANT* pdv = dp->rgvarg;
						if (is_put(flags))
						{
							bool f = (dp->cNamedArgs == 1) && (dp->rgdispidNamedArgs[0] == DISPID_PROPERTYPUT);
							if (!f)
								return E_NOTIMPL;
							--c;
							res = pdv++;
						}

						void* ptr = this;
						hr = safe_caller_t<>::call_prop(&s_call_disp, ptr, flags, c, pdv, res, pexi, did, this);
						return hr;
					}


					IDispatchMBV_t(IUnknown* punkMD) :unkMD(punkMD), mcd(0), context(0) {};
					HRESULT init(IBindCtx* bctx, LPOLESTR pszDisplayName, int funmarshal, DWORD* pflags)
					{
						HRESULT(*CreateContext)(BOOL fUnmarshal, IBindCtx* bctx, LPOLESTR pszDisplayName, DWORD* pflags, void ** ppcontext);
						HRESULT hr;
						smart_ptr_t<IUnknown> t = unkMD;
						MBV_t* pm = 0;
						OLE_CHECK_hr(unkMD.QueryInterface(IID_NULL, (void**)&pm));
						;
						OLE_CHECK_PTR(CreateContext = pm->mcd.CreateContext);
						OLE_CHECK_hr(CreateContext(funmarshal, bctx, pszDisplayName, pflags, &context));
						mcd = &pm->mcd;
						return hr;
					}

					~IDispatchMBV_t() {
						if (mcd && (&mcd->OnExit)) mcd->OnExit(context);
					}

					smart_ptr_t<IUnknown> unkMD;
					MarshalCallbackData_t* mcd;
					void* context;


				};








				inline HRESULT impl_SetMarshalContext(IUnknown * pContext)
				{
					marshal_context.reset(pContext);
					return S_OK;
				}



				inline HRESULT impl_CreateInstanceEx(IUnknown * pUnkOuter, LPOLESTR pszDisplayName, IStream* stream, REFIID riid, void ** ppvObject, IBindCtx* pbc)
				{
					HRESULT hr;
					OLE_CHECK_PTR(ppvObject);
					OLE_CHECK_hr(register_class_map_t<>::GetClassObject(pszDisplayName, IID_NULL, 0, &marshal_context.p));

					/*					smart_ptr_t<IUnknown> unk;
										OLE_CHECK_hr(IDispatchMBV_t::CreateInstance(marshal_context,pszDisplayName,__uuidof(IUnknown),unk._ppQI(),0));
										if(stream)
										{
											smart_ptr_t<IPersistStream> ps;
											OLE_CHECK_hr(unk.QueryInterface(ps._ppQI()));


										}

										return hr=unk.QueryInterface(riid,ppvObject);
					  */





					if (!stream) return hr = IDispatchMBV_t::CreateInstance(marshal_context, pbc, pszDisplayName, riid, ppvObject, 0);
					smart_ptr_t<IDispatch> callback;
					smart_ptr_t<IStream> callback_stream;

					LARGE_INTEGER pos, pos2;


					//co_releaser_t  co_releaser(stream);
					OLE_CHECK_hr(CoUnmarshalInterface(stream, __uuidof(IDispatch), callback._ppQI()));

					OLE_CHECK_hr(invoke_callback(callback, &callback_stream.p));

					smart_ptr_t<IPersistStream> ps;

					OLE_CHECK_hr(IDispatchMBV_t::CreateInstance(marshal_context, pbc, pszDisplayName, __uuidof(IPersistStream), ps._ppQI(), 0));
					OLE_CHECK_hr(ps->Load(callback_stream));
					hr = ps->QueryInterface(riid, ppvObject);
					return hr;


				}

				inline HRESULT GetClassID(CLSID* clsid)
				{
					return T::creator_t::GetClassID(&clsid);
				}


				inline static CLSID GetCLSID() {
					//return T::creator_t::get_clsid();
					CLSID clsid = {};
					T::creator_t::GetClassID(&clsid);
					return clsid;
				};

				smart_ptr_t<IUnknown> marshal_context;

			};





			struct MarshalByValueDispatchRegister_impl_t :IUnknown_base_impl<MarshalByValueDispatchRegister_impl_t, IMarshalByValueDispatchRegister>
			{
				virtual HRESULT STDMETHODCALLTYPE Register(LPOLESTR alias, MarshalCallbackData_t* data)
				{
					HRESULT hr;
					CLSID clsid;
					OLE_CHECK_PTR((data && (&data->GetClassID)));
					OLE_CHECK_hr(data->GetClassID(&clsid));
					smart_ptr_t<IUnknown> marshal_context(new mbv_callback_class_factory_t<>::MBV_t(*data), false), unk;
					class_object_pair_t* cop = 0;

					//hr=register_class_map_t<>::get_instance().register_class( CLSID clsid,GetClassObject_t pClassObject,IUnknown** ppUnk,class_object_pair_t** pp_pair=0,IUnknown* marshal_context=0)
					OLE_CHECK_hr(ltx_register_class(clsid, &mbv_callback_class_factory_t<>::GetClassObject, alias, 0, 0));
					OLE_CHECK_hr(ltx_register_class(clsid, &mbv_callback_class_factory_t<>::GetClassObject, 0, 0, 0));
					OLE_CHECK_hr(register_class_map_t<>::get_instance().register_marshal_context(clsid, marshal_context));




					//smart_ptr_t<IUnknown> unk(new MBV_t(*data),false),unk_old(cop->pmarshal_context,false);
					//cop->pmarshal_context=unk.detach();
						//,class_object_pair_t::GetClassObject_t pClassObject,const wchar_t* alias=0,IUnknown** ppUnk=0,class_object_pair_t** pp_pair=0);
					return hr;
				};
			};
			/*
			template<class T>
			inline HRESULT reset_to_QueryInterface(T* ptr,REFIID riid,void** ppObject)
			{

			}

			*/
			struct __declspec(uuid("{038F4D99-ED67-4a5f-BF96-C700C9641BF0}")) mbv_dispatch_register_t :simple_factory_t<mbv_dispatch_register_t>
			{

				inline HRESULT impl_CreateInstanceEx(IUnknown * pUnkOuter, LPOLESTR pszDisplayName, IStream* stream, REFIID riid, void ** ppvObject, IBindCtx * pbc)
				{
					HRESULT hr;
					smart_ptr_t<MarshalByValueDispatchRegister_impl_t> pmbv(new MarshalByValueDispatchRegister_impl_t, 0);
					return hr = pmbv.QueryInterface(riid, ppvObject);
				};
			};

		}//script_engine
	};//com_apartments
};//ipc_utils

