#pragma once

#include "static_constructors_0.h" 
//#include "tls_dll.h" 
//HEAP_CREATE_ENABLE_EXECUTE//,long flOptions2=flOptions&(~HEAP_CREATE_ENABLE_EXECUTE)

#define CHECK_HEAP 1


template <class T,class Heap, size_t AddSizeb=4>
class heap_allocator { 
public:
	enum 
	{
		appbyte=AddSizeb
	};
	template<class _Other>
	struct rebind
	{	// convert an allocator<_Ty> to an allocator <_Other>
		typedef heap_allocator<_Other,Heap,AddSizeb> other;
	};


	typedef T* pointer;
	typedef const T* const_pointer;

	typedef	typename T& reference;
	typedef typename const T& const_reference;

	typedef T value_type;
	typedef T VT_Type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef Heap heap_t;


	heap_allocator() throw ()
	{	// construct default allocator (do nothing)
	}

	heap_allocator(const heap_allocator<T,Heap,AddSizeb>&) throw ()
	{	// construct by copying (do nothing)
	}

	template<class _Other>
	heap_allocator(const heap_allocator<_Other,Heap,AddSizeb>& t) throw ()
	{	// construct from related allocator (do nothing)
	}

	template<class _Other>
	heap_allocator<T,Heap,AddSizeb>& operator=(const heap_allocator<_Other,Heap,AddSizeb>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	pointer address(reference x)
	{ return &x; };
	const_pointer const_address(const_reference x)
	{ return &x; };

	pointer allocate(size_type n)
	{
		return (T*) Heap::allocate(n*sizeof(VT_Type)+appbyte);
	};

	pointer reallocate(pointer p,size_type n)
	{
		return (pointer) Heap::reallocate(n*sizeof(VT_Type)+appbyte,p);
	};


	void deallocate(pointer p,size_t )
	{
		Heap::deallocate(p);
	};

	void construct(pointer _Ptr, const VT_Type& _Val)
	{	// construct object at _Ptr with value _Val
		void  *_Vptr = _Ptr;
		::new (_Vptr) T(_Val);
	};

	void destroy(pointer _Ptr)
	{	// destroy object at _Ptr
		(_Ptr)->~VT_Type();
	}
	//size_type init_page_size();
	size_type max_size() const
	{
		size_type _Count = (size_type)(-1) / sizeof (VT_Type);
		return (0 < _Count ? _Count : 1);
	};
};


struct process_heap
{
	inline static	HANDLE heap()
	{
		return ::GetProcessHeap();
	}
};

struct virtual_heap_mem
{
      struct system_info_t
	  {
		   SYSTEM_INFO si;  
           system_info_t()
		   {
			   GetSystemInfo(&si);
		   }
		
		
	  };

	inline static	SYSTEM_INFO& system_info()
	{
		return class_initializer_T<system_info_t>::get().si;
	}
};


/*

template<long flOptions,class Derived=int>
struct sys_heap : public class_initializer<sys_heap<flOptions,Derived> >
{
	//	
	//friend struct self_t::helper;

	


	inline static	HANDLE heap()
	{
		return ref_heap();
	}
	//protected:	
	inline static	HANDLE& ref_heap()
	{
		static HANDLE s_h;
		return s_h;
	}

	inline	static void static_ctor()
	{
		HANDLE h=::HeapCreate(flOptions&(~HEAP_ZERO_MEMORY),0,0);
		ref_heap()=h;
	}
	inline	static void static_dtor()
	{
		HANDLE h=ref_heap();
		::HeapDestroy(h);
	}

	
};
*/

template<long flOptions,class Derived=int>
struct sys_heap  
{
	//	
	//friend struct self_t::helper;
	struct _heap_t
	{
		HANDLE h;
		_heap_t()
		{
			h=::HeapCreate(flOptions&(~HEAP_ZERO_MEMORY),0,0);
		}
		~_heap_t()
		{
			::HeapDestroy(h);
		}

	};


	inline static	HANDLE heap()
	{
		return class_initializer_T<_heap_t>::get().h;
	}
	//protected:	

};


#include <set>


template <long flags=HEAP_ZERO_MEMORY,class Heap=process_heap>
struct sys_heap_mem_base
{
	enum 
	{
		flags_a=flags&(~HEAP_CREATE_ENABLE_EXECUTE),
		flags_f=HEAP_NO_SERIALIZE&flags
	};
  inline static void* allocate(size_t cb)
  {
	  return   ::HeapAlloc(Heap::heap(),flags_a,cb);
  };
  inline static bool deallocate(void* p)
  {
      #ifndef _NO_DEBUG_HSSH_
	  int c=HeapSize(Heap::heap(),0,p);
	   if(c>=0)
		   for(int n=0;n<c;n++) ((char*)p)[n]=0xEB;
		   //memset(p,0xEB,c);
      #endif
	  return   ::HeapFree(Heap::heap(),flags_f,p);
  };
  inline static void* reallocate(size_t cb,void* p=0)
  {
	    if(p) p=::HeapReAlloc(Heap::heap(),flags_a,p,cb);
		else  p=allocate(cb);
		return p;
  };
  inline static bool check(void* p)
  {
	  __try{
            return HeapValidate(Heap::heap(),0,p);
	  }
	  __except(EXCEPTION_EXECUTE_HANDLER){}
	  return false;
  }
};


template <bool mcheck=true>
struct heap_mem_global_set_check
{
    //typedef   heap_allocator<void*,sys_heap_mem_base<HEAP_NO_SERIALIZE,sys_heap<HEAP_NO_SERIALIZE> > >  inner_allocator_t;
	//
	typedef   sys_heap_mem_base<HEAP_ZERO_MEMORY,sys_heap<HEAP_ZERO_MEMORY> > inner_hmb_t; 
	//typedef   sys_heap_mem_base<HEAP_ZERO_MEMORY> inner_hmb_t; 
	typedef   heap_allocator<void*,inner_hmb_t>  inner_allocator_t;
	//heap_allocator<T, sys_heap_mem<flags,sys_heap<flags> > >
	//typedef std::set<void*,std::less<void*>,inner_allocator_t> heap_set_t;
	//
	  typedef std::set<void*,std::less<void*>> heap_set_t;
	struct synchro_set
	{
		struct CS
		{
			CRITICAL_SECTION m_cs;
			CS(){

				//
				InitializeCriticalSection(&m_cs);
				//InitializeCriticalSectionAndSpinCount(&m_cs,512+256);
			}
			~CS()
			{
				DeleteCriticalSection(&m_cs);
			} // on  	  DLL_PROCESS_DETACH
			inline void lock(){ EnterCriticalSection(&m_cs);}
			inline void unlock(){ LeaveCriticalSection(&m_cs);}

		} ;

		template <class CS>
		struct locker_t
		{
			CS& r;
			locker_t(CS& c):r(c)
			{
				r.lock();
			}
			~locker_t()
			{
				r.unlock();
			}
		};
		CS mutex;
       heap_set_t m_set;

inline	   bool check(void* p)
	   {
		   locker_t<CS> lock(mutex);
		   return (m_set.find(p)!=m_set.end());
	   }
inline       bool push(void* p)
	   {
		   if(!p) return false;
		   locker_t<CS> lock(mutex);
		   return m_set.insert(p).second;
	   }

inline	   bool pop(void* p)
	   {
		   locker_t<CS> lock(mutex);
		   heap_set_t::iterator i= m_set.find(p);
		   if(i==m_set.end())
			   return false;
           m_set.erase(i); 
		   return true;
	   }
       inline	   long count()
	   {
		   locker_t<CS> lock(mutex);
		   return m_set.size();
	   }
		
	   inline	   void lock(){mutex.lock();}
	   inline	   void unlock(){ mutex.unlock();}
	};

	

static synchro_set& get_ptr_set()
{
	return class_initializer_T<synchro_set>::get();
}
  

};


template <>
struct heap_mem_global_set_check<false>
{
	
	typedef   sys_heap_mem_base<HEAP_ZERO_MEMORY,sys_heap<HEAP_ZERO_MEMORY> > inner_hmb_t; 
	typedef   heap_allocator<void*,inner_hmb_t>  inner_allocator_t;
	//typedef std::set<void*,std::less<void*>,inner_allocator_t> heap_set_t;
	

	struct synchro_set
	{
		template <class CS>
		struct locker_t
		{
		
			locker_t(CS& c)
			{
				
			}
			~locker_t()
			{
				
			}
		};
		
		inline	   void lock(){};
		inline	   void unlock(){};
         
		inline	   bool check(void* p)
		{
			return true;
		}
		inline       bool push(void* p)
		{
			return true;
		}

		inline	   bool pop(void* p)
		{
			return true;
		}
		inline	   long count()
		{
				return -1;
		}

	};



	static synchro_set& get_ptr_set()
	{
		return class_initializer_T<synchro_set>::get();
	}


};



//typedef heap_mem_global_set_check<false> heap_mem_global_set;
//
//

typedef heap_mem_global_set_check<CHECK_HEAP> heap_mem_global_set;


//template <long flags=HEAP_ZERO_MEMORY,class Heap=process_heap,bool fcheck=CHECK_HEAP>
//struct sys_heap_mem:sys_heap_mem_base<(fcheck)?flags|HEAP_NO_SERIALIZE:flags,Heap>
template <long flags=HEAP_ZERO_MEMORY,class Heap=process_heap,bool fcheck=CHECK_HEAP>
struct sys_heap_mem:sys_heap_mem_base<flags,Heap>
{

	typedef typename heap_mem_global_set_check<fcheck> heap_mem_global_set;
	typedef typename Heap heap_t;

    inline static HANDLE heap()
	{
		return heap_t::heap();
	}

	inline static void* allocate(size_t cb)
	{
		heap_mem_global_set::synchro_set& ss= heap_mem_global_set::get_ptr_set();
		heap_mem_global_set::synchro_set::locker_t<heap_mem_global_set::synchro_set> lock(ss); 

		HANDLE h=heap();

		void* p=::HeapAlloc(h,flags_a,cb);
		//heap_mem_global_set::get_ptr_set().push(p);
		 ss.push(p);
		return p;
	};
	inline static bool deallocate(void* p)
	{
		bool f;
		heap_mem_global_set::synchro_set& ss= heap_mem_global_set::get_ptr_set();
		heap_mem_global_set::synchro_set::locker_t<heap_mem_global_set::synchro_set> lock(ss); 
         HANDLE h=heap();
		//heap_mem_global_set::get_ptr_set().pop(p);
		f=ss.pop(p);
		return   (f)?::HeapFree(h,flags_f,p):0;
	};
	inline static bool check(void* p)
	{
		return heap_mem_global_set::get_ptr_set().check(p);
	}

};


//
/*
template<> class  tss_allocator<void>
{	// generic allocator for type void
public:
	typedef void _Ty;
	typedef _Ty  *pointer;
	typedef const _Ty  *const_pointer;
	typedef _Ty value_type;

	template<class _Other>
	struct rebind
	{	// convert an allocator<void> to an allocator <_Other>
		typedef tss_allocator<_Other> other;
	};

	tss_allocator() throw ()
	{	// construct default allocator (do nothing)
	}

	tss_allocator(const tss_allocator<_Ty>&) throw ()
	{	// construct by copying (do nothing)
	}

	template<class _Other>
	tss_allocator(const tss_allocator<_Other>&)  throw ()
	{	// construct from related allocator (do nothing)
	}

	template<class _Other>
	tss_allocator<_Ty>& operator=(const tss_allocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}
};
*/