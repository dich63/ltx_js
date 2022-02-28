#pragma once

 #include "argsxx.h"
#include "os_utils.h"
#include "safearrayutils.h"
#include "COM_dispatch_helper.h"
#include "dispex.h"
#include <iterator>




#define p_p(a) prop(#a)
#define m_p(a) prop(#a)

struct disp_caller: 
		public call_helper<PA_COM_prepare_invoke<disp_caller>,VARIANT_IN,COM_IDispatchAdapter<disp_caller> >
		
{
      PA_COM_prepare_invoke<disp_caller> m_invoker;
      COM_IDispatchAdapter<disp_caller> m_inner_adapter_for_as;
	     
    
       HRESULT m_hr; 

	  CComPtr<IDispatch> m_disp;
	  CComPtr<IDispatchEx> m_dispex;


inline bool get_curr_disp(CComPtr<IDispatch>& disp)
{
	if(m_invoker.m_dispid==DISPID_VALUE)
			disp=m_disp;
	else 
		disp=as<IDispatch*>();
	return true;
}

inline disp_caller  prop(DISPID did)
{
	CComPtr<IDispatch> disp;
	get_curr_disp(disp);
	return disp_caller(disp,did,m_invoker.m_prop_mask);
}

inline disp_caller  prop(_bstr_t name)
{
	CComPtr<IDispatch> disp;
	get_curr_disp(disp);
   _bstr_t buf;
	wchar_t* ptail, *pname=name;
//inline bool  _wlcheckni(wchar_t* p,wchar_t* ps,int cb=-1,wchar_t** pptail=NULL)

bool ftree;
  if(ftree=(ptail=StrStrW(pname,L".")))
   {
      ptail++;
      int l=std::distance(pname,ptail);
	  wchar_t* p=buf=bstr_alloc(l);
	  StrCpyNW(p,pname,l);
       pname=p;
  }
       pname=buf=trim(pname);

	LPOLESTR lpsz=pname;
	DISPID did=0;
  if((disp)&&(name.length()))
		m_hr=disp->GetIDsOfNames(IID_NULL,const_cast<LPOLESTR*>(&lpsz), 1, LOCALE_USER_DEFAULT,&did);
   if(ftree) return disp_caller(disp,did,m_invoker.m_prop_mask).prop(ptail);
   return disp_caller(disp,did,m_invoker.m_prop_mask);
}


disp_caller(_bstr_t mon_name,DISPID did=DISPID_VALUE,DWORD prop_mask=0)
{
          
    CComPtr<IUnknown> unk;
    m_hr=parse_get_object(mon_name,&unk);
	if(SUCCEEDED(m_hr))
		m_hr=unk.QueryInterface(&m_disp);
	m_hr=init(m_disp,did,prop_mask);
}




disp_caller(IDispatch* pdisp=NULL,DISPID did=DISPID_VALUE,DWORD prop_mask=0)
{
	m_hr=init(pdisp,did,prop_mask);
}
disp_caller(IDispatch* pdisp,_bstr_t pm_name,DWORD prop_mask=0)
{
 
	m_hr=init(pdisp,pm_name,prop_mask);
}

inline HRESULT  init (IDispatch* pdisp=NULL,_bstr_t pm_name=BSTR(NULL),DWORD prop_mask=0)
{
   LPOLESTR lpsz=pm_name;
   DISPID did=0;
 if((pdisp)&&(pm_name.length()))
	 m_hr=pdisp->GetIDsOfNames(IID_NULL,const_cast<LPOLESTR*>(&lpsz), 1, LOCALE_USER_DEFAULT,&did);
   return     init(pdisp,did,prop_mask);

}
inline HRESULT init (IDispatch* pdisp=NULL,DISPID did=DISPID_VALUE,DWORD prop_mask=0)
	{
		m_disp=pdisp;
       m_invoker.m_owner=this;
       m_invoker.m_dispid=did;
       m_invoker.m_prop_mask=prop_mask;
	  set_prepare_invoke(&m_invoker);
	 m_inner_adapter_for_as=m_invoker.do_prepare(0,NULL);
     m_inner_adapter_for_as.flag_complete=1;
	 return m_hr;
	};

	template <class N>
	inline COM_IDispatchAdapter<disp_caller>::cast_pp<N>  as()
	{	
		return m_inner_adapter_for_as.as<N>();	
	}

inline   operator IDispatch*()
{
	return get_dispatch();
}

inline IDispatch* get_dispatch()
{
	return m_disp.p;
};
inline operator VARIANT_IN()
{
   return m_disp.p;	
}

/*
inline IDispatchEx* get_dispatchex()
{
	if(!m_disp) return NULL;
   if(!m_dispex)
   {
	   if(FAILED(m_disp.QueryInterface(&m_dispex)))
		   m_dispex.Release();
   }
	return m_dispex.p;
};

*/
};

