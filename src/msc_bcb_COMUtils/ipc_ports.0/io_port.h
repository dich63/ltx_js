#pragma once

#include <Winsock2.h>
#include <windows.h>




#include <stdlib.h>
#include "video/singleton_utils.h"
#include "wchar_parsers.h"
#include "heap_allocator.h"
#include "istream_overlapped.h"
#include "static_constructors_0.h"
#include <list>
#include <string>
#include <vector>

#include "ipc_utils.h"



//template <class number_T,class vector_T=number_T,class matrix_T=number_T,template<class,class,class> class OperationT=overloaded_matrix_op_t >


struct ipc_monitor_t:IUnknown
{
    virtual bool __stdcall is_active()=0;
	virtual void __stdcall abort(bool fbreak=false)=0;
} ;



struct ipc_connection_t
{
	virtual int __stdcall recv(void* buf,int len,int flags)=0;
	virtual int __stdcall send(void* buf,int len,int flags)=0;
	virtual bool __stdcall is_active()=0;
	virtual void __stdcall disconnect()=0;
	virtual void __stdcall abort(bool fbreak=false)=0;
	virtual HANDLE __stdcall fd()=0;
} ;


template <class Ovl>
struct io_binder_t
{


	static   void __stdcall s_iocr(	DWORD dwErrorCode,DWORD dwNumberOfBytesTransfered,OVERLAPPED* povl)
	{
		if(povl)
		{
            	Ovl& o=*(static_cast<Ovl*>(povl));
			  o(dwErrorCode,dwNumberOfBytesTransfered);

		}

	}

 static inline HRESULT seterror(bool f)
 {
 	 //ULONG  (WINAPI *RtlNtStatusToDosError)( NTSTATUS Status );
	 HRESULT hr=S_OK;
	 if(!f) 
	 {

        hr=GetLastError();
	      //if(SET_PROC_ADDRESS(GetModuleHandleA("NTdll.dll"),RtlNtStatusToDosError))
		  //hr=RtlNtStatusToDosError(hr);
	 }
     return hr; 
	  
 }
 io_binder_t(){}
	  
inline static HRESULT bind(Ovl* povl)
  {	  
	  HRESULT hr;
	  OVERLAPPED& check=*povl;
	  return hr=bind(HANDLE(povl->fd()));
  }

inline static HRESULT bind(HANDLE hf)
  {	  
	  HRESULT hr;
	   bool f=BindIoCompletionCallback(hf,LPOVERLAPPED_COMPLETION_ROUTINE(&s_iocr),0);
	  return hr=seterror(f);
  }


};

struct pipe_io_object
{
	HRESULT hr;
	HANDLE hpipe;
	typedef unsigned long ulong_t;
	
	struct opt_t
	{
		ulong_t PIPE_MODE;
		ulong_t maxinstance;
		ulong_t pipe_buf_size;
		ulong_t PIPE_TIMEOUT;
		ulong_t  fshared;

		opt_t(bool shared=0,
			ulong_t _PIPE_MODE=PIPE_TYPE_MESSAGE |      // message-type pipe 
			PIPE_READMODE_BYTE |  // message-read mode 
			PIPE_WAIT // blocking mode
			,ulong_t timeout=2000
			,ulong_t _maxinstance=PIPE_UNLIMITED_INSTANCES,ulong_t bufsize=4096
			)
			:fshared(shared),PIPE_TIMEOUT(timeout)
			,maxinstance(_maxinstance),pipe_buf_size(bufsize),PIPE_MODE(_PIPE_MODE){};
      
		template <class Args>
		opt_t& load( Args& args)
		{
			PSTRUCT_SET(args,this,PIPE_MODE);
			PSTRUCT_SET(args,this,maxinstance);
			PSTRUCT_SET(args,this,pipe_buf_size);
			PSTRUCT_SET(args,this,fshared);
			PSTRUCT_SET(args,this,PIPE_TIMEOUT);
			return *this;
		}
/*
		template <class Args>
		opt_t(Args& args)
		{
			new(this) opt_t;
				load(args);
		}
*/

	};

   struct url_resolver_t
   {
	   std::wstring url;
	   opt_t opts;
	   url_resolver_t(){}

	   
	   url_resolver_t(const char* purl,opt_t _opts=opt_t()):opts(_opts)
	   {
          if(purl) url=(wchar_t*)char_mutator<CP_UTF8>(purl);
	   }

	   url_resolver_t(const wchar_t* purl,opt_t _opts=opt_t())
		   :opts(_opts),url((purl?purl:L"")){}

	   
	   inline   HRESULT send_recv(IN void* buf_in,DWORD cbin, OUT void* buf_out=0,IN OUT DWORD* pcbout=0,int timeout=NMPWAIT_USE_DEFAULT_WAIT)
	   {
		    HRESULT hr=0;
		    bool f=CallNamedPipeW(url.c_str(),buf_in,cbin,buf_out,*pcbout,pcbout,timeout);
		   if(!f)
			   hr=GetLastError();
		   return hr;

	   };



   };

   inline HANDLE fd(){ return hpipe;}


   struct SDS_t 
   {
	   SECURITY_DESCRIPTOR sd;   
	   SECURITY_ATTRIBUTES _sa;
	   HRESULT hr;
	   SDS_t()
	   {
		   hr=S_OK;
		   bool f=InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)
			   &&SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
		   if(f)
		   {
			   SECURITY_ATTRIBUTES t={sizeof(SECURITY_ATTRIBUTES),&sd,false}; 
			   _sa=t;

		   }
		   else hr=GetLastError();

	   }

	   inline SECURITY_ATTRIBUTES* operator()(bool fremote)
	   {
		   return ((hr==S_OK)&&fremote)?&_sa:NULL;  
	   }
   };


    HRESULT create(const url_resolver_t& resolver)
	{


        SDS_t& sds=class_initializer_T<SDS_t>::get();

		bool fremoted=resolver.opts.fshared;
		if(fremoted&&(sds.hr))
			return hr=sds.hr;
			  
   
	    hpipe=CreateNamedPipeW( 
		   resolver.url.c_str(),
		   PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,
		   resolver.opts.PIPE_MODE,  // mode 
		   resolver.opts.maxinstance,               // number of instances 
		   resolver.opts.pipe_buf_size,   // output buffer size 
		   resolver.opts.pipe_buf_size,   // input buffer size 
		   resolver.opts.PIPE_TIMEOUT,            // client time-out 
		   sds(fremoted));
				
		return hr=(hpipe!=INVALID_HANDLE_VALUE)?S_OK:GetLastError();

	}


inline static	HANDLE _open_pipe(const wchar_t* pn)
{
	//SDS_t& sds=class_initializer_T<SDS_t>::get();
	HANDLE h = CreateFileW( 
		pn,   // pipe name 
		GENERIC_READ |  // read and write access 
		GENERIC_WRITE, 
		0,              // no sharing 
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe 
		FILE_FLAG_OVERLAPPED,              // default attributes 
		NULL);          // no template file 
	return h;
}

	//
HRESULT connect(const url_resolver_t& resolver,DWORD dwMode = PIPE_READMODE_BYTE,int wpt=2000)
//HRESULT connect(const url_resolver_t& resolver,DWORD dwMode = PIPE_READMODE_MESSAGE,int wpt=2000)
	{
		const wchar_t* pn=resolver.url.c_str();
		bool f;
		if(((hpipe=_open_pipe(pn))==INVALID_HANDLE_VALUE)&&(f=WaitNamedPipeW(pn,wpt)))
				hpipe=_open_pipe(pn);
		f=(hpipe!=INVALID_HANDLE_VALUE);
		
		; 
		if(f)
			 f = SetNamedPipeHandleState( 
			hpipe,    // pipe handle 
			&dwMode,  // new pipe mode 
			NULL,     // don't set maximum bytes 
			NULL);    // don't set maximum time 

		 return hr=(f)?S_OK:GetLastError();
	}


	bool accept(OVERLAPPED* ovl,long* pstate=0)
	{
		long lll;
		long &state=(pstate)?*pstate:lll;
		
		
		bool f;
           state=-1;
		if(hr) return false;                   
		   state=0;
		f=ConnectNamedPipe(hpipe,ovl);
		if(f) return E_FAIL;
		hr=GetLastError();
		if(hr==ERROR_PIPE_CONNECTED)
		{
			state=2;
			hr=S_OK;
			f=ReadFile(hpipe,dummy<DWORD>(),0,dummy<DWORD>(),ovl);
			if((f)||((hr=GetLastError())==ERROR_IO_PENDING))
							return true;
		}
		if(hr==ERROR_IO_PENDING) 
		{
			state=1;
			return true;
		}

		//if(hr)	close();

		
		return hr==S_OK;

	}
    
    void disconnect()
	{
		if(hpipe==(INVALID_HANDLE_VALUE)) return;
		 //
		 FlushFileBuffers(hpipe);
         DisconnectNamedPipe(hpipe); 
	}
	void close()
	{
		HRESULT hr;
		bool f;
            if(hpipe!=(INVALID_HANDLE_VALUE)) 
			{
				 f=FlushFileBuffers(hpipe);
				 hr=GetLastError();
				CloseHandle(make_detach(hpipe,INVALID_HANDLE_VALUE));
			}

			
			
	}

	pipe_io_object():hpipe(INVALID_HANDLE_VALUE),hr(E_POINTER)	{};
	~pipe_io_object()
	{
		close();
	}
  
};


template <class IO_Object=pipe_io_object, template<class> class binder_t=io_binder_t>
struct io_port_t
{
	//typedef void (__stdcall *on_connect_callback_t)(void* pcontext,ISequentialStream* pstream,void* pconnection); 
	typedef void (*on_connect_callback_t)(void* pcontext,ISequentialStream* pstream,void* pconnection); 
	
	typedef mutex_cs_t mutex_t;
    typedef typename IO_Object io_object_t;
	typedef typename io_object_t::url_resolver_t url_resolver_t;
	typedef typename io_object_t::opt_t opt_t;
   

	//typedef _BINDER  binder_t;
	
	//typedef _HEAP  heap_t;


struct   monitor_t
{

  typedef typename std::list<void*> connection_list_t;

  connection_list_t connection_list;

  url_resolver_t resolver;

  mutex_t mutex;
  bool fremoted;
  bool factive;
  volatile long active_connection;
  event_local_t event_closing;
  

  inline HRESULT get_waitable_event(HANDLE* pevent)
  {
	  HRESULT hr;
	  if(!pevent) return E_POINTER;
	  HANDLE hp=GetCurrentProcess();
      if(!DuplicateHandle(hp,event_closing,hp,pevent,SYNCHRONIZE,0,0))
		  return hr=GetLastError();
	  return S_OK;
  }

   
  template <class D>
   struct overlapped_t:OVERLAPPED
   {
	   typedef typename StreamOverlapped<D> stream_t;
	   typedef typename overlapped_t<D> base_t;



   	  volatile LONG ref_count;

	    io_object_t io_object;
		mutex_t mutex,mutex_io;

		inline void lock()  { mutex_io.lock();}
		inline void unlock() { mutex_io.unlock();}


	   stream_t stream;
	   HRESULT hr;
	   long state;
	   inline ULONG  AddRef(void) 
	   { 

		   return InterlockedIncrement(&ref_count); 
	   }

	   inline ULONG  Release(void) 
	   { 
		   ULONG l=InterlockedDecrement(&ref_count); 
		   if(l==0)
		   {
			   D* _this=static_cast<D*>(this);
			   delete _this; 
		   }
		   return l;
	   }


	   inline HANDLE fd()
	   {
		   return io_object.fd();
	   }
	   inline static HANDLE maskevent(HANDLE hEvent,bool f=true)
	   {
		   ULONG_PTR u=(f)? ULONG_PTR(hEvent)|1:ULONG_PTR(hEvent)&(~ULONG_PTR(1));
		   return HANDLE(u);
	   }

	   inline static bool close_handle(HANDLE h)
	   {
		   // ULONG_PTR u=ULONG_PTR(h)&(~ULONG_PTR(1));
		   // h=HANDLE(u);  
		   if(GetHandleInformation(h,dummy<DWORD>()))
		   {
			   return CloseHandle(h);
		   }
		   else return false;
	   }


	   void close()
	   {

		   locker_t<mutex_t> lock(mutex);
		   if(state<0) return;
		   if(hEvent)
		   {
			   HANDLE he=maskevent(make_detach(hEvent),0);
			   //SetEvent(he);
			   close_handle(he);
		   }

		   io_object.close();


		   state=-1; 
	   };

	   overlapped_t():ref_count(1),state(0),hr(E_POINTER)
	   {
		   OVERLAPPED b={};
		   OVERLAPPED* t=static_cast<OVERLAPPED*>(this);
		   *t=b;
		   hEvent=CreateEvent(0,1,0,0); 
		   mutex.set_spin_count(20000);
		   mutex_io.set_spin_count(20000);
	   };


	   ~overlapped_t()
		 {
			 	 close();
		 }

   };


   struct client_connection_t:overlapped_t<client_connection_t>
   {
        client_connection_t(const url_resolver_t& resolver)
		{
			hr= io_object.connect(resolver);
            stream=stream_t(this);
		}
   };



  
   struct connection_t:overlapped_t<connection_t>
   {
      





	  ipc_utils::smart_ptr_t<monitor_t> monitor;
	  connection_list_t::iterator iconnection; 




	  struct ipc_connection_impl_t:ipc_connection_t
	  {
          //i_connection_t(connection_t& )
		  virtual int __stdcall recv(void* buf,int len,int flags)
		  {
			  connection_t::stream_t&  stream=owner()->stream;
			  HRESULT hr;
			  DWORD cbt;
			  if(flags&MSG_WAITALL)
			  {
				  char *p=(char *) buf;
				  DWORD cb=len,cbr;
				  cbt=0;
				  while(SUCCEEDED(hr=stream.Read(p,cb,&cbr)))
				  {
					  cbt+=cbr;
					  if(cbt>=DWORD(len)) break;
					  p+=cbr;
					  cb-=cbr;
				  }

			  }
			  else hr=stream.Read(buf,len,&cbt);
		  
			  if(SUCCEEDED(hr))				  return cbt;
			  else 	  SetLastError(hr);

				       return -1;
  		  
		  };
		  virtual int __stdcall send(void* buf,int len,int flags)
		  {

			  connection_t::stream_t&  stream=owner()->stream;
			  HRESULT hr;
			  DWORD cbw;
			  if(SUCCEEDED(hr=stream.Write(buf,len,&cbw)))
				  return cbw;
			  else 	  SetLastError(hr);
			  return -1;

		  }
		  virtual bool __stdcall is_active()
		  {
			  return owner()->monitor->is_active(); 
		  };
		  virtual void __stdcall disconnect()
		  {
              owner()->disconnect(); 
		  };

		  virtual void __stdcall abort(bool fbreak=false)
		  {
			  
			  owner()->monitor->abort(fbreak); 
		  };
		  virtual HANDLE __stdcall fd()
		  {
			  return owner()->fd(); 
		  };

		  inline connection_t* owner()
		  {
			  char *p=(char*)this;
			  return (connection_t*)(p-offsetof(connection_t,ipc_connection));
		  }



	  } ipc_connection;


inline   void close()
	  {

		  
		  {
  		    locker_t<mutex_t> lock(monitor->mutex);

			 InterlockedDecrement(&monitor->active_connection);

		   if(iconnection!=monitor->connection_list.end())
		    {
	  		   monitor->connection_list.erase(iconnection);
		      iconnection=monitor->connection_list.end();

		    }
		      
		      int ncc=monitor->can_closing();
		     //if((!monitor->factive)&&(monitor->connection_list.empty()))
			//	  monitor->event_closing();
                     
		  }
		  
		  //base_t::close();		  
	  }


ULONG release(){ return Release();};






	  connection_t(monitor_t* m):monitor(m)
	  {
		  bool f;

		 //ipc_connection.p=this;

		 iconnection=monitor->connection_list.end();
		 

		 state=0;
		 
                   
		 
		 hr= io_object.create(monitor->resolver);
		 
		 if(hr)
			 return;

		 stream=stream_t(this);
		 


		 locker_t<mutex_t> glock(monitor->mutex);
		 locker_t<mutex_t> lock(mutex);
		 
		 
		 if(monitor->factive) 
		 {

          //hEvent=CreateEvent(0,1,0,0);  

		  iconnection=monitor->connection_list.insert(monitor->connection_list.end(),this);
		  hr=monitor->binder.bind(this);
		  if(hr) return;
		  f= io_object.accept(this,&state);
		  hr=io_object.hr;
	   
		 }
	
      }


         ~connection_t()
		 {
			 
			 close();
		 };

void     disconnect()
	   {
		   locker_t<mutex_t> lock(mutex);
		    if(state<0) return;
            io_object.disconnect(); 
			state=-1;
	   }


  void io_completion()
{
	on_connect_callback_t on_connect_callback=monitor->on_connect_callback;
	if(on_connect_callback)
	{
		//#pragma check_stack(off) 
		//__try{
		   on_connect_callback(monitor->pcontext,&stream,(void*)&ipc_connection);
		   //RaiseException(0,0,0,0);

		//}__except(EXCEPTION_EXECUTE_HANDLER){	}
        //#pragma check_stack()  
		 //
	}
	 //delete this;
}

inline void operator()()
{
	try{
   io_completion();

	}catch(...){

	};
	Release();

}

inline void operator()(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered)
{

/*
       struct asyn_t
	   {
		   
		   ipc_utils::smart_ptr_t<connection_t> connection;
		   inline void operator()()
		   {
			   connection->io_completion();
		   };
		   
		   asyn_t(connection_t* p):connection(p,false){};

	   };
*/
     

	if(dwErrorCode==0)
	{
     
		InterlockedIncrement(&monitor->active_connection);

		monitor->new_connect();  

		locker_t<mutex_t> lock(mutex);
		state|=0x100;
		hEvent=maskevent(hEvent);
		//if(S_OK==asyn_call(this,1,false)) 
		//asyn_t* asyn=new asyn_t(this);
		//if(S_OK!=asyn_call(asyn)) 
		//
		//	  delete asyn;
		if(S_OK!=asyn_call(this,&connection_t::io_completion,&connection_t::release))
		{
			  locker_t<mutex_t> lock(monitor->mutex);
              shell_run(L"cmd.exe",L"/K mode con: cols=60 lines=10 &&@echo off&& color ce &&@echo ... &&@echo ... &&@echo ...System ThreadPool Corrupted!!!... &&@echo ...&&@echo ...");
			  ExitProcess(-1);
		}
			//FatalAppExitA(0,"System ThreadPool Corrupted!!!..."); 
			   
	}
	  //Release();  
}

   };




/*
static   VOID CALLBACK FileIOCompletionRoutine(
	   DWORD dwErrorCode,
	   DWORD dwNumberOfBytesTransfered,
	   connection_t* pconnection
	   )
   {
	   if(!pconnection) return;

	   if(dwErrorCode==0)
	   {
           pconnection->monitor->new_connect();  
		   if(0!=asyn_call(pconnection))
			    delete pconnection;
		   
		   
	   }
	   else     delete pconnection;  
   };
	
*/	




inline  long	AddRef()
	{
		locker_t<mutex_t> lock(mutex);
		return inner_addref();
	
	}
  long	Release()
  {
      locker_t<mutex_t> lock(mutex);
	  return inner_release();
  }
template <class CH>
  static monitor_t* create(const CH* url,const opt_t& opt=opt_t(),int cp=CP_UTF8)
  { 
	  return new monitor_t(char_mutator<>(url,0,0,cp),opt);
  };


  bool is_active()
  {
      locker_t<mutex_t> lock(mutex);
	  return factive;
  }

  unsigned long wait_on_close(unsigned long tio=INFINITE)
  {
        return event_closing.lock(tio);
  }

inline int  can_closing()
{
  locker_t<mutex_t> lock(mutex);
  long cc=InterlockedCompareExchange(&active_connection,0,0);
  if((!cc)&&(!factive))
	  event_closing();
   return cc;
}


inline  int abort(bool fbreak=true)
  {
	  {
	  
	  locker_t<mutex_t> lock(mutex);
	  factive=false;
	  if(fbreak)
	  {

	  
	  for(connection_list_t::iterator i=connection_list.begin();i!=connection_list.end();++i)
	  {
		  

		  connection_t* pc=(connection_t*)(*i);
		  if(pc) pc->disconnect();

		  
	  }

	  }

    
	  }

     return  can_closing(); 
  
  }


HRESULT new_connect()
{
 connection_t* p=new connection_t(this);
  //delete p;
 HRESULT hr=p->hr;
 //if(hr==ERROR_IO_PENDING)  return 0;
 //if(hr==ERROR_PIPE_CONNECTED)  return p;
 if((hr)&&(hr!=ERROR_IO_PENDING)) 
	 p->Release();
  return hr;
 };

on_connect_callback_t set_connect_callback(void* new_connect_callback=0,void* _pcontext=0)
{
      locker_t<mutex_t> lock(mutex);
	  abort(1);
	  wait_on_close();
	  factive=true;
	  event_closing.signal(0);
	  pcontext=_pcontext;
	  return make_detach(on_connect_callback,(on_connect_callback_t)new_connect_callback);
}
  typedef typename binder_t<connection_t>  io_binder_t;

  private:

	  long ref_count;

      on_connect_callback_t on_connect_callback;	  
	  void *pcontext;

	  monitor_t(wchar_t* url,const opt_t& opts=opt_t())
		  :ref_count(1),resolver(url,opts),
		  factive(true),
		  on_connect_callback(0),pcontext(0),active_connection(0){};



	  ~monitor_t()
	  {
         abort(false);
		 event_closing.lock();		 
		 
	  }

	  inline  long	inner_addref()
	  {
		  return ++ref_count;
	  }

	  inline  long	inner_release()
	  {
		  long l=--ref_count;
		  if(l==0)
			  delete this;
		  return l;
	  }
  
  
  io_binder_t binder;


};

  typedef typename monitor_t::io_binder_t  io_binder_t;

  ipc_utils::smart_ptr_t<monitor_t> monitor;
  opt_t options;
  HRESULT hr;

//  io_port_t(const opt_t& _options=opt_t()):options(_options){};
    io_port_t( opt_t _options=opt_t()):options(_options),hr(E_POINTER){};



 inline  void abort()
  {
	  monitor->abort();
  }

 struct buf_t:WSABUF
 {
	 buf_t(void* p=0,int _len=0)
		 {
			 buf=(char*)p;
			 len=_len;
	     };
	 template<class VECTOR>
 	 buf_t(const VECTOR& v)
	 {
		 len=v.size()*sizeof(VECTOR::value_type);
		 buf=(len)?(char*)&v[0]:(char*)&len;
	 };
		
 };

 struct buf_out_t:buf_t
 {
	 std::vector<char> v;
	 buf_out_t(int _len):buf_t(0,_len),v(_len)
	 {
		 if(len) buf=&v[0];
	 }

	 int size(){return v.size();};
 };
  
 template <class SGBUF>
 struct safeBUF
 {
	 char *pbuf;
	 DWORD* plen;
	 DWORD  l;
	 DWORD  t;

	 safeBUF(SGBUF* p)
	 {
		 if(p)
		 {
           pbuf=(char *)p->buf;
		   plen=(DWORD*)&p->len;
		 }
		 else
		 {
			 pbuf=(char *)&t;
			 plen=&(l=0);
		 }
		 
	 }
    inline char * buf()
	{
		return pbuf;
	}
	inline DWORD& len()
	{
		return *plen;
	}

 };

 


 template <class CH,class SGBUF,class SGBUF2>
inline  static HRESULT send_recv(const CH* pipename,IN SGBUF* buf_in,OUT SGBUF2* buf_out,int timeout=NMPWAIT_USE_DEFAULT_WAIT)
 {
	 HRESULT hr=S_OK;
	 bool f;
	 safeBUF<SGBUF> sbuf_in(buf_in);
	 safeBUF<SGBUF2> sbuf_out(buf_out);
/*
	 f=CallNamedPipeW(char_mutator<>(pipename),sbuf_in.buf(),sbuf_in.len(),sbuf_out.buf(),sbuf_out.len(),&sbuf_out.len(),timeout);
	 if(!f)
		 hr=GetLastError();
*/
	 io_object_t::url_resolver_t rs(pipename);
	 hr=rs.send_recv(sbuf_in.buf(),sbuf_in.len(),sbuf_out.buf(),&sbuf_out.len(),timeout);
	 return hr;
 
 }

template <class SGBUF,class SGBUF2>
inline HRESULT send_recv(IN SGBUF* buf_in,OUT SGBUF2* buf_out,int timeout=NMPWAIT_USE_DEFAULT_WAIT)
 {
	 HRESULT hr;
 
	 return hr=send_recv(monitor->resolver.url.c_str(),buf_in,buf_out,timeout);
 
 }

//hr= io_object.create(monitor->resolver); 

template <class CH,class Intf>
static HRESULT connect(const CH* url,Intf** ppStream,int cp=CP_UTF8)
{
	typedef monitor_t::client_connection_t client_t;
    HRESULT hr; 
	ipc_utils::smart_ptr_t<client_t> connection(new client_t(url_resolver_t(url)),false);
	 		//(url_resolver_t(url),false);
	
	hr=connection->hr;
	if(!hr)
		hr=connection->stream.QueryInterface(__uuidof(Intf),(void**)ppStream);
//	resolver
   return HRESULT_FROM_WIN32(hr);
}


template <class CH>
HRESULT create (const CH* url,void* on_connect_callback,void* pcontext=0,int cp=CP_UTF8)
{
	return create(url,cp).listen(on_connect_callback,pcontext);
}

template <class CH>
io_port_t& create (const CH* url,int cp=CP_UTF8)
{
	monitor.reset(monitor_t::create(url,options,cp),false);
	return *this;
}

io_port_t& listen(void* on_connect_callback,void* pcontext=0)
{
	
	hr=E_POINTER;
	if(monitor)
	{
		
	


	monitor->set_connect_callback(on_connect_callback,pcontext);
	hr=monitor->new_connect();
	bool f=(hr==ERROR_IO_PENDING)||(hr==S_OK);
	if(!f)
	{
		monitor->event_closing.signal(1);
     	hr=HRESULT_FROM_WIN32(hr);
	}
	
	}
   return *this;
}

inline operator HRESULT() {return hr;}

unsigned long wait_shutdown(int timeout=-1)
{
	return monitor->wait_on_close(timeout);
}

int shutdown(bool force=false)
{
	return monitor->abort(force);
}


};





