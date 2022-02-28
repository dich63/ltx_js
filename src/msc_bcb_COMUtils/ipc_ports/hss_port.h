#pragma once


#include "socket_utils.h" 

#include "pipe_port.h"
//#include "handle_array.h"
#include "shared_closures.h"
#include "heap_allocator.h"
#include "shared_allocator.h"
#include "wchar_parsers.h"
#include "video/singleton_utils.h"

#include <intrin.h>
#include "closures.h"
#include "tls_dll.h"
#include <Shlwapi.h>
#include <strstream>
#include "wbs_buffer.h"


#include "hss_port_types.h"


typedef hss_port_types::region_t hss_region_t;
typedef hss_port_types::HANDSHAKE_DATA_t hss_HANDSHAKE_DATA_t;

//#include "wbshandshake.h" 
#include <list>


//#pragma comment(lib ,"Ws2_32.lib")

#pragma pack(push)
#pragma pack(1)



// {53001C5B-EDD4-4122-B0B4-67DF3FC2C4D5}



template <class PT>
ptr_holder<char>  make_callback_proc(void *proc,void* proch,PT*& on_proc)
{

	ptr_holder<char> proc_holder ;
	void* p;
    DWORD e=hss_error();
	if(proc==HSD_IO_PORT_PTR)

		proc_holder.pv=on_proc=(PT*)hss_create_completion_port_closure(0,ULONG_PTR(proch));

	else
	{

		if(p=hss_get_io_signaler(proc))
		{
			proc_holder.pv=on_proc=(PT*)hss_create_completion_port_closure(p,ULONG_PTR(proch));

		}
		else
		{
			on_proc=(PT*)proc;
			proc_holder=(char*)((proch==HSD_SAME_PTR)?proc:proch);
		}
	}
	
	set_hss_error(e);
	return proc_holder;

};





typedef hs_socket_utils::handle_holder handle_holder;





//__declspec(selectany) ___s_crt hss___crt32__;


struct hss_port_base
{
	typedef unsigned char ibyte_t;
	enum
	{
		REGION_COUNT =HSS_BUF_COUNT
	};

	typedef    hs_socket_utils::LastErrorSaver LastErrorSaver;

	struct region:hss_region_t   {
		/*
		union	   {
			struct{long long sign;};
			struct{long pid[2];};
			struct{SOCKET s;long pid_srv;};
			struct   {
				long sizeb;
				long offset;	   
					};
			
		};
		*/
		//region(long o=0,long s=0):offset(o),sizeb(s){};
		region(long o=0,long s=0){offset=(o);sizeb=(s);};
	};

	struct region_ptr
	{
		union{
			struct{

				//unsigned 
					long sizeb;
				char* ptr;
			};
			struct   { void* pv[2];};
			WSABUF wsabuf;
		};
		region_ptr():ptr(0),sizeb(0){};
		region_ptr(hss_region_t r,void* p=0)
		{
			sizeb=r.sizeb;
			ptr=(sizeb)?((char*)p)+r.offset:(char*)0;
		};
		inline  operator bool(){ return ptr;}
  
       
	};

	struct handshake_buffer:hs_socket_utils::socket_events::OVERLAPPED_ex //OVERLAPPED
	{
		//typedef handshake_buffer  base_t;
		


		SOCKET s;

		ULONG_PTR fclose_after;
		ULONG_PTR reply_state;
		

		//unsigned long pid;
        //unsigned long pid_host;
		
		union{
		
		struct{         
         
         region_ptr url;
		 region_ptr host;
		 region_ptr params;
		 region_ptr confirm_reply;
		 region_ptr request;
		 region_ptr error_reply;
		 region_ptr url_list;
         
		 	};
		struct{
			region_ptr regions[REGION_COUNT];
		};
		};

         void *abortproc; 
		 void *pcallback;

		inline size_t req_size()
		{
			size_t c=0;
			for(size_t n=0;n<REGION_COUNT;n++)
				c+=regions[n].sizeb;
			return c;

		}

/*
        long set_flags(long f)
		{
           return InterlockedExchange((LONG volatile*)&fclose_after,f); 
		}
		
*/
		//pt->p.tn[0]=base_typeid((T*)0);

		

	};

typedef	hs_socket_utils::CS CS;
	
	template <class CS>
	struct locker_t:  hs_socket_utils::locker_t<CS>
	{
		locker_t(CS& c):hs_socket_utils::locker_t<CS>(c){};
	};

	


static	inline region calc_region(void* pbase,void*p ,int cb)
	{
		region r;
		r.offset=int(p)-int(pbase);
		r.sizeb=cb;
		return r;
	}



inline static HRESULT send_region(SOCKET s, region_ptr& r,HANDLE hAbortEvent=INVALID_HANDLE_VALUE)
{
   if(r.sizeb)
	   return hs_socket_utils::send_WSABUF(s,1,&r.wsabuf, hAbortEvent);
   else return S_OK;
}

inline static HRESULT send_region_wbs_string(SOCKET s, region_ptr& r,HANDLE hAbortEvent=INVALID_HANDLE_VALUE)
{
	WSABUF wsb[3]={{1,"\x00"},r.wsabuf,{1,"\xFF"}};
		return hs_socket_utils::send_WSABUF(s,3,wsb, hAbortEvent);
	
}


};


ptr_holder<char> get_recv_lines(SOCKET s)
{
	hss_port_base::LastErrorSaver lerr;

	
	struct sockgetter
	{
           SOCKET s;
	   sockgetter(SOCKET _s):s(_s){};

     inline int operator()(char* p,int count) const
	 {
		 return ::recv(s,p,count,0);
	 }

	} ;



	std::strstream stream;

	int cb=get_recv_lines(stream,sockgetter(s));

	if(cb<0) return lerr=WSAGetLastError(),  0;


	ptr_holder<char> ph=shared_heap<char>().allocate_holder(cb+32);
	i_handle::size_ref(ph)=cb;
	memcpy(ph.p,stream.str(),cb);
	return ph;
};



SOCKET tcp_connect_by_string(char* address_port,HANDLE habortevent=INVALID_HANDLE_VALUE)
{

	hs_socket_utils::host_port_resolver hpr(address_port,habortevent);

	   hpr.connect();
	   if(hpr)
		  return hpr.detach();

	return INVALID_SOCKET;
}


inline ptr_holder<char> create_shared_buffer(int cb,void* p=0,int cbp=-2)
{
	ptr_holder<char> pbuf= shared_heap<char>().allocate_holder(cb);
	if(cbp==-2) cbp=cb;
	if((p)&&(cbp<=cb)) memcpy(pbuf.p,p,cbp);
	return pbuf;
}



ptr_holder<char>  unicode_to_char(wchar_t* s_u,int len=-1,unsigned int CP_XX=CP_UTF8)
{

	hss_port_base::LastErrorSaver lerr; 
	ptr_holder<char> s_c;
	int cb=0;

	if(len==0)
	{
		return shared_heap<char>().allocate_holder(1);

	}
	if(s_u)	{


		if(len<0) len=wcslen(s_u)+1;
		
		cb=WideCharToMultiByte(CP_XX,0,s_u,len,0,0,0,0);
		if(cb){
			s_c=shared_heap<char>().allocate_holder(cb);
			cb=WideCharToMultiByte(CP_XX,0,s_u,len,s_c,cb,0,0);
		} 
	}
	if(!cb) lerr.set();

	return s_c;
};

ptr_holder<wchar_t> char_to_unicode(char* s_utf,int len=-1,unsigned int CP_XX=CP_UTF8)
{
	hss_port_base::LastErrorSaver lerr; 
	ptr_holder<wchar_t> s_wc;
	wchar_t c[2];

	if(len==0)
	{
		return shared_heap<wchar_t>().allocate_holder(1);

	}
	if(s_utf)
	{

		if(len<0) len=strlen(s_utf)+1;
		//len=-1;
		int cb=MultiByteToWideChar(CP_XX,0,s_utf,len,c,0);
		if(cb)
		{
			s_wc=shared_heap<wchar_t>().allocate_holder(cb);
			int cbe=MultiByteToWideChar(CP_XX,0,s_utf,len,s_wc,cb);
		}
		lerr.set();
	}
	return s_wc;

};


inline ptr_holder<wchar_t> vsh_printf(wchar_t* fmt, va_list argptr)
{
	_bstr_t res;
	int cb=_vscwprintf(fmt,argptr);
	ptr_holder<wchar_t> pbuf= shared_heap<wchar_t>().allocate_holder(cb+1);
	vswprintf(pbuf,cb+1,fmt, argptr);
	return pbuf; 
}
inline ptr_holder<wchar_t>  sh_printf(wchar_t* fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	return vsh_printf(fmt,argptr); 
}

inline ptr_holder<char> vsh_printf(char* fmt, va_list argptr)
{
	_bstr_t res;
	int cb=_vscprintf(fmt,argptr);
	ptr_holder<char> pbuf= shared_heap<char>().allocate_holder(cb+1);
	vsprintf(pbuf,fmt, argptr);
	return pbuf; 
}
inline ptr_holder<char>  sh_printf(char* fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	return vsh_printf(fmt,argptr); 
}

template <class ARGLIST>
inline wchar_t* get_hssh_list(wchar_t* name,wchar_t* hnn,ARGLIST& args)
{
	wchar_t buf[1024];
	swprintf(buf,L"%s[%s]",hnn,name);
	//argv_env<wchar_t> args;
	//wchar_t *ev=_wgetenv(buf);
	wchar_t *ev=args[buf];
	if(!ev)
	{
		   swprintf(buf,L"%s[\"%s\"]",hnn,name);
		 //ev=_wgetenv(buf);
		 ev=args[buf];
		if(!ev)
		{
			swprintf(buf,L"%s[\'%s\']",hnn,name);
			//ev=_wgetenv(buf);
			ev=args[buf];
		}
	}
 return ev;
}

inline v_buf<wchar_t>& get_hssh_list_all(wchar_t* name,wchar_t* hnn,v_buf<wchar_t>& buf=v_buf<wchar_t>())
{
       argv_env_r<wchar_t> arg_env;
	   argv_reg<wchar_t> arg_reg_usr(L"Software\\hssh\\namespaces");
	   argv_reg<wchar_t> arg_reg_sys(L"SOFTWARE\\hssh\\namespaces",HKEY_LOCAL_MACHINE);
       argv_ini<wchar_t> arg_ini(L"namespases.win",L".ini");
	   arg_env.parent=&arg_reg_usr;
	   arg_reg_usr.parent=&arg_reg_sys;
	   arg_reg_sys.parent=&arg_ini;
	   wchar_t* env= get_hssh_list(name,hnn,arg_env);
	   return file_make_dir(env,false,buf);
	   //return expand_env(env,buf);
}

ptr_holder<wchar_t> parseNS(wchar_t* wurl,int* perr=0)
{
	ptr_holder<wchar_t> tmp;

     if(perr) *perr=-1;   
	if(!wurl) return 0;
	//wchar_t *p=wcsrchr(wurl,L':');
	 
      tmp=shared_heap<wchar_t>().allocate_holder(safe_len(wurl,1));
	  wcscpy(tmp,wurl);
	  wurl=tmp;
	 wchar_t *p; 

	 //if(p=find_iter("[::]",4,pnb,pne))
	 if(p=StrStrW(wurl,L"=::"))
		 *p=0;
	 else
	 {
     p=wcschr(wurl,L';');
	 if(p) *p=0;
	 }

	 p=StrRStrIW(wurl,0,L"::");
	 
	 if(!p) {  return 0;}
	else
      {
        
		  //int cb=int(p)-int(wurl);
		  //tmp=shared_heap<wchar_t>().allocate_holder(cb+1);
		  //memcpy(tmp.p,wurl,cb);
		  
		  //wurl=tmp;
		  //wurl[cb]=0;
		  *p=0;p+=2;
		  if(wurl[0]==L'^') ++wurl;
          
	   }
	while((*wurl)&&(wurl[0]==L'/')) wurl++;
	//StrStrIW(p,L"hssh:");

	wchar_t buf[1024];

//	argv_env<wchar_t> arg_env;
//	wchar_t *ev=get_hssh_list(wurl,L"hssh.namespaces",arg_env);

	v_buf<wchar_t> tmpbuf;
	wchar_t *ev=get_hssh_list_all(wurl,L"hssh.namespaces",tmpbuf);

	//swprintf(buf,L"hssh.namespaces[%s]",wurl);
	//wchar_t *ev=_wgetenv(buf);
	if(!ev)  { if(perr) *perr=1; return 0;}

	ptr_holder<wchar_t> res=sh_printf(L"%s/%s",ev,p);
    if(perr) *perr=0;   
	return res;
}

wchar_t* reparseNS(wchar_t* wurl,int* perr=0,ptr_holder<wchar_t>& keep=ptr_holder<wchar_t>())
{
	if(wurl==HS_PROCESS_PORT_BY_FACTORY_W) 
	{
		wurl=_wgetenv(HSS_ENV_PROCESS_PORT_BY_FACTORYW);
	}
   keep= parseNS(wurl,perr);
   if(keep.p) return keep;
   else return wurl;
};



template <int INSTANCES=2,size_t _BUFSIZE= 2*4096>
struct hss_port:hss_port_base
{



	enum
	{
     BUFSIZE=_BUFSIZE
	};
typedef typename hss_port<INSTANCES,BUFSIZE> self_type;
typedef typename self_type* this_ptr;
typedef typename pipe_port<false,INSTANCES,BUFSIZE,1,void*> pipe_port_t;



struct HANDSHAKE_DATA:hss_HANDSHAKE_DATA_t
{
	typedef hss_region_t region;
inline   static  region& sign(char* s="HS>||<SH")
{
	//static char s[]="HS>||<SH";
	return *((region*)s);
}
//
/*   
	union{
		
		struct
		{
          long sizeb;
		  HRESULT crc32;
		  char prefix[8];
		  long flags;
		  long err_code;
		};
		struct
		{
          region crc32_size;
		  region signature;
		  region flags_err;
		  region pids;
		  region cmd;
		  region fd_data;
        
		  union{
			  struct{
    	  region urn_data;
		  region host_data;
		  region params_data;
		  region replay_data;
		  region request_data;
		  region error_data;
          region url_list;
			  };
			  struct{
				  region regions[REGION_COUNT];
				  };

		  };
		  SOCKET fd,fds;//INVALID_SOCKET 
		  union{
		  WSAPROTOCOL_INFOW wsadata;
		  OVERLAPPED ovl;
		  struct{
			  HANDLE shared_handles[16];
		  };
		  };
		  char buf[BUFSIZE/2];
		};
		struct{char _c[BUFSIZE];};
	};
//*/
///*
HANDSHAKE_DATA()
{  memset(_c,0,BUFSIZE); }
//*/

inline WSAPROTOCOL_INFOW* get_socket_info()
{
	if(!check_signature()) return NULL;
	region_ptr rfd(fd_data,this);
	return (rfd.sizeb)?(WSAPROTOCOL_INFOW*)rfd.ptr:(WSAPROTOCOL_INFOW*)NULL;
};

inline bool check_signature()
{
  return sign().sign==signature.sign;
}

inline SOCKET  get_socket()
{
	
	if(check_signature())
	{
     
    if(fd==INVALID_SOCKET)
	{
	
	WSAPROTOCOL_INFOW* pi=get_socket_info();
	if(pi) {
		fd=WSASocketW(FROM_PROTOCOL_INFO,FROM_PROTOCOL_INFO,FROM_PROTOCOL_INFO,pi, 0, FROM_PROTOCOL_INFO);
		
		}
	}
	return  fd;
	}
	return INVALID_SOCKET;
}
  
inline SOCKET  detach_socket()
 {
     SOCKET s=get_socket();
      fd=INVALID_SOCKET;
	  fd_data.sizeb=0; 
	 return s;
 }
};

//typedef typename HANDSHAKE_DATA::region_ptr region_ptr;
//typedef typename HANDSHAKE_DATA::region region;





struct HANDSHAKE_DATA_holder
{
  //void *reserved;
  HANDSHAKE_DATA hsd;
  void* abortproc;
  int replaymode;
  void* pcontext;
  HANDSHAKE_DATA_holder():abortproc(0),replaymode(0)
  {
	  hsd=HANDSHAKE_DATA();
     //reset();
  };

inline operator HANDSHAKE_DATA&()
{
	return hsd;
}

void  reset() const
{
   HANDSHAKE_DATA_holder* t=const_cast<HANDSHAKE_DATA_holder*>(this);
   //t->hsd.fd_data.sizeb=0;
   t->hsd.crc32_size.sign=0;
   t->hsd.signature.sign=0;
}

inline void copy_from(const HANDSHAKE_DATA_holder& hs)
{
	hsd=hs.hsd;
	hs.reset();
}

HANDSHAKE_DATA_holder(const HANDSHAKE_DATA& h)
{
	hsd=h;
}

 HANDSHAKE_DATA_holder(const HANDSHAKE_DATA_holder& hs)
 {
    copy_from(hs);
 }


HANDSHAKE_DATA_holder(HANDSHAKE_DATA_holder& hs)
{
   copy_from(hs);
}
void operator=(const HANDSHAKE_DATA_holder& hs)
{
    copy_from(hs);
}
void operator=(HANDSHAKE_DATA_holder& hs)
{
	copy_from(hs);
}

SOCKET  get_socket()
  { 
	  SOCKET fd=hsd.get_socket();
    return  fd;
  }
inline SOCKET  detach_socket()
{
	SOCKET fd=hsd.detach_socket();
	return  fd;
}
region_ptr get_request_data()
{
  return region_ptr(hsd.request_data,&hsd);	
}

region_ptr get_replay_data()
{
	return region_ptr(hsd.replay_data,&hsd);	
}

region_ptr get_error_data()
{
	return region_ptr(hsd.error_data,&hsd);	
}
region_ptr get_params_data()
{
	return region_ptr(hsd.params_data,&hsd);	
}

region_ptr get_region_ptr(int n)
{
	return region_ptr(hsd.regions[n],&hsd);	
}

 ~HANDSHAKE_DATA_holder()
 {
	 
	 SOCKET fd=get_socket();
   if(fd!=INVALID_SOCKET) 
	   closesocket(fd);
 }
 bool cmd_is(const char*p)
 {
	 long siz=hsd.cmd.sizeb;
	 if(siz==0) return false;
	 char * ps=region_ptr(hsd.cmd).ptr;
	 if(p&&ps)
	 {
		 return StrCmpNIA(ps,p,siz)==0;
	 }
	 return false;
 };

 bool cmd_is(int cmd_id)
 {
	 if(hsd.cmd.sizeb) return false;
	 return hsd.cmd.offset==cmd_id;
 }

};

typedef typename ptr_holder<HANDSHAKE_DATA_holder> hd_holder;

typedef typename std::list<hd_holder> socked_list_t;

typedef  void ( *interceptor_proc_t)(void* pcontext,HANDSHAKE_DATA_holder* phdh, bool * phandled );	
typedef  void ( *exiter_proc_t)(void* pcontext);	


    //CS mutex;  
	//socked_list_t socked_list_;

	struct synchro_list_t
	{
	     
        socked_list_t socked_list;
		CS mutex;  
		handle_holder sem;
		handle_holder portevent;
		handle_holder acceptevent;
		ptr_holder<event_signaler> port_event_signaler;
		bool fport_destroy_notify;
		int replaymode;

        interceptor_proc_t interceptor_proc;
		ptr_holder<char> interceptor_context_holder;
		char* pinterceptor_context; 
		void* pcmd_context;

		inline void lock(){ mutex.lock();}
		inline void unlock(){ mutex.unlock();}


		//i_handle::completion_port_ref(pss)=pss->get_io_signaler();

		inline void* get_io_signaler()
		{
			void* p=interceptor_context_holder.p;
			return p=i_handle_cast<completion_port_signaler>(p);
		}
        synchro_list_t(interceptor_proc_t _interceptor_proc=0,void *pcontext=0,void* hcontext=HSD_SAME_PTR)
		{
			replaymode=0;
			pcmd_context=0;
			fport_destroy_notify=1;
			  interceptor_proc= _interceptor_proc;
              interceptor_context_holder=make_callback_proc(pcontext,hcontext,pinterceptor_context);

		/*	
          interceptor_proc= _interceptor_proc;
		  if(pcontext==HSD_IO_PORT_PTR)
			  interceptor_context_holder.pv=pinterceptor_context=(char*)hss_create_completion_port_closure(0,ULONG_PTR(hcontext));
		  else
		  {
		   pinterceptor_context=(char*)pcontext;



		   interceptor_context_holder=(char*)((hcontext!=HSD_SAME_PTR)?hcontext:pcontext);
  */
         /*
	     if(interceptor_context_holder)
		   {
			   void* *pIO=hss_get_io_signaler(interceptor_context_holder);

		   }

		   

		  }
		  */
		   //interceptor_context_holder=(char*)hcontext;

          sem.m_h=CreateSemaphore(NULL,0,0x7FFFFFFF,NULL);

		 // if(interceptor_proc) port_event_signaler=make_event_signaler(portevent);
		}

inline		void set_port_event(HANDLE hevent)
{
         portevent.dup_from(hevent);
		 if(interceptor_proc) port_event_signaler=make_event_signaler(hevent);

}
		inline long push( HANDSHAKE_DATA_holder* phsd)
		{

			locker_t<CS> lock(mutex);
               
		    socked_list.push_back(phsd);
			LONG lp;
			ReleaseSemaphore(sem,1,&lp);
			WSASetEvent(acceptevent);
			return socked_list.size();
		}
       inline unsigned long pop( HANDSHAKE_DATA_holder** pphsd,long to=INFINITE,bool falertable=false)
	   {
		   if(!pphsd) return -1;
		   HANDLE hh[2]={sem,portevent};
		   DWORD ws=WaitForMultipleObjectsEx(2,hh,false,to,falertable);
		   if(ws==WAIT_OBJECT_0)
		   {
			  locker_t<CS> lock(mutex);
               *pphsd=socked_list.front().detach();
                socked_list.pop_front();
		   }

		   return ws;



	   }
		 

	} ;
    
    ptr_holder<synchro_list_t> synchro_list;
	hs_socket_utils::file_handle_list_t   file_handle_list;

	//HANDLE hsem;
	std::pair<HANDLE,unsigned int> port_pair;

inline socked_list_t & socked_list()
{
	return synchro_list->socked_list;
   //return socked_list_;
}

inline HANDLE semaphore()
{
  return synchro_list->sem;
}

#define this_lock locker_t<CS> _ll_000__00_(synchro_list->mutex);

/*
static inline unsigned long Crc32(unsigned char *buf, unsigned long len)
	{

		unsigned long crc = 0xFFFFFFFFUL;
		while (len--) 
			crc = hss___crt32__.crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);

		return crc ^ 0xFFFFFFFFUL;

	};

*/

static  unsigned long Crc32(HANDSHAKE_DATA& hsd)
{
	const int c=sizeof(region);
	unsigned char *buf=((unsigned char *)&hsd)+c;
     int sb=hsd.crc32_size.sizeb;
	 return hs_socket_utils::Crc32(buf,sb-c);
	//return ::Crc32(buf,sizeof(HANDSHAKE_DATA)-c);
}


static bool setCrc32(HANDSHAKE_DATA& hsd)
{
 if(hsd.crc32_size.sizeb>BUFSIZE) return false;
  hsd.crc32_size.offset=Crc32(hsd);
  return true;
};



bool check_hs(HANDSHAKE_DATA& hsd)
{
    //bool f=(hsd.signature.sign==hsd.sign().sign);
	bool f=hsd.check_signature();
	f=f&&(hsd.crc32_size.sizeb<=BUFSIZE);
	f=f&&(hsd.crc32_size.offset==Crc32(hsd));
	f=f&&(hsd.pids.pid[1]==GetCurrentProcessId()); 
   return f;
}


static inline BOOL wait_pipe(wchar_t* pn,int to)
{
  return WaitNamedPipeW(pn,to);
}
static inline BOOL wait_pipe(char* pn,int to)
{
	return WaitNamedPipeA(pn,to);
}

template <class CH>
static HRESULT set_to_process(HANDSHAKE_DATA& hsd,int pid,SOCKET s,handshake_buffer* phsptr,CH* pipename=0)
{
        handshake_buffer& hs=*phsptr;
         int cbdata=hs.req_size();
        int cbfull=int(hsd.buf)-int(&hsd)+cbdata+REGION_COUNT*2;
		bool f=sizeof(HANDSHAKE_DATA)>cbfull;
		if(!f) return E_FAIL;

		hsd.pids.pid[0]=GetCurrentProcessId(); 
		hsd.pids.pid[1]=pid;

         void* pbase=&hsd;
		 if(pipename)
		 {
			 BOOL f=wait_pipe(pipename,5000);
			 if(!f)
			 {

				 hsd.crc32=HRESULT_FROM_WIN32(GetLastError());
				 if(hs.fclose_after&2)   {hs.s=INVALID_SOCKET; closesocket(s);}
				 return hsd.crc32;
			 }
		 }

	 if((hsd.cmd.sizeb==0)&&((hsd.cmd.offset==0)||(hsd.cmd.offset==2)))
	 {

		if(SOCKET_ERROR==WSADuplicateSocketW(s,pid,&hsd.wsadata))
		{
			hsd.signature=hsd.sign();
			hsd.sizeb=cbfull;
			hsd.err_code=HRESULT_FROM_WIN32(GetLastError());
            setCrc32(hsd);
    		return hsd.err_code;
		}
		if(hs.fclose_after&2)   {hs.s=INVALID_SOCKET; closesocket(s);}
		
		
	 }
	 else 	 if((hsd.cmd.sizeb==0)&&(hsd.cmd.offset==1))
	 {
		 if(s!=INVALID_SOCKET)
		 hsd.shared_handles[0]=hs_socket_utils::dup_handle(pid,HANDLE(s));
	 }

	 ;
	    hsd.fd=INVALID_SOCKET;
		
		hsd.fd_data=calc_region(pbase,&hsd.wsadata,sizeof(hsd.wsadata));

			//region(offsetof(HANDSHAKE_DATA,wsadata),sizeof(hsd.wsadata));

		 char* p=hsd.buf;
         for(int n=0;n<REGION_COUNT;n++)
		 {
             region_ptr& rc=hs.regions[n];
			 size_t sb=rc.sizeb;
			 if((sb)&&(rc.ptr))
			 {
                 memcpy(p,rc.ptr,sb);
               hsd.regions[n]=calc_region(pbase,p,sb);
			   p+=sb+2;
			 }
 		 }
		
	 hsd.signature=hsd.sign();
	 hsd.sizeb=cbfull;

	 setCrc32(hsd);

     return  S_OK;

}

/*
static HRESULT set_to_process(HANDSHAKE_DATA& hsd,int pid,SOCKET s,const char* req,const char* rep,const char* reperr=0,const char* params=0)
{
    handshake_buffer hsb= handshake_buffer();

	int cbreq=safe_len(req);
	int cbrep=safe_len(rep);
	int cbreperr=safe_len(reperr);
	int cbparams=safe_len_zerozero(params);


	return 0;
};
*/

static HRESULT set_to_process(HANDSHAKE_DATA& hsd,int pid,SOCKET s,const char* req,const char* rep,const char* reperr=0,const char* params=0)
{

     int cbreq=safe_len(req);
	 int cbrep=safe_len(rep);
	 int cbreperr=safe_len(reperr);
	 int cbparams=safe_len_zerozero(params);
	 int cbfull=int(hsd.buf)-int(&hsd)+cbreq+cbrep+cbreperr+cbparams+16;
	 bool f=sizeof(HANDSHAKE_DATA)>cbfull;
	 if(!f) return E_FAIL		 ;

     if(SOCKET_ERROR==WSADuplicateSocketW(s,pid,&hsd.wsadata))
		 return hsd.crc32=HRESULT_FROM_WIN32(GetLastError());
	    hsd.fd=INVALID_SOCKET;
	    hsd.pids.pid[0]=GetCurrentProcessId(); 
		hsd.pids.pid[1]=pid;
	    hsd.fd_data=region(offsetof(HANDSHAKE_DATA,wsadata),sizeof(hsd.wsadata));
		char* p=hsd.buf;
		hsd.request_data=region(int(p)-int(&hsd),cbreq);
        p=safe_copy(p,req,cbreq);
		p++;
		hsd.replay_data=region(int(p)-int(&hsd),cbrep);
        p=safe_copy(p,rep,cbrep);
		p++;
		hsd.error_data=region(int(p)-int(&hsd),cbreperr);
		p=safe_copy(p,reperr,cbreperr);
		p++;
		hsd.params_data=region(int(p)-int(&hsd),cbparams);
        p=safe_copy(p,params,cbparams);

        hsd.signature=hsd.sign();
		hsd.sizeb=cbfull;

	   setCrc32(hsd);
	return  S_OK;
}

 static inline HANDSHAKE_DATA to_process(int pid,SOCKET s,handshake_buffer* phsptr)
{
     HANDSHAKE_DATA hsd=HANDSHAKE_DATA();
        set_to_process(hsd, pid, s, phsptr);
		return hsd;
}


 template<class CH>
 static inline HRESULT send_to_process(CH* pipename,int pid,SOCKET s,handshake_buffer* phsptr,region cmd=region())
 {

	 struct  BOUT
	 {
		 int err;
		 WSAPROTOCOL_INFOW wsainfo;

	 } ;


	 HANDSHAKE_DATA hsd=HANDSHAKE_DATA();
	 hsd.cmd=cmd;
	 HRESULT  hr=set_to_process(hsd, pid, s,phsptr,pipename);
	 
	 if(hr)
	 {

	 }
	 {
		 //inline static int send_recv(const char* sPipename,void* bufin,int cbr,void* bufout=0,int* pcbw=0,int to=0)
		 int cb=sizeof(int),err=0;
		 if((cmd.offset==2)&&(cmd.sizeb==0))
		 {
			 
			 //v_buf<BOUT> buf(1);
			 //BOUT& bout=*buf.get();
			 BOUT bout;
			 
                cb=sizeof(BOUT);
			 hr=pipe_port_t::send_recv(pipename,&hsd,hsd.sizeb,&bout,&cb,5000);
			 
			 if(SUCCEEDED(hr))
			 {
				
				 err=bout.err;
				 if(err==0)
				 {
					 closesocket(s);
					 s=WSASocketW(FROM_PROTOCOL_INFO,FROM_PROTOCOL_INFO,FROM_PROTOCOL_INFO,&bout.wsainfo, 0, FROM_PROTOCOL_INFO);
					 phsptr->s=s;
 				 }

			 }
			 

		 }
		 else  hr=pipe_port_t::send_recv(pipename,&hsd,hsd.sizeb,&err,&cb,5000);
		 if((SUCCEEDED(hr))&&  (cb>=sizeof(int))) 
			 hr=HRESULT_FROM_WIN32(err);

	 }
	 //   hr=HRESULT_FROM_WIN32(GetLastError());

	 return hr;
 }



 template<class CH>
static inline HRESULT send_to_process(CH* pipename,int pid,SOCKET s,const char* req,const char* rep,const char* reperr=0,const char* params=0)
 {
	 HANDSHAKE_DATA hsd=HANDSHAKE_DATA();
	 HRESULT  hr=set_to_process(hsd, pid, s, req, rep, reperr,params) ;
	 //if(SUCCEEDED(hr))
	 hsd.err_code=hr;
	 {
		 //inline static int send_recv(const char* sPipename,void* bufin,int cbr,void* bufout=0,int* pcbw=0,int to=0)
		 int cb=sizeof(int), err=0;
          
		  hr=pipe_port_t::send_recv(pipename,&hsd,hsd.sizeb,&err,&cb,5000);
		  if((SUCCEEDED(hr))&&  (cb>sizeof(int))) 
			  hr=HRESULT_FROM_WIN32(err);

	 }
		 //   hr=HRESULT_FROM_WIN32(GetLastError());
	 
	 return hr;
 }


inline int get_HANDSHAKE_DATA(HANDSHAKE_DATA_holder* phsd=0,int to=INFINITE,bool falertable=true)
{
	HANDLE hh[2]={semaphore(),port_pair.first};
     DWORD ws=WaitForMultipleObjectsEx(2,hh,false,to,falertable);
	 if(ws==WAIT_OBJECT_0)
	{
      pop_HANDSHAKE_DATA(phsd);
	}
	 
	 return ws;
}



class replay_socket
{

  ptr_holder<synchro_list_t> synchro_list;
  hd_holder hd;
/*
  struct OVERLAPPED_ex:OVERLAPPED
  {
	  HANDLE hEvent2;
  }	  ovl;
*/
  replay_socket(synchro_list_t *plist,HANDSHAKE_DATA_holder* phd)
  {
      synchro_list=plist;

	  if(phd==0) 
		  return;
	  hd=phd;
	  hd->replaymode=synchro_list->replaymode;
/*	  ovl=OVERLAPPED_ex();
	  ovl.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
	  ovl.hEvent2=synchro_list->portevent;*/

  }

  ~replay_socket(){};// CloseHandle(ovl.hEvent);}




inline  void tproc()
{
   
   

   bool fhandled=false;
   

   if(synchro_list->interceptor_proc) {

	   if(hd)
	   {
		  //locker_t<synchro_list_t> lock(*(synchro_list.p));
          hd->abortproc=synchro_list->port_event_signaler;
		  hd->pcontext=synchro_list->pcmd_context;
	   }
	   
	   bool f=(hd)||(synchro_list->fport_destroy_notify);
	    if(f) closure_stub::safe_call(synchro_list->interceptor_proc,synchro_list->pinterceptor_context,hd,&fhandled);
		hd.release();
         //synchro_list->interceptor_proc(synchro_list->pinterceptor_context,hd,&fhandled);
   }
   if(!fhandled)
   {
	   if(hd)
	   {
 
	   SOCKET s=hd->get_socket();
	   if(s==INVALID_SOCKET) return;
	    hd->abortproc=synchro_list->port_event_signaler;
	   if(SUCCEEDED(hss_port_base::send_region(s,hd->get_replay_data(),synchro_list->portevent)))
		   synchro_list->push(hd);
	   }
	   else synchro_list->push(0);
   }	
   
/*
   struct cleaner
   {  
	     SOCKET s;
		 ~cleaner(){CancelIo(HANDLE(s));} 
   } _cls={s};

   WSABUF wsabuf=hd->get_replay_data().wsabuf;

   int rc,err;
   if(wsabuf.len)
   {
         DWORD cb=0,Flags=0;
       rc=WSASend(s,&wsabuf,1,&cb,0,&ovl,0);
	   if ( (rc) && 
		   (WSA_IO_PENDING != (err = WSAGetLastError()))) 
		   return;


	   while(WSA_WAIT_IO_COMPLETION==(rc = WSAWaitForMultipleEvents(2, &ovl.hEvent,FALSE,INFINITE,TRUE))){};

  	   if(rc!=WSA_WAIT_EVENT_0) return;

	   rc = WSAGetOverlappedResult(s, &ovl, &cb, FALSE, &Flags);
	   if(rc==FALSE) return ;
   }

   	   synchro_list->push(hd);
*/
   
};

  static unsigned int __stdcall s_proc(void* p)
  {
	  //Sleep(10000);
	  try{

       replay_socket* tp=(replay_socket*)p;

	   try{    tp->tproc();  } catch(...){

	   }

        delete tp;

	   }  catch(...) {

	  }
	  return 0;
  }

  long run()
  {
	  if(this)
	  {

	  
	  unsigned int tid;
       handle_holder ht= (HANDLE)_beginthreadex(NULL,0,&s_proc,this,CREATE_SUSPENDED,&tid);
	   if(ht==0) delete this;
	   else
	   {
		   ResumeThread(ht);
	    return 0;
	   }
	  }
	  return -1;
  }

  long pool()
  {
	  //LastErrorSaver lerr;
	  
  
	  //

//         ULONG fl=WT_EXECUTELONGFUNCTION;
//		 WT_SET_MAX_THREADPOOL_THREADS(fl,256);
//	  if(QueueUserWorkItem((LPTHREAD_START_ROUTINE)&s_proc,this,fl))
	  if(QueueUserWorkItem((LPTHREAD_START_ROUTINE)&s_proc,this,WT_EXECUTELONGFUNCTION))
			  return 0;
       //lerr.set();
	  //delete this;
	  return -1;
  }

public:
inline static long push_asio(synchro_list_t *plist,const HANDSHAKE_DATA_holder* phsd)
{
	   //locker_t<synchro_list_t> lock(*plist);
       hd_holder hd;
   if(phsd) {
    hd=shared_heap<HANDSHAKE_DATA_holder>().allocate_holder();
	hd.ref()=*phsd;
	   }
	return   (new replay_socket(plist,hd))->pool();

};

};




inline long push_HANDSHAKE_DATA(const HANDSHAKE_DATA_holder* phsd)
{
     
	return replay_socket::push_asio(synchro_list,phsd);
	//this_lock;
	//socked_list().push_back(*phsd);
	//socked_list.back()=*phsd;
	//LONG l=-1;
	//ReleaseSemaphore(hsem,1,&l);
	//return 0;
}

inline long pop_HANDSHAKE_DATA(HANDSHAKE_DATA_holder* phsd)
{
	this_lock;
	/*
    HANDSHAKE_DATA_holder& r=socked_list().front();
	if(phsd) *phsd= r;
	socked_list().pop_front();
	return socked_list().size();
	*/
	return 0;
}

inline static const GUID& stop_guid()
{
     const GUID _stop_guid = { 0x53001c5b, 0xedd4, 0x4122, { 0xb0, 0xb4, 0x67, 0xdf, 0x3f, 0xc2, 0xc4, 0xd5 } };
     return _stop_guid;
};

 void on_send_recv(unsigned long id,char* bufin,int cbr,char* bufout,int* pcbw,HANDLE hAbortEvent)
{
   
	if(bufin==0)
	{
      file_handle_list.clear();  
      push_HANDSHAKE_DATA(0);
     return;

	}

	if((cbr==sizeof(GUID))&&(stop_guid()==*((GUID*)bufin)))
	{
		file_handle_list.clear();  
		push_HANDSHAKE_DATA(0);
		return;
          
	}


          HANDSHAKE_DATA_holder& hsd=*((HANDSHAKE_DATA_holder*)bufin);
          long& errcode=  *((long*)bufout);
		  *pcbw=sizeof(long);
		  bool f;
		  if(0) {
  		  f=check_hs(hsd);
          ULONG cc=Crc32(hsd.hsd);
          ULONG cm=hsd.hsd.crc32_size.offset;
		  log_printf("check_hs=%d   crc32=%08x  ncrc32=%08x \n",f,cm,cc);
		  }
		  f=check_hs(hsd);

		  //if(f||hsd.hsd.err_code)
		  if(f)
		  {
           
           if(hsd.cmd_is(int(0)))
		   {
			  if(hsd.hsd.get_socket_info()||hsd.hsd.err_code)
			{
				  push_HANDSHAKE_DATA(&hsd);
				errcode=0;
			}
    			else   errcode=-2;
		   }
		   else 
		   {
              if(hsd.cmd_is(int(2)))
			  {
				  //
				  DWORD pidsrc=hsd.hsd.pids.pid[0];
				  WSAPROTOCOL_INFOW* pwsainf=(WSAPROTOCOL_INFOW*)(bufout+4);
				  SOCKET s=hsd.hsd.get_socket();
				  if((s!=INVALID_SOCKET)&&(0== WSADuplicateSocketW(s,pidsrc,pwsainf)))
				  {
					 closesocket(s);
					 *pcbw=sizeof(WSAPROTOCOL_INFOW)+4;
					  errcode=0;
					  return;
				  }
				   errcode=GetLastError();
				   *pcbw=4;
				  return;
			  }

              if(cmd_proc) 
			  {
				  HANDSHAKE_DATA* pout=(HANDSHAKE_DATA*)bufout;
				  pout->sizeb=0;

                 errcode=cmd_proc(pcmd_context,&hsd,pout);
				 if(errcode==0)
				 {
					 *pcbw=pout->sizeb;
				 }

             }
		   }

		  }
		  else errcode=-1;

}

static void log_printf(char* fmt,...)
{
  char buf[256];
  va_list args;
  va_start( args, fmt );
  vsprintf(buf,fmt,args);
  OutputDebugStringA(buf);
}


	static  int   s_on_send_recv(void* pcontext,unsigned long id,char* bufin,int cbr,char* bufout,int* pcbw,HANDLE hAbortEvent)
	{
           //
		this_ptr(pcontext)->on_send_recv(id,bufin,cbr,bufout,pcbw,hAbortEvent);
		log_printf("hss: %05d :cbr=%d pbuf=%p\n",id,cbr,bufin);
		return 0;
	};



	HRESULT hr;

	//typedef  typename pipe_port<false,INSTANCES,BUFSIZE> pipe_port_t;
     typedef  HRESULT ( * cmd_proc_t)(void* pcontext,HANDSHAKE_DATA_holder* phdh,HANDSHAKE_DATA* pout);	

    SOCKET listen_socket;
	WSAEVENT hsockevent;
	cmd_proc_t cmd_proc;
	void * pcmd_context;

	
   hss_port(interceptor_proc_t interceptor_proc=0,void *pcontext=0,void *hcontext=HSD_SAME_PTR,int _replaymode=0)
   {
	     int err;
	   //static class_initializer_T<WSA_initerfiniter> t;
	   hs_socket_utils::socket_lib_init_once t;
       cmd_proc=0;
	   pcmd_context=0;

	   if(0)
	   {
        listen_socket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		BOOL l = TRUE;
		  err= ioctlsocket (listen_socket, FIONBIO, (unsigned long* ) &l);
          hsockevent=WSACreateEvent();
		  err=WSAEventSelect(listen_socket,hsockevent,FD_ACCEPT|FD_READ);
	   }

        synchro_list_t *p=new (shared_heap<synchro_list_t>().allocate()) synchro_list_t(interceptor_proc,pcontext,hcontext);
	    synchro_list.attach(p);
		synchro_list->acceptevent.dup_from(hsockevent);
		synchro_list->replaymode=_replaymode;
		//i_handle::acquire(p);
		//int lll=i_handle::release(p);
	  	   
   }

   ~hss_port()
   {
       
	   if(0)
	   {
		   WSACloseEvent(hsockevent); 
		   closesocket(listen_socket);
	   }
	       
	   pipe_port_t::safe_shutdown(port_pair);  
	   //if(hsem) CloseHandle(hsem);
   }


   inline void stop()
   {
	   //std::pair<HANDLE,long> pp=port_pair;
	     // InterlockedExchange((LONG volatile*)&port_pair.first,0);
         //pipe_port_t::safe_shutdown(pp);  
          SetEvent(synchro_list->portevent);
   }

   inline unsigned long wait_terminated(long timeout=INFINITE,bool falertable=true,long wakemask=0)
   {
	   HANDLE hh[1]={synchro_list->portevent};
	   DWORD rw,flags=(falertable)? MWMO_ALERTABLE|MWMO_INPUTAVAILABLE:MWMO_INPUTAVAILABLE;   
	   return rw=MsgWaitForMultipleObjectsEx(1,hh,timeout,wakemask,flags);
   }


   
template<class CH>
HANDLE   start(CH* pipname,int timeout=5000,HANDLE hf=0)
   {
     
     //
	   //hsem=CreateSemaphore(NULL,0,0x7FFFFFFF,NULL);
	   int pid=GetCurrentProcessId();
	   port_pair=pipe_port_t::create_thread_instance(pipname,&s_on_send_recv,this,timeout,pid,hf);

	    //synchro_list->portevent.dup_from(port_pair.first);
         synchro_list->set_port_event(port_pair.first);
	   //port_pair=pipe_port_t::create_thread_instance(pipname,&s_on_send_recv,0,timeout);
     return port_pair.first;

   }





};


struct socket_port_base{};

template <class HSPort=hss_port<> >
struct socket_port:socket_port_base
{
     
   

   typedef typename  HSPort hss_port_t;
   typedef typename   hss_port_t::HANDSHAKE_DATA_holder HANDSHAKE_DATA;
   typedef typename   hss_port_t::HANDSHAKE_DATA HANDSHAKE_DATA_raw;
   
   typedef typename  hss_port_base::handshake_buffer  SOCKET_DATA;
   typedef typename  hss_port_base::region_ptr region_ptr;
   typedef typename  hss_port_base::region sh_region;
   
   typedef typename  socket_port<HSPort> socket_port_t;
   typedef typename  socket_port_t self_t;
   typedef typename   self_t* this_t;

   typedef HRESULT (__stdcall * send_NS_closure_t)(SOCKET_DATA* psd);
   
     hss_port_t port;
	 handle_holder hfile;
	 v_buf<char> pipename;
	 v_buf<char> port_id;

	 ptr_holder<wchar_t> filename;
	 long pid;
	 bool fport_destroy_notify;
	 bool fport_virtual;
	 double conn_id;
	 bool fmkdir;
	 ULONG_PTR user_data;
	 

inline static region_ptr make_region(void* p,int cb=-1)
{
       region_ptr r=region_ptr();
	   r.ptr=(char*)p;
	   if(p)
		   r.sizeb=(cb<0)?strlen((char*)p)+1:cb;
	   return r;
}
inline static region_ptr make_regionZZ(char* p)
{
	int cb=(p)? safe_len_zerozero(p)+1 : 0;
    return make_region(p,cb);
}

         
inline    HRESULT _cmd_(HANDSHAKE_DATA* phd,HANDSHAKE_DATA_raw* pout)
         { 
            HRESULT hr=E_NOTIMPL;
			HANDLE hf=0;
			wchar_t*pfn,*palias;
			if(phd->cmd_is(1))
			{ 
              if(!(pfn=(wchar_t*)phd->get_region_ptr(0).ptr)) return E_INVALIDARG;  
			  if(!(palias=(wchar_t*)phd->get_region_ptr(1).ptr)) return E_INVALIDARG;  
			   
              if(SUCCEEDED(hr=create(pfn,1000,true,&hf)))
				  if(!port.file_handle_list.push(palias,hf))
				  {
					  CloseHandle(hf);
					  hr=HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
				  };
			}
			

             return hr;   
         }
      static HRESULT s_cmd_(void* p,HANDSHAKE_DATA* phd,HANDSHAKE_DATA_raw* pout)
	  {
        
		  return this_t(p)->_cmd_(phd,pout);

	  }


	  void set_uuids( void *ph)
	  {
		  SOCKET_DATA_h* shd=(SOCKET_DATA_h*)ph;

		  argv_zzs<char> args;
		  args["hid:"]=port_id.get();
		  
		  hss_port_base::region_ptr& h=shd->regions[HSD_HOST];
		  hss_port_base::region_ptr& url=shd->regions[HSD_URL];
		  args[":"]=(h.ptr)?h.ptr:"";
		  args["hp:"]=(url.ptr)?url.ptr:"";
		  args["cid:"]=++conn_id;

		  
		  const char* p=args.flat_str();

		  //hss_port_base::region_ptr& r=regions[HSD_SOCKET_REF];
		  shd->set_region(HSD_SOCKET_REF,(void*)p,-2);
 		  		  
	  }


       inline HRESULT unmarshal_data(HANDSHAKE_DATA_raw* hdshare,HSS_INFO** ppovl)
	   {     
		     HRESULT hr;
             if(!(hdshare&&ppovl)) return E_POINTER;
              
             SOCKET_DATA_h*& povl=*((SOCKET_DATA_h**)ppovl);		 

			 ptr_holder<HANDSHAKE_DATA> hd=shared_heap<HANDSHAKE_DATA>().allocate_holder();
			 hd->hsd=*hdshare;
			 hd->pcontext=this;
			 hd->replaymode=3;
             ptr_holder<SOCKET_DATA_h> shd=create_SOCKET_DATA(); 
             //shd->hd.ref()=*hdshare;
			 shd->hd=hd;
			 shd->pport=this;
			 shd->init();
			 hr=(shd->s!=INVALID_SOCKET)?S_OK:E_FAIL;
             if(SUCCEEDED(hr)) povl=shd.detach();
			 return hr;
	   }

	   inline HRESULT marshal_data(int pid,HSS_INFO* povl,HANDSHAKE_DATA_raw* phdshare)
	   {
		   HRESULT hr;
		   if(!(phdshare&&povl)) return E_POINTER;

		   SOCKET_DATA_h* sovl=(SOCKET_DATA_h*)povl;
		   
            HANDSHAKE_DATA_raw hdr;

			hr=port.set_to_process(hdr,pid,povl->s,sovl,(wchar_t*)0);
			if(SUCCEEDED(hr))
			{
				*phdshare=hdr;
				//povl->pid_dest=pid;
			}
		   //HRESULT set_to_process(HANDSHAKE_DATA& hsd,int pid,SOCKET s,handshake_buffer* phsptr,CH* pipename=0)

		   return hr;

	   }

      static void s_interceptor(void* proch,HANDSHAKE_DATA* phd,bool* phandled)
	  { 
		    *phandled=true;
			if(phd==0)
			{
               //if(fport_destroy_notify) 
				   closure_stub::safe_call(proch,0);
				return;
			}
           ptr_holder<SOCKET_DATA_h> shd=create_SOCKET_DATA();
		    
		     int s_dbg=sizeof(SOCKET_DATA_h);
		    void* abortproc=phd->abortproc;

			int replaymode=phd->replaymode;

			shd->hd=phd;
             shd->pport=(socket_port_t*)phd->pcontext;
			 shd->user_data=shd->pport->user_data;
				 //(ULONG_PTR)shd->pport->port.synchro_list->pinterceptor_context;
            
			shd->init();
			

			//this_t(phd->pcontext)->set_uuids(shd);


			
			
			
			//attempt_make_confirm();
			
			void* pc=hss_get_signaler(shd.p);

			//shd->attach_region(HSD_SIGNALER,(char*)abortproc);

			closure_stub::safe_call(proch,shd,0,(void*)(phd->hsd.err_code));
               
	  }


	  
	 socket_port(void *pcallback=0,void* hcontext=HSD_SAME_PTR,int replay_mode=0)
		 :hfile(0),pipename(0)
		 ,port((pcallback)?&s_interceptor:0,pcallback,hcontext,replay_mode&(~HSF_SRD_CREATE_PATH))
		 ,fport_destroy_notify(1),fport_virtual(0)
	 {
		         //
		 fmkdir=(replay_mode&(HSF_SRD_CREATE_PATH))!=0;
		 conn_id=0;
		 port_id.cat(uuid_gen());
		 port.cmd_proc=&s_cmd_;
		 port.pcmd_context=this;
		 port.synchro_list->pcmd_context=this;
		 user_data=(ULONG_PTR)hcontext;
		        pid=GetCurrentProcessId();
	 };

	 inline ptr_holder<event_signaler> get_event_signaler()
	 {
		 return port.synchro_list->port_event_signaler;
	 }
/*
	 socket_port():hfile(0),pipename(0)
	 {

		 pid=GetCurrentProcessId();
	 };
*/

	 //~socket_port():hfile(0){CloseHandle(hfile);};
/*
	 SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),NULL,true};
	 HANDLE hf=CreateFileW(L"\\\\?\\o:\\ddd",GENERIC_WRITE|GENERIC_READ,
		 FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE,0);//|FILE_FLAG_DELETE_ON_CLOSE
	 return hf;
	 generate_pipe_name(
*/




    
inline    HRESULT write(HANDLE hf, v_buf<char>& pipen=generate_pipe_name())
			{

				char b[64],*ppi;
				DWORD dw=0,sz;
				DWORD cb=sprintf(b,"pid=%d",pid);
				b[cb+1]=b[cb]=0;

				if(!WriteFile(hf,(char*)"port=",5,&dw,0))
					return HRESULT_FROM_WIN32(GetLastError());
                 if(sz=pipen.size_b())
				 {				 
					 ppi=(char*)pipen;
				    if(!WriteFile(hf,ppi,sz+1,&dw,0))
					 return HRESULT_FROM_WIN32(GetLastError());
				 }
				if(!WriteFile(hf,b,cb+2,&dw,0))
					return HRESULT_FROM_WIN32(GetLastError());
				FlushFileBuffers(hf);
                    //pipename=pipen;
					return S_OK;
      			};

inline    HRESULT read(HANDLE hf)
{

	DWORD dw=0;

	
	int cb=GetFileSize(hf,&dw);
	if(cb<0) 
         return HRESULT_FROM_WIN32(GetLastError());
	 v_buf<char> buf(cb);

	if(!ReadFile(hf,(char*)buf,cb,&dw,0))
		return HRESULT_FROM_WIN32(GetLastError());

	std::vector<char*> args=args_ZZ<char>(buf);
	char *p;

	if(find_named_value("pid",args,&p)<0) return E_FAIL;
	 pid=atoi(p);

	if(find_named_value("port",args,&p)<0) return E_FAIL;
	 pipename.v.resize(strlen(p)+1);
	 strcpy(pipename,p);

   

//named_value(const char* name, LIST const& argv,const char*  def="",const char sep='=')


	return S_OK;
};


template <long flags=LOCKFILE_EXCLUSIVE_LOCK>
  struct FileLocker
  {
	  OVERLAPPED ov;
	  HANDLE hf;
	  HRESULT hr;
	  bool f;

	  FileLocker(HANDLE h):hf(h),hr(0)
	  {
		  DWORD dw=0;
        memset(&ov,0,sizeof(OVERLAPPED));
		if(f=LockFileEx(hf,flags,0,-1,0,&ov))
		//if(f=LockFileEx(hf,0,0,-1,0,&ov))
		{
			if(GetOverlappedResult(hf,&ov,&dw,true))
			   return ;
		}
           hr=HRESULT_FROM_WIN32(GetLastError());
	  }

       ~FileLocker()
	   {
		 if(f) UnlockFileEx(hf,0,-1,0,&ov);
	   }
  };



inline	HRESULT create(wchar_t* fn_service_name,int timeout=5000,bool fAliasOnly=false,HANDLE* phf=0)
	{
		HRESULT hr;
      SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),NULL,true};

         if(fmkdir)
			 file_make_dir(fn_service_name);

	  HANDLE hf=(phf)?*phf:INVALID_HANDLE_VALUE;
	  if((hf==INVALID_HANDLE_VALUE)||(hf==0))
		  hf= CreateFileW(fn_service_name,GENERIC_WRITE|GENERIC_READ,
		  FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE,0);//|FILE_FLAG_DELETE_ON_CLOSE
        if(hf==INVALID_HANDLE_VALUE) return hr=HRESULT_FROM_WIN32(GetLastError());
		 handle_holder hfh(hf);
		 {
          FileLocker<LOCKFILE_EXCLUSIVE_LOCK> fl(hf);

         if(fAliasOnly==false)
		 {
		  pipename=generate_pipe_name();
		  char *pn=pipename;
		  if(!port.start(pn,timeout,hfh.dup()))
				return hr=HRESULT_FROM_WIN32(GetLastError());
		 }
         if(FAILED(hr=fl.hr)) return hr;  

         if(FAILED(hr=write(hf,pipename))) return hr;  
         
		 if(fAliasOnly&&(phf)) *phf=hfh.detach();

          //f=UnlockFileEx(hf,0,-1,0,&ov);
		 }

		  //hfile.attach(hh.detach());
		  return S_OK;
				
	}

static inline long long get_file_size(HANDLE hf)
{
   LARGE_INTEGER ll=LARGE_INTEGER();
    GetFileSizeEx(hf,&ll) ;
	return ll.QuadPart;
}
 inline	HRESULT wait_port_creation(char* fn_service_name,int to=INFINITE,bool falertable=false,bool fdelf=1)
{
	
	ptr_holder<wchar_t> wurl= char_to_unicode(fn_service_name,-1,CP_UTF8);
	return wait_port_creation(wurl,to,falertable,fdelf);

};



inline	HRESULT wait_port_creation(wchar_t* fn_service_name,int to=INFINITE,bool falertable=false,bool fdelf=true)
{
	HRESULT hr;
	int err;
	struct fdeleter
	{
		wchar_t * wportname;
		fdeleter(wchar_t *pn):wportname(pn){};
		~fdeleter(){ if(wportname) DeleteFileW(wportname);}
	} ;

	//ptr_holder<wchar_t> wportname=parseNS(wurl,&err); 
	wchar_t * wportname=reparseNS(fn_service_name,&err,filename);
	if(!wportname) return E_POINTER;
	
     DWORD fattr=0;//FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE; 
	
	  fdelf=(err!=1)&&fdelf;
          bool f;
	  //(void) fdeleter( (fdelf)?wportname:0);
	  {
		  handle_holder hh0=CreateFileW(wportname,GENERIC_WRITE|GENERIC_READ,
		  FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);//|FILE_FLAG_DELETE_ON_CLOSE
		  FileLocker<LOCKFILE_EXCLUSIVE_LOCK> fl(hh0);
		  f=SetEndOfFile(hh0);
	  }
	 


	HANDLE hf=CreateFileW(wportname,GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,0,OPEN_ALWAYS,fattr,0);//|FILE_FLAG_DELETE_ON_CLOSE
	if(hf==INVALID_HANDLE_VALUE) return hr=HRESULT_FROM_WIN32(GetLastError());

    

    //fdeleter fdelete((fdelf)?wportname:0);

	
    handle_holder hh(hf);
	bool finf=(to==INFINITE);
	unsigned char c=1;
	DWORD tic=GetTickCount(),dt;
	do 
	{

      if(get_file_size(hf))
	  {
		  FileLocker<LOCKFILE_EXCLUSIVE_LOCK> fl(hf);
		  if(FAILED(hr=fl.hr)) return hr;  
		  if(FAILED(hr=read(hf))) return hr; 
		  if(hss_port_t::wait_pipe(pipename,to))
			  return S_OK;
		  else 	return  HRESULT_FROM_WIN32(GetLastError());
	  }
	  if(c>unsigned(to)) break;
      SleepEx(c*=2,falertable);
	  if(c==0) c=1;
       dt=GetTickCount()-tic;
	} while(finf||(dt<to));

  
  return HRESULT_FROM_WIN32(WAIT_TIMEOUT);
}


inline	HRESULT open(wchar_t* fn_service_name)
{
	HRESULT hr;
	SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),NULL,true};
	HANDLE hf=CreateFileW(fn_service_name,GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,0,OPEN_EXISTING,0,0);//|FILE_FLAG_DELETE_ON_CLOSE
	if(hf==INVALID_HANDLE_VALUE) 
	{
	//	_cwprintf(L"error in fn=%s\n",fn_service_name);
		return hr=HRESULT_FROM_WIN32(GetLastError());
	}
	handle_holder hh(hf);
	
	FileLocker<LOCKFILE_EXCLUSIVE_LOCK> fl(hf);
	if(FAILED(hr=fl.hr)) return hr;  
	if(FAILED(hr=read(hf))) return hr;  


	//hfile.attach(hh.detach());
	return S_OK;

};

inline HRESULT send(SOCKET s,const char* req,const char* rep,const char* reperr=0,const char* params=0)
{
	HRESULT hr=E_FAIL;
	if((pid)&&(!pipename.empty())) 
	{
		hr=hss_port_t::send_to_process((char*)pipename,pid,s,req,rep,reperr,params);
		 
	}
	return hr;
}

inline HRESULT send(SOCKET s,SOCKET_DATA* phsptr)
{
	HRESULT hr=E_FAIL;
	if((pid)&&(!pipename.empty())) 
	{
		hr=hss_port_t::send_to_process((char*)pipename,pid,s,phsptr);

	}
	return hr;
}

inline HRESULT remove_NS_alias(wchar_t* port_alias_name)
{
      bool f=port.file_handle_list.remove_alias(port_alias_name);
	  return (f)?S_OK:E_INVALIDARG;
}
inline HRESULT link_NS_alias_to(wchar_t* port_alias_name,wchar_t* port_name)
{
     HANDLE hf=port.file_handle_list.get_alias_handle(port_alias_name);
	 if(hf==INVALID_HANDLE_VALUE) return E_INVALIDARG;
	 HRESULT hr;
	 socket_port<> sp;
	 hr=sp.open_NS(port_name,0);
	 if(FAILED(hr)) return hr;
     hr=sp.create_NS_alias(port_alias_name,hf);
	return hr;
}

inline HRESULT create_NS_alias(wchar_t* port_alias_name,HANDLE hf=INVALID_HANDLE_VALUE)
{
	   HRESULT hr;
       SOCKET_DATA soda;
        int err;
	   ptr_holder<wchar_t> ph;

       wchar_t * wportalias=reparseNS(port_alias_name,&err,ph);
	   if(err!=0)
		   return E_ACCESSDENIED;
		   

	    if((hf==INVALID_HANDLE_VALUE)&&(!wportalias))
			return E_INVALIDARG;
//
		  soda.s=(SOCKET)hf;
		  soda.regions[0].ptr=(char*)wportalias;
		  soda.regions[0].sizeb=i_handle::size(wportalias);

		  soda.regions[1].ptr=(char*)port_alias_name;
		  soda.regions[1].sizeb=safe_len(port_alias_name,1);



		  sh_region cmd;
		  cmd.offset=1;

		   hr=send_NS_cached(&soda,cmd);
		  return hr;
        //hr=open(wportalias);
}


inline HRESULT send_NS(SOCKET_DATA* phsptr,bool fns_only=true,bool fsockcheck=true,sh_region cmd=sh_region())
{
	if(!phsptr) return E_POINTER;
	HRESULT hr=E_FAIL;
	SOCKET_DATA&  hs=*phsptr;

     if(fsockcheck)   
	    if(INVALID_SOCKET==hs.s) return hr;

	


	ptr_holder<wchar_t> wurl= char_to_unicode(hs.url.ptr,hs.url.sizeb,CP_UTF8);

       hr=open_NS(wurl,fns_only);
	if(SUCCEEDED(hr))
	   hr=send_NS_cached(phsptr,cmd);
	return hr;
};


inline HRESULT open_NS(wchar_t* wurl,bool fns_only=true)
{
	HRESULT hr;
   int err;
  //ptr_holder<wchar_t> wportname=parseNS(wurl,&err); 
   wchar_t * wportname=reparseNS(wurl,&err,filename);

    if(!wportname) return E_POINTER;

    if((err<0)&&fns_only) return E_ACCESSDENIED;
   //if((err)&&(fns_only))
      hr=open(wportname);

	return hr;
};

inline HRESULT create_process_and_send_NS(SOCKET_DATA* phsptr)
{
  HRESULT hr=E_FAIL;
  	return hr;
}

inline HRESULT send_NS_cached(SOCKET_DATA* phsptr,sh_region cmd=sh_region())
{
	HRESULT hr=E_FAIL;
	SOCKET_DATA&  hs=*phsptr;

	//if(INVALID_SOCKET==hs.s) return hr;
	if((pid)&&(!pipename.empty())) 
	{
		//hr=hss_port_t::send_to_process((char*)pipename,hs.pid=pid,hs.s,phsptr);
		hr=hss_port_t::send_to_process((char*)pipename,pid,hs.s,phsptr,cmd);

	}
	return hr;
}




static ptr_holder<self_t> create_NS(wchar_t*  filename,int replay_mode=0,void* pcallback=0,void * hcontext=HSD_SAME_PTR)
{
	hss_port_base::LastErrorSaver lr;
	HRESULT hr;

	//ptr_holder<self_t> psp=shared_heap<self_t>().allocate_holder();
	//self_t *p=new(shared_heap<char>().allocate(sizeof(self_t))) self_t(pclosure);

    //self_t *p=sh_nef_t)(pclosure);
	//ptr_holder<self_t> psp;
	//(p,false);
	//psp.p=p; 
	
	ptr_holder<self_t> psp(sh_new(self_t)(pcallback,hcontext,replay_mode),false);

	hr=psp->create(reparseNS(filename,0,psp->filename));
	//hr=psp->create(parseNS(filename));
	if(FAILED(lr.set(hr))) return 0;

	i_handle::event_ref(psp)=psp->get_event_signaler();
	i_handle::completion_port_ref(psp)=psp->port.synchro_list->get_io_signaler();

	return psp;
}

static ptr_holder<self_t> create_NS(char*  filename,int replay_mode=0,void* pcallback=0,void * hcontext=HSD_SAME_PTR,unsigned int CP_XX=CP_UTF8)
{
	hss_port_base::LastErrorSaver lr;
	ptr_holder<self_t> psp;
	wchar_t* wfilename;
	ptr_holder<wchar_t> wfn;
		
	if(filename!=HS_PROCESS_PORT_BY_FACTORY)
	{
		wfilename=wfn=char_to_unicode(filename,-1,CP_XX);
	}
	else wfilename=HS_PROCESS_PORT_BY_FACTORY_W;

 	if(!wfilename) return lr.set(),0;
	 psp=create_NS(wfilename,replay_mode,pcallback,hcontext);
	
   lr.set();
   return psp;
}

inline SOCKET  accept( SOCKET_DATA ** ppparams=0) 
{
	
	ptr_holder<SOCKET_DATA> sd= accept_descriptor();

	if(sd.p)
	{
		
      //  SOCKET s=make_detach(sd->s,INVALID_SOCKET);
		SOCKET s=sd->s;
		//sd->set_flags(0);
		sd->fclose_after=0;
		
		if(ppparams) *ppparams=sd.detach();
		return s;
	}
	

	return INVALID_SOCKET;
}

struct SOCKET_DATA_h:SOCKET_DATA
{ 

	
	/*
	struct region_buf 
	{
		long sizeb;
		char buf[1];
	};
   */




	struct wbs_getter_t
	{
		HRESULT hr;
		SOCKET_DATA_h* hovl;
		wbs_getter_t(SOCKET_DATA_h* _hovl):hovl(_hovl),hr(E_FAIL){}

		inline operator HRESULT(){ return HRESULT_FROM_WIN32(hr);}

		int operator()(void *pbuf,int cb,int flags=0)
		{
			WSABUF wb={cb,(char*)pbuf};
			return (*this)(1,&wb);
		}

		int operator()(int nbuf,WSABUF *pbuf)
		{
			
			SOCKET s=hovl->s;
			DWORD flags=0,nbytes=-1;

			int rc=WSARecv(s,pbuf,nbuf,&nbytes,&flags,hovl,0);
			if(rc==0)
			{hr=0; return nbytes;}
			if((hr=WSAGetLastError())==WSA_IO_PENDING)
			{
				hr=0;
				if(GetOverlappedResult((HANDLE)s,hovl,&nbytes,true))
					return  nbytes;

			}
			hr=GetLastError();
			return -1;
       

		}

	};

	struct wbs_setter_t
	{
		HRESULT hr;
		SOCKET_DATA_h* hovl;
		wbs_setter_t(SOCKET_DATA_h* _hovl):hovl(_hovl),hr(E_FAIL){}

		inline operator HRESULT(){ return HRESULT_FROM_WIN32(hr);}

    	int operator()(void *pbuf,int cb,int flags=0)
		{
			WSABUF wb={cb,(char*)pbuf};
			return (*this)(1,&wb);
		}

		int operator()(int nbuf,WSABUF *pbuf)
		{

			SOCKET s=hovl->s;
			DWORD flags=0,nbytes=-1;

			int rc=WSASend(s,pbuf,nbuf,&nbytes,flags,hovl,0);
			if(rc==0)
			 {hr=0; return nbytes;}
			if((hr=WSAGetLastError())==WSA_IO_PENDING)
			{
				hr=0;
				if(GetOverlappedResult((HANDLE)s,hovl,&nbytes,true))
					return  nbytes;
				
		     }
			 hr=GetLastError();
             return -1;
		}

	};








     

    typedef    wbs_string_loader_t<wbs_getter_t> string_loader_t;

	typedef  ptr_holder<char> region_buf_h ;

	typedef std::list<region_buf_h> region_list_t;
	typedef hss_port_base::locker_t<hss_port_base::CS> locker_t;
     typedef hss_port_base::CS mutex_t;
    typedef  void (__stdcall * cpio_proc_t)(long error,long bytes_transfered,void* povl);

	ptr_holder<HANDSHAKE_DATA>  hd;
	region_list_t rl;
	mutex_t mutex;
	ptr_holder<event_signaler> close_event;
	//HANDLE hportevent;

    ptr_holder<cpio_proc_t>  cpio_proc_holder;
	cpio_proc_t cpio_proc;
	socket_port_t * pport;

	wbs_getter_t wbs_getter;
	wbs_setter_t wbs_setter;
	string_loader_t wbsloader;//(wbs_getter,1);

	// VERSION >7
	wbs_frame_io_t wbs_frame_io;
	bool fv8;

	ptr_holder<char>  last_recv;



    

	SOCKET_DATA_h():wbs_getter(this),wbs_setter(this),wbsloader(wbs_getter,1){};

	inline void lock()	{

        mutex.lock();
	}
	inline void unlock()	{

		mutex.unlock();
	}
	





inline  int check_SRD()
{
	return atoi(named_value(REF_ID_DEMAND,args_ZZ(regions[HSD_PARAMS].ptr),"0"));
}

inline HRESULT final_replay(HANDLE hAbortEvent=INVALID_HANDLE_VALUE)
{
   /*
	if((reply_state&3)==3) return S_OK;  
   if(SUCCEEDED(reply_state)) return make_confirm(3,hAbortEvent);
   else return E_FAIL;
   */
   return S_OK;  
}
inline HRESULT make_confirm(int f_socket_ref_demand_mode=0, HANDLE hAbortEvent=INVALID_HANDLE_VALUE)
{
	hss_port_base::LastErrorSaver lr;
	if((reply_state&3)==3) return ERROR_ALREADY_ASSIGNED;
//  if(SUCCEEDED(hss_port_base::send_region(s,hd->get_replay_data(),synchro_list->portevent)))
	//bufT<char,40> suid;
	//uuid_gen(suid);
	HRESULT hr=E_INVALIDARG;
	if((reply_state&1)==0)
	{
		
		if(regions[HSD_REPLY].sizeb==0)
		{
			
          bool f=(regions[HSD_REQUEST].sizeb>=2)&&(safe_cmpni(regions[HSD_REQUEST].ptr,"hss",3)==0);
		  if(!f)
			return S_FALSE;
		}
		else
		{
		  hr=hss_port_base::send_region(s,regions[HSD_REPLY],hAbortEvent);
		  if(SUCCEEDED(hr)) reply_state=3&(reply_state|1);
			else
			{
			reply_state|=0x80000000;
			return lr.set(hr);
			}
    		  double *pv;
	     	  fv8=(pv=(double *)regions[HSD_WS_VERSION].ptr)&&(*pv>0);

		}
		 //regions[HSD_REPLY]=hss_port_base::region_ptr();
	}
    
	 int fm=f_socket_ref_demand_mode;
   if(((reply_state&2)==0)&&fm&&check_SRD())
   {


	   if(1)
	   {
        hss_port_base::region_ptr& r=regions[HSD_SOCKET_REF];
		if((!r)&&(fm&HSF_SRD_AUTO))
		{
			pport->set_uuids(this);
         /*
          hss_port_base::region_ptr& h=regions[HSD_HOST];
		  hss_port_base::region_ptr& url=regions[HSD_URL];
		  ptr_holder<char> ph;
		  if((url))
		  {
			  //ph=sh_printf(":=%s\0hp:=%s\0\0",url.ptr,h.ptr);
			  char *path=url.ptr;
	  
              while(*path=='/') path++;
			  char* fmt[2]={":=%s%c",":=%s%chp:=%s%c"};
			  ph=sh_printf(fmt[(h)?1:0],path,0,h.ptr,0);//,suid.get(),0);
			  
			  attach_region(HSD_SOCKET_REF,ph);
			  std::vector< char*> vp=args_ZZ<char>(ph);
			 

		  }
		  */

		}
	   }
       
       hr=hss_port_base::send_region_wbs_string(s,regions[HSD_SOCKET_REF],hAbortEvent);
	   if(SUCCEEDED(hr)) reply_state=3&(reply_state|2);
	   else   reply_state|=0x80000000;
		   
	   
   }
	 
	return lr.set(hr);

}


SOCKET reset_socket(int flags=0,SOCKET snew=INVALID_SOCKET)
{
	    hss_port_base::LastErrorSaver lr;
        locker_t lock(mutex);
		 
		short lflags=LOWORD(flags);
     
		if(HSF_REPLY&lflags){
			DWORD err=make_confirm(HIWORD(flags));
			lr.set(err);
            if(FAILED(err))
				return INVALID_SOCKET;
	    }

		SOCKET rs=s;
		if(lflags&HSF_SET) {s=snew;}
		if(lflags&HSF_CLOSED) {fclose_after=1;};
		if(lflags&HSF_FAST_CLOSED) {fclose_after=2;};
		if(lflags&HSF_DETACH) { fclose_after=0;};
		if(lflags&HSF_DUP)  { rs=hs_socket_utils::dup_socket(s);lr.set();}
        if( ( ( lflags&&(HSF_DUP|HSF_DETACH) ) == (HSF_DUP|HSF_DETACH) )&&(rs!=INVALID_SOCKET)) 
			                 closesocket(make_detach(s,INVALID_SOCKET)); 
		return rs;
}

inline ULONG state()
{    
	 locker_t lock(mutex);
     ULONG st=0;
	 for(int n=0;n<hss_port_base::REGION_COUNT;n++)
	     if(regions[n]) st|=(1<<n);

	 if(hs_socket_utils::is_socket(s)) st|=(1<<HSD_SOCKET);
	 
   return st;
}
char*  reset_region(int n,int flags=0,void* pv=0,int cb=-1)
{
	if(n==HSD_SIGNALER)
	{
         if(flags&1) attach_region(HSD_SIGNALER,(char*)pv);
		 return (char*)abortproc;
	}
	if(n==HSD_USER_DATA)
	{       
		    if(flags&1) user_data=(ULONG_PTR)pv;
			return (char*)user_data;
	}


        if(n==HSD_STATE)
		{
          return (char*) state();
		}
		if(n==HSD_REPLY_STATE)
		{
			return (char*) reply_state;
		}
	    if(n==HSD_SOCKET)
		{ 
			
			return (char*)reset_socket(flags,(SOCKET)pv);
		}

        //locker_t lock(mutex);
	  if(flags&1)
		return   set_region(n,pv,cb);
	    else return  get_region(n,(int*)pv);
	   
};

char*  get_region(int n,int *pcb=0)
   {  

	   hss_port_base::LastErrorSaver lr;

	   if((n<0)||(n>=hss_port_base::REGION_COUNT))
		   return lr.set(HRESULT_FROM_WIN32(ERROR_INVALID_INDEX)),0;
 
          locker_t lock(mutex);
      
	   
          if(pcb) *pcb=regions[n].sizeb;
		  return regions[n].ptr;
  
       
   }


char*  set_region(int n,void* pv,int cb=-1)
{  
	hss_port_base::LastErrorSaver lr;

	locker_t lock(mutex);

	if((n<0)||(n>=hss_port_base::REGION_COUNT))
		return lr.set(HRESULT_FROM_WIN32(ERROR_INVALID_INDEX)),0;

	

	hss_port_base::region_ptr& r=regions[n];
	

	char *p = (char*) pv;
    
	if(p)
	{

        if(cb==-2) cb= safe_len_zerozero(p)+1 ;
		else cb=(cb<0)?strlen(p)+1:cb;
	}
	else cb=0;

	//if(cb==-2) cb=(p)? hss_port_t::safe_len_zerozero(p)+1 : 0;
	//else if(cb<0) cb=(p)?strlen(p)+1:0;

	if(cb)
	{
         region_buf_h rb=shared_heap<char>().allocate_holder(cb);
         memcpy(rb,p,cb);
         rl.push_back(rb);
		 r.sizeb=cb;
		 r.ptr=rb;
	}
	else 		r=hss_port_base::region_ptr();

	return r.ptr; 
}

char* attach_region(int n,char* ph)
{
	hss_port_base::LastErrorSaver lr;

	locker_t lock(mutex);

	if(n==HSD_SIGNALER)
	{
        event_signaler* pes=0;
		void* p=hss_get_signaler(ph);
     
       if((p)&&(pes=i_handle_cast<event_signaler>(p)))
	   {

		 abortproc=pes;
	     address_out(1);
		 //		 hkObjects[0]=pes->handle();
		 //hkcount=2;
		 //
		 hkObjects[1]=pes->handle();
		 //
		 hkcount=3;
         rl.push_back((char*)pes);
	   }
	   	return (char*)pes;
	}

	if((n<0)||(n>=hss_port_base::REGION_COUNT))
		return lr.set(HRESULT_FROM_WIN32(ERROR_INVALID_INDEX)),0;

	hss_port_base::region_ptr& r=regions[n];
	int cb=i_handle::size(ph);  
	if(cb)
	{
				
		rl.push_back(ph);
		r.sizeb=cb;
		r.ptr=ph;
	}
	else 		r=hss_port_base::region_ptr();
     return r.ptr; 
};

inline  void set_close_signaler()
 {
	 close_event.attach(hss_create_event_closure());
	 hkObjects[0]=close_event->handle();
	 hkcount=2;
	 if(i_handle::verify_ptr(this))
		 i_handle::event_ref(this)=close_event;
 }


      
   void attempt_make_confirm()
   {
	   HRESULT hr;
	   int replaymode=hd->replaymode;
	   if(((HSF_SRD_VIRTUAL&replaymode)==0)&&(replaymode)) make_confirm(replaymode);
	   
    }

	void init()
	{
        locker_t lock(mutex);
		if(!hd.p) return;

		
		
		s=hd->detach_socket();

         int replaymode=hd->replaymode;


	


	//	pid=hd->hsd.pids.pid[0];

		
		if((HSF_SRD_VIRTUAL&replaymode)==0)
        		attach_region(HSD_SIGNALER,(char*)hd->abortproc);

		if(	error_code=hd->hsd.err_code)
			return;

		for( int n=0;n<hss_port_base::REGION_COUNT;n++)
		{

			//regions[n]=hss_port_base::region_ptr(hd->hsd.regions[n],hd.p);
			hss_port_base::region_ptr r=hss_port_base::region_ptr(hd->hsd.regions[n],hd.p);
            set_region(n,r.ptr,r.sizeb);

		}

		

		
		fclose_after=1;

		if(((HSF_SRD_VIRTUAL&replaymode)==0)&&(replaymode)) 
			make_confirm(replaymode);
		
		hd.release();
		

	}
	~SOCKET_DATA_h()
	{
		
		if((fclose_after)&&(s!=INVALID_SOCKET)) 
		{
			 //			CancelIo(HANDLE(s));
			//if(!shutdown(s,SD_BOTH))
				 closesocket(s);
		}
	}

};

inline static ptr_holder<SOCKET_DATA_h> create_SOCKET_DATA(bool fclose_after=0,SOCKET s=INVALID_SOCKET)
{
	ptr_holder<SOCKET_DATA_h>  sdh=shared_heap<SOCKET_DATA_h>().allocate_holder(); 
	sdh->s=s;
	sdh->fclose_after=fclose_after;
	sdh->cpio_proc=0;
	sdh->set_close_signaler();
//	const char* pp=i_handle::id(sdh);
//	i_handle::acquire(sdh.p);
//    int ll=i_handle::release(sdh.p);

	return sdh;
};

inline ptr_holder<SOCKET_DATA>  accept_descriptor()//(long timeout=INFINITE,bool falertable=false)
{
   //ptr_holder<HANDSHAKE_DATA>  ph;//=shared_heap<HANDSHAKE_DATA>().allocate_holder();

	
    //ptr_holder<SOCKET_DATA_h>  sdh=shared_heap<SOCKET_DATA_h>().allocate_holder(); 
	//sdh->s=INVALID_SOCKET;
    ptr_holder<SOCKET_DATA_h>  sdh=create_SOCKET_DATA();

	int f=port.synchro_list->pop(sdh->hd.address(),INFINITE,false);
	if(f==WAIT_OBJECT_0)
	{
      sdh->init();  
	}
   //port.get_HANDSHAKE_DATA(ph.p,INFINITE,false);
   //port.synchro_list->pop(ph.address(),INFINITE,false);

   return sdh;
}


/*

inline ptr_holder<HANDSHAKE_DATA>  accept_descriptor()
{
	ptr_holder<HANDSHAKE_DATA>  ph;//=shared_heap<HANDSHAKE_DATA>().allocate_holder();

	//port.get_HANDSHAKE_DATA(ph.p,INFINITE,false);
	port.synchro_list->pop(ph.address(),INFINITE,false);

	return ph;
}


inline SOCKET  accept( char ** ppparams=0) 
{
	ptr_holder<HANDSHAKE_DATA>  ph=accept_descriptor();
    SOCKET s=ph->detach_socket();
	if((ph.p)&&(ppparams))
	{
		hss_port_t::region_ptr r=ph->get_params_data();
       if(r)
	   {
		    //
		   std::vector< char*> vp=args_ZZ(r.ptr);

              char* p =(char*)shared_heap<char>().allocate(r.sizeb+1);
			  memcpy(p,r.ptr,r.sizeb);
			  p[r.sizeb]=0;

			 // std::vector< char*> vp2=args_ZZ(p);
			  *ppparams=p;
	   }

	}
	
	return s;
}
*/


};



#pragma pack(pop)