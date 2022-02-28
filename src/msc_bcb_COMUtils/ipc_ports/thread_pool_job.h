#pragma once

#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>

//#include "ipc_ports/exp_pade_data.h"
//#include "numeric/sparse_mkl.h"
template <int _VERS=0>
struct thread_pool_job_t
{
	

	typedef int (* job_proc_t)(void* p,int n);

	enum{
		maxjobs= MAXIMUM_WAIT_OBJECTS
	};

	typedef	struct job_packet_t {
		typedef int(*packet_proc_t)(void* context,void* params);
		
		packet_proc_t proc;
		void *context;
		void* params;
		int n;
	} job_packet_t,*pjob_packet_t;

	struct job_t
	{
		int n;
		UINT_PTR res;
		HRESULT syserror;


		static DWORD __stdcall s_thread_proc(void* p)
		{
			job_t* j=((job_t*)p);
			HANDLE habort=j->owner->haborts[j->n];
			while(WAIT_IO_COMPLETION==WaitForSingleObjectEx(habort,INFINITE,true)){

			};
			return 0;
		}
		static void __stdcall s_job_proc(ULONG_PTR up)
		{
			job_t* j=((job_t*)up);
			j-> make_job();
		}

		int make_job()
		{
			
			__try{
				syserror=0;

				job_proc_t jproc;
				void*  context;
				jproc=(job_proc_t)InterlockedCompareExchangePointer((PVOID volatile*)&owner->job_proc,0,0);
				context=InterlockedCompareExchangePointer((PVOID volatile*)&owner->context,0,0);
				if(jproc)
					res=jproc(context,n);
				else syserror=-1;
			}__except(EXCEPTION_EXECUTE_HANDLER)
			{
				syserror=-1;
			}
			
			SetEvent(owner->hevents[n]);
			return res;
		}

		job_t():owner(0){}
		void init(int _n,thread_pool_job_t* _owner){

			DWORD tid;
			n=(_n);
			owner=_owner;

			owner->hevents[n]=CreateEvent(0,true,false,0);
			owner->haborts[n]=CreateEvent(0,true,false,0);
			owner->hthreads[n]=CreateThread(0,0,(LPTHREAD_START_ROUTINE)& s_thread_proc,this,0,&tid);
		}    

		int start(){

          DWORD f;
			ResetEvent(owner->hevents[n]); 
			f=QueueUserAPC(&s_job_proc,owner->hthreads[n],ULONG_PTR((void*)this)); 
			if(!f)
				syserror=GetLastError();

			return f;

		}

		~job_t(){ close();}

		void close()
		{
			if(owner)
			{
				CloseHandle(owner->hthreads[n]); 
				CloseHandle(owner->haborts[n]); 
				CloseHandle(owner->hevents[n]); 
				owner=0;

			}

		}






		thread_pool_job_t* owner;


	};

	inline    static void lock_events(int num,HANDLE *ph){


		for(int n=0;n<num;n++)
			ResetEvent(ph[n]);

	}

	inline    static void unlock_events(int num,HANDLE *ph){

		for(int n=0;n<num;n++)
			SetEvent(ph[n]);
	}


	inline bool check_jobs(int n)
	{
		return (n>=0)&&(n<njobs);
	}




	int start_jobs(job_proc_t jobproc,void * _context)
	{
		DWORD ws;
		int ns;
		//job_proc=jobproc;
		//context=_context;
		InterlockedExchangePointer((PVOID volatile*)&job_proc,jobproc);
		InterlockedExchangePointer((PVOID volatile*)&context,_context);

		for(int n=0;n<njobs;n++)
			jobs[n].start();

		ws=WaitForMultipleObjects(njobs,hevents,true,INFINITE);
		ns=ws-WAIT_OBJECT_0;
		if(!check_jobs(ns)) return 0;

		return njobs;
	}

	int get_index_shift_handle(int ns,int& N,HANDLE* ph,int* pindex){

		if((ns>=0)&&(ns<N))
		{

			int inxd=pindex[ns];
			if(!check_jobs(inxd)) return -1;

			--N;
			for(int k=ns;k<N;k++)
		 {
			 pindex[k]=pindex[k+1];
			 ph[k]=ph[k+1];
		 }
			return inxd;
		}
		else return -1;
	}

	int start_jobs(job_proc_t jobproc,void * _context,job_proc_t jobproc_complete,void * _context2=0)
	{

		if(!jobproc_complete) return  start_jobs(jobproc,_context);

		HANDLE hes[maxjobs];
		int indexes[maxjobs];

		//job_proc=jobproc;
		//context=_context;
		InterlockedExchangePointer((PVOID volatile*)&job_proc,jobproc);
		InterlockedExchangePointer((PVOID volatile*)&context,_context);

		context2=(_context2) ?_context2:_context;



		for(int n=njobs-1;n>=0;n--)
		{
			indexes[n]=n; 
			hes[n]=hevents[n];
			jobs[n].start();
		}		



		DWORD ws;
		int ns,N=njobs,i;


		while(N)
		{

			ws=WaitForMultipleObjects(N,hes,false,INFINITE);
			ns=ws-WAIT_OBJECT_0;
			//if(!check_jobs(ns)) return 0;
			i=get_index_shift_handle(ns,N,hes,indexes);
			if(i<0) return -1;
			//hes[i]=hprocess;
			jobproc_complete(context2,i);

		}


		return njobs;
	}


	thread_pool_job_t(int _njobs=0):njobs(0)
	{
		if(_njobs)
			reset(_njobs);
	}

	thread_pool_job_t& reset(int new_njobs=0)
	{
		unlock_events(njobs,haborts);
		WaitForMultipleObjects(njobs,haborts,true,INFINITE);
		for(int n=0;n<njobs;n++) 
		{
			
			//TerminateThread(hthreads[n],-1);
			jobs[n].close();

		}

		njobs=new_njobs;


		//hprocess=OpenProcess(PROCESS_ALL_ACCESS,0,GetCurrentProcessId());
		for(int n=0;n<njobs;n++)
		{
			jobs[n].init(n,this);
		}

		return *this;

	}

	~thread_pool_job_t()
	{
		reset(0);		
	}

	int njobs;
	HANDLE hevents[maxjobs];
	HANDLE hthreads[maxjobs];
	HANDLE haborts[maxjobs];
	job_t jobs[maxjobs];

	job_proc_t  job_proc;
	void * context, *context2;
	HANDLE hprocess;

	//WaitForMultipleObjects()
};