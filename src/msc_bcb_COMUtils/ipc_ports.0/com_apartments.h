#pragma once
#include "ipc_ports/ipc_utils.h"
#include "dummy_dispatch_aggregator.h"
#include "ipc_ports/imarshal_aggregator.h"

#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>

//#include "static_com.h"
#include "dispid_find.h"

namespace ipc_utils {
	namespace com_apartments
	{

        inline void __add_appartment_thread();
		inline void __rem_appartment_thread();

        template <int Vers=0>  
		struct apartment_list_t
		{

			struct handle_holder_t
			{
				HANDLE hthread;
				HANDLE hevent;
				handle_holder_t():hthread(0),hevent(0){};
				~handle_holder_t(){
					if(hthread) CloseHandle(hthread);
					if(hevent) CloseHandle(hevent);
				}
				void set_events(HANDLE ht,HANDLE he){
                     hevent=dup_handle(he);
					 hthread=dup_handle(ht);
				}
				HANDLE get_hthread(){

					return dup_handle(hthread);
				}




			};
			
			typedef  typename std::list<handle_holder_t> hlist_t;
			typedef  typename  hlist_t::iterator iterator_t;

			typedef mutex_cs_t mutex_t;
			mutex_t mutex;
			hlist_t hlist;

			inline iterator_t push_handles(HANDLE ht,HANDLE he)
			{
				locker_t<mutex_t> lock(mutex);
				hlist.push_front(handle_holder_t());
				iterator_t i=hlist.begin();
				(*i).set_events(ht,he);
				return i;
			}

			inline void pop_handles(iterator_t i)
			{
				locker_t<mutex_t> lock(mutex);
				hlist.erase(i);
			}

			inline int count()
			{
				locker_t<mutex_t> lock(mutex);
				return hlist.size();
			}


			inline static void abort(handle_holder_t& hh)
			{
				if(hh.hevent) SetEvent(hh.hevent);
			}


			int stop_all(int tio=INFINITE)
			{
				HANDLE hs[MAXIMUM_WAIT_OBJECTS];
				DWORD ws;
				locker_t<mutex_t> lock(mutex);
				int c;
				//std::for_each(hlist.begin(),hlist.end(),abort);
				
                for(;;)
				{

				

				if((c=hlist.size())==0) return c;
				for(iterator_t i=hlist.begin();i!=hlist.end();++i)
					if((*i).hevent) SetEvent((*i).hevent);

				if(!tio) return c;

				c=0;
				for(iterator_t i=hlist.begin();i!=hlist.end();++i){
					hs[c]=(*i).get_hthread();
                  c++;  
				  if(c>MAXIMUM_WAIT_OBJECTS) break;
				}

				{
					unlocker_t<mutex_t> unlock(mutex);
					ws=::WaitForMultipleObjects(c,hs,true,tio);
					for(int k=0;k<c;k++) CloseHandle(hs[k]);
				}
				   
				   if(ws==WAIT_TIMEOUT)
					   return c=hlist.size();

				}
				
			}
			inline 	static apartment_list_t& get_instance()
			{
				return class_initializer_T<apartment_list_t>().get();
			}


		};


		inline static void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName)
		{

			struct{
				DWORD dwType; // must be 0x1000
				LPCSTR szName; // pointer to name (in user addr space)
				DWORD dwThreadID; // thread ID (-1=caller thread)
				DWORD dwFlags; // reserved for future use, must be zero
			} 	info={0x1000,szThreadName,dwThreadID,0};



			__try
			{
				RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (ULONG_PTR*)&info );
			}
			__except (EXCEPTION_CONTINUE_EXECUTION){}
		};





		template< class IniterFiniter=ipc_utils::COMInit_t , class IniterFiniter2=int>
		struct apartment_t: _non_copyable_
		{
			DWORD am,tid;
			kobj_holder_t hthread;
			event_local_t abort_event,init_event;
			void* context;
			mutex_cs_t mutex;
			IniterFiniter2 inifini2;

			struct item_t
			{
				void* pcontext;
				void (__cdecl *exiproc)(void* p);
				inline void clear()
				{
					try{
						if(exiproc) exiproc(pcontext);
					}catch(...){};
				}

			};

			typedef std::list<item_t> itemlist_t;


			int item_id; 
			itemlist_t items;

			//static int wait_loop(event_local_t& e)
			static int wait_loop(HANDLE hko,HWND hwnd=0)
			{
				//return ipc_utils::loop()||ipc_utils::wait_alertable(&e.hko);
				return loop(hwnd)||wait_alertable(&hko,1);
			}

			static int wait_loop_if(HANDLE hko,HWND hwnd)
			{
				//return ipc_utils::loop()||ipc_utils::wait_alertable(&e.hko);
				int f=0;
				(hwnd)&&(f=loop(hwnd));
				return f||wait_alertable(&hko,1);
			}



			static DWORD WINAPI s_tproc(void* p){

				__try{
					((apartment_t*)p)->proc();
				}
				__except(EXCEPTION_EXECUTE_HANDLER){}
				return 0;
			}

			inline void clear_list()
			{
				locker_t<mutex_cs_t> mutex;
				for(itemlist_t::iterator i=items.begin();i!=items.end();++i)
					i->clear();
			}

			ULONG_PTR ALS_set(void* context, void (__cdecl *exiproc)(void* ))
			{
				locker_t<mutex_cs_t> mutex;
				item_t it={context,exiproc};
				items.push_front(it);
				itemlist_t::iterator i=items.begin();
				return ULONG_PTR(&i);
			}
			void* ALS_get(ULONG_PTR l)
			{
				locker_t<mutex_cs_t> mutex;
				itemlist_t::iterator* pi=(itemlist_t::iterator*)l;
				return (*pi)->pcontext;
			}

			void ALS_clear(ULONG_PTR l)
			{
				locker_t<mutex_cs_t> mutex;
				itemlist_t::iterator* pi=(itemlist_t::iterator*)l;
				items.erase(*pi);

			}


			inline void proc()
			{

				//
				SetThreadName(tid,(am==COINIT_APARTMENTTHREADED)?"DICH:[STA]":"DICH:[MTA]");

				IniterFiniter initer(am);

				init_event.signal();
				__add_appartment_thread();

				apartment_list_t<>::iterator_t iregister;

				iregister=apartment_list_t<>::get_instance().push_handles(hthread.hko,abort_event.hko);

				for(;;)	  
					try {
						DWORD dw;
						if(wait_loop(abort_event))		    
						{  
							clear_list();
							//return;
							break;		       
						}

				} catch(...){};

             apartment_list_t<>::get_instance().pop_handles(iregister);
				__rem_appartment_thread();


			}



			apartment_t(DWORD _am=COINIT_APARTMENTTHREADED,void* _context=0):am(_am),context(_context)
			{

				if(hthread.hko=CreateThread(NULL,0,&s_tproc,(void*)this,0,&tid))
					init_event.lock();

				init_event.reset();

			}

			virtual	   ~apartment_t(){
				close();
				//hthread.lock();
				//
				HWND hwnd=get_com_hwnd();
				while(!wait_loop(hthread)){};
				//while(!wait_alertable(&hthread.hko,1));
			}



			inline void close(){
				abort_event.signal(); 
			};


			template< class T>
			bool   call_asyn(T* pt,bool ff=false){

				if(ff) return asyn_su::_pool<T,true>(pt).call_in_thread(hthread.hko);
				else return asyn_su::_pool<T,false>(pt).call_in_thread(hthread.hko);

			}    

			template<class RetType,class Arg,class Functor>
			RetType call_sync(Functor& fun,Arg& arg,RetType defV=RetType(),int flag_msg_loop=0)
			{
				struct asyn_helper_t
				{
					RetType result;
					Functor& fun;
					Arg& arg;

					event_local_t evnt;
					asyn_helper_t(Functor& _fun,Arg& _arg,RetType defV)
						:fun(_fun),arg(_arg),result(defV){
							evnt.signal(0);
					}
					void operator ()()
					{
						result=fun(arg);
						evnt.signal();

					}
					RetType wait(int f)
					{

						if(!f) evnt.lock();
						else while(!wait_loop(evnt)){};

						return result;
					}

				};

				asyn_helper_t* p=new asyn_helper_t(fun,arg,defV);
				struct closer_t
				{
					asyn_helper_t* p;
					~closer_t(){ delete p;}

				} _closer={p};


				if(call_asyn(p,false))
					return p->wait(flag_msg_loop);
				else return defV;

			}

			static void on_exit(  void* p)
			{
				delete (apartment_t*)p;
			}

			static HWND get_com_hwnd(DWORD tid=GetCurrentThreadId(),DWORD pid=GetCurrentProcessId())
			{		
				HWND hwF=0;
				char buf[256];
				HRESULT hr;


				while(hwF=FindWindowExA(HWND_MESSAGE,hwF,0,0))
				{

					DWORD id=0,t,i;
					t=GetWindowThreadProcessId(hwF,&id);

					if((t==tid)&&(id==pid))
					{
						hr=0;
						if(i=GetClassNameA(hwF,buf,256))
						{if(StrCmpNIA(buf,"OleMainThreadWndClass",256)==0)
						return hwF;					 
						}
						else hr=GetLastError();
					}							
				};
			};



		};










		struct deffered_t:IDispatch_impl_t<deffered_t>
		{


		};


		struct error_info_helper_t:IDispatch_impl_t<error_info_helper_t>
		{
			inline HRESULT set_context(void* pContext)
			{

				punk=(IUnknown*)pContext;
				return S_OK;
			};

			IUnknown* punk;
		};




		struct mta_singleton_protected_t
		{					 
			typedef apartment_t<> apartment_t;
			


			inline 	static smart_GIT_ptr_t<IDispatch>& _external_connector()
			{
				return  class_initializer_T<mta_singleton_protected_t>().get()._external_c;

			}

			inline 	static apartment_t& _apartment()
			{
				return class_initializer_T<mta_singleton_protected_t>().get().MTA();
			}
			inline static	long _add_apartment(int n)
			{
				mta_singleton_protected_t& m=class_initializer_T<mta_singleton_protected_t>().get();
                return InterlockedExchangeAdd(&m.app_ref_count,n);

			}
			inline void stopMTA()
			{
				locker_t<mutex_t> lock(mutex);
				if(p_mta) p_mta->close();

			};
			inline bool _stop_and_wait(int tio=INFINITE)
			{
				stopMTA();
				int t0=GetTickCount(),t;
				while(_add_apartment(0)>0)
				{

					loop();
					Sleep(100);
					t=int(GetTickCount())-t0;
					if((tio>0)&&(t>tio)) return false;
				}
				return true;
			}




			//protected:
			
			smart_GIT_ptr_t<IDispatch> _external_c;
			typedef mutex_cs_t mutex_t ;	
			mutex_t mutex;
			volatile LONG app_ref_count;
			
			apartment_t& MTA()
			{
				locker_t<mutex_t> lock(mutex);
				if(!p_mta)
				{
					p_mta= new apartment_t(COINIT_MULTITHREADED);
					SetThreadName(p_mta->tid,"DICH: MTA singleton");
				}
				return *p_mta;
			}			//:apartment_t<ipc_utils::COMInit_t>
			
			apartment_t* p_mta;
			mta_singleton_protected_t():p_mta(0){app_ref_count=0;}
			~mta_singleton_protected_t(){delete p_mta;}


		};

		struct mta_singleton_t: protected mta_singleton_protected_t
		{
			typedef mta_singleton_protected_t::apartment_t apartment_t;



			inline static	long add_apartment(int n)
			{
				return _add_apartment(n);
			}

			inline 	static	smart_GIT_ptr_t<IDispatch>& external_connector()
			{
				return _external_connector();
			}

			inline 	static aggregator_container_helper&	add_external_connector(aggregator_container_helper& container)
			{
				HRESULT hr;
				smart_GIT_ptr_t<IDispatch> & ec= external_connector();
				if(ec)
				{
					smart_ptr_t<IDispatch> ecd;
					if(SUCCEEDED(hr=ec.unwrap(ecd)))
						container.add_EC(ecd);
				}
				return container;
			}

			inline 	static apartment_t& apartment()
			{
				return _apartment();
			}

			inline static bool stop_and_wait(int tio=INFINITE)
			{

			  mta_singleton_protected_t& m=class_initializer_T<mta_singleton_protected_t>().get();
			  return m._stop_and_wait(tio);

			}
				
		};


        inline void __add_appartment_thread()
		{
			mta_singleton_t::add_apartment(1);

		}
		inline void __rem_appartment_thread()
		{
			mta_singleton_t::add_apartment(-1);

		}

	};//com_apartments
};//ipc_utils