#pragma once
#include "conio.h"
#include "comutil.h"
#include "dispcall/typedef_traits.h"

#ifndef ATL_NO_VTABLE
#define ATL_NO_VTABLE
#endif 

namespace MBVLocalHost
{

typedef  ULONG64  HANDLE64;

   template <class T>
   struct com_ptr
   {

   inline T* operator=(T* t)
	   {
		   if(p!=t)
		   {
			   T* ps=p;
			   p=t;
			   if(t) t->AddRef();
			   if(ps) ps->Release();
		   }   
		   return t;
	   }

    com_ptr(T* t=NULL)
	{    
		p=t;
		if(t) t->AddRef();
    };

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
	inline T** address()
	{
         Release();
		return &p;
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

	inline T* Attach(T* t)
	{
		Release();
		return p=t;
	}

    T* p;   
   };






	template <class T,class locker_ptr=com_ptr<T>, class Intf=IUnknown, const IID& iid=__uuidof(Intf)>
	struct MBVHolderI: public Intf
	{
        
		typedef typename MBVHolderI<T,locker_ptr,Intf,iid>  MBVHolderType;
		 

		STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
		{
			if ((riid == IID_IUnknown)||(riid == iid))
			{
				*ppvObject = static_cast<void*>(this);
				AddRef();
				return S_OK;
			}
			return E_NOINTERFACE;
		}


		STDMETHOD_(ULONG,AddRef)()
		{
			return InterlockedIncrement(&m_refcount);
		}
		STDMETHOD_(ULONG,Release)()
		{

			ULONG l = InterlockedDecrement(&m_refcount);
			if (l == 0)
				delete this;
			return l;
		}
		MBVHolderI():m_refcount(255),m_plocked(NULL){};

		inline T* Detach()
		{
              T* t=m_plocked;
			  m_plocked=NULL;
			return t;
		}

		MBVHolderI(T* punk,long rc=1):m_refcount(rc)
		{		
			m_plocked=punk;
		}

		virtual		~MBVHolderI()	{};

		inline  IUnknown* GetUnknown()
		{
			return static_cast<IUnknown*>(this);
		}
            
		LONG volatile m_refcount;
     	 locker_ptr m_plocked;
		

	};

/*
template <class Intf=IUnknown, const IID& iid=__uuidof(Intf)>
	struct com_ptr_stub_locker:com_ptr<Intf>
	{
		enum
		{
          is_fake=false,
		  is_not_fake=true
		};

inline	HRESULT   stub_create(Intf* pUnk)
		{
			HRESULT hr;
			if(!pUnk)
    		if(FAILED(hr=CreateStreamOnHGlobal(0,true,&m_psrvlock_stub))) return hr;

			if(FAILED(hr=CoMarshalInterface(m_psrvlock_stub,iid,pUnk,MSHCTX_LOCAL,0,MSHLFLAGS_NORMAL))) 
				return hr;
			LARGE_INTEGER ll=LARGE_INTEGER();
			ULARGE_INTEGER lr=ULARGE_INTEGER();
			m_psrvlock_stub->Seek(ll,STREAM_SEEK_SET,&lr);
			return hr;
		}

inline void stub_free()
{
	Release();
	if(m_psrvlock_stub.p)
		CoReleaseMarshalData(m_psrvlock_stub); 
}

inline  Intf* operator=(Intf* i)
{
  if(p!=i)
  {
   stub_free();
   p=i;
   stub_create(p);
  }
  return p;
}
com_ptr_stub_locker():com_ptr(){};

com_ptr_stub_locker(Intf* pi):com_ptr(pi)
{
	stub_create(pi);
};
~com_ptr_stub_locker()
 { 
	 stub_free();
 }

inline 	 operator Intf*()
{
	return p;
}

inline HRESULT out_unknown(VARIANT* pv)
{
	HRESULT hr;
	VARIANT r={VT_UNKNOWN};
	if(!p) return E_NOTIMPL;
	hr=p->QueryInterface(__uuidof(IUnknown),&r.byref);
	if(SUCCEEDED(hr))  *pv=r; 
	return hr;

}

        com_ptr<IStream> m_psrvlock_stub;
};


struct  //__declspec(uuid("00000000-0000-0000-0000-000000000000"))
	ifake
{
	//inline void Release(){};
	//inline void AddRef(){};
	
};

//template <const IID& iid>
template <>
struct com_ptr_stub_locker<ifake,__uuidof(IUnknown)>
{
	enum
	{
		is_fake=true,
		is_not_fake=false
	};
	com_ptr_stub_locker(){};
	inline operator ifake*(){return NULL;};
	inline HRESULT out_unknown(VARIANT* pv)	{		return E_NOTIMPL;};

};

//DECL_TYPE_SELECTOR_PRFX(TYPESELECTOR,external_driver_type)
//#define EXTERNAL_INTF_SELECTOR TYPE_SELECT_IF(TYPESELECTOR,external_driver_type)
*/

template <class T,bool f,HRESULT hRNO=S_FALSE>  //E_NOTIMPL> 
struct external_connector
{
	enum	{ is_connect=false};
	external_connector(T* t){};
	inline HRESULT out_unknown(ULONG ,VARIANT* )	{return hRNO;}
	inline    HRESULT  unmarshal(VARIANT& v){  return hRNO;}
	
};

template <class T,HRESULT hRNO> 
struct external_connector<T,true, hRNO>
{
	typedef typename T::external_driver_type external_driver_type;
	T* m_t;

	enum{ is_connect=true};
	external_connector(T* t):m_t(t){};

 inline   HRESULT  unmarshal(VARIANT& v)
 {
	 HRESULT hr;
void** ppdriver=m_t->get_external_driver_address();
if(ppdriver)
{
   	if( ((v.vt==VT_UNKNOWN)||(v.vt==VT_DISPATCH))&&(v.byref))
	{
        *ppdriver=NULL;
       //hr=v.punkVal->QueryInterface(__uuidof(external_driver_type),(void**)&(m_t->m_external_driver)); 
		hr=v.punkVal->QueryInterface(__uuidof(external_driver_type),ppdriver);  
	   return hr;
	}
}
    return hRNO;
     //m_t->m_external_driver
 }

	inline HRESULT out_unknown(ULONG pid,VARIANT* pv)
	{
		HRESULT hr=hRNO;

		VARIANT r={VT_UNKNOWN};
		//if(!m_t) return E_NOTIMPL;
		IUnknown* pdriver=m_t->get_external_driver(pid);
		if(pdriver)
		{

        //if(m_t->m_external_driver)     		hr=m_t->m_external_driver->QueryInterface(__uuidof(IUnknown),&r.byref);
   	    	hr=pdriver->QueryInterface(__uuidof(IUnknown),&r.byref);
    		if(SUCCEEDED(hr))  *pv=r; 
		}
		return hr;

	}
};


	template <class T>
	struct MBVCallBack: public MBVHolderI<T,com_ptr<T>,IDispatch>
	{
	public:
		typedef typename MBVCallBack<T>  MBVHolderType;

     
		 
        DECL_IS_TYPEDEFER(selectorT,external_driver_type);

		 MBVCallBack():MBVHolderI(){};
		 MBVCallBack(T* punk,long rc):MBVHolderI(punk,rc){};

		STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)	{if (pctinfo) *pctinfo = 0;	return S_OK;}
		STDMETHOD(GetTypeInfo)(UINT,LCID,ITypeInfo**){return E_NOTIMPL;}
		STDMETHOD(GetIDsOfNames)(REFIID,LPOLESTR*,UINT,LCID,DISPID*){return E_NOTIMPL;}


		STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
			LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
			EXCEPINFO* pexcepinfo, UINT* puArgErr)
		{
			HRESULT hr;
			if(!pvarResult)  return S_OK;

           if(pdispparams->cArgs==0) return DISP_E_BADPARAMCOUNT;

			const bool is_driver=selectorT<T>::result;

			if(is_driver&&(dispidMember==1))
			{
				external_connector<T,is_driver> ec((T*)m_plocked);
				return hr=ec.out_unknown(pdispparams->rgvarg[0].intVal,pvarResult);
			}



//         bool f=external_locker::is_not_fake;
//		 hr=m_external_object.out_unknown(pvarResult);

			if(dispidMember==DISPID_VALUE) 
			{
		      
              hr=m_plocked->IMarshalLocalHost_MarshalToClientProcess(pdispparams->rgvarg[0].intVal,pvarResult);
				return hr;
			}

			


/*
			if(external_locker::is_not_fake&&(dispidMember==1))
			{
				return hr=m_external_object.out_unknown(pvarResult);
				
			}
*/
             return DISP_E_MEMBERNOTFOUND;
		};
      //external_locker m_external_object;

	};





template<class MBVHolderType,class T>
	 HRESULT LockToStream(IStream* pstrm,T* punk,DWORD dwDestContext,
		void  *pvDestContext,DWORD mshlflags)
	{
		HRESULT hr;
		//MBVHolderType *ph=new MBVHolderType(punk,1);
		com_ptr<MBVHolderType> ph;
		 ph.p=new MBVHolderType(punk,1);
		//CComPtr<IUnknown> unk;
		//IUnknown *p=0;
		//ph->QueryInterface(__uuidof(IUnknown),(void**)&p);
		hr=CoMarshalInterface(pstrm,__uuidof(IUnknown),ph->GetUnknown(),dwDestContext,pvDestContext,mshlflags);
		//ph->Release();
		return hr;
	};

template<class MBVHolderType>
	 HRESULT AddMarshalSizeMax(DWORD dwDestContext,
		void __RPC_FAR *pvDestContext,
		DWORD mshlflags,
		DWORD __RPC_FAR *pSize)
	{
		HRESULT hr;
		DWORD siz=0;
		MBVHolderType blank;//(NULL,255); 
		hr=CoGetMarshalSizeMax(&siz,__uuidof(IUnknown),blank.GetUnknown(),dwDestContext,pvDestContext,mshlflags);
		if(SUCCEEDED(hr))
			(*pSize)+=siz;
		return hr;
	}

inline 	 HRESULT UnlockFromStream(IStream* pstrm)
	 {
		 HRESULT hr;
		 return hr=CoReleaseMarshalData(pstrm);
	 };

inline 	 HRESULT UnmarshalCallbackData(IStream* pstrm,VARIANT* pv=NULL,VARIANT* pv1=NULL)
{
	HRESULT hr;
	
	  //MBVHolderI<IDispatch> disp;//(NULL,255);
	  com_ptr<IDispatch> disp;

	if(SUCCEEDED(hr=CoUnmarshalInterface(pstrm,__uuidof(IDispatch),disp.ppQI())))
	{
	
		VARIANT V={VT_I4};
		V.intVal=GetCurrentProcessId();
       DISPPARAMS dp = {&V, NULL, 1, 0};
       if(pv1)
	   {
           //DISPPARAMS dp = {NULL, NULL,0, 0};		   
		   hr=disp->Invoke(1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dp,pv1, NULL, NULL);
	   }
	   hr=disp->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dp,pv, NULL, NULL);

	
	}
    else hr= UnlockFromStream(pstrm);
	return hr;
};




	template <class T,const CLSID& clsid,bool fhold=true,class MBVHolder=MBVCallBack<T> >//MBVHolderI<T> >
	class ATL_NO_VTABLE IMarshalLocalHostImpl : public IMarshal
	{
	public:
		
		IMarshalLocalHostImpl(){

		}

		// IMarshal
#define CHECK_FAIL_CONTEXT(dwDestContext)  if(!( pT->IMarshalLocalHost_CheckContext(dwDestContext)) ) return E_FAIL;


		bool IMarshalLocalHost_CheckContext(DWORD dwDestContext)
		{
            bool f=(dwDestContext!=MSHCTX_DIFFERENTMACHINE);  
			return f;
		}

		STDMETHOD(GetUnmarshalClass)
			(
			REFIID riid, 
			void __RPC_FAR *pv, 
			DWORD dwDestContext, 
			void __RPC_FAR *pvDestContext, 
			DWORD mshlflags, 
			CLSID __RPC_FAR *pCid
			)
		{
			
			T* pT = static_cast<T*>(this);
          CHECK_FAIL_CONTEXT(dwDestContext);


		  return pT->IMarshal_GetUnmarshalClass(riid, pv, dwDestContext,pvDestContext, mshlflags,pCid);
		}
		STDMETHOD(ReleaseMarshalData)
			(
			IStream __RPC_FAR *pStm
			)
		{
			
			T* pT = static_cast<T*>(this);
			return pT->IMarshal_ReleaseMarshalData(pStm);
		}
		STDMETHOD(DisconnectObject)
			(
			DWORD dwReserved
			)
		{
			
			T* pT = static_cast<T*>(this);
			return pT->IMarshal_DisconnectObject(dwReserved);
		}

		STDMETHOD(GetMarshalSizeMax)
			( 
			REFIID riid,
			void __RPC_FAR *pv,
			DWORD dwDestContext,
			void __RPC_FAR *pvDestContext,
			DWORD mshlflags,
			DWORD __RPC_FAR *pSize
			)
		{
			
			

			HRESULT hr;
			T* pT = static_cast<T*>(this);
			CHECK_FAIL_CONTEXT(dwDestContext);
			 hr=pT->IMarshalLocalHost_GetMarshalSizeMax(riid, pv, dwDestContext, 
				pvDestContext, mshlflags, pSize,0);
             if(FAILED(hr)) return hr; 
              if(fhold)  
				  hr=AddMarshalSizeMax<MBVHolder>(dwDestContext,pvDestContext,mshlflags,pSize);
               return hr;
		}
		STDMETHOD(MarshalInterface)
			( 
			IStream __RPC_FAR *pStm,
			REFIID riid,
			void __RPC_FAR *pv,
			DWORD dwDestContext,
			void __RPC_FAR *pvDestContext,
			DWORD mshlflags
			)
		{
			

			HRESULT hr;
			T* pT = static_cast<T*>(this);
			CHECK_FAIL_CONTEXT(dwDestContext);

			hr= pT->IMarshalLocalHost_MarshalInterface(pStm, riid, pv, dwDestContext,
				pvDestContext, mshlflags,0);
			if(FAILED(hr)) return hr; 
			IUnknown* punk=pT->GetUnknown();
			if(fhold)  
				hr=LockToStream<MBVHolder>(pStm,pT,dwDestContext,pvDestContext,mshlflags);
			return hr;
		}
		STDMETHOD(UnmarshalInterface)
			( 
			IStream __RPC_FAR *pStm,
			REFIID riid,
			void __RPC_FAR *__RPC_FAR *ppv
			)
		{
			HRESULT hr,hrr;
			
			T* pT = static_cast<T*>(this);
			
			hr=pT->IMarshalLocalHost_UnmarshalInterface(pStm, riid, ppv);

            hrr=UnlockFromStream(pStm);  

			return hr;
		}

		HRESULT IMarshalLocalHost_MarshalToClientProcess(DWORD ClientProcessId,VARIANT* pMarshalData)
		{
			return E_NOTIMPL;
		}

		HRESULT IMarshal_GetUnmarshalClass
			(
			REFIID riid, 
			void __RPC_FAR *pv, 
			DWORD dwDestContext, 
			void __RPC_FAR *pvDestContext, 
			DWORD mshlflags, 
			CLSID __RPC_FAR *pCid
			)
		{
		
              *pCid=clsid;
			return S_OK;
		}
		HRESULT IMarshal_ReleaseMarshalData(IStream __RPC_FAR *pStm	)
		{
			HRESULT hrr;
            //Beep(1777,777);
			//
			return S_OK;
		}
		HRESULT IMarshal_DisconnectObject(DWORD dwReserved)
		{
			return S_OK;
		}





/*
  inline bool context_fail(DWORD dwDestContext)
  {
         T* pT = static_cast<T*>(this);
		 return pT->IMarshalLocalHost_CheckContext(DWORD dwDestContext)
  }

*/


		HRESULT IMarshalLocalHost_GetMarshalSizeMax
			( 
			REFIID riid,
			void __RPC_FAR *pv,
			DWORD dwDestContext,
			void __RPC_FAR *pvDestContext,
			DWORD mshlflags,
			DWORD __RPC_FAR *pSize,
			DWORD mshlopt
			)
    		{
				*pSize=0;
			return S_OK;
			}
		HRESULT IMarshalLocalHost_MarshalInterface
			( 
			IStream __RPC_FAR *pStm,
			REFIID riid,
			void __RPC_FAR *pv,
			DWORD dwDestContext,
			void __RPC_FAR *pvDestContext,
			DWORD mshlflags,
			DWORD mshlopt
			)
		{
				return S_OK;
		}
		HRESULT IMarshalLocalHost_UnmarshalInterface
			( 
			IStream __RPC_FAR *pStm,
			REFIID riid,
			void __RPC_FAR *__RPC_FAR *ppv
			)
		{
			return E_NOTIMPL;
		}

};


template <class T,const CLSID& clsid,class Creator>//MBVHolderI<T> >
class ATL_NO_VTABLE IMarshalPureMBV: public IMarshalLocalHostImpl<T,clsid,false> 
{
  public:
	  bool IMarshalLocalHost_CheckContext(DWORD dwDestContext){return true;};
	IMarshalPureMBV(){};
	HRESULT IMarshalLocalHost_UnmarshalInterface
		( 
		IStream __RPC_FAR *pStm,
		REFIID riid,
		void __RPC_FAR *__RPC_FAR *ppv
		)
	{
	 HRESULT hr;
	 Creator* pc=0;
	 if(SUCCEEDED(hr=Creator::CreateInstance(&pc)))
	       hr=pc->QueryInterface(riid,ppv);

	  if(FAILED(hr)) delete pc;

	  return hr;
	};
};

template <class D,bool fextdriver=false>
struct VariantWrapper
{
  HRESULT	hr;
  VARIANT  v,vd;
  bool fclear;
  bool flocked;
  D* pd;

  VariantWrapper(IStream* pstrm)
  {
	  flocked=0;
	  pd=NULL;
	   v.vt=VT_EMPTY;
	  //v=VARIANT();
	  if(fextdriver)
	  {
       vd.vt=VT_EMPTY;
	   fclear=SUCCEEDED(hr=UnmarshalCallbackData(pstrm,&v,&vd));
	  }
	  else 
        fclear=SUCCEEDED(hr=UnmarshalCallbackData(pstrm,&v));
     
	 if(fclear) init();
  }
  VariantWrapper(VARIANT* pv=NULL,bool _fclear=true)
  {
	  hr=E_FAIL; 
	  pd=NULL;
	  if(fextdriver) vd.vt=VT_EMPTY;
	  fclear=_fclear;
	  flocked=false;
	  if(pv) v=*pv;
	  else
	  {
	       v=VARIANT();	  
		   int szD=sizeof(D);
		   if(!(v.parray=SafeArrayCreateVector(VT_UI1,0,szD))) return;
		   v.vt=(VT_UI1|VT_ARRAY);
	  }
	  init();
  }

HRESULT Detach( VARIANT* pvOUT)
{
	if( !pvOUT) return E_POINTER;
	if(FAILED(hr)) return hr;
    if(flocked) SafeArrayUnlock(v.parray);
	fclear=false;
	flocked=false;
    *pvOUT=v;
	v.vt=VT_EMPTY;
	return S_OK;
}

void init()
 {
	 pd=NULL;
	 
     flocked=false;
     if((v.vt=(VT_UI1|VT_ARRAY))&&(v.parray))
	 {
		 hr=E_FAIL;
		 if(SafeArrayGetDim(v.parray)!=1) return;
		 long l=v.parray->rgsabound->cElements;
		 
		 //if(FAILED(SafeArrayGetLBound(v.parray, 1, &lsize))) return;
		 int szD=sizeof(D);
		 if(l<sizeof(D)) return;
		 SafeArrayLock(v.parray);
		 l=0;
         if(FAILED(SafeArrayPtrOfIndex(v.parray,&l,(void**)&pd))) return;
		 flocked=true;
		 if(pd) hr=S_OK;
	 }
 }




~VariantWrapper()
{
	if(flocked) SafeArrayUnlock(v.parray);
	if(fclear) VariantClear(&v);
	 if(fextdriver)  VariantClear(&vd);
}

operator D&()
{
	return *pd;
}

inline VARIANT& driver()
{
	return vd;
}

};



template <class T,const CLSID& clsid,typename MarshalData=T::MARSHAL_DATA>
struct IMarshalLocalHostDataImpl:IMarshalLocalHostImpl<T,clsid,true,MBVCallBack<T>>
{
	//	typedef typename T::MARSHAL_DATA MARSHAL_DATA_TYPE;
		typedef typename MarshalData MARSHAL_DATA_TYPE;

		DECL_IS_TYPEDEFER(selectorT,external_driver_type);
		//DECL_IS_TYPEDEFER(selector_2,MARSHAL_DATA_declarator_type);


inline   void** get_external_driver_address(){return NULL;}
inline   IUnknown* get_external_driver(DWORD pid){return NULL;}





template <typename L>
inline static HRESULT _unmarshal_cast(L* pt,MARSHAL_DATA_TYPE& md,REFIID riid,void **ppv,...)
{
	HRESULT hr;
	hr=pt->IMarshalLocalHostData_UnmarshalInterface(md,riid,ppv);
	return hr;
}

template <typename L>
inline static HRESULT _unmarshal_cast(L* pt,MARSHAL_DATA_TYPE& md,REFIID riid,void **ppv,typename L::MARSHAL_DATA_declarator_type* ptt )
{
	HRESULT hr;
	hr=ptt->IMarshalLocalHostData_UnmarshalInterface(md,riid,ppv);
	return hr;
}


	HRESULT IMarshalLocalHost_UnmarshalInterface(IStream *pStm,REFIID riid,	void **ppv)
	{
		HRESULT hr;
		 
		 VariantWrapper<MARSHAL_DATA_TYPE,selectorT<T>::result> vw(pStm);
		if(SUCCEEDED(hr=vw.hr))
		{
           T* pT = static_cast<T*>(this);
           external_connector<T,selectorT<T>::result> ec(pT);
		   hr=ec.unmarshal(vw.driver());
		   if(FAILED(hr)) return hr;
		   //hr=pT->IMarshalLocalHostData_UnmarshalInterface(vw,riid,ppv);
		   hr=_unmarshal_cast(pT,vw,riid,ppv,pT);
		} 

		return hr;
	}
HRESULT IMarshalLocalHostData_UnmarshalInterface( MARSHAL_DATA_TYPE& md ,REFIID riid,void**ppv)
{
				return E_NOTIMPL;
}




template <typename L>
inline static HRESULT _marshal_to_client_cast(L* pt,DWORD ClientProcessId,MARSHAL_DATA_TYPE& md,...)
{
        	HRESULT hr;
   hr=pt->IMarshalLocalHostData_MarshalToClientProcess(ClientProcessId,md);
   return hr;
}

template <typename L>
inline static HRESULT _marshal_to_client_cast(L* pt,DWORD ClientProcessId,MARSHAL_DATA_TYPE& md,typename L::MARSHAL_DATA_declarator_type* ptt )
{
	HRESULT hr;
	hr=ptt->IMarshalLocalHostData_MarshalToClientProcess(ClientProcessId,md);
	return hr;
}



HRESULT IMarshalLocalHost_MarshalToClientProcess(DWORD ClientProcessId,OUT VARIANT* pMarshalData)
{
	HRESULT hr;
     if(!pMarshalData) return E_POINTER;

     VariantWrapper<MARSHAL_DATA_TYPE> vw;
	 if(FAILED(vw.hr)) return vw.hr;
	 //MARSHAL_DATA_TYPE* pmd=vw.pd;
	 T* pT = static_cast<T*>(this);

	 //if(selector_2<T>::result)
	//	 hr=static_cast<T::MARSHAL_DATA_declarator_type*>(pT)->IMarshalLocalHostData_MarshalToClientProcess(ClientProcessId,vw);
	 //else 
	//	 hr=pT->IMarshalLocalHostData_MarshalToClientProcess(ClientProcessId,vw);
	 hr=_marshal_to_client_cast(pT,ClientProcessId,vw,pT);

	 if(SUCCEEDED(hr))
	 {
		 hr=vw.Detach(pMarshalData);
	 }
	 return hr;

}

HRESULT IMarshalLocalHostData_MarshalToClientProcess(DWORD ClientProcessId,OUT MARSHAL_DATA_TYPE& md) 
{
	return E_NOTIMPL;
}

HRESULT IMarshal_ReleaseMarshalData(IStream __RPC_FAR *pStm	)
{
	HRESULT hr;
	//Beep(1777,777);
	//
	
	return hr=UnlockFromStream(pStm);
}

};





#define EXTERNAL_DRIVER_DECLARE(i_type,member) \
	typedef i_type external_driver_type;\
	inline   void** get_external_driver_address(){	return (void**)(&member);}\
	inline external_driver_type* get_external_driver(DWORD pid){return member;}




template <int handle_count=1,class USERDATA=int>
struct MAP_DATA
{
	enum
	{
		count_h=handle_count+1
	};
	
	union
	{
		struct{
			HANDLE64 hmap,hmutex;
			HANDLE64 hrsrvd[handle_count-1];
		};
		struct{HANDLE64 h[count_h];};
	};
	union
	{
	struct _OPTS
	{
		ULONG64 flags;
		DWORD  dwDesiredAccess; 
		DWORD   pid_creator;
		ULONG64 offset;
		ULONG64 size;
		ULONG64 tag_index;

	} opts;
	struct{

	ULONG64 flags;
	DWORD  dwDesiredAccess; 
	DWORD   pid_creator;
	ULONG64 offset;
	ULONG64 size;
	ULONG64 tag_index;
	};
	};

	USERDATA userdata;

	MAP_DATA()
	{
		memset(this,0,sizeof(*this));
	}
	inline void close()
	{ 

		for(int n=0;n<=handle_count;n++) CloseHandle((HANDLE)h[n]);
	}
	inline void clear()
	{
           close();
	}

template <class MAP_DATA_other>
HRESULT DuplicateTo(MAP_DATA_other& mdo,DWORD ClientProcessId=GetCurrentProcessId())
{
	int ch=min(count_h,mdo.count_h);


	struct HCloser
	{
		HANDLE h;
		HCloser(HANDLE ah):h(ah){}
		~HCloser(){CloseHandle(h);}
		operator HANDLE()
		{
			return h;
		}

	};
     DWORD ProcessId=GetCurrentProcessId();

	HCloser hcp=::OpenProcess(PROCESS_DUP_HANDLE,false,ClientProcessId);

	if(!hcp)	return HRESULT_FROM_WIN32(GetLastError());
	
	

	for(int n=0;n<ch;n++)
	{
		if(h[n]==0) continue;
		HANDLE* phs=(HANDLE*)(h+n);
		mdo.h[n]=0;
		HANDLE* phd=(HANDLE*)(mdo.h+n);
		if(!::DuplicateHandle(::GetCurrentProcess(),*phs,hcp,phd,0,false,DUPLICATE_SAME_ACCESS))
			return HRESULT_FROM_WIN32(GetLastError());

	}
    /*
	mdo.dwDesiredAccess=dwDesiredAccess; 
	mdo.offset=offset;
	mdo.size=size;
	mdo.tag_index=tag_index;
	*/
	mdo.opts=opts;
	mdo.userdata=userdata;
    mdo.pid_creator=::GetCurrentProcessId();

   return S_OK;
}

};

template <class _MD>
struct MD_Ptr
{
	_MD* p;
   MD_Ptr(_MD* _p=0)
   {
	   p=_p;
   }
  ~MD_Ptr()
  {
   if(p) p->clear();
  }
_MD*  Detach()
{
    _MD* t=p;
	p=0;
	return t;
}
inline 	_MD* operator->()
{
	return p;
}
};


//template <class Creator,int handle_count=1>
template <class Creator,class _MD=MAP_DATA<> >
struct MARSHAL_DATA_declarator_N
{


	//typedef  MARSHAL_DATA_declarator_N<Creator,handle_count> MARSHAL_DATA_declarator_type;
	enum{};
	
	//typedef MAP_DATA<handle_count> MARSHAL_DATA;
	typedef _MD MARSHAL_DATA;
	typedef  MARSHAL_DATA_declarator_N<Creator,MARSHAL_DATA> MARSHAL_DATA_declarator_type;

	MARSHAL_DATA m_mbv;


	~MARSHAL_DATA_declarator_N()
	{
		m_mbv.close();
	}

	struct LockerPtr
	{
		//MARSHAL_DATA_declarator_type* m_this;
		HANDLE hmutex;
		LockerPtr(MARSHAL_DATA_declarator_type* _this,bool fnolock=false)
		{
			hmutex=(HANDLE)_this->get_MARSHAL_DATA().hmutex;
			if(!fnolock) 
				TryLock(INFINITE);

		}
		~LockerPtr()
		{
			if(hmutex)
				ReleaseMutex(hmutex);
		}

		inline bool TryLock(DWORD timeout=0)
		{
			if(!hmutex) return true;
			return WAIT_OBJECT_0==WaitForSingleObject(hmutex,timeout);
		}

	};

	typedef LockerPtr      ObjectLock_MBV;

	inline MARSHAL_DATA& get_MARSHAL_DATA()
	{
		return m_mbv;
	}


	HRESULT InitializeUnmarshalData()
	{
		return S_OK;
	};
	HRESULT IMarshalLocalHostData_UnmarshalInterface( MARSHAL_DATA& md ,REFIID riid,void**ppv)
	{
		HRESULT hr;
		//Creator::_BaseClass* pc=0;
		Creator* pc=0;
		hr=Creator::CreateInstance(&pc);
		if(FAILED(hr)) return delete pc, hr;
		pc->get_MARSHAL_DATA()=md;
		hr=pc->InitializeUnmarshalData();
		if(FAILED(hr)) return delete pc, hr;
		hr=pc->QueryInterface(riid,ppv);
		if(FAILED(hr)) return delete pc, hr;
		return hr;
		//return Creator::CreateInstance(md,riid,ppv);
	}




inline	HRESULT IMarshalLocalHostData_MarshalToClientProcess(DWORD ClientProcessId,OUT MARSHAL_DATA& mcd) 
{
	return get_MARSHAL_DATA().DuplicateTo(mcd,ClientProcessId);
	//return DuplicateHandlesMBV(mcd,m_mbv,ClientProcessId);
}

template <class MAP_DATA_other>
HRESULT DuplicateMBVFrom(MAP_DATA_other& mdo)
{
   return mdo.DuplicateTo(get_MARSHAL_DATA());
}


/*
inline static HRESULT  DuplicateHandlesMBV(OUT MARSHAL_DATA& mcd,IN MARSHAL_DATA& m_mbv,DWORD ClientProcessId=GetCurrentProcessId()) 
	{

		struct HCloser
		{
			HANDLE h;
			HCloser(HANDLE ah):h(ah){}
			~HCloser(){CloseHandle(h);}
			operator HANDLE()
			{
				return h;
			}

		};



		HCloser hcp=::OpenProcess(PROCESS_DUP_HANDLE,false,ClientProcessId);


		if(!hcp)	return HRESULT_FROM_WIN32(GetLastError());

		mcd=m_mbv; 
		mcd.pid_creator=::GetCurrentProcessId();

		for(int n=0;n<=handle_count;n++)
		{
			if(mcd.h[n]==0) continue;
			if(!::DuplicateHandle(::GetCurrentProcess(),m_mbv.h[n],hcp,mcd.h+n,0,false,DUPLICATE_SAME_ACCESS))
				return HRESULT_FROM_WIN32(GetLastError());

		}

		return S_OK;
	}
*/
};


};