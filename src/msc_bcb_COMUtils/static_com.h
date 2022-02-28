#pragma once


#include <windows.h>
#include <windef.h>
#include <atldef.h>
#include <comdef.h>
#include <shellapi.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atldef.h>
#include <dispex.h>
#include <handle_array.h>
#include <vector>
#include <algorithm>
#include <os_utils.h>

#include "dummy_dispatch_aggregator.h"

template < class Intf>
struct interface_deleter_t
{
  static void on_exit(Intf* p)
  {
    if(p) p->Release();
  }
};


inline HRESULT  get_dispid_ex(IDispatch* p,BSTR name,DISPID* pid,DWORD grfdex=fdexNameEnsure|fdexNameCaseInsensitive)
{
	HRESULT hr;
	hr=p->GetIDsOfNames(IID_NULL,&name,1, LOCALE_USER_DEFAULT,pid);
	if(FAILED(hr))
	{
		HRESULT hr2;
		struct _rel
		{
			~_rel(){ if(p) p->Release(); }
			IDispatchEx* p;
		} dispex={NULL};

		if(SUCCEEDED(hr2=p->QueryInterface(__uuidof(IDispatchEx),(void**)&dispex.p)))
		{
			hr=dispex.p->GetDispID(name,grfdex,pid);
		}
	}
	return hr;
};



template <class T, const IID* piid=&IID_IUnknown,bool FTM=false>
class CStaticUnknown : public T
{
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
	{
		if ((riid == IID_IUnknown) || (riid == *piid))
		{
			*ppvObject = (T*)static_cast<T*>(this);
			AddRef();
			return S_OK;
		}
		
		HRESULT hr=E_NOINTERFACE;
        //FreeThreadedMarshaler(this,riid,ppvObject,hr);
          aggregator_helper((IUnknown*)this,riid,ppvObject,hr);
		return hr;
	}
	STDMETHOD_(ULONG,AddRef)()
	{ return 2; }
	STDMETHOD_(ULONG,Release)()
	{
		return 2;
	}
 HRESULT set_aggregator(IUnknown* punk=0)
{	
   return aggregator_helper.reset(punk);
}
 HRESULT set_aggregator_context(void* pctx=0)
 {	
	 return aggregator_helper.set_context(pctx);
 }


protected:
	//FreeThreadedMarshaler_t<FTM,CStaticUnknown> FreeThreadedMarshaler;
	aggregator_helper_helper<> aggregator_helper;
};

template <class T, const IID* piid,bool FTM=false>
class CDynamicUnknown : public CStaticUnknown<T,piid,FTM>
{
protected:
	CDynamicUnknown():m_refcount(0){}; 
public:
	LONG volatile m_refcount;
	STDMETHOD_(ULONG,AddRef)()
	{
		return InterlockedIncrement(&m_refcount);
	}
	STDMETHOD_(ULONG,Release)()
	{
           
		ULONG l = InterlockedDecrement(&m_refcount);
		if (l == 0)
		{
			/*if(aggregator_helper)
			{
				AddRef();
				set_aggregator();
                Release(); 

			}
			else
			*/
			delete this;
		}
		return l;
	}

static CDynamicUnknown CreateInstance()
{
	return new CDynamicUnknown();
}
virtual ~CDynamicUnknown(){};
};


#define DISPATCH_GETSF (DISPATCH_PROPERTYGET|DISPATCH_METHOD)
#define DISPATCH_PUTSF (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)


#define is_get(f) (f&DISPATCH_GETSF)
#define is_put(f) (f&DISPATCH_PUTSF)



template <class T,class Unk=CStaticUnknown<IDispatch,&__uuidof(IDispatch)> >
class CDispatchT: public Unk //CStaticUnknown<IDispatch,&__uuidof(IDispatch)>
{
public:

	CDispatchT():grfdflt(0){};

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


	inline HRESULT GetDispID(
		BSTR bstrName,
		DWORD grfdex,
		DISPID *pid
		){ return E_NOTIMPL; };


	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
		LCID lcid, DISPID* rgdispid)
	{
		if (rgdispid == NULL) return E_POINTER; 
		
			return ((T*)this)->GetDispID(*rgszNames,grfdflt,rgdispid);
		
        
	}


	void do_invoke(int wFlags,int dispid,int argc,void* ,VARIANT& res,HRESULT &hr)
	{
		
	};

	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
		DISPPARAMS &dp=*pdispparams;

		  int argc=dp.cArgs;
		_variant_t *pargv=(_variant_t *)dp.rgvarg;
		_variant_t dummy;
		_variant_t *pres=(pvarResult)?((_variant_t*)pvarResult):&dummy;
         HRESULT hr=E_NOTIMPL;
		 try
		 {
			 if(DISPATCH_GETSF&wFlags)
			 {
                ((T*)this)->do_invoke(wFlags,dispidMember,argc,pargv,*pres,hr);
				return hr;
			 }
			 if(DISPATCH_PUTSF&wFlags)
			 {
                if((dp.cNamedArgs==1)&&(argc>=1))
				{
					if((dp.rgdispidNamedArgs)&&(*dp.rgdispidNamedArgs==DISPID_PROPERTYPUT))
					{
                        pres=(_variant_t *)dp.rgvarg;
						pargv=(_variant_t *)(dp.rgvarg+1);
						((T*)this)->do_invoke(wFlags,dispidMember,argc-1,pargv,*pres,hr);
					}

				}
				return hr;
			 }
               //  void T::do_invoke(WORD wFlags,DISPID dispidMember,int argc,_variant_t* argv,_variant_t& value,HRESULT& hr)
		 }
		   catch(...)    ///catch(_com_error& ce)
		 {

			 hr=DISP_E_EXCEPTION;
			 // if(S_OK==::GetErrorInfo())
		 }

		//return do_invoke()
	}

	inline IDispatch* get_Dispatch(bool faddref=true)
	{
		if(faddref) AddRef();
	    return (IDispatch*)(static_cast<T*>(this));
	}

    inline HRESULT get_VARIANT(VARIANT* pv)
	{
		if(!pv) return E_POINTER;
		 VARIANT r={VT_DISPATCH};
		 *pv=VARIANT(*this);
		 AddRef();
		 return S_OK;

	}


    inline   operator IDispatch*()
	{
         return (IDispatch*)(static_cast<T*>(this));
	}
      inline   operator VARIANT()
	  {
		  VARIANT r={VT_DISPATCH};
		  r.pdispVal=(IDispatch*)(static_cast<T*>(this));
          return r;
	  }
	
DWORD grfdflt;
};

template <class T>
struct CDispatchClassHolderT: public CDispatchT<T,CDynamicUnknown<IDispatch,&__uuidof(IDispatch)> >
{	    
	inline    VARIANT get_as_VARIANT()
	{
		VARIANT r={VT_DISPATCH};
		r.pdispVal=(IDispatch*)(static_cast<T*>(this));
		return r;
	};
        
};

template <class T,class Unk=CDynamicUnknown<IDispatch,&__uuidof(IDispatch)> >
class CDispatchHolderT: public CDispatchT<T,Unk>
{
 public:
	 STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
		 LCID lcid, DISPID* rgdispid)
	 {
		 return m_pdispatch->GetIDsOfNames(riid,rgszNames,cNames,lcid,rgdispid);
	 }

	 STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		 LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		 EXCEPINFO* pexcepinfo, UINT* puArgErr)
	 {
		 return m_pdispatch->Invoke(dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr);
	 }
     IDispatch* m_pdispatch;
   CDispatchHolderT(IDispatch* pdispatch,bool faddref=true):m_pdispatch(pdispatch)
   {
	   if(faddref) m_pdispatch->AddRef();
   }
virtual   ~CDispatchHolderT()
   {
      m_pdispatch->Release();
   }
};



template <class T,class Unk=CStaticUnknown<IDispatch,&__uuidof(IDispatch)>,int nargmax=32>
class CDispatchFunction: public CDispatchT<T,Unk> 
{
 public:


inline HRESULT call(int argc,VARIANT* pargv,VARIANT* presult,EXCEPINFO* pexcepinfo)
{
	return E_NOTIMPL;
};

inline HRESULT call_prop(WORD wFlags,int argc,VARIANT* pargv,VARIANT* presult,EXCEPINFO* pexcepinfo)
{
	T* t=static_cast<T*>(this);
	if(is_get(wFlags)) return t->call(argc,pargv,presult,pexcepinfo);
	return E_NOTIMPL;
};


inline bool findname(LPOLESTR name,DISPID& dispid)
{
  return false;
}

STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
						 LCID lcid, DISPID* rgdispid)
{
          T* t=static_cast<T*>(this);
		  if( t->findname(*rgszNames,*rgdispid))
		  {              
			  return S_OK;
		  }
         return E_NOTIMPL;
}

inline static VARIANT _vnp(){ VARIANT v={VT_ERROR};v.scode=DISP_E_PARAMNOTFOUND;return v; };

	 STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		 LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		 EXCEPINFO* pexcepinfo, UINT* puArgErr)
	 {
		 HRESULT hr=E_NOTIMPL;
            if((dispidMember==0))
			{
				int c=pdispparams->cArgs;
				VARIANT * pargs=0,*pdv=pdispparams->rgvarg;

				std::vector<VARIANT> vargs(nargmax,_vnp());
				if(is_put(wFlags))
				{

				bool f=(pdispparams->cNamedArgs==1)&&(pdispparams->rgdispidNamedArgs[0]==DISPID_PROPERTYPUT);
				//&&(*(pdispparams->rgdispidNamedArgs)==D)
                   if(!f)
					   return E_NOTIMPL;
				    --c;
					pvarResult=pdv++;
										                      
				}

				if(c>0)
				{
					vargs.resize(max(c,nargmax));
				   std::reverse_copy(pdv,pdv+c,vargs.begin());
				   //pargs=&vargs[0];
				}
                 if(max(c,nargmax)) pargs=&vargs[0];

				T* t=static_cast<T*>(this);
				return hr=t->call_prop(wFlags,c,pargs,pvarResult,pexcepinfo);
				
			}
	 }
};

inline void _fill_ExceptInfo(EXCEPINFO* pexcepinfo,wchar_t* pmsg,wchar_t* psource=0,void* proc=0) 
{
	if(pexcepinfo)
	{  
		*pexcepinfo=EXCEPINFO();
		bstr_t bdesc= (wchar_t*)pmsg;
		bool f=psource;
		bstr_t bsource=(f)?(wchar_t*)psource:process_info(proc);
		pexcepinfo->scode=DISP_E_EXCEPTION;
		pexcepinfo->bstrDescription=bdesc.Detach();
		pexcepinfo->bstrSource=bsource.Detach();

	}
}

template <class Unk=CStaticUnknown<IDispatch,&__uuidof(IDispatch)> >
struct dispatch_caller:CDispatchFunction<dispatch_caller<Unk> , Unk>
{

	typedef void (__stdcall * raise_error_proc)(wchar_t* smessage,wchar_t* ssource);
	typedef void (__cdecl * invoke_proc_type)
		(
		VARIANT* pResult,
		void* pcntx,
		raise_error_proc error,
		int argc,
		VARIANT* pvarg,
		...
		);

	struct sraiser{};

	invoke_proc_type  m_proc;
	void * m_pcontext;
	WORD m_flags_mask;

	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject){
		if(riid==GUID_NULL)
		{
			if(!ppvObject) return E_POINTER;
			*ppvObject=m_pcontext;
            return S_OK;
		}
		else __super::QueryInterface(riid,ppvObject);
	}


	dispatch_caller(invoke_proc_type  proc=0,void * pcntxt=0,unsigned fmask=DISPATCH_GETSF):m_proc(proc),m_pcontext(pcntxt),m_flags_mask(fmask)
	{
	}

	template <typename N,typename P >
	dispatch_caller(N proc,P pcntxt,unsigned fmask=DISPATCH_GETSF)
	{
		union
		{
			struct{N n;};
			struct{invoke_proc_type  proc;};
		} su;

		su.n=proc;
		m_proc=su.proc;
		m_pcontext=(void*)pcntxt;
		m_flags_mask=fmask;
	}

	inline bool static fill_ExceptInfo(EXCEPINFO* pexi,IErrorInfo* ei=0)
	{
		if(!pexi) return 0;
		IErrorInfo* eih=0;
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
		if(eih) eih->Release();
		return 1;
	};


	static	void __stdcall __raise_exception(wchar_t* smessage,wchar_t* ssource=0)
	{
		RaiseException(0xBABAEB,0,2,(ULONG_PTR*)&smessage);
	}

	static	DWORD __exception_filter(dispatch_caller* pthis,EXCEPTION_POINTERS* pep,
		EXCEPINFO* pexcepinfo)
	{
		PEXCEPTION_RECORD per=pep->ExceptionRecord;
		if(pexcepinfo)
		{
			if(per->ExceptionCode==0xBABAEB)
			{
			_fill_ExceptInfo(pexcepinfo,(wchar_t*)per->ExceptionInformation[0],(wchar_t*)per->ExceptionInformation[1],pthis->m_proc);
			}
			else fill_ExceptInfo(pexcepinfo);
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	inline static void safecall(void *proc,void *presult=0,void* pctx=0,void *perrp=0,DWORD c=0,void *pvar=0,DWORD cn=-1,void* pf=0,void* pex=0,void* flags=0)
	{
		if(!proc) return ;

#if defined(_WIN64)

		typedef void (*x64__safe_wild_call_t)(void* param0,void* param1,void* param2,void* param3,void* param4,void* param5,void* param6,void* param7,void* param8);

		((x64__safe_wild_call_t)proc)(presult,pctx,perrp,(void*)c,pvar,(void*)cn,pf,pex,flags);
			//(param0,param1,param2,param3,param4,param5,param6,param7);

#else

		DWORD ustack;
		__asm {
			mov ustack,ESP;
			push flags;
			push pex;
			push pf;
			push cn;
			push pvar;
			push c;
			push perrp;
			push pctx;
			mov  ECX,presult;
			push ECX;
			mov  EAX,proc;
			call EAX;
			mov ESP,ustack;
		}; 
#endif

	}

	inline HRESULT call_prop(WORD wFlags,int argc,VARIANT* pargv,VARIANT* presult,EXCEPINFO* pexcepinfo)
	{
		HRESULT hr=S_OK;
		VARIANT r={VT_EMPTY};
        if(!presult) presult=&r;

		__try
		{
           
			bool f=(wFlags&m_flags_mask);
			if(f&&(m_proc)) 
				safecall(m_proc,presult,m_pcontext,&__raise_exception,argc,pargv,argc-1,&hr,pexcepinfo,(void*)wFlags);
			else	 __raise_exception(L"server callback method not implemented.");
		}
		__except(__exception_filter(this,GetExceptionInformation(),pexcepinfo))
		{	
			hr=DISP_E_EXCEPTION;
		};
		::VariantClear(&r);
		return hr;
	};


	inline bool findname(LPOLESTR name,DISPID& dispid)
	{

		if(StrCmpNIW(name,L"val",3)==0) return dispid=0,true; 
		if(StrCmpNIW(name,L"call",4)==0) return dispid=0,true;
		if(StrCmpIW(name,L"_")==0) return dispid=0,true;
		return false;
	}


};



template < typename IniFini,bool _FTM>
struct dyn_disp_caller:dispatch_caller<CDynamicUnknown<IDispatch,&__uuidof(IDispatch),_FTM> >
{
	IniFini module_locker;
	void * m_exproc;
	void * m_pctx2;
	template <typename N,typename P,typename N2,typename P2 >
	dyn_disp_caller(N proc,P pcntxt,N2 exproc,P2 pcntxt2,unsigned fmask)
		:dispatch_caller<CDynamicUnknown<IDispatch,&__uuidof(IDispatch),_FTM> >(proc,pcntxt,fmask)
	{
		union
		{
			struct{N2 n;};
			struct{void * proc;};
			struct{ dispatch_caller<CDynamicUnknown<IDispatch,&__uuidof(IDispatch),_FTM> >::invoke_proc_type iproc;};
		} su;

		su.n=exproc;
		m_exproc=su.proc;
		m_pctx2=(void*)pcntxt2;

		//module_locker=m_exproc;
		module_locker=m_proc;

	}
	virtual		~dyn_disp_caller()
	{
		try
		{

			OutputDebugStringW(L"call ~dyn_disp_caller\n");
			safecall(m_exproc,m_pctx2);
		}
		catch(...)
		{};
	}
};



template < typename IniFini, typename N,typename P,typename N2,typename P2 >
inline VARIANT create_dispatch_wrapperF_aggr(N proc,P pcntxt,N2 exproc=(void*)0,P2 pcntx2=(void*)0,IDispatch** ppdisp=NULL,unsigned fmask=DISPATCH_GETSF,IUnknown* pouter=0)
{
	//typedef dispatch_caller<CDynamicUnknown<IDispatch,&__uuidof(IDispatch)> > dyn_disp_caller;

	VARIANT r={VT_DISPATCH};

		dyn_disp_caller<IniFini,false>* pdc=new dyn_disp_caller<IniFini,false>(proc,pcntxt,exproc,pcntx2,fmask);	
		pdc->set_aggregator(pouter);
		r.pdispVal=pdc->get_Dispatch(true);

	if(ppdisp) *ppdisp=r.pdispVal;
	//pdc->get_Dispatch(true);
	return r;
}



template < typename IniFini, typename N,typename P,typename N2,typename P2 >
inline VARIANT create_dispatch_wrapperF(N proc,P pcntxt,N2 exproc=(void*)0,P2 pcntx2=(void*)0,IDispatch** ppdisp=NULL,unsigned fmask=DISPATCH_GETSF,bool FTM=false)
{
	//typedef dispatch_caller<CDynamicUnknown<IDispatch,&__uuidof(IDispatch)> > dyn_disp_caller;
 
	VARIANT r={VT_DISPATCH};

  if(FTM)
  {
    dyn_disp_caller<IniFini,true>* pdc=new dyn_disp_caller<IniFini,true>(proc,pcntxt,exproc,pcntx2,fmask);	
	r.pdispVal=pdc->get_Dispatch(true);
  }
  else {
	  dyn_disp_caller<IniFini,false>* pdc=new dyn_disp_caller<IniFini,false>(proc,pcntxt,exproc,pcntx2,fmask);	
	  r.pdispVal=pdc->get_Dispatch(true);
  }
	if(ppdisp) *ppdisp=r.pdispVal;
		//pdc->get_Dispatch(true);
	return r;
}

template <typename N,typename P,typename N2,typename P2 >
inline VARIANT create_dispatch_wrapper(N proc,P pcntxt,N2 exproc=(void*)0,P2 pcntx2=(void*)0,IDispatch** ppdisp=NULL,unsigned fmask=DISPATCH_GETSF,bool FTM=false)
{
	return create_dispatch_wrapperF<void*>(proc,pcntxt,exproc,pcntx2,ppdisp,fmask,FTM);
}


template <typename N,typename P,typename N2,typename P2 >
inline VARIANT create_dispatch_wrapper_aggr(IUnknown* pouter,N proc,P pcntxt=(void*)0,N2 exproc=(void*)0,P2 pcntx2=(void*)0,IDispatch** ppdisp=NULL,unsigned fmask=DISPATCH_GETSF)
{
	return create_dispatch_wrapperF_aggr<void*>(proc,pcntxt,exproc,pcntx2,ppdisp,fmask,pouter);
}


//template <typename Intf>

struct VARIANT_S:VARIANT
{
	VARIANT_S():VARIANT(){};
	~VARIANT_S(){VariantClear(this);};
};

template<class Intf>
inline VARIANT create_weak_ref(Intf* p,VARIANT& v=VARIANT_S())
{   
	struct weak_ref
	{
		static void __cdecl  get( VARIANT* presult,Intf* p
			,void*,int ,void*,int ,HRESULT& hr,void*,unsigned)
		{
			
            if((presult)&&(p))
			{
				hr=p->QueryInterface(__uuidof(IUnknown),&(presult->byref=0));  
					if(SUCCEEDED(hr))
						V_VT(presult)=VT_UNKNOWN;
					VariantChangeType(presult,presult,0,VT_DISPATCH);
			}
			else  hr=E_POINTER;
			
		}
	};

	//if(!punk) return VARIANT();
	//proc
	return v=create_dispatch_wrapperF<int>(&weak_ref::get,p,FARPROC(0),int(0));
}





template <class T> 
struct CStaticDispatch :public  CDispatchT<T,CStaticUnknown<IDispatch,&__uuidof(IDispatch)> > {};

template <class T > 
struct CDynamicDispatch :public  CDispatchT<T,CDynamicUnknown<IDispatch,&__uuidof(IDispatch)> > {};

#define disp_ind(i,n)  (n-1-i)


