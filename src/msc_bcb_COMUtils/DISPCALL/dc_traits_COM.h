#pragma once

#include "dc_traits.h"
#include <comdef.h>
#include "tls_dllex.h"


#define  _VA(a) (_variant_t(a).Detach())

template <typename ArgInHolder,int  mode>
struct call_helper_error_handler<VARIANT,ArgInHolder,mode>
{
	TSS_errno_dflt hr;
	member_if<long,false> argNu;
};

/*
template <typename ArgInHolder>
struct call_helper_error_handler<VARIANT,ArgInHolder,cheh::tss>
{
	member_if<long,false> hr;
	member_if<long,false> argNu;
};
*/
template <typename ArgInHolder>
struct call_helper_error_handler<VARIANT,ArgInHolder,cheh::except>
{

typedef   typename   RefCount_fake<0>	refcounter;


	struct error_info
	{
		inline bool success(){ return true;}
		inline bool fail(){ return false;}
		inline void set_defflag(){};
		inline  operator long(){return 0;}     
	};



inline	static error_info  check_error(long hr,void* pweakparams=NULL)
	{
		//get_error_info().scode=hr;
		//_com_issue_error(h);
		if (FAILED(hr)) 
				throw _com_error(hr,(IErrorInfo*)pweakparams,true);
		return error_info();
	}
inline	static error_info  check_error()
	{
		return error_info();
	}
	
};
struct __COM_eh_TSS
{};

template <typename ArgInHolder>
struct call_helper_error_handler<VARIANT,ArgInHolder,cheh::tss>
{

typedef   typename   call_helper_error_handler	refcounter;
   
	struct error_info
	{
		HRESULT scode;
        long refcount;
        long maxrefcount;
		long defflag;
        //_bstr_t msg;
      
		error_info(long hr=0):scode(hr),refcount(0),maxrefcount(0),defflag(0){};
		inline bool success()
		{
			return scode>=0;
		}
		inline bool fail(){ return scode<0;}
		inline  operator long(){return scode;}   

		inline 	long inc()
		{
             reset();
            ++refcount;
			if(maxrefcount<refcount) maxrefcount=refcount;
			return refcount;
		}
		inline 	long dec()
		{
			return --refcount;
		}
     
		inline void set_defflag()
		{
           defflag=1;
		}
        error_info& reset()
		{
			if((defflag==1)||(refcount==0))
			{
				defflag=0;
				//msg.Attach(NULL);
				scode=0;
			} 
			return *this;
		}

   	};


		
	inline static	long inc()
	{
		return get_error_info().inc();
	}
	inline static	long dec()
	{
		return get_error_info().dec();
	}
	

	inline	static error_info&	get_error_info()
	{
		//static static error_info ei=error_info();
		return *((error_info*)thread_singleton_ptr_auto_create<error_info,VARIANT>());
	}

	inline	 static error_info&  check_error(long hr,void* pweakparams=NULL)
	{
		get_error_info().scode=hr;
		return get_error_info();
	}
	inline static	error_info&   check_error()
	{
		return get_error_info().reset();
	}

//
inline static long scode()
{
  return get_error_info().scode;
}
};

#define FCAN_CLEAR 1
struct VARIANTARG_Holder;

struct VARIANTARG_PutTypeConverter
{

	typedef  VARIANTARG_Holder ValuePutType; 

	template <typename A>       
inline	long operator()(A* pa,VARIANTARG_Holder* pval);

};



struct VARIANTARG_Holder
{

typedef call_helper_error_handler<VARIANTARG,VARIANTARG_Holder,cheh::DFLT_CHEH> error_handler;

    VARIANTARG m_v;
	long m_flag;
    flipflop_ptr_rc<char,no_free<char*>,error_handler::refcounter> sc;
	VARIANTARG_Holder():m_flag(FCAN_CLEAR)
{
  m_v=VARIANTARG();
}
template <typename A>
VARIANTARG_Holder( A const& val )
{
  m_flag=FCAN_CLEAR;
  VARIANTARG_PutTypeConverter()(&val,this);
  
}


   ~VARIANTARG_Holder()
   {
	   if((m_v.vt!=VT_EMPTY)&&(FCAN_CLEAR&m_flag) ) ::VariantClear(&m_v);
   }

operator  VARIANTARG&()
{
	return m_v;
}

};



struct VARIANTARG_GetTypeConverter
{

	typedef  VARIANTARG_Holder ValueGetType; 

	template <typename A>       
	inline	long operator()(VARIANTARG_Holder* pval,A* pa)
	{
      //     
		 //A a=A();//=A( _variant_t(*pval->m_v));
		A a=A( _variant_t(pval->m_v));
		*pa=a; 
		return S_OK;
	};

};

template <typename A>       
inline long VARIANTARG_PutTypeConverter::operator()(A* pa,VARIANTARG_Holder* pval)
{
    HRESULT hr=VARIANTARG_Holder::error_handler::check_error();
	if(SUCCEEDED(hr))
	try
	{
        //_variant_t vv(1.e-10,VT_EMPTY);
		 //vv=*pa; 
		pval->m_v=_variant_t(*pa).Detach();
	}
	catch (_com_error& e)
	{
		pval->sc.reset("error++");
	  // VARIANTARG_Holder::error_handler::error_info &ei=VARIANTARG_Holder::error_handler::get_error_info();
	  //  ei.msg=e.Description();
	//	ei.scode=e.Error();
	}
		
	return hr;
};

