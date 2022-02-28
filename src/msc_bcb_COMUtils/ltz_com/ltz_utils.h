#pragma once
#include <windows.h>
#include "tls_dll.h"
#include "static_constructors_0.h"

namespace saturnz
{

namespace com_utils
{
	class com_init_t 
	{

	public:
		HRESULT hr;
		com_init_t(DWORD tm=COINIT_APARTMENTTHREADED)
		{
			hr=CoInitializeEx(NULL,tm);
		};
		~com_init_t()
		{
			if(SUCCEEDED(hr)) CoUninitialize();
		};

	inline static HRESULT apartment_type(DWORD* ptype)
		{
			if(NULL==ptype) return E_POINTER;
			com_init_t com(COINIT_APARTMENTTHREADED);
			HRESULT hr=com.hr;
			if(hr==S_OK) return CO_E_NOTINITIALIZED;
			else if(hr==RPC_E_CHANGED_MODE) *ptype=COINIT_MULTITHREADED;
			else if(hr==S_FALSE) *ptype=COINIT_APARTMENTTHREADED;
			else return E_UNEXPECTED;
			return S_OK;
		};

	};

	template <class T>
	struct com_ptr_t
	{		

		inline T* operator=(T* t){ return reset(t); }

		inline T* reset(T* t)
		{
			if(p==t) return p;

			if(p) p->Release();
			p=t;
			if(t) t->AddRef();

			return t;
		}

		inline T* operator=(const com_ptr_t<T>& ct)
		{
			(*this)=ct.p;
			return p;
		}
		com_ptr_t(T* t=NULL)
		{    
			p=t;
			if(t) t->AddRef();
		};
		com_ptr_t( const com_ptr_t<T>& cp) 
		{
			if(p=cp.p) p->AddRef();

		}


		inline void Release()
		{
			if(p) p->Release();
			p=NULL;
		}
		~com_ptr_t()
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

		
		inline T* Detach()
		{
			T* t=p;
			p=NULL;
			return t;
		}

		inline com_ptr_t<T>& Attach(T* t)
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
		template<class Intf>
		HRESULT QueryInterface(Intf** ppvObject)
		{
			HRESULT hr;
			if(p) return hr=p->QueryInterface(__uuidof(Intf),(void**)ppvObject);
			else return hr=E_POINTER;
		}

		HRESULT CoCreateInstance( bstr_t bclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) 
		{
			HRESULT hr;
			CLSID rclsid;
			if(SUCCEEDED(hr=CLSIDFromString(bclsid,&rclsid)))
			return hr=CoCreateInstance(rclsid, pUnkOuter, dwClsContext);
		}

		HRESULT CoCreateInstance( REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) 
		{
			HRESULT hr;
			Release();
			return hr=::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
		}

		T* p;   
	};

    template<class I>
    inline HRESULT toVARIANT(I* pintf,VARIANT* pv)
	{
        if(!(pintf&&pv))return E_POINTER;
		VARIANT v={VT_UNKNOWN};
		HRESULT hr_di,hr;
		  hr=pintf->QueryInterface(__uuidof(IUnknown),&v.byref);

		if(SUCCEEDED(hr))
		{
          hr_di=VariantChangeType(&v,&v,0,VT_DISPATCH);
          *pv=v;
		}
		return hr;
	}
/*
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

*/
	struct GIT_s
	{
		HRESULT hr;
		DWORD tid;
		com_ptr_t<IGlobalInterfaceTable> GIT;
		GIT_s():tid(GetCurrentThreadId())
		{
			com_init_t ci(0);
			hr=GIT.CoCreateInstance(CLSID_StdGlobalInterfaceTable);	  

		}
		inline operator IGlobalInterfaceTable*(){return GIT;}

		template<class Intf>
		inline     HRESULT wrap(Intf* p,DWORD* pdw,bool frelease=false)
		{
			HRESULT hr;
			if(!p) return E_POINTER;


			hr=GIT->RegisterInterfaceInGlobal(p,__uuidof(Intf),pdw);
			if(frelease&&SUCCEEDED(hr)) p->Release();

			return hr;

		};    

        template<class Intf>
		inline   HRESULT unwrap(DWORD dw,Intf** pp,bool frevoke=true)
		{
			HRESULT hr,hr0;
			hr=(pp)?GIT->GetInterfaceFromGlobal(dw,__uuidof(Intf),(void**)pp):S_FALSE;
			if(frevoke&&SUCCEEDED(hr))
				hr0=GIT->RevokeInterfaceFromGlobal(dw);
			return hr;
		}

		inline   HRESULT unwrap(DWORD dw)
		{
			com_init_t ci(0);
			HRESULT hr0;
			return hr0=GIT->RevokeInterfaceFromGlobal(dw);
		}

		inline static GIT_s& get_GIT()
		{
			return class_initializer_T<GIT_s>().get();
		}



	};




	template <class T,class com_exeption>
	struct com_GIT_ptr //: ClonerT<com_GIT_ptr<T,com_exeption> >
	{

		struct fakeUnknown_t
		{

			fakeUnknown_t():cookie(0),refcount(1){}

			LONG AddRef(){ return InterlockedIncrement(&refcount); }
			LONG Release(){ return InterlockedDecrement(&refcount); }
			volatile LONG refcount;
			DWORD  cookie;

		};
		com_GIT_ptr():m_cookie(0),hr(E_NOINTERFACE) {};

		inline static HRESULT check_err(HRESULT hr)
		{
			if(FAILED(hr)) throw com_exeption(hr);
			return hr;
		};
		inline IGlobalInterfaceTable* GIT()
		{
          return GIT_s::get_GIT().GIT;
		}

        template<class I> 
		inline HRESULT CopyTo(I ** pp)
		{
			return GIT_s::get_GIT().unwrap(m_cookie,pp,false);
				//;->GetInterfaceFromGlobal(m_cookie,__uuidof(I),(void**)pp);
		}
		inline HRESULT CopyTo(VARIANT* pv)
		{
		    	HRESULT hr;
				if(!pv) return E_POINTER;
				IUnknown* punk=0;
				if(SUCCEEDED(hr=CopyTo(&punk)))
				{
                    hr=toVARIANT(punk,pv);
					punk->Release();
				}
		   return hr;
		}

		inline  operator com_ptr_t<T>() 
		{
			com_ptr_t<T> cp;
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
		

		inline void attach(DWORD dw)
		{
			clear();
			m_cookie=dw;
		}

		inline DWORD dettach()
		{
			DWORD dw=m_cookie;
			m_cookie=0;
			return dw;
		}

		
		inline void operator=(T  * p)
		{
			clear();
			//if(p) check_err(GIT_s::get_GIT().GIT->RegisterInterfaceInGlobal(p,__uuidof(T),&m_cookie));
             hr=GIT_s::get_GIT().wrap(p,&m_cookie);
		}

		/*

		inline com_GIT_ptr<T,com_exeption>& operator=( const com_GIT_ptr<T,com_exeption>& cl)
		{

			if(cl.m_cookie)
			{
				//this->operator =(com_ptr<T>(cl));
				com_ptr_t<T> t;
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
		*/


		inline void  clear()
		{
			//IGlobalInterfaceTable* pGIT=0;
			if(m_cookie) hr=GIT_s::get_GIT().unwrap(m_cookie);
			/*
			if((m_cookie)&&SUCCEEDED(get_global_table(&pGIT)))
			{
				//pGIT->RevokeInterfaceFromGlobal(m_cookie);


			}*/
			m_cookie=0;
		};

		~com_GIT_ptr()
		{
			clear();
		}
		DWORD m_cookie;
		HRESULT hr;
		
	};


	inline 	bool is_VARIANT_empty(VARIANT&v )
	{
		VARTYPE t=v.vt;
		return (t==VT_NULL)||(t==VT_EMPTY)||(t==VT_ERROR);
	}

	template <VARTYPE VT >
	struct variant_cast_t
	{
		VARIANT s;
        VARIANT* pv;
		HRESULT hr;
		variant_cast_t(VARIANT& src):pv(&src),hr(0)
	   {
		   {
            VARIANT t={};
			s=t;
		   }
		   if(src.vt!=VT)
		   {
			  hr=VariantChangeType(pv=&s,&src,0,VT);
		   }	   

	   }
	   inline  operator VARIANT*()
	   {
            return pv;
	   }

	   inline  VARIANT* operator->()
	   {
		   return pv;
	   }
	   inline  operator bool()
	   {
		   return SUCCEEDED(hr);
	   }

	   ~variant_cast_t(){ if(s.vt) VariantClear(&s); };

	};

/*

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

		inline T* thisT()
		{
			return static_cast<T*>(this);
		}

		inline bool checkQueryInterface(REFIID riid, void** ppvObject,HRESULT& hr)
		{
			return false;
		}
		STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
		{
			
			HRESULT hr=E_NOINTERFACE;
             
			if(!thisT()->checkQueryInterface(riid,ppvObject,hr))
			if ((riid == IID_IUnknown) || (riid == *piid))
			{
				*ppvObject = (void*)static_cast<T*>(this);
				AddRef();
				return S_OK;
			}

			return hr;
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


}
*/
};
};