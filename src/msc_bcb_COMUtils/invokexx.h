#pragma once

#include "argsxx.h"
#include "dispcall/typedef_traits.h"


//template <class RetType,class ArgType=int,bool fargn=false,bool fptr=true>
struct Error_Not_Impl
{
  typedef Error_Not_Impl invoke_helper_type;
  typedef int arg_type;
  typedef int ret_type;

 struct Emps
{
	void* m_pp;
	int m_cb;
	Emps(void*pp,int cb)
	{
     m_pp=pp;
	 m_cb=cb;
	};
  inline ret_type operator()(arg_type* p,...)
  {
	  return m_cb;
  }

};
  typedef Emps invoke_type;

};

template <typename T>
struct auto_typer
{
	typedef typename  T::invoke_helper_type invoke_helper_type;
};




template <typename T,typename Helper= auto_typer<Error_Not_Impl> >
struct invoker
{


	//typedef typename Helper helper_type;
	//
///*
	//typedef typename      Helper::invoke_helper_type _helper_type;
	//
	typedef typename      T::invoke_helper_type _helper_type;
	typedef typename      _helper_type::arg_type   _arg_type;
	typedef  typename     _helper_type::ret_type   _ret_type;
    typedef  typename    _helper_type::invoke_type   _invoke_type;
//*/
	_ret_type test( _arg_type a)
	{
		T* thisT= static_cast<T*>(this);
         return    (_invoke_type(thisT,1))(&a);
	}


};

template<class T>
struct CC
{
	struct _Error_KKKK
	{
		//_Error_KKKK(T*t){};
	
	};
typedef _Error_KKKK NN;

	void dodo()
	{
		T* thisT= static_cast<T*>(this);
		T::NN(thisT).fu();
	}
};


