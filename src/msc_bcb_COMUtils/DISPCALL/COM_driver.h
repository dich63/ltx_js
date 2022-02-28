#pragma once

#include "dc_traits.h"
#include "dc_traits_COM.h"
#include "dispex.h"
#include "os_utils.h"
#define DISPATCH_GET_ALL (DISPATCH_METHOD|DISPATCH_PROPERTYGET)
#define DISPATCH_PUT_ALL (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF) 
#define DISPATCH_ALL (DISPATCH_GET_ALL|DISPATCH_PUT_ALL) 


#define CHECK_FAIL_(hr,a) if(FAILED(hr=(a))) \
	return hr;
#define CHECK_FAIL_hr(a)  CHECK_FAIL_(hr,a)
#define CHECK_FAIL(a) {HRESULT __hr00;CHECK_FAIL_(__hr00,a);}
#define CHECK_IMPL(p) if(!(p)) return E_NOTIMPL;
#define CHECK_PTR(p) if(!(p)) return E_POINTER;



struct ComDispatchDriver_base
{

	struct ErrorHelper
	{
		EXCEPINFO& m_ei;
		GUID m_iid;
		ErrorHelper(EXCEPINFO& ei,GUID iid=GUID_NULL ):m_ei(ei),m_iid(iid){};

		template <class I>
		inline HRESULT	get_error_info(I** ppIntf)
		{	HRESULT hr;
		if(!ppIntf) return E_POINTER;
		CComPtr<IErrorInfo> ei;
		CHECK_FAIL_hr(get_error_info(&ei));
		hr=ei->QueryInterface(__uuidof(I),(void**)ppIntf);
		return hr;
		}

		inline HRESULT	get_error_info(IErrorInfo** ppIntf)
		{ 
			HRESULT hr;

			if(!ppIntf) return E_POINTER;

			CComPtr<ICreateErrorInfo> cei;
			CHECK_FAIL_hr(CreateErrorInfo(&cei));

			CHECK_FAIL_hr(cei->SetGUID(m_iid));
			CHECK_FAIL_hr(cei->SetSource(m_ei.bstrSource));
			CHECK_FAIL_hr(cei->SetDescription(m_ei.bstrDescription));
			CHECK_FAIL_hr(cei->SetHelpFile(m_ei.bstrHelpFile));
			CHECK_FAIL_hr(cei->SetHelpContext(m_ei.dwHelpContext));
			return hr=cei->QueryInterface(__uuidof(IErrorInfo),(void**)ppIntf);
		}


	};




	CComPtr<IDispatch> m_disp;
    CComPtr<IDispatchEx> m_dispex;
	DISPID m_dispid;
	HRESULT m_hr;
	ComDispatchDriver_base():m_dispid(DISPID_VALUE),m_hr(E_POINTER){};

inline HRESULT invoke_base(WORD flags,DISPPARAMS* pdp,VARIANTARG* presult,IErrorInfo** pperrorinfo)
{
	HRESULT hr,hr0;
   if(!m_disp) return E_NOINTERFACE;
    CComExcepInfo exepinfo;
    hr=m_disp.p->Invoke(m_dispid,IID_NULL,LOCALE_USER_DEFAULT,flags,pdp,presult,&exepinfo,NULL);
	if((DISP_E_EXCEPTION==hr)&&(pperrorinfo))
	{
      hr0=ErrorHelper(exepinfo).get_error_info(pperrorinfo);
	}

   return hr;
}
inline HRESULT   get_id_base(BSTR name,DISPID* pdid)
{
   HRESULT hr;
   if(!m_disp) return E_NOINTERFACE;
   return hr=m_disp.p->GetIDsOfNames(IID_NULL,&name, 1, LOCALE_USER_DEFAULT,pdid);
}

};


struct ComProper
{
   //HRESULT hr; 
  _bstr_t m_name;
  template <typename A>
  ComProper(A const& a)
  {
	  variant_t tmp=a;
      tmp.ChangeType(VT_BSTR);
      m_name=bstr_t(tmp.Detach().bstrVal,false);
  }
ComProper(ComProper const& c)
{
  m_name=c.m_name;
}

  ComProper(){};

inline operator BSTR()
{
 return m_name;
}
};



template <class CollectorDelegator,class PropID=ComProper,
class _ValueGetTypeConverter=VARIANTARG_GetTypeConverter,
class _ValuePutTypeConverter=VARIANTARG_PutTypeConverter>
struct ComDispatchDriver:ComDispatchDriver_base
{


    typedef typename   ComDispatchDriver Driver;   
	typedef typename   CollectorDelegator::error_handler error_handler;
	typedef typename  CollectorDelegator::VHolder ValueType;	
    typedef typename  CollectorDelegator::allocator allocator;	
	typedef typename long FlagType;	
	typedef typename PropID prop_id_type;	

	typedef _ValueGetTypeConverter ValueGetTypeConverter;
	typedef _ValuePutTypeConverter ValuePutTypeConverter;

	typedef typename ValueGetTypeConverter::ValueGetType ValueGetType;
	typedef typename ValuePutTypeConverter::ValuePutType ValuePutType;

	  FlagType m_flags;
	  DWORD  m_grfdex;
   
   ComDispatchDriver(IDispatch *pdisp=NULL):m_grfdex(0),m_flags(0)
   {
	   //fdexNameCaseSensitive
	   m_disp=pdisp;
	   m_dispid=DISPID_VALUE;
   };

   ComDispatchDriver(wchar_t *pname,DISPID did=DISPID_VALUE):m_grfdex(0),m_flags(0)
   {
         m_hr=parse_get_object(pname,&m_disp);
		 m_dispid=did;
   };


ComDispatchDriver& operator  =(ComDispatchDriver_base& drb )
   {
         *((ComDispatchDriver_base*)(this))=drb;

          return *this;
   }


    struct DP_Holder
	{
	   DISPPARAMS m_dp;
	   HRESULT m_hr;
	   DISPID m_putid; 
	   std::vector<VARIANTARG,allocator> m_argv;
	   HRESULT save_params(CollectorDelegator& cd)
	   {
          if (!cd)
			  return E_FAIL;  
		  m_dp=DISPPARAMS();
		  size_t argc=cd->size();  
		  m_dp.cArgs=argc;
          m_argv.resize(argc+1);
          std::reverse_copy(cd->begin(),cd->end(),m_argv.begin()+1);
		  m_dp.rgvarg=&(m_argv[0]);
		  return S_OK;
	   };
      DP_Holder(CollectorDelegator& cd,bool fput=false,ValuePutType* pv=NULL)
	  {
          if(SUCCEEDED(m_hr=save_params(cd)))
		  {	  
		   if(fput)
		   {
             m_putid = DISPID_PROPERTYPUT;
             m_dp.cNamedArgs=1;
			 m_dp.cArgs++;
			 m_dp.rgdispidNamedArgs=&m_putid;
             m_argv[0]=VARIANTARG(*pv);
   		   }
		   else m_dp.rgvarg++;
		  }

	  }
	  operator DISPPARAMS*()
	  {
         return &m_dp;  
	  }
	  operator HRESULT()
	  {
         return m_hr;
	  }
	};


	inline long do_get_no_result(CollectorDelegator cd,long flag)
	{
		if(!cd) return 0;
          ValueGetType tmp;
		return do_get(cd,flag,&tmp);
	};
   
	inline HRESULT invoke(WORD flags,DISPPARAMS* pdp,VARIANTARG& result)
	{  
		HRESULT hr; 
        CComPtr<IErrorInfo>   errorinfo;
		hr=invoke_base(flags,pdp,&result,&errorinfo);
         SetErrorInfo(NULL,errorinfo);  
        return hr;
	}

inline	long do_get(CollectorDelegator cd,long flagmask,ValueGetType* pv)//=NULL)
	{
		HRESULT hr;
		DP_Holder dh(cd);
        if(FAILED(hr=dh)) return hr;
		//ValueGetType tmp;
		//if(!pv) pv=&tmp;
        ValueGetType& result=*pv;
		WORD flags=DISPATCH_GET_ALL;
		//flags&=flagmask; 
		return hr=invoke(flags,dh,result);
	}

inline	long do_put(CollectorDelegator cd,long flagmask,ValuePutType* pv)
	{
		HRESULT hr;
        DP_Holder dh(cd,true,pv);
		if(FAILED(hr=dh)) return hr;
        WORD flags=DISPATCH_PUT_ALL;
        //flags&=flagmask; 
		ValueGetType result;
		return hr=invoke(flags,dh,result);
	};

	long   get_prop_driver(prop_id_type name,Driver* pd)
	{ 
		HRESULT hr;
		DISPID did;
		if(SUCCEEDED(hr=get_id_base(name,&did)))
		{
          *pd=*this;
		   pd->m_dispid=did;
		} 
		
		return hr;
	};
	long	get_driver_from_getval(ValueGetType* pval,Driver* pd)
	{
		HRESULT hr;
		
        VARIANTARG& V=*pval;
        if(SUCCEEDED(hr=VariantChangeType(&V,&V,0,VT_DISPATCH)))
				pd->m_disp=V.pdispVal;
		  return hr;
	}

};
