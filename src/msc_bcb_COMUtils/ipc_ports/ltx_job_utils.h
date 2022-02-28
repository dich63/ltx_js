#pragma once
// #include "ltx_job_utils.h"
#include "buf_fm.h"
#include "iocp_utils.h"
#include "LogicalProcessorInformation.h"
#include "wchar_parsers.h"
using namespace ipc_utils;
using namespace ipc_utils::com_apartments;
using namespace ipc_utils::com_apartments::script_engine;
using namespace ltx_helper;




template <int _VERSION=0>
struct ltx_job_utils_t
{

	enum{
		nano100=10000000,
		my_JobObjectNotificationLimitInformation=12,
		my_JobObjectLimitViolationInformation=13
	};


#pragma push(pack)
#pragma pack(1)

	typedef struct my_PERFORMANCE_INFORMATION {
		DWORD cb,rr;
		SIZE_T CommitTotal;
		SIZE_T CommitLimit;
		SIZE_T CommitPeak;
		SIZE_T PhysicalTotal;
		SIZE_T PhysicalAvailable;
		SIZE_T SystemCache;
		SIZE_T KernelTotal;
		SIZE_T KernelPaged;
		SIZE_T KernelNonpaged;
		SIZE_T PageSize;
		DWORD HandleCount;
		DWORD ProcessCount;
		DWORD ThreadCount,rr2;
	};



	typedef struct my_JOBOBJECT_LIMIT_VIOLATION_INFORMATION {
		DWORD LimitFlags;
		DWORD ViolationLimitFlags;
		DWORD64 IoReadBytes;
		DWORD64 IoReadBytesLimit;
		DWORD64 IoWriteBytes;
		DWORD64 IoWriteBytesLimit;
		LARGE_INTEGER PerJobUserTime;
		LARGE_INTEGER PerJobUserTimeLimit;
		DWORD64 JobMemory;
		DWORD64 JobMemoryLimit;
		DWORD RateControlTolerance,RateControlToleranceLimit;		
	};

	struct my_JOBOBJECT_NOTIFICATION_LIMIT_INFORMATION {

		typedef enum _JOBOBJECT_RATE_CONTROL_TOLERANCE {
			ToleranceLow = 1,
			ToleranceMedium,
			ToleranceHigh
		} JOBOBJECT_RATE_CONTROL_TOLERANCE;

		typedef enum _JOBOBJECT_RATE_CONTROL_TOLERANCE_INTERVAL {
			ToleranceIntervalShort = 1,
			ToleranceIntervalMedium,
			ToleranceIntervalLong
		} JOBOBJECT_RATE_CONTROL_TOLERANCE_INTERVAL;
		DWORD64 IoReadBytesLimit;
		DWORD64 IoWriteBytesLimit;
		LARGE_INTEGER PerJobUserTimeLimit;
		DWORD64 JobMemoryLimit;
		JOBOBJECT_RATE_CONTROL_TOLERANCE RateControlTolerance;
		JOBOBJECT_RATE_CONTROL_TOLERANCE_INTERVAL RateControlToleranceInterval;
		DWORD LimitFlags,xx;
	} ;

   #pragma pop(pack)

	typedef arguments_t<variant_t> arguments_t;

	typedef i_mm_region_ptr::int64_t int64_t;

	typedef ltx_helper::mbv_map_reader_t<variant_t>  mbv_map_reader_t;
	typedef ltx_helper::mbv_map_writer_t<variant_t>  mbv_map_writer_t;


	template <class N,bool FCOPY=false>
	struct  type_buffer_t
	{
		typedef std::map<const char*,void*> ptrmap_t;


		type_buffer_t():count(0),p(0),ph(0){};

		~type_buffer_t(){		          
			::free(ph);
		}


		template<class O>
		N* link(size_t _count,O* po,bool fcopy=FCOPY){

			count=_count;
			if((!fcopy)&&(typeid(O).name()==typeid(N).name())){
				p=(N*)po;
				if(ph){
					free(ph);
					ph=0;
				}
			}
			else {
				p=ph=(N*)::realloc(ph,(count)*sizeof(N));
				for(size_t k=0;k<count;k++)
					p[k]=N(po[k]);	   		   		   
			}

			return p;		

		};

		N* link_vt(VARTYPE vt,size_t _count,void* po,bool fcopy=FCOPY){

			switch (vt)
			{
			case VT_R8:
				return link( _count,(double*)po,fcopy);
				break;
			case VT_R4:
				return link( _count,(float*)po,fcopy);
				break;
			case VT_I1:
				return link( _count,(signed char*)po,fcopy);
				break;
			case VT_UI1:
				return link( _count,(unsigned char*)po,fcopy);
				break;
			case VT_I2:
				return link( _count,(signed short*)po,fcopy);
				break;
			case VT_UI2:
				return link( _count,(unsigned short*)po,fcopy);
				break;
			case VT_BOOL:
				return link( _count,(VARIANT_BOOL*)po,fcopy);
				break;
			case VT_I4:
				return link( _count,(signed int*)po,fcopy);
				break;
			case VT_UI4:
				return link( _count,(unsigned int*)po,fcopy);
				break;
			case VT_I8:
				return link( _count,(LONGLONG*)po,fcopy);
				break;
			case VT_UI8:
				return link( _count,(ULONGLONG*)po,fcopy);
				break;

			}
			return 0;
		}


		template<class O>
		O* copy_to(O* po)
		{
			for(size_t k=0;k<count;k++)
				po[k]=O(p[k]);

			return po;

		}


		bool linked(){ 
			return !ph;
		};    

		operator N*() {	
			return p;
		}
		N* p,*ph;
		char* name;
		size_t count;

	};








	static void __cdecl  Terminate
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{


		BOOL ff;

		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;

		ipc_utils::smart_ptr_t<i_mbv_kernel_object> ko;
		kernel_object_t* pko;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(ko._address()));
		OLE_CHECK_VOID_hr(ko->GetContext((void**)&(pko=0)));
		HANDLE hjob=pko->hko;
		DWORD ec=arguments[1].def<int>(-1);
		ff=::TerminateJobObject(hjob,ec);
		OLE_CHECK_VOID_hr_cond(ff,GetLastError());
	}


	static void __cdecl  GetAffinity
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{
		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;
		BOOL ff;
		DWORD_PTR pa,sa;
		double d;
		ff=GetProcessAffinityMask(GetCurrentProcess(),&pa,&sa);
		OLE_CHECK_VOID_hr_cond(ff,GetLastError());
		DWORD_PTR a=(arguments[0].def<int>(0))?pa:sa;
		if(arguments.length()>1)
		{
          int lh=arguments[1].def<int>(0);
		   unsigned int r;
		  if(lh) r=(unsigned int)(a>>32);
		  else r=(unsigned int)(a>>0);
		  result=double(r);
		}
		else{
		  d=a;
		  result=d;
		}

	}

	struct hh_t{
		HANDLE h;
		hh_t(HANDLE _h=0):h(0){};
		~hh_t(){ ipc_utils::safe_close_handle(h); };
		operator HANDLE(){ return h;}
	};

	static void __cdecl  AssignProcess
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{
		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;
		BOOL ff;


		HANDLE hjob,hprocess;

		hh_t hhjob;

		ipc_utils::smart_ptr_t<i_mbv_kernel_object> ko;
		kernel_object_t* pko;
		

		if(SUCCEEDED(arguments[0].QueryInterface(ko._address())))
		{
			OLE_CHECK_VOID_hr(ko->GetContext((void**)&(pko=0)));
			hjob=pko->hko;
		}
		else
		{
			bstr_t name=arguments[0].def<bstr_t>();

			OLE_CHECK_VOID_hr_cond(name.length(),E_INVALIDARG);
			hjob=hhjob.h=OpenJobObjectW(JOB_OBJECT_ASSIGN_PROCESS,0,name);

			OLE_CHECK_VOID_hr_cond(hjob,GetLastError());
		}


		int pid=arguments[1].def<int>(0);
		if(pid==-1) 
			pid=GetCurrentProcessId();
/*
		struct hh_t{
			HANDLE h;
			~hh_t(){ ipc_utils::safe_close_handle(h); };
		} hh={OpenProcess(PROCESS_SET_QUOTA|PROCESS_TERMINATE,false,pid)};
		*/

         hh_t hh=hprocess=OpenProcess(PROCESS_SET_QUOTA|PROCESS_TERMINATE,false,pid);
		OLE_CHECK_VOID_hr_cond(hprocess,GetLastError());						
		ff=AssignProcessToJobObject(hjob,hprocess);  
		OLE_CHECK_VOID_hr_cond(ff,GetLastError());		


	}

	static void __cdecl  NotificationLimitViolation
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{
		BOOL ff;
		DWORD len;


		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;

		ipc_utils::smart_ptr_t<i_mbv_kernel_object> ko;
		kernel_object_t* pko;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(ko._address()));
		OLE_CHECK_VOID_hr(ko->GetContext((void**)&(pko=0)));




		HANDLE hjob=pko->hko;
		my_JOBOBJECT_NOTIFICATION_LIMIT_INFORMATION jn={};		
		DWORD& LimitFlags=jn.LimitFlags;



		if(ff=!arguments[1].is_empty())
		{
			mbv_map_reader_t mr((VARIANT)arguments[1]);

			OLE_CHECK_VOID_hr(mr);
			if((jn.JobMemoryLimit=mr[variant_t(L"memory")].def<double>(0))>0)
				LimitFlags|=JOB_OBJECT_LIMIT_JOB_MEMORY;
			if((jn.PerJobUserTimeLimit.QuadPart=mr[variant_t(L"time")].def<double>(0))>0)
				LimitFlags|=JOB_OBJECT_LIMIT_JOB_TIME;
			if(LimitFlags)
			{
				ff=SetInformationJobObject(hjob,JOBOBJECTINFOCLASS(12),&jn,sizeof(jn));
				OLE_CHECK_VOID_hr_cond(ff,GetLastError());
			}				

		}
		mbv_map_writer_t mw;
		OLE_CHECK_VOID_hr(mw);
 #define SET_VAL_MAP(a) OLE_CHECK_VOID_hr(mw.set_value(L###a,double(jn.a)))

        jn=my_JOBOBJECT_NOTIFICATION_LIMIT_INFORMATION(); 

		ff=QueryInformationJobObject(hjob,JOBOBJECTINFOCLASS(12),&jn,sizeof(jn),&len);
		OLE_CHECK_VOID_hr_cond(ff,GetLastError());
		SET_VAL_MAP(LimitFlags);
		SET_VAL_MAP(PerJobUserTimeLimit.QuadPart);
		SET_VAL_MAP(JobMemoryLimit);


#undef SET_VAL_MAP		
		hr=mw.detach(&result);

	}

	static void __cdecl  QueryLimitViolation
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{



		BOOL ff;
		DWORD len;
		DWORD xxlen=sizeof(my_JOBOBJECT_NOTIFICATION_LIMIT_INFORMATION);


		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;

		ipc_utils::smart_ptr_t<i_mbv_kernel_object> ko;
		kernel_object_t* pko;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(ko._address()));
		OLE_CHECK_VOID_hr(ko->GetContext((void**)&(pko=0)));

		HANDLE hjob=pko->hko;

		mbv_map_writer_t mw;
		OLE_CHECK_VOID_hr(mw);



		my_JOBOBJECT_LIMIT_VIOLATION_INFORMATION j;

		int iii=JOBOBJECTINFOCLASS(13);

		j.LimitFlags=JOB_OBJECT_LIMIT_JOB_MEMORY|JOB_OBJECT_LIMIT_JOB_TIME;

		//ff=QueryInformationJobObject(hjob,JOBOBJECTINFOCLASS(my_JobObjectLimitViolationInformation),&j,sizeof(j),&len);
		ff=QueryInformationJobObject(hjob,JOBOBJECTINFOCLASS(13),&j,sizeof(j),&len);
		OLE_CHECK_VOID_hr_cond(ff,GetLastError());
		//bi.ThisPeriodTotalKernelTime.QuadPart

#undef SET_VAL_MAP
#define SET_VAL_MAP(a) OLE_CHECK_VOID_hr(mw.set_value(L###a,double(j.a)))

		SET_VAL_MAP(LimitFlags);
		SET_VAL_MAP(ViolationLimitFlags);
		SET_VAL_MAP(IoReadBytes);
		SET_VAL_MAP(IoReadBytesLimit);
		SET_VAL_MAP(IoWriteBytes);
		SET_VAL_MAP(IoWriteBytesLimit);
		SET_VAL_MAP(JobMemory);
		SET_VAL_MAP(JobMemoryLimit);
		SET_VAL_MAP(RateControlTolerance);
		SET_VAL_MAP(RateControlToleranceLimit);


#undef SET_VAL_MAP
#define SET_VAL_MAP(a) OLE_CHECK_VOID_hr(mw.set_value(L###a,double(j.a.QuadPart)/double(nano100)))


		SET_VAL_MAP(PerJobUserTime);
		SET_VAL_MAP(PerJobUserTimeLimit);


#undef SET_VAL_MAP		
		hr=mw.detach(&result);

	}



	static void __cdecl  QueryAccountingInformation
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{



		BOOL ff;
		DWORD len;


		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;

		ipc_utils::smart_ptr_t<i_mbv_kernel_object> ko;
		kernel_object_t* pko;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(ko._address()));
		OLE_CHECK_VOID_hr(ko->GetContext((void**)&(pko=0)));

		HANDLE hjob=pko->hko;

		mbv_map_writer_t mw,mwic;
		OLE_CHECK_VOID_hr(mw);
		OLE_CHECK_VOID_hr(mwic);


		JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION j;
		JOBOBJECT_BASIC_ACCOUNTING_INFORMATION& bi=j.BasicInfo;
		IO_COUNTERS&                            ii=j.IoInfo;
		ff=QueryInformationJobObject(hjob,JobObjectBasicAndIoAccountingInformation,&j,sizeof(j),&len);
		OLE_CHECK_VOID_hr_cond(ff,GetLastError());
		//bi.ThisPeriodTotalKernelTime.QuadPart
#undef SET_VAL_MAP

#define SET_VAL_MAP(a) OLE_CHECK_VOID_hr(mw.set_value(L###a,double(bi.a.QuadPart)/double(nano100)))

		SET_VAL_MAP(TotalUserTime);
		SET_VAL_MAP(TotalKernelTime);
		SET_VAL_MAP(ThisPeriodTotalUserTime);
		SET_VAL_MAP(ThisPeriodTotalKernelTime);

#undef SET_VAL_MAP
#define SET_VAL_MAP(a) OLE_CHECK_VOID_hr(mw.set_value(L###a,double(bi.a)))

		SET_VAL_MAP(TotalPageFaultCount);
		SET_VAL_MAP(TotalProcesses);
		SET_VAL_MAP(ActiveProcesses);
		SET_VAL_MAP(TotalTerminatedProcesses);

#undef SET_VAL_MAP
#define SET_VAL_MAP(a) OLE_CHECK_VOID_hr(mwic.set_value(L###a,double(ii.a)))


		SET_VAL_MAP(ReadOperationCount);
		SET_VAL_MAP(WriteOperationCount);
		SET_VAL_MAP(OtherOperationCount);
		SET_VAL_MAP(ReadTransferCount);
		SET_VAL_MAP(WriteTransferCount);
		SET_VAL_MAP(OtherTransferCount);

#undef SET_VAL_MAP

		variant_t vc;
		OLE_CHECK_VOID_hr(mwic.detach(&vc));
		OLE_CHECK_VOID_hr(mw.set_value(L"IO",vc));
		hr=mw.detach(&result);

	}

	static void __cdecl  IsProcessInJob
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{




		BOOL ff,fr;
		DWORD len;


		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;



		int pid=arguments[0].def<int>(GetCurrentProcessId());

		struct process_hh_t{
			HANDLE hp;
			~process_hh_t(){ if(hp) CloseHandle(hp);}

		} phh={OpenProcess(PROCESS_QUERY_INFORMATION,0,pid)} ;

		OLE_CHECK_VOID_hr_cond(phh.hp,GetLastError());


		ipc_utils::smart_ptr_t<i_mbv_kernel_object> ko;

		HANDLE hp=phh.hp,hjob=0;		


		if(SUCCEEDED(arguments[1].QueryInterface(ko._address())))
		{		
			kernel_object_t* pko;		
			OLE_CHECK_VOID_hr(ko->GetContext((void**)&(pko=0)));
			hjob=pko->hko;
		}

		ff=::IsProcessInJob(hp,hjob,&fr);
		OLE_CHECK_VOID_hr_cond(ff,GetLastError());
		result=bool(fr);

	}

	static void __cdecl  SetInformation
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{

#define set_def_value(k) update(L###k,k);


		BOOL ff;
		DWORD len;


		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;

		ipc_utils::smart_ptr_t<i_mbv_kernel_object> ko;
		kernel_object_t* pko;
		OLE_CHECK_VOID_hr(arguments[0].QueryInterface(ko._address()));
		OLE_CHECK_VOID_hr(ko->GetContext((void**)&(pko=0)));




		HANDLE hjob=pko->hko;
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION jel={};
		JOBOBJECT_BASIC_LIMIT_INFORMATION& jbl=jel.BasicLimitInformation; 
		DWORD& LimitFlags=jbl.LimitFlags;



		if(ff=!arguments[1].is_empty())
		{
			mbv_map_reader_t mr((VARIANT)arguments[1]);

			OLE_CHECK_VOID_hr(mr);



			DWORD_PTR aff=0;
			bool faffinity=false;



			ff=QueryInformationJobObject(hjob,JobObjectExtendedLimitInformation,&jel,sizeof(jel),&len);
			OLE_CHECK_VOID_hr_cond(ff,GetLastError());



			if(mr.is(L"processors"))
			{
				ipc_utils::smart_ptr_t<i_mm_region_ptr> rg;    
				OLE_CHECK_VOID_hr(mr[variant_t(L"processors")].QueryInterface(rg._address()));
				VARTYPE vt;
				i_mm_region_ptr::int64_t Count;
				long ElementSize;
				//GetElementInfo(VARTYPE* pvt,int64_t* pCount=0,long* pElementSize=0)=0;
				OLE_CHECK_VOID_hr(rg->GetElementInfo(&vt,&Count,&ElementSize));
				i_mm_region_ptr::ptr_t<char> prg(rg,-1);
				OLE_CHECK_VOID_hr(prg);
				type_buffer_t<UINT32> npbuf;
				UINT32* np=npbuf.link_vt(vt,Count,(char*)prg);
				aff=0;
				for(int n=0;n<Count;n++)
					aff+=(1<<np[n]);
				faffinity=true;

			}
			else if(mr.is(L"affinity"))
			{

				aff=mr[variant_t(L"affinity")].def<double>(0);
				faffinity=true;
			}





			if(faffinity)
			{
				DWORD_PTR pa,sa;		
				ff=GetProcessAffinityMask(GetCurrentProcess(),&pa,&sa);
				OLE_CHECK_VOID_hr_cond(ff,GetLastError());			

				LimitFlags|=JOB_OBJECT_LIMIT_AFFINITY;           
				jbl.Affinity=aff&sa;

			}

			double process_memory=0,job_memory=0,memory=0;
			double job_time=-1,process_time=-1,time=-1;	
			double minws=0,maxws=0;

			mr.set_def_value(process_memory);
			mr.set_def_value(job_memory);

			mr.set_def_value(process_time);
			mr.set_def_value(job_time);

			mr.set_def_value(memory);
			mr.set_def_value(time);

			mr.set_def_value(minws);
			mr.set_def_value(maxws);

			if(memory>0) job_memory=memory;
			if(time>=0) job_time=time;



			/*
			process_memory=mr[L"process_memory"].def<double>(0);
			job_memory=mr[L"job_memory"].def<double>(0);

			job_time=mr[L"job_time"].def<double>(-1);
			process_time=mr[L"process_time"].def<double>(-1);
			*/


			if(process_memory>0)
			{
				LimitFlags|=JOB_OBJECT_LIMIT_PROCESS_MEMORY;
				jel.ProcessMemoryLimit=process_memory;
			}

			if(job_memory>0)
			{
				LimitFlags|=JOB_OBJECT_LIMIT_JOB_MEMORY;
				jel.JobMemoryLimit=job_memory;
			}

			if(job_time>=0)
			{
				LimitFlags|=JOB_OBJECT_LIMIT_JOB_TIME;
				jbl.PerJobUserTimeLimit.QuadPart=job_time*nano100;
			}

			if(process_time>=0)
			{
				LimitFlags|=JOB_OBJECT_LIMIT_PROCESS_TIME;
				jbl.PerProcessUserTimeLimit.QuadPart=process_time*nano100;
			}

			if(minws<=maxws)
			{



				if(minws>0)
				{
					LimitFlags|=JOB_OBJECT_LIMIT_WORKINGSET;
					jbl.MinimumWorkingSetSize=minws;
				}
				if(maxws>0)
				{
					LimitFlags|=JOB_OBJECT_LIMIT_WORKINGSET;
					jbl.MaximumWorkingSetSize=maxws;
				}


			}


			//JOBOBJECT_LIMIT_VIOLATION_INFORMATION

			ff=SetInformationJobObject(hjob,JobObjectExtendedLimitInformation,&jel,sizeof(jel));       


			//DWORD ec=arguments[1].def<int>(-1);
			//ff=::TerminateJobObject(hjob,ec);
			OLE_CHECK_VOID_hr_cond(ff,GetLastError());
		}


		jel=JOBOBJECT_EXTENDED_LIMIT_INFORMATION();

		mbv_map_writer_t mw;
		OLE_CHECK_VOID_hr(mw);


		ff=QueryInformationJobObject(hjob,JobObjectExtendedLimitInformation,&jel,sizeof(jel),&len);
		OLE_CHECK_VOID_hr_cond(ff,GetLastError());
		//bi.ThisPeriodTotalKernelTime.QuadPart
#undef SET_VAL_MAP
#define SET_VAL_MAP(a) OLE_CHECK_VOID_hr(mw.set_value(L###a,double(jbl.a.QuadPart)/double(nano100)))


		SET_VAL_MAP(PerProcessUserTimeLimit);
		SET_VAL_MAP(PerJobUserTimeLimit);

#undef SET_VAL_MAP
#define SET_VAL_MAP(a) OLE_CHECK_VOID_hr(mw.set_value(L###a,double(jbl.a)))


		SET_VAL_MAP(LimitFlags);
		SET_VAL_MAP(MinimumWorkingSetSize);
		SET_VAL_MAP(MaximumWorkingSetSize);
		SET_VAL_MAP(ActiveProcessLimit);
		SET_VAL_MAP(PriorityClass);
		SET_VAL_MAP(SchedulingClass);

		SET_VAL_MAP(Affinity);

		UINT64 a;
		a= UINT64(0xFFFFFFFF)&jbl.Affinity;
		OLE_CHECK_VOID_hr(mw.set_value(L"Affinity_l",double(a)));
		a= UINT64(0xFFFFFFFF)&(jbl.Affinity>>32);
		OLE_CHECK_VOID_hr(mw.set_value(L"Affinity_h",double(a)));

#undef SET_VAL_MAP
#define SET_VAL_MAP(a) OLE_CHECK_VOID_hr(mw.set_value(L###a,double(jel.a)))


		SET_VAL_MAP(ProcessMemoryLimit);
		SET_VAL_MAP(JobMemoryLimit);
		SET_VAL_MAP(PeakProcessMemoryUsed);
		SET_VAL_MAP(PeakJobMemoryUsed);



#undef SET_VAL_MAP

		hr=mw.detach(&result);




	};

	static void __cdecl  GetPerformanceInfo
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{



		BOOL ff;
		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;

		my_PERFORMANCE_INFORMATION pi={sizeof(my_PERFORMANCE_INFORMATION)};


		ff=::GetPerformanceInfo((PERFORMANCE_INFORMATION*)&pi,sizeof(pi)); 
		OLE_CHECK_VOID_hr_cond(ff,GetLastError());		

		mbv_map_writer_t mw;
		OLE_CHECK_VOID_hr(mw);
#undef SET_VAL_MAP
#define SET_VAL_MAP(a) OLE_CHECK_VOID_hr(mw.set_value(L###a,double(pi.a)))

		SET_VAL_MAP(CommitTotal);
		SET_VAL_MAP(CommitLimit);
		SET_VAL_MAP(CommitPeak);
		SET_VAL_MAP(PhysicalTotal);
		SET_VAL_MAP(PhysicalAvailable);
		SET_VAL_MAP(SystemCache);
		SET_VAL_MAP(KernelTotal);
		SET_VAL_MAP(KernelPaged);
		SET_VAL_MAP(KernelNonpaged);
		SET_VAL_MAP(PageSize);
		SET_VAL_MAP(HandleCount);
		SET_VAL_MAP(ProcessCount);
		SET_VAL_MAP(ThreadCount);



#undef SET_VAL_MAP
		hr=mw.detach(&result);

	};


	inline static double FT2double(FILETIME& ft)
	{
		return *((int64_t*)(&ft));
	}


	static void __cdecl  GetSystemTimes
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{



		BOOL ff;
		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;


		FILETIME idle,kernel,user;



		ff=::GetSystemTimes(&idle,&kernel,&user); 
		OLE_CHECK_VOID_hr_cond(ff,GetLastError());		

		mbv_map_writer_t mw;
		OLE_CHECK_VOID_hr(mw);

#undef SET_VAL_MAP
#define SET_VAL_MAP(a) OLE_CHECK_VOID_hr(mw.set_value(L###a,FT2double(a)/double(nano100) ) ) 

		SET_VAL_MAP(idle);
		SET_VAL_MAP(kernel);
		SET_VAL_MAP(user);



#undef SET_VAL_MAP
		hr=mw.detach(&result);

	};







	static void __cdecl  memtest
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{

		BOOL ff;
		DWORD len;


		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;
		void* p=0;

		hr=E_INVALIDARG;
		int mode=arguments[1].def<int>(1);
		INT64 sz=arguments[0].def<double>(0);


		switch(mode)
		{
		case 0 :
			{
				std::vector<char> v(sz);
				OLE_CHECK_VOID_hr_cond(&v[0],E_OUTOFMEMORY);hr=0;
			}
			break;
		case 1 :
			OLE_CHECK_VOID_hr_cond(p=malloc(sz),E_OUTOFMEMORY);hr=0;
			break;
		case 2 :
			OLE_CHECK_VOID_hr_cond(p=HeapAlloc(GetProcessHeap(),0,sz),E_OUTOFMEMORY);hr=0;
			break;
		case 3 :
			OLE_CHECK_VOID_hr_cond(p=VirtualAlloc(0,sz,MEM_COMMIT,PAGE_READWRITE),GetLastError());hr=0;
			break;
		case 4 :
			buf_fm_T<char>* b=new buf_fm_T<char>(0,sz);
			p=(*b);
			OLE_CHECK_VOID_hr_cond(p,E_OUTOFMEMORY);hr=0;

			for(char*i=(char*)p;i<((char*)p)+sz;i++) *i=11;

			break;


		};

		result=double(INT64(p));

	};


	static void __cdecl  iocptest
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{

		BOOL ff;
		DWORD len;
		HRESULT status;


		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;
		void* p=0;

		hr=E_INVALIDARG;
		ipc_utils::smart_ptr_t<i_mbv_kernel_object> iocp;
		ipc_utils::smart_ptr_t<i_mbv_socket_context> sock;

		kernel_object_t * piocp;
		shared_socket_data_t* psock;
		OLE_CHECK_VOID_hr( arguments[0].QueryInterface(iocp._address()));
		OLE_CHECK_VOID_hr( arguments[1].QueryInterface(sock._address()));

		OLE_CHECK_VOID_hr( iocp->GetContext((void**)&piocp) );
		OLE_CHECK_VOID_hr( sock->GetContext((void**)&psock) );        
		HANDLE h,hiocp=piocp->hko,hsock=(HANDLE)psock->sock;

		 status= iocp_utils::iocp_t<>::get_instance().clearIOCP(hsock);

		OLE_CHECK_VOID_hr_cond(h=::CreateIoCompletionPort(hsock,hiocp,111,0),GetLastError());
		result=!!status;
	}


	static void __cdecl  fatal
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{
		//CoUninitialize();
		//HeapFree((void**)0x1fff1,0,(void**)0x1fff1);
		GlobalFree(&result);
	}

	static void __cdecl  LogicalProcessorInformation
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{
		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;
		LogicalProcessorInformation_t  lpi;
		OLE_CHECK_VOID_hr_cond(lpi,E_NOTIMPL);

		bstr_t delim=arguments[0].def<bstr_t>();

		argv_zzs<wchar_t> args;
		args[L"logicalProcessorCount"]=lpi.logicalProcessorCount;
		args[L"numaNodeCount"]=lpi.numaNodeCount;
		args[L"processorCoreCount"]=lpi.processorCoreCount;
		args[L"processorL1CacheCount"]=lpi.processorL1CacheCount;
		args[L"processorL2CacheCount"]=lpi.processorL2CacheCount;
		args[L"processorL3CacheCount"]=lpi.processorL3CacheCount;
		args[L"processorPackageCount"]=lpi.processorPackageCount;
		int cb;
		const wchar_t* p=args.flat_str(false,delim,&(cb=0));
		BSTR b=SysAllocStringLen(0,cb);
		memcpy(b,p,2*cb);
		result.vt=VT_BSTR;
		result.bstrVal=b;
	}

	static void __cdecl is_hyperthreading
		( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t pcca)
	{
		arguments_t arguments(pcca);
		HRESULT &hr=arguments.hr;
		LogicalProcessorInformation_t  lpi;
		OLE_CHECK_VOID_hr_cond(lpi,E_NOTIMPL);	
		result=lpi.is_hyperthreading();
	}

};


PROC_LIST_BEGIN(ltx_job_list)
PROC_ITEM_STRUCT(ltx_job_utils_t<>,Terminate)
PROC_ITEM_STRUCT(ltx_job_utils_t<>,SetInformation)
PROC_ITEM_STRUCT(ltx_job_utils_t<>,GetAffinity)
PROC_ITEM_STRUCT(ltx_job_utils_t<>,AssignProcess)
PROC_ITEM_STRUCT(ltx_job_utils_t<>,QueryAccountingInformation) 
PROC_ITEM_STRUCT(ltx_job_utils_t<>,QueryLimitViolation) 
PROC_ITEM_STRUCT(ltx_job_utils_t<>,IsProcessInJob) 
PROC_ITEM_STRUCT(ltx_job_utils_t<>,GetSystemTimes) 
PROC_ITEM_STRUCT(ltx_job_utils_t<>,GetPerformanceInfo) 
PROC_ITEM_STRUCT(ltx_job_utils_t<>,NotificationLimitViolation) 
PROC_ITEM_STRUCT(ltx_job_utils_t<>,memtest) 
PROC_ITEM_STRUCT(ltx_job_utils_t<>,iocptest) 
PROC_ITEM_STRUCT(ltx_job_utils_t<>,fatal) 
PROC_ITEM_STRUCT(ltx_job_utils_t<>,LogicalProcessorInformation) 
PROC_ITEM_STRUCT(ltx_job_utils_t<>,is_hyperthreading) 
PROC_LIST_END;