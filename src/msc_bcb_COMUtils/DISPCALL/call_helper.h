#ifndef __SMART_CALLERSdd__H
#define __SMART_CALLERSdd__H
#include "argsxx.h"
#include "ipc.h"
#include "stdarg.h"
#include "tls_dll.h"
#include "tls_dllex.h"
#include <static_constructors_0.h>

#include <typeinfo.h>

#include "os_utils.h"


#define ABSTRACT =NULL
#define OVERRIDE_OP_EQU_EX(_Class,_BaseClass,arg,override_feature) \
	template <class A>\
	inline _Class& operator =(A& arg)\
 {\
 override_feature;\
 return  *this; }\
 template <class A>\
 inline _Class& operator =(A const& arg)\
 {\
 override_feature;\
 return  *this;  }

#define OVERRIDE_OP_EQU(_Class,_BaseClass) \
	OVERRIDE_OP_EQU_EX(_Class,_BaseClass,__arg_000137,*((_BaseClass*)this)=__arg_000137 )







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
//
#define   _ctdbg 

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
	inline operator N()	{return N();}
};








struct caller_friend_base
{
};



template <class RetType,class ArgType=int,bool fargn=false,bool fptr=true>
struct wild_caller_base 
	:public caller_friend_base
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

	explicit  wild_caller_base(void* ptr=NULL)
	{
		m_ptr=ptr;
	}


};



template <class RetType>
struct helper_constructor
{

	inline operator  RetType()
	{
		return RetType();
	}
};




template <class RetType,class ArgType=int>
struct invoker_test
{
inline	RetType  operator()(int argc,ArgType* pargv)
	{

         ///.................
		return RetType();
	}
};





struct __df_wild_caller_tss {};

typedef   wild_tls<0,__df_wild_caller_tss>    TSS_this_dflt;
typedef   wild_tls<1,__df_wild_caller_tss>    TSS_argc_dflt;
typedef   wild_tls<2,__df_wild_caller_tss>    TSS_errno_dflt;




template <class RetType,class ArgType=int,class TSS_this=TSS_this_dflt>
class tss_wild_caller_base: public wild_caller_base<RetType,ArgType,true,false>
{
public:

	//typedef typename InvokeType invoke_type;
	inline size_t argc()
	{
		return m_argc;
	};
	tss_wild_caller_base(){}
protected:

	virtual RetType inner_invoke(size_t argc,ArgType* plist) ABSTRACT;
	//{      return RetType();	};

	static RetType  s_prepare_invoke(int plist)
	{
         tss_wild_caller_base* _this=TSS_this();   
		return 	_this->inner_invoke(_this->m_argc,(ArgType*)&plist);   
	}
	virtual void* inner_caller(long narg)
	{ 

		TSS_this()=this;
		return &s_prepare_invoke;
	};

};

template <class RetType,class ArgType=int,class InvokeType=invoker_test<RetType,ArgType>,class TSS_this=TSS_this_dflt>
class tss_wild_caller: public tss_wild_caller_base<RetType,ArgType,TSS_this>
{
  public:

	//typedef typename InvokeType invoke_type;

	InvokeType* m_prepare_invoke;
	InvokeType inner_prepare_invoke;

	tss_wild_caller(InvokeType* pinvoke=NULL)
	{
		m_prepare_invoke=(pinvoke)? pinvoke:&inner_prepare_invoke;
	}
protected:

	virtual RetType inner_invoke(size_t argc,ArgType* plist)
	{
		return (*m_prepare_invoke)(argc,(ArgType*)((argc>0)? plist:NULL ));
	}

	inline void set_prepare_invoke(InvokeType* pinvoke)
	{
		m_prepare_invoke=pinvoke;
	}


};



/*
template <class RetType,class ArgType=int,class InvokeType=invoker_test<RetType,ArgType>,class TSS_this=TSS_this_dflt>
class tss_wild_caller: public wild_caller_base<RetType,ArgType,true,false>
{
public:

	//typedef typename InvokeType invoke_type;

	InvokeType* m_prepare_invoke;
	InvokeType inner_prepare_invoke;


inline size_t argc()
{
	return m_argc;
}
	tss_wild_caller(InvokeType* pinvoke=NULL)
	{
		m_prepare_invoke=(pinvoke)? pinvoke:&inner_prepare_invoke;
	}
protected:

virtual RetType inner_invoke(ArgType* plist)
{
	return (*m_prepare_invoke)(m_argc,(ArgType*)((m_argc>0)? plist:NULL ));
}

inline void set_prepare_invoke(InvokeType* pinvoke)
{
   m_prepare_invoke=pinvoke;
}

static RetType  s_prepare_invoke(int plist)
{
  	return 	((tss_wild_caller*)TSS_this())->inner_invoke((ArgType*)&plist);   
}
virtual void* inner_caller(long narg)
{ 

	TSS_this()=this;
	return &s_prepare_invoke;
};

};
*/




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
struct  wild_caller:public  wild_caller_base<RetType,ArgType,fargn,fptr>
{
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