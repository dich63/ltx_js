#pragma once
//#include "com_global.h" 
#include "ipc_utils.h" 
#include "tls_dll.h" 
#include "sa_utils.h" 
#include "com_marshal_templ.h"
#include <algorithm>

namespace ipc_utils {
	namespace com_apartments
	{
#pragma pack(push)
#pragma pack(1)

		template <int _Version=0>
		struct tls_com_t: non_copyable_t
		{
			struct buffer_t
			{
				INT32 capacity;
				INT32 size;
				char p[1];
			};
			struct tls_struct{};

			typedef tls_com_t<_Version> self_t;

			static self_t& get_instance()
			{
				self_t* p;
				if(!(p=tls_var<tls_struct>()))
				{
					p=new tls_com_t;
					tss_cleaner::on_exit_thread(&on_exit,p);
					tls_var<tls_struct>()=p;
				}
                return *p;

			};

			inline int length()
			{
				return (buffer)?buffer->size:0;
			}

			template <class N>
			N* get_ptr()
			{
				return (buffer)?((N*)buffer->p):(N*)0;
			}
			char* set_buffer(int cb,void* ptr=0)
			{
				
				if(((!buffer))||(buffer->capacity<cb))
				{					
					buffer=(buffer_t*)realloc(buffer,cb+4*(sizeof(INT32)));
					if(!buffer) return 0;
					buffer->capacity=cb;
					
				}
				buffer->size=cb;
				if(ptr)	{ 
					memcpy(buffer->p,ptr,cb);
				 *((INT32*)(buffer->p+cb))=0;
				}
                return   buffer->p;
			}

		protected:
			static void on_exit(void* p)
			{
				delete (self_t*)p;
			}
			tls_com_t():buffer(0){};
			~tls_com_t(){ free(buffer); };
			buffer_t* buffer;
		};

#pragma pack(pop)

		template <class Engine>
		struct global_com_t: non_copyable_t
		{
			typedef typename Engine engine_t;
			typedef apartment_t<> apartment_t;
			typedef mutex_cs_t mutex_t ;
			typedef  mutex_ko_t mutex_host_t;

			struct marshal_helper_t:i_marshal_helper_t
			{   
				typedef base_marshal_utils_t<> marshal_utils_t;
				virtual  HRESULT marshal_hko(IStream* pStm,HANDLE hko=0){
					HRESULT hr;
					return hr=marshal_utils_t::marshal_hko_to_process(hko,pid,pStm);
				};
				virtual  HRESULT marshal_interface(IStream* pStm,REFIID riid,IUnknown* pObj){
					HRESULT hr;
					return hr=CoMarshalInterface(pStm,riid,pObj,MSHCTX_LOCAL,0,MSHLFLAGS_NORMAL); 
				};
				virtual  HRESULT unmarshal_hko(IStream* pStm,HANDLE* phko=0){
					HRESULT hr;
					return hr=marshal_utils_t::unmarshal_hko(pStm,phko);

				};
				virtual HRESULT unmarshal_and_release(IStream *pStm,REFIID riid,void** ppObj){
					HRESULT hr;
					return hr=marshal_utils_t::unmarshal_and_release(pStm, riid, ppObj);
				};
				virtual HRESULT marshal_BSTR(BSTR bstr,IStream* pStm)
				{
					HRESULT hr;
					return hr=marshal_utils_t::marshal_BSTR(bstr,pStm);							
				}

				virtual HRESULT unmarshal_BSTR(IStream* pStm,BSTR* pbstr)
				{
					HRESULT hr;
					return hr=marshal_utils_t::unmarshal_BSTR(pStm,pbstr);							
				}

				marshal_helper_t(DWORD _pid=GetCurrentProcessId()):pid(_pid){};
				DWORD pid;
			};

			marshal_helper_t marshal_helper;


			mutex_t mutex;
			mutex_host_t host_mutex;
			HWND hwinconsole;
			apartment_t * p_mta, * p_sta;

			DWORD dw_cookie_js,dw_cookie_vbs;
			DWORD dw_cookie_js_this;
			//DWORD dw_cookie_external_event;
			smart_GIT_ptr_t<IDispatch> external_event;
			callback_lib_arguments_t callback_lib_arguments;
			HANDLE hprocess_job;








			HWND console_hwin(int setm=0,HWND hnew=0)
			{
				locker_t<mutex_t> lock(mutex);
				return (setm)? ipc_utils::make_detach(hwinconsole,hnew):hwinconsole;			    
			}

			DWORD console_pid()
			{
				DWORD pid=0;

				HWND h= console_hwin();
				GetWindowThreadProcessId(h,&pid);				
				return pid;

			}







			bool finstall_STS;

			struct creator_t
			{

				DWORD dw_cookie_vbs;
				DWORD dw_cookie_js;
				DWORD dw_cookie_js_this;

				HRESULT operator()(DWORD cookie) 
				{
					HRESULT hr;
					smart_ptr_t<IDispatch> engine_disp;

					SetThreadName(GetCurrentThreadId(),"DICH: STA singleton");
					//HRESULT create_apartment_script_engine(Intf** ppdisp ,IDispatch* exc_event=0,wchar_t* lang=L"JScript",int fdebug=3,bool fnosafe=0,int with_process=1)

					//if(SUCCEEDED(hr=engine_t::CreateInstance(L"JScript",0,&engine_disp.p,false)))
					engine_t* pengine=0;

					if(SUCCEEDED(hr=create_apartment_script_engine(&engine_disp.p,0,L"JScript",0,1,0,&pengine)))
						hr=ipc_utils::GIT_t::get_instance().wrap(engine_disp.p,&dw_cookie_js);



					variant_t js_this;
					OLE_CHECK_hr(pengine->_call(L"this",&js_this));
					hr=ipc_utils::GIT_t::get_instance().wrap(js_this.pdispVal,&dw_cookie_js_this);
					HRESULT hr0;

					hr0=pengine->call(L"pid=process_id=$$[0]",0,1,&variant_t(int(GetCurrentProcessId())));
					hr0=pengine->call(L"lib_name=$$[0]",0,1,&variant_t((wchar_t*)(module_t<>::module_name())));
					hr0=pengine->call(L"process_name=$$[0]",0,1,&variant_t((wchar_t*)(module_t<>::module_name((void*)0))));
					hr0=pengine->call(L"lib_path=$$[0]",0,1,&variant_t((wchar_t*)(module_t<>::module_path())));
					hr0=pengine->call(L"process_path=$$[0]",0,1,&variant_t((wchar_t*)(module_t<>::module_path((void*)0))));


					buf_res_T<char> bres2(module_t<>::__getHinstance(),L"#103",RT_HTML);
					if(bres2)	 	
						hr0=pengine->_call(bstr_t(bres2));

					return hr;
				}


			}; 


			template <class O>
			HRESULT unwrap_external_event(O& o)
			{
				locker_t<mutex_t> lock(mutex);
				return external_event.unwrap(o);
			}

			bool set_external_event(IDispatch** pevent)
			{
				return   external_event.reset(pevent);
			}




			inline DWORD js_cookie(bool finit=false) 
			{
				HRESULT hr;
				locker_t<mutex_t> lock(mutex);
				if(finit&&(!dw_cookie_js))
				{

					p_sta= new apartment_t(COINIT_APARTMENTTHREADED);
					creator_t creator;
					DWORD dw=0;
					creator.dw_cookie_js=0;
					dw_cookie_js=0;
					hr=p_sta->call_sync(creator,dw_cookie_vbs,E_FAIL);


					if(FAILED(hr)) return 0;
					dw_cookie_js=creator.dw_cookie_js;
					dw_cookie_js_this=creator.dw_cookie_js_this;

				}

				return dw_cookie_js_this;
			}


			inline void create_process_job() {
				SECURITY_ATTRIBUTES _sa = {};
				HANDLE h;
				hprocess_job = NULL;
				h = ::CreateJobObjectW(&_sa, NULL);
				if (!h) return;
				
				hprocess_job = h;
				JOBOBJECT_EXTENDED_LIMIT_INFORMATION j = {};
				j.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION | JOB_OBJECT_LIMIT_BREAKAWAY_OK | JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
				SetInformationJobObject(h, JobObjectExtendedLimitInformation, &j, sizeof(j));

			}



			global_com_t():p_mta(0),p_sta(0),dw_cookie_js(0),dw_cookie_js_this(0),finstall_STS(0),host_mutex(L"ltx_host_mutex_C3EE17B206E04671917D95AC5474D20F"){
				//COMInitF_t ci;
				//locker_t<mutex_t> lock(mutex);

				//engine_t::CreateInstance(L"JScript",0,&(dw_cookie_engine=0));
				callback_lib_arguments_t t={&calback_aggregator_t<IDispatch>::CreateInstanceV,&ltxGetObject,&bindObjectWithArgs};
				callback_lib_arguments=t;

				create_process_job();


			};

			~global_com_t(){

				{
					locker_t<mutex_t> lock(mutex);
					GIT_t::get_instance().unwrap(dw_cookie_js);
					
				}
				ipc_utils::safe_close_handle(hprocess_job);
				delete p_mta;
				delete p_sta;
				
			}

			/*
			struct locker_t
			{
			mutex_t* pmutex;  	
			locker_t (){  (pmutex=&global_com_t::get_instance().mutex)->lock(); }
			~locker_t (){  pmutex->unlock();}

			};

			*/	
			/*
			apartment_t& MTA()
			{

			locker_t<mutex_t> lock(mutex);
			if(!p_mta)
			{
			p_mta= new apartment_t(COINIT_MULTITHREADED);
			SetThreadName(p_mta->tid,"DICH: MTA singleton");
			}
			return *p_mta;

			return mta_singleton_t:apartment();
			}
			*/

			inline static global_com_t& get_instance()
			{
				return class_initializer_T<global_com_t>().get();
			}
		};

		//template <class T, class IDispatchX = IDispatch, class RefCounter = RefCounterMT_t<1> >
		//struct IDispatch_base_t :IDispatch_impl_base_t<T, IUnknown_base_impl<T, IDispatchX, RefCounter > > {};

		template<class bname_t>
		HRESULT _disp_get(WORD flags, bname_t& bn, LCID lsid, DISPPARAMS &dp, IDispatch* pdisp, VARIANT* pr, EXCEPINFO* pei, UINT* perr) {
			HRESULT hr;
			DISPID did;
			//bstr_t b = (wchar_t*)bn;
			wchar_t*p = bn;
			OLE_CHECK_hr(pdisp->GetIDsOfNames(IID_NULL, &p, 1, lsid, &did));
			return hr = pdisp->Invoke(did, IID_NULL, lsid, flags, &dp, pr, pei, perr);
		};

		inline HRESULT jsarray2vector(IDispatch* args, std::vector<variant_t>& vv,bool freverse=false) {
			typedef bstr_c_t<64> bname_t;
			HRESULT hr;
			OLE_CHECK_PTR(args);
			DISPPARAMS dp = {};

			bname_t bname;
			memcpy(bname, L"length", 12);
			variant_t vr;
			VARIANT  vi = {};
			OLE_CHECK_hr(_disp_get(DISPATCH_PROPERTYGET | DISPATCH_METHOD, bname, LOCALE_USER_DEFAULT,dp,args, &vr, NULL, NULL));
			OLE_CHECK_hr(VariantChangeType(&vi, &vr, 0, VT_I4));
			auto len = vi.intVal;
			auto last = len - 1;		

			//vv.clear();
			vv.resize(len);
			
			VARIANT *pv = vv.data();

			for (auto k = 0; k < len; k++) {

				bname = (freverse) ? last - k:k;

				hr = _disp_get(DISPATCH_PROPERTYGET, bname, LOCALE_USER_DEFAULT, dp, args, pv + k, NULL, NULL);
				if (!(SUCCEEDED(hr) || (DISP_E_UNKNOWNNAME == hr))) {
					return hr;
				}
			} 

			return hr;

		};

		inline HRESULT jsarray2vector(VARIANT vargs, std::vector<variant_t>& vv, bool freverse = false) {
			HRESULT hr;
			smart_ptr_t<IDispatch> disp;
			OLE_CHECK_hr(disp.reset(vargs));
			return hr = jsarray2vector(disp, vv, freverse);
		}

		struct function_apply_t:IDispatch_base_t<function_apply_t, IDispatch, RefCounterFake_t<1> >{
			///*
			inline	HRESULT impl_Invoke(DISPID _id, REFIID riid, LCID lsid, WORD w, DISPPARAMS* pdp, VARIANT* res, EXCEPINFO* pei, UINT* perr) {
				HRESULT hr = E_NOTIMPL;

				smart_ptr_t<IDispatch> disp;
				DISPID id = DISPID_VALUE;
				DISPPARAMS dp = {};
				if (pdp->cArgs == 0) {
					if (res) {
						
						res->vt = VT_BSTR;
						res->bstrVal = bstr_t(L"ltx: call_apply( funcion [, array-like arguments])\n").Detach();
						return S_OK;

					}

				}else 
					OLE_CHECK_hr_cond((DISPID_VALUE == _id) && (pdp->cArgs > 0), E_INVALIDARG);

				UINT c = pdp->cArgs - 1;

				
				OLE_CHECK_hr(disp.reset(pdp->rgvarg[c]));

				if (c) {

					VARIANT *pv = sa_utils::PVARIANT_Ref2Ind(pdp->rgvarg + c - 1);
					OLE_CHECK_PTR_hr(pv);
					if (pv->vt==VT_BSTR) {
						OLE_CHECK_hr(disp->GetIDsOfNames(IID_NULL, &(pv->bstrVal), 1, lsid, &id));
						c--;
					}
				}


				if (c) {

					VARIANT *pv = sa_utils::PVARIANT_Ref2Ind( pdp->rgvarg + c - 1);
					OLE_CHECK_PTR_hr(pv);			
					 
					bool fsafe_array = (pv->vt&VT_ARRAY);

					if (!fsafe_array) {

						std::vector<variant_t> params;
						OLE_CHECK_hr(jsarray2vector(pv[0], params, true));

						dp.cArgs = params.size();
						dp.rgvarg = params.data();

						hr = disp->Invoke(id, riid, lsid, w, &dp, res, pei, perr);
					}
					else {

						sa_utils::safe_array_ptr_t sa_ptr;
						

						OLE_CHECK_hr(sa_ptr.init(pv, VT_VARIANT));

						size_t len = sa_ptr.length;

						std::vector<VARIANT> vparams;
						vparams.resize(len);

						VARIANT* ps = sa_ptr, *pd = vparams.data();

						std::reverse_copy(ps, ps + len, pd);
						dp.cArgs = len;
						dp.rgvarg = pd;
						hr = disp->Invoke(id, riid, lsid, w, &dp, res, pei, perr);

					}					
				}
				else {
					hr = disp->Invoke(id, riid, lsid, DISPATCH_METHOD, &dp, res, pei, perr);
				}
				return hr;
			}
			
			//*/


			//
			/*
			typedef bstr_c_t<64> bname_t;
			inline HRESULT _disp_get(WORD flags, bname_t& bn, LCID lsid, DISPPARAMS &dp,IDispatch* pdisp,VARIANT* pr, EXCEPINFO* pei, UINT* perr){
				HRESULT hr;
				DISPID did;
				//bstr_t b = (wchar_t*)bn;
				wchar_t*p = bn;
				OLE_CHECK_hr(pdisp->GetIDsOfNames(IID_NULL,&p, 1, lsid, &did));
				return hr = pdisp->Invoke(did, IID_NULL, lsid,flags, &dp, pr, pei, perr);
			};
			

			

			inline	HRESULT impl_Invoke(DISPID id, REFIID riid, LCID lsid, WORD w, DISPPARAMS* pdp, VARIANT* res, EXCEPINFO* pei, UINT* perr) { 
				HRESULT hr= E_NOTIMPL;

				smart_ptr_t<IDispatch> disp;

				DISPPARAMS dp = {};

				OLE_CHECK_hr_cond((DISPID_VALUE==id)&&(pdp->cArgs > 0), E_INVALIDARG);

				UINT c = pdp->cArgs - 1;

				OLE_CHECK_hr(disp.reset(pdp->rgvarg[c]));

				if (c) {

					smart_ptr_t<IDispatch> args;

					OLE_CHECK_hr(args.reset(pdp->rgvarg[c-1]));

					bname_t bname;
					memcpy(bname, L"length", 12);

					variant_t vr;
					VARIANT  vi = {};
					OLE_CHECK_hr(_disp_get(DISPATCH_PROPERTYGET|DISPATCH_METHOD, bname, lsid, dp,args.p,&vr, pei, perr));
					OLE_CHECK_hr(VariantChangeType(&vi, &vr, 0, VT_I4));



					auto len = vi.intVal;
					auto last = len - 1;

					std::vector<variant_t> params(len);
					VARIANT *pv = params.data();

					for (auto k = 0; k < len; k++) {
						
						bname = last - k;
						hr =_disp_get(DISPATCH_PROPERTYGET, bname, lsid, dp, args.p, pv + k, pei, perr);
						if (!(SUCCEEDED(hr) || (DISP_E_UNKNOWNNAME == hr))) {
							return hr;
						}						
					}
					
					dp.cArgs = len;
					dp.rgvarg = pv;

					hr = disp->Invoke(DISPID_VALUE, riid, lsid, w, &dp, res, pei, perr);

				}
				else {
					hr = disp->Invoke(DISPID_VALUE, riid, lsid, DISPATCH_METHOD, &dp, res, pei, perr);
				}		

								

				return hr; 
			};
			//*/
			
				
			inline static IDispatch* get_ptr() {

				return (IDispatch*)&class_initializer_T<function_apply_t>().get();

			}
		
		};


	};//com_apartments
};//ipc_utils