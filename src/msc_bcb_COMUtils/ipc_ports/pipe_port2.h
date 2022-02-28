#pragma  once

#include <Winsock2.h>
#include <windows.h> 
#include <process.h> 
#include <stdio.h>
#include "char_parsers.h"
#include <comdef.h>
#include <vector>
#include <rpc.h>
//#include "io_port.h"

#include <tchar.h>
#include <strsafe.h>

#include "hss_port_types.h"

//#include <heap_allocator.h>

#pragma comment(lib, "Rpcrt4")
/*
#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 

#define INSTANCES 4 
#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096
*/


//PIPE_UNLIMITED_INSTANCES

template <bool fremoted=false,int INSTANCES=2,size_t _BUFSIZE=hss_port_types::HANDSHAKE_DATA_t::BUFSIZE,int hadd=1,class IniterFiniter=void*>
struct pipe_port
{
  
enum
{
 BUFSIZE=_BUFSIZE,
 CONNECTING_STATE=0 ,
 READING_STATE=1 ,
 WRITING_STATE=2 ,
 HANDLES_FIRST=INSTANCES+2,
 HANDLES_NUM=HANDLES_FIRST+hadd
 
};

typedef typename pipe_port<fremoted,INSTANCES,BUFSIZE,hadd,IniterFiniter> this_type;

typedef struct 
{ 
	OVERLAPPED oOverlap; 
	HANDLE hPipeInst; 
	DWORD cbRead;
	DWORD cbToWrite; 
	DWORD dwState; 
	BOOL fPendingIO; 
	char chRequest[BUFSIZE];
	char chReply[BUFSIZE];
} PIPEINST, *LPPIPEINST; 

HRESULT hr;
IniterFiniter inifini;
PIPEINST Pipe[INSTANCES]; 
HANDLE hEvents[HANDLES_NUM]; 
unsigned long id;
void* pcontext;

inline HANDLE& handles(int n)
{
	return hEvents[HANDLES_FIRST+n];
}

static SECURITY_ATTRIBUTES* security_attributes()
{
 

  if(fremoted)
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

 };

      return NULL;
}

~pipe_port()
{
	DWORD dw;
	
	for(int n=0;n<INSTANCES;n++)
	{
		if(Pipe[n].hPipeInst) 
		{
			if(GetHandleInformation(Pipe[n].hPipeInst,&dw)) 
			{
				CancelIo(Pipe[n].hPipeInst);
				CloseHandle(Pipe[n].hPipeInst);
			}
			
		}
	}
	SetEvent( hEvents[INSTANCES]);
//
	if(on_send_recv_) on_send_recv_(pcontext,++id,0,0,0,0,hEvents[INSTANCES]);
	
	for(int n=0;n<HANDLES_NUM;n++) 
	{
		if(hEvents[n])
		if(GetHandleInformation(hEvents[n],&dw))
		  CloseHandle(hEvents[n]);
	}
}
void stop()
{
 SetEvent( hEvents[INSTANCES]);
}


static void shutdown(HANDLE h)
{
	
	SetEvent(h);
	CloseHandle(h);
	
}

template <class P>
static void safe_shutdown(P pair)
{
if((pair.first)&&(pair.second))
{
	HANDLE ht=OpenThread(SYNCHRONIZE,false,pair.second);
	if(!ht) return;
	shutdown(pair.first);
	while(WAIT_IO_COMPLETION==WaitForSingleObjectEx(ht,INFINITE,true)){};
		CloseHandle(ht);
}	
}


void log_printf(char* fmt,...)
{

}

typedef int (cdecl * on_send_recv_t)(void* pcontext,unsigned long id,char* bufin,int cbr,char* bufout,int* pcbw,HANDLE hAbortEvent);

on_send_recv_t on_send_recv_;

inline static int send_recv(const wchar_t* wsPipename,void* bufin,int cbr,void* bufout=0,int* pcbw=0,int to=5000)
{
      if(cbr>BUFSIZE) return -1;
	  DWORD cbw=0;
	  if(bufout)
	  cbw=(pcbw)?(DWORD)*pcbw:(DWORD)BUFSIZE;
	  else bufout=&cbw; 
	  pcbw=(pcbw)?pcbw:(int*)&cbw;
      if(cbw>BUFSIZE) return -1;
	  bool f;
	  f=CallNamedPipeW(wsPipename,bufin,cbr,bufout,cbw,(DWORD*)pcbw,to); 
		  return (f)?0:HRESULT_FROM_WIN32(GetLastError());
}

inline static int send_recv_IO(const char* url,void* bufin,int cbr,void* bufout=0,int* pcbw=0,int to=0)
{
	HRESULT hr;
	struct  io_pipe_port_t:io_port_t<pipe_io_object,io_binder_t>{};
	typedef ISequentialStream i_kox_stream;
	struct  kox_holder
	{
        i_kox_stream* stream;
		kox_holder():stream(0){}
		~kox_holder(){ if(stream) stream->Release();}

	} holder;
	
	if(FAILED(hr=io_pipe_port_t::connect(url,&holder.stream))) 
		 return hr;
    i_kox_stream* stream=holder.stream; 
	 ULONG ct,cbw;
	 if(bufout)
		 cbw=(pcbw)?(DWORD)*pcbw:(DWORD)BUFSIZE;
	 if(FAILED(hr=stream->Write(bufin,cbr,&ct))) 
		 return hr;
	 if(FAILED(hr=stream->Read(bufout,cbw,(pcbw)?(ULONG*)pcbw:&ct))) 
		 return hr;


}
inline static int send_recv(const char* sPipename,void* bufin,int cbr,void* bufout=0,int* pcbw=0,int to=0)
{
	HRESULT hr;
	if(cbr>BUFSIZE) return -1;
	DWORD cbw=0;
	if(bufout)
		cbw=(pcbw)?(DWORD)*pcbw:(DWORD)BUFSIZE;
	if(cbw>BUFSIZE) return -1;
	bool f;

   //
	return hr=send_recv_IO(sPipename,bufin,cbr, bufout,pcbw,to);

	f=CallNamedPipeA(sPipename,bufin,cbr,bufout,cbw,(DWORD*)pcbw,to); 
	return (f)?0:HRESULT_FROM_WIN32(GetLastError());
}

static HANDLE create_named_pipe(const wchar_t* wsPipename,int PIPE_TIMEOUT)
{
  return CreateNamedPipeW( 
		wsPipename,            // pipe name 
		PIPE_ACCESS_DUPLEX |     // read/write access 
		FILE_FLAG_OVERLAPPED,    // overlapped mode 
		PIPE_TYPE_MESSAGE |      // message-type pipe 
		PIPE_READMODE_MESSAGE |  // message-read mode 
		PIPE_WAIT,               // blocking mode 
		INSTANCES,               // number of instances 
		BUFSIZE,   // output buffer size 
		BUFSIZE,   // input buffer size 
		PIPE_TIMEOUT,            // client time-out 
		security_attributes());                 
}
static HANDLE create_named_pipe(const char* sPipename,int PIPE_TIMEOUT)
{
  return CreateNamedPipeA( 
		sPipename,            // pipe name 
		PIPE_ACCESS_DUPLEX |     // read/write access 
		FILE_FLAG_OVERLAPPED,    // overlapped mode 
		PIPE_TYPE_MESSAGE |      // message-type pipe 
		PIPE_READMODE_MESSAGE |  // message-read mode 
		PIPE_WAIT,               // blocking mode 
		INSTANCES,               // number of instances 
		BUFSIZE,   // output buffer size 
		BUFSIZE,   // input buffer size 
		PIPE_TIMEOUT,            // client time-out 
		security_attributes());                 
}

template <class CH>
pipe_port(const CH* tsPipename,int PIPE_TIMEOUT=5000,int pid=GetCurrentProcessId()):on_send_recv_(0),id(0),pcontext(0),hr(0)
{
	
     bool frr=fremoted;
	 memset(Pipe,0,sizeof(PIPEINST)*INSTANCES);
	 memset(hEvents,0,sizeof(HANDLE)*(HANDLES_NUM));

     hEvents[INSTANCES] = CreateEvent(NULL,TRUE,FALSE,NULL);
	 if(hEvents[INSTANCES]==0)
	 {
		 hr=HRESULT_FROM_WIN32(GetLastError());
		 return;
	 }

	 hEvents[INSTANCES+1] =OpenProcess(SYNCHRONIZE,0,pid);
	 if(hEvents[INSTANCES+1]==0)
	 {
		 hr=HRESULT_FROM_WIN32(GetLastError());
		 return;
	 }

   
           
	for (int i = 0; i < INSTANCES; i++) 
	{ 

		// Create an event object for this instance. 

		hEvents[i] = CreateEvent(NULL,TRUE,TRUE,NULL);

		if (hEvents[i] == NULL) 
		{
			hr=HRESULT_FROM_WIN32(GetLastError());
			log_printf("CreateEvent failed with %d.\n", GetLastError()); 
			stop();
			return ;
		}

		HRESULT hrr0;
		/*
		if(0)
		{
			BOOL ff=WaitNamedPipeA(tsPipename,10);
			hrr0=GetLastError();
		}
		*/
		Pipe[i].oOverlap.hEvent = hEvents[i]; 

		Pipe[i].hPipeInst =create_named_pipe(tsPipename,PIPE_TIMEOUT);
			

		if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE) 
		{
			hr=HRESULT_FROM_WIN32(GetLastError());
			log_printf("CreateNamedPipe failed with %d.\n", GetLastError());
			stop();
			return ;
		}

		

		// Call the subroutine to connect to the new client

		Pipe[i].fPendingIO = ConnectToNewClient( 
			Pipe[i].hPipeInst, 
			&Pipe[i].oOverlap); 

		Pipe[i].dwState = Pipe[i].fPendingIO ? 
        CONNECTING_STATE : // still connecting 
		READING_STATE;     // ready to read 
	} 

}

int start(on_send_recv_t on_send_recv)
{
  on_send_recv_=on_send_recv;
  return start();
}
int start()
{

	DWORD i, dwWait, cbRet, dwErr; 
	BOOL fSuccess; 
    
	while (1) 
	{ 
		// Wait for the event object to be signaled, indicating 
		// completion of an overlapped read, write, or 
		// connect operation. 

		dwWait = WaitForMultipleObjects( 
			INSTANCES+2,    // number of event objects 
			hEvents,      // array of event objects 
			FALSE,        // does not wait for all 
			INFINITE);    // waits indefinitely 

		// dwWait shows which pipe completed the operation. 


		i = dwWait - WAIT_OBJECT_0;  // determines which pipe 
		if(i==INSTANCES) return 1;
		if(i==(INSTANCES+1)) return 2;
		if (i < 0 || i > (INSTANCES - 1)) 
		{
			log_printf("Index out of range.\n"); 
			return 0;
		}

		// Get the result if the operation was pending. 

		if (Pipe[i].fPendingIO) 
		{ 
			fSuccess = GetOverlappedResult( 
				Pipe[i].hPipeInst, // handle to pipe 
				&Pipe[i].oOverlap, // OVERLAPPED structure 
				&cbRet,            // bytes transferred 
				FALSE);            // do not wait 

			switch (Pipe[i].dwState) 
			{ 
				// Pending connect operation 
			case CONNECTING_STATE: 
				if (! fSuccess) 
				{
					log_printf("Error %d.\n", GetLastError()); 
					return 0;
				}
				Pipe[i].dwState = READING_STATE; 
				break; 

				// Pending read operation 
			case READING_STATE: 
				if (! fSuccess || cbRet == 0) 
				{ 
					DisconnectAndReconnect(i); 
					continue; 
				} 
				Pipe[i].dwState = WRITING_STATE; 
				Pipe[i].cbRead=cbRet;
				break; 

				// Pending write operation 
			case WRITING_STATE: 
				if (! fSuccess || cbRet != Pipe[i].cbToWrite) 
				{ 
					DisconnectAndReconnect(i); 
					continue; 
				} 
				Pipe[i].dwState = READING_STATE; 
				break; 

			default: 
				{
					log_printf("Invalid pipe state.\n"); 
					return 0;
				}
			}  
		} 

		// The pipe state determines which operation to do next. 

		switch (Pipe[i].dwState) 
		{ 
			// READING_STATE: 
			// The pipe instance is connected to the client 
			// and is ready to read a request from the client. 

		case READING_STATE: 
			fSuccess = ReadFile( 
				Pipe[i].hPipeInst, 
				Pipe[i].chRequest, 
				BUFSIZE*sizeof(TCHAR), 
				&Pipe[i].cbRead, 
				&Pipe[i].oOverlap); 

			// The read operation completed successfully. 

			if (fSuccess && Pipe[i].cbRead != 0) 
			{ 
				Pipe[i].fPendingIO = FALSE; 
				Pipe[i].dwState = WRITING_STATE; 
				continue; 
			} 

			// The read operation is still pending. 

			dwErr = GetLastError(); 
			if (! fSuccess && (dwErr == ERROR_IO_PENDING)) 
			{ 
				Pipe[i].fPendingIO = TRUE; 
				continue; 
			} 

			// An error occurred; disconnect from the client. 

			DisconnectAndReconnect(i); 
			break; 

			// WRITING_STATE: 
			// The request was successfully read from the client. 
			// Get the reply data and write it to the client. 

		case WRITING_STATE: 
			GetAnswerToRequest(&Pipe[i]); 

			fSuccess = WriteFile( 
				Pipe[i].hPipeInst, 
				Pipe[i].chReply, 
				Pipe[i].cbToWrite, 
				&cbRet, 
				&Pipe[i].oOverlap); 

			// The write operation completed successfully. 

			if (fSuccess && cbRet == Pipe[i].cbToWrite) 
			{ 
				Pipe[i].fPendingIO = FALSE; 
				Pipe[i].dwState = READING_STATE; 
				continue; 
			} 

			// The write operation is still pending. 

			dwErr = GetLastError(); 
			if (! fSuccess && (dwErr == ERROR_IO_PENDING)) 
			{ 
				Pipe[i].fPendingIO = TRUE; 
				continue; 
			} 

			// An error occurred; disconnect from the client. 

			DisconnectAndReconnect(i); 
			break; 

		default: 
			{
				log_printf("Invalid pipe state.\n"); 
				return 0;
			}
		} 
	} 

}


VOID DisconnectAndReconnect(DWORD i) 
{ 
	// Disconnect the pipe instance. 

	if (! DisconnectNamedPipe(Pipe[i].hPipeInst) ) 
	{
		//printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
	}

	// Call a subroutine to connect to the new client. 

	Pipe[i].fPendingIO = ConnectToNewClient( 
		Pipe[i].hPipeInst, 
		&Pipe[i].oOverlap); 

	Pipe[i].dwState = Pipe[i].fPendingIO ? 
CONNECTING_STATE : // still connecting 
	READING_STATE;     // ready to read 
} 

// ConnectToNewClient(HANDLE, LPOVERLAPPED) 
// This function is called to start an overlapped connect operation. 
// It returns TRUE if an operation is pending or FALSE if the 
// connection has been completed. 

BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo) 
{ 
	BOOL fConnected, fPendingIO = FALSE; 

	// Start an overlapped connection for this pipe instance. 
	fConnected = ConnectNamedPipe(hPipe, lpo); 

	// Overlapped ConnectNamedPipe should return zero. 
	if (fConnected) 
	{
		log_printf("ConnectNamedPipe failed with %d.\n", GetLastError()); 
		return 0;
	}

	switch (GetLastError()) 
	{ 
		// The overlapped connection in progress. 
	case ERROR_IO_PENDING: 
		fPendingIO = TRUE; 
		break; 

		// Client is already connected, so signal an event. 

	case ERROR_PIPE_CONNECTED: 
		if (SetEvent(lpo->hEvent)) 
			break; 

		// If an error occurs during the connect operation... 
	default: 
		{
			log_printf("ConnectNamedPipe failed with %d.\n", GetLastError());
			return 0;
		}
	} 

	return fPendingIO; 
}

VOID GetAnswerToRequest(LPPIPEINST pipe)
{

	ULONG pid = 0;
	GetNamedPipeClientProcessId(pipe->hPipeInst,&pid);
       on_send_recv_(pcontext,pid,pipe->chRequest,pipe->cbRead,pipe->chReply,(int*)&(pipe->cbToWrite=0),hEvents[INSTANCES]);	
	/*
	_tprintf( TEXT("{%d}   [%d] %s :\n"),++gc, pipe->hPipeInst, pipe->chRequest);
	StringCchCopy( pipe->chReply, BUFSIZE, TEXT("Default answer from server") );
	pipe->cbToWrite = (lstrlen(pipe->chReply)+1)*sizeof(TCHAR);
	*/
	//Sleep(10);
}



static unsigned __stdcall s_thread_proc( void* p )
{
	try
	{
	this_type* pt=(this_type*)p;
    try
    {
       pt->start();
    }
      catch (...) {}
	 delete pt;
	}
	catch (...){}

	 return 0;
}

struct thread_pair
{
	
	HANDLE first;
    unsigned int second;
inline 	operator HANDLE()
	{
        return first;
	}

inline 	operator std::pair<HANDLE,unsigned int>()
{
	
	return std::make_pair(first,second);
}

};

struct thread_pair_holder
{
	thread_pair et;
	thread_pair_holder(thread_pair t):et(t){};
	~thread_pair_holder(){
		safe_shutdown(et);
	};
};



template <class CH>
static thread_pair create_thread_instance(const CH* tsPipename,on_send_recv_t on_send_recv,
										  void* pcontext,int time_out=-1,int pid=GetCurrentProcessId(),HANDLE hf=0)
{
     //HANDLE hret=0,ht;
	 HRESULT hr;
    thread_pair pp={0,0};

    this_type* pt= new this_type(tsPipename,time_out,pid);
       pt->handles(0)=hf;
	hr=pt->run(on_send_recv,pcontext,pp);
	if(FAILED(hr)) delete pt;
    SetLastError(hr);
	return pp;

};

template <class Allocator,class CH>
static thread_pair_holder* create_instance(const CH* tsPipename,on_send_recv_t on_send_recv,void* pcontext,int time_out=-1,int pid=GetCurrentProcessId(),HANDLE hf=0)
{
	//HANDLE hret=0,ht;
	


	HRESULT hr;


	thread_pair_holder* pp=0;
	thread_pair et=create_thread_instance(tsPipename,on_send_recv,pcontext,time_out,pid,hf);
	if(SUCCEEDED(hr=GetLastError()))
	{
        pp=new(Allocator().allocate(sizeof(thread_pair_holder))) thread_pair_holder(et);
	}
   SetLastError(hr);
   return pp;
};



protected:
   HRESULT run(on_send_recv_t on_send_recv,void* pcontext,thread_pair& pair)
   {
	   if(FAILED(hr)) return hr;
	   this->on_send_recv_=on_send_recv;
	   this->pcontext=pcontext;
	   unsigned int tid;
	   struct _hh
	   {
		   HANDLE h;
		   ~_hh(){CloseHandle(h);}
	   } hh={CreateThread(0,0,LPTHREAD_START_ROUTINE(&s_thread_proc),this,CREATE_SUSPENDED,LPDWORD(&tid))};
		   //

		   //{(HANDLE)_beginthreadex(NULL,0,&s_thread_proc,this,CREATE_SUSPENDED,&tid)}	;


	   if(hh.h)
	   {
		   HANDLE h=hEvents[INSTANCES],hp=GetCurrentProcess();

		   bool f=DuplicateHandle(hp,h,hp,&h,DUPLICATE_SAME_ACCESS,false,DUPLICATE_SAME_ACCESS);
		   thread_pair pp= {h,tid};
		   pair=pp;
		   ResumeThread(hh.h); 
		   
		   return 0;
	   }
	   else return hr=HRESULT_FROM_WIN32(GetLastError());
   }
private:
	pipe_port(pipe_port& ){};
	pipe_port(const pipe_port& ){};

};

inline char* hex_encode(char* dest,const void*src,int cb)
{
	static const char s_hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
		'A', 'B', 'C', 'D', 'E', 'F'};

      unsigned const char* pstr=(unsigned const char*)src;
	   char* pdest=dest;
	  for(int n=0;n<cb;++n)
	  {
         unsigned char c=pstr[n];
		 *pdest++=s_hex[(c>>4)&0x0F];
		 *pdest++=s_hex[(c)&0x0F];
	  }
   return dest;     
}

inline char* uuid_gen(char* buf=(char*)bufT<char,40>())
{
    //bufT<char,33> buf;
	union u_t{
		struct{long  iid[4];};
		struct{unsigned char cd[4*4];};
		GUID uu;
	} u  ;
    UuidCreateSequential((GUID*)&u.iid);
	char*p=buf;
	p[32]=0;
	//sprintf(p,"%08x%08x%08x%08x",u.iid[0],u.iid[1],u.iid[2],u.iid[3]); 
	return hex_encode(p,&u,sizeof(GUID));
};




inline v_buf<char> generate_pipe_name(const char* prefix="hs_sh")
{
	//GUID iid;
	char* p=(prefix)?prefix:"";
	return v_buf<char>().printf("\\\\.\\pipe\\%s%s",p,uuid_gen());

	v_buf<char> buf(128);
	
    int cb=sprintf(buf,"\\\\.\\pipe\\%s%s",p,uuid_gen());
    //buf.v.resize(cb+1);
	buf.resize(cb);
	return buf;

    long  iid[4]; 
	//if(SUCCEEDED(UuidCreateSequential((GUID*)&iid)))
	UuidCreateSequential((GUID*)&iid);
	{
		//RPC_S_UUID_LOCAL_ONLY			RPC_S_UUID_NO_ADDRESS


		char* p=(prefix)?prefix:"";
		int cb=sprintf(buf,"\\\\.\\pipe\\%s%08x%08x%08x%08x",p,iid[0],iid[1],iid[2],iid[3]); 
		buf.v.resize(cb+1);
	}
	return buf;
}


/*
#include "shared_allocator.h"
//#include "os_utils.h"
void *gphh;
template<int nclient,class CH>
inline int pipe_port_test(CH* pn,pipe_port<>::on_send_recv_t on_send_recv=0)
{
   struct  dflt
   {
	    static  int   on_send_recv(void* pcontext,unsigned long id,char* bufin,int cbr,char* bufout,int* pcbw,HANDLE hAbortEvent)
	   {

		   printf("%05d :cbr=%d %s\n",id,cbr,bufin);
		   return 0;
	   };

   };
if(!nclient)
{
  on_send_recv=(on_send_recv)?on_send_recv:dflt::on_send_recv;
HANDLE hea;


void* phh=pipe_port<>::create_instance<shared_allocator<pipe_port<>::thread_pair_holder> >(pn,on_send_recv,0,5000);//,6068);
gphh=phh;


//getchar();
{
	COMINIT_F;
//variant_t v=i_lib_ptr::_get_instance(phh);
IUnknown* punk=	i_lib_ptr::i_get(phh);
ComClipboard()=punk;
int ddd;
}
//ComClipboard()=variant_t(100);
//i_lib_ptr::acquire(phh);
i_lib_ptr::release(phh);
//getchar();
printf("AA");
//
i_lib_ptr::release(phh);
//i_lib_ptr::release(phh);
getchar();
printf("AC");
ComClipboard()=variant_t(100);
getchar();
printf("AVV");

//hea=pipe_port<>::create_thread_instance(pn,on_send_recv,0,5000);

}
else
{
	char buf[pipe_port<0>::BUFSIZE];
  for(int i=0;i<nclient;i++)
  {
    sprintf(buf,"%d\0",i+1);  
    pipe_port<>::send_recv(pn,buf,pipe_port<>::BUFSIZE);
  }
  if(0) 
	  for(int i=0;i<100;i++)
  {
	  sprintf(buf,"%d\0",i+1);  
	  HRESULT hr=pipe_port<>::send_recv(pn,buf,pipe_port<>::BUFSIZE);
  }
}
return 0;
}
*/
