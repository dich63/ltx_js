#pragma once
#include "../pp/static_constructors_0.h"
#include <windows.h>
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
}

template <class tls_class>
struct stack_tls_holder
{
	void *m_ptr;
   stack_tls_holder(void *p)
   {
     m_ptr=tls_class();
      tls_class()=p;
   }
 ~stack_tls_holder()
 {
      tls_class()=m_ptr;
 }
 inline tls_class get_current(){return tls_class();}; 
};