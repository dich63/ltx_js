#pragma once

#include <windows.h> 

inline LONG64 get_process_time(HANDLE hp=GetCurrentProcess())
{
	FILETIME ft,ftt;
   LONG64 ftUser, ftKernel;
   bool f=GetProcessTimes(hp, &ft, &ftt,(FILETIME*)&ftKernel,(FILETIME*)&ftUser);
   if(f) return ftUser+ftKernel;
   return 0;
}

inline LONG64 get_thread_time(HANDLE hp=GetCurrentThread())
{
	FILETIME ft,ftt;
	LONG64 ftUser, ftKernel;
	bool f=GetThreadTimes(hp, &ft, &ftt,(FILETIME*)&ftKernel,(FILETIME*)&ftUser);
	if(f)	return ftUser;//+ftKernel;
	return 0;
}

inline LONG64 get_system_time()
{
	LONG64 ftIdle;
	LONG64 ftUser, ftKernel;
	bool f=GetSystemTimes((FILETIME*)&ftIdle,(FILETIME*)&ftKernel,(FILETIME*)&ftUser); 
	//GetSystemTimeAsFileTime((FILETIME*)&ft);
	if(f) return ftIdle+ftUser+ftKernel;
	return 0;
}

struct process_usage
{
	LONG64 tp,ts;
	HANDLE hp;
	process_usage(HANDLE h=GetCurrentProcess()):hp(h){start();};

inline double start()
{
	//double r=get();
ts=get_system_time();
  tp=get_process_time(hp);
  
  return 0;
};
inline double get()
{
    LONG64 p,s;
	p=get_process_time(hp)-tp;
	s=get_system_time()-ts;
    if(s==0) return s;
   return double(p)/double(s);
}
};

struct thread_usage
{
	LONG64 tp,ts;
	HANDLE hp;
	thread_usage(HANDLE h=GetCurrentThread()):hp(h){start();};

	inline double start()
	{
		//double r=get();
		ts=get_system_time();
		tp=get_thread_time(hp);
		
		
		return 0;
	}
	inline double get()
	{
		LONG64 p,s;
		p=get_thread_time(hp)-tp;
		s=get_system_time()-ts;
		if(s==0) return 0;
		return double(p)/double(s);
	}

	
};

struct thread_group_usage
{
	LONG64 tp;
	LONG64 ts;
	HANDLE hp;
    PHANDLE pht;
	LONG64 tt[64];
	double dpt;
    double dtt[64];
	int count;
	thread_group_usage(int cnt, PHANDLE pt,HANDLE h=GetCurrentProcess())
		:hp(h),count(cnt),pht(pt),dpt(0)
	{		//start();	
	};

	inline void start()
	{
		//double r=get();
		ts=get_system_time();
        tp=get_process_time(hp);
        for(int n=0;n<count;n++)
		tt[n]=get_thread_time(pht[n]);
         
		
	}

	inline void stop()
	{
		//double r=get();
		
		LONG64 t;
		for(int n=0;n<count;n++)
			tt[n]=(t=get_thread_time(pht[n]))? t-tt[n]: 0;
		
		tp=(t=get_process_time(hp))? t-tp: 0;
		ts=(t=get_system_time())? t-ts: 0;
		
		
	};

inline double of_process()
{
	if(tp<ts) return double(tp)/double(ts);
	return 0;
};

inline double of_thread(int n,bool fperprocess=true)
{
	
   LONG64 t=tt[n],tps;
   if(fperprocess) tps=tp;
   else tps=ts;
   if((tps>0)&&(t<=tps))
	   return double(t)/double(tps);
 	return 0;
}


};

