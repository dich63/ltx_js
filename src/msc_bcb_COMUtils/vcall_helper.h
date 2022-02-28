#pragma once
//#include "os_utils.h"
#include "COMErrorHelper.h"


#include "variant_resolver.h"
#include <vector>
#include <algorithm>
#include <shlwapi.h>

/*
#define DISPATCH_METHOD         0x1
#define DISPATCH_PROPERTYGET    0x2
#define DISPATCH_PROPERTYPUT    0x4
#define DISPATCH_PROPERTYPUTREF 0x8
*/


struct vcall_helper
{
	HRESULT hr;
	SAFEARRAY* m_psa;

inline static bool is_optional(const VARIANT& v,bool fex=false)
	{

		if(fex)
		{
			VARTYPE vt=v.vt&VT_TYPEMASK;
			if(vt==(VT_VARIANT)&&(v.vt&VT_BYREF))
			{
				if(!v.byref) return true;
				vt= v.pvarVal->vt;
			}
			fex=(vt==VT_EMPTY)||(vt==VT_NULL)||(vt==VT_ERROR);

			return fex;
		}


		return (v.vt==VT_ERROR)&&(v.scode==DISP_E_PARAMNOTFOUND);
	}



	inline static LONG32	get_total_size(SAFEARRAY* psa)
	{
		if(psa->cDims<=0) return 0;
		LONG32 e=1;
		for(int n=0;n<psa->cDims;n++)
			e*=psa->rgsabound[n].cElements;
		return e;
	}

	inline static int set_flags(VARIANT& voptions,int& flags)
	{
		HRESULT hr;
	  	if(is_optional(voptions))
			 return flags;

        VARIANT vi=VARIANT();
		if(SUCCEEDED(hr=VariantChangeType(&vi,&voptions,0,VT_I4)))
		{
			if(vi.intVal) flags=vi.intVal;
		}
		else
		{
         variant_resolver vo(&voptions);
		 wchar_t* p;
		 if((vo->vt==VT_BSTR)&&(p=vo->bstrVal))
		 {
			 int ii=0;
			 
             if(StrChrIW(p,L'g')) ii|=DISPATCH_PROPERTYGET;
			 if(StrChrIW(p,L'm')) ii|=DISPATCH_METHOD;
			 if(StrChrIW(p,L'p')) ii|=DISPATCH_PROPERTYPUT;
			 if(StrChrIW(p,L'r')) ii|=DISPATCH_PROPERTYPUTREF;
             if(ii) flags=ii;  

		 }

		}
		 

		return  flags;
	}

	template <class T >
	vcall_helper(T* pdisp,DISPID dispid,VARIANT args,VARIANT options,VARIANT* pResult,
		int flags=(DISPATCH_METHOD|DISPATCH_PROPERTYGET)):m_psa(0)
	{
		if(!pdisp) {hr=E_POINTER; return ;}



		std::vector<VARIANT> vargs;
		int argc;

		DISPID  putid=DISPID_PROPERTYPUT;

//		 int f=VINT_def(options,flags);
//		 if(f!=0) flags=f;  

		set_flags(options,flags);

		bool fput=flags&(DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF);

		DISPPARAMS dp={NULL,(fput)?&putid:NULL,0,(fput)?1:0};

		if(!is_optional(args))
		{
			variant_resolver vr(&args);
			if(vr.vt()!=(VT_VARIANT|VT_ARRAY))
			{  hr=E_INVALIDARG;return;}

			SAFEARRAY *psa=vr->parray;
			if(psa)
			{
				VARIANT* pvarg=0;
				int c=get_total_size(psa);
				if((c)&&SUCCEEDED(hr=SafeArrayAccessData(psa,(void**)&pvarg)))
				{
					m_psa=psa;
					vargs.resize(c);


					std::reverse_copy(pvarg,pvarg+c,vargs.begin());
					//SafeArrayUnaccessData(psa);
					dp.rgvarg=&vargs[0];
					dp.cArgs=c;

				}



			}


		};



		CComExcepInfo exceptinfo;	
		VARIANT tmp=VARIANT();
		hr=pdisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,flags,&dp,&tmp,&exceptinfo, NULL);
		if(FAILED(hr))
		{
			ErrorHelper(exceptinfo).set_error();
		}
		else  if(pResult) 
			*pResult=tmp;

	}


	~vcall_helper()
	{
		if(m_psa)  SafeArrayUnaccessData(m_psa);
	}


	inline operator HRESULT()
	{
		return hr;
	}

};

#pragma pack(push)
#pragma pack(1)
struct DISPPARAMS_vector_t
{

	int vt;
	SAFEARRAY sa;
	std::vector<VARIANT> vargs;
	VARIANT res;
	
	DISPPARAMS_vector_t(DISPPARAMS* pdp=0)
	{
		reset(pdp);

	}

DISPPARAMS_vector_t&	reset(DISPPARAMS* pdp=0,bool femptyarray=false)
	{
		res.vt=VT_ERROR;
		res.scode=DISP_E_PARAMNOTFOUND;

        if(!pdp) return *this;
        int c=pdp->cArgs;
		if((!c)&&(!femptyarray)) return *this;
		vargs.resize(c);
		std::reverse_copy(pdp->rgvarg,pdp->rgvarg+c,vargs.begin());
		vt=VT_VARIANT;
		sa.cbElements=sizeof(VARIANT);
		sa.cDims=1;
		sa.fFeatures=FADF_HAVEVARTYPE|FADF_VARIANT;
        sa.cLocks=0;

		sa.pvData=(c)?&vargs[0]:NULL;

		sa.rgsabound[0].cElements=c;
		sa.rgsabound[0].lLbound=0;
		res.vt=VT_VARIANT|VT_ARRAY;
		res.parray=&sa;
		return *this;
	}
inline operator VARIANT&()
{
	return res;
}

};

#pragma pack(pop)