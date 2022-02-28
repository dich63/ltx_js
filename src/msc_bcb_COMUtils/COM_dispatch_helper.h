#pragma once



#include <tls_dllex.h>
#include "call_helper.h"

#include <vector>
#include <list>
#include <string>
#include <algorithm>

#define disp_ind(i,n)  (n-1-i)


struct ErrorHandler  
{


};





template<>
class CVarTypeInfo< VARIANT* >
{
public:
	static const VARTYPE VT = VT_VARIANT|VT_BYREF;
	static VARIANT* VARIANT::* const pmField;
};

__declspec( selectany ) VARIANT* VARIANT::* const CVarTypeInfo< VARIANT* >::pmField = &VARIANT::pvarVal;


struct VARIANT_REF_OUT
{
  VARIANT& r_v;
  VARIANT_REF_OUT(VARIANT& v):r_v(v){};
 inline operator VARIANT()
 {
	 return r_v;
 }
};



struct VARIANT_IN
{
	VARIANT m_v;

	struct VARIANT_BUF_ITEM
	{
		VARIANT m_v;
		Delegator<bool,true,false> fb;

		VARIANT_BUF_ITEM(VARIANT v)	{m_v=v;	}
		VARIANT_BUF_ITEM(){	m_v.vt=0;}
		~VARIANT_BUF_ITEM()
		{
			if((fb)&&m_v.vt)
				::VariantClear(&m_v);
		}

	};



	template<typename T>
	struct Assign
	{

		Assign(VARIANT& vdest,T const& val)
		{  
			//if(is_inherited<VARIANT,_variant_t>::val)
			const bool fs=	is_inherited<T,SAFEARRAY*>::val;
			const bool f=	is_inherited<T,VARIANT>::val;
			const bool f2=	          
				is_inherited<const wchar_t*,T>::val
				|| is_inherited<wchar_t*,T>::val
				|| is_inherited<const char*,T>::val
				||	is_inherited<char*,T>::val;

			const bool f3=is_inherited<std::wstring,T>::val||is_inherited<std::string,T>::val;

			const int nn=(fs)?4:((f2)?1:((f3)?3:2));
			const int nt=(f)?0:nn;

			make<nt>(vdest,val);
		}

		template<int tn>
		inline 	void  make(VARIANT& vdest,T const& val);

		template<>
		inline 	void  make<4>(VARIANT& vdest,T const& val)
		{
			vdest=VARIANT();
			vdest.vt=VT_ERROR;
			///*
			if(SUCCEEDED(vdest.scode=SafeArrayGetVartype(val,&vdest.vt)))
			{
				vdest.vt|=VT_ARRAY;
				vdest.parray=val;
			}
			else VARIANT_IN::hr()=vdest.scode;
			//*/
		};

		template<>
		inline 	void  make<0>(VARIANT& vdest,T const& val)
		{
			vdest=(VARIANT)val;
		};

		template<>
		inline 	void  make<1>(VARIANT& vdest,T const& val)
		{
			vdest=VARIANT_IN::add_buf(val); 
		};

		template<>
		inline 	void  make<2>(VARIANT& vdest,T const& val)
		{
			vdest=VARIANT();
			vdest.vt=CVarTypeInfo<T>::VT;
			vdest.*CVarTypeInfo<T>::pmField=val;
			//vdest=_variant_t(val).Detach();
		};

		template<>
		inline 	void  make<3>(VARIANT& vdest,T const& val)
		{

			vdest=VARIANT_IN::add_buf(val.c_str()); 
		};


	};


	//typedef std::vector<VARIANT_BUF_ITEM>  VIBUF;	
	typedef std::list<VARIANT_BUF_ITEM>  VIBUF;	

	inline static   VIBUF&  get_buf()
	{
		//static
			thread_singleton_ptr_auto_create<VIBUF,VARIANT_IN> s_buf;
		return  *((VIBUF*)s_buf);
	}

	///*
	inline static   HRESULT&  hr()
	{
		//static	 
		thread_singleton_varT<HRESULT,VARIANT_IN> s_hr;

		return s_hr; 
		//*(&HRESULT(s_hr));
	}	
	//*/	


	template<typename T>
	inline static VARIANT& add_buf(T const & v )
	{
	//	static VARIANT_BUF_ITEM empty=VARIANT_BUF_ITEM();
		VIBUF& buf= get_buf();
		VARIANT r=_variant_t(v).Detach();
		buf.push_back(VARIANT_BUF_ITEM(r));
        return r; 
	}



	inline static  void reset_buf() 
	{
		get_buf().clear();
   /*		
        VIBUF& sb=get_buf();
		int cb=sb.size();
        for(int i=0;i<cb;i++)	  sb[i].m_v.bstrVal[0]=L'?';
   */

	}

	VARIANT_IN(){m_v=VARIANT();}
     
	///*
	template<class T>
	VARIANT_IN(T const& t)
	{
		//  const bool f=	is_inherited<T,VARIANT>::val;
		//	 
		Assign<T>(m_v,t);
	};

   
    VARIANT_IN(CComPtr<IDispatch>const & o)
	{
       Assign<IDispatch*>(m_v,o.p); 
	}
	VARIANT_IN(CComPtr<IUnknown>const & o)
	{
		Assign<IUnknown*>(m_v,o.p); 
	}

	inline operator VARIANT&()
	{
		return m_v;
	}

	//*/
};

#define DISPATCH_GET_ALL (DISPATCH_METHOD|DISPATCH_PROPERTYGET)
#define DISPATCH_PUT_ALL (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF) 
#define DISPATCH_ALL (DISPATCH_GET_ALL|DISPATCH_PUT_ALL) 

template <class Owner>
struct COM_prepare_invoke_base
{
	Owner* m_owner;
	DISPID m_dispid;
	int m_argc;
	std::vector<VARIANT> m_argv;
	DWORD m_prop_mask;

	COM_prepare_invoke_base(Owner* o=NULL,DISPID dispid=DISPID_VALUE,DWORD prop_mask=0)
		:m_owner(o),m_dispid(dispid), m_prop_mask(prop_mask),m_argc(0)
	{ }

	inline void reset_params()
	{
		m_argv.clear();
	}
	inline HRESULT save_params(int argc,VARIANT_IN* pargv)
	{
		HRESULT hr;

		if(SUCCEEDED(hr=VARIANT_IN::hr()))
		{
			ExcepInfo().Clear();
			m_argv.resize(argc+1);
			m_argv[0]=VARIANT();
			std::vector<VARIANT>::iterator it=m_argv.begin()+1;
			std::reverse_copy(pargv,pargv+argc,it);
			m_argc=argc;
		}   
		return hr;

	};


	inline HRESULT   invoke(DWORD flag,DISPPARAMS& dp,VARIANT* pres=NULL)
	{
		HRESULT hr;
		VARIANT res=VARIANT();
		IDispatch* pdisp;

		CComExcepInfo& exinfo= ExcepInfo();

		if( (m_owner)&&(pdisp=m_owner->get_dispatch())) 
		{
			
			hr=pdisp->Invoke(m_dispid,IID_NULL,LOCALE_USER_DEFAULT,flag,&dp,&res,&exinfo,NULL); 
			//hr=pdisp->Invoke(m_dispid,IID_NULL,LOCALE_USER_DEFAULT,flag,&dp,&res,NULL,NULL); 
		}  
		else  hr=E_POINTER;
		VARIANT_IN::hr()=hr;
		if(SUCCEEDED(hr))
		{
			if(pres)  *pres=res;
			else VariantClear(&res);
		};
		//VARIANT_IN::reset_buf();
		//reset_params();
		return hr;
	}



	inline HRESULT   put_putref_invoke(DWORD flag,VARIANT_IN* pval)
	{
		HRESULT hr;
		m_argv[0]=*pval;
		DISPID putid = DISPID_PROPERTYPUT;
		DISPPARAMS dp={ &(m_argv[0]),&putid,m_argc+1,1}; 
		return hr=invoke(flag,dp);  
	};


	inline HRESULT   get_call_invoke(DWORD flag,VARIANT* pres)
	{
		HRESULT hr;
		DISPPARAMS dp={(m_argc>0)?(&(m_argv[1])):NULL ,NULL,m_argc,0};  
		return hr=invoke(flag,dp,pres);  
	};
	inline static   CComExcepInfo&  ExcepInfo()
	{
		//static 
		return  *((CComExcepInfo*)thread_singleton_ptr_auto_create<CComExcepInfo,VARIANT_IN>());
	}

inline Owner* getOwner() const
{
	return m_owner;
}

};




//public call_helper<PA_COM_prepare_invoke<disp_caller>,VARIANT_IN,COM_IDispatchAdapter<disp_caller> >

template <class Owner>
struct COM_IDispatchAdapter	:
	public  call_helper<COM_IDispatchAdapter<Owner>,VARIANT_IN,COM_IDispatchAdapter<Owner> >
	//,	public VARIANT_IN

{

	long flag_complete;
	DWORD m_prop_mask;
    _variant_t m_tmp;
    //VARIANT_IN m_vin;


template <class N>
struct cast_pp
{
_variant_t m_r;
COM_IDispatchAdapter<Owner> &const m_da;
cast_pp(COM_IDispatchAdapter<Owner>& const da):m_da(da){}
 inline operator N()
 {
	 m_r.Clear();
	 m_da.do_get(&m_r);
	 //VARIANT_IN::reset_buf();
	 return m_r;
 }

inline N def(N nd)
{
    m_r.Clear();
	if(SUCCEEDED(m_da.do_get(&m_r)))
	{
		try
		{
			return m_r;
		}
		catch (...){}
	}
	return nd;

}


 inline cast_pp& operator =(N n)
 {
     VARIANT_IN vn(n);
     m_da.do_put(&vn);
	 //VARIANT_IN::reset_buf();
	 return *this;
 }
~cast_pp()
{
}

};



	//		std::auto_ptr<int> m_holder_for_call_her;


typedef COM_prepare_invoke_base<Owner> prepare_type;
	//prepare_type* m_pi;

   Delegator<prepare_type*> m_pi;
   Delegator<Owner*,NULL,NULL,true> m_holder_for_call_helper;

    COM_IDispatchAdapter():flag_complete(0){};
	COM_IDispatchAdapter(prepare_type* pi):flag_complete(0)
	{
        m_pi=pi;
		if(pi) m_prop_mask=pi->m_prop_mask;
		//set_prepare_invoke(this);
	};

template <class N>
inline cast_pp<N>  as()
{
	return cast_pp<N>(*this);
}


	virtual COM_IDispatchAdapter<Owner>* get_prepare_invoke()
	{
		return this;
	}


HRESULT call_helper_from_get(Owner*& po)
{
     
	 _variant_t res;
	 IDispatch *pdisp=NULL;   
	 HRESULT hr=VARIANT_IN::hr();

	 if(SUCCEEDED(hr)&&SUCCEEDED(hr=do_get(&res)))
	 {
		 hr=VariantChangeType(&res,&res,0,VT_DISPATCH);
		 VARIANT_IN::hr()=hr;
		 if(SUCCEEDED(hr)) pdisp=res.pdispVal;
	 }

	  po=new Owner(pdisp);
	 return hr;
} 




inline COM_IDispatchAdapter<Owner> do_prepare(int argc,VARIANT_IN* pargv)
	{
	
		/*
         _variant_t res;
          IDispatch *pdisp=NULL;   
          HRESULT hr=VARIANT_IN::hr();


		 if(SUCCEEDED(hr)&&SUCCEEDED(hr=do_get(&res)))
		 {
           hr=VariantChangeType(&res,&res,0,VT_DISPATCH);
		   VARIANT_IN::hr()=hr;
		   if(SUCCEEDED(hr)) pdisp=res.pdispVal;
   		 }
        
    
           Owner* po=new Owner(pdisp);
     */
		Owner* po;
        call_helper_from_get(po);
		COM_IDispatchAdapter<Owner> adapter=po->get_prepare_invoke()->do_prepare(argc,pargv);
		 adapter.m_holder_for_call_helper=po;
	
		return adapter;
	};

//__if_exists(Owner::prop){

inline Owner prop(DISPID id)
{
	Owner* po=NULL;
	call_helper_from_get(po);
	if(po)
	{
		m_holder_for_call_helper=po;  
		return po->prop(id);
	}
	return Owner();

}

inline Owner prop(_bstr_t nm)
{
    Owner* po=NULL;
	call_helper_from_get(po);
	if(po)
	{
 	  m_holder_for_call_helper=po;  
      return po->prop(nm);
	}
	return Owner();
}

//};


/*
	COM_IDispatchAdapter(COM_IDispatchAdapter &s)
	{
		m_pi=s.m_pi;
		m_prop_mask=s.m_prop_mask;
		s.flag_complete=1;
		flag_complete=0;
	}

*/
	inline COM_IDispatchAdapter& gmode(DWORD mask) const
	{
		m_prop_mask|=mask;
		return *this;
	};




	inline HRESULT do_get(VARIANT* pres=NULL)
	{
		if(!m_pi) return E_POINTER;
		DWORD flag=DISPATCH_GET_ALL;
		flag_complete=1;
		return m_pi->get_call_invoke(flag, pres);
	};

	inline HRESULT do_put(VARIANT_IN* pval)
	{
		if(!m_pi) return E_POINTER;
		DWORD flag=DISPATCH_PUT_ALL;
		flag_complete=1;
		return m_pi->put_putref_invoke(flag,pval);

	};

template <bool F>
inline void get_on_destroy(){}

template <>
inline void get_on_destroy<true>()
{
	if(!m_pi) return;
	if(!flag_complete) 	do_get();
    VARIANT_IN::reset_buf();

}



	~COM_IDispatchAdapter()
	{

		//if(!m_pi) return;

		//if(!flag_complete) 	do_get();


         ///
		get_on_destroy<true>();
		//if(!m_pi) return;
		//if(!flag_complete) 	do_get();
         //m_pi->reset_params();
          //VariantClear(&m_v);
		 
        
	}

	template <class A>
	inline COM_IDispatchAdapter& operator =(A a)
	{
		VARIANT_IN val(a);
		do_put(&val);
		return *this;
	};

//	
	/*
	template <class A>
	inline operator A() const
	{
		A* paa;
		//_variant_t va;
		  
     //	 const_cast<COM_IDispatchAdapter*>(this)->do_get(&m_tmp);
		return A(m_tmp);
		//		return A();
	}
*/  // 


/*
inline operator VARIANT() const
{
	//_variant_t va;
	VARIANT va=VARIANT();
   const_cast<COM_IDispatchAdapter*>(this)->do_get(&va);
	return va;//.Detach();
}
*/


///*

inline operator  _variant_t() const
{
	_variant_t va;
	const_cast<COM_IDispatchAdapter*>(this)->do_get(&va);
	return va;
}

inline operator  IDispatch*() 
{
	HRESULT hr;
    m_tmp.Clear();
	const_cast<COM_IDispatchAdapter*>(this)->do_get(&m_tmp);
	VARIANT_IN::hr()=hr=VariantChangeType(&m_tmp,&m_tmp,0,VT_DISPATCH);
	if(SUCCEEDED(hr))
	return m_tmp.pdispVal;

	return NULL;
}

inline operator  VARIANT_IN()
{
    m_tmp.Clear();
   do_get(&m_tmp);
   return m_tmp;   
}

//_variant_t m_tmp;
//*/

//*/
//
/*
inline operator COM_IDispatchAdapter()
{
  return *this;
}
//*/


};


template <class Owner, class PropAdapter>
struct COM_prepare_invoke:public COM_prepare_invoke_base<Owner>
{
   
    
	

	COM_prepare_invoke(Owner* o=NULL,DISPID dispid=DISPID_VALUE,DWORD prop_mask=0)
		:COM_prepare_invoke_base(o,dispid,prop_mask)
	{ }


	PropAdapter __cdecl operator()(int argc,VARIANT_IN* pargv)
	{
		PropAdapter* paa=NULL;
		save_params(argc,pargv);
        PropAdapter adapter(this);
		return adapter;
	};

inline PropAdapter do_prepare(int argc,VARIANT_IN* pargv)
{
	save_params(argc,pargv);
	PropAdapter adapter(this);
	return adapter;
};


};

template <class Owner>
struct PA_COM_prepare_invoke: public COM_prepare_invoke<Owner,COM_IDispatchAdapter<Owner> >
{
	typedef  COM_IDispatchAdapter<Owner>   PropAdapterType;
	PA_COM_prepare_invoke(Owner* o=NULL,DISPID dispid=DISPID_VALUE,DWORD prop_mask=0)
		:COM_prepare_invoke(o,dispid,prop_mask){ };
};

