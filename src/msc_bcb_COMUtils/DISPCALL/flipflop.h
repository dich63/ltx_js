#pragma once
#include <windows.h>
#include <memory>
#include <call_helper.h>

template<typename N,class Allocator>
inline N* allocate(size_t n=1)
{
   //typedef typename Allocator::bind<N>::other allocatorN;
	return Allocator::rebind<N>::other().allocate(n);
}

template<typename N,class Allocator>
inline void deallocate(void *p)
{
	//typedef typename Allocator::bind<N>::other allocatorN;
	Allocator::rebind<N>::other().deallocate((N*)p,sizeof(N));
	
}

template<typename N,class Allocator>
inline void free(void *p)
{
	//typedef typename Allocator::bind<N>::other allocatorN;
    Allocator::rebind<N>::other a;
    a.destroy((N*)p);
	a.deallocate((N*)p,sizeof(N));

}


template<void *proc>
struct cd_free
{
	typedef void( cdecl *proc_sc)(void*);
	inline static  void do_proc(void*p)
	{
		if(proc) (proc_sc(proc))(p);
	}

	inline void operator ()(void *p ){do_proc(p);}

};

template<void *proc>
struct std_free
{
	typedef void( _stdcall *proc_sc)(void*);
	inline static  void do_proc(void*p)
	{
		if(proc) (proc_sc(proc))(p);
	}
	inline void operator ()(void *p ){do_proc(p);}

};


template<void *proc,bool fff=true>
struct std_free_if
{
	bool m_f;
	std_free_if(bool f=fff):m_f(f){};
	typedef void( _stdcall *proc_sc)(void*);
	inline void operator ()(void *p )
	{
		if(m_f&&proc) (proc_sc(proc))(p);
	}

};



template<void *proc,DWORD_PTR dwp1,DWORD_PTR dwp2>
struct std_free2
{
	typedef void( _stdcall *proc_sc)(void*,DWORD_PTR ,DWORD_PTR);
	inline static  void do_proc(void*p,DWORD_PTR dw1=dwp1,DWORD_PTR dw2=dwp2)
	{
		if(proc) (proc_sc(proc))(p,dw1,dw2);
	}
	inline void operator ()(void *p ){do_proc(p);}
};

template <typename N  >
struct no_free
{
	inline static  void do_proc(N p){};
	inline void operator ()(N p){}
};

template <typename N>
struct o_free
{
	inline static  void do_proc(void*p){ if(p) delete (N)p;};
	inline void operator ()(void *p ){do_proc(p);}
};

template <typename N,typename Allocator=std::allocator<N> >
struct a_free
{
	//typedef typename Allocator::rebind<N>::other allocatorN;
	inline static  void do_proc(void*p)
	{ if(p)
	{
      free<N,Allocator>(p);  
     /*
	  //delete (N)p;
    
      allocatorN a;
	  a.destroy((N*)p);
	  a.deallocate((N*)p,sizeof(N));
	  */
	}
	};
	inline void operator ()(void *p ){do_proc(p);}
};



struct holder_struct_base
{
	virtual ~holder_struct_base(){};
};
template <typename T>
struct holder_struct:  public  holder_struct_base
{
	//T volatile m_delegate;
	  T  m_delegate;
	//virtual ~reset_holder_cond();
	operator T const &()
	{
		return *(const_cast<T*>(&m_delegate));
	}
	inline T* ptr()
	{
		return &m_delegate;
	}
};

template <typename T,typename B=T>
struct RefCast
{  
	T &m_r;
	RefCast(T const & t ):m_r()
inline 	operator B()  
	{
		return B(m_r);
	}
};
template <typename T,typename B=T*>
struct PtrCast
{  
	T const &m_r;
	PtrCast(T const & t ):m_r(t){}
	inline 	operator B()  
	{
		return B(&m_r);
	}

	
};



template <typename T,typename deleter=no_free<T*>,class _Cast=PtrCast<T> >
struct  holder_struct_cond : public holder_struct<T>
{
  holder_struct_cond(T const& t=T())
  {
	  m_delegate=t;
  };

  virtual ~holder_struct_cond()
  {
	  //_Cast cc(m_delegate);
	  //T* p=cc;
    deleter()(_Cast(m_delegate));  
  };
  
};



template <typename N,typename reset>
struct reset_holder
{
	N volatile m_delegate;
	reset_holder(N d):m_delegate(d){};
inline void clear()
{
reset()(m_delegate);
}
	~reset_holder()
	{
		clear();
	}
};




//template <typename N, N inival=N(),N finival=N(),void(* reset)(void*)=NULL,ff_cc::callconv fcallconv=ff_cc::cc>
template <typename N, N inival=N(),N finival=N(),class reset=no_free<N> >
struct flipflop: public reset_holder<N,reset>
{
	//N volatile m_delegate;
	flipflop(N n=inival):reset_holder<N,reset>(n){};

	flipflop(flipflop const &s ):reset_holder<N,reset>(inival)
	{
		//clear();
		m_delegate=s.m_delegate;
		const_cast<flipflop*>(&s)->m_delegate=finival;
	};
	
	inline flipflop& operator  =(flipflop s)
	{
         clear();
		m_delegate=s.m_delegate;
		s.m_delegate=finival;
		return *this;
	}

	inline operator N volatile&() // const
	{
		return m_delegate;
	}

//
	/*
	inline void operator  =(N n)
	{
		m_delegate=n;
	}
//*/

	inline bool operator !()
	{
		return  finival==m_delegate;
	}
	/*
	inline N volatile& operator ->()
	{
	return m_delegate;
	}
	*/
};


__declspec(selectany) long grefcount=0; 

template <typename PN, class Deleter>
struct flipflop_interceptor_fake_dbg
{


	struct fake_create
	{
		fake_create(PN const& pn)
		{
			if(pn){
				++grefcount;
				_ctdbg(" ffAddRef=%d\n",grefcount);
			}
		};
	};

	struct fake_free
	{
		inline void operator ()(PN pn )
		{ 
			Deleter()(pn) ;
			if(pn){
				--grefcount;
				_ctdbg(" ffRelease=%d\n",grefcount);
			}
		}
	};

	typedef  typename fake_create  assigner;
	typedef  typename fake_free  deleter;
	//	typedef  typename Deleter  deleter;
};



template<long rc>
struct RefCount_fake
{
inline 	long inc(){return rc;}
inline 	long dec(){return rc;}
};


template <typename PN, class Deleter,class RefCount>
struct flipflop_interceptor_rc
{
	struct _create
	{
		_create(PN const& pn)
		{
			if(pn) RefCount().inc();
		};
	};
	struct _free
	{
		inline void operator ()(PN pn )
		{ 
			Deleter()(pn) ;
			if(pn) RefCount().dec();
				  
		}
	};
	typedef  typename _create  assigner;
	typedef  typename  _free  deleter;
};




template <typename PN, class Deleter>
struct flipflop_interceptor_fake
{
	struct fake_create
	{
		fake_create(PN const& )
		{
		};
	};
	typedef  typename fake_create  assigner;
	typedef  typename Deleter  deleter;
};

#define FF_INTERCEPTOR flipflop_interceptor_fake
//_dbg


template <typename N, class RESET = o_free<N*>,class interceptor=FF_INTERCEPTOR<N*,RESET>>
struct flipflop_ptr: public flipflop<N*,(N*)(NULL),(N*)(NULL),typename interceptor::deleter>
{
	typedef  typename N* ptr_type;
	typedef  typename N  value_type;

	flipflop_ptr(N* pn=NULL):flipflop(pn)
	{
		void (interceptor::assigner(pn)); 
	};    

	inline N* operator ->()
	{
		return m_delegate;
	}
    inline void reset(N* p=NULL)
	{
       if(m_delegate!=p)
	   {
         //RESET()(m_delegate);
		   clear();
	      void (interceptor::assigner(p)); 
          m_delegate=p;

	   }
	}

	/*
	inline  operator N* ()
	{
		return const_cast<N*>(m_delegate);
	}


	N const & operator*() const 
	{
        return *m_delegate;
	}
	*/
};

template <typename N,class RESET = o_free<N*>,class RefCounter=RefCount_fake<0> >
struct flipflop_ptr_rc: public flipflop_ptr<N,RESET,flipflop_interceptor_rc<N*,RESET,RefCounter> > 
{
	flipflop_ptr_rc(N* pn=NULL):flipflop_ptr(pn){};
};
	

