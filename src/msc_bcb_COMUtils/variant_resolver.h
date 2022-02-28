#pragma once
//#include <atldef.h>
#include <comdef.h>

#define _FAILED_RETURN(a) if(FAILED(a))  return


#define CHECK_FAIL_(hr,a) if(FAILED(hr=(a))) \
	return hr;
#define CHECK_FAIL_hr(a)  CHECK_FAIL_(hr,a)
#define CHECK_FAIL(a) {HRESULT __hr00;CHECK_FAIL_(__hr00,a);}
#define CHECK_IMPL(p) if(!(p)) return E_NOTIMPL;
#define CHECK_PTR(p) if(!(p)) return E_POINTER;


inline VARIANT Ref_VARIANT(VARIANT* pv,bool fcheckref=1)
{
     if (fcheckref&&(pv->vt&VT_BYREF)) return *pv;
	 VARIANT rv={(VT_BYREF|VT_VARIANT)};
	  rv.byref=pv;
	  return rv;
	 
}


inline HRESULT VARIANT_Ref_VARIANT_Ind(VARIANT &v,VARIANT*& ptr)
{ 

	if((v.vt)==(VT_BYREF|VT_VARIANT))
	{
		ptr=v.pvarVal;
		//((VARIANT*)(v.byref));
		return S_OK;
	}
	ptr=&v;
	return S_FALSE;
}




class variant_resolver
{
public:
inline	variant_resolver(VARIANT* pV,bool f_no_disp_prop_resolve=false,bool f_ref_in=true):m_pV(NULL),hr(S_OK)
	{
		VARIANT   UNASSIGNED={VT_EMPTY};

		struct com_deleter_t
		{
            IUnknown* p;
			com_deleter_t(IUnknown* pu=0):p(pu){};
			~com_deleter_t(){ p&&p->Release(); };
		};

		while(S_OK==VARIANT_Ref_VARIANT_Ind(*pV,pV));

		if(VT_ARRAY&V_VT(pV))
		{
			if(V_VT(pV)&VT_BYREF)
			{
				m_V=UNASSIGNED;
				m_V.vt=(VT_TYPEMASK&V_VT(pV))|VT_ARRAY;
				m_V.parray=*(pV->pparray);
				m_pV=&m_V;
			}
			else m_pV=pV;
			return;
		}

		if( (V_VT(pV)&VT_BYREF)) 
		{
			if(!f_ref_in) {m_pV=pV;return ;}

		    _FAILED_RETURN(hr=VariantCopyInd(&m_v,pV));
			pV=&m_v;

		};

		m_pV=pV;   
		VARTYPE vt=V_VT(pV);

		if(f_no_disp_prop_resolve) 
		{ 
          
			return;
		}
		
		if((vt!=VT_UNKNOWN)&&(vt!=VT_DISPATCH)) 
		{
			
			return;
		}
		//CComPtr<IDispatch> disp;
		IDispatch* disp=0;
		

		if(V_VT(pV)==VT_UNKNOWN)
		{
			//CComPtr<IUnknown> unk=V_UNKNOWN(pV);
			com_deleter_t unk(V_UNKNOWN(pV));
			_FAILED_RETURN(hr=unk.p->QueryInterface(__uuidof(IDispatch),(void**)&disp));
			
		}
		else disp=V_DISPATCH(pV); 

		com_deleter_t dpp(disp);
						
			 DISPPARAMS dp = {NULL, NULL, 0, 0};
			 
			 _FAILED_RETURN(hr=disp->Invoke(DISPID_VALUE, IID_NULL,LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,&dp,&m_v, NULL, NULL));
			m_pV=&m_v;
		

	}

    inline VARTYPE vt()
	{
		return VARIANT(*this).vt;
	}

	inline operator HRESULT&()
	{
		return hr;
	}

	inline operator VARIANT*()
	{
		return m_pV;
	}
	
	inline operator VARIANT&()
	{
		return *m_pV;
	}
	VARIANT* operator->() const throw()
	{
		
		return m_pV;
	}

inline	VARIANT    dup(){return _variant_t(*m_pV).Detach();}
public:
	_variant_t m_v;
	VARIANT m_V,*m_pV;
	HRESULT hr;
	// test_class test;
};

inline HRESULT set_default(VARIANT &V,_variant_t v=_variant_t())
{
  if(V.vt==VT_ERROR)
  {
     V=v.Detach();
	 return S_OK;
  }
  return S_FALSE;
}


inline HRESULT getDISPID(IDispatch* pdisp,VARIANT &V,DISPID* pdispid)
{
   HRESULT hr;
   CHECK_PTR(pdispid);
   variant_resolver vr(&V,true);
   if(FAILED(vr)) return vr;
   if(vr.vt()==VT_BSTR)
   {
  
	hr=pdisp->GetIDsOfNames(IID_NULL,&(vr->bstrVal), 1, LOCALE_USER_DEFAULT, pdispid);
    if(SUCCEEDED(hr)) return hr;
   };
   VARIANT r={VT_EMPTY};
   if(SUCCEEDED(hr=VariantChangeType(&r,(VARIANT*)vr,0,VT_I4)))
          *pdispid=r.intVal;
   return hr;

}






#define _VOUT(a)    _variant_t(a).Detach()
#define _VIN(a)    VARIANT(_variant_t(a))
#define _V _VOUT

#define _FAILED_RETURN(a) if(FAILED(a))  return

