#ifndef __SMART_CALLERS__H
#define __SMART_CALLERS__H
#include "argsxx.h"
#include <malloc.h>
#include <static_constructors_0.h>
#include <tls_dll.h>

/*

#pragma warning(push)
#pragma warning(disable : 4793)
void __cdecl operator()(const wchar_t *pszFmt, ...) const
{
va_list ptr; va_start(ptr, pszFmt);
ATL::CTrace::s_trace.TraceV(m_pszFileName, m_nLineNo, atlTraceGeneral, 0, pszFmt, ptr);
va_end(ptr);
}
#pragma warning(pop)
*/

/*
template < class T >
class s_array
{ 
  public:
	T* pt;
s_array():pt(0);
s_array(T* p,n)
{

}
s_array(){_freea(pt);};
};
*/

template < class T >
inline void mirror_permutation(T* p,long n)
{
	T* tail=p+n;

	T* plast=p+(n>>1);
	register T t;
	while(p!=plast)
	{
		t=*(--tail);
		*tail=*p;
		*(p++)=t;
	}
	//n>>=1;

}


class this_storage_base
{

protected:
	static __declspec(thread)    void* g_this;    
};
__declspec(thread,selectany) void* this_storage_base::g_this;//=NULL;

struct __tls_{};

template <class C>
class this_storage  //:protected 
{

protected:
//thread_storage_dll_base<0> m_tls;
typedef struct tls_data
{
	void *pthis;
	long hr;
	wchar_t msg[512]; 
} *ptls_data;

inline static tls_data* tls()
{
	tls_data* p=tls_var<__tls_>();
  if(!p) p =new tls_data;
  return p;
}

inline void save_this()
{
	//g_this=(C*)this;
	//
	//thread_storage_dll_base<0,__tls_>::set_thread_var((C*)this);
	 //tls_var<__tls_>()=(C*)this;
      tls()->pthis=(void*)((C*)this);

	 //int ii=tls_var<__tls_>().tls_index();
	//thread_storage_dll_base<0>::ptr()=(C*)this;
	
}
static inline C* tls_this()
{
	//
     return 	(C*)(tls()->pthis);
	//return (C*)thread_storage_dll_base<0,__tls_>::get_thread_var();
		
}
public:
static inline long tls_index(){ return tls_var<__tls_>().tls_index();}
};




//================================================================================
#undef PROC_TYPE_OPERATOR_DECLARE
#define PROC_TYPE_OPERATOR_DECLARE(n,m) \
	typedef  RetType  (__cdecl   *PINVOKE_PROC_TYPE##n##m)(ARGS_Z( ArgType A_,n,m) ); \
inline operator PINVOKE_PROC_TYPE##n##m()\
{\
      save_this();\
    argc=(long) CARGS(n,m);\
  return PINVOKE_PROC_TYPE##n##m(&_invoke_proc);\
}  ;


template <class T,class ArgType=int,class RetType=int>
class call_helper:public this_storage<call_helper<T,ArgType,RetType> >
{  
public:

	_DECLARE_METHODS_ARGS_OF(64,PROC_TYPE_OPERATOR_DECLARE);



	T& m_t;   
	call_helper(T& _t):m_t(_t){};
protected:
	long argc;

static __declspec(noinline)	RetType __cdecl _invoke_proc(ArgType first)
	{   
		call_helper* _this=tls_this();
		long cb=_this->argc;
		//return _this->m_t.prepare_invoke(cb,(cb)?(&first):NULL);
		return _this->m_t.prepare_invoke(cb,&first);
	};

};



#endif