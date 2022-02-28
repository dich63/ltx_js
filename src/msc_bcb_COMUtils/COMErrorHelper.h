#pragma once



#include "variant_resolver.h"
//#include "safearrayutils.h"
#include "os_utils.h"
#include "vector"
//SafeArrayGetElemsize



inline VARIANT* VARIANT_deep_ref(VARIANT* pv)
{ 
	__try
	{
		while ((pv)&&(V_VT(pv)==(VT_BYREF|VT_VARIANT))) pv=V_VARIANTREF(pv);

	}__except(EXCEPTION_EXECUTE_HANDLER )
	{
		return NULL;
	}

	return pv;
}
//CComVariant
//GetVariantElemsizeSize(

inline long VARIANT_clear_ref(VARIANT* pd,VARIANT* ps)
{
	long cb=-1;
	if(!ps) return 0;
	VARTYPE vt=V_VT(ps);
	if((vt&VT_BYREF)==0)
		*pd=*ps;
	else
	{
		vt&=(~VT_BYREF);
		if(!(cb=GetVariantElemsizeSize(vt))) return 0;
		pd->vt=vt;
		memcpy(&(pd->byref),ps->byref,cb);
	} 

	return cb;
}

struct safe_in_DISPARAMS
{
	HRESULT m_hr;
	DISPPARAMS m_dm;
	VARIANTARG* m_pvar;
	bool fref;

	inline bool check_ref(DISPPARAMS* pdp)
	{

		for(int n=0;n<pdp->cArgs;n++) 
			if(VT_BYREF&(pdp->rgvarg[n].vt)) return fref=true;
		return fref=false;
	}

	safe_in_DISPARAMS(DISPPARAMS* pdp,DISPPARAMS* pdp2=NULL):m_pvar(NULL),m_hr(S_OK)
	{

		m_dm=*pdp;
		if((pdp->cArgs)&&(pdp->rgvarg==NULL)) 
		{m_hr=DISP_E_BADVARTYPE;return;}
		if((pdp2)||check_ref(pdp))
		{


			if(pdp2) m_dm.cArgs+=pdp2->cArgs;  

			m_dm.rgvarg=m_pvar=(VARIANTARG*)calloc(m_dm.cArgs,sizeof(VARIANTARG));

			if(!m_pvar) {m_hr=E_OUTOFMEMORY;return;}

			//for(int n=0;n<pdp->cArgs;n++) 
			LPVARIANTARG pd=pd=m_pvar;
			for(LPVARIANTARG ps=pdp->rgvarg ;ps!=pdp->rgvarg+pdp->cArgs;ps++,pd++) 
			{
				VARIANT* pp=VARIANT_deep_ref(ps);

				if(!VARIANT_clear_ref(pd,pp)) 
				{m_hr=DISP_E_BADVARTYPE;return;}         
			}
			if(pdp2)
			{
				for(LPVARIANTARG ps=pdp2->rgvarg;ps!=pdp2->rgvarg+pdp2->cArgs;ps++,pd++) 
				{
					VARIANT* pp=VARIANT_deep_ref(ps);

					if(!VARIANT_clear_ref(pd,pp)) 
					{m_hr=DISP_E_BADVARTYPE;return;}         
				}


			}

		}

	};
	~safe_in_DISPARAMS()
	{
		if(m_pvar) free(m_pvar);
	}
	inline operator DISPPARAMS*()
	{
		return &m_dm;
	}
	inline operator HRESULT()
	{
		return m_hr;
	}
	inline DISPPARAMS* operator->()
	{
		return &m_dm;
	}

};



struct exceptinfo_t: EXCEPINFO
{
	exceptinfo_t(){ memset(this,0,sizeof(exceptinfo_t));}
	~exceptinfo_t()
	{
		SysFreeString( bstrSource);
		SysFreeString(bstrDescription);
		SysFreeString(bstrHelpFile);
	}

};

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

	HRESULT set_error()
	{
		HRESULT hr;
		CComPtr<IErrorInfo> eih;
		if(SUCCEEDED(hr=get_error_info(&eih)))
			hr=SetErrorInfo(0,eih);

		return hr;
	}

};



inline bool fill_ExceptInfo(EXCEPINFO* pexi,IErrorInfo* ei=0)
{

	if(!pexi) return 0;
	CComPtr<IErrorInfo> eih;
	if(ei==0)
	{
		if(S_FALSE==GetErrorInfo(0,&eih))
			return 0;
		ei=eih;
	}
	EXCEPINFO& exi=*pexi;
	exi.scode=DISP_E_EXCEPTION;
	ei->GetDescription(&(exi.bstrDescription));
	ei->GetSource(&(exi.bstrSource));
	ei->GetHelpContext(&(exi.dwHelpContext));
	ei->GetHelpFile(&(exi.bstrHelpFile));
	return 1;
};


inline HRESULT set_ExceptInfo(EXCEPINFO* pexi,HRESULT hr=DISP_E_EXCEPTION)
{
	if(FAILED(hr))
		fill_ExceptInfo(pexi);
	return hr;
}


/*
HRESULT invoke(IDispatch* p, VARIANT* pvarParam1,VARIANT* pvarRet,EXCEPINFO* pei=NULL) throw()
{
DISPPARAMS dispparams = { pvarParam1, NULL, 1, 0};
return p->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD, &dispparams, pvarRet, pei, NULL);
}
*/


