#pragma once
#include <static_constructors_0.h>
#include <windows.h>
#include <corecrt_malloc.h>

struct __default_base {};

template <int nTLS,class NewBase=__default_base>
class thread_storage_dll_base:class_initializer<thread_storage_dll_base<nTLS,NewBase> >
{

public:
	//static long m_tls_index;
	inline	static void static_ctor()
	{
		tls_index_ref()=::TlsAlloc(); 
		//AtlTrace(L"thread_storage_dll_base<%d>::tls_index=%d\n",nTLS,m_tls_index);
	}
	inline	static void static_dtor()
	{
		::TlsFree(tls_index_ref());
	};
	//protected:
	inline 	static    void* get_thread_var()
	{ 
		return ::TlsGetValue(tls_index_ref());
	};    
	inline 	static    void set_thread_var(void*p )
	{
		::TlsSetValue(tls_index_ref(),p); 
	};

	inline 	static long tls_index()
	{
		
		return tls_index_ref();
	}
protected:
	
	inline 	static long& tls_index_ref()
	{
        static long s_tls_index;
		return s_tls_index;
	}

};

/*
template <int nTLS,class NewBase>
//__declspec(selectany)
long thread_storage_dll_base<nTLS,NewBase>::m_tls_index;
*/




template <int ntls,class NewBase=__default_base> 
class wild_tls
{

public:
	//inline  wild_type(thread_storage_dll_base<nTLS>& ts): m_ts( ts){};
	template <class A>
	inline operator A()
	{
		//	A dummy; // for DEBUG type test
		return (A) thread_storage_dll_base<ntls,NewBase>::get_thread_var();
	}
	template <class A>
	inline wild_tls& operator =(A a )
	{
		thread_storage_dll_base<ntls,NewBase>::set_thread_var((void*)a);
		return *this;
	}
   template <class A,class B>
   inline A swap(B b)
   {
      A a=(A) thread_storage_dll_base<ntls,NewBase>::get_thread_var();
       thread_storage_dll_base<ntls,NewBase>::set_thread_var((void*)b);
      return a; 
   }
   template <class A>
   inline A make_detach(A b)
   {
        return swap(b);
   }
   template <class A>
   inline A make_detach()
   {
	   return swap(A());
   }
    
	inline 	long tls_index()
	{
		return thread_storage_dll_base<ntls,NewBase>::tls_index();
	}

};

inline 
wild_tls<0> tls_var()
{
	return wild_tls<0>();
}


template <int ntls> 
inline
wild_tls<ntls> tls_var()
{
	return wild_tls<ntls>();
}

template <class NewBASE> 
inline
wild_tls<0,NewBASE> tls_var()
{
	return wild_tls<0,NewBASE>();
}

template <class NewBASE,int nt> 
inline
wild_tls<nt,NewBASE> tls_var()
{
	return wild_tls<nt,NewBASE>();
};


struct vallocator
{

 static void* realloc(void *p,unsigned int cb,unsigned int *pcb=0)
 {


	 p=VirtualAlloc(p,cb,MEM_COMMIT,PAGE_READWRITE);
	 
 if(!p)
 {
       p=VirtualAlloc(0,cb,MEM_COMMIT,PAGE_READWRITE);
   if(!p) return NULL;
 }
  if(pcb)
  {
	 SYSTEM_INFO si;
     GetSystemInfo(&si);
	 DWORD ff=si.dwAllocationGranularity-1;
      *pcb=(~ff)&(cb+ff);
  }
	 return p;
 }
static void free(void* p)
{
	VirtualFree(p,0,MEM_RELEASE);
}

};

template <int nalign=64>
struct align_allocator
{

	static void* realloc(void *p,unsigned int cb,unsigned int *pcb=0)
	{


		p=_aligned_realloc(p,cb,nalign);
			//
		if(!p)
		{
			p=_aligned_malloc(cb,nalign);
			if(!p) return NULL;
		}
		if(pcb)
		{
			*pcb=cb;
		}
		return p;
	}
	static void free(void* p)
	{
		_aligned_free(p);
	}

};




//#define  BUF_ALLOCATOR vallocator
//
#define  BUF_ALLOCATOR align_allocator<64>
#define KB ULONG64(1024)
#define MB (KB*KB)
struct tls_buffer_singleton_dflt{};
template <int num=0,typename NN=tls_buffer_singleton_dflt>
struct tls_buffer_singleton
{
	
	struct buffer
	{
		union
		{
			struct {char c[128];};
			struct
			{ 
				ULONG32 count;
				ULONG32 refcount; 
			};
		};
		char ptr[1];
	};
    //buffer* pb;
	static inline buffer* pbuf()
	{
		return (buffer*) tls_var<tls_buffer_singleton,num>();
	}
	template<typename N>
	static inline N* ptrT()
	{
		return (N*)pbuf()->ptr;
	};

	static inline void* ptr()
	{
		return pbuf()->ptr;
	};


	//*
	template<typename N>
	inline operator N*()
	{
		return (N*)ptr();
	}
	//*/
	tls_buffer_singleton(unsigned int cb=(8*MB))
	{
		buffer* pb;
		pb=pbuf();
		if(!pb)
		{
		
			//if(!(pb=(buffer*)::calloc(cb,1))) return;
     		if(!(pb=(buffer*)BUF_ALLOCATOR::realloc(0,cb,&cb))) return;
			pb->count=cb;
			pb->refcount=0;
			tls_var<tls_buffer_singleton,num>()=pb;
		}

		if(pb->count<cb)
		{
			//buffer* p=(buffer*)::realloc(pb,cb);
           buffer* p=(buffer*)BUF_ALLOCATOR::realloc(pb,cb,&cb);

			if(!p) return ;
             pb->count=cb;
			tls_var<tls_buffer_singleton,num>()=pb=p;
		} 
		pb->refcount++;
	}
	~tls_buffer_singleton()
	{
		buffer* pb;
		pb=pbuf();	
		if(pb)
		{
			pb->refcount--;
			if(pb->refcount<=0)
			{
			 	//::free(pb);
               BUF_ALLOCATOR::free(pb);
			    tls_var<tls_buffer_singleton,num>()=(void*)0;
			}
		}
	}
};


#include <list>
#include <vector>
struct tss_cleaner
{

//#pragma check_stack(push,off)
//#pragma runtime_checks( "", off )

	struct tss_item
	{

		typedef void (__stdcall *free_proc_type)(void *);

		void* m_p;
		void *m_destructor;
		//tss_item(void *p=0,void *destructor=0): m_p(p),m_destructor(destructor){};
		tss_item(): m_p(0),m_destructor(0){};
		inline void init(void *dproc,void*p)
		{
			m_destructor=dproc;
			m_p=p;
		};
		inline static void safe_call(void *proc,void* param,void* param2=0)
		{
			if((param)&&(proc))
			{
				__try
				{

#if defined(_WIN64)

					typedef void (*x64__safe_wild_call_t)(void* ,void*,void* );

					((x64__safe_wild_call_t)proc)(param,param2,param2);

#else


					DWORD ustack;
					
                   __asm {
					     mov ustack,ESP;
						 push 0x00000000;
						 push param2;
						 push param2;
						 mov  ECX,param;
						 push ECX;
						 mov  EAX,proc;
						 call EAX;
                         mov ESP,ustack;
				         }; 
					
#endif
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
				}

			}
		}
		inline void clear()
		{
			m_p=0;
		}
		~tss_item()
		{
			void* p=tls_var<tss_item>();
			safe_call(m_destructor,m_p,p);
		}


	};
#pragma runtime_checks( "", restore ) 
#pragma check_stack(pop)

	typedef std::list<tss_item> tss_heap_list;

	// tls_var<tss_item>


	inline static void clean_tss(DWORD dwReason)
	{
		tss_heap_list* phl=tls_var<tss_item>();
		if(phl) 
		{
			tls_var<tss_item>()=dwReason;
			delete phl;
			tls_var<tss_item>()=0;
		}

	};

	inline static int   __process_exit()
	{
         clean_tss(DLL_PROCESS_ATTACH);
		 return 0;
	}
	inline static void clean_tss_if_reason(unsigned long dwReason)
	{
		if(dwReason ==DLL_PROCESS_ATTACH)
		{
			//_onexit(&__process_exit);
		}
		else
		if((dwReason == DLL_THREAD_DETACH)|| (dwReason == DLL_PROCESS_DETACH))
			clean_tss(dwReason);
	}

	inline static void* on_exit_thread(void* proc,void *ptr)
	{
		tss_heap_list* phl=tls_var<tss_item>();
		if(!phl) 
		{
			phl=new tss_heap_list();

			tls_var<tss_item>()=phl;
		};

		if(!phl) return  0;
		//phl->push_back(tss_item(ptr,proc));

		phl->push_front(tss_item());
		tss_item& ssi=phl->front();
		ssi.init(proc,ptr);  

		return &ssi;
	}

	inline static int remove_exit_thread_key(void* key,bool fmake=0)
	{
        tss_heap_list* phl=tls_var<tss_item>();
		if(!phl) return 0;
		tss_heap_list::iterator i;
		for(i=phl->begin();i!=phl->end();i++)
		{
           tss_item& ti=*i;
		   if(key==(void*)&ti)
		   {
			   if(!fmake) ti.clear();
			   phl->erase(i);
			   return 1;
		   }
   		}

	}

};

struct tls_LastErrorSaver
{
	HRESULT err;
	tls_LastErrorSaver(DWORD e=tls_var<tls_LastErrorSaver>()){
		set(err=e);
	};
	~tls_LastErrorSaver(){SetLastError(err);tls_var<tls_LastErrorSaver>()=err;};
	inline DWORD set(DWORD e=GetLastError())
	{
		;
		return err=e;
	}
	inline DWORD get()
	{
		return err;
	}

	operator HRESULT()
	{
		return HRESULT_FROM_WIN32(err);
	}
	tls_LastErrorSaver& operator =(DWORD e)
	{
		set(e);
		return *this;
	}
};
