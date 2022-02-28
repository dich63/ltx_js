#pragma once

#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>

#include "ipc_ports/ipc_utils.h"
//#include "numeric/sparse_mkl.h"





template <int _VERS=0>
struct thread_pool_job_t
{
	

	typedef int (* job_proc_t)(void* p,int n);
	typedef int(*on_destroy_t)(void* p);

	enum{
		maxjobs= MAXIMUM_WAIT_OBJECTS
	};


	typedef	struct job_packet_t {

		typedef int(*packet_proc_t)(void* context,void* params, job_packet_t* packet);
		//on_destroy_t  on_destroy;
		packet_proc_t proc;		
		void *context;
		void* params;
		job_packet_t* on_complete_packet;
		INT_PTR  n, ngroup;		
		void* inner_ptr;

	} job_packet_t, *pjob_packet_t;




	struct job_packet_inner_t:job_packet_t {

		int make() {

			return proc(context,params,(job_packet_t*)this);
		};

		job_packet_inner_t() :job_packet_t() {}

	};

	struct  handles_t {
		typedef HANDLE* PHANDLE;
		PHANDLE h;
		int njob,nactive;
		inline 	operator PHANDLE() {
			return h;
		}
		handles_t(int _njob):njob(_njob),nactive(0){
			
		}
		~handles_t() {			
			::free(h);
		}

		inline void reset(int _njob) {
			int sz;
			for (int n = 0; n < njob; n++) {
				CloseHandle(h[n]);
			}
			njob = _njob;

			sz = sizeof(HANDLE)*_njob;
			h = (PHANDLE)realloc(h,sz, sizeof(HANDLE));
			memset(h, 0, sz);


		}

		inline 	 HANDLE& operator [](int k) {

			return h[k];
		}

	};

	struct job_t
	{
		struct pair_packet_job_t {
			job_packet_inner_t* p;
			job_t* j;
			pair_packet_job_t(job_packet_t* _p, job_t* _j) :p((job_packet_inner_t*)_p), j(_j) {}
		};

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
			//pair_packet_job_t*  pj = (pair_packet_job_t*)up;
			job_packet_inner_t* p = (job_packet_inner_t*)up;
			job_t* j = (job_t*)p->inner_ptr;
			
			j-> make_job(p);

			//delete pj;			
			
		}

		int make_job(job_packet_inner_t* jp)
		{
			
			__try{
				syserror=0;
				if(jp->proc) 
					 jp->make();
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

		int push(pjob_packet_t pjp){

          DWORD f;

		    

			ResetEvent(owner->hevents[n]); 

			//pair_packet_job_t * pj = new pair_packet_job_t(pjp, this);
			pjp->inner_ptr = (void*)this;

			f=QueueUserAPC(&s_job_proc,owner->hthreads[n],ULONG_PTR((void*)pjp));
			if (!f){
				syserror = GetLastError();
				//delete pj;
			}

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





	int start_job_packets( pjob_packet_t* pjp,int np)
	{
		DWORD ws;

		HANDLE hes[maxjobs];
		int indexes[maxjobs];

		int ns;
		int nactive = 0;
		int ng =ngroup;
		syserror = 0;
		
		
		//job_proc=jobproc;
		//context=_context;
		
		for (int k = 0; k < np; k++) {
			pjob_packet_t p = pjp[k];
			if (p) {
				job_packet_t& jp = *(p);
				int n = jp.n;

				//if ((jp.ngroup == ng) && (jp.proc)) {
				if (jp.proc) {

					hes[nactive++] = hevents[n];
					if (!jobs[n].push(&jp)) {
						syserror = jobs[n].syserror;
						return 0;
					}



				}
			}
				  
		}

		

		ws=WaitForMultipleObjects(nactive,hevents,true,INFINITE);
		ns=ws-WAIT_OBJECT_0;

		if(!(ns >= 0) && (ns<njobs)) return 0;

		return njobs;
	}

	bool set_thread_affinity(int n, KAFFINITY Mask, WORD Group=0) {
		if (!check_jobs(n))
			return false;
		GROUP_AFFINITY ga = { Mask,Group },tmp;
		BOOL f;
		f=::SetThreadGroupAffinity(hthreads[n], &ga, &tmp);
		if (!f) syserror = GetLastError();
		return f;
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



	static int all_processor_count() {
		return GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
	}

	static int processor_group_count() {
		return GetMaximumProcessorGroupCount();
	}



	thread_pool_job_t(int _njobs=-1,int g=0):njobs(0), ngroup(g)	{		
		
		if(_njobs)
			reset(_njobs);
	}

	thread_pool_job_t& reset(int new_njobs=0)
	{
		if (new_njobs == -1)
			new_njobs = all_processor_count();
			  
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
	int ngroup;
	job_proc_t  job_proc;
	void * context, *context2;
	HANDLE hprocess;
	HRESULT syserror;

	//WaitForMultipleObjects()
};

template <class T,class P=void*>
struct packet_base_t:thread_pool_job_t<>::job_packet_t {


	typedef job_packet_t packet_t;

	int make(P *params) {
		return 0;
	}

	static int packet_proc_s(void* context, void* params, packet_t* packet) {
		return static_cast<T*>(context)->make((P)params, packet);		
	};

	static int on_destroy_s(void*p) {
		delete static_cast<T*>(p);
	}

	packet_base_t(P prms,int _n=0,int g=0) :thread_pool_job_t<>::job_packet_t() {
	
		context=static_cast<T*>(this);
		proc = &packet_proc_s;
		//on_destroy = &on_destroy_s;
		params = (void*)prms;
		n = _n;
		ngroup = g;
	};

	HRESULT last_error() {
		thread_pool_job_t<>::job_t* pjob = (thread_pool_job_t<>::job_t*)inner_ptr;
		return pjob->owner->syserror;
	}
	bool set_thread_affinity(KAFFINITY Mask, WORD Group = 0) {

	 thread_pool_job_t<>::job_t* pjob=(thread_pool_job_t<>::job_t*)inner_ptr;
	 return pjob->owner->set_thread_affinity(Mask, Group);
	}

	
};