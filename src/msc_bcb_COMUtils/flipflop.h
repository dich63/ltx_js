#pragma once
#include <windows.h>

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






template <typename N,N &  _ref,typename reset=o_free<N> >
struct reset_holder_ref
{	
	//inline	reset_holder_ref(N const&  r):_ref(r){};
	inline	~reset_holder_ref()
	{
		reset()(_ref);
	}

};




/*
template <typename N,typename reset>
struct reset_holder2: reset_holder_base<N,reset>
{
N  m_delegate;
reset_holder2():reset_holder_base<N,reset>(m_delegate){};
};
*/






template <typename N,typename reset>
struct reset_holder
{
	N volatile m_delegate;
	reset_holder(N d):m_delegate(d){};
	~reset_holder()
	{
		reset()(m_delegate);
	}
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
struct reset_holder_cond //:public reset_holder_cond_T<N>
{
	N volatile m_delegate;
	reset m_r;

	reset_holder_cond(reset const&  r=reset(),N const& d=N())
	{
	  init(r,d);
	};

inline reset_holder_cond const & init(reset const&  r=reset(),N const& d=N())
{
	m_r=r; 
	*(const_cast<N*>(&m_delegate))=d;
	return *this;
}

	~reset_holder_cond()
	{
		m_r(const_cast<N*>(&m_delegate));
	};

inline  reset & deleter()
{
  return m_r;
}

};




template <typename N,typename reset=no_free<N>>
struct flipflop_struct
{
	N volatile m_delegate;
	flipflop_struct(N d=N()):m_delegate(d){};
	~flipflop_struct()
	{
		reset()((void*)&m_delegate);
	}
	flipflop_struct(flipflop_struct const &s )
	{
		m_delegate=s.m_delegate;
		const_cast<flipflop_struct*>(&s)->m_delegate=N();
	};

	inline flipflop_struct& operator  =(flipflop_struct s)
	{
		m_delegate=s.m_delegate;
		s.m_delegate=N();
		return *this;
	}

	inline operator N volatile&() // const
	{
		return m_delegate;
	}

	inline operator N*() // const
	{
		return &m_delegate;
	}

	inline void operator  =(N n)
	{
		m_delegate=n;
	}

};





//template <typename N, N inival=N(),N finival=N(),void(* reset)(void*)=NULL,ff_cc::callconv fcallconv=ff_cc::cc>
template <typename N, N inival=N(),N finival=N(),class reset=no_free<N> >
struct flipflop:reset_holder<N,reset>
{
	//N volatile m_delegate;
	flipflop(N n=inival):reset_holder<N,reset>(n){};

	flipflop(flipflop const &s ):reset_holder<N,reset>(inival)
	{
		m_delegate=s.m_delegate;
		const_cast<flipflop*>(&s)->m_delegate=finival;
	};
	
	inline flipflop& operator  =(flipflop s)
	{
		m_delegate=s.m_delegate;
		s.m_delegate=finival;
		return *this;
	}

	inline operator N volatile&() // const
	{
		return m_delegate;
	}

	inline void operator  =(N n)
	{
		m_delegate=n;
	}


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

template <typename N, class RESET = o_free<N*> >
struct flipflop_ptr: public flipflop<N*,(N*)(NULL),(N*)(NULL),RESET>
{
	flipflop_ptr(N* n=NULL):flipflop(n){};    

	inline N* operator ->() const
	{
		return m_delegate;
	}
    inline void reset(N* p=NULL)
	{
       if(m_delegate!=p)
	   {
         RESET()(m_delegate);
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

