#pragma once
#include "hs_socket.h"
#include "video/singleton_utils.h"
#include <map>
#include <set>
#include <math.h>
#include	<mstcpip.h>

struct pipe_console_t
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
	static CS& cons_mutex()
	{
		static CS cm;
		return cm;
	}
	static  int cs_printf(const wchar_t* fmt,va_list argptr)
	{
		return _vcwprintf( fmt, argptr);
	}
	static  int cs_printf(const char* fmt,va_list argptr)
	{
		return _vcprintf( fmt, argptr);
	}

	template <class CH>
	static void   title_printf(const CH* fmt,...)
	{
		if(!is_console()) return;
		va_list argptr;
		va_start(argptr, fmt);
		v_buf<CH> vb;
		vb.vprintf(fmt,argptr);
		SetConsoleTitleW(char_mutator<CP_THREAD_ACP>(vb.get()));
	}
	template <class CH>
	static void   log_printf(const CH* fmt,...)
	{
		if(!is_console()) return;
		locker_t<CS> lock(cons_mutex());
		va_list argptr;
		va_start(argptr, fmt);
		cs_printf(fmt,argptr);
	}


};

template <int _bufsize=8*1024>
struct pair_socket_reflector_t
{
           

	typedef  int (__stdcall *op_proc_t)(void*,int);
	typedef  int (__stdcall *disconnect_proc_t)(char*);


	struct packet_pair_t
	{
        
		
		enum
		{
			bufsize=_bufsize
		};



		struct operation_t
		{
             enum op_type
			 {    
				 op_unknow=0,
				 op_recv=1,
				 op_send=2
			 };

			typedef  int (__stdcall * operation_proc_t)(
				SOCKET s,
				LPWSABUF lpBuffers,
				DWORD dwBufferCount,
				LPDWORD lpNumberOfBytesSent,
				DWORD dwFlags,
				LPWSAOVERLAPPED lpOverlapped,
				LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
				);


			typedef operation_t* this_t;

			OVERLAPPED ovl;
			WSABUF wsa_recv;
			WSABUF wsa_send;
			SOCKET s_recv;
			SOCKET s_send;
			packet_pair_t* owner;
			DWORD BytesRECV,BytesSEND;
			int err;
			op_type opt;
			int num;
			

			// operation_proc_t operation_proc;

			inline int op(DWORD BytesTransferred)  
			{
				DWORD cbop=0;

				ovl=OVERLAPPED();

				if(BytesTransferred==0)
				{
					
					if(0)if(opt==op_recv)
					{
						wsa_send.len=0;
						int err=check_err(WSASend(s_send,&wsa_send,1,&cbop,0,&ovl,NULL));
						
					}
					owner->shutdown_socket_pair();
					return release(),0;
					//owner->close_socket_pair();
					
				}

				if(BytesRECV==0)
				{
					BytesRECV=BytesTransferred;
					BytesSEND=0;
				}
				else 
					BytesSEND+=BytesTransferred;

/*
				if((bufsize!=BytesRECV)&&(bufsize!=BytesSEND))
				{
                    cprintf("recv=%d send=%d\n",BytesRECV,BytesSEND); 
				} 
*/
				

				if (BytesRECV > BytesSEND)
				{
					wsa_send.buf=wsa_recv.buf+BytesSEND;
					wsa_send.len=BytesRECV-BytesSEND;

					int err=check_err(WSASend(s_send,&wsa_send,1,&cbop,0,&ovl,NULL));
					if(err)		   
						return release(),err;
					opt=op_send;

				}
				else
				{
					BytesRECV=0;
					DWORD Flags=0;
					int err=check_err(WSARecv(s_recv,&wsa_recv,1,&cbop,&Flags,&ovl,NULL));
					if(err)		   
						return release(),err;
					opt=op_recv;
				}        

				return 0;	

			}

	

			int  init(packet_pair_t* po,SOCKET r,SOCKET s,char *pbuf,int len=bufsize)
			{
				DWORD cbop=0,Flags=0;
				owner=po;
				hss_acquire(owner);
				s_recv=r;
				s_send=s;
				wsa_recv.len=len;
				wsa_recv.buf=pbuf;
				BytesRECV=BytesSEND=0;
				//				closesocket(s_recv); 				return 0;
				int err=check_err(WSARecv(s_recv,&wsa_recv,1,&cbop,&Flags,&ovl,NULL));
				opt=op_recv;
				if(err)		   
					return release(),err;
				else     return 0;

			}

			static int __stdcall s_op(void *p ,DWORD cb)
			{
				return (p)? this_t(p)->op(cb):-1;
			}

			void release()
			{
				if(owner) hss_release(make_detach(owner));

			}

			inline  HRESULT  check_err(int res)
			{
				HRESULT err;
				//if(res== SOCKET_ERROR)
				if(res)
				{
					if (err=WSAGetLastError() != ERROR_IO_PENDING)
					{
						return err;
					}
				}
				return 0;
			}


		};

		operation_t io_op[2];

		pair_socket_reflector_t* powner;
		v_buf<char> vpipename;

		inline static	int keep_alive(SOCKET s,int tw=2000,int ti=1000)
		{
			struct tcp_keepalive	alive={1,tw,ti};
			DWORD sz;
			int r=WSAIoctl(s, SIO_KEEPALIVE_VALS, &alive,sizeof(alive),NULL,0,&sz,NULL,NULL);
			return r;

		}

static		VOID CALLBACK WaitOrTimerCallback(
			PVOID lpParameter,
			BOOLEAN TimerOrWaitFired
			)
           {
			   int cc=0;
           };



		packet_pair_t(pair_socket_reflector_t<_bufsize>* psr,SOCKET sock1,SOCKET sock2,char* sinfo=0)
			:s1(sock1),s2(sock2),hr(0),powner(0)
		{
               HANDLE hIOCP = psr->hIOCP;
			   vpipename.cat(sinfo);

			   
			if(!CreateIoCompletionPort(HANDLE(sock1),hIOCP,ULONG_PTR(&operation_t::s_op),0))
			{
				hr=GetLastError();
				return;
			};
			if(!CreateIoCompletionPort(HANDLE(sock2),hIOCP,ULONG_PTR(&operation_t::s_op),0))
			{
				hr=GetLastError();
				return;
			};
			io_op[0].num=0;
			io_op[1].num=1;
			int res;
            res=keep_alive(sock1);
			res=keep_alive(sock2);

			//
			/*
			HANDLE hev=WSACreateEvent(), hh;

			 res=WSAEventSelect(sock1,hev,FD_CLOSE);
			 res=RegisterWaitForSingleObject(&hh,hev,&WaitOrTimerCallback,(void*)sock1,INFINITE,WT_EXECUTEONLYONCE);

			 //shutdown_socket_pair();
			 
			 WSAPROTOCOL_INFOW wsai;
			 res=WSADuplicateSocketW(sock1,GetCurrentProcessId(),&wsai);
			 closesocket(sock1);

			 while(1){Sleep(500);}
			 //*/







			if(1)
			{

			
			if(hr=io_op[0].init(this,sock1,sock2,buf1))
				return;
			if(hr=io_op[1].init(this,sock2,sock1,buf2))
				return;
			};

			powner=psr;
			hss_acquire(powner);

		};

		static void on_exit(packet_pair_t* p)
		{
			p->~packet_pair_t();
		};

		 inline void close_socket_pair()
		 {
			 ::closesocket(s1);
			 ::closesocket(s2);
		 }
		 inline void shutdown_socket_pair()
		 {
			 ::shutdown(s1,SD_BOTH);
			 ::shutdown(s2,SD_BOTH);
		 }
		~packet_pair_t(){

			close_socket_pair();
			if(powner->disconnect_proc)
				powner->disconnect_proc(vpipename.get());
			hss_release(powner);
			pipe_console_t::log_printf("%s :disconnected\n",vpipename.get());
		}

		HRESULT hr;
		SOCKET s1,s2; 
		char buf1[bufsize];
		char buf2[bufsize];
	};

	HANDLE hIOCP,hExitEvent;
	LONG fabort;
	HANDLE hthreads[64];
	int nthreads;
	disconnect_proc_t disconnect_proc;


	static void __stdcall s_loop(pair_socket_reflector_t* p)
	{
		p->loop();
	}
	inline long check_abort()
	{
		return InterlockedExchangeAdd((LONG volatile*)&fabort,0);
	}

	inline long set_abort()
	{
		return InterlockedExchange((LONG volatile*)&fabort,1);
	}

	void loop()
	{
		DWORD BytesTransferred;
		
		int cc=0;
		HRESULT herr;

		while(!check_abort())
		{
			op_proc_t op_proc=0;
			OVERLAPPED* povl=0;
			
			if(!GetQueuedCompletionStatus(hIOCP, &BytesTransferred,(PULONG_PTR)&op_proc,&povl,500))
			{
				/*
				if((herr=GetLastError())!=WAIT_TIMEOUT) 
				{
					//hss_release(this);
					//return;  

				}
				continue;
				*/
				if((herr=GetLastError())==WAIT_TIMEOUT) continue;
				else
				{
					cc=-1;
				}

			};
             //_cprintf("pic..%d            \r",cc++);
			if(op_proc)
				op_proc(povl,BytesTransferred);

		}
		return ;
	}



	pair_socket_reflector_t(int _nthreads=1):fabort(0),nthreads(_nthreads),disconnect_proc(0)
	{
		DWORD tid;
		//hExitEvent=CreateEvent(0,1,0,0);
		hIOCP=CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		for(int n=0;n<nthreads;n++)
					  	hthreads[n]=CreateThread(0,0,LPTHREAD_START_ROUTINE(&s_loop),this,0,&tid);
		
		
	}

	~pair_socket_reflector_t()
	{
		//WaitForSingleObject(hAbortEvent);
		//SetEvent(hExitEvent);
		set_abort();
        WaitForMultipleObjects(nthreads,hthreads,true,INFINITE);
		for(int n=0;n<nthreads;n++)
			CloseHandle(hthreads[n]);

		CloseHandle(hIOCP);
	}

	static void on_exit(pair_socket_reflector_t* p)
	{
		p->~pair_socket_reflector_t();
	};

	void push_pair(SOCKET s1,SOCKET s2,char* sinfo=0)
	{
		packet_pair_t* pp  =new(hss_alloc(sizeof(packet_pair_t))) packet_pair_t(this,s1,s2,sinfo);
		
		hss_weak_ref(pp,&packet_pair_t::on_exit,pp);
		hss_release(pp);
	}

	void push_pair(std::pair<SOCKET,SOCKET> ss)
	{
		push_pair(ss.first,ss.second);
	}


static pair_socket_reflector_t*  create(double nthreads=-2)
{
  if(nthreads<0) 
  {
	  SYSTEM_INFO si;
	  GetSystemInfo(&si);
	  nthreads*=-int(si.dwNumberOfProcessors);
  }
  int nt=max(nthreads,nthreads+0.5);

  if(nt<1) nt=1;

  pair_socket_reflector_t* p=new (hss_alloc(sizeof(pair_socket_reflector_t))) pair_socket_reflector_t(nt);
  hss_weak_ref(p,&pair_socket_reflector_t::on_exit,p);
  return p;
}

};



template <int _bufsize=8*1024>
struct socket_port_pipe_t
{
	
	struct lessSTR
		: public std::binary_function<char*,char*, bool>
	{	
	 inline bool operator()(const char* l, const char* r) const
		{	
			return strcmpi(l,r)<0;
		}
	};

	struct lessSTRs
		: public std::binary_function<char*,char*, bool>
	{	
		inline bool operator()(const std::string& l, const std::string& r) const
		{	
            lessSTR lss; 
			return lss(l.c_str(),r.c_str());
		}
	};

	typedef  pipe_console_t::CS CS;

	//typedef  std::map<char*,HSS_INFO*,lessSTR> pipemap_t;
	typedef  std::map<char*,std::pair<int,HSS_INFO*>,lessSTR> pipemap_t;
	typedef  std::set<std::string,lessSTRs> pipeset_connected_t;




	void* hport,*hclosure,*hclosure_disconnect;
	pair_socket_reflector_t<_bufsize>* psr;
	CS mutex;
	int conn_count;
	HRESULT hr;

	pipemap_t pipemap;
	pipeset_connected_t pipeset_connected;
	v_buf<wchar_t> vportname;

static void __stdcall s_on_accept(socket_port_pipe_t* p,HSS_INFO* povl)
{
   p->on_accept(povl);

}

static void __stdcall s_on_disconnect(socket_port_pipe_t* p,char* pipename)
{
	p->on_disconnect(pipename);

}

//log_printf(char* fmt)


inline HSS_INFO* get_second_or_set_first(char* name,HSS_INFO* povl)
{
	  HSS_INFO* res=NULL;  
      locker_t<CS> lock(mutex);
	  ///HSS_INFO*& rh=pipemap[name];
	  std::pair<int,HSS_INFO*>& rhp=pipemap[name];

	  res=rhp.second;
	  if(res==NULL) 
	  {  
		  hss_acquire(povl);
		  rhp.second=povl;
		  
	  }
	  else pipemap.erase(name);
	  
	  return res;
}


inline std::pair<int,HSS_INFO*> get_second_or_set_first(char* name,std::pair<int,HSS_INFO*> pair_povl)
{
	//HSS_INFO* res=0;  
	std::pair<int,HSS_INFO*> res;
	locker_t<CS> lock(mutex);
	///HSS_INFO*& rh=pipemap[name];

	//if(pipemap_connected[name])
	//	return res;
	if(pipeset_connected.find(name)!=pipeset_connected.end())
		return res;

	std::pair<int,HSS_INFO*>& rhp=pipemap[name];


	res=rhp;
	if(res.second==0) 
	{  
		hss_acquire(pair_povl.second);
		//rhp.second=povl;
		rhp=pair_povl;
		char* peern=hss_reset_SOCKET_DATA(pair_povl.second,HSD_REMOTE_INFO);
		pipe_console_t::log_printf("%s waiting pipe=\"%s\" ....\n",peern,name);
		

	}
	else 
	{
        int m1=pair_povl.first;
		int m2=rhp.first;
		if((m1|m2)&&(m1-m2))
		{
            return std::pair<int,HSS_INFO*>();
       	}
		else
		{
			pipeset_connected.insert(name);
			conn_count=pipeset_connected.size();
			pipe_console_t::title_printf(L"%s  connection count=%d",vportname.get(),conn_count);
			
			pipemap.erase(name);
			


		}
	}

	return res;
}


inline  int conection_count()
{
     locker_t<CS> lock(mutex);
	 return pipeset_connected.size();
}

inline  void on_disconnect(char* pipename)
{
	  locker_t<CS> lock(mutex);
      pipeset_connected.erase(pipename);
	  pipe_console_t::title_printf(L"%s  connection count=%d",vportname.get(),conection_count());
}

inline  void on_accept(HSS_INFO* hovl)
 {
	 ULONG st=hss_websocket_handshake_chain(hovl);
	 hss_reset_socket(hovl,HSF_REPLAY_SRD_AUTO);
     
   char* params=(char*)hss_reset_SOCKET_DATA(hovl,HSD_PARAMS,HSF_GET);
   argv_zz<char>  args(params); 
   char* pipename=args["pipe"];

   int mode=args["mode"].def(0);
     mode= args["charge"].def(mode);


   if(!pipename) return;

   std::pair<int,HSS_INFO*> pp=get_second_or_set_first(pipename,std::make_pair(mode,hovl));
    HSS_INFO* psecond=pp.second;
	

   // HSS_INFO* psecond=get_second_or_set_first(pipename,hovl);
	if(psecond)
	{
		SOCKET s1=hss_reset_socket(hovl,HSF_DETACH);
		SOCKET s2=hss_reset_socket(psecond,HSF_DETACH);
		int m2=pp.first;
		
		char* peer2=hss_reset_SOCKET_DATA(hovl,HSD_REMOTE_INFO);
		char* peer1=hss_reset_SOCKET_DATA(psecond,HSD_REMOTE_INFO);
		
		//v_buf<char> info;
		 // info.printf
		pipe_console_t::log_printf("connected: peep[%s](%d) <=> \"%s\" <=> peep[%s](%d)\n",peer1,mode,pipename,peer2,m2);

		psr->push_pair(s1,s2,pipename);
		hss_release(psecond);
	}
 }

   socket_port_pipe_t(wchar_t* portname,double nthreads=-1):psr(0),hport(0),hclosure(0),hclosure_disconnect(0),conn_count(0),hr(E_FAIL)
   {
      vportname.cat(portname);
      hclosure=hss_create_stdcall_closure(&s_on_accept,this);
	         
	  if(!hclosure) return;
	  hclosure_disconnect=hss_create_stdcall_closure(&s_on_disconnect,this);
	  if(!hclosure_disconnect) return;
	  hport=hss_create_socket_portW(portname,HSF_SRD_AUTO,hclosure,HSD_SAME_PTR);
	  if(!hport) return;

	  psr=pair_socket_reflector_t<_bufsize>::create(nthreads);
	  if(!psr) return;
       
	  psr->disconnect_proc=(pair_socket_reflector_t<_bufsize>::disconnect_proc_t)hclosure_disconnect;
	  pipe_console_t::title_printf(L"%s",vportname.get());
	  //disco

   }
   ~socket_port_pipe_t()
   {
	   
	   hss_release(psr);
	   hss_release(hport);
	   hss_release(hclosure);
	   hss_release(hclosure_disconnect);
	   for(pipemap_t::iterator i=pipemap.begin();i!=pipemap.end();++i) 
		   hss_release(i->second.second);
   }

   static void on_exit(socket_port_pipe_t* p)
   {
	   p->~socket_port_pipe_t();
   };


   template <class CH>
   static socket_port_pipe_t*  create(CH* portname,int nthreads=-2)
   {	   
	   socket_port_pipe_t* port=new (hss_alloc(sizeof(socket_port_pipe_t))) 
		   socket_port_pipe_t(char_mutator<CP_UTF8>(portname),nthreads);
	   hss_weak_ref(port,&socket_port_pipe_t::on_exit,port);
	   void** ppi=hss_info_ll(port);
	   ppi[5]=hss_get_signaler(port->hport);
	   return port;
   }


};


#define  CP_X CP_THREAD_ACP
#define s_m(s) char_mutator<CP_X>(#s)
#define _m(s) char_mutator<CP_X>(s)

template <class ARGS>
HRESULT pipe_service_run(ARGS& args) 
{
	HRESULT hr;
	int cm=args[s_m(console.mode)].def(1);
	if(cm)
	{
		if(cm&2) AttachConsole(ATTACH_PARENT_PROCESS);
		AllocConsole();
	}

	double numthread=args[s_m(nthreads)].def<double>(-1);
	int tio=args[s_m(timeout)].def(int(INFINITE));

	socket_port_pipe_t<>* spp=0;

	ARGS::char_type* pn=args[s_m(port)];
	if(!args)
		pn=args[s_m(portname)];
	spp=socket_port_pipe_t<>::create(pn,numthread);


/*
	if(args[s_m(port)].is())
	  spp=socket_port_pipe_t<>::create((ARGS::char_type*)args[s_m(port)],numthread);
	else
		spp=socket_port_pipe_t<>::create((ARGS::char_type*)args[s_m(portname)],numthread);
*/
	hr=hss_error();
	
	if(FAILED(hr)) return hr;
	hss_wait_event(spp);

	//int c;
	bool f=tio>=0;
	while(int c=spp->conection_count())
	{
		//pipe_console_t::title_printf(L"connection count=%d",c);
		if(f)
		{
			tio-=200;

			if(tio<0) break;
		}
		Sleep(200);
	}

	hss_release(spp);

	return hr;

}
