#pragma once

#include "argsxx.h"
#include "copier.h"
#include <stdarg.h>

#define pargs_of(p,ofs) pargv(&p,ofs)
//#define pargv  refref<arg_type>




template <typename T,typename A,typename R=A,bool cstatic=false>
struct invoke_helper_base
{
  typedef typename A arg_type;
  typedef typename R ret_type;
  typedef typename T owner_type;
  typedef typename refref<A> pargv;
	int m_argc;
    T* m_p; 
 invoke_helper_base(T* p,int c):m_argc(c),m_p(p){};

 inline int count()
   {   return    m_argc; }
 inline T& owner()
   {  return    *m_p;}
};

template <typename T,typename A,typename R>
struct invoke_helper_base<T,A,R,true>
{
	typedef typename A arg_type;
	typedef typename R ret_type;
	typedef typename T owner_type;
	typedef typename refref<A> pargv;
	
	T* m_p; 
	invoke_helper_base(T* p,int c):m_p(p){};
    invoke_helper_base(T* p):m_p(p){};
	
	inline T& owner()
	{  return    *m_p;}
};



#define INVOKE_DECLARE(invoke_proc)\
	typedef	struct __invoke_helper_tag_0000:_helper_base_type {\
	__invoke_helper_tag_0000(owner_type* t,int cb):_helper_base_type(t,cb){};\
	inline    ret_type  operator()(arg_type* t,...){\
	return owner().##invoke_proc##(count(),&t);   }\
	inline    ret_type  operator()(){\
	return owner().##invoke_proc##(0);   }\
} invoke_helper_type;

#define INVOKE_DECLARE_0(invoke_proc)\
	typedef	struct __invoke_helper_tag_00001:_helper_base_type {\
	__invoke_helper_tag_00001(owner_type* t,int cb):_helper_base_type(t,cb){};\
	inline    ret_type  operator()(_arg_type0* pt,...){\
	return owner().##invoke_proc##(*pt,count(),((arg_type**)&pt)+1);}\
} invoke_helper_type;


#define INVOKE_STACK_DECLARE(invoke_proc,buffer_type)\
	template <int c>\
	struct invoke_helper_stack:_helper_base_type_stack{\
	invoke_helper_stack(owner_type* t,int cb):_helper_base_type_stack(t){};\
	static inline const int count()  {return c;};\
	inline    _ret_type  operator()(_arg_type* pt,...){\
	buffer_type __buf_[c+1];\
	return owner().invoke_proc(c,&pt,__buf_); }\
	inline    _ret_type  operator()(){\
	buffer_type __buf_[1];\
	return owner().invoke_proc(0,(_arg_type**)0,__buf_);}\
};

#define INVOKE_STACK_DECLARE_STRUCT(invoke_proc,buffer_struct)\
	template <int c>\
    struct invoke_helper_stack:_helper_base_type_stack{\
	invoke_helper_stack(owner_type* t,int cb):_helper_base_type_stack(t){};\
	static inline const int count()  {return c;};\
	inline    ret_type  operator()(_arg_type* pt,...){\
    buffer_struct<c> __buf_;\
	return owner().invoke_proc(c,&pt,&__buf_);}\
	inline    ret_type  operator()(){\
	buffer_struct<0> __buf_;\
	return owner().invoke_proc(0,(_arg_type**)0,&__buf_);}\
};

//stack_buff<c> buf;\ 


template <typename T,typename A,typename R,bool cstatic=false>
struct auto_definer
{

	typedef typename  A arg_type;
	typedef typename  R ret_type;
	template <typename S,int c>
	struct def 
	{
		typedef typename  S::invoke_helper_type invoke_helper_type;
        typedef typename  S*   class_ptr_type;
	};

};

template <typename T,typename A,typename R>
struct auto_definer<T,A,R,true>
{

	typedef typename  A arg_type;
	typedef typename  R ret_type;

	template <typename S,int c>
	struct def 
	{
		typedef typename  S::invoke_helper_stack<c> invoke_helper_type;
		typedef typename  S*   class_ptr_type;
	};
	
};





template <typename T,typename A0,typename A,typename R,bool cstatic=false>
struct auto_definer_0:auto_definer<T,A,R,cstatic>
{
  typedef typename  A0 arg_type0;
};

template <typename T,typename Definer>
struct invoke_base_XX
{
	typedef typename Definer  definer_type;
	typedef typename Definer::arg_type  _arg_type;
	typedef typename Definer::ret_type  _ret_type;
	
	typedef typename  invoke_helper_base<T,_arg_type,_ret_type>  _helper_base_type;
	typedef typename  invoke_helper_base<T,_arg_type,_ret_type,true>  _helper_base_type_stack;

	typedef typename  _helper_base_type::pargv    _pargv;
};

//#undef _OPERATOR_DECLARE_MACRO_XX_TEMP

# undef TMP_F07FA8EA478F7FDA03DFA69E3824099A
#define TMP_F07FA8EA478F7FDA03DFA69E3824099A(n,m) \
inline	_ret_type operator() (ARGS_Z( _arg_type A_,n,m) ) \
{   const int c=CARGS(n,m);\
	typedef  Definer::def<T,c>::invoke_helper_type helper;\
    typedef  Definer::def<T,c>::class_ptr_type Ptr;\
    Ptr pt=  static_cast<Ptr>(this);\
	return helper(pt, c)(ARGS_Z(&A_,n,m));\
}

template <typename T,typename Definer,bool withtxt=false>
struct invoke_base:invoke_base_XX<T,Definer>
{
   _DECLARE_METHODS_ARGS_OF(64,TMP_F07FA8EA478F7FDA03DFA69E3824099A);
};

# undef TMP_F07FA8EA478F7FDA03DFA69E3824099A
#define TMP_F07FA8EA478F7FDA03DFA69E3824099A(n,m) \
	inline	_ret_type operator() ( _arg_type0 txt COMMA_IF(n,m) ARGS_Z( _arg_type A_,n,m) ) \
{\
	const int c=CARGS(n,m);\
	typedef  Definer::def<T,c>::invoke_helper_type helper;\
	typedef  Definer::def<T,c>::class_ptr_type Ptr;\
	Ptr pt=  static_cast<Ptr>(this);\
	return helper(pt,c)(   &txt COMMA_IF(n,m)    ARGS_Z(&A_,n,m));\
}

template <typename T,typename Definer>
struct invoke_base<T,Definer,true>:invoke_base_XX<T,Definer>
{
	typedef typename Definer::arg_type0  _arg_type0;
	_DECLARE_METHODS_ARGS_OF(64,TMP_F07FA8EA478F7FDA03DFA69E3824099A);
};
# undef TMP_F07FA8EA478F7FDA03DFA69E3824099A



template <typename T,typename ArgType,typename RetType=ArgType,bool cstatic=false>
struct   invoke_with_helper:invoke_base<T,auto_definer<T,ArgType,RetType,cstatic>>
{
};

template <typename T,typename ArgType0,typename ArgType,typename RetType=ArgType>
struct   invoke_with_helper_0:invoke_base<T,auto_definer_0<T,ArgType0,ArgType,RetType>,1>
{
};



/*
// EXAMPLES .....
struct AAtxt:invoke_base<AAtxt,auto_definer_0<AAtxt,char*,double,int>,1>
{
	struct invoke_helper:invoke_helper_base<AAtxt,double,int>
	{
	
		invoke_helper(AAtxt* t,int cb):invoke_helper_base<AAtxt,double,int>(t,cb){};
		inline    ret_type  operator()(char** txt,...)
		{
			//refref<int>
			va_list argptr;
			va_start(argptr,txt);

			cargv r((arg_type**)argptr);
			printf("%s :%d]",*txt,count());
			for(int i=0;i<count();i++)
				printf(" %g",r[i]);
			printf("\n");

			return  count();
		}


	};

	typedef invoke_helper invoke_helper_type;
};


struct AA:invoke_with_helper<AA,double,int> //struct AA:invoke_base<AA,auto_definer<AA,double,int> >
{

typedef	struct invoke_helper:invoke_helper_base<AA,double,int>
	{
		invoke_helper(AA* t,int cb):invoke_helper_base<AA,double,int>(t,cb){};
		inline    ret_type  operator()(arg_type* t,...)
		{

			cargv r(&t);
			printf(" %d]",count());
			for(int i=0;i<count();i++)
				printf(" %g",r[i]);
             printf("\n");

			return  count();
		}

		inline    ret_type  operator()()
		{
			return  count();
		}


	} invoke_helper_type;

//typedef invoke_helper invoke_helper_type;


};


int _tmain(int argc, _TCHAR* argv[])
{
	AA ab;
//	va_list

ab();
ab(1,2,3);
ab(1,2,3,5,6,7,7,888,990);


AAtxt abt;
	//aasa.dodo();
//bool f=aasa.morda();
abt("pypa");
abt("pypa",1,2,3);
abt("pypa",1,2,3,5,6,7,7,888,990);


	return 0;
}
*/
