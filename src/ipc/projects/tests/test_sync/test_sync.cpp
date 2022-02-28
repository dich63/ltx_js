// test_sync.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "windows.h"

#include "ipc_ports/ipc_utils_base.h"
#include "ipc_ports/hook_coregister.h"
#include "wchar_parsers.h"
#include "singleton_utils.h"
#include "tls_dll.h" 
#include "ipc_ports/file_map_utils.h"
#include "ipc_ports/sm_region.h"
#include "ipc_ports/sparse_file_map.h"
#include "dispid_find.h"
#include "ipc_ports/moniker_parsers.h"
#include "numeric/exp_pade_data.h"
//#include "ipc_ports/mbv_buffer2.h"
#include "ipc_ports/thread_pool_job.h"
volatile LONG ll=0;
HRESULT ghr;
typedef ipc_utils::stopwatch_t stopwatch_t;

bool check_thread_lifetime(DWORD tid )
{
	HANDLE h;
	if(h=OpenThread(THREAD_QUERY_INFORMATION,0,tid))
	{
		DWORD ec;
		if(!GetExitCodeThread(h,&ec))
			return false;
		CloseHandle(h); 
		return (ec==STILL_ACTIVE);
	}

	return false;
}
#define KB (1024)
#define MB (1024*1024)
#define SZZ (MB)
//double* p=new double[SZZ];
//
char* p=new char[SZZ];
//typedef   mutex_def_t mutex_my_t;
typedef   mutex_ko_t mutex_my_t;
mutex_my_t gmutex;
mutex_def_t komutex;


template < int TIODEF=8,int SPINLOCKDEF=1024,int THREAD_VALIDFATE_COUNT=16>
struct shared_robust_mutex_t
{
	struct shared_robust_data_t
	{ 
		long tid;
		long refcount;
	};

	shared_robust_mutex_t(void* _ptr,int _thread_validate_count=THREAD_VALIDFATE_COUNT,  int _tio=TIODEF, int _spinlockcount=SPINLOCKDEF):
ptr((shared_robust_data_t*)_ptr),tio(_tio),spinlockcount(_spinlockcount),thread_validate_count(_thread_validate_count)
,cid(GetCurrentThreadId())
{}

FORCEINLINE 	long compare_exchange(long value,long comparand )
{
	return _InterlockedCompareExchange((volatile LONG*)&ptr->tid,value,comparand);  
}
FORCEINLINE bool try_lock_low()
{
	long ctid=GetCurrentThreadId();
//	ctid=cid;
	long t; 
	//t=InterlockedExchangeAdd((volatile LONG*)&ptr->tid,0);
  t=compare_exchange(ctid,0);
	if(t==ctid)
	{
		ptr->refcount++;		
		return true;
	}
	

	if(t==0)
	{		
			ptr->refcount=1;
			return true;		
	}
	return false;
}

FORCEINLINE bool try_unlock_low(long& rc)
{
	long ctid,t;	
	
	//
	t=compare_exchange(0,0);	
	//t=InterlockedExchangeAdd((volatile LONG*)&ptr->tid,0);

//	
	ctid=GetCurrentThreadId();
	//	ctid=cid;

	if(t==ctid)
	{		
		rc=--(ptr->refcount);
		if(rc<=0){
			//ptr->refcount=0;
			compare_exchange(0,ctid);
		}

		return true;
	}

	return false;
}

FORCEINLINE  bool try_spin_lock()
{

	if(try_lock_low()) return true;
	for(int slc=0;slc<spinlockcount;slc++)
	{
		if((slc&0x7)==0) SwitchToThread();
		if (try_lock_low()) 
			return true;
	}
	return false;
}

inline bool try_lock()
{
	if(try_spin_lock()) return true;
	else {
	   Sleep(tio);
	   return try_spin_lock();
	}
	
}

FORCEINLINE  bool lock(){

    if(try_lock_low()) return true;

	if(thread_validate_count>0) 
	{

		for(;;)
		{       

			for(int kt=0;kt<thread_validate_count;kt++)
			{
				if(try_lock()) return true;

			}

			thread_reset();
		}

	}
	else 
		for(;;)	{
			if(try_lock())
				return true; 
		}

  return true;
}

FORCEINLINE  bool unlock()
{
	long rc;
	return try_unlock_low(rc);
}

inline long thread_reset()
{
	long tid=compare_exchange(0,0);
	if((tid)&&(!validate_thread(tid)))
		tid=compare_exchange(0,tid);	
	return tid;
}

inline static	bool validate_thread(DWORD tid,HRESULT* phr=0)
{
	HRESULT tt;
	HRESULT& hr=(phr)?*phr:tt;
	HANDLE h;
	if(h=OpenThread(THREAD_QUERY_INFORMATION,0,tid))
	{
		hr=S_OK;
		DWORD ec;
		if(!GetExitCodeThread(h,&ec))
			return false;
		CloseHandle(h); 
		return (ec==STILL_ACTIVE);
	}
	else hr=GetLastError();


	return (hr==E_ACCESSDENIED);
}


shared_robust_data_t* ptr;
int tio;
int spinlockcount;
int thread_validate_count;
int cid;

};


shared_robust_mutex_t<>::shared_robust_data_t srd={};
shared_robust_mutex_t<>::shared_robust_data_t srd0={2528,777};
shared_robust_mutex_t<> srmutex(&srd,16,8,512);
shared_robust_mutex_t<> srmutex0(&srd0,16);


struct ASUN_t
{
	void operator()()
	{
		locker_t<mutex_my_t> lock(gmutex);
		locker_t<shared_robust_mutex_t<>> lock2(srmutex);
		//Sleep(10000);
		//Sleep(-1);
		
	}
};

int testm(int w=1)
{ 

	{
		locker_t<shared_robust_mutex_t<>> lock(srmutex);
		{
			locker_t<shared_robust_mutex_t<>> lock2(srmutex);

		}
		

	}
	ASUN_t asy;
	asyn_call(&asy,1,0);
	stopwatch_t cs;
	double t;
	Sleep(1000);
	cs.Start();
	HANDLE hev=CreateEvent(0,1,1,0);
	
	for(int k=0;k<SZZ;k++){
		//
		//	   while(InterlockedCompareExchange(&ll,0,0));
		//
		//
		//		   InterlockedCompareExchange(&ll,0,0);
		//			locker_t<mutex_def_t> lock(komutex);
		//
		//		locker_t<mutex_my_t> lock(gmutex);
		//	
		//
		//
		//
		locker_t<shared_robust_mutex_t<>> lock(srmutex);
		
		//		PulseEvent(hev);
				//SetEvent(hev);
		//tls_var<11>()=k;
		p[k]=k;//tls_var<11>();
	}

	t=cs.Sec();

	fwprintf(stdout,L"%g  MB/sec =%g\n",t,double(SZZ)/double(MB)/t);
	if(w) getchar();
	return 0; 
}
volatile long gll;
template <class A>
int testtxt(A& args)
{ 
  gll=0;
  stopwatch_t cs;
  double t;
  Sleep(1000);
  cs.Start();

	for(int k=0;k<SZZ;k++){
		//
		//	   while(InterlockedCompareExchange(&ll,0,0));
		//
		//
		//		   InterlockedCompareExchange(&ll,0,0);
		//			locker_t<mutex_def_t> lock(komutex);
		//
		//		locker_t<mutex_my_t> lock(gmutex);
		//	
		//
		//
		//locker_t<shared_robust_mutex_t<>> lock(srmutex);

		//		PulseEvent(hev);
		//SetEvent(hev);
		//tls_var<11>()=k;
		gll+=args[L"region.handle"].def<double>(11);//tls_var<11>();
	}

	t=cs.Sec();

	fwprintf(stdout,L"%g  MB/sec =%g tic/sec =%d\n",t,double(SZZ)/double(MB)/t,int(double(SZZ)/t));
	 getchar();
	return gll; 
}


static		void __stdcall s_wtc_proc( void* kp,BOOLEAN twf)
{

}

void testfm(HANDLE hm,ULONGLONG siz,HANDLE hm2)
{
	//file_map_utils::region_aligner_t ra; 
	//int y=ra.sz_item;

  ULONGLONG siz2;
  siz2= sm_region::mm_copy<20>(hm,0,siz);
  ghr=GetLastError();

	typedef double value_t;
	double t;
	 value_t *pf,*pfb;
	const ULONGLONG si=sizeof( value_t);
	ULONGLONG c=siz/si;
	mutex_ko_t mutex;

	sm_region::region_cache_t<2> rc(hm);

	stopwatch_t cs;
	cs.Start();
	ULONGLONG step=1;//2*(4*1024)/sizeof(value_t);


	for (ULONGLONG k=0;k<c;k+=step)
	{
		//		locker_t<mutex_ko_t> lock(mutex);
			//locker_t<shared_robust_mutex_t<>> lock2(srmutex);
		pf=rc.recommit_ptr< value_t>(si,k*si); 
		pf[0]=k;
		pfb=rc.recommit_ptr< value_t>(si,(c-1-k)*si); 
		pfb[0]=-k;
		

	}


	;
     
   

   t=cs.Sec();

   fwprintf(stdout,L"mmm:: %g  MB/sec =%g tic/sec =%d\n",t,double(siz)/double(MB)/t,int(double(c)/t));

   fwprintf(stdout,L"mm_copy ...\n");
   getchar();
double de;
   cs.Start();
    siz2= sm_region::mm_copy<20>(hm,hm2,siz);
   t=cs.Sec();
   de=siz-siz2;

   

   fwprintf(stdout,L"mmm:: %g %g  MB/sec =%g tic/sec =%d\n",t,de,double(siz)/double(MB)/t,int(double(c)/t));

   CloseHandle(hm2);
   fwprintf(stdout,L"CloseHandle\n");
    getchar();
   char* ptr=(char*)malloc(siz);
cs.Start();
	   siz2= sm_region::mp_copy<20>(hm,ptr,siz);
   t=cs.Sec();
   de=siz-siz2;
   fwprintf(stdout,L"mp:: %g %g  MB/sec =%g tic/sec =%d\n",t,de,double(siz)/double(MB)/t,int(double(c)/t));
   getchar();

   cs.Start();
   siz2= sm_region::pm_copy<20>(ptr,hm,siz);
   t=cs.Sec();
   de=siz-siz2;
   fwprintf(stdout,L"pm:: %g %g  MB/sec =%g tic/sec =%d\n",t,de,double(siz)/double(MB)/t,int(double(c)/t));
   getchar();

}

void test_thread_jobs(int nt=10)
{
     thread_pool_job_t<> thread_pool_job(nt);
}

#define MB ((long long)(1024*1024))
HRESULT hrq;


struct bstr_test_t
{
	LONGLONG lenb;
	wchar_t buf[1];
};

#include <map>
#include <typeinfo.h>

/*
template <class N>
struct  type_buffer_t
{

	type_buffer_t():count(0),p(0){};
   
   template <class Other>    	
      load(size_t _count,Other* pv=0)		  
	{
		
		p=(N*)::realloc(p,(count=_count)*sizeof(N));
		if(pv)
			for(size_t k=0;k<count;k++)
				p[k]=N(pv[k]);
		
	}
	  ~type_buffer_t(){  ::free(p);}

	  operator N*() {return p;}

  N* p;
  size_t count;
  

};
*/
template<class F>
struct matrix3x3_t
{
	F v[3][3];    
};


template <class N,bool FCOPY=false>
struct  type_mutator_t
{
	typedef std::map<const char*,void*> ptrmap_t;

	type_mutator_t(size_t _count,void* _p):count(_count),p((N*)_p){
		
		if(FCOPY) p=get_type_cache<N>();
	};

	~type_mutator_t(){

		ptrmap_t::iterator i=ptrmap.begin();
		for(;i!=ptrmap.end();i++)
		{
			::free(i->second);
		}


	}
	


template <class O>    	
   operator O*() {
	   
	   if(typeid(O).name()==typeid(N).name())
	     return (O*)p;
	   else return get_type_cache<O>();
   }
template <class O> 
  O* get_type_cache()
   {
	   void*& ptr=ptrmap[typeid(O).name()];

	   if(!ptr)
	   {
		   ptr=::malloc((count)*sizeof(O));
           O* po=(O*)ptr;
		   for(size_t k=0;k<count;k++)
			   po[k]=O(p[k]);	   		   		   

	   }
	   return (O*)ptr;
	   
   }

    
   N* p;
   char* name;
   size_t count;
   ptrmap_t ptrmap;
};



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
		if((!fcopy)&&(typeid(O).name()==typeid(N).name()))
			p=(N*)po;
		else
		{
			p=ph=(N*)::realloc(ph,(count)*sizeof(N));
			for(size_t k=0;k<count;k++)
				p[k]=N(po[k]);	   		   		   

		}

		return p;		

	};
	
	operator N*() {	
		return p;
	}
	N* p,*ph;
	char* name;
	size_t count;
	
};



void testtypes()
{
	type_buffer_t<double> tbd;

	type_buffer_t<int> tbi;


	double a[5]={1,2,3,4,5};
	int *ia,*ja,*ka;
	double *pa,*pb;

	type_mutator_t<double> tm(5,a);

	ia=tm;
	ja=tm;
	pa=tm;

	tbd.link(5,a);
	tbi.link(5,a);

	pb=tbd;
	ka=tbi;





	
	int l=ia[3];
}

IStream* psstyaut;

static HRESULT get_ip_address(const char* hn   ,char** pp)
{
  HRESULT hr;
  hostent * record;
  
  OLE_CHECK_hr_cond(pp,E_POINTER);
  OLE_CHECK_hr_cond(hn,E_POINTER);
  OLE_CHECK_hr_cond(record = gethostbyname(hn),WSAGetLastError());
  in_addr * address = (in_addr * )record->h_addr;
  *pp = inet_ntoa(*address);
  return S_OK;
}

int _tmain(int argc, _TCHAR* argv[])
{ 

  char *p1,*p2;
  ghr=get_ip_address("dx",&p2);
  ghr=get_ip_address("10.120.60.211",&p1);
  


	hostent * record = gethostbyname("dx");
	hostent * record2 = gethostbyname("10.120.60.211");
	 matrix3x3_t<double> mdd1;
	int ssz=sizeof(mdd1);
  testtypes();
   return 0;
	double ddd=0;

	INT64 * iddd=(INT64 *)&ddd;

	test_thread_jobs();
	return 0;
	HANDLE hp=OpenProcess(PROCESS_ALL_ACCESS,0,GetCurrentProcessId());
	//hp=0;
	HANDLE hhh[2]={0,hp};
	DWORD wss;
  wss=WaitForMultipleObjects(2,hhh,0,2000);
	HMODULE hmm=LoadLibraryW(L"D:\\tmp\\exp_pade_data\\Debug\\exp_pade_data.dll");

	exp_pade_helper_t eph;

  wchar_t dir1[256], dir2[256];
  variant_t yyv=-1334565323.77;
  INT_PTR uui=yyv;

  GetSystemDirectory(dir1,256);
  GetSystemWow64Directory(dir2,256);


  ;




  //  testfm();
	//long long yyyyy;
	__int64 yyyyy=(__int64(1)<<53)-1,yy;
	//long long llyy;
	ULONGLONG llyy;
	double dyy;


VARTYPE vttt=	VARTYPE_finder_t<>::VarType(&yyyyy);
variant_t v=yyyyy;
yy=v;
llyy=v;
dyy=v;
DWORD dwlll;
HANDLE hinv=INVALID_HANDLE_VALUE,hhhd;
hinv=0;
BOOL fh=GetHandleInformation(hinv,&dwlll);
hrq=GetLastError();

	hhhd=ipc_utils::dup_handle(hinv);

ghr=HRESULT_FROM_WIN32(S_FALSE);
if(0)
{
	bstr_test_t bb={0x0000000100000000};
	LONGLONG lenb=SysStringByteLen(bb.buf);
	LONGLONG lenw=SysStringLen(bb.buf);

	BSTR bss=SysAllocStringByteLen(0,0xffffFFFF/2);
	lenb=SysStringByteLen(bss);


   argv_zzs<wchar_t> args;
    bss=SysAllocStringLen(L"AAAAAAAAAAA",100);
   CoTaskMemFree(bss);


   HANDLE hiv=INVALID_HANDLE_VALUE,hiv0=0;
   CloseHandle(hiv);
   CloseHandle(hiv0);


   std::wstring ssa=L"0xFFFFFFF";
   argv_zzs<wchar_t>::tstringstream strm(ssa);
   int iii;
   strm>>iii;


}  
   
	


	SYSTEM_INFO si;
	GetSystemInfo(&si);

	LARGE_INTEGER l,loff,loff2;
	l.QuadPart=1*1024*MB;
      loff.QuadPart=-1;
	  loff.HighPart=loff.HighPart&0x7FFFFFFF;

	//args[L"int64"]=loff.QuadPart;

	//loff2.QuadPart=args[L"int64"];



	//l.QuadPart=256*MB;
	//	l.QuadPart=16*MB;
	HANDLE hm,hm2,hm3,hm4,hm5;
     HANDLE hhmm[1000];
	int co=0;
	v_buf<wchar_t> bufp;
	if(0)for(;co<1000;co++)
	{
		wchar_t* pn=bufp.printf(L"nn%d",co);
		hhmm[co]=CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE|SEC_COMMIT,l.HighPart,l.LowPart,pn);
		if(!hhmm[co]) break;
	}
   hrq=GetLastError();
	
	//hm=CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE|SEC_COMMIT,l.HighPart,l.LowPart,0);
	//hm2=CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE|SEC_COMMIT,l.HighPart,l.LowPart,0);
BOOL fw;
     SIZE_T smi,sma;

    fw=GetProcessWorkingSetSize(GetCurrentProcess(),&smi,&sma);

   hm=CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE|(0*SEC_RESERVE),l.HighPart,l.LowPart,0);
   hrq=GetLastError();
	hm2=CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE|(0*SEC_RESERVE),l.HighPart,l.LowPart,0);
	hrq=GetLastError();
char* pp0=0,*pp1;

/*
	char* pp0=0,*pp1;
	SIZE_T s00=4*1024*MB;
	

	loff.QuadPart=0;

	pp0=(char*) MapViewOfFile(hm,FILE_MAP_WRITE,loff.HighPart,loff.LowPart,s00);
	pp1=(char*)VirtualAlloc(pp0,s00,MEM_COMMIT,PAGE_READWRITE);
	memset(pp1,'A',s00/4);
	//pp1=(char*)VirtualAlloc(pp0,s00,MEM_RESERVE,PAGE_READWRITE);
    //fw=VirtualFree(pp0,s00,MEM_DECOMMIT);
	fw=VirtualUnlock(pp0,s00); 
    hrq=GetLastError();
	fw=VirtualFree(pp0,s00,MEM_DECOMMIT);
	fw=FlushViewOfFile(pp0,s00);
		hrq=GetLastError();
	fw=UnmapViewOfFile(pp0);



    loff.QuadPart=1*1024*MB;
	pp0=(char*) MapViewOfFile(hm,FILE_MAP_WRITE,loff.HighPart,loff.LowPart,s00);
	pp1=(char*)VirtualAlloc(pp0,s00,MEM_COMMIT,PAGE_READWRITE);
	memset(pp1,'A',s00/4);
	//pp1=(char*)VirtualAlloc(pp0,s00,MEM_RESERVE,PAGE_READWRITE);
	//fw=VirtualFree(pp0,s00,MEM_DECOMMIT);
	fw=UnmapViewOfFile(pp0);




	*/
    hrq=GetLastError();

    testfm(hm,l.QuadPart,hm2);
	return 0;

	void* ppm,*ppm2;
	

	sm_region::region_cache_t<> rc(hm);

    sm_region::sm_region_t rg,rg2;
	pp0=sm_region::map_region(hm,1024,0,&rg);
	strcpy(pp0,"0123456789ABCDEF");
	pp1=sm_region::map_region(hm,3,63*1024,&rg2);
	pp1=sm_region::map_region(hm,3,100*MB+3,&rg2);



	//locker_t<shared_robust_mutex_t<>> lock(srmutex0);
	
	pp0=0;

	pp0=(char*)MapViewOfFileEx(hm,FILE_MAP_ALL_ACCESS,0,0,KB,pp0);
	hrq=GetLastError();
	pp0=(char*)MapViewOfFileEx(hm,FILE_MAP_ALL_ACCESS,0,0,KB,pp0);
	hrq=GetLastError();
	pp0=(char*)MapViewOfFileEx(hm,FILE_MAP_ALL_ACCESS,0,0,2*KB,pp0);
	hrq=GetLastError();

    
	LONGLONG offs=0,sizl=0;
	int sz=4096;
	BOOL fff;
	//	sz=0x10000;
	if(0)
		for(;offs<l.QuadPart;offs+=0x10000)
	{
        pp0=(char*)MapViewOfFile(hm,FILE_MAP_ALL_ACCESS,0,offs,sz);
		memset(pp0,11,sz);
		//pp0[0]=1;
		//pp0[sz-1]=1;
		

		sizl+=sz;
		if(!pp0) break;
	}


	


	

	pp0=(char*)MapViewOfFile(hm,FILE_MAP_ALL_ACCESS,0,0,2*0x10000);

	pp0[4096-1]=-11;
	fff=UnmapViewOfFile(pp0+0x10000);
	pp0[4096-1]=-1;

	fff=UnmapViewOfFile(pp0);


	pp1=(char*)MapViewOfFile(hm,FILE_MAP_ALL_ACCESS,0,0,4096+1);
	hrq=GetLastError();

	pp0[4096-1]=11;
	pp1[2*4096-1]=11;

	getchar();



  

     ppm=MapViewOfFile(hm,FILE_MAP_ALL_ACCESS,0,0,256*MB);


	loff.QuadPart=8*1024*MB;
	ppm2=MapViewOfFile(hm,FILE_MAP_ALL_ACCESS,loff.HighPart,loff.LowPart,256*MB);

	//asyn_su::
	//asyn_su::_pool<AA,false>::register_kobject(komutex.hko);
	HRESULT hr=0;
	//int aahh=SetEnvironmentVariableW(0,0);
   hr=GetLastError();
	HANDLE hko;
	
	HANDLE h;
	hr=0;
	DWORD tt=GetCurrentThreadId();

	bool ff= shared_robust_mutex_t<>::validate_thread(5700,&hr);
	//h=OpenThread(THREAD_QUERY_INFORMATION,0,2168);
	//hr=GetLastError();
	testm(0);
	
	argv_env<wchar_t> arg;

	arg[L"region.handle"]=22;
	argv_zzs<wchar_t> aarg;
       aarg=arg;
	
	return testtxt(aarg);
	DWORD pid=GetCurrentThreadId();
	for(;;)
	{
		for(int c=0;c<10;c++)
		{
			for(int k=0;k<10000;k++) InterlockedCompareExchange(&ll,0,0);
			// InterlockedIncrement(&ll);
			//SwitchToThread();
			//SleepEx(10,1);
			//check_thread_lifetime(pid);
			Sleep(8);
		}
		check_thread_lifetime(pid);

	}
	return 0;
}

