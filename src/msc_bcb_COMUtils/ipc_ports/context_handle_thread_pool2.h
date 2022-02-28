#pragma once

#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>

#include "ipc_utils.h"

#include <vector>
#include <list>
//#include "numeric/sparse_mkl.h"

#include "tpt.h"




template <int _VERS=0>
struct thread_pool_tree_t
{
	
	typedef thread_pool_tree_t<_VERS>* pthread_pool_job_t;
	typedef int (* job_proc_t)(void* p,int n);
	typedef int(*on_destroy_t)(void* p);

	enum{
		maxjobs= MAXIMUM_WAIT_OBJECTS
	};






	struct job_packet_inner_t:job_packet_t {

		
		static int s_run_pool_proc(job_packet_t* packet) {

			//return ((job_packet_inner_t*)packet)->run_child_pool();
			return packet->child_packets.run_pool(&packet->child_packets);

		}
		
		/*
		inline int run_child_pool() {

			thread_pool_tree_t* tpt =(thread_pool_tree_t*) child_packets.pool_context;
			if (tpt) {

				return tpt->safe_start_job_packets(child_packets.packets, child_packets.count);			
			
			}
			return -1;
		}*/


			
		inline int make() {

			child_run = &s_run_pool_proc;
			fill_job_packets(child_packets);
			//child_packets.run_pool = &run_job_packets_s;
			return proc(context,params,(job_packet_t*)this);
		};

		job_packet_inner_t() :job_packet_t() {
			
		}

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

		void init(int _n,thread_pool_tree_t* _owner){

			DWORD tid;
			n=(_n);
			owner=_owner;

			owner->hevents[n]=CreateEvent(0,true,false,0);
			owner->haborts[n]=CreateEvent(0,true,false,0);
			owner->hthreads[n]=CreateThread(0,0,(LPTHREAD_START_ROUTINE)& s_thread_proc,this,0,&tid);

			char* inf;
			v_buf<char> buf;
			thread_pool_tree_t* prnt = owner->owner_pool;
			if (prnt) {
				inf = buf.printf("PoolTree N:%d  deep: %d  parent: N%d ", int(n), owner->level,prnt->npool);
			}
			else {
				inf = buf.printf("PoolTree N:%d  deep: %d   root ", int(n), owner->level);
			}
			
			SetThreadName(tid,inf);
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






		thread_pool_tree_t* owner;


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



	static int run_job_packets_s(job_packets_t* pt) {

		thread_pool_tree_t* pool;

		if (pt && (pool = (thread_pool_tree_t*)pt->pool_context)) {
			return pool->safe_start_job_packets(pt->packets, pt->count);
		}		

		else
			return -1;
	};
	 
	struct super_packet_t:job_packet_t {

		operator bool() {

			return packets.size() > 1;
		}
		
		static int s_proc(void* context, void * params, job_packet_t* _this) {
			return  ((super_packet_t*)_this)->make();

		}

		super_packet_t() :job_packet_t() {
			proc = &s_proc;
		}
		inline int make() {
			int sz = packets.size();

			//job_packet_inner_t** pp = &packets[0];
			if(sz){
				job_packet_inner_t** pp =(job_packet_inner_t**) &packets[0];
				for (int k = 0; k < sz; k++) {
					job_packet_inner_t* p = pp[k];
					
					p->make();
				}
				
			}
			/*
			auto i = packets.begin();
			for (; i != packets.end();++i) {
				job_packet_t* p = *i;
				p->proc(p->context, p->params, p);
			}
			*/
			return sz;
			
		}

		inline pjob_packet_t push(pjob_packet_t p, integer_t nn, pthread_pool_job_t pool) {

			job_packets_t &c = p->child_packets;

			//if (!c.pool_context) 
			{
								
				c.thread_max = pool->njobs;
				pthread_pool_job_t* pc = pool->pchild_pools;
				c.pool_context = pc[nn];
				fill_job_packets(c);
			}
			packets.push_back(p);			
			n = nn;
			return this;
		};

		//
		std::vector<pjob_packet_t> packets;
		//		std::list<pjob_packet_t> packets;
	};

	int safe_start_job_packets(const  pjob_packet_t* pjp, int np) {

		if (pjp&&np) {

			/*
			if (njobs < np)
				return -1;*/

			std::vector<super_packet_t> super_packets(njobs);
			std::vector<job_packet_t*> packets_buf(njobs);

			job_packet_t** p_packets = (njobs) ? &packets_buf[0] : NULL;

			
			for (int k = 0; k < np; k++) {
				pjob_packet_t p = pjp[k];
				if (p&&p->proc) {
					integer_t n = p->n;

					p_packets[n] = super_packets[n].push(p, n,this);
					//p_packets[n]->context = (void*)this;
					//p_packets[n] = &super_packets[int(n)];
				}
			}

			return start_job_packets(p_packets, njobs);
		}
		return 0;

	}


	int start_job_packets(const  pjob_packet_t* pjp,int np)
	{
		DWORD ws;



		
		

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
						return -1;
					}



				}
			}
				  
		}

		

		WaitAll(nactive,hes);
		
		return nactive;
	}


	bool set_thread_affinity(int n, KAFFINITY Mask, WORD Group=0) {
		BOOL f;
		if (n == -1) {
			f = true;
			for (int k = 0; k < njobs; k++) {
				f = f && set_thread_affinity(k, Mask, Group);
			}
			return f;
		}
		if (!check_jobs(n))
			return false;
		GROUP_AFFINITY ga = { Mask,Group },tmp;
		
		f=::SetThreadGroupAffinity(hthreads[n], &ga, &tmp);
		if (!f) syserror = GetLastError();
		return f;
	}

	HRESULT tree_set_thread_affinity(int l,int* nn, KAFFINITY Mask, WORD Group = 0) {
		
		BOOL f;
		if (l <= 0) {
			return E_INVALIDARG;
			if (l <2)
			 f= set_thread_affinity(*nn, Mask, Group);
			else {
				int n = *nn;
				if (n < child_pools.count())
				{
					f=child_pools[n]->tree_set_thread_affinity(l - 1, nn + 1, Mask, Group);
				 }
				else f= false;
			}
		}
		else f=false;

		return f ? S_OK : HRESULT_FROM_WIN32(GetLastError());
				
	}

	inline static void fill_job_packets(job_packets_t& p) {
		//p.pool_context = (void*)pool;
		p.run_pool = &run_job_packets_s;
		p.set_affinity =&s_tree_set_thread_affinity;
	}

	static long s_tree_set_thread_affinity(job_packets_t* pt,int l,int* nn, KAFFINITY Mask, WORD Group ) {		 

		thread_pool_tree_t* pool;

		if (pt && (pool = (thread_pool_tree_t*)pt->pool_context)) {
			return pool->tree_set_thread_affinity(l,(int*)nn,Mask,Group);
		}

		else return E_POINTER;
	};
	

	static int all_processor_count() {
		return GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
	}

	static int processor_group_count() {
		return GetMaximumProcessorGroupCount();
	}



	thread_pool_tree_t(int _njobs=-1,int g=0)
		:njobs(0), ngroup(g) ,owner_pool(NULL), npool(0), level(0) {
		
		if(_njobs)
			reset(_njobs);
	}

	thread_pool_tree_t(int* njobtree, int deep=1) 
		:njobs(0), ngroup(0), owner_pool(NULL), npool(0),level(0) {

		reset_ex(deep,njobtree);
	}
	static void WaitAll(int n, const HANDLE* h) {

		while (n >= 64) {
			WaitForMultipleObjects(64, h, true, INFINITE);
			n -= 64;
			h += 64;
		}
		WaitForMultipleObjects(n,h, true, INFINITE);	
		
	}

	thread_pool_tree_t& reset_ex(int deep = 0,int *pnjobs=NULL, thread_pool_tree_t* _owner_pool =NULL,int _npool=0,int _level=0)
	{
		owner_pool = _owner_pool;
		npool = _npool;
		level = _level;

		thread_pool_tree_t* pc;

		if (!deep) {
			reset(); 
		}
		else {
			
			reset(*pnjobs);
			if (deep> 1) {
				for (int n = 0; n < njobs; n++) {
					  
					pc = new  thread_pool_tree_t(0);
					pc->reset_ex(deep - 1, pnjobs + 1 ,this, n, _level+1);
					child_pools[n] = pc;
					//pc->owner_pool = this;
					//pc->npool = n;
					//

				}
			}

		}
		
		return *this;
	}


	thread_pool_tree_t& reset(int new_njobs=0)
	{
		if (new_njobs == -1)
			new_njobs = all_processor_count();
		
		unlock_events(njobs,haborts);
		
		WaitAll(njobs,haborts);

		root_packets = job_packets_t();
		fill_job_packets(root_packets);
		root_packets.pool_context = (void*)this;
		

		//root_packets

		for(int n=0;n<njobs;n++) 
		{
			
			//TerminateThread(hthreads[n],-1);
			jobs[n].close();
			thread_pool_tree_t*& pc =child_pools[n];
			delete pc;
			pc = 0;
		}

		root_packets.thread_max = njobs=new_njobs;

		hevents.resize(njobs);
		hes.resize(njobs);
		hthreads.resize(njobs);
		haborts.resize(njobs);
		jobs.resize(njobs);
		pchild_pools=child_pools.resize(njobs).get();
		

		//hprocess=OpenProcess(PROCESS_ALL_ACCESS,0,GetCurrentProcessId());
		for(int n=0;n<njobs;n++)
		{
			jobs[n].init(n,this);
		}

		return *this;

	}

	~thread_pool_tree_t()
	{
		reset(0);		
	}

	static void on_destroy_s(void*p) {
		delete static_cast<thread_pool_tree_t*>(p);
	}
	static void on_destroy_from_root_packet_s(void*p) {
		char* pc = (char*)p;
		pc -= offsetof(thread_pool_tree_t, root_packets);
		delete static_cast<thread_pool_tree_t*>((void*)pc);
	}


	inline static void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
	{

		struct {
			ULONG_PTR dwType; // must be 0x1000
			LPCSTR szName; // pointer to name (in user addr space)
			ULONG_PTR dwThreadID; // thread ID (-1=caller thread)
			ULONG_PTR dwFlags; // reserved for future use, must be zero
		} 	info = { 0x1000,szThreadName,dwThreadID,0 };



		__try
		{
			RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		}
		__except (EXCEPTION_CONTINUE_EXECUTION) {}
	};



	int njobs;
	v_buf<HANDLE> hevents,hes;
	v_buf<HANDLE> hthreads;
	v_buf<HANDLE> haborts;
	v_buf<job_t> jobs;
	
	v_buf<pthread_pool_job_t> child_pools;
	pthread_pool_job_t* pchild_pools;

	int ngroup;
	job_proc_t  job_proc;
	void * context, *context2;
	HANDLE hprocess;
	HRESULT syserror;

	pthread_pool_job_t owner_pool;
	int npool, level;
	v_buf<char> info;
	job_packets_t root_packets;
	//WaitForMultipleObjects()
};

template <class T,class P=void*>
struct packet_base_t:job_packet_t {


	typedef job_packet_t packet_t;

	int make(P *params) {
		return 0;
	}

	static int packet_proc_s(void* context, void* params, packet_t* packet) {
		
		    T* t = static_cast<T*>(packet->context);
			return t->make((P)packet->params, packet);
	};

	static int on_destroy_s(void*p) {
		delete static_cast<T*>(p);
	}

	packet_base_t(P prms,int _n=0,int g=0) :job_packet_t() {
	
		context=static_cast<T*>(this);
		proc = &packet_proc_s;
		//on_destroy = &on_destroy_s;
		params = (void*)prms;
		n = _n;
		ngroup = g;
	};

	HRESULT last_error() {
		thread_pool_tree_t<>::job_t* pjob = (thread_pool_tree_t<>::job_t*)inner_ptr;
		return pjob->owner->syserror;
	}

	bool set_thread_affinity(KAFFINITY Mask, WORD Group = 0) {

	 thread_pool_tree_t<>::job_t* pjob=(thread_pool_tree_t<>::job_t*)inner_ptr;
	 return pjob->owner->set_thread_affinity(Mask, Group);
	}

	
};