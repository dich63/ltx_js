#pragma once
//websocket_handler.h

#include "ipc_utils.h"
#include "wchar_parsers.h"
//#include "singleton_utils.h"
#include "sha1.h"
#include "wbs_buffer.h"
#include "singleton_utils.h"
#include "rc_holder.h"
#include <list>
#include "callback_context_arguments.h"
#include "dummy_dispatch_aggregator.h"
//HasOverlappedIoCompleted

namespace wbs_handlers {

	

#pragma pack(push)
#pragma pack(1)

	typedef unsigned char uint8_t;
	typedef unsigned int uint32_t;
	typedef unsigned short uint16_t;
	typedef unsigned long long uint64_t;
	typedef  long long int64_t;

	struct wbs_frame_header_base_t
	{
		union{
			struct{
				uint8_t opcode:4;
				uint8_t rsv3:1;
				uint8_t rsv2:1;
				uint8_t rsv1:1;
				uint8_t fin:1;
			};
			struct{ uint8_t fields_low;};

		};
		union{
			struct{
				uint8_t len:7;
				uint8_t mask:1;
			};
			struct{ uint8_t fields_high;};
		};

	};

	struct wbs_frame_header_t: wbs_frame_header_base_t
	{
      uint8_t  inner_buf[12];


	};


#pragma pack(pop)


    template <class T, class Intf> 
	struct wbs_FMT_base_t
	{

	
		STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject){

			HRESULT hr;
			OLE_CHECK_PTR(ppvObject);
			if((__uuidof(Intf)==riid)||(__uuidof(IUnknown)==riid))
			{
                *ppvObject=static_cast<T*>(this);
				return S_OK;
			}			
			dda(this,riid,ppvObject,hr=E_NOINTERFACE);

			return hr;
			
		}
		virtual ULONG __stdcall AddRef(){
			return refcounter.AddRef(this); 
		};
		virtual ULONG __stdcall  Release(){
			return refcounter.Release(this); 
		};      


	  protected:
      RefCounterMT_t<1> refcounter;
	  DummyIDispatchAggregator_t<Intf> dda;

	};


   
    template<int VERS=0>
	struct wbs_recv_buffer_t:i_wbs_recv_buffer,wbs_FMT_base_t<wbs_recv_buffer_t<VERS>,i_wbs_recv_buffer>
	{
		

		//wbs_recv_buffer_t
		//is_socket

           

	};

	template<int VERS=0>
	struct wbs_handler_t:i_wbs_handler,wbs_FMT_base_t<wbs_handler_t<VERS>,i_wbs_handler>
	{
		
		HRESULT hr_s;
		
		shared_socket_data_t* psocket_data;
		ipc_utils::smart_ptr_t<i_wbs_recv_buffer> recv_buffer;		
		ipc_utils::smart_ptr_t<i_mbv_socket_context> socket_context;	
		event_local_t recv_event,recv_event_sync,send_event_sync;
		mutex_cs_t mutex,mutex_send;
		OVERLAPPED ovl_recv;
		wbs_frame_header_base_t frame_header;
		HANDLE hwo;


		inline OVERLAPPED* reset_ovl_recv()
		{
			ovl_recv=OVERLAPPED();
            ovl_recv.hEvent=recv_event.hko;
			return &ovl_recv;
		}
		inline OVERLAPPED* ror()
		{
			return reset_ovl_recv();
		}

		void __stdcall s_on_recv(void* ptr,BOOLEAN )
		{
            static_cast<wbs_handler_t*>(ptr)->on_recv();
		}
		wbs_handler_t():hr_s(E_POINTER),psocket_data(0),recv_event(false),recv_event_sync(false),send_event_sync(false),hwo(0){

			   locker_t<mutex_cs_t> lock(mutex);
			      bool f;				  
				  f=RegisterWaitForSingleObject(&hwo,recv_event.hko,&s_on_recv,this,INFINITE,WT_EXECUTEDEFAULT);
			      hr_s=f?S_OK:HRESULT_FROM_WIN32(GetLastError());
				  InitializeSRWLock
				};

		~wbs_handler_t(){
               locker_t<mutex_cs_t> lock(mutex);
			   UnregisterWait(hwo);
		}

		HRESULT init(VARIANT vrecv_buffer,VARIANT vsock){
			HRESULT &hr=hr_s;
            OLE_CHECK_hr(hr);
			OLE_CHECK_hr(recv_buffer.reset(vrecv_buffer));           
			OLE_CHECK_hr(socket_context.reset(vsock));          
			OLE_CHECK_hr(socket_context->GetContext((void**)&psocket_data));        
			return hr;
		}


		inline SOCKET sock()
		{
			return psocket_data->sock;
		}

		HRESULT post_recv_header(){

			locker_t<mutex_cs_t> lock(mutex);
			HRESULT hr;
			DWORD cb=-1;
			int i;
			WSABUF wsb={1,&frame_header};
			i=WSARecv(sock(),&wsb,1,&cb,0,reset_ovl_recv(),0);
			if(i==0)
			{
               FatalAppExitW(0,"־ילכÿ ~= ERROR_IO_PENDING");     
			}
			else {
				hr=WSAGetLastError();
				if(hr==ERROR_IO_PENDING)
					hr=S_OK;
			}

			return HRESULT_FROM_WIN32(hr);

		}

		bool close(HRESULT hreason=GetLastError())
		{

              return false;
		}

		bool read_sync(void* p,DWORD cb,DWORD* pcbout)
		{
			DWORD cbout=-1;
			int i;
			WSABUF wsb={1,p};
			OVERLAPPED ovl={};
			ovl.hEvent=recv_event_sync.hko;
			i=WSARecv(sock(),&wsb,cb,&cbout,0,&ovl,0);



		}

		bool read_frame_header()
		{
			DWORD nbt;
			bool f;
			f=GetOverlappedResult(sock(),&ovl_recv,&nbt,false);
			if(!(f&&(nbt!=1))) 
			  return  close();
			

		}
		void on_recv()
		{
			
			locker_t<mutex_cs_t> lock(mutex);
			//




			
			
		}



		HRESULT start()
		{

		}

		//wbs_recv_buffer_t
		//is_socket



	};






	inline HRESULT mm_alloc_buffer(int64_t len,ipc_utils::smart_ptr_t<i_mm_region_ptr>& region_ptr,bool fsparse=false)
	{
      HRESULT hr;
	  //s_parser_t<wchar_t> sp;
	  ipc_utils::bstr_fmt_t<> monstr(L"ltx.bind:mm_buffer:length=%g;type=uint8;flags.sparse=%d",double(len),int(fsparse));	  
	  //monstr.printf(L"ltx.bind:mm_buffer:length=%g;type=uint8;flags.sparse=%d",double(len),int(fsparse));
	  hr=::CoGetObject(monstr,0,__uuidof(i_mbv_buffer_ptr),region_ptr._ppQI());
	  return hr;
	}


	struct global_counter_t//:i_holder_t<global_counter_t>
	{

		global_counter_t():refcount(0){};
        event_local_t evnt;
		volatile long refcount;

		long connect(){ 
            evnt.signal(0);
			return _InterlockedIncrement(&refcount); 
		}

		long disconnect(){ 
			
			long l= _InterlockedDecrement(&refcount); 
			if(l<=0) evnt.signal(1);
			return l;

		}
           
	};

	struct i_asio
	{
      virtual HRESULT begin_recv(OVERLAPPED* ovl)=0;
	  virtual HRESULT recv(void* pbuf,int cb,int* pcbop)=0;	  
	  virtual HRESULT send(void* pbuf,int cb,int* pcbop)=0;	  	  
	  virtual HRESULT close()=0;	  	  

	};


   template <class Getter,class Setter>
	struct wbs_frame_t
	{
		typedef Getter getter_t;
		typedef Setter setter_t;
		setter_t* sender;
		getter_t* recver;
		event_local_t recv_event;

		wbs_frame_t(const getter_t* pg,const setter_t* ps):recver(pg),sender(ps),recv_event(false)
		{	

		}



	};


/*
	struct sock_io_t
	{
		typedef HRESULT (* io_proc_t)(void* context,int cb,void* p,OVERLAPPED* ovl=0);
		typedef HRESULT (* io_close_t)(void* context);
		io_proc_t read;
		io_proc_t write;
	};

*/
	struct recv_ovl_t
	{
		OVERLAPPED ovl;
		wbs_frame_base_t header;
		char ctrlbuf[128];
		event_local_t recv_event;
		SOCKET sock;
        
		recv_ovl_t(SOCKET s)
			:ovl(),header(),sock(s)	{
			ovl.hEvent=recv_event;
		}



	};




	struct websocket_handler_t{


		struct releaser_t{
			IUnknown* unk ;
			releaser_t(IUnknown* p):unk(p){};
			~releaser_t(){
				unk&&unk->Release();
			}
		};

		struct wbs_msg_t{

			typedef std::list<wbs_msg_t> msg_list_t;
		    wbs_frame_base_t header;
			ipc_utils::smart_ptr_t<i_mm_region_ptr> buffer;
			std::vector<char> small_buffer;
			msg_list_t* owner;
			wbs_msg_t(msg_list_t* o=0):owner(o),header(){};

           };

		typedef wbs_msg_t::msg_list_t msg_list_t;

			msg_list_t recv_list;


		 static 	void CALLBACK s_frame_begin(i_holder_ptr_t<recv_ovl_t>* ovl,BOOLEAN)
		 {
			 try
			 {
				 releaser_t rlsr(ovl);


			 }
			 catch (...)
			 {		 	
			 }			 
			  
		 }



		
		//


	};
}; //namespace wbs_handlers