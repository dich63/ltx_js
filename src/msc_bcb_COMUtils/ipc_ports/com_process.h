#pragma once
//com_process.h
#include "com_script_engine.h"
#include "ltx_call_helper.h"
#include "ltx_factories.h" 
#include "com_global.h" 
#include "net_com_interop_ltx.h"
//#include <VersionHelpers.h>

//#include "os_utils.h"

#define LTX_PROCESS_OBJREF L"ltx.{5097F904-F18E-4099-99E0-F8C194349E06}"
#define MONIKER_EXTERNAL_OBJECT_OLD L"DICH_EXTERNAL_OBJECT_REF_MONIKER"

namespace ipc_utils {


	namespace com_apartments
	{

		struct handle_t {

			handle_t() :h(INVALID_HANDLE_VALUE) {

			}



			inline void close() {

				if (check())
					CloseHandle(h);
				h = INVALID_HANDLE_VALUE;
			}


			inline HANDLE reset(HANDLE hsrc, bool finher = false) {
				HANDLE htp = GetCurrentProcess();
				bool f;
				close();
				f = DuplicateHandle(htp, hsrc, htp, &h, 0, finher, DUPLICATE_SAME_ACCESS);
				return h;
			}

			~handle_t() {
				if (check())
					CloseHandle(h);
			}
			inline bool check() {
				return !((h == INVALID_HANDLE_VALUE) || (h == NULL));
			}
			inline operator bool() {
				return check();
			}
			inline operator HANDLE() {
				return h;
			}
			HANDLE h;

		};


		namespace script_engine
		{


			//com_process_t



			//	template <class T ,class IDispatchX=IDispatch,class RefCounter=RefCounterMT_t<1> >
			//	struct IDispatch_base_t:IDispatch_impl_base_t<T,IUnknown_base_impl<T,IDispatchX,RefCounter > >{};


			struct utils_creator_t
			{


			};






			struct __declspec(uuid("{68386F39-877D-4083-B727-F6843A239568}")) utils_factory_t :simple_factory_t<utils_factory_t>
			{


				inline HRESULT impl_CreateInstanceEx(IUnknown * pUnkOuter, LPOLESTR pszDisplayName, IStream* stream, REFIID riid, void ** ppvObject, IBindCtx* pbc)
				{
					HRESULT hr = E_NOINTERFACE;
					bstr_t tmp;
					wchar_t* p = tmp = (wchar_t*)pszDisplayName, *ptail;
					if (moniker_parser_t<wchar_t>::_lcheckni(p, L"external", 6, &ptail))
					{
						//
						/*
						wchar_t buf[1024];
						if(GetEnvironmentVariableW(LTX_PROCESS_OBJREF,buf,1024))
						//return hr=CoGetObject(bstr_t(buf),0,riid,ppvObject);
						return hr = super_factory_t::CoGetObject(bstr_t(buf), riid, ppvObject,pbc);

						///*/
						moniker_parser_t<wchar_t> mp;
						return hr = ::CoGetObject(bstr_t((wchar_t*)mp.getenv(LTX_PROCESS_OBJREF)), 0, riid, ppvObject);
					}
					else
						if (moniker_parser_t<wchar_t>::_lcheckni(p, L"srv", 3, &ptail) || moniker_parser_t<wchar_t>::_lcheckni(p, L"lpc", 3, &ptail))
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
							wchar_t * srvopts = L"", *p;
							if (ptail && (ptail[0] == L':') && (ptail[1] == L'[') && (p = mp.between(ptail, L'[', L']', &ptail)))
								srvopts = p;

							wchar_t sysdir[256] = L"";
							GetSystemDirectoryW(sysdir, 256);



							wchar_t* mn = mp.get_version_module_path(ptail, &ptail);

							if (mp.wow_index == 0) GetSystemWow64DirectoryW(sysdir, 256);

							wchar_t* Os[2] = { L"win32",L"x64" };
							const int osx = sizeof(void*) / 8;

							int ihwndc = (int)process_com_t::get_instance().console_hwin();


							bstr_t bindstr = mp.printf(L"ltx.bind:process:%s;flags.assoc=0;file=%s/rundll32.exe;cmd=\"%s\",runsrv console.hwnd=%d parent.platform=%s %s"
								, srvopts, sysdir, (wchar_t*)mn, ihwndc, Os[osx], srvopts);

							smart_ptr_t<IDispatch> process;
							//OLE_CHECK_hr(CoGetObject(bindstr,0,__uuidof(IDispatch),process._ppQI()));
							OLE_CHECK_hr(super_factory_t::CoGetObject(bindstr, __uuidof(IDispatch), process._ppQI(), pbc));
							variant_t res, res2, extref;
							//
							/*
							{
								wchar_t buf[1024];
								if (GetEnvironmentVariableW(LTX_PROCESS_OBJREF, buf, 1024));
								extref = buf;

							}
							//*/
							//variant_t extref=_wgetenv(LTX_PROCESS_OBJREF);
							//OLE_CHECK_hr(call_disp(process,&res,int(0)));
							//OLE_CHECK_hr(call_disp(process,&res,int(0),variant_t(_wgetenv(LTX_PROCESS_OBJREF))));
							//
							OLE_CHECK_hr(call_disp(process, &res, int(0), (wchar_t*)mp.getenv(LTX_PROCESS_OBJREF)));

							//OLE_CHECK_hr(call_disp(process, &res, int(0),extref));




							if ((res.vt == VT_DISPATCH) || (res.vt == VT_UNKNOWN))
							{
								//ComClipboard()=res.punkVal;
								if ((safe_len(ptail = moniker_parser_t<wchar_t>::_ltrim(ptail)) > 1) && (*ptail == L':'))
								{
									smart_ptr_t<IDispatch> binder;
									if (res.vt == VT_DISPATCH) binder.reset(res.pdispVal);
									else OLE_CHECK_hr(res.punkVal->QueryInterface(__uuidof(IDispatch), binder._ppQI()));

									VARIANT vbcxt = { (pbc) ? VT_UNKNOWN : VT_NULL };
									vbcxt.punkVal = pbc;

									//	OLE_CHECK_hr(call_disp(binder,res.GetAddress(),int( binder_factory_t::bindexid),variant_t(ptail)));
									//
									disp_call_t bindex(binder, binder_factory_t::bindexid);
									//
									OLE_CHECK_hr(bindex(variant_t(ptail), vbcxt));
									//OLE_CHECK_hr(bindex(variant_t(ptail)));

									//
									res.Attach(bindex.result.Detach());



									if ((res.vt != VT_DISPATCH) && (res.vt != VT_UNKNOWN))
										return hr = OLE_E_CANT_GETMONIKER;


								}

								OLE_CHECK_hr(res.punkVal->QueryInterface(riid, ppvObject));
								//ComClipboard()=(IUnknown*)*ppvObject;


								return hr;
							}
							else 	return hr = E_UNEXPECTED;

							//rundll32.exe 


						}
						else
							if (moniker_parser_t<wchar_t>::_lcheckni(p, L"clipbrd", 6, &ptail))
							{
								return 	hr = CoGetObject(bstr_t(win_ver_6_t<>::ClipboardText()), 0, riid, ppvObject);
							}
							else if (moniker_parser_t<wchar_t>::_lcheckni(p, L"old_style.external", -1, &ptail))
							{
								moniker_parser_t<wchar_t> mn(ptail);

								int fdebug = mn.args[L"debug"].def<int>(0);
								//int fenviron=mn.args[L"env"].def<int>(0);

								int cb;
								wchar_t* pbuf = mn.get_cache(1024);
								if (cb = GetEnvironmentVariableW(MONIKER_EXTERNAL_OBJECT_OLD, pbuf, 1024))
								{
									OLE_CHECK_hr(CoGetObject(bstr_t(pbuf), 0, riid, ppvObject));
								}
								else {


									if (fdebug & 1)
									{
										wchar_t* hookfn = mn.get_cache().cat((wchar_t*)module_t<>::module_name((void*)0)).cat(L".LifeTimeX_hook");
										buf_fm_T<HOOK_PROCESS_INFO, OPEN_EXISTING>  hook_buf(hookfn);
										if (hook_buf&&moniker_parser_t<wchar_t>::_spin(&hook_buf->fready, 100, 100)) {
											wchar_t* pobjref = hook_buf->objref;
											if (SUCCEEDED(hr = CoGetObject(bstr_t(pobjref), 0, riid, ppvObject)))
											{
												SetEnvironmentVariableW(MONIKER_EXTERNAL_OBJECT_OLD, pobjref);
												return hr;
											}
										}
									}
									if (fdebug & 2)
									{
										bstr_t  bcstr = win_ver_6_t<>::ClipboardText();
										if (SUCCEEDED(hr = CoGetObject(bcstr, 0, riid, ppvObject)))
										{
											SetEnvironmentVariableW(MONIKER_EXTERNAL_OBJECT_OLD, bcstr);
											return hr;
										}

									}


									return hr = E_NOINTERFACE;

								}



							}
							else if (moniker_parser_t<wchar_t>::_lcheckni(p, L"progid:", 7, &ptail))
							{
								clsid_t clsid(ptail);
								OLE_CHECK_hr(clsid.hr);
								return hr = ::CoCreateInstance((CLSID)clsid, 0, CLSCTX_ALL, riid, ppvObject);

							}
							else if (moniker_parser_t<wchar_t>::_lcheckni(p, L"wsh:", 4, &ptail))
							{
								bstr_t  bcstr = bstr_t(L"script:") + bstr_t(ptail);
								OLE_CHECK_hr(CoGetObject(bcstr, 0, riid, ppvObject));
							}
							else if (moniker_parser_t<wchar_t>::_lcheckni(p, L".net:", 5, &ptail) || moniker_parser_t<wchar_t>::_lcheckni(p, L"dot-net:", 8, &ptail))
							{
								moniker_parser_t<wchar_t> mp(p, pbc);
								AppDomainHolder2 appdomain;
								VARIANT v = { VT_EMPTY };
								bstr_t fn, tn, n;
								tn = mp.bind_args(L"class").def<bstr_t>();
								if (tn.length() == 0)
									tn = mp.bind_args(L"type").def<bstr_t>();

								n = mp.bind_args(L"name").def<bstr_t>();
								fn = mp.bind_args(L"file").def<bstr_t>();

								n = mp.undecorateQQ(mp.trim(n));
								fn = mp.undecorateQQ(mp.trim(fn));
								tn = mp.undecorateQQ(mp.trim(tn));


								if (tn.length() && (fn.length() || n.length()))
								{
									if (fn.length()) {
										OLE_CHECK_hr(appdomain.CreateComponentFrom(fn, tn, &v));
									}
									else {
										OLE_CHECK_hr(appdomain.CreateComponent(n, tn, &v));
									}

									*ppvObject = v.byref;

								}
								else hr = E_INVALIDARG;
								return hr;

							}
							else if (moniker_parser_t<wchar_t>::_lcheckni(p, L"js.factory:", 11, &ptail)) {

								moniker_parser_t<wchar_t> mp(NULL, pbc);
								variant_t args;
								if (mp.bind_disp.p) {
									args = mp.bind_disp.p;
								}
								js_t js(ptail);
								js(args);
								hr = js.result2QueryInterface(riid, ppvObject);
								return hr;
							}
							else if (moniker_parser_t<wchar_t>::_lcheckni(p, L"global:", 7, &ptail)) {

								/*
								  moniker_parser_t<wchar_t> mp(p, pbc);

								  bstr_t b;
								  b = mp.bind_args(L"").def<bstr_t>(L"");
								  b = mp.bind_args(L"name").def<bstr_t>(b);

								  wchar_t* pname = mp.undecorateQQ(mp.expandenv((wchar_t*)b));
								*/
								moniker_parser_t<wchar_t> mp;
								wchar_t* pname = mp.undecorateQQ(mp.expandenv((wchar_t*)ptail));

								wchar_t* bindstr = mp.printf(L"ltx.bind:js.factory:require('register').bindGlobal('%s')", pname);
								hr = super_factory_t::CoGetObject(bindstr, riid, ppvObject);
								return hr;

							}
							else if (moniker_parser_t<wchar_t>::_lcheckni(p, L"host.script:", 11, &ptail)) {

								hr = super_factory_t::CoGetObject(L"ltx.bind:js.factory:require('register').global_vm()", riid, ppvObject);
								return hr;

							}
							else
							if (moniker_parser_t<wchar_t>::_lcheckni(p, L"ref:", 4, &ptail)) {

								ipc_utils::smart_ptr_t<IDispatch> ref;
								ipc_utils::smart_ptr_t<IUnknown> unk;
								variant_t res;
								bstr_t buf;
								if (moniker_parser_t<wchar_t>::_lcheckni(ptail, L"ref:", 4)) {									
									ptail =(wchar_t*)(buf = bstr_t(L"ltx.bind:") + bstr_t(ptail));
								}

								//OLE_CHECK_hr( ref.CoGetObject(ptail));
								OLE_CHECK_hr(::ltxGetObjectEx(ptail, __uuidof(IDispatch),ref._ppQI(), pbc));
								OLE_CHECK_hr(call_disp(ref, &res));	
								OLE_CHECK_hr(unk.reset(res));

								return hr = unk.QueryInterface(riid, ppvObject);
							}
							
					//OLE_CHECK_hr(super_factory_t::CoGetObject(bindstr, __uuidof(IDispatch), process._ppQI(), pbc));





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
				stub_holder_t() :hstub(0), hr(E_POINTER) {};

				stub_holder_t(IUnknown* unk) :hstub(0), hr(E_POINTER)
				{
					if (unk)
						reset(unk);
				}
				stub_holder_t(const wchar_t* base64_str) :hstub(0), hr(E_POINTER)
				{

					reset(base64_str);
				}

				stub_holder_t& reset(const wchar_t* base64_str)
				{
					reset();
					hr = (hstub = moniker_parser_t<wchar_t>::base64_decode(moniker_parser_t<wchar_t>::_trim((wchar_t*)base64_str))) ? S_OK : E_INVALIDARG;
					return *this;
				}

				stub_holder_t& detach_to(stub_holder_t& sh) {

					sh.hstub = make_detach(this->hstub);
					sh.hr = make_detach(this->hr, E_POINTER);

					return *this;
				}

				~stub_holder_t() {
					clear();
					ipc_utils::DBG_PRINT("stub_holder_t DESTROY");
				};

				stub_holder_t& clear() {

					if (hstub) reset();
					return *this;
				};

				stub_holder_t& reset(IUnknown* unk = 0)
				{
					HGLOBAL t = make_detach(hstub);
					if (unk)
						hr = ipc_utils::CoMarshalInterfaceToHGlobal(unk, &hstub);
					if (t) ipc_utils::CoReleaseCOMStub(t, true);
					return *this;
				}

				inline HRESULT detachInterface() {

					if (hstub && ((hstub = GlobalFree(hstub)) != NULL))
						return HRESULT_FROM_WIN32(GetLastError());
					else
						return S_OK;
				}

				inline operator bool()
				{
					return SUCCEEDED(hr);
				}
				inline bstr_t objref(bool fdetach = false)
				{
					bstr_t r;
					if (SUCCEEDED(hr))
					{
						r = moniker_parser_t<wchar_t>::base64_encode(hstub, true);
						if ((fdetach) && (FAILED(hr = detachInterface())))
							return bstr_t();
					}
					return r;

				}

				inline HRESULT unwrap(REFIID iid, void**ppObj)
				{
					HRESULT hr0;
					OLE_CHECK_hr(hr);
					hr0 = ipc_utils::CoUnmarshalInterfaceFromHGlobal(hstub, iid, ppObj);
					return hr0;

				}

				HGLOBAL hstub;
				HRESULT hr;
			};


			
			template<class Parent>
			struct process_args_t :args_base_t<process_args_t<Parent>, RefCounterMT_t<1>, mutex_cs_t >
			{
				typedef _com_error com_exception;
				enum {
					id_result = -8,
					id_wait = -9,
					id_pid = -10,
					id_parent = -11,
					id_decoder = -12
				};




				inline HRESULT findname(LPOLESTR name, DISPID& dispid) {
					HRESULT hr;
					int i;

					static  BEGIN_STRUCT_NAMES(pn)

						DISP_PAIR_IMPLC_LEN(result, id_result, 3)
						DISP_PAIR_IMPLC_LEN(wait, id_result, 4)
						DISP_PAIR_IMPLC(pid, id_pid)
						DISP_PAIR_IMPLC(parent, id_parent)
						DISP_PAIR_IMPLC(fjson, id_decoder)
						END_STRUCT_NAMES;
					if (SUCCEEDED(hr = find_id(pn, name, &dispid)))
						return hr;
					return hr = __super::findname(name, dispid);
				}


				inline	HRESULT impl_Result(WORD flags, DISPPARAMS*dp, VARIANT*res, EXCEPINFO*pexi)
				{
					HRESULT hr;
					try
					{
						{
							locker_t<mutex_t> lock(mutex);
							if (dp->cArgs != 1) return E_INVALIDARG;
							//hr = 0;
							if (process_disp->result_decoder) {
								smart_ptr_t<IDispatch> decoder;
								if (SUCCEEDED(hr = process_disp->result_decoder.unwrap(decoder))) {
									DISPPARAMS dps = { dp->rgvarg,0,1,0 };
									hr = decoder->Invoke(0, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &dps, result.GetAddress(), 0, 0);
								}
							}
							else {
								hr = VariantCopyInd(result.GetAddress(), dp->rgvarg);
							}

						}

						hevent.signal();

					}
					catch (com_exception& e)
					{
						hr = e.Error();
					}
					catch (...) {
					}
					return hr;

				}


				DWORD wait_alertable_process(DWORD tio) {
					DWORD ws;
					HANDLE hh[2] = { hevent.hko ,hprocess.hko };
					//ULONGLONG ts = GetTickCount64();
					while (WAIT_IO_COMPLETION == (ws = ::WaitForMultipleObjectsEx(2, hh, false, tio, true))) {};
					return ws;
				}

				//DuplicateHandle(GetCurrentProcess(),h,GetCurrentProcess(),&hko,0,FALSE,DUPLICATE_SAME_ACCESS);

				bool  set_process_handle(HANDLE  hp) {
					HANDLE hcp = GetCurrentProcess();
					return DuplicateHandle(hcp, hp, hcp, &hprocess.hko, 0, FALSE, DUPLICATE_SAME_ACCESS);
				}



				HRESULT result_callback(DWORD tio, VARIANT* pv = 0, int fstrict = 0) {
					HRESULT hr;
					DWORD ws;
					//if(ws=WaitForSin)
					//static int wait_loop(HANDLE* phko, int n = 1, int dwMilliseconds = INFINITE, DWORD dwFlags = MWMO_INPUTAVAILABLE | MWMO_ALERTABLE)
					//
					//ws = wait_loop(&hevent.hko,1,tio);
					//ws = WaitForSingleObjectEx(hevent.hko, tio, true);
					ws = wait_alertable_process(tio);

					if (WAIT_OBJECT_0 == ws) {

						hr = S_OK;
					}
					else if ((WAIT_OBJECT_0 + 1) == ws) {
						hr = HRESULT_FROM_WIN32(ERROR_PROCESS_ABORTED);// (RPC_S_COMM_FAILURE);//0x80070079;// HRESULT_FROM_WIN32(WSAETIMEDOUT);
					}
					else if (WAIT_TIMEOUT == ws) {
						hr = 0x80070079;
					}
					else
					{
						hr = E_FAIL;
					}

					if (S_OK == hr) {

						if (pv) {
							locker_t<mutex_t> lock(mutex);
							

								variant_t tmp = result;
								*pv = tmp.Detach();
							

						}
					}
					else if ((!fstrict))
					{
						if (pv) {
							VARIANT v = { VT_ERROR };
							v.scode = hr;
							*pv = v;
						}

						hr = S_OK;
					}

					return hr;

				}

				inline	HRESULT impl_Wait(WORD flags, DISPPARAMS*dp, VARIANT*res, EXCEPINFO*pexi)
				{
					return E_NOTIMPL;
				}


				inline DWORD 	wait_result(VARIANT* res)
				{
					DWORD ws;

				}



				inline	HRESULT impl_Invoke(DISPID id, REFIID riid, LCID lsid, WORD flags, DISPPARAMS*dp, VARIANT*res, EXCEPINFO*pexi, UINT* pu) {

					HRESULT hr;

					if (id == id_result)
						return hr = impl_Result(flags, dp, res, pexi);
					else if (id == id_wait)
						return hr = impl_Wait(flags, dp, res, pexi);

					if (is_get(flags) && res)
					{

						if (id == id_parent)
						{
							bool f;
							VARIANT v = { VT_EMPTY };
							HRESULT hr0;
							hr0 = super_factory_t::CoGetObject(L"ltx.bind:external", __uuidof(IDispatch), &v.byref);
							f = SUCCEEDED(hr0);
							if (f) v.vt = VT_DISPATCH;
							*res = v;
							return f ? S_OK : S_FALSE;

						}
						if ((id == id_pid))
						{
							VARIANT v = { VT_I4 };
							v.intVal = GetCurrentProcessId();
							*res = v;
							return S_OK;

						}
						if ((id == id_decoder))
						{
							VARIANT v = { VT_BOOL };
							v.boolVal = (process_disp->result_decoder) ? VARIANT_TRUE : VARIANT_FALSE;
							*res = v;
							return S_OK;

						}

					}
					return __super::impl_Invoke(id, riid, lsid, flags, dp, res, pexi, pu);

				}




				void hold_params(DISPPARAMS* pdp, bool fholdparams) {
					if (fholdparams) {
						int len = pdp->cArgs;
						vholder.resize(len);
						for (int i = 0; i < len; i++) {
							vholder[i] = pdp->rgvarg[i];
						}
					}
				}

				variant_t result;
				event_local_t hevent;
				kobj_holder_t hprocess;

				std::vector<variant_t> vholder;
				DWORD pid;
				bool fdecoder;
				Parent* process_disp;

				process_args_t(HANDLE h_event, DISPPARAMS* pdp, bool fholdparams=false)
					:args_base_t<process_args_t, RefCounterMT_t<1>, mutex_cs_t >(pdp), hevent(h_event) {
					hold_params(pdp, fholdparams);
				}

				~process_args_t() {
					DBG_PRINT(L" process_args_t DESTROY ");
				}
			};


			struct  __declspec(uuid("{55DA116A-AEA5-4d68-88F8-8DBF19FFFE88}"))  process_factory_t :simple_factory_t<process_factory_t, true, false>
			{
				enum {
					id_env = 1,
					id_exitcode = 2,
					id_terminate = 3,
					id_pid,
					id_resume,
					id_putenv
				};

				struct process_disp_t :IDispatch_base_t<process_disp_t> // IDispatch_impl_t<process_disp_t>					
				{


					struct desktop_creator_t
					{
						bstr_t desk;
						HDESK hdesk, hdeskc;
						HRESULT hr;
						desktop_creator_t(bstr_t _desk) :hdesk(0), hdeskc(0), hr(0)
						{
							wchar_t *p = _desk;
							if (p&&p[0])
							{

								if (*p == '+') {
									desk = p + 1;
									hdeskc = CreateDesktopW(desk, 0, 0, 0, GENERIC_ALL, 0);
									if (!hdeskc) hr = HRESULT_FROM_WIN32(GetLastError());
								}
								else
								{



									desk = _desk;
									hdesk = OpenDesktopW(desk, 0, 0, GENERIC_ALL);
									if (!hdesk) hr = HRESULT_FROM_WIN32(GetLastError());
									else CloseDesktop(hdesk);
								}

							}
						}
						~desktop_creator_t()
						{
							//if(hdesk) CloseDesktop(hdesk);
							if (hdeskc) CloseDesktop(hdeskc);
						}

					};


					inline HRESULT findname(LPOLESTR name, DISPID& dispid) {

						HRESULT hr;
						static  BEGIN_STRUCT_NAMES(pn)
							DISP_PAIR_IMPLC(call, DISPID_VALUE)
							DISP_PAIR_IMPLC(run, DISPID_VALUE)
							DISP_PAIR_IMPLC_LEN(env, id_env, 3)
							DISP_PAIR_IMPLC_LEN(error, id_exitcode, 3)
							DISP_PAIR_IMPLC(exitcode, id_exitcode)
							DISP_PAIR_IMPLC(pid, id_pid)
							DISP_PAIR_IMPLC_LEN(terminate, id_terminate, 4)
							DISP_PAIR_IMPLC(resume, id_resume)
							DISP_PAIR_IMPLC(putenv, id_putenv)
							END_STRUCT_NAMES;
						hr = find_id(pn, name, &dispid);
						return hr;
					}





					inline bool set_std_io(STARTUPINFOW& si, bool &f) {




						if (h_in) {
							si.hStdInput = h_in;
							f |= true;
						}
						if (h_out) {
							si.hStdOutput = h_out;
							f |= true;
						}
						if (h_err) {
							si.hStdError = h_err;
							f |= true;
						}

						if (f)
							si.dwFlags |= STARTF_USESTDHANDLES;
						return f;
					}


					inline	HRESULT impl_Invoke(DISPID id, REFIID riid, LCID, WORD flags, DISPPARAMS*dp, VARIANT*res, EXCEPINFO*pexi, UINT*) {



						arguments_t args(dp);

						HRESULT hr = E_UNEXPECTED;




						try
						{
							locker_t<mutex_cs_t> lock(mutex);

							arguments_t args(dp);
							if ((id == id_terminate) && (DISPATCH_METHOD&flags))
							{
								int exc = args[0].def<int>(-1);
								if (fautojob) {
									return (::TerminateJobObject(hjob, exc)) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
								}
								else {

									return (TerminateProcess(processinfo.hProcess, exc)) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
								}

							}
							else if (id == id_resume) {

								VARIANT v = { VT_I4 };

								v.intVal = ::ResumeThread(processinfo.hThread);

								if (res)
									*res = v;

								return (v.intVal >= 0) ? S_OK : HRESULT_FROM_WIN32(GetLastError());

							}
							else if ((id == id_exitcode) && is_get(flags) && res)
							{
								VARIANT v = { VT_I4 };

								//locker_t<mutex_cs_t> lock(mutex);

								DWORD exitcode = -1;


								if (!GetExitCodeProcess(processinfo.hProcess, &exitcode))
									return HRESULT_FROM_WIN32(GetLastError());

								int fmt = args[0].def<int>(0);
								if (fmt)
								{
									v.vt = VT_BSTR;
									bstr_t msg;
									if (fmt == 16) msg = v_buf<wchar_t>().printf(L"0x%08x", exitcode);
									else msg = error_msg(exitcode);
									v.bstrVal = msg.Detach();

								}
								else
									v.intVal = exitcode;




								*res = v;
								return S_OK;
							}
							else if (id == id_pid) {
								VARIANT v = { VT_I4 };
								v.intVal = processinfo.dwProcessId;
								if (res) {
									*res = v;
								}
								return S_OK;
							}

							else if (id == id_env)
							{

								//int flagApp,cb;
								int cb;
								bstr_t e = args[0].def(bstr_t());
								if (cb = e.length()) {
									argsE.init(cb, e);
									/*
									argv_zzs<wchar_t> argsE;
									argsE= argv_zz<wchar_t>((wchar_t*)env);
									env=argsE.flat_str();
									*/
								}

								hr = toVARIANT(static_cast<IUnknown*>(this), res);
								return hr;
							}
							else if (id_putenv == id) {
								bstr_t key = args[0].def(bstr_t());
								bstr_t value = args[1].def(bstr_t());
								argsE[(wchar_t*)key] = (wchar_t*)value;
								return S_OK;
							}



							if ((id != 0) || is_put(flags))return hr = E_NOTIMPL;


							if (process_args_suspend) {

								hr = process_args_suspend->result_callback(tio, res, fstrict);
								return hr;

								//WSAETIMEDOUT

							}


							if (processinfo.hProcess) return E_ACCESSDENIED;



							//OLE_CHECK_PTR(res);


							//inline int start_process_desktop(bstr_t desktop,bstr_t lpCmdLine,DWORD waittime=INFINITE,PROCESS_INFORMATION* ppi=NULL,DWORD crflags=0,void* penv=0,bool finher=0)

							//bstr_t cmd;
							wchar_t* cmd;
							//argv_zzs<wchar_t> argsE;
							DWORD  CF;

							bool fsuspend = (cf & CREATE_SUSPENDED);// || (tio != INFINITE);

							{

								locker_t<mutex_cs_t> lock(mutex);

								//exitcode=-1;			
								/*
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
								*/


								/*
								if(env.length())
									argsE= argv_zz<wchar_t>((wchar_t*)env);
								else argsE= argv_env<wchar_t>();
								*/



								CF = cf | CREATE_UNICODE_ENVIRONMENT;
							}


							event_local_t hevent;

							smart_ptr_t <process_args_t<process_disp_t> > process_args(new process_args_t<process_disp_t>(hevent, dp, fsuspend), false);

							process_args->process_disp = this;

							stub_holder_t stub_holder(process_args);




							argsE[LTX_PROCESS_OBJREF] = stub_holder.objref();
							argsE[L"LTX_MONIKER_EXTERNAL_OBJECT"] = LTX_PROCESS_OBJREF;


							if (fsuspend) {
								process_args_suspend = process_args;
								stub_holder.detach_to(stub_holder_suspend);
							}


							wchar_t* pent = (wchar_t*)argsE.flat_str();

							fdebug && (CF = CF | CREATE_SUSPENDED);


							desktop_creator_t& dc = desktop;
							OLE_CHECK_hr(dc.hr);

							{


								STARTUPINFOW si = { sizeof(si),0,dc.desk,0,0,0,0,0,0,0,0,STARTF_USESHOWWINDOW,wshow };
								moniker_parser_t<wchar_t> mp;
								OLE_CHECK_hr(mp.cmdline_parse(&cmd, filename, cmdline, path));

								wchar_t* pexename = (flag_assoc) ? 0 : (wchar_t*)filename;

								if (!fnestedjob) CF |= CREATE_BREAKAWAY_FROM_JOB;

								if (hjob) {
									CF |= CREATE_SUSPENDED;

								}


								bool f, fih = false;
								set_std_io(si, fih);

								if (!(f = start_process_desktop_ex(bstr_t(), pexename, cmd, 0, &processinfo, CF, pent, fih, &si)))
								{
									hr = HRESULT_FROM_WIN32(GetLastError());
									return hr;
								}


								f = process_args->set_process_handle(processinfo.hProcess);
								OLE_CHECK_hr_cond(f, GetLastError());


								if (hjob)
								{

									BOOL f = AssignProcessToJobObject(hjob, processinfo.hProcess);

									if (0 == fautojob) {
										ipc_utils::safe_close_handle(ipc_utils::make_detach(hjob));
									}

									if (!f)
									{
										hr = HRESULT_FROM_WIN32(GetLastError());
										TerminateProcess(processinfo.hProcess, hr);
										return hr;
									}

									(fdebug) || (fsuspend) || (ResumeThread(processinfo.hThread));

								}


								//MessageBox
							}
							if (fdebug)
							{

								MessageBoxW(0, v_buf<wchar_t>().printf(L"process[%d]: %s\n cmdline=\"%s\""
									, processinfo.dwProcessId, (wchar_t*)filename, (wchar_t*)cmdline)
									, L"Waiting Debugger", MB_OK | MB_SYSTEMMODAL | MB_ICONINFORMATION);
								ResumeThread(processinfo.hThread);
							}


							//InterlockedExchange(&pid,processinfo.dwProcessId);

							if (fsuspend) {

								if (res)
								{
									*res = variant_t((IDispatch*)this).Detach();

									//WSAETIMEDOUT
								}

								return S_OK;
							}


							DWORD ws;

							{
								unlocker_t<mutex_cs_t> unlock(mutex);
								hr = process_args->result_callback(tio, res, fstrict);
								/*
								if((res)&&SUCCEEDED(hr)&&(!!result_decoder)){

									variant_t v = ipc_utils::make_detach(*res, VARIANT());

									DISPPARAMS dps = { &v,0,1,0 };
									hr = result_decoder->Invoke(0, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &dps, res, 0, 0);
								}
								*/

							}
							/*
							if(tio)
							{
							  unlocker_t<mutex_cs_t> unlock(mutex);

							  HANDLE hh[2]={hevent.hko,processinfo.hProcess};
							  ws=WaitForMultipleObjectsEx(2,hh,false,tio,true);
							}

							if(ws!=WAIT_OBJECT_0)
							{
							locker_t<mutex_cs_t> lock(mutex);
							DWORD ec=-1;
							if(GetExitCodeProcess(processinfo.hProcess,&ec));
							exitcode=ec;

							//if(ec) return hr=HRESULT_FROM_WIN32(ec);
							}
							*/


							//hr = process_args->result_callback(tio, res);
							/*
							if(res)
							{
								*res=process_args->result.Detach();
							}
							*/

							process_args->clear();
							return hr;



						}
						catch (com_exception& e)
						{
							hr = e.Error();
						}
						catch (...) {
						}
						return hr;

						return S_OK;
					}



					//process_disp_t(bstr_t _desktop,bstr_t _filename,bstr_t _cmdline,bstr_t _env,int _wshow,DWORD _cf,int _fdebug=0)
					process_disp_t(bstr_t _desktop, bstr_t _filename, bstr_t _cmdline, int _wshow, DWORD _cf, int _fdebug = 0)
						:wshow(_wshow), cf(_cf), xexitcode(-1), pid(0), fdebug(_fdebug), fdesktop_create(0), tio(INFINITE), hjob(0),
						desktop(_desktop), fautojob(0), fstrict(0)
					{

						//desktop=_desktop;

						filename = _filename;
						cmdline = _cmdline;
						//env=_env;

					}
					~process_disp_t() {

						fdebug = -1;
						ipc_utils::safe_close_handle(hjob);
						DBG_PRINT(L"DESTROY process_disp_t");


					}

					argv_zzs<wchar_t> argsE;
					mutex_cs_t mutex;
					bstr_t filename, cmdline, path;// ,desktop ;
					desktop_creator_t desktop;
					int wshow, fdesktop_create;
					DWORD cf;
					DWORD tio;
					volatile LONG pid;
					processinfo_t processinfo;
					LONG xexitcode;
					int fdebug, fautojob;
					int flag_assoc;
					int fnestedjob;
					int fstrict;
					HANDLE hjob;
					//
					handle_t h_in, h_err, h_out;

					smart_ptr_t < process_args_t<process_disp_t> > process_args_suspend;
					stub_holder_t stub_holder_suspend;
					smart_GIT_ptr_t<IDispatch> result_decoder;



				};


				struct process_git_t : IGITDispatch_t<process_git_t>
				{
					process_git_t(DWORD dw) :IGITDispatch_t<process_git_t>(dw) {};
					process_git_t(IDispatch* pd) :IGITDispatch_t<process_git_t>(pd) {};

				};

				struct creator_process_t
				{
					HRESULT hr;
					DWORD dwCookie;
					smart_ptr_t<process_disp_t> pd;

					creator_process_t(process_disp_t *p) :pd(p, false), hr(E_FAIL), dwCookie(0) {}
					inline HRESULT	operator()(DWORD t)
					{
						return hr = GIT_t::get_instance().wrap((IDispatch*)pd.p, &dwCookie);
					}

				};

				inline HRESULT get_job_handle(moniker_parser_t<wchar_t>& mp, HANDLE& hjob, int& fautojob)
				{
					HRESULT hr;

					moniker_parser_t<wchar_t>::arg_result_t& r = mp.bind_args(L"job");
					ipc_utils::smart_ptr_t<i_mbv_kernel_object> ko;
					kernel_object_t* pko;
					if (SUCCEEDED(hr = r.QueryInterface(ko._address())))
					{

						OLE_CHECK_hr(ko->GetContext((void**)&(pko = 0)));
						hjob = ipc_utils::dup_handle(pko->hko);
						return S_OK;

					}
					else {
						bstr_t n = r.def<bstr_t>();
						if (!n.length())
							return S_OK;

						wchar_t* pn = mp.undecorateQQ((wchar_t*)n);
						HANDLE h;
						//::CreateJobObjectW(NULL, NULL);
						if ((wcslen(pn) == 1)) {


							if (*pn == L'*') {
								h = script_engine_t::process_com_t::get_instance().hprocess_job;
								hjob = dup_handle(h);
							}
							else if (*pn == L'+') {
								hjob = ipc_utils::create_job();
								fautojob = 1;
							}


						}
						else {
							h = OpenJobObjectW(JOB_OBJECT_ASSIGN_PROCESS, false, pn);
							OLE_CHECK_hr_cond(h, GetLastError());
							hjob = h;
						}


						return S_OK;
					}





					return hr;


				}

				static HRESULT set_std_io_handle(moniker_parser_t<wchar_t>& mp, const wchar_t* name, handle_t& h) {

					HRESULT hr;
					kernel_object_t* pko;

					h.close();

					smart_ptr_t<i_mbv_kernel_object> std_io;


					OLE_CHECK_hr(mp.bind_args(name).QueryInterface(std_io));



					OLE_CHECK_hr(std_io->GetContext((void**)&(pko = 0)));

					h.reset(pko->hko, true);

					return h ? S_OK : E_FAIL;


				}


				static bool _set_std_io_handle(i_mbv_kernel_object* ko, handle_t& h) {

					HRESULT hr = E_FAIL;
					h.close();
					kernel_object_t* pko = NULL;

					if (ko&&SUCCEEDED(hr = ko->GetContext((void**)&pko))) {
						h.reset(pko->hko, true);
					}

					return h.check();

				}



				inline HRESULT create_iaggregator(aggregator_container_helper_t& container, LPOLESTR pszDisplayName, IStream*, GUID& uiid, smart_ptr_t<iaggregator_helper>& disp, IBindCtx * pbc)
				{
					HRESULT hr,hr0;
					moniker_parser_t<wchar_t> mp(pszDisplayName, pbc);

					/*
					wchar_t* desk=mp.trim(mp.args[L"desk"]);
					wchar_t* filename=mp.trim(mp.args[L"file"]);
					wchar_t* cmdline=mp.trim(mp.args[L"cmd"]);
					*/



					wchar_t* desk = (mp.trim(mp.bind_args(L"desk").def<bstr_t>(L"")));
					wchar_t* filename = mp.trim(mp.bind_args(L"file").def<bstr_t>(L""));
					wchar_t* cmdline = (mp.trim(mp.bind_args(L"cmd").def<bstr_t>(L"")));




					//filename=mp.trim(mp.bind_args(L"file").def<bstr_t>(L""));

					//DWORD cf=mp.args[L"cf"].def<int>(0);
					//DWORD sw=mp.args[L"sw"].def<int>(SW_SHOW);

					DWORD cf = mp.bind_args(L"cf").def<int>(0);
					DWORD sw = mp.bind_args(L"sw").def<int>(SW_SHOW);
					DWORD tio = mp.bind_args(L"tio").def<int>(INFINITE);

					int  jhook = 0, flag_assoc;
					jhook = mp.bind_args(L"dbghook").def<int>(jhook);
					flag_assoc = mp.bind_args(L"flags.assoc").def<int>(1);






					//process_disp_t* process=;


					process_git_t* process_git;

					process_disp_t* process_disp = new process_disp_t(desk, filename, cmdline, sw, cf, jhook);

					OLE_CHECK_hr(process_disp->desktop.hr);
					process_disp->flag_assoc = flag_assoc;
					process_disp->tio = tio;


					set_std_io_handle(mp, L"std.in", process_disp->h_in);
					set_std_io_handle(mp, L"std.out", process_disp->h_out);
					set_std_io_handle(mp, L"std.err", process_disp->h_err);


					wchar_t* env = mp.get_cache((wchar_t*)mp.bind_args(L"env").def<bstr_t>()).cat(L"\0");
					int cbenv;
					if (cbenv = safe_len(env)) {
						process_disp->argsE.init(cbenv, env);
					}
					else {
						process_disp->argsE = argv_env_r<wchar_t>();
					}

					env = mp.get_cache((wchar_t*)mp.bind_args(L"env+").def<bstr_t>()).cat(L"\0");

					if (cbenv = safe_len(env)) {
						argv_zzs<wchar_t> senv(cbenv, env);
							process_disp->argsE.update(senv);
											
					}

					{

						bstr_t bpathp = mp.bind_args(L"path+").def<bstr_t>();

						if (bpathp.length()) {

							bpathp += L";" + bstr_t((wchar_t*)process_disp->argsE[L"path"]);
							process_disp->argsE[L"path"] = (wchar_t*)bpathp;

						}
					}

					/*
										smart_ptr_t<i_mbv_kernel_object> std_io;


										mp.bind_args(L"std.in").QueryInterface(std_io);
										_set_std_io_handle(std_io,process_disp->h_in);

										mp.bind_args(L"std.err").QueryInterface(std_io);
										_set_std_io_handle(std_io,process_disp->h_err);

										mp.bind_args(L"std.out").QueryInterface(std_io);
										_set_std_io_handle(std_io,process_disp->h_out);

					*/






					OLE_CHECK_hr(get_job_handle(mp, process_disp->hjob, process_disp->fautojob));

					process_disp->fnestedjob = mp.bind_args(L"job.nested").def<int>(1);
					process_disp->fstrict = mp.bind_args(L"flags.strict").def<int>(0);

					//result_decoder
					smart_ptr_t<IDispatch> rd;
					if (SUCCEEDED(hr0=mp.bind_args(L"result.decoder").QueryInterface(rd))) {
						process_disp->result_decoder.reset(rd);
					}

					creator_process_t cp(process_disp);


					DWORD tt = 0;
					if (COMInit_t::is_MTA())	hr = cp(tt);
					else hr = mta_singleton_t::apartment().call_sync(cp, hr, E_UNEXPECTED);


					if (FAILED(hr)) return hr;

					process_git = new process_git_t(cp.dwCookie);

					disp = aggregator_helper_creator_t<process_git_t>::New(process_git);

					mta_singleton_t::add_external_connector(container);
					return (disp) ? S_OK : E_OUTOFMEMORY;

				}






			};


			struct  __declspec(uuid("{AD20A6BB-8C1E-4fdf-92BD-BC3C07F846C4}"))  stub_holder_factory_t :simple_factory2_t<stub_holder_factory_t, true, false>
			{

				struct IStubHolder_aggregator_t :base_aggregator_t<IStubHolder_aggregator_t, i_stub_holder>
				{
					stub_holder_t sh;
					virtual HRESULT STDMETHODCALLTYPE objref(BSTR* pref) {
						HRESULT hr;
						OLE_CHECK_PTR(pref);
						bstr_t b = sh.objref();
						if (SUCCEEDED(hr = sh.hr))
							*pref = b.Detach();


						return hr;


					};
					virtual HRESULT STDMETHODCALLTYPE wrap(IUnknown* punk) {
						HRESULT hr;
						return  hr = sh.reset(punk);

					};
					virtual HRESULT STDMETHODCALLTYPE unwrap(REFIID riid, void** ppObj) {
						HRESULT hr;
						return hr = sh.unwrap(riid, ppObj);
					};
					inline stub_holder_t* stub_ptr() {
						return &sh;
					}

				};
				struct stub_holder_disp_t :IDispatch_impl_t<stub_holder_disp_t>
				{
					inline	HRESULT impl_Invoke(DISPID id, REFIID riid, LCID, WORD flags, DISPPARAMS*dp, VARIANT*res, EXCEPINFO*pexi, UINT*) {

						HRESULT hr = S_OK;
						arguments_t args(dp);
						if (args.argc)
						{
							smart_ptr_t<IUnknown> unk;
							VARIANT v = args[0];
							if (v.vt != VT_NULL)
								OLE_CHECK_hr(args[0].QueryInterface(unk._address()));
							psh->reset(unk);
							if (!unk) {
								psh->hr = E_POINTER;
								return S_OK;
							}
						}

						OLE_CHECK_hr(psh->hr);

						if (res)
						{

							VARIANT v = { VT_BSTR };
							v.bstrVal = psh->objref().Detach();
							*res = v;

						}

						return hr;


					}

					stub_holder_disp_t(stub_holder_t* _psh) :psh(_psh) {};

					stub_holder_t* psh;

				};

				inline HRESULT set_iaggregators(aggregator_container_helper_t& container, LPOLESTR pszDisplayName, IStream*, IBindCtx * pbc)
				{

					HRESULT hr;
					smart_ptr_t<iaggregator_helper> helper, helper2;


					IStubHolder_aggregator_t*  psha = new IStubHolder_aggregator_t;
					helper = aggregator_helper_creator_t<IStubHolder_aggregator_t>::New(psha);
					helper2 = aggregator_helper_creator_t<stub_holder_disp_t>::New(new stub_holder_disp_t(psha->stub_ptr()));

					hr = container.add(__uuidof(i_stub_holder), helper).add(__uuidof(IDispatch), helper2);
					return hr;

				}

			};


			//struct  __declspec(uuid("C328C5EF-F7E0-4603-845F-639BA04346DD"))  stub_holder_cache_factory_t :simple_factory2_t<stub_holder_cache_factory_t, true, false>
			struct  __declspec(uuid("C328C5EF-F7E0-4603-845F-639BA04346DD"))  stub_holder_cache_factory_t :simple_factory2_t<stub_holder_cache_factory_t, false, false>
			{
				struct creator_t {

					//stub_holder_cache_creator_t(stub_holder_cache_factory_t* _owner) :owner(_owner) {}

					creator_t(stub_holder_cache_factory_t* _owner) :dwCookie(0), hr(E_FAIL), owner(_owner) {};

					HRESULT operator()(LPOLESTR pszDisplayName) {

						aggregator_container_helper_FMT_EC<false, false> container;
						OLE_CHECK_hr(owner->set_iaggregators(container, pszDisplayName, NULL, NULL));
						//hr = container.QueryInterface(riid, ppvObject);
						return hr = container.unwrap(&dwCookie);

					};

					HRESULT QuieryInterface(REFIID riid, void** pp) {
						OLE_CHECK_hr(hr);
						return hr = ipc_utils::GIT_t::get_instance().unwrap(dwCookie, riid, pp);
					}

					HRESULT hr;
					DWORD dwCookie;
					stub_holder_cache_factory_t* owner;

				};

				struct stub_holder_list_disp_t :IDispatch_impl_t<stub_holder_list_disp_t>
				{

					struct stub_holder2_t {

						void reset(IUnknown* punk) {
							if (punk) {
								unk.reset(punk);
								stub.reset(punk);
							}
						};
						void  objref(VARIANT* res) {
							if (res) {

								VARIANT v = { VT_BSTR };
								v.bstrVal = stub.objref().Detach();
								*res = v;
							}

						};
						~stub_holder2_t() {
							stub.reset();
						}
						stub_holder_t stub;
						smart_ptr_t<IUnknown> unk;
					};

					inline	HRESULT impl_Invoke(DISPID id, REFIID riid, LCID, WORD flags, DISPPARAMS*dp, VARIANT*res, EXCEPINFO*pexi, UINT*) {

						HRESULT hr = E_INVALIDARG;
						arguments_t args(dp);

						locker_t<mutex_cs_t> lock(mutex);


						if (args.argc == 1)
						{
							smart_ptr_t<IUnknown> unk;
							OLE_CHECK_hr(args[0].QueryInterface(unk._address()));
							if (unk) {

								shl.push_back(stub_holder2_t());
								stub_holder2_t& h = shl.back();
								h.reset(unk);
								OLE_CHECK_hr(h.stub.hr);

								h.objref(res);

								return hr;
							}
							return S_OK;
							//psh->reset(unk);

						}
						else if (args.argc > 1) {
							int i = args[0].def<int>(-1), c = shl.size();

							if (i == -1)
								shl.clear();
							else return hr = E_INVALIDARG;

							VARIANT v = { VT_I4 };
							v.intVal = c;
							if (res)
								*res = v;
							return S_OK;
						}

						if ((args.argc == 0)) {

							VARIANT v = { VT_I4 };
							v.intVal = shl.size();
							if (res)
								*res = v;
							return S_OK;
						}

						return hr;


					}

					//stub_holder_disp_t():psh(_psh) {};
					mutex_cs_t mutex;
					std::list<stub_holder2_t> shl;

				};

				inline HRESULT impl_CreateInstanceEx(IUnknown * pUnkOuter, LPOLESTR pszDisplayName, IStream* stream, REFIID riid, void ** ppvObject, IBindCtx * pbc)
				{
					HRESULT hr;
					/*
					aggregator_container_helper_FMT_EC<false, false> container;

					OLE_CHECK_hr(this->set_iaggregators(container, pszDisplayName, stream, pbc));
					hr = container.QueryInterface(riid, ppvObject);
					*/
					//moniker_parser_t<wchar_t> mp(pszDisplayName, pbc);


					creator_t creator(this);

					if (COMInit_t::is_MTA())
						hr = creator(pszDisplayName);
					else
						hr = mta_singleton_t::apartment().call_sync(creator, pszDisplayName, E_FAIL);

					if (SUCCEEDED(hr))
						hr = creator.QuieryInterface(riid, ppvObject);

					return hr;
				}

				inline HRESULT set_iaggregators(aggregator_container_helper_t& container, LPOLESTR pszDisplayName, IStream*, IBindCtx * pbc)
				{

					HRESULT hr;
					smart_ptr_t<iaggregator_helper> helper;



					helper = aggregator_helper_creator_t<stub_holder_list_disp_t>::New(new stub_holder_list_disp_t());

					hr = container.add(__uuidof(IDispatch), helper);
					return hr;

				}

			};



		}; // script_engine
	}; // com_apartments
}; //ipc_utils
