#pragma once
#include "winconsole.h"
#include "ipc.h"

template <class T>
struct unhandled_ef_base
{
	struct  MSG_ID
	{
		wchar_t* msg;
		DWORD code;

	};

	
	//

inline static wchar_t* get_msg(EXCEPTION_POINTERS& ep)
{
	return  get_msg(ep.ExceptionRecord->ExceptionCode);
}

inline static wchar_t* get_msg(DWORD dwCode )
{

	static  MSG_ID __except_msg[]=
	{
		L"EXCEPTION_ACCESS_VIOLATION",      STATUS_ACCESS_VIOLATION,
		L"EXCEPTION_DATATYPE_MISALIGNMENT", STATUS_DATATYPE_MISALIGNMENT,
		L"EXCEPTION_BREAKPOINT",            STATUS_BREAKPOINT,
		L"EXCEPTION_SINGLE_STEP",           STATUS_SINGLE_STEP    ,
		L"EXCEPTION_STACK_OVERFLOW",STATUS_STACK_OVERFLOW  ,
		L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED", STATUS_ARRAY_BOUNDS_EXCEEDED,
		L"EXCEPTION_FLT_DENORMAL_OPERAND",  STATUS_FLOAT_DENORMAL_OPERAND,
		L"EXCEPTION_FLT_DIVIDE_BY_ZERO",    STATUS_FLOAT_DIVIDE_BY_ZERO ,
		L"EXCEPTION_FLT_INEXACT_RESULT",    STATUS_FLOAT_INEXACT_RESULT,
		L"EXCEPTION_FLT_INVALID_OPERATION", STATUS_FLOAT_INVALID_OPERATION,
		L"EXCEPTION_FLT_OVERFLOW",          STATUS_FLOAT_OVERFLOW,
		L"EXCEPTION_FLT_STACK_CHECK",       STATUS_FLOAT_STACK_CHECK,
		L"EXCEPTION_FLT_UNDERFLOW",         STATUS_FLOAT_UNDERFLOW,
		L"EXCEPTION_INT_DIVIDE_BY_ZERO",    STATUS_INTEGER_DIVIDE_BY_ZERO,
		L"EXCEPTION_INT_OVERFLOW",          STATUS_INTEGER_OVERFLOW,
		L"EXCEPTION_PRIV_INSTRUCTION",      STATUS_PRIVILEGED_INSTRUCTION,
		L"EXCEPTION_IN_PAGE_ERROR",         STATUS_IN_PAGE_ERROR,
		L"MS Visual C++  EXCEPTION",        0xE06D7363,
		L"Delphi,BCB.. EXCEPTION",          0x0EEFFACE 
	};


	int n=sizeof(__except_msg)/sizeof(MSG_ID);
	for(int k=0;k<n;k++)
		if(__except_msg[k].code==dwCode) return __except_msg[k].msg;
	return L"Unknown EXCEPTION..";
}

};

template <class T>
struct unhandled_ef: unhandled_ef_base<T>
{
	proc_stub_allocator psa;
    LPTOP_LEVEL_EXCEPTION_FILTER old_proc;	
	LONG __stdcall unhandled_ExceptionFilter(EXCEPTION_POINTERS& ep)
	{
		LONG	exc_op=EXCEPTION_EXECUTE_HANDLER;
		((T*)this)->on_unhandled_exception(ep,exc_op); 
		return exc_op;
	}

	unhandled_ef()
	{
		old_proc=SetUnhandledExceptionFilter(psa.stub_create(METHOD_PTR(unhandled_ef<T>,unhandled_ExceptionFilter),this)); 
	}
	~unhandled_ef()
	{
		//SetUnhandledExceptionFilter(old_proc); 
		SetUnhandledExceptionFilter(NULL); 
	}

};


template <class T>
struct unhandled_ef_static: unhandled_ef_base<T>
{
	proc_stub_allocator psa;
static	LONG __stdcall unhandled_ExceptionFilter(EXCEPTION_POINTERS& ep)
	{
		LONG	exc_op=EXCEPTION_EXECUTE_HANDLER;
		T::on_unhandled_exception(ep,exc_op); 
		return exc_op;
	}

	unhandled_ef_static()
	{
		SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER(&unhandled_ExceptionFilter)); 
	}
};