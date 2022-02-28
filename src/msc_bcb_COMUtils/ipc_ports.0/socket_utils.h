#pragma once

#include <Winsock2.h>
#include <Mswsock.h>
#include "wchar_parsers.h"
#include "shared_closures.h"


#pragma comment(lib ,"Ws2_32.lib")
#pragma comment(lib ,"Mswsock.lib")

#include "static_constructors_0.h" 
#include "tls_dll.h"

#include <map>
#include <string>

#pragma pack(1)
namespace hs_socket_utils
{

	struct CS
	{
		CRITICAL_SECTION m_cs;
		CS(){

			InitializeCriticalSection(&m_cs);
		}
		~CS()
		{
			DeleteCriticalSection(&m_cs);
		} // on  	  DLL_PROCESS_DETACH
		inline void lock(){ EnterCriticalSection(&m_cs);}
		inline void unlock(){ LeaveCriticalSection(&m_cs);}

	};

	template <class CS>
	struct locker_t
	{
		CS& r;
		locker_t(CS& c):r(c)
		{
			r.lock();
		}
		~locker_t()
		{
			r.unlock();
		}
	};

	typedef tls_LastErrorSaver LastErrorSaver;

	

	struct WSA_initerfiniter
	{
		HRESULT err;
		LPFN_CONNECTEX ConnectEx;
		LPFN_TRANSMITPACKETS TransmitPackets; 
		
		WSA_initerfiniter()
		{
			
			ConnectEx=0;
            TransmitPackets=0;
			WORD wVersionRequested;
			WSADATA wsaData;
			wVersionRequested = MAKEWORD( 2, 2 );
			err = WSAStartup( wVersionRequested, &wsaData );
			if(!err)
			{
				SOCKET s=::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
				

				struct soh
				{
                    SOCKET s;
					~soh(){::closesocket(s);}
				} __soh={s};

                
				
				GUID wiid=WSAID_CONNECTEX;
				DWORD cbr;
				if(WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,&wiid,sizeof(wiid),&ConnectEx,sizeof(ConnectEx),&cbr,0,0))
				{	
					err=WSAGetLastError();
				   return;
				}

				GUID wiid2=WSAID_TRANSMITPACKETS;
				
				if(WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,&wiid2,sizeof(wiid),&TransmitPackets,sizeof(TransmitPackets),&cbr,0,0))
						err=WSAGetLastError();				
              
			}

		}
		~WSA_initerfiniter()
		{
			if(err==0) WSACleanup();

		}
		inline operator bool(){ return err==0;}
	};

	typedef   class_initializer_T<WSA_initerfiniter> socket_lib_init_once;

	struct socket_io_t
	{
		inline WSA_initerfiniter* operator->()
		{
			return &socket_lib_init_once::get();
		}
		inline operator bool(){ return socket_lib_init_once::get();}
	};



    



    struct file_handle_list_t
	{
		//std::list<HANDLE> lh;
		typedef std::map<std::wstring,HANDLE> list_t;
		list_t lh;
		CS mutex;
           

inline		void clear()
		{
			locker_t<CS> lock(mutex);
			list_t::iterator i=lh.begin();
			for(;i!=lh.end();++i)
			{
				HANDLE h=i->second;
				if(h) CloseHandle(h);
			}
			lh.clear();
		}
        ~file_handle_list_t()
		{
            clear();
		}
inline    HANDLE get_alias_handle(wchar_t* walias,bool fremove=false)
	{
		if(!walias) return INVALID_HANDLE_VALUE;
		 v_buf<wchar_t> buf;
          walias=copy_z(walias,buf);
		  wcslwr(walias);

		locker_t<CS> lock(mutex);
		list_t::iterator i=lh.find(walias);
		if(i==lh.end()) return INVALID_HANDLE_VALUE;
		HANDLE h=i->second;
		if(fremove) lh.erase(i);	
		return h;
	};
inline    bool remove_alias(wchar_t* walias)
	{
		HANDLE h=get_alias_handle(walias,true);
		if(h==INVALID_HANDLE_VALUE) return false;
		CloseHandle(h);
		return true;
	}
inline	bool push( wchar_t* walias, HANDLE h)
		{
			if(!walias) return false;

			v_buf<wchar_t> buf;
			walias=copy_z(walias,buf);
			wcslwr(walias);

			locker_t<CS> lock(mutex);
			std::pair<list_t::iterator,bool> pp=lh.insert(std::make_pair(walias,h));
			return pp.second;
            //lh.push_back(h);

		}
inline int count()
     {  
        locker_t<CS> lock(mutex);
        return lh.size();  
     }

	};


	struct handle_holder
	{
		HANDLE m_h;
		handle_holder(HANDLE h=0):m_h(h) {};

		inline void reset(HANDLE h=0)
		{

			if(h=make_detach(m_h,h)) CloseHandle(h);

		}
		~handle_holder(){ reset();};

		inline HANDLE attach(HANDLE h)
		{
			reset(h);
			return m_h;
		}
		inline HANDLE detach()
		{
			return make_detach(m_h);
		}
		inline operator HANDLE()
		{
			return get();
		}
		inline  HANDLE get()
		{
			HANDLE h=(HANDLE)InterlockedExchangeAdd((LONG volatile* )&m_h,0);
			return h;
		}

		inline HANDLE dup_from(HANDLE h, bool finh=false,HANDLE hp=GetCurrentProcess())
		{

			return (DuplicateHandle(GetCurrentProcess(),h,hp,&m_h,DUPLICATE_SAME_ACCESS,finh,DUPLICATE_SAME_ACCESS))?get():0;
		}

		inline HANDLE dup(bool finh=false,HANDLE hp=GetCurrentProcess())
		{
			HANDLE h;
			get();
			return 	(DuplicateHandle(GetCurrentProcess(),m_h,hp,&h,DUPLICATE_SAME_ACCESS,finh,DUPLICATE_SAME_ACCESS))?h:0;
		}


	private:
		handle_holder(handle_holder& h){};
		handle_holder(const handle_holder& h){};

	};

	inline bool is_socket(SOCKET s)
	{
		return (INVALID_SOCKET!=s)&&(GetFileType(HANDLE(s))==FILE_TYPE_PIPE);
	}

inline HANDLE	dup_handle(HANDLE h, bool finh=false,HANDLE hp=GetCurrentProcess())
{
	if(h==INVALID_HANDLE_VALUE) return 0;
	if(h==0) return 0;
	handle_holder hh;
	hh.dup_from(h,finh,hp);
	return hh.detach();
}

inline HANDLE	dup_handle(DWORD pid,HANDLE h)
{
	handle_holder hh=OpenProcess(PROCESS_DUP_HANDLE,0,pid);
	if(hh==INVALID_HANDLE_VALUE) return INVALID_HANDLE_VALUE;
	return dup_handle(h,0,hh); 
}

inline static HRESULT send_WSABUF(SOCKET s,int nbuf,WSABUF* pwsabuf,HANDLE hAbortEvent=INVALID_HANDLE_VALUE)	
{

	WSABUF& wsabuf=*pwsabuf;

	struct OVERLAPPED_ex:OVERLAPPED
	{
		HANDLE hEvent2;
	}	;

	OVERLAPPED_ex    ovl=OVERLAPPED_ex();

	handle_holder hevent=CreateEvent(NULL,TRUE,FALSE,NULL);

	ovl.hEvent=hevent;
	ovl.hEvent2=hAbortEvent;

	int ecount=(hAbortEvent!=INVALID_HANDLE_VALUE)?2:1;  

	int rc,err;

	{
		DWORD cb=0,Flags=0;
		rc=WSASend(s,pwsabuf,nbuf,&cb,0,&ovl,0);

		if ( (rc) && 
			(WSA_IO_PENDING != (err = WSAGetLastError()))) 
			return HRESULT_FROM_WIN32(err);


		while(WSA_WAIT_IO_COMPLETION==(rc = WSAWaitForMultipleEvents(ecount, &ovl.hEvent,FALSE,INFINITE,TRUE))){};

		if(!HasOverlappedIoCompleted(&ovl))
		{
			CancelIo(HANDLE(s));
			return E_FAIL;
		}

	}

	return S_OK;
};
        
          

	inline static HRESULT recv_WSABUF(SOCKET s,int nbuf,WSABUF* pwsabuf,DWORD *pnbytes,DWORD flags=0,HANDLE hAbortEvent=INVALID_HANDLE_VALUE)	
	{

		WSABUF& wsabuf=*pwsabuf;

		struct OVERLAPPED_ex:OVERLAPPED
		{
			HANDLE hEvent2;
		}	;

		OVERLAPPED_ex    ovl=OVERLAPPED_ex();

		handle_holder hevent=CreateEvent(NULL,TRUE,FALSE,NULL);

		ovl.hEvent=hevent;
		ovl.hEvent2=hAbortEvent;

		int ecount=(hAbortEvent!=INVALID_HANDLE_VALUE)?2:1;  

		int rc,err;
		
		{
			DWORD Flags=0;
			//rc=WSASend(s,pwsabuf,nbuf,&cb,0,&ovl,0);
              rc=WSARecv(s,pwsabuf,nbuf,pnbytes,&flags,&ovl,0);
			if ( (rc) && 
				(WSA_IO_PENDING != (err = WSAGetLastError()))) 
				return HRESULT_FROM_WIN32(err);
			

			while(WSA_WAIT_IO_COMPLETION==(rc = WSAWaitForMultipleEvents(ecount, &ovl.hEvent,FALSE,INFINITE,TRUE))){};

			if(!HasOverlappedIoCompleted(&ovl))
			{
				CancelIo(HANDLE(s));
				return E_FAIL;
			}

		}

		return S_OK;
	};

   

	template <class T>
	unsigned long Crc32(T* p,size_t c=1)
	{

		struct __s_crt__
		{
			unsigned long crc_table[256];
			__s_crt__()
			{
				unsigned long crc;
				for (int i = 0; i < 256; i++)
				{
					crc = i;
					for (int j = 0; j < 8; j++)
						crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;

					crc_table[i] = crc;
				};
			};

			inline unsigned long make(void *p, unsigned long len)
			{
				unsigned char *buf=(unsigned char *)p;
				unsigned long crc = 0xFFFFFFFFUL;
				while (len--) 
					crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);

				return crc ^ 0xFFFFFFFFUL;

			};

		};


		return	class_initializer_T<__s_crt__> ::get().make(p,sizeof(T)*c);
	}		

inline 	SOCKET  dup_socket(SOCKET s )
	{
		SOCKET fd;
		WSAPROTOCOL_INFOW wsd;

		if(SOCKET_ERROR != WSADuplicateSocketW(s,GetCurrentProcessId(),&wsd))
			return fd=WSASocketW(FROM_PROTOCOL_INFO,FROM_PROTOCOL_INFO,FROM_PROTOCOL_INFO,&wsd, 0, FROM_PROTOCOL_INFO);
		else return INVALID_SOCKET;
	};


struct sock_holder
{
	SOCKET s;  
	sock_holder(SOCKET _s):s(_s){};
	~sock_holder()
	{
		if(s!=INVALID_SOCKET) 
			::closesocket(s);
	}

};

struct socket_events
{

	LastErrorSaver hr;

	



	struct OVERLAPPED_ex:OVERLAPPED_shared_closure_base_t  
	{
		
		OVERLAPPED_ex()
		{
			
           memset(this,0,sizeof(*this));   
		   
           hEvent=CreateEvent(0,1,1,0);
		   hkcount=1;
		}
		~OVERLAPPED_ex()
		{
			
			DWORD dw;
            WSACloseEvent(hEvent);
		}
inline   OVERLAPPED* address_out(int freset=0)
	{
         if(freset) SetEvent(hEvent);
		 else    ResetEvent(hEvent);
		Internal=0;InternalHigh=0;Offset=0;OffsetHigh=0;
		return this;

	}
	private:
         OVERLAPPED_ex(OVERLAPPED_ex&);
		 OVERLAPPED_ex(const OVERLAPPED_ex&);
	};

	OVERLAPPED_ex ovl;

	socket_events():hr(0){}//{memset(&ovl,0,sizeof(ovl));};
	inline   void     init_event(HANDLE h1=INVALID_HANDLE_VALUE,HANDLE h2=INVALID_HANDLE_VALUE)
	{

		//		memset(&ovl,0,sizeof(ovl));
		//		ovl.hEvent=::CreateEvent(0,1,0,0);
		//ovl.hkcount=1;
		if(h1!=INVALID_HANDLE_VALUE)  ovl.hkcount++;
		if(h2!=INVALID_HANDLE_VALUE)  ovl.hkcount++;
		//ovl.h[0]=dup_handle(h1);
		//ovl.h[1]=dup_handle(h2);
		ovl.hkObjects[0]=h1;
		ovl.hkObjects[1]=h2;

	};
	socket_events(HANDLE h1,HANDLE h2=INVALID_HANDLE_VALUE):hr(0)
	{
          init_event(h1,h2);
	}

	int  recv(SOCKET s,char* buf,int len,int flags)
	{

		WSABUF wsabuf={len,buf};
		int rc,err;
		bool f;
		DWORD nbytes=0;

		{
			DWORD Flags=0;
			//rc=WSASend(s,pwsabuf,nbuf,&cb,0,&ovl,0);
			rc=WSARecv(s,&wsabuf,1,&nbytes,(LPDWORD)&flags,&ovl,0);
			if((rc==0)&&(nbytes==0)) 
			{
				hr=err=ERROR_GRACEFUL_DISCONNECT;
			    return 0;
			}
			if ( (rc) && (WSA_IO_PENDING != (err= WSAGetLastError()))) 
				return hr=err, -1;
			DWORD ws;
			ws=WaitForMultipleObjects(ovl.hkcount,&ovl.hEvent,false,INFINITE); 
			if(ws!=WAIT_OBJECT_0)
				return CancelIo(HANDLE(s)),hr=err=E_ABORT,-1;


			   f=WSAGetOverlappedResult(s,&ovl,&nbytes,true, (LPDWORD)&flags);

			if(!f)
				return CancelIo(HANDLE(s)),hr=err=E_ABORT,-1;


		}

        return nbytes;
	};

};


struct host_port_resolver:socket_events
{
   		

	   //LastErrorSaver hr;
	   SOCKET s;
	   std::vector<char> vbuf;
	   const char* host;
	   const char* port;
	   int portn;
	   struct hostent* hs;
	   sockaddr_in ai;
	   LPFN_CONNECTEX ConnectEx;
	   char* ip;

/*
	   struct OVERLAPPED_ex:OVERLAPPED  
	   {
          HANDLE h[2];
		  int hkcount;
         ~OVERLAPPED_ex()
		 {
			 //for(int i=-1;i<2;i++)
			//	 if(h[i]) CloseHandle(h[i]);
			 if(hEvent) CloseHandle(hEvent);
		 }
	   };

        OVERLAPPED_ex ovl;
*/

		struct accept_data
		{
			//			int cb;
 			int cbls;
			SOCKADDR_IN* pls;
			int cbrs;
			SOCKADDR_IN* prs;
			void* abortproc;
			void* pcallback;
			SOCKET s;
			char buf[256];
			accept_data()
			{
				memset(this,0,sizeof(accept_data));
				//				cb=sizeof(accept_data);
			}
		};

 /*
inline   void     init_event(HANDLE h1=INVALID_HANDLE_VALUE,HANDLE h2=INVALID_HANDLE_VALUE)
   {
              
	   memset(&ovl,0,sizeof(ovl));

	   ovl.hEvent=::CreateEvent(0,1,0,0);
	   ovl.hkcount=1;
	   if(h1!=INVALID_HANDLE_VALUE)  ++ovl.hkcount;
	   if(h2!=INVALID_HANDLE_VALUE)  ++ovl.hkcount;
	   //ovl.h[0]=dup_handle(h1);
	   //ovl.h[1]=dup_handle(h2);
	   ovl.h[0]=h1;
	   ovl.h[1]=h2;

   };
*/
inline	bool     init(const char* host,const  char* port,HANDLE h1=INVALID_HANDLE_VALUE,HANDLE h2=INVALID_HANDLE_VALUE)
		{
			    init_event(h1,h2);

			    s=INVALID_SOCKET;
                socket_lib_init_once t;

			   if((!host)||(*host==0)) host="0.0.0.0";
                 if(!host) host="";
			   if(!port) port="";
			    portn=atoi(port);

				hs=gethostbyname(host);
				if(!hs)
				{
					hr=WSAGetLastError();
					return false;
				}
				memset(&ai,0,sizeof(ai));
				ip= inet_ntoa (*(struct in_addr *)*hs->h_addr_list);
				ai.sin_family=AF_INET;
				//
				ai.sin_addr.s_addr =*(u_long *) hs->h_addr_list[0];
                //				ai.sin_addr.s_addr = inet_addr(ip);
				ai.sin_port=htons(portn);

				//				s =::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				//
				s=WSASocketW(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
				if (s != INVALID_SOCKET) 
					return true;

                 hr=WSAGetLastError();
             return true;
		}

	  host_port_resolver(const char* hostport,HANDLE h1=INVALID_HANDLE_VALUE,HANDLE h2=INVALID_HANDLE_VALUE)
		  :port(0),host(0)
	   {      
		      
			 if(hostport)
			 {
		 
			 port = strrchr(hostport,':');

			 if(port)
			 {
               
       		 int cb=int(port++)-int(hostport);
			 
			 vbuf.resize(cb+1);
			 host=&(vbuf[0]);
			 memcpy(&(vbuf[0]),hostport,cb);
			 }
			 else 
			 {
				 port="";
				 host=hostport;
			 }
			 }
			    init(host,port,h1,h2);
	   }

	   host_port_resolver(const char* host,const char* port,HANDLE h1=INVALID_HANDLE_VALUE,HANDLE h2=INVALID_HANDLE_VALUE)
	   {
          init(host,port,h1,h2);
	   }

inline 	   SOCKET	   detach() {

			   return make_detach(s,INVALID_SOCKET);
		   }

inline 	   void close()  {

           if (s != INVALID_SOCKET) 
			    ::closesocket( detach());
        }

	   ~host_port_resolver(){ close(); };

	   inline    operator bool()   
	   {
		   return hr==S_OK;
	   }

   host_port_resolver& bind(bool freuseaddr=true)
   {
       if(hr!=S_OK) return *this;
	   
		   long bs=1;
		   if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(const char*)&bs,sizeof(bs)))
			   hr=WSAGetLastError();
		   else  if(::bind( s,(SOCKADDR*) &ai,sizeof(ai)) == SOCKET_ERROR)
				   hr=WSAGetLastError();
                  else  if (listen( s, SOMAXCONN) == SOCKET_ERROR) 
          			     hr=WSAGetLastError();
      	   
	   return *this;

   }


   SOCKET accept()
   {
	   //if(hr!=S_OK) return INVALID_SOCKET;

	   struct AS
	   {
          SOCKET s;  
		  ~AS()
		  {
			  if(s!=INVALID_SOCKET) 
				  ::closesocket(s);
		  }

	   } a={::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)};

	   if(a.s==INVALID_SOCKET) {hr=WSAGetLastError(); return INVALID_SOCKET;}

	   bool f;
	   DWORD la[32],cr,err,dwf,ca,cl;
	   ca=sizeof(SOCKADDR_IN)+16;

       f=AcceptEx(s,a.s,la,0,ca,ca,&cr,&ovl);
	   if(!f)
	   {
           if(((err=WSAGetLastError())!=ERROR_IO_PENDING))
			   return hr=err,INVALID_SOCKET;
		   DWORD ws;
		   ws=WaitForMultipleObjects(ovl.hkcount,&ovl.hEvent,false,INFINITE); 
		   if(ws!=WAIT_OBJECT_0)
		   			   return CancelIo(HANDLE(s)),hr=err=E_ABORT,INVALID_SOCKET;

		   
		   f=WSAGetOverlappedResult(s,&ovl,&cr,true, &dwf);

		   if(!f)
				   return CancelIo(HANDLE(s)),hr=err=E_ABORT,INVALID_SOCKET;

		   SOCKADDR_IN* pls;
		   SOCKADDR_IN* prs;
			GetAcceptExSockaddrs(la,cr,ca,ca,(LPSOCKADDR*)&pls,(LPINT)&cl,(LPSOCKADDR*)&prs,(LPINT)&ca);
		   dwf=2;

		   // if(ws!=WAIT_OBJECT_0)
		   //if(!HasOverlappedIoCompleted(&ovl)) return hr=err=E_ABORT,INVALID_SOCKET;
   
	   }
		   return make_detach(a.s);
   };



   HRESULT accept(accept_data *pad)
   {
	   //if(hr!=S_OK) return INVALID_SOCKET;
	   pad->s=INVALID_SOCKET;

	   struct AS
	   {
		   SOCKET s;  
		   ~AS()
		   {
			   if(s!=INVALID_SOCKET) 
				   ::closesocket(s);
		   }

	   } a={::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)};

	   if(a.s==INVALID_SOCKET) {hr=WSAGetLastError(); return hr;}

	   bool f;
	   DWORD cr,err,dwf,ca,cl;
	   ca=sizeof(SOCKADDR_IN)+16;
	    char* la=pad->buf;
	   f=AcceptEx(s,a.s,la,0,ca,ca,&cr,&ovl);
	   if(!f)
	   {
		   if(((err=WSAGetLastError())!=ERROR_IO_PENDING))
			   return hr=err;
		   DWORD ws;
		   ws=WaitForMultipleObjects(ovl.hkcount,&ovl.hEvent,false,INFINITE); 
		   if(ws!=WAIT_OBJECT_0)
			   return CancelIo(HANDLE(s)),hr=err=E_ABORT;


		   f=WSAGetOverlappedResult(s,&ovl,&cr,true, &dwf);

		   if(!f)
			   return CancelIo(HANDLE(s)),hr=err=E_ABORT;

		   SOCKADDR_IN* pls;
		   SOCKADDR_IN* prs;
		   GetAcceptExSockaddrs(la,cr,ca,ca,(LPSOCKADDR*)&(pad->pls),(LPINT)&(pad->cbls),(LPSOCKADDR*)&(pad->prs),(LPINT)&(pad->cbrs));
		   dwf=2;
		   err = setsockopt(a.s, SOL_SOCKET,SO_UPDATE_ACCEPT_CONTEXT,(char*)&s,sizeof(s));
		   if(err) 
			   return CancelIo(HANDLE(s)),hr=err=E_ABORT;
               pad->s=make_detach(a.s);
		   // if(ws!=WAIT_OBJECT_0)
		   //if(!HasOverlappedIoCompleted(&ovl)) return hr=err=E_ABORT,INVALID_SOCKET;

	   }
	   return S_OK;;
   };


   host_port_resolver& connect()
   { 
	   HRESULT err;
	   bool f;

	   

	   if(0)
	   {

	   
	   if(::connect(s,(SOCKADDR*)&ai,sizeof(ai))!=0)
		   hr=err=WSAGetLastError();
	   return *this;
	   };
	   if(hr==S_OK) 
	   {
		   //ConnectEx
		   GUID wiid=WSAID_CONNECTEX;
		   DWORD cbr;
		   hr=err=WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,&wiid,sizeof(wiid),&ConnectEx,sizeof(ConnectEx),&cbr,0,0);
		   if(err!=S_OK) 
			   return *this;

 		   //f=ConnectEx(s,(struct sockaddr*)&ai,sizeof(ai),NULL,0,&cbr,&ovl);
		   cbr=0;

		   sockaddr_in sa;
		   memset(&sa,0,sizeof(sockaddr_in));
		       sa.sin_family=ai.sin_family; 
		       //sa.sin_port=0;   
		       //sa.sin_addr=INADDR_ANY;

               if(::bind( s,(SOCKADDR*) &sa,sizeof(sa)))
			   {
                hr=err=WSAGetLastError();
				return *this;
			   }
			   
		   f=ConnectEx(s,(struct sockaddr*)&ai,sizeof(ai),0,0,&cbr,&ovl);
		   if((!f)&&((err=WSAGetLastError())==ERROR_IO_PENDING))
		   {
             DWORD ws;
              ws=WaitForMultipleObjects(ovl.hkcount,&ovl.hEvent,false,INFINITE); 
              if(ws!=WAIT_OBJECT_0) hr=err=E_ABORT;
			  else
			  {
			      f=GetOverlappedResult(HANDLE(s),&ovl,&cbr,1);
				  if(f&&(0==setsockopt( s, SOL_SOCKET,SO_UPDATE_CONNECT_CONTEXT,NULL,0)))
					   hr=S_OK;
				  else hr=err=WSAGetLastError();

			  }
			  //if(err==0)
			  //err = setsockopt( s, SOL_SOCKET,SO_UPDATE_CONNECT_CONTEXT,NULL,0);
   		   }
		   else   hr=err;
		   
  	   //AcceptEx
         
	   }
    
	   return *this;
         
   }



};




struct sockinfo
{
    char str[512];
	DWORD cb;
	sockinfo(void *psockadrr,int sz)
	 {
		 
		 const int smax=sizeof(str);
		 str[0]=0;
		int f=WSAAddressToStringA(LPSOCKADDR(psockadrr),sz,0,str,&(cb=smax));
		if(f) cb=1;
	
        if(smax-cb>sz)
		{
          memcpy(str+cb,psockadrr,sz);
		  cb+=sz;
		}
	 }
};

struct sockinfo1
{
	char peername[64];
	char sockname[64];
	char report[128];
	sockinfo1(SOCKET s)
	{
		sockaddr_in soi;
		char* p;
		unsigned long pn;
		int sz;
		sz=sizeof(sockaddr_in);
		HRESULT  err,hr;
		err=getpeername(s,(sockaddr*)&soi,&sz);
		hr=GetLastError();
		if(err==SOCKET_ERROR) 
			strcpy(peername,"??.??.??.??:??");
		else  sprintf(peername,"%s:%d",inet_ntoa(soi.sin_addr),htons(soi.sin_port));
		sz=sizeof(sockaddr_in);
		if(0!=getsockname(s,(sockaddr*)&soi,&sz))
			strcpy(sockname,"??.??.??.??:??");
		else  sprintf(sockname,"%s:%d",inet_ntoa(soi.sin_addr),htons(soi.sin_port));

		sprintf(report,"sockname=%s ;peername=%s",sockname,peername);

	}
};


};