#pragma once

#include "heap_allocator.h"
#include "shared_allocator.h"
#include "char_parsers.h"
#include <intrin.h>
#include "closures.h"
#include "tls_dll.h"
#include <Tlhelp32.h>

const DWORD HGEXZERO=HEAP_ZERO_MEMORY|HEAP_GENERATE_EXCEPTIONS,
HEXGEXZERO=HGEXZERO|HEAP_CREATE_ENABLE_EXECUTE;


///*
template <class T,long flags=HGEXZERO>
struct shared_heap:shared_allocator<T,heap_allocator<T, sys_heap_mem<flags,sys_heap<flags> > > > {

	inline static bool check_ptr(void* p){	 return allocator::heap_t::check(p);};
};
//*/
//
/*
template <class T>
struct shared_heap:shared_allocator<T,heap_allocator<T, sys_heap_mem<HEAP_ZERO_MEMORY,process_heap> > > {

	inline static bool check_ptr(void* p){	 return allocator::heap_t::check(p);};
};
//*/
template <class T>
struct shared_heap_execute:shared_allocator<T,heap_allocator<T, sys_heap_mem<HEXGEXZERO,sys_heap<HEXGEXZERO> > > > {

	inline static bool check_ptr(void* p){	 return allocator::heap_t::check(p);};
};

//#define shared_heap shared_heap_execute

//template <class T,long flags=HEAP_ZERO_MEMORY>
//struct shared_heap_0:shared_allocator<T,heap_allocator<T, sys_heap_mem<flags,sys_heap<flags> > >,  ref_count_COM_support<0>  >  {};
//template <class T,class A=std::allocator<T>,class Header=ref_count_COM_support<1> > 



#define  sh_new(t) ::new((void*)shared_heap<t>().allocate(sizeof(t))) t
#define  sh_new_execute(t) ::new(shared_heap_execute<t>().allocate(sizeof(t))) t
#define  sh_alloc(cb) shared_heap<char>().allocate(cb)





template <typename N,typename E>
inline  ptr_holder<closure_stub> create_stdcall_closure(N proc,void* pcontext,E exitproc,void* pcontext2)
{

	//void* p=shared_heap<closure_stub,HEAP_CREATE_ENABLE_EXECUTE>().allocate();
	return ptr_holder<closure_stub>(sh_new_execute(closure_stub)(proc,pcontext,exitproc,pcontext2),0);
}




inline  ptr_holder<event_signaler> make_event_signaler(HANDLE hevent)
{
	//return ptr_holder<closure_stub>(sh_new_execute(closure_stub)(new event_signaler(hevent),&event_signaler::set,&event_signaler::exit),0);
	ptr_holder<event_signaler> es(sh_new_execute(event_signaler)(hevent),0);
	void* p=es.pv;
	//i_handle::get_info(p)->cv[4]=(const char*)p;
	i_handle::event_ref(p)=p;
	return es;
}




struct OVERLAPPED_shared_closure_base_t:OVERLAPPED
{
	typedef OVERLAPPED_shared_closure_base_t  base_t;
	HANDLE hkObjects[2];
	int hkcount;
	HRESULT   error_code;
	ULONG_PTR user_data;
	//DWORD pid_dest;

};


struct completion_port_signaler:closure_signaler
{
	typedef completion_port_signaler base_t;
	typedef long(* addref_t)(void *p);

	
	ULONG_PTR key;
	addref_t addref_proc;

	inline operator proc_t ()
	{
		return proc_t(this);
	}

	inline void* post(OVERLAPPED* povl,DWORD nbt,HRESULT hr=0)
	{
		tls_LastErrorSaver lerr;

		OVERLAPPED_shared_closure_base_t* pscb=i_handle_cast<OVERLAPPED_shared_closure_base_t>(povl);
		if(pscb) pscb->error_code=hr;
		

		BOOL f=::PostQueuedCompletionStatus(handle(),nbt,key,povl);

		if(f) safe_call(addref_proc,povl);
		else lerr=GetLastError();
		return (void*)f;
	}


inline static   completion_port_signaler* get_io_completion_port_signaler(void* ph)
	{
		tls_LastErrorSaver lerr;

		if(!(ph=i_handle::verify_ptr(ph))) return lerr=E_HANDLE,0;
		else if (!i_handle_cast<completion_port_signaler>(ph=i_handle::completion_port_ref(ph)))
			return lerr=E_HANDLE,0;

		return (completion_port_signaler*)ph;
	}

	inline  static void* fd_attach(void* hIOport,SOCKET s,ULONG_PTR key){

           completion_port_signaler* port=(completion_port_signaler*)get_io_completion_port_signaler(hIOport);
		   if(!port) return 0;
		   void* hp=(s!=INVALID_SOCKET)?CreateIoCompletionPort(HANDLE(s),port->handle(),key,0):port->handle();
		  return hp;
		}

	inline OVERLAPPED* QueuedCompletionStatus(DWORD wtime=INFINITE,DWORD* pnbt=0,PULONG_PTR pKey=0)
	{
		tls_LastErrorSaver lerr(0);
		DWORD nbt;
		ULONG_PTR Key;
		if(0==pKey) pKey=&Key;
		if(0==pnbt) pnbt=&nbt;

		OVERLAPPED* povl=0;
		BOOL f=::GetQueuedCompletionStatus(m_h,pnbt,pKey,&povl,wtime);
		if(f)
		{
			OVERLAPPED_shared_closure_base_t* pscb=i_handle_cast<OVERLAPPED_shared_closure_base_t>(povl);
			if(pscb) lerr=make_detach(pscb->error_code);
		}
		else lerr=GetLastError();
		return povl;
	}

	inline  static void* __stdcall  s_post(completion_port_signaler* p,OVERLAPPED* povl,DWORD nbt)
	{
		return p->post(povl,nbt);
	}



	completion_port_signaler(HANDLE hIOport,ULONG_PTR _key,addref_t arp=0)
		:closure_signaler(0)
		,key(_key)
		,addref_proc(arp)
	{
		if((hIOport)&&(INVALID_HANDLE_VALUE!=hIOport))
		{
			void *p;

			if(p=get_io_completion_port_signaler(hIOport))
			{
				completion_port_signaler* pcps=i_handle_cast<completion_port_signaler>(p);
				if(pcps) hIOport=pcps->handle(); 
			}

			assign_handle(hIOport);
		}
		else
		{
			handle_ref()=CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0); 
		}
		init(&s_post,this);
	}

};


inline DWORD __stdcall parent_pid(DWORD cid=GetCurrentProcessId())
{
	// if(cid==DEFCID) cid=GetCurrentProcessId();
	HANDLE hp= CreateToolhelp32Snapshot(TH32CS_SNAPALL,NULL);
	PROCESSENTRY32 pe;
	pe.dwSize=sizeof(PROCESSENTRY32);
	DWORD pid=0;
	bool fnp=Process32First(hp,&pe);
	while(fnp)
	{
		if (cid==pe.th32ProcessID)
		{
			pid=pe.th32ParentProcessID;
			break;
		}
		fnp=Process32Next(hp,&pe);
	};
	CloseHandle(hp);
	return pid;
};

