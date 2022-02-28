#pragma once
#include <static_constructors_0.h>
#include <windows.h>
//
#include <tls_dll.h>
#include "os_utils.h"


#define DFLT_ITEM_SIZE int(128)
#define POOL_SIZE_LOG 12 
// 4 KB 

template <typename T,const T  t >
struct sdef
{};

//namespace{


namespace  no_win_2003_vista
{
typedef void (__stdcall * pflsproc)(void*) ;
DWORD (__stdcall * FlsAlloc)(void*) ;
BOOL (__stdcall * FlsFree)(DWORD ) ;
BOOL (__stdcall * FlsSetValue)(DWORD ,void*) ;
void* (__stdcall * FlsGetValue)(DWORD ) ;

inline bool init_fiber()
{
  HMODULE h=GetModuleHandleW(L"kernel32.dll");
  if(SET_PROC_ADDRESS(h,FlsAlloc))
  {
    SET_PROC_ADDRESS(h,FlsFree);
	SET_PROC_ADDRESS(h,FlsSetValue);
	SET_PROC_ADDRESS(h,FlsGetValue);
	return true;
  }
  else
  {
       FlsFree=&TlsFree;
       FlsGetValue=&TlsGetValue;
       FlsSetValue=&TlsSetValue;
	   return false;
  }
}
};
//}

#define _WSV no_win_2003_vista

//#define SET_PROC_ADDRESS_NS(h,ns,a) _set_dll_proc(h,(void**)&ns##::##a,#a)


template <int n_item_size>
struct  buf_list
{

//template <int n_item_size>
struct _buf_item
{
	union{
		struct{
			void* ptr;
			void* pallocator;
		};
		struct{
			long long llval;
		};
		struct{
			wchar_t msg[n_item_size];
		};

	};

	typedef void ( __cdecl * free_ptype)(void* ptr,void*);
	//void ( __cdecl * free_ptr)(void* ptr,void*);
	free_ptype free_ptr;

	inline void free()
	{
		if((ptr)&&(&free_ptr))
			free_ptr(ptr,pallocator);
	}
	inline void reset(void* newptr, void ( __cdecl * newproc)(void*))
	{
		free();
		ptr=newptr;
		free_ptr=(free_ptype)newproc;
	}
	inline void reset(void* newptr,void* allocator,free_ptype newproc )
	{
		free();
		ptr=newptr;
		pallocator=allocator;
		free_ptr=newproc;
	}

	inline	 bool is_null()
	{
		return ptr==NULL;
	}
} ;

  //
typedef  typename  _buf_item   buf_item;
//typedef   buf_item_n<n_item_size> buf_item;


    DWORD  tid;
	HANDLE hthread,hgc;
	long count; 
	buf_item plst[1];
     
	~buf_list()
	{
		for(int n=count-1;n>=0;n--)
	    	plst[n].free();
	
		if(hgc)
		{
			UnregisterWait(hgc);
		    CloseHandle(hthread);
		}
	}

static void __stdcall gc_proc(buf_list* pthis,bool )
{
   delete pthis;
}
static void __stdcall fls_proc(buf_list* pthis)
{
	delete pthis;
}

static buf_list* create(long count,DWORD tid=GetCurrentThreadId())
{
	buf_list* pbl= ::new (calloc(1,sizeof(buf_list)+sizeof(buf_item)*count)) buf_list(count,tid);

	if(!(pbl->tid)) 
		return delete pbl,NULL;

    return pbl;
}
protected:
buf_list(long _count,DWORD _tid=GetCurrentThreadId())
{
  count=_count;
  bool fsuccess=true;
  bool fFiber=(GetCurrentThreadId()==_tid)&&(_WSV::FlsAlloc);
  if(!fFiber)
  {
  hthread=OpenThread(THREAD_ALL_ACCESS,false,_tid);
  fsuccess=::RegisterWaitForSingleObject(&hgc,hthread,WAITORTIMERCALLBACK(&gc_proc),this,INFINITE,WT_EXECUTEONLYONCE);  
  }
 tid=(fsuccess)?_tid:0;

}


};
//typedef buf_list* pbuf_list;


//typedef buf_list<256>::buf_item tls_item;      

template <int n_item_size=DFLT_ITEM_SIZE>
struct thread_dll_buf:public class_initializer<thread_dll_buf<n_item_size> >
{

     //static long m_tls_index;
	 //static long m_last_index;

	 inline	static void static_ctor()
	 {
        
    	 
		 tls_index();
		 //m_tls_index=::TlsAlloc(); 
		 //m_last_index=0;
		 //AtlTrace(L"thread_storage_dll_base<%d>::tls_index=%d\n",nTLS,m_tls_index);
	 }
	 inline	static void static_dtor()
	 {
		 ::TlsFree(tls_index());
	 };





typedef buf_list<n_item_size>   tls_buf;      
typedef tls_buf* ptls_buf;
typedef typename buf_list<n_item_size>::buf_item tls_item; 

inline static  tls_item* get_item(long index)
 {
	 void *p=_WSV::FlsGetValue(tls_index());
     if(!p)
	 {  
        _WSV::FlsSetValue(tls_index(),p=tls_buf::create(last_index()));  
	 } 
   if((index>=0)&&(index<last_index())) 
   return ptls_buf(p)->plst+index;
   return NULL;
 }


inline static long& last_index()
{
	static long s_last_index=0;
	return s_last_index;
};

inline static long tls_index()
{
	 
	static long s_tls_index=(_WSV::init_fiber())? (_WSV::FlsAlloc(&buf_list<n_item_size>::fls_proc))  :(::TlsAlloc() ) ;
	return s_tls_index;
};

inline static void reset()
{
    ptls_buf p=ptls_buf(_WSV::FlsGetValue(tls_index()));
	if(p)
	{
		_WSV::FlsSetValue(tls_index(),NULL);
        //if(p->hgc) UnregisterWait(p->hgc);		 
		delete p;
	}
};


inline static long s_add_index()
{
   tls_index();
  //return m_last_index++;
  return last_index()++;
};

};

/*
template <int n_item_size>
__declspec( selectany ) long   thread_dll_buf<n_item_size>::m_last_index=0;
template <int n_item_size>
__declspec( selectany ) long   thread_dll_buf<n_item_size>::m_tls_index;
*/

template <class T,typename Container=T,int n_item_size=DFLT_ITEM_SIZE>
struct thread_singleton_ptr_base:public class_initializer<thread_singleton_ptr_base<T,Container,n_item_size> >
{


typedef  typename buf_list<n_item_size>::buf_item tls_item;      

	//static long m_index;
thread_singleton_ptr_base()
{
  int mi=thread_dll_buf<n_item_size>().last_index();
}
inline static long& item_index()
{
      static long s_index=0;
	  return s_index;
}

	inline	static void static_ctor()
	{
         // thread_dll_buf<n_item_size>::static_ctor();
         
		item_index()=thread_dll_buf<n_item_size>::s_add_index();
	}
	inline	static void static_dtor(){};
	static 	void __cdecl free_ptr(void* ptr)
	{
		delete (T*)ptr;
	};



inline	static  tls_item& get_buf_item()
{
  tls_item* pib=thread_dll_buf<n_item_size>::get_item(item_index());
  return *pib;
}
inline	static  bool is_null()
{
	return get_buf_item().is_null();
}

inline static void   reset(T* pt,void (__cdecl *proc)(void*))
{
  get_buf_item().reset((void*)pt,proc);
}

};

//template <class T,typename C,int n_item_size>
//long thread_singleton_ptr_base<T,C,n_item_size>::m_index=0;


template <class T,typename Container=T,int n_item_size=DFLT_ITEM_SIZE>
struct thread_singleton_var:	public thread_singleton_ptr_base<T,Container,n_item_size>
{
 template <typename L>
 operator L()
 {
   return   L(get_buf_item().llval);
 }
 template <typename L>
 thread_singleton_var& operator =(L l)
 {
    get_buf_item().llval=(long long)(l);
	return *this;
 }

};

template <class L,typename Container,int n_item_size=DFLT_ITEM_SIZE>
struct thread_singleton_varT:	public thread_singleton_ptr_base<L,Container,n_item_size>
{
	operator L&()
	{
		return  *((L*)( &get_buf_item().llval));
	}
	
/*
	thread_singleton_varT& operator =(L l)
	{
		//int ls=min(sizeof(L),max(sizeof(buf_list::buf_item.llval));
        int ls=min(sizeof(L),max(sizeof(tls_item.llval),sizeof(tls_item.msg)));
        memcpy(&get_buf_item().llval,&l,ls);  
		return *this;
	}
*/
};


#include <memory>
template <class T,typename Container=T,class Allocator=std::allocator<T>,int n_item_size=DFLT_ITEM_SIZE>
struct thread_singleton_ptr:	public thread_singleton_ptr_base<T,Container,n_item_size>
{

	//static long m_index;
/*
	inline	static void static_ctor()
	{
		  m_index=thread_dll_buf::s_add_index();
	}
	inline	static void static_dtor(){};
*/

static 	void __cdecl free_ptr(void* ptr)
	{
        
		//delete (T*)ptr;
		Allocator::rebind<T>::other a;
		a.destroy((T*)ptr);
		a.deallocate((T*)ptr,sizeof(T));

	};


 static T* s_get_ptr()
 {
   return (T*)(thread_dll_buf<n_item_size>::get_item(item_index())->ptr);
 }

 virtual   T* get_ptr() const
 {
   return s_get_ptr();
/*
   buf_list::buf_item* pib=thread_dll_buf::get_item(m_index);
             
     
     if(pib->is_null())
	 {
	     pib->reset(new T(),&free_ptr);  
	 }

   return (T*)(pib->ptr);
*/

 }

 inline  void   reset(T* pt)
 {
	 thread_singleton_ptr_base::reset(pt,&free_ptr);
 }



 inline	operator T*() const 
	{
		return get_ptr();
	}

inline	thread_singleton_ptr& operator =(T* t) 
{
  reset(t);
  return *this;
}

 inline 	T& operator*() const
	{
		
		return *get_ptr();
	}
	
 inline 	T* operator->() //const 
	{
	
		return get_ptr();
	}
 inline 	bool operator!() const 
	{
		return is_null();
	}

};






template <class T,typename Container=T,class Allocator=std::allocator<T>,int n_item_size=DFLT_ITEM_SIZE>
struct thread_singleton_ptr_auto_create:public thread_singleton_ptr<T,Container,Allocator,n_item_size>
{
	
/*	static 	void __cdecl free_ptr(void* ptr)
	{
		delete (T*)ptr;
	};
*/

	inline	operator T*() const 
	{
		return _ptr();
	}


 virtual T* get_ptr() const
 {
   return _ptr();
 }
 inline static T* _ptr()
{
    tls_item* pib=thread_dll_buf<n_item_size>::get_item(item_index());
	if(pib->is_null())
	{
        
		 //::new( allocate<VHolder,allocator>())  VHolder(val);
    	//pib->reset(new T(),&free_ptr);  
		pib->reset(::new( Allocator::rebind<T>::other().allocate(1)) T(),&free_ptr);  
	}
	return (T*)(pib->ptr);
};
};

#define thread_auto_ptr_auto thread_singleton_ptr_auto_create

//template <class T,class C>
//long thread_auto_ptr<T,C>::m_index=0;
//static 
//long thread_storage_dll_base<nTLS,NewBase>::m_tls_index;

template <typename T,typename U=char>
struct void_types
{
	enum { isvoid=0 };
	typedef  T Type;
};
template <typename U>
struct void_types<void,U>
{
	enum { isvoid=1 };
	typedef  U Type;
};

template <class Holder,unsigned char pool_size_log2=POOL_SIZE_LOG,class BaseAllocator=std::allocator<void> > // size=2^pool_size_log2
struct tls_pool
{


/*
struct POOL_TLS
	{
		typedef void* pointer;
		long RefCount;
		long AllocSizeMax;
		long FreeSize;
		long LastAllocSize;

		unsigned long adress_mask;
		long granularity;
		long max_size;
		char* ptr;
		char* ptrbottom;

		POOL_TLS()
		{
			SYSTEM_INFO si;
			GetSystemInfo(&si);
			granularity=si.dwAllocationGranularity;
			RefCount=0;
			size_t siz= ((pool_size+granularity-1)/granularity)*granularity;
			FreeSize=max_size=siz-sizeof(POOL_TLS);
			ptrbottom=ptr=((char*)this)+siz; 
			LastAllocSize=0;
			adress_mask=~(granularity-1); 
		}


		inline bool check_in(size_t siz)
		{
			siz=((siz+0x3)>>2)<<2;
			return FreeSize>=siz;
		};

		inline void* allocate(size_t siz)
		{
			siz=((siz+0x3)>>2)<<2;
			if(FreeSize>=siz)
			{
				ptr-=(LastAllocSize=siz);
				++RefCount;
				{
				size_t si=size_t(ptrbottom)-size_t(ptr);
				if(si>AllocSizeMax) AllocSizeMax=si;
				}
				FreeSize-=LastAllocSize;
				//return this;
				return ptr;
			}
			else return calloc(siz,1);
		}

		inline void deallocate(pointer p)
		{
			if( (DWORD_PTR(p)&DWORD_PTR(adress_mask))==DWORD_PTR(this))
			{
				if( (--RefCount)<=0) 
				{
					RefCount=0;
					FreeSize=max_size;
					ptr=ptrbottom;

				}
				else
					if(ptr==p) 
					{
						ptr+=LastAllocSize;
						FreeSize+=LastAllocSize;
						
					}
                 LastAllocSize=0;
			}
			else  ::free(p);
		}

		void* operator new(size_t bytes)
		{
			SYSTEM_INFO si;
			GetSystemInfo(&si);
			long granularity=si.dwAllocationGranularity;

			size_t siz= ((pool_size+granularity-1)/granularity)*granularity;

			return ::VirtualAlloc(NULL,siz,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
		}
		void operator delete(void* ptr, size_t bytes)
		{
			VirtualFree(ptr,0,MEM_RELEASE);
		}


	};
*/



	struct POOL_TLS_data
	{
		long RefCount;
		long AllocSizeMax;
		long OverFlops;
		long OverFlopsMax;
		long FreeSize;
		long LastAllocSize;
		char* ptr;
		//char* ptrbottom;
		//char* ptrtop;
	protected:
		POOL_TLS_data(long _FreeSize):RefCount(0),FreeSize(_FreeSize),LastAllocSize(0),AllocSizeMax(0),OverFlops(0),OverFlopsMax(0)
		{};

	};


	struct POOL_TLS:POOL_TLS_data
	{
		
    	
		typedef void* pointer;

		inline static void* _malloc(size_t n)
		{
			return BaseAllocator::rebind<char>::other().allocate(n);
		}
		inline static void _free(void* ptr, size_t n=0)
		{
          BaseAllocator::rebind<char>::other().deallocate((char*)ptr,n);
		}

		
		enum
		{   
			full_size_POOL_TLS=size_t(1)<<pool_size_log2,
			top_offset=sizeof(POOL_TLS_data),
			bottom_offset=full_size_POOL_TLS,
			size_POOL_TLS=full_size_POOL_TLS-top_offset,
			min_block_size=sizeof(pointer),
			bs_shift=min_block_size/2

		};

      char buf[size_POOL_TLS];

		POOL_TLS():POOL_TLS_data(size_POOL_TLS)
		{
			//size_t siz=full_size_POOL_TLS ; 
			ptr=bottom();
			/*
            LastAllocSize=0;
			RefCount=0;
			
			FreeSize=size_POOL_TLS; //max_size=
			//ptrbottom=ptr=((char*)this)+siz; 
			//adress_mask=~(granularity-1); 
			*/
		};

  inline bool in_pool(void* p)
  {
    register LONG_PTR off_p= LONG_PTR(p)-LONG_PTR(this);
	return (off_p>=top_offset)&&(off_p<bottom_offset);
  }
   inline char* bottom()
   {
       return ((char*)this)+full_size_POOL_TLS;
   }

  inline size_t realignblock(size_t siz)
  {
	  return ((siz+(min_block_size-1))>>bs_shift)<<bs_shift;
  };
	inline bool check_in(size_t siz)
		{
			siz=((siz+(min_block_size-1))>>bs_shift)<<bs_shift;
			return FreeSize>=siz;
		};

		inline void* allocate(size_t siz)
		{	 
		if( (siz=realignblock(siz))<=FreeSize )
			{
				ptr-=(LastAllocSize=siz);
				++RefCount;
				{
					size_t si=size_t(bottom())-size_t(ptr);
					if(si>AllocSizeMax) AllocSizeMax=si;
				}
				FreeSize-=LastAllocSize;
				
				return ptr;
			}
		else{ 
			 if(++OverFlops>OverFlopsMax) OverFlopsMax=OverFlops;
			  return  _malloc(siz);
		     }
		}

		inline void deallocate(pointer p)
		{
			//if( (DWORD_PTR(p)&DWORD_PTR(adress_mask))==DWORD_PTR(this))
			if(in_pool(p))
			{
				if( (--RefCount)<=0) 
				{
					RefCount=0;
					FreeSize=size_POOL_TLS;
					ptr=bottom();

				}
				else
					if(ptr==p) 
					{
						ptr+=LastAllocSize;
						FreeSize+=LastAllocSize;

					}
					LastAllocSize=0;
			}
			else  
			{
				_free(p);
				--OverFlops;
			};
		}

/*
		void* operator new(size_t )
		{
			
			size_t siz=full_size_POOL_TLS ;
			return ::calloc(siz,1);
		}
		void operator delete(void* ptr, size_t bytes)
		{
			//VirtualFree(ptr,0,MEM_RELEASE);
			::free(ptr);
		}
*/

	};



	template <class T>
	class tss_allocator { 
	public:
		template<class _Other>
		struct rebind
		{	typedef tss_allocator<_Other> other;};


		typedef T* pointer;
		typedef const T* const_pointer;
		typedef  typename void_types<T>::Type VT_Type;
		typedef	typename VT_Type& reference;
		typedef typename const VT_Type& const_reference;
		typedef T value_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
        typedef thread_singleton_ptr_auto_create<POOL_TLS,POOL_TLS,BaseAllocator>  thread_singleton_POOL_TLS;

		tss_allocator() throw (){}

		tss_allocator(const tss_allocator<T>&) throw ()	{}

		template<class _Other>
		tss_allocator(const tss_allocator<_Other>& t) throw (){}

		template<class _Other>
		tss_allocator<T>& operator=(const tss_allocator<_Other>&)
		{	
			return (*this);
		}

		pointer address(reference x)
		{ return &x; };
		const_pointer const_address(const_reference x)
		{ return &x; };
		pointer allocate(size_type n)
		{
			return (T*) thread_singleton_POOL_TLS::_ptr()->allocate(n*sizeof(VT_Type));
		};
		void deallocate(pointer p,size_t )
		{
			thread_singleton_POOL_TLS::_ptr()->deallocate(p);
		};

		void construct(pointer _Ptr, const VT_Type& _Val)
		{	
			void  *_Vptr = _Ptr;
			::new (_Vptr) T(_Val);
		};

		void destroy(pointer _Ptr)
		{	// destroy object at _Ptr
			(_Ptr)->~VT_Type();
		}
		
		size_type max_size() const
		{
			size_type _Count = (size_type)(-1) / sizeof (VT_Type);
			return (0 < _Count ? _Count : 1);
		};
	};

	
	typedef    tss_allocator<void>  allocator ;

	struct wild_cast
	{
		void* m_p;
		wild_cast(void* p):m_p(p){};
		template<typename A>
		inline  operator A()
		{
			return (A)m_p;
		}
	};

	inline static wild_cast malloc(size_t n)
	{
		return wild_cast(thread_singleton_POOL_TLS()->allocate(n));
	}
	inline static void free(void*p)   
	{
		thread_singleton_POOL_TLS()->deallocate(p);
	}  

inline static POOL_TLS* get_pool()
{
	return  allocator::thread_singleton_POOL_TLS();
}
};

//
//namespace{
volatile LOCAL_DECLARE(int)=thread_dll_buf<>::last_index();
//};
//volatile int fhfh=thread_dll_buf<>::last_index();