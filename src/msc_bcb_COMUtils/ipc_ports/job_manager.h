#pragma once
//#include "job_manager.h"

#include <windows.h>
#include <psapi.h>
#include <list>
#include <mutex>
#include <string>
#include <memory>
#include <map>



#define OLE_CHECK_ZERO(a) if(FAILED((a))) return 0;

#define OLE_CHECK_PTR(p) if(!(p)) return E_POINTER;
#define OLE_CHECK_VOID(a) if(FAILED((a))) return;
#define OLE_CHECK_VOID_hr(a) if(FAILED(hr=(a))) return;
#define OLE_CHECK_VOID_hr_if(f,h) if(f){ hr=HRESULT_FROM_WIN32(h);    return;};
#define OLE_CHECK_VOID_hr_cond(f,h) if(!(f)){ hr=HRESULT_FROM_WIN32(h);    return;} else { hr=S_OK;};
#define OLE_CHECK_hr_cond(f,h) if(!(f)){ hr=HRESULT_FROM_WIN32(h);    return hr;} else { hr=S_OK; };
#define OLE_CHECK_hr(a) if(FAILED( hr=(a) ) ) return hr;
#define OLE_CHECK_WIN32_hr(a) if(FAILED( hr=HRESULT_FROM_WIN32(a) ) ) return hr;
#define OLE_CHECK(a) ;{ HRESULT hr__0000;if(FAILED( hr__0000=(a) ) ) return hr__0000;};

//std::shared_ptr p;


///*


struct hkobject_t :non_copyable_t
{
	static HANDLE dup_handle(HANDLE h, HANDLE hcp=GetCurrentProcess())
	{
		BOOL f;
		if (h && (h != INVALID_HANDLE_VALUE))
		{
			HANDLE ht = GetCurrentProcess();
			f = ::DuplicateHandle(hcp, h, ht, &h, DUPLICATE_SAME_ACCESS, false, DUPLICATE_SAME_ACCESS);
			return f ? h : 0;
		}
		return 0;
	}

	inline hkobject_t& reset(HANDLE h = 0, HANDLE hcp = GetCurrentProcess())
	{
		if (hko) 
			::CloseHandle(hko);
		 hko = dup_handle(h, hcp);
		 return *this;
	}

	inline HANDLE dettach()
	{
		HANDLE t = hko;
		hko = 0;
		return t;
	}
	inline hkobject_t& attach(HANDLE h)
	{
		if (hko)
			::CloseHandle(hko);
		hko = h;
		return *this;
	}

	hkobject_t(HANDLE h ,bool fdup=true) :hko(0){
		if(fdup) reset(h);
		else hko = h;
	}
	hkobject_t() :hko(0){};
	~hkobject_t() {
		reset();
	}

	hkobject_t& operator=(HANDLE h){

		return reset(h);
	}


	operator HANDLE(){ return hko; };

	HANDLE hko;
};

//*/

template <int _VERS_ = 0>
struct job_manager_t{

	enum{
		nano100 = 10000000,
		mlsec=1000
	};

	typedef UINT64 uint64_t;
	typedef INT64 int64_t;
	typedef UINT32 uint32_t;
	typedef INT32 int32_t;


	typedef std::wstring string_t;

	typedef std::list<int> processor_list_t;
	typedef job_manager_t<_VERS_> manager_t;
	typedef std::recursive_mutex mutex_t;
	typedef std::lock_guard<mutex_t> locker_t;

	struct job_t;

	//typedef std::map<std::wstring, job_t*> jobmap_t;
	typedef std::map<uint32_t, job_t*> jobmap_t;
	typedef typename jobmap_t::iterator jobiterator_t;

	static std::wstring uniquie_name()
	{
		wchar_t buf[128];
		int ii[4];
		CoCreateGuid((GUID*)ii);
		wsprintfW(buf,L"%x%x%x%x-job-", ii[0], ii[1], ii[2], ii[3]);
		return buf;
	}

	struct job_t{

		//job_t() :manager(0), hr(E_POINTER), hasyn(0){};


		HANDLE* hasyn_address()
		{
			hasyn && UnregisterWait(hasyn);

			return &(hasyn = 0);
		}


		void wtc_proc(bool f_tio){


			terminate();

			if (f_tio && (!fulltime_viol))
			{
				fulltime_viol = true;

				if (SUCCEEDED(register_kobject(logtime)))
					return;
					
			}
			
			
				delete this;
			
				
		};

		static		void __stdcall s_wtc_proc(void* p, BOOLEAN f_tio)
		{
			static_cast<job_t*>(p)->wtc_proc(f_tio);
		}

		HRESULT register_kobject(double tio)// = INFINITE)
		{
			DWORD dwt = (tio<0) ? INFINITE : DWORD(tio * mlsec);
			
			if (RegisterWaitForSingleObject(hasyn_address(), hsignal, &s_wtc_proc, this, dwt, WT_EXECUTELONGFUNCTION | WT_EXECUTEONLYONCE))
				return S_OK;
			
			return HRESULT_FROM_WIN32(GetLastError());
		}


		bool get_violation()
		{
			bool f;
			DWORD dw;
			if (FAILED(hr)) return false;
			f = QueryInformationJobObject(hjob, JobObjectLimitViolationInformation, &jlvi, sizeof(jlvi), &dw);
			return f;
		}

		bool on_complete_port(DWORD msg_id, DWORD pid)
		{
			bool f;
			if (JOB_OBJECT_MSG_NOTIFICATION_LIMIT == msg_id)
			{
				f=get_violation();
				terminate();
			}
				return true;
		}

		static bool s_on_complete_port(ULONG_PTR key,DWORD msg_id,OVERLAPPED* ovl){
			 
			return static_cast<job_t*>((void*)key)->on_complete_port(msg_id, ovl ? DWORD(ovl) : 0);
		}

		HRESULT set_job_limits()
		{
			HRESULT hr;
			BOOL f;
			

			     

			     JOBOBJECT_ASSOCIATE_COMPLETION_PORT jacp = { (void*)key, manager->hiocp};

				 OLE_CHECK_hr_cond(f = ::SetInformationJobObject(hjob,JobObjectAssociateCompletionPortInformation, &jacp, sizeof(jacp)), GetLastError());



				 //JOBOBJECT_EXTENDED_LIMIT_INFORMATION j = {};
				 //j.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION | JOB_OBJECT_LIMIT_BREAKAWAY_OK | JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
				 //OLE_CHECK_hr_cond(f = SetInformationJobObject(hko, JobObjectExtendedLimitInformation, &j, sizeof(j)), GetLastError());

                JOBOBJECT_EXTENDED_LIMIT_INFORMATION j = {};
				JOBOBJECT_BASIC_LIMIT_INFORMATION& jbli = j.BasicLimitInformation;
				jbli.Affinity = list2affinity(processors);
				jbli.LimitFlags = JOB_OBJECT_LIMIT_AFFINITY|JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION | JOB_OBJECT_LIMIT_BREAKAWAY_OK | JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
				OLE_CHECK_hr_cond(f = ::SetInformationJobObject(hjob, JobObjectExtendedLimitInformation, &j, sizeof(j)), GetLastError());
			

				JOBOBJECT_NOTIFICATION_LIMIT_INFORMATION jnli = {};

				if (usertime > 0){
					jnli.PerJobUserTimeLimit.QuadPart = usertime * nano100;
					jnli.LimitFlags |= JOB_OBJECT_LIMIT_JOB_TIME;
				}
				if (memlimit > 0){
					jnli.JobMemoryLimit = memlimit;
					jnli.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
				}

				OLE_CHECK_hr_cond(f = ::SetInformationJobObject(hjob, JobObjectNotificationLimitInformation, &jnli, sizeof(jnli)), GetLastError());

			



			return hr;
		}

		template <class _ARG>
		HRESULT init(int pid,_ARG& argv)
		{
			if (FAILED(hr)) 
				return hr;
			

			int max_pcount = manager->max_pcount;

			processor_list_t& pm = manager->processors;

			int pm_count = pm.size();

			jobmap_t& jobmap = manager->jobmap;
			int64_t& memory_avail = manager->memory_avail;
			//usertime, fulltime, logtime;
			
			OLE_CHECK_hr_cond((fulltime = argv[L"quota.set.time"].def<double>(0))>0, E_INVALIDARG);
			OLE_CHECK_hr_cond((usertime = argv[L"quota.set.usertime"].def<double>(fulltime))>0, E_INVALIDARG);
			OLE_CHECK_hr_cond((logtime = argv[L"quota.set.logtime"].def<double>(3000))>0, E_INVALIDARG);

			memlimit = argv[L"quota.set.memory"].def<double>(0);
			OLE_CHECK_hr_cond((0<memlimit) && (memlimit < memory_avail), E_INVALIDARG);

			pcount = argv[L"quota.set.pcount"].def<double>(max_pcount);

			OLE_CHECK_hr_cond((0<pcount)&&(pcount<=pm_count), E_INVALIDARG);

			//int pid = argv[L"pid"].def<int>(0);
			process_id = pid;
			hkobject_t hp (OpenProcess(PROCESS_ALL_ACCESS, 0, pid),false);

			OLE_CHECK_hr_cond((HANDLE)hp, E_INVALIDARG);
			INT_PTR ih = argv[L"quota.set.handle"].def<double>(0);
			if (ih) {
				hsignal.reset((HANDLE)ih, hp);
					OLE_CHECK_hr_cond((HANDLE)hsignal, E_INVALIDARG);
			}
			else hsignal.hko = hp.dettach();

			
			//hsignal.attach()
			
			fit = 0;
			auto ib = processors.begin();
			auto jb = pm.begin();
			auto je = std::next(jb, pcount);

			processors.splice(ib, pm, jb, je);

			memory_avail -= memlimit;
			fit |= 2;

			OLE_CHECK_hr(set_job_limits());
			OLE_CHECK_hr(register_kobject(fulltime));

			//for (int k = 0;k<pcount)
				//processors.push_back()


			

			return hr;
		}

		

		template <class _ARG>
		job_t(int pid, _ARG& argv, manager_t* pm, std::wstring n = uniquie_name()) :manager(pm), name(n), hr(E_POINTER), hasyn(0), fulltime_viol(0), jlvi(),fit(0){

			//if (manager)
			{
				locker_t lock(manager->mutex);
				//it = manager->jobmap.end();

				HANDLE h = ::CreateJobObjectW(0, name.c_str());
				OLE_CHECK_VOID_hr_cond(h, GetLastError());
				//hr = S_OK;
				hjob.attach(h);

				key = manager->next_key();
				OLE_CHECK_VOID_hr(init(pid,argv));

				auto ip = manager->jobmap.insert(std::make_pair(key, this));
				it = ip.first;
				fit |= 1;
			}
		};

		~job_t()
		{ 
			locker_t lock(manager->mutex);
			terminate(); 

			if(fit&1) 
				manager->jobmap.erase(it);
		}

		void quota_restore(){
			//locker_t lock(manager->mutex);
			if (fit & 2)
			{
				int64_t& memory_avail = manager->memory_avail;
				memory_avail += memlimit;
				processor_list_t& pm = manager->processors;
				auto je = pm.end();
				pm.splice(je, processors);
				fit &= ~uint32_t(2);
			}
		}


		void terminate(){
			BOOL f;
			locker_t lock(manager->mutex);
			quota_restore();
			if (SUCCEEDED(hr))
			{
				hr = E_ABORT;
				
				
				//if (it != manager->jobmap.end())
					//  manager->jobmap.erase(it);
					  TerminateJobObject(hjob, -1);
					  JOBOBJECT_BASIC_LIMIT_INFORMATION j = {};
					  j.LimitFlags = JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
					  j.ActiveProcessLimit = 0;
					  f = ::SetInformationJobObject(hjob, JobObjectBasicLimitInformation, &j, sizeof(j));

					  hasyn_address();

			}
			
		}

		

		bool fulltime_viol;

		manager_t* manager;
		string_t  name;
		jobiterator_t it;
		processor_list_t processors;
		
		hkobject_t hjob,hsignal;
		HANDLE hasyn;
		HRESULT hr;
		double usertime, fulltime, logtime;
		int64_t memlimit;
		int pcount;
		int process_id;
		uint32_t fit;
		JOBOBJECT_LIMIT_VIOLATION_INFORMATION jlvi;
		uint32_t key;
	};

	

	





	static	int affinity2list(DWORD_PTR a, processor_list_t& pl)
	{
		pl.clear();
		DWORD_PTR i = 1;
		//for (int n = 0; n < sizeof(a); n++ )
		int k = 0;
		do
		{
			if (i&a)
				pl.push_back(k++);

		} while ((i <<= 1));

		return k;
	};

	static	ULONG_PTR list2affinity(processor_list_t& pl)
	{
		ULONG_PTR a = 0;
		for (auto i = pl.begin(); i != pl.end(); ++i)
		{
			a |= ULONG_PTR(1) << (*i);
		}
		return a;
	}

	uint32_t next_key(){

		return (__key+=2);
	}


	bool set_global_object()
	{
		BOOL f;
		HANDLE h= gjob.hko = CreateJobObjectW(0, global_job_name());
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION j = {};
		JOBOBJECT_BASIC_LIMIT_INFORMATION& jbli = j.BasicLimitInformation;
		jbli.Affinity = list2affinity(processors);
		jbli.LimitFlags = JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION | JOB_OBJECT_LIMIT_BREAKAWAY_OK | JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		f = ::SetInformationJobObject(h, JobObjectExtendedLimitInformation, &j, sizeof(j));
		if (f) f = AssignProcessToJobObject(h, GetCurrentProcess());
		return f;
	}


	job_manager_t() :hiocp(0), __key(1){

		HRESULT& hr = hrg;
		DWORD_PTR pa, sa;
		BOOL f;		
		HANDLE hp=GetCurrentProcess();
		f = set_global_object();
	

		
		OLE_CHECK_VOID_hr_cond(f = ::GetPerformanceInfo(&pi, sizeof(pi)), GetLastError());
		OLE_CHECK_VOID_hr_cond(f = ::GetProcessAffinityMask(hp,&pa, &sa), GetLastError());
		OLE_CHECK_VOID_hr_cond(hiocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0), GetLastError());

		
		max_pcount=affinity2list(pa, processors);
		memory_avail =  pi.PhysicalAvailable*pi.PageSize;
		

	};

	~job_manager_t(){

		if (hiocp)
		{
			locker_t lock(mutex);
			CloseHandle(hiocp);
		}

	}

	template<class ARG_IN, class ARG_OUT>
	HRESULT job_create(ARG_IN& arg_in, ARG_OUT& arg_out, int pid){
		HRESULT hr;
		
		arg_out.clear();
		
		job_t* pjob = new job_t(pid,arg_in, this);
		hr = pjob->hr;
		if (SUCCEEDED(hr))
		 {
			arg_out[L"job.name"] = pjob->name.c_str();
			arg_out[L"job.key"] = pjob->key;

		 }

		 else
		{
			arg_out[L"error"] = int(hr);
			delete pjob;
		}

		return hr;
	}

	template<class ARG_IN, class ARG_OUT>
	HRESULT call(ARG_IN& arg_in,ARG_OUT& arg_out,int pid){
		
		HRESULT hr=E_INVALIDARG;

		

		s_parser_t<wchar_t>::case_t<false> cs((wchar_t*)arg_in[L"cmd"]);
		
		if (cs(L"quota.set"))
			return hr = job_create(arg_in, arg_out,pid);

		 

		return hr;
	}

	int   on_send_recv( int pid, char* bufin, int cbr, char* bufout, int& cbw)
	{
		v_buf<char> vin(bufin, bufin + cbr);
		char_mutator<CP_UTF8, true> cm_in(vin.get());
		argv_zz<wchar_t>  arg_in((wchar_t*)cm_in);
		argv_zzs<wchar_t> arg_out;		
		call(arg_in, arg_out, pid);
		char_mutator<CP_UTF8, true> cm_out(arg_out.flat_str());
		cbw = cm_out.char_count(true);
		 memcpy(bufout, (char*)cm_out, cbw);
			return 0;
	}


static	int cdecl  s_on_send_recv(void* pcontext, unsigned long pid, char* bufin, int cbr, char* bufout, int* pcbw, HANDLE hAbortEvent)
	{
		return static_cast<job_manager_t*>(pcontext)->on_send_recv(pid, bufin, cbr, bufout, *pcbw);
	}

	HRESULT loop_once(DWORD tio=INFINITE)
	{
		HRESULT hr = hrg;
		BOOL f;
		DWORD msg_id;
		DWORD_PTR key;
		OVERLAPPED* povl;

		if (SUCCEEDED(hr))
		{
			if (f = GetQueuedCompletionStatus(hiocp, &msg_id, &key, &povl, tio))
			{
				locker_t lock(mutex);
				auto i = jobmap.find(key);
				if (i != jobmap.end())
				{
					//job_t* pj = (*i).second;

					(*i).second->on_complete_port(msg_id, povl ? DWORD(povl) : 0);
				}
				//job_t::s_on_complete_port(key, cb, povl);

				//static_cast<job_t*>((void*)key)->on_complete_port(msg_id, ovl ? DWORD(ovl) : 0);

			}
			else return HRESULT_FROM_WIN32(GetLastError());
			
		}
		return hr;
	}

	void loop_forever(){
		for (;;)
			loop_once();
	}

	typedef void(*log_proc_t)(wchar_t*);
	static log_proc_t& log_proc(){
		static  log_proc_t lp;
		return lp;
	}

	
	static const wchar_t* global_job_name()
	{
		return L"{4447EBC4-3E3F-4E56-BA80-8DF2ED34E231}-gobal-job";
	}

	/*
	job_t* operator [](const wchar_t* n)
	{

		return (n)?jobmap[n]:0;
	}
	*/
	HRESULT hrg;
	PERFORMANCE_INFORMATION pi;
	HANDLE hiocp;	
	hkobject_t gjob;
	mutex_t mutex;
	processor_list_t processors;
	jobmap_t jobmap;
	int max_pcount;
	int64_t memory_avail;
	uint32_t __key;
	
};