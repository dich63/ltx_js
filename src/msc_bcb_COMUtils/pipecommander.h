#pragma once
#include <windows.h>

typedef int ( *sender_receiver_type)(void* pcntx,int cbr,byte* pbuf_in,int* pcbw,byte* ppbuf_out);	
struct win32_pipe_commander_base
{
    typedef unsigned char byte;
    typedef int ( *signal_type)(void* pcntx);
    void*  handle;
    void*  refcount;
	int  rsize;
	char * pread_buf;
	int  wsize;
	char * pwrite_buf;
	sender_receiver_type sender_receiver_proc;
	void * pcntx;
	bool fremote_share;
	char pipename[1];

};


struct win32_pipe_commander:win32_pipe_commander_base
{

	/*
	SECURITY_DESCRIPTOR sd;   
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),&sd,false}; 
	*/
	
static inline void close(void* p)
{
   if(p)
   {
          //&((win32_pipe_commander_base*)p)->handle,
	   //HANDLE h=(HANDLE) InterlockedExchangePointer(&((win32_pipe_commander_base*)p)->handle,INVALID_HANDLE_VALUE);
	    //CloseHandle(h);  
	   _aligned_free(p);
   }
};
static inline void* initialize(char* pname,int cb_read,int cb_write,
									  sender_receiver_type srvproc=0,void *pcntx=0,
									   bool fremote_share=0)
{
	      
	     size_t len=(pname)?strlen(pname):0;
		 if(!len) return 0;
	     size_t size=sizeof(win32_pipe_commander)+len+cb_read+cb_write+16;
         void* p= _aligned_malloc(size,64);
		 
          if(p)   
		  {
			  new(memset(p,0,size)) win32_pipe_commander(pname,len,cb_read,cb_write,srvproc,pcntx,fremote_share);
		  }

//         win32_pipe_commander& wpc=*((win32_pipe_commander*)p);
		 

    return p;

}
//protected:

win32_pipe_commander(char* pname,int nlen,int cb_read,int cb_write,sender_receiver_type srvproc=0,void *_pcntx=0,
					 bool fremote_=0)//rsize(cb_read),wsize(cb_write)
{
         rsize=cb_read;
		 wsize=cb_write;
         sender_receiver_proc=srvproc;
		 pcntx=_pcntx;
		 fremote_share=fremote_;
		 strcpy(pipename,pname);
		 pread_buf=pipename+nlen;
		 pwrite_buf=pread_buf+cb_read+8;

	//sender_receiver_proc=srvproc;,pcntx(_pcntx)
        //:rsize(cb_read),wsize(cb_write),sender_receiver_proc(srvproc),pcntx(_pcntx)
}

inline int AddRef()
{
 return	InterlockedIncrement((long*)&refcount);
}
inline int Release()
{
	return	InterlockedDecrement((long*)&refcount);
}
/*
inline int run()
{
	BOOL fConnected; 
	DWORD dwThreadId; 
	HANDLE hPipe, hThread; 
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe"); 

	// The main loop creates an instance of the named pipe and 
	// then waits for a client to connect to it. When the client 
	// connects, a thread is created to handle communications 
	// with that client, and the loop is repeated. 

	for (;;) 
	{ 
		hPipe = CreateNamedPipeA( 
			pipename,             // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			wsize,                  // output buffer size 
			rsize,                  // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 

		if (hPipe == INVALID_HANDLE_VALUE) 
		{
			printf("CreatePipe failed"); 
			return 0;
		}

        InterlockedExchangePointer(&handle,hPipe); 
		fConnected = ConnectNamedPipe(hPipe, NULL) ? 
        TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 
		InterlockedExchangePointer(&handle,INVALID_HANDLE_VALUE); 



}
*/

};