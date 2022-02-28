#pragma once
#include "conio.h"
//#include "comutil.h"
#include "../tls/tls_dll.h"
#include "../pp/copier.h"
#include "dispex.h"
//#include "ltx.h"

   template <class T>
   struct com_ptr
   {

   inline T* operator=(T* t)
	   {
		   if(p==t) return p;
		   
	   	   if(p) p->Release();
		    p=t;
		   if(t) t->AddRef();
		   
		   return t;
	   }

  inline T* operator=(const com_ptr<T>& ct)
  {
       (*this)=ct.p;
	   return p;
  }
    com_ptr(T* t=NULL)
	{    
		p=t;
		if(t) t->AddRef();
    };
	com_ptr( const com_ptr<T>& cp) 
	{
       if(p=cp.p) p->AddRef();

	}
	

	inline void Release()
	{
         if(p) p->Release();
		 p=NULL;
	}
	~com_ptr()
	{
       if(p) p->Release();
	};

	inline 	T* operator->()
	{
		return p;
	}
	inline 	 operator T*()
	{
		return p;
	}

	inline void** ppQI()
	{
         Release();
		return (void**)&p;
	}

	inline void** GetAddress()
	{
		
		return ppQI();
	}

	T& operator*() const
	{

		return *p;
	}
	inline T* Detach()
	{
		T* t=p;
		p=NULL;
		return t;
	}

	inline com_ptr<T>& Attach(T* t)
	{
		Release();
		 p=t;
		 return *this;
	}

	HRESULT CopyTo(T** pp)
	{
		if(!pp) return E_POINTER;
		*pp=p;
		if(p) p->AddRef();
		return S_OK;
	};

	 HRESULT CoCreateInstance( REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) 
	{
		HRESULT hr;
        Release();
		return hr=::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
	}

    T* p;   
   };



inline HRESULT get_global_table(IGlobalInterfaceTable** ppGIT)
{
	HRESULT hr;
	if(!ppGIT) return E_POINTER;
   IGlobalInterfaceTable* pGIT=tls_var<IGlobalInterfaceTable>();
	if(!pGIT) 
	{
		hr=CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IGlobalInterfaceTable), (void**)&pGIT);
		if(FAILED(hr)) return hr;         
            tls_var<IGlobalInterfaceTable>()=pGIT;
		
	}
   *ppGIT=pGIT;
   return S_OK;
};

template <class T,class com_exeption>
struct com_GIT_ptr //: ClonerT<com_GIT_ptr<T,com_exeption> >
{
	com_GIT_ptr():m_cookie(0) {};

inline static HRESULT check_err(HRESULT hr)
{
	if(FAILED(hr)) throw com_exeption(hr);
	return hr;
};

inline HRESULT CopyTo(T ** pp)
{
 return getGIT()->GetInterfaceFromGlobal(m_cookie,__uuidof(T),(void**)pp);
}

inline  operator com_ptr<T>() 
 {
   com_ptr<T> cp;
   check_err(CopyTo(&cp.p));
   return cp;
 }
/*
inline void clone_to(com_GIT_ptr<T,com_exeption> * cl)
{
  com_ptr<T> cp=*this;
  *cl=cp.p;
}
*/
operator DWORD()
{
	return m_cookie;
}
static inline IGlobalInterfaceTable* getGIT() 
{
   IGlobalInterfaceTable* pGIT=0;
  check_err(get_global_table(&pGIT));
  return pGIT;
}

inline void operator=(T  * p)
{
 clear();
 if(p) check_err(getGIT()->RegisterInterfaceInGlobal(p,__uuidof(T),&m_cookie));
}

inline com_GIT_ptr<T,com_exeption>& operator=( const com_GIT_ptr<T,com_exeption>& cl)
{

	if(cl.m_cookie)
	{
         //this->operator =(com_ptr<T>(cl));
		com_ptr<T> t;
		check_err(getGIT()->GetInterfaceFromGlobal(cl.m_cookie,__uuidof(T),t.ppQI()));
        this->operator =(t);
	}
	return *this;
}


com_GIT_ptr( const com_GIT_ptr<T,com_exeption> & cl)
{
  m_cookie=0;
  if(cl.m_cookie)
  {
	  this->operator =(cl);
  }

}


inline void  clear()
{
  IGlobalInterfaceTable* pGIT=0;
	if((m_cookie)&&SUCCEEDED(get_global_table(&pGIT)))
	{
		pGIT->RevokeInterfaceFromGlobal(m_cookie);
		
	}
	m_cookie=0;
};

~com_GIT_ptr()
{
	clear();
}
	DWORD m_cookie;
};



inline BSTR BSTR_from_pchar(char* pstr)
{
  if(!pstr) return 0;
  size_t l=strlen(pstr);
  BSTR bres=SysAllocStringLen(0,l);
  int ff= MultiByteToWideChar(CP_THREAD_ACP,0,pstr,l,bres,l);
  return bres;

}





template <class ArgList,class val_type=VARIANT>
struct DispParams:DISPPARAMS
{
  DISPID putid;
  
  DispParams(VARIANT* buffer, int argc,const ArgList& parg,val_type* const pput_value=0)
  {
     rgvarg=buffer;
	 
     if(pput_value)
	 {
        rgdispidNamedArgs=&(putid= DISPID_PROPERTYPUT);
		*(buffer++)=*pput_value;
         cArgs=argc+1;
		 cNamedArgs=1;
	 }
	 else
	 {
		 cArgs=argc;
		 cNamedArgs=0;
         rgdispidNamedArgs=0;
	 }

      copy_to_collection(argc,parg,buffer,true);
  }
 
inline DispParams& add(VARIANT& v)
{
  VARIANT* plast=rgvarg+cArgs++;
   *plast=v;
   return *this;
};

};



struct MultiThreadModel
{
	inline 	static long Increment(long* p)  {return InterlockedIncrement(p) ;}
	   static long  Decrement(long* p) {return InterlockedDecrement(p);}
	
};

template <class T, const IID* piid=&IID_IUnknown,class ThreadingModel=MultiThreadModel>
class IUnknown_simple_impl : public T
{
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
	{
		if ((riid == IID_IUnknown) || (riid == *piid))
		{
			*ppvObject = (void*)static_cast<T*>(this);
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

inline T* thisT()
{
  return static_cast<T*>(this);
}
inline long* pref_count()
{
	return 0;
}

	STDMETHOD_(ULONG,AddRef)()
	{ 
		long * pref=thisT()->pref_count();
		if(pref) return ThreadingModel::Increment(pref);
		return 1; 
	}
	STDMETHOD_(ULONG,Release)()
	{
		long * pref=thisT()->pref_count();
		if(pref) return ThreadingModel::Decrement(pref);
		return 1;
	}
};


#define DISPATCH_GETSF (DISPATCH_PROPERTYGET|DISPATCH_METHOD)
#define DISPATCH_PUTSF (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)


#define is_get(f) (f&DISPATCH_GETSF)
#define is_put(f) (f&DISPATCH_PUTSF)



template <int STACK_BUFFSIZE=64>
struct STACK_BUFFER
{
	typedef STACK_BUFFER<STACK_BUFFSIZE> sbuf_type;


	void *pcontext;
	//void *presult;
	void* ptrs[STACK_BUFFSIZE];
  

	STACK_BUFFER(void* ptr,DISPPARAMS* pdp, void* pfill=&VNOPARAM):pcontext(ptr)
	{
		//for(int i=0;i<STACK_BUFFSIZE;i++) ptrs[i]=pfill;
		
		int cf=pdp->cNamedArgs;
		int cb=pdp->cArgs;
		int cbf=cb-cf;
		
		if((cf==1)&&(pdp->rgdispidNamedArgs)&&(pdp->rgdispidNamedArgs[0]==DISPID_PROPERTYPUT))
		{
           ptrs[0]=pdp->rgvarg;
		}
		
		for(int n=cf;n<cb;n++)
		{
			ptrs[n]=&(pdp->rgvarg[cb-n]);
		}
		for(void** p=ptrs+cb;p!=ptrs+STACK_BUFFSIZE;p++) *p=pfill;  
	};

};




template <class T,class Unk=IUnknown_simple_impl<IDispatch,&__uuidof(IDispatch)> >
struct IDispatch_simple_impl
{


	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
	{
		if (pctinfo == NULL) return E_POINTER; 
		*pctinfo = 0;
		return S_OK;
	}
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
	{
		if (pptinfo == NULL) return E_POINTER; 
		return E_NOTIMPL;
	}

	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
		LCID lcid, DISPID* rgdispid)
	{
		T* thisT=static_cast<T*>(this);
		return thisT->inner_GetDispID(*rgszNames,thisT->dflt_flags_dex(),rgdispid);
	
	}

	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
		T* thisT=static_cast<T*>(this);
		return thisT->inner_invoke(dispidMember,wFlags,pdispparams,pvarResult,pexcepinfo);
				
	}
//======================================================
inline DWORD dflt_flags_dex()
{
  return 0;
}
inline HRESULT inner_invoke(DISPID dispid,WORD wFlags,DISPPARAMS* pdispparams, VARIANT* pvarResult,EXCEPINFO* pexcepinfo )
{
	return E_NOTIMPL;
}
inline HRESULT inner_GetDispID(BSTR name,DWORD grfdex,DISPID* rgdispid)
{
	return E_NOTIMPL;
}

};

#include <exception>

template <class com_variant,class com_exception>
struct wrapper_base:IDispatch_simple_impl<wrapper_base<com_variant,com_exception> >
{

    typedef    STACK_BUFFER<64> stack_buf_type;

	void* m_context;
	int m_flags;
	DISPID m_dispid;
	typedef void* N;
	union
	{
     N procn;
     com_variant ( *get_call_type)(stack_buf_type);
	 void ( *put_call_type)(stack_buf_type);
	};
 template <typename N>
    wrapper_base(int flags,N nproc,void *pcntxt):m_flags(flags),m_context(flags),procn(nproc),m_dispid(di){};

	inline HRESULT inner_invoke(DISPID dispid,WORD wFlags,DISPPARAMS* pdispparams, VARIANT* pvarResult,EXCEPINFO* pexcepinfo )
	{
      if((dispid==m_dispid)&&(wFlags&m_flags))  
	  { 
		try
		{

		}
		catch(com_exception& e)
		{

		}
		catch(std::exception& e)
		{

		}
		catch(...)
		{
			return E_FAIL;

		}
		return S_OK;
	  }

		return E_NOTIMPL;
	}

};