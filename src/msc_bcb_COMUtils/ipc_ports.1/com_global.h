#pragma once
//#include "com_global.h" 
#include "ipc_utils.h" 
#include "tls_dll.h" 

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
			mutex_t mutex;
			HWND hwinconsole;
			apartment_t * p_mta, * p_sta;

			DWORD dw_cookie_js,dw_cookie_vbs;
			DWORD dw_cookie_js_this;
			//DWORD dw_cookie_external_event;
			smart_GIT_ptr_t<IDispatch> external_event;
			callback_lib_arguments_t callback_lib_arguments;





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


			global_com_t():p_mta(0),p_sta(0),dw_cookie_js(0),dw_cookie_js_this(0),finstall_STS(0){
				//COMInitF_t ci;
				//locker_t<mutex_t> lock(mutex);

				//engine_t::CreateInstance(L"JScript",0,&(dw_cookie_engine=0));
				callback_lib_arguments_t t={&calback_aggregator_t<IDispatch>::CreateInstanceV,&ltxGetObject};
				callback_lib_arguments=t;

			};

			~global_com_t(){

				{
					locker_t<mutex_t> lock(mutex);
					GIT_t::get_instance().unwrap(dw_cookie_js);

				}
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



	};//com_apartments
};//ipc_utils