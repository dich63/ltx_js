#pragma once


#include "video/singleton_utils.h"
#include "wchar_parsers.h"
#include "heap_allocator.h"
#include "istream_overlapped.h"
#include <list>
#include <string>


//template <class number_T,class vector_T=number_T,class matrix_T=number_T,template<class,class,class> class OperationT=overloaded_matrix_op_t >

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

 inline HRESULT seterror(bool f)
 {
 	 ULONG WINAPI (*RtlNtStatusToDosError)( NTSTATUS Status );
	 HRESULT hr=S_OK;
	 if(!f) 
	 {

        hr=GetLastError();
	  if(SET_PROC_ADDRESS(GetModuleHandleA("NTdll.dll",RtlNtStatusToDosError)))
		  hr=RtlNtStatusToDosError(hr);
	 }
     return hr; 
	  
 }
 io_binder_t(){}
	  
  HRESULT bind(Ovl* povl)
  {	  
	  HRESULT hr;
      OVERLAPPED& check=*povl;
	  bool f=BindIoCompletionCallback(HANDLE(povl->fd()),LPOVERLAPPED_COMPLETION_ROUTINE(&s_iocr),0);
	  return hr=seterror(f);
  }
  inline operator bool()
  {
          return hr==S_OK;
  }
  inline operator HRESULT()
  {
	  return hr;
  }

};


template <class Ovl>
struct io_binder2_t
{
  int ii;
};

template <template<class> class binder_t=io_binder2_t >
struct pipe_port_t2
{
  binder_t<DWORD> binder;
};


template <template<class> class binder_t=io_binder_t>
struct pipe_port_t
{
	typedef void (__stdcall *on_connect_callback_t)(void* pcontext,ISequentialStream* pstream); 
	enum
	{
		pipe_buf_size=4096
	};
	typedef mutex_cs_t mutex_t;
	//typedef _BINDER  binder_t;
	
	//typedef _HEAP  heap_t;

template <class N>
struct   smart_ptr_t
{

	N* p;
	smart_ptr_t(N* _p=0,bool faddref=1):p(_p)
	{
        if(p) p->AddRef(); 
	}
	~smart_ptr_t()
	{
		reset();
	}
	smart_ptr_t(const smart_ptr_t& sp)
	{
        reset(sp);
	}

smart_ptr_t& operator =(const smart_ptr_t& sp)
{
	return reset(sp);
}

inline smart_ptr_t& reset(N* np=0)
{
	if(np!=p)
	{
	   N* t=make_detach(p,np);
	   if(p) p->AddRef();
	   if(t) t->Release();
	}
	return *this;
}
inline  N* operator->()
 {
	return p;
 }
inline   operator N*()
{
	return p;
}

};

struct   monitor_t
{

	typedef typename std::list<HANDLE> handlelist_t;

   

	handlelist_t handlelist;
	std::wstring pipename;
	mutex_t mutex;
	bool fremoted;
	bool factive;



  static SECURITY_ATTRIBUTES* security_attributes(bool fshared)
  {


	  if(fshared)
	  {
		  static struct SDS 
		  {
			  SECURITY_DESCRIPTOR sd;   
			  SDS()
			  {
				  InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
				  SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
			  }

		  } sds;

		  static	SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),&sds.sd,false}; 

		  return &sa;

	  } 
	  else return NULL ;
  }

   struct connection_t:OVERLAPPED
   {
	  StreamOverlapped stream;
      HANDLE hPipe;
	  smart_ptr_t<monitor_t> monitor;
	  handlelist_t::iterator ihandle; 
	  connection_t* pnext;
	  HRESULT hr;
	  long state;
	  char buf[256];
	  DWORD size;

	  inline HANDLE fd()
	  {
		  return hPipe;
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

static	  void close()
	  {

		   HANDLE he=maskevent(make_detach(hEvent,INVALID_HANDLE_VALUE),0);
		   locker_t<mutex_t> lock(monitor->mutex);
	   

		   SetEvent(he);
		   close_handle(he);
		   HANDLE h=*ihandle;
		   if(state>=0) 
			   monitor->handlelist.erase(ihandle);
		   close_handle(h);
		   state=-1;
	  }

	  static HANDLE create_named_pipe(const wchar_t* wsPipename,bool fshared=0,int PIPE_TIMEOUT=1000)
	  {
		  return CreateNamedPipeW( 
			  wsPipename,            // pipe name 
			  PIPE_ACCESS_DUPLEX |     // read/write access 
			  FILE_FLAG_OVERLAPPED,    // overlapped mode 
			  PIPE_TYPE_MESSAGE |      // message-type pipe 
			  PIPE_READMODE_MESSAGE |  // message-read mode 
			  PIPE_WAIT,               // blocking mode 
			  PIPE_UNLIMITED_INSTANCES,               // number of instances 
			  pipe_buf_size,   // output buffer size 
			  pipe_buf_size,   // input buffer size 
			  PIPE_TIMEOUT,            // client time-out 
			  security_attributes(fshared));                 
	  }


	  connection_t(monitor_t* m):monitor(m)
	  {
		  bool f;
		 OVERLAPPED b={};
		 state=0;
		 //pbuf=0;
		 size=0;
         OVERLAPPED* t=(OVERLAPPED*)this;
		 *t=b;
		 hPipe=create_named_pipe(monitor->pipename.c_str(),monitor->fremoted);
		 hr=(hPipe)?S_OK:GetLastError();
		 if(hr) return;
		 stream=StreamOverlapped(this);
		 

		 locker_t<mutex_t> lock(monitor->mutex);
		 
		 if(monitor->factive) 
		 {
          hEvent=CreateEvent(0,1,0,0);   
		  ihandle=monitor->handlelist.insert(monitor->handlelist.end(),hEvent);
		  hr=monitor->binder.bind(fd(),this);
		  if(hr) return;
		   f=ConnectNamedPipe(hPipe,this);
		  if(f) return E_FAIL;
		  hr=GetLastError();
		  if(hr==ERROR_PIPE_CONNECTED)
		  {
           state=2;
		   hr=S_OK;
		   f=ReadFile(hPipe,dummy<DWORD>(),0,dummy<DWORD>(),this);
		   if((f)&&((hr=GetLastError())==ERROR_IO_PENDING))
		   {
			   return;
		   }
		   
		  }
           if(hr==ERROR_IO_PENDING) state=1;
		 }
		 else 
		 {
			 CloseHandle(hPipe);
			 hr=E_FAIL;
		 }
		// f=monitor->binder.bind(this);
		 //if(!f) Release();

      }



      ~connection_t()
	  {
         
		   close();

         //SetEvent(h);
		 //CloseHandle(hPipe);
		 //CloseHandle(h);

	  }
   };



inline void iocompletion()
{
	on_connect_callback_t on_connect_callback;
	on_connect_callback=monitor->on_connect_callback;
/*
	bool factive;
	{
		locker_t<mutex_t> lock(monitor->mutex);
		
		factive=monitor->factive;		
	}
	
	if(factive&&(on_connect_callback))
	{
	   	 

      
	}
*/

	 delete this;

}

inline void operator()()
{
   iocompletion();
}

inline void operator()(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered)
{

	if(dwErrorCode==0)
	{
		monitor->new_connect();  
		if(asyn_call(this)) 
			return;
	}
	delete this;  
}

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
inline  long	Release()
  {
      locker_t<mutex_t> lock(mutex);
	  return inner_release();
  }
template <class CH>
  static monitor_t* create(const CH* pipename,int cp=CP_UTF8)
  { return new monitor_t(char_mutator<CH>(pipename,0,0,cp));};



  void abort()
  {
	  locker_t<mutex_t> lock(mutex);
	  factive=false;
	  for(handlelist_t::iterator i=handlelist.begin();i!=handlelist.end();++i)
	  {
		  HANDLE h=*i;
		  if(h!=INVALID_HANDLE_VALUE)
		  {
	  	  FlushFileBuffers(*i);
  		  DisconnectNamedPipe(*i);
		  }
	  }

	  
  }


HRESULT new_connect()
{
 connection_t* p=connection_t(this);
 HRESULT hr=p->hr;
 //if(hr==ERROR_IO_PENDING)  return 0;
 //if(hr==ERROR_PIPE_CONNECTED)  return p;
 if((hr)&&(hr!=ERROR_IO_PENDING)) delete p;
  return hr;
 };

on_connect_callback_t* set_connect_callback(on_connect_callback_t* new_connect_callback=0,void* _pcontext=0)
{
      locker_t<mutex_t> lock(mutex);
	  pcontext=_pcontext;
	  return make_detach(on_connect_callback,new_connect_callback);
}
  
  private:

	  long ref_count;

      on_connect_callback_t on_connect_callback;	  
	  void *pcontext;

	  monitor_t(wchar_t* pn)
		  :ref_count(1),pipename(pn),
		  fremoted(0),factive(true),
		  on_connect_callback(0),pcontext(0){};

	  ~monitor_t()
	  {
         abort();
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
  
  binder_t<connection_t> binder;


};

  smart_ptr_t<monitor_t> monitor;

  

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




 template <class CH,class SGBUF>
inline  static HRESULT send_recv(const CH* pipename,IN SGBUF* buf_in,OUT SGBUF* buf_out=0,int timeout=NMPWAIT_USE_DEFAULT_WAIT)
 {
	 HRESULT hr=S_OK;
	 bool f;
	 safeBUF<SGBUF> sbuf_in(buf_in);
	 safeBUF<SGBUF> sbuf_out(buf_out);
	 f=CallNamedPipeW(char_mutator<>(pipename),sbuf_in.buf(),sbuf_in.len(),sbuf_out.buf(),sbuf_out.len(),&sbuf_out.len(),timeout);
	 if(!f)
		 hr=GetLastError();
	 return hr;
 
 }
 
template <class CH>
HRESULT create (CH* pipename,on_connect_callback_t on_connect_callback,void* pcontext=0,int CXX=CP_UTF8)
{
	HRESULT hr;
	monitor->create(pipename,CXX)->set_connect_callback(on_connect_callback,pcontext);
	return hr=monitor->new_connect();
}


};