#ifndef __SMART_CALLERS__H
#define __SMART_CALLERS__H
#include "argsxx.h"
#include <malloc.h>
#include <static_constructors_0.h>
#include <tls_dllex.h>
#include <autocloser.h>

//struct __tls_{};



template <typename N, N inival=N(),N finival=inival,bool fd=false>
struct Delegator
{
	N volatile m_delegate;
	Delegator():m_delegate(inival){};

	Delegator(Delegator const &s )
	{
		m_delegate=s.m_delegate;
		const_cast<Delegator*>(&s)->m_delegate=finival;
	};

template<bool f>
inline void _dfree(){};

template<>
inline void _dfree<true>()
{
	if(m_delegate)
	delete m_delegate;
};


   ~Delegator()
   {
	  _dfree<fd>();
   }
	inline Delegator& operator  =(Delegator s)
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

 inline N volatile & operator ->()
 {
     return m_delegate;
 }

};






template <class C>
class this_storage  
{
public:

inline void save_this()
{
  tls_this()=(C*)this; 
}
static inline C*& tls_this()
{
	return thread_singleton_varT<C*,this_storage<C>>();
}

};






//================================================================================
#undef PROC_TYPE_OPERATOR_DECLARE
#define PROC_TYPE_OPERATOR_DECLARE(n,m) \
	typedef  RetType  (__cdecl   *PINVOKE_PROC_TYPE##n##m)(ARGS_Z( ArgType A_,n,m) ); \
inline operator PINVOKE_PROC_TYPE##n##m()\
{\
      save_this();\
    argc=(long) CARGS(n,m);\
  return PINVOKE_PROC_TYPE##n##m(&s_invoke_proc);\
}  ;





template <class T,class ArgType=int,class RetType=int>
class call_helper:public this_storage<call_helper<T,ArgType,RetType> >
{  
public:

	_DECLARE_METHODS_ARGS_OF(64,PROC_TYPE_OPERATOR_DECLARE);

	
	call_helper(T* pt=NULL):m_prepare_invoke(pt){};
protected:
	long argc;
	T* m_prepare_invoke;   

inline RetType invoke_proc(ArgType* pfirst)
{
   //return (*m_prepare_invoke)(argc,(argc)?(pfirst):NULL); 
     T& rpi=*m_prepare_invoke;
     //RetType r= (rpi)(argc,(argc)?(pfirst):NULL);
	 //RetType r;
		 //=  
	return get_prepare_invoke()->do_prepare(argc,(argc)?(pfirst):NULL); 
}

static 	RetType __cdecl s_invoke_proc(ArgType first)
	{   
     	return tls_this()->invoke_proc(&first);
	};

inline T* set_prepare_invoke(T* pi)
{
	return m_prepare_invoke=pi;
}


public:

virtual T* get_prepare_invoke()
{
	return m_prepare_invoke;
}
};



#endif