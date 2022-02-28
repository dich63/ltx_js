#pragma once 
#include <windows.h>


struct thread_pool_t
{
    
	std::vector<HANDLE> hthreads; 
    int nt;
	  thread_pool_t():nt(0)
	  {
	  }

inline static void  __stdcall  s_proc(void* p)
{
	for(;;)
	{
	__try
	{
       SleepEx(INFINITE,true);  		
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
          FatalAppExitW(0,L"thread_pool error!!!");
	}
	}
}

inline void set_pool_priority(DWORD prior=THREAD_PRIORITY_NORMAL)
{
 for(int n=0;n<nt;++n)
	 SetThreadPriority(hthreads[n],prior);
}
inline thread_pool_t& init(int num,DWORD prior=THREAD_PRIORITY_NORMAL)
{
	clear();
	nt=num;
     hthreads.resize(nt);
	 HANDLE ht;
	 DWORD tid;
	 for(int n=0;n<nt;++n)
	 {
       ht=CreateThread(0,0,LPTHREAD_START_ROUTINE(&s_proc),0,0,&tid); 
	   if(!ht) FatalAppExitW(0,L"thread create fail!!!");
	   SetThreadPriority(ht,prior);
       hthreads[n]=ht;
	 }
	 return *this;
}

inline   void clear()
  {
	  for(int n=0;n<nt;++n)
	  {
		  HANDLE ht=hthreads[n];
		  TerminateThread(ht,0);
		  CloseHandle(ht);
	  }
  }

~thread_pool_t()
{
	clear();
}

inline bool call(int n,void* proc,void* param)
{
   return QueueUserAPC(PAPCFUNC(proc),hthreads[n],ULONG_PTR(param)); 
}
};


template <class Handler,bool finnerpool=false>
struct thread_splitter_t
{

	std::vector<HANDLE> mutexes; 
	std::vector<HANDLE> events;
	typedef typename thread_splitter_t<Handler,finnerpool> selt_t;
	typedef  selt_t* this_t;
	typedef Handler handler_t;
	handler_t* handler;
	typedef typename std::pair<int,this_t> item_t;
	std::vector<item_t> items; 
	int nt;
	thread_pool_t inner_pool;



	inline 	void proc(int n)
	{
		(*handler)(n,nt);
	}
	static		void __stdcall s_proc( item_t* p)
	{         
		DWORD ws;
		this_t t=p->second;
		int n=p->first;
		//HANDLE hm=t->mutexes[n];
		HANDLE he=t->events[n];
		//ws=WaitForSingleObject(hm,INFINITE);
		//ws=SetEvent(he);
		__try
		{
			t->proc(n);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)	{
			FatalAppExitW(0,L"pool error!!!");
		}
		ws=SetEvent(he);
		//ws=ReleaseMutex(hm);


	};

	DWORD run(int timeout=INFINITE)
	{

		if(!handler)  return -2;
		if(nt<=0) return -3;
       BOOL f;
		for(int n=0;n<nt;++n)
		{
			f=ResetEvent(events[n]);
			//f=ReleaseMutex(mutexes[n]);
			
			if(finnerpool)
			{
               inner_pool.call(n,&s_proc,&items[n]);
			}
			else
			if(!QueueUserWorkItem((LPTHREAD_START_ROUTINE)&s_proc,&items[n],WT_EXECUTELONGFUNCTION))
				throw 1;
		}


		return wait(timeout);


	}

	inline DWORD wait(int timeout=INFINITE)
	{
		DWORD ww;
		if(nt<=0) return -2;
		ww=WaitForMultipleObjects(nt,&(events[0]),1,timeout);
		//ww= WaitForMultipleObjects(nt,&(mutexes[0]),1,timeout);
		return ww;
	}


	static inline      int get_num_processors()
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return si.dwNumberOfProcessors;
	};

	thread_splitter_t(handler_t* _handler=0,int numt=get_num_processors(),DWORD prior=THREAD_PRIORITY_NORMAL):nt(0)
		//:handler(_handler),nt(numt),mutexes(numt),events(numt),items(numt)
	{
		if(handler=_handler) init(_handler,numt,prior);
	}



	thread_splitter_t&  init(handler_t* _handler,int numt=get_num_processors(),DWORD prior=THREAD_PRIORITY_NORMAL)
	{ 
		close();
		nt=numt;
		handler=_handler;
		mutexes.resize(nt);
		events.resize(nt);
		items.resize(nt);

		for(int n=0;n<nt;++n)
		{
			events[n]=CreateEvent(0,1,1,0);
			mutexes[n]=CreateMutex(0,1,0);
			items[n].first=n;
			items[n].second=this;
		}
		if(finnerpool) inner_pool.init(nt,prior);
		return *this;
	} 
	void  close()
	{
		//wait();
		for(int n=0;n<nt;++n)
		{
			CloseHandle(mutexes[n]);
			CloseHandle(events[n]);
		}
		mutexes.clear();
		events.clear();
	}
	~thread_splitter_t()
	{
		close();
	}

};

