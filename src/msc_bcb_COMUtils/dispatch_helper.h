//---------------------------------------------------------------------------
#ifndef dispatch_helperH
#define dispatch_helperH
//---------------------------------------------------------------------------
#include "smart_callers.h"
#include "smart_proper.h"
#include "oleauto.h"
#include <vector>
#include "os_utils.h"
#include <typeinfo.h>
//=======================================

#define P_P(a,b) a[#b]
#define p_p(b) prop(#b)



#define COMCheck  _com_util::CheckError

template<>
class _Cast<VARIANT>
{

public:
	VARIANT d;
	///*
	_Cast()
	{
		VARIANT _d={0}
		;d=_d;
	};
	template<class Ta>
	inline  Ta& type_cast(Ta& t, VARIANT& v)
	{

		return t=_variant_t(v);
	}
	template<class Ta>
	inline  Ta& type_cast(Ta& t, VARIANT const& v)
	{

		return t=Ta(_variant_t(v));
	}
/*
   template<class Ta>
   inline prop_adapter<Ta,VARIANT>& type_cast(Ta& t, VARIANT const& v)
   {

   }
*/


	inline  int& type_cast(int& t, VARIANT& v)
	{
		return t=(long)_variant_t(v);
	}
	inline  int& type_cast(int& t, VARIANT const& v)
	{

		return t=(long)_variant_t(v);
	}

	inline  VARIANT& assign(const int& a)
	{
		return d=_variant_t(long(a)).Detach();
	}

	template<class T>
	inline  VARIANT& assign(const T& a)
	{
		 d=_variant_t(a).Detach();
         
		 return d;
	}


	//template<>
	inline  const VARIANT& assign(const VARIANT& a)
	{
		return a;
	}
	//    template<>

	operator VARIANT&()
	{
		return d;
	}
	operator const VARIANT&()
	{
		return d;
	}
};

template<class T>
inline  T& type_cast(T&t, VARIANT v)
{
	return t=_variant_t(v);
}

//===================================================================================


#define FVARIANT_CLEAR 1
#define FVARIANT_PROP  2

//template<,typename Cast=_Cast<VARIANT> >
//template <class Cast=_Cast<VARIANT> >
//
/*
template<>
//class type_adapter<VARIANT,_Cast<VARIANT> >: public VARIANT
class type_adapter<VARIANT>: public VARIANT
{
	//
	typedef   _Cast<VARIANT> Cast;

public:
	DWORD flag;
	inline type_adapter():flag(0)
	{
      vt=VT_EMPTY;
	};

	template<class A>
	inline type_adapter(A& a)
	{
		
		*(LPVARIANT(this))=Cast().assign(a);
	}

	template<class A>
	inline type_adapter(A const& a)
	{   
		
		*(LPVARIANT(this))=Cast().assign(a);  
		
	}



	~type_adapter()
	{
		SP_TRACE(_T("TA destructor: %s\n"),CA2T(typeid( this ).name()));
	};

	operator VARIANT&()
	{
		return *this;
	};

};

*/

template<>
class type_adapter<VARIANT,_Cast<VARIANT> >: public VARIANT
{
	typedef   _Cast<VARIANT> Cast;
public:
	DWORD flag;
	 type_adapter():flag(1)
	{
		vt=VT_EMPTY;
	};

/*
type_adapter(const type_adapter&  t)
   {
	   
      *LPVARIANT(this)=*LPVARIANT(&t);
	  flag=t.flag;
   }
*/
	 ~type_adapter()
	 {
		 if(flag) VariantClear(this);
		 SP_TRACE(_T("TA destructor: %s\n"),CA2T(typeid( this ).name()));
	 };

/*
    template <class A>
		 inline type_adapter& operator =(A& arg)
	 {
       vt=0;
	 *((_variant_t*)this)=arg;
	 return  *this; 
	 }

 */
	template<class A>
	 type_adapter(A& a)
	{
       flag=1;
     *LPVARIANT(this)=Cast().assign(a);	
	}

	template<class A>
	 type_adapter(A const& a)
	{   
      flag=1;  
      *LPVARIANT(this)=Cast().assign(a);	
	}

	
	
	 type_adapter(VARIANT const& v)
	{   
       flag=0;  
	  *LPVARIANT(this)=v;
	}

	 type_adapter(VARIANT & v)
	{   
		flag=0;  
		*LPVARIANT(this)=v;
	}

	 type_adapter(_variant_t const& v)
	{   
		flag=0;  
		*LPVARIANT(this)=v;
	}

	 type_adapter(_variant_t & v)
	{   
		flag=0;  
		*LPVARIANT(this)=v;
	}

	 type_adapter(const LPSAFEARRAY  psa)
	{   
		flag=0;  
		vt=VT_ARRAY;
		parray=psa;
	}

	 


	operator VARIANT&()
	{
		return *this;
	};

};


template<class T>
inline  T& type_cast(T&t,const type_adapter<VARIANT>& v)
{
	return t=_variant_t(*( LPVARIANT)(&v));
}

typedef   type_adapter<VARIANT> VTA;





template<class Cast=_Cast(VARIANT)>
class dispatch_adapterC:
	public invoke_helper_base<dispatch_adapterC<Cast>,VARIANT,Cast>
{

public:
CComPtr<IDispatch> m_disp;
DISPID m_id;

//EXCEPINFO ei;
static __declspec(thread)    HRESULT hr;


dispatch_adapterC<Cast>(_bstr_t progid):invoke_helper_base(this),m_id(0)
{
	
		hr=m_disp.CoCreateInstance(progid);

}


dispatch_adapterC<Cast>(const _variant_t& pt):invoke_helper_base(this),m_id(0)
{
	VARIANT v={0};
   if(SUCCEEDED(hr=VariantChangeType(&v,&pt,0,VT_DISPATCH)))
   {
	   m_disp=v.pdispVal;
   };

}

//dispatch_adapterC operator =

dispatch_adapterC(IDispatch* pdisp=NULL,DISPID id=DISPID_VALUE):invoke_helper_base(this),m_id(id)
{
   m_disp=pdisp;
   SP_TRACE(_T("@@@@@@@@@@@[%p] Constructor dispatch_adapterC ID=%d \n"),this,m_id);
};

dispatch_adapterC(const dispatch_adapterC& src):invoke_helper_base(*((dispatch_adapterC*)&src))
{   
    m_id=src.m_id;
	m_disp=src.m_disp;
	//	fdelayget=src.fdelayget;
	SP_TRACE(_T("@@@@@@@@@@@[%p] COPY Constructor dispatch_adapterC ID=%d \n"),this,m_id);
};

~dispatch_adapterC()
{
SP_TRACE(_T("@@@@@@@@@@@[%p] Destructor dispatch_adapterC ID=%d \n"),this,m_id);
};
	//
OVERRIDE_OP_EQU(dispatch_adapterC,invoke_helper_base);


inline dispatch_adapterC& operator =(dispatch_adapterC& arg)
{
	m_id=src.m_id;
	m_disp=src.m_disp;
	SP_TRACE(_T("@@@@@@@@@@@[%p] operator = dispatch_adapterC ID=%d \n"),this,m_id);
return  *this; 
}

inline dispatch_adapterC& operator =(dispatch_adapterC const& src)
{
	m_id=src.m_id;
	m_disp=src.m_disp;
	SP_TRACE(_T("@@@@@@@@@@@[%p] operator const = dispatch_adapterC ID=%d \n"),this,m_id);
 return  *this; 
}


inline DISPID get_dispid(const _variant_t& v )
{
   hr=S_OK;
   DWORD id=-1;
  if(v.vt==VT_BSTR)
  {
    hr=m_disp.GetIDOfName(v.bstrVal,&id);
	
  }
  else 
  {
	  VARIANT V;
	  if(SUCCEEDED(hr=VariantChangeType(&V,&v,0,VT_I4)))
		  id=V.intVal;

  }
  return id;
}

	inline HRESULT get_prop(VARIANT*pv)
	{

    if(FAILED(hr))
	{
	 if(pv)	*pv= VarError(hr);
	 return hr;
	} 
     std::vector<VARIANT> Varg;
         		//_bstr_t s=typeid( this ).name();
		SP_TRACE(_T(" %s :pgetV...[%08x]\n"),(wchar_t*)CA2T( typeid( this ).name()),(void*)pv);
         
		WORD flag=(fstate&1)? DISPATCH_METHOD:DISPATCH_PROPERTYGET;
        LPVARIANT p=0;

		if(argc>0)
		{
		flag|=DISPATCH_PROPERTYGET;
        Varg.resize(argc);        
		//
		std::vector<VARIANT>::iterator pb= Varg.begin();
        std::reverse_copy(argv,argv+argc,pb); 
		 p=&Varg[0];
		}
		DISPPARAMS dp={p,NULL,argc,0};    
		hr= internal_invoke(flag,&dp,pv);
		if(FAILED(hr))
		{
			if(pv)	*pv= VarError(hr);
	
		} 
        return hr; 
	}
	inline HRESULT put_prop(const VARIANT*pv)
	{
		std::vector<VARIANT> Varg;
		SP_TRACE(_T(" %s :pputV...[%p]\n"),(wchar_t*)CA2T( typeid( this ).name()),pv);
       
       WORD flag=DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF;
	   DISPID putid = DISPID_PROPERTYPUT;
	   LPVARIANT p=LPVARIANT(pv);
	   if(argc>0)
	   {
        Varg.resize(argc+1);        
        std::vector<VARIANT>::iterator pb= Varg.begin();
	    std::reverse_copy(argv,argv+argc,pb+1);  
	     Varg[0]=*pv;
        p=&Varg[0];
 	   };
	   
       DISPPARAMS dp={p,&putid,argc+1,1};
        
      return internal_invoke(flag,&dp,NULL);

	}
	inline HRESULT internal_invoke(WORD flag,DISPPARAMS *pdp,VARIANT*presult)
	{
   	 	hr=m_disp->Invoke(m_id,IID_NULL,LOCALE_USER_DEFAULT,flag,pdp,presult,NULL,NULL);
		//m_id=DISPID_VALUE;
		//Varg.resize(0);
		reset_state();
      return hr;
	};

/*
inline  dispatch_adapterC<Cast> get_pp(_bstr_t name)
{
	DISPID id=0;
  hr=m_disp.GetIDOfName(name,&id);
  return dispatch_adapterC<Cast>(m_disp,id);
}

*/


inline dispatch_adapterC<Cast> operator [](_bstr_t name)
{
    return prop(name);
}
dispatch_adapterC<Cast> prop(_bstr_t name)
{
	DISPID id=0;
	wchar_t *ptail=0;
    CComPtr<IDispatch> cd;
	
	if(fstate&FSTATE_PROP)
	{
		_variant_t v; 

	if(FAILED(hr=m_disp.GetProperty(m_id,&v))) return *this;
	if(FAILED(hr=VariantChangeType(&v,&v,0,VT_DISPATCH)) )return *this;
		cd=v.pdispVal;
	}
	else	cd=m_disp;

    wchar_t *pn=wcstok_s(name,L".",&ptail);
 	while(*ptail)
	{
     _variant_t v; 
      if(FAILED(hr=cd.GetPropertyByName(pn,&v)))return *this;
      if(FAILED(hr=VariantChangeType(&v,&v,0,VT_DISPATCH)) )return *this;
	  cd=v.pdispVal;
      pn=wcstok_s(NULL,L".",&ptail);
	}
	hr=cd.GetIDOfName(pn,&id);
	if(FAILED(hr)) return *this;
	dispatch_adapterC<Cast> d(cd,id);
	d.fstate=FSTATE_PROP;
	return d;
}

};
  

typedef dispatch_adapterC<_Cast<VARIANT> > dispatch_adapter;
__declspec(thread,selectany) HRESULT dispatch_adapter::hr;//=NULL;

/*
dis.pp["Root"](1,2,3,3);
dis.pp["Root"].pp["Agr"](1,2,3,3);
dis._pp(Root)._pp(Agr)(1,2,3,3);
*/

#endif

