#pragma once

#include <windows.h>
#include <process.h> 

template <int N,class T>
inline void zeroN(T* p)
{	struct _ha {T m_h[N];};
	*((_ha*)(p))=_ha();  
}

template <int N>
struct char_buf_N
{
	char b[N];
	operator char*()
	{
		return b;
	}
	operator const char*() const
	{
		return b;
	}
char_buf_N<N>& printf(char* fmt,...)
{
	 va_list argptr;
	 va_start(argptr, fmt);
	 vsprintf(b,fmt,argptr); 
	 return *this;
}

};



template <int N>
struct handle_array_base
{

	HANDLE m_h[N];
typedef typename char_buf_N<N*16+4> char_buf;
/*	struct char_buf
	{
		char b[N*16+4];
		operator char*()
		{
			return b;
		}
	};
*/
	inline  char_buf str()
	{
		char_buf b;
		char buf[100];
		sprintf(b,"%d",m_h[0]);
		for(int n=1;n<N;n++)
		{
          sprintf(buf,",%d",m_h[n]);
		  strcat(b,buf);
		}
		return b;
	};

	

HANDLE& operator[]( int n)
{
	return m_h[n];
}
inline int wait(int n=-1,int timeout=INFINITE)
{
	DWORD dw;
	if(n>=0)
	{
      return dw=WaitForSingleObject(m_h[n],timeout);   
	}
    bool fwaitall=(n==-2);
  return dw=WaitForMultipleObjects(N,m_h,fwaitall,timeout);
};
};

template <int N>
handle_array_base<N>& load_from_str(char* pstr,handle_array_base<N>& ha=handle_array_base<N>())
{
	return ha;
};

template <int N>
struct handle_array:handle_array_base<N>
{
	
  handle_array( const handle_array<N>  & hb)
  {
	  //(*this)=hb.duplicate();
	  hb.duplicate_to(this);

  };

   handle_array()
   {
	   zeroN<N>(m_h);
    //  struct _ha {HANDLE m_h[N];};
     // *((_ha*)(m_h))=_ha();  
   };

HANDLE& close(int n)
{
 CloseHandle(m_h[n]);
 m_h[n]=0;
 return m_h[n];
}
void close()
{
for(int n=0;n<N;n++) close(n);
}

~handle_array()
{
	close();
//for(int n=0;n<N;n++) CloseHandle(m_h[n]);
}


inline  void operator =(const handle_array<N>  &  hb ) 
{
	//*((handle_array_base<N>*)this)=hb.duplicate();
     //handle_array_base<N> *p=const_cast<handle_array<N>*>();
	   hb.duplicate_to(this);
	//return *this;
}


 HRESULT  duplicate_to(handle_array_base<N> *ph,int pid=GetCurrentProcessId(),bool finherited=false) const
{
	if(!ph) return E_POINTER;
	handle_array_base<N> h;
	handle_array<1> hp;
	handle_array<N> herr;
	hp[0]=OpenProcess(PROCESS_DUP_HANDLE,0,pid);
	
	bool f;
	HANDLE hs=GetCurrentProcess();
	for(int n=0;n<N;n++)
	{
		f=DuplicateHandle(hs,m_h[n],hp[0],&(h[n]),0, finherited,DUPLICATE_SAME_ACCESS);
		if(!f) 
		{
	            return HRESULT_FROM_WIN32(GetLastError()); 
		}
	}
     	*ph=h;
		return S_OK;
}

 handle_array_base<N> duplicate(int pid=GetCurrentProcessId(),bool finherited=false)
{
	handle_array<1> hp;
    handle_array<N> herr;
	hp[0]=OpenProcess(PROCESS_DUP_HANDLE,0,pid);
	handle_array_base<N> h;
	bool f;
	HANDLE hs=GetCurrentProcess();
	for(int n=0;n<N;n++)
	{
      f=DuplicateHandle(hs,m_h[n],hp[0],&(h[n]),0, finherited,DUPLICATE_SAME_ACCESS);
	  if(!f) 
	  {
		  herr=h;
		  return handle_array<N>();	          
	  }
	}
     return h; 
}
 HANDLE dup_inherited(int n,bool finherited=true)
 {
	 HANDLE hp=GetCurrentProcess(),ht=INVALID_HANDLE_VALUE;
     DuplicateHandle(hp,m_h[n],hp,&ht,0, finherited,DUPLICATE_SAME_ACCESS);
	 return ht;
 }
};


inline int loop_event(void* handle=INVALID_HANDLE_VALUE,int timeout=-1,int wakeMask=QS_ALLINPUT,int flags=MWMO_INPUTAVAILABLE)
{
	flags&=(~MWMO_WAITALL); 
	handle_array<1> hh;
	if(HANDLE(handle)==INVALID_HANDLE_VALUE)
	{
        hh[0]=OpenProcess(PROCESS_ALL_ACCESS,0,GetCurrentProcessId());
		handle=hh[0];
	}
	//(void*)GetCurrentProcess()
	DWORD ws=MsgWaitForMultipleObjectsEx(1, (HANDLE* )&handle,timeout,wakeMask,flags|MWMO_ALERTABLE);
	if(ws==WAIT_OBJECT_0) return 0;
	else if(ws==(WAIT_OBJECT_0+1)) return 1;
	else if(ws==WAIT_IO_COMPLETION) return 2;
    else if(ws==WAIT_TIMEOUT) return 3;
	else return -1;
}

inline int  loop_event_msg(HANDLE hko,int timeout=-1,bool fquit=false)
{
	DWORD ws;
	

		while(ws=loop_event(hko,timeout))
		{
			try
			{
			MSG msg;
			//if(ws<0)
				while(PeekMessage(&msg,0,0,0,PM_REMOVE))
				{
					if((fquit)&&(msg.message==WM_QUIT)) return 0;
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			catch (...){}

		}
	

	return ws;
}



struct _initer_finiter_fake
{
	_initer_finiter_fake(void*p){};
};

template<class T,int nhandles=1,class IniterFiniter=_initer_finiter_fake>
struct thread_base
{
  unsigned m_tid;
  handle_array<nhandles> m_handles;


  thread_base():m_tid(0){};
  virtual  ~thread_base(){};

inline int id()
{
	return m_tid;
}

inline int _ht_ind()
{
  return 0;
}
inline handle_array<nhandles>& handles()
{
	return m_handles;
}

inline HANDLE& hthread()
{
	T* pthis= static_cast<T*>(this);
   return m_handles[pthis->_ht_ind()];
}

inline int terminate(int e=0)
{
  return TerminateThread(hthread(),e);
}

inline 	HANDLE run(bool fdirect=false,bool fapi=false)
	{
		
        T* pthis= static_cast<T*>(this);
		if(fdirect) 
		{
            m_tid=GetCurrentThreadId();
			hthread()=OpenProcess(PROCESS_ALL_ACCESS,0,m_tid);
			return (HANDLE)pthis->thread_proc();
		}
		
		
		HANDLE h;
		//ht[0]=CreateThread(NULL,0,&s_thread_proc,pthis,0,&dw);
		if(fapi) h=hthread()=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE )&s_thread_proc,pthis,CREATE_SUSPENDED,(DWORD*)&m_tid);
          else h=hthread()=(HANDLE)_beginthreadex(0,0,&s_thread_proc,pthis,CREATE_SUSPENDED,&m_tid);
		  if(h) ResumeThread(h);
		return h;

	};



 inline int  thread_proc(){return 0;};

 inline bool join(long timeout=INFINITE,bool fmsg=0)
 {
	 HANDLE h=hthread();
	 if(h==0) return true;
	 if(fmsg) return WAIT_TIMEOUT!=loop_event_msg(h,timeout);
	 else  return WAIT_TIMEOUT!=WaitForSingleObject(h,timeout);
 }

inline int exit_code()
{
	DWORD dw=-1;
	GetExitCodeThread(hthread(),&dw);
	return dw;
}

protected:
	
	static unsigned int __stdcall s_thread_proc(void* p )
	{
		try
		{
			T* pt=static_cast<T*>(p);
			IniterFiniter irfr(pt);
			return  pt->thread_proc();
			
	   	}
		catch(...){}
		
	};

};







template<class T,int N_add=0>
struct thread_pulser:thread_base< thread_pulser<T,N_add> >
{
	enum
	{
       ko_base=0,
       ko_event_begin=ko_base+0,
	   ko_source_thread=ko_base+1,
       ko_event_end=ko_base+2,
       ko_event_pulse=ko_base+3,
	   ko_mutex=ko_base+4,
	   ko_mapfile=ko_base+5
	};
 typedef typename      thread_pulser<T,N_add> pulser_type;
	enum {N=4+N_add};
	handle_array<N> handles;
 thread_pulser(handle_array_base<N>& h)	
 {
    handles=h;
 };

inline bool lock(int t=INFINITE)
{
	DWORD ws==WaitForSingleObject(handles[ko_mutex],t);
	return (WAIT_OBJECT_0==ws)||(WAIT_ABANDONED==ws);
};

inline bool unlock()
{
	return ReleaseMutex(handles[ko_mutex]);
}
bool is_connect()
{
  return  WAIT_OBJECT_0==WaitForSingleObject(handles[ko_mutex],0);
}

bool pulse()
{
	//
    //
	ResetEvent(handles[ko_event_end]);
	if(WAIT_OBJECT_0!=WaitForMultipleObjects(2,&handles[ko_event_begin],false,INFINITE))
		return false; 
	PulseEvent(handles[ko_event_pulse]); 
	if(WAIT_OBJECT_0==WaitForMultipleObjects(2,&handles[ko_source_thread],false,INFINITE))
		return false; 

	return true;
	//PulseEvent(hevent0); 
	//SignalObjectAndWait(hevent1,hevent0,INFINITE,0); 
};



int do_signal(){return -1;}


inline int  thread_proc_once()
{
	int ec;
    T* pthis= static_cast<T*>(this);
	SignalObjectAndWait(handles[ko_event_begin],handles[ko_event_pulse],INFINITE,0); 
	ec=pthis->do_signal();
	SetEvent(handles[ko_event_end]);
	return ec;

}

inline int  thread_proc()
{
	int ec;
	while(!(ec= thread_proc_once()))
	{};
	return ec;	
};

};

template<class T>
inline T& ref(T t)
{
	return t;
}





template <typename N>
N& make_ref(N t=N())
{
	return t;
};


template<class RetType,class Arg,class Functor>
RetType call_in_thread_context(Functor  &fun,Arg& arg,HANDLE hevent,HANDLE hthread,RetType defV=RetType())
{

	struct thread_context_caller
	{
		typedef typename thread_context_caller* this_t;     

		RetType m_result;
		Arg& m_arg;
		Functor  & m_functor;
		HANDLE m_hevent;
		inline void call_proc()
		{
			WaitForSingleObject(m_hevent,0);//  cache reset
			try{
				m_result=m_functor(m_arg);
			}catch (...){};

			::SetEvent(m_hevent);         
		}
		static VOID CALLBACK s_APC(ULONG_PTR dw)
		{
			this_t(dw)->call_proc();
		}



		thread_context_caller( Functor  & f,Arg& a,HANDLE hevent, 
			HANDLE hthread,RetType& defV):m_functor(f),m_arg(a),m_hevent(hevent)
		{
			DWORD wr;
			HRESULT hr;
			m_result=defV;

			if(WaitForSingleObject(hthread,0)==WAIT_TIMEOUT) //  cache reset
			{ 
				::ResetEvent(hevent);
				void *p=static_cast<void*>(this);
				HANDLE h[2]={hevent,hthread};
				if(::QueueUserAPC(&s_APC,hthread,ULONG_PTR(p))) 
					while(WAIT_IO_COMPLETION==(wr=::WaitForMultipleObjectsEx(2,h,false,INFINITE,true)));
			}
			hr=GetLastError();

		}

		inline	RetType& operator()()
		{
			return m_result;
		}
	};


	return thread_context_caller(fun,arg,hevent,hthread,defV)();

};


template<class RetType,class Arg,class Functor>
RetType call_in_thread_context(Functor  &fun,Arg& arg,int tid=::GetCurrentThreadId(),RetType defV=RetType())
{


	struct s_gc
	{
		HANDLE h[2];
		~s_gc()
		{
			::CloseHandle(h[0]);
			::CloseHandle(h[1]);
		}
	} 
	gc={
		::CreateEvent(0,true,0,0),
		::OpenThread(THREAD_SET_CONTEXT|SYNCHRONIZE,false,tid)		
		//::OpenThread(THREAD_ALL_ACCESS,false,tid)		
	};

	return   call_in_thread_context<RetType>(fun,arg,gc.h[0],gc.h[1],defV);

}




struct initializator_singleton
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
   
 

struct locker
{

	locker()
	{
		initializator_singleton::getCS()->lock();
	}
	~locker()
	{
		initializator_singleton::getCS()->unlock();
	}
};

inline static CS*  getCS()
{
    struct CS_deleter
	{
        CS* p;
		CS_deleter():p(0)
		{

		};
		~CS_deleter()
		{
            InterlockedExchangeAdd((LONG volatile *)&p,0);
			delete p;
		};
	};

	static CS* pcs;
	static CS_deleter deleter;

	if(!InterlockedExchangeAdd((LONG volatile *)&pcs,0))
	{
		
		CS* t=new CS;
		InterlockedCompareExchange((LONG volatile *)&pcs,(LONG)t,0);
		if(pcs==t) deleter.p=t;
		 else delete t;
		
	};

	return pcs;
}

};




/*
template<class RetType,class Arg,class Functor>
RetType call_in_thread_context(Functor  &fun,Arg arg,int tid=::GetCurrentThreadId())
{
  return call_in_thread_context_ref<RetType>(fun,arg,tid);
};
*/




/*
bool run(bool fdirect=false)
{
	if(fdirect) return thread_proc();
	handle_array<1> ht;
	DWORD dw;
	ht[0]=CreateThread(NULL,0,&s_thread_proc,this,0,&dw);
	return (ht[0]!=0);
};
protected:
	static DWORD __stdcall s_thread_proc(void* p )
	{
		return  static_cast< pulser_type*>(p)->thread_proc();
	};
*/




template <class Dispatcher,class IniterFiniter=_initer_finiter_fake>
struct pipe_thread:thread_base<pipe_thread<Dispatcher>,3,IniterFiniter>
{
	HRESULT m_hr;
	Dispatcher* pdisp;
	int m_t;
	pipe_thread(Dispatcher* p,int timeout=500):pdisp(p),m_hr(S_OK),m_t(timeout)
	{
		if(!p) {m_hr=E_POINTER; return ;}

		SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),NULL,true};

		if(!::CreatePipe(&handles()[1],&handles()[2],&sa,0))
			m_hr=HRESULT_FROM_WIN32(GetLastError());

	}

	operator HRESULT()
	{
		return m_hr;
	}


	int  thread_proc()
	{
		const int maxcb=4096;
		char buf[maxcb+2]; 
		HANDLE hf=handles()[1];
		DWORD cb=0,cb2=maxcb,cb3=-1;

		//while(PeekNamedPipe(hf,buf,maxcb,&cb2,&cb,&cb3))
		while(PeekNamedPipe(hf,0,0,0,&cb,0))
		{
			if(cb)
			{
				cb=min(cb,maxcb);  
				if(!ReadFile(hf,buf,cb,&cb,NULL)) break;
				buf[cb]=0;buf[cb+1]=0;
				pdisp->on_read(cb,buf);
			}
			cb3=cb2=maxcb;
			Sleep(m_t);
		}
	
		return 0;
	};

	int write(int cb,void* p)
	{
		HANDLE hf=handles()[2];
		DWORD cbt=-1;
		if(!WriteFile(hf,p,cb,&cbt,NULL))
			return -1;
		return cbt;
	}
};

