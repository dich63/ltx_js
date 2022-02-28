#ifndef __SMART_CALLERS2__H
#define __SMART_CALLERS2__H
#include "argsxx.h"
#include "ipc.h"
#include "stdarg.h"
#include "tls_dll.h"
#include "tls_dllex.h"
#include <static_constructors_0.h>

#include <typeinfo.h>

#include "os_utils.h"




#define   __ctdbg_(cl,...) \
{\
	const char* s=typeid(cl).name();\
	_bstr_t bb= bprintf(__VA_ARGS__);\
	bb=_bstr_t(s)+L":"+bb;\
	OutputDebugStringW(bb);\
} 


#define   __ctdbg(...) \
{\
	const char* s=typeid(*this).name();\
	_bstr_t bb= bprintf(__VA_ARGS__);\
	bb=_bstr_t(s)+L":"+bb;\
	OutputDebugStringW(bb);\
} 

#define   _ctdbg    __ctdbg


template <class N, bool f>
struct  member_if
{

	//		typedef    PtrType<N,is_inherited<void*,N>::val>  PtrN;


	N m_v;
	inline  void operator =(N const& v)
	{
		m_v=v;
	}
	inline operator N&()
	{
		return m_v;
	}
	
#if !defined(__BORLANDC__)  // D.. Builder
	N& operator  ->()
	{
	return m_v;
	}
#endif	

};

template <class N>
struct  member_if<N,false>
{

	inline  void operator =(N const& v){};
	//	inline operator N()	{return N();}
};







template <class RetType>
struct call_stack_tuner:public class_initializer<call_stack_tuner<RetType>  >
{



	static	void static_ctor()
	{
		int k=shift_first();
		//printf("type:%s :=%d\n",typeid(RetType).name(),k);   
	};

	static	void static_dtor(){};


inline	static int shift_first()
	{
		static int sf=_shift_first();
		return sf; 
	}

	template<class T>
inline	static void* ardess_first(T** ppt,int fstub=false)
	{
		 int sf=shift_first();
		ppt+=(fstub)? (1+(sf<<1)):(1+sf);
		return ppt;
	};

private:

	// check return value -> register or stack 
	static RetType _cdecl _call_stack_check(void* pret,void* ,int* piout,...)
	{
		*piout=(pret)? 0:1;
		return RetType();
	};

	//typedef   void (__cdecl * _pv)(void* p1,void* p2,int* p3,int* p4);
	typedef   void (__cdecl * _pv)(...);
	static int _shift_first()
	{
		RetType dummy;
		void *p=&_call_stack_check;
		int iout=-1;
		(_pv(p))((void*)&dummy,NULL,&iout,&iout);
		return iout;
	}
};







template <class RetType,class ArgType=int,bool fargn=false,bool fptr=true>
struct wild_caller
{
		
//==========================================================================
#undef PROC_TYPE_OPERATOR_DECLARE
#define PROC_TYPE_OPERATOR_DECLARE(n,m) \
	typedef  RetType  (__cdecl   *PINVOKE_PROC_TYPE##n##m)(ARGS_Z( ArgType A_,n,m) ); \
	inline operator PINVOKE_PROC_TYPE##n##m()\
	{\
	return PINVOKE_PROC_TYPE##n##m( _inner_caller<fptr>(long CARGS(n,m) ));\
}  ;

	_DECLARE_METHODS_ARGS_OF(64,PROC_TYPE_OPERATOR_DECLARE);

//==========================================================================

	member_if<void*,fptr> m_ptr;
	member_if<long,fargn> m_argc;

	template <bool f> 
	inline void* _inner_caller(long narg)
	{
		m_argc=narg;
		return m_ptr;
	};

	template <> 
	inline void* _inner_caller<false>(long narg)
	{
		m_argc=narg;
		//if(fptr)  else
		return inner_caller(narg);
	}

    virtual void* inner_caller(long narg){ return NULL;};

	explicit  wild_caller(void* ptr=NULL)
	{
		m_ptr=ptr;
	}



//=====================================================================
	inline	static int shift_first()
	{
		//static int sf=_shift_first();
		//static 
		int sf=call_stack_tuner<RetType>::shift_first();
		return sf; 
	}

	template<class T>
	inline 	static ArgType* ardess_first(T** ppt,int fstub=false)
	{
		return (ArgType*)call_stack_tuner<RetType>::ardess_first(ppt,fstub);
		/* 
		static int sf=shift_first();
		ppt+=(fstub)? (1+(sf<<1)):(1+sf);
		return ppt;
		*/
	};





	//protected:
/*	
private:

	// check return value -> register or stack 
	static RetType _cdecl _call_stack_tuner(void* pret,void* ,int* piout,...)
	{
		*piout=(pret)? 0:1;
		return RetType();
	};

	//typedef   void (__cdecl * _pv)(void* p1,void* p2,int* p3,int* p4);
	typedef   void (__cdecl * _pv)(...);
	static int _shift_first()
	{
		RetType dummy;
		void *p=&_call_stack_tuner;
		int iout=-1;
		(_pv(p))((void*)&dummy,NULL,&iout,&iout);
		return iout;
	}
*/
};







template <class RetType,class ArgType=int>
struct invoker
{
inline	RetType  operator()(int argc,ArgType* pargv)
	{


		return RetType(argc);
	}
};







struct __df_wild_caller_tss
{};
template <class RetType,class ArgType=int,class InvokeType=invoker<RetType,ArgType>,class TSS_this=__df_wild_caller_tss>
class tss_wild_caller: public wild_caller<RetType,ArgType,true,false>
{
public:
	InvokeType* m_prepare_invoke;
	InvokeType inner_prepare_invoke;

	tss_wild_caller(InvokeType* pinvoke=NULL)
	{
		m_prepare_invoke=(pinvoke)? pinvoke:&inner_prepare_invoke;
	}
protected:

inline RetType inner_invoke(void* plist)
{
	
	return (*m_prepare_invoke)(m_argc,(ArgType*)((m_argc>0)? plist:NULL ));
}

struct fake 
{
void* xx;
fake(tss_wild_caller* pc=NULL)
{
   tls_var<TSS_this>()=pc;
_ctdbg("constuctor\n"); // never ...!!!
 //throw 1;
}

~fake()
{
 _ctdbg("destructor\n");
}

inline  tss_wild_caller*   operator->()
{
  return caller();
}
inline static tss_wild_caller *   caller()
{
	return (tss_wild_caller*)tls_var<TSS_this>();
}
};


static RetType  s_prepare_invoke(fake plist)
{
  //return  plist->inner_invoke(&plist);
	
	  //
return 	((tss_wild_caller*)tls_var<TSS_this>())->inner_invoke(&plist);   
}
virtual void* inner_caller(long narg)
{ 
	//
	tls_var<TSS_this>()=this;
	//fake(this);
	return &s_prepare_invoke;
};

};






template <class RetType,class ArgType=int,class InvokeType=invoker<RetType,ArgType>,class EXECUTE_Allocator=exe_allocator>
class stub_wild_caller: public wild_caller<RetType,ArgType,true,false>
{

public:
	EXECUTE_Allocator allocator;
	pproc_stub_base m_ptr;
	InvokeType* m_prepare_invoke;
	InvokeType inner_prepare_invoke;
	
	typedef stub_wild_caller* const* pcthis_type;
	inline  pproc_stub_base get_stub()
	{
				if(!m_ptr)
		{
			if(shift_first())
				m_ptr= allocator.create<PROC_STUB_STRUCT>()->init(METHOD_PTR(stub_wild_caller,inner_invoke),(void*)this);
			else m_ptr= allocator.create<PROC_STUB>()->init(METHOD_PTR(stub_wild_caller,inner_invoke),(void*)this);

			
		};
		return m_ptr;
	}
	stub_wild_caller(InvokeType* pinvoke=NULL):m_ptr(NULL)
	{
		m_prepare_invoke=(pinvoke)? pinvoke:&inner_prepare_invoke;
	}
	~stub_wild_caller()
	{
		//stub_allocator::free(m_ptr);
		allocator.destroy(m_ptr);
	}

	//inline RetType prepainvoke

   //
template<class T>
	inline ArgType* pfirst(T** ppthis)
	{
       return  (ArgType*) (ppthis+(call_stack_tuner<RetType>::shift_first()<<1));  
	}


	RetType __stdcall inner_invoke()
	{

		//	
		 //void* p= pfirst(&this);
		//		ArgType* plist=(m_argc>0)? ardess_first(&const_cast<stub_wild_caller*>(this),true):NULL;
		//
		ArgType* plist=pfirst(&const_cast<stub_wild_caller*>(this));
		return (*m_prepare_invoke)(m_argc,plist);
		//;
		// ArgType* plist= (ArgType*)ardess_first(&const_cast<stub_wild_caller*>(this),true);
	}

	virtual void* inner_caller(long narg)
	{
		return get_stub();
	};



};





#endif