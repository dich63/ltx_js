#pragma once
#include <windows.h>
#include <DispEx.h>
#include <vector>

// #include "dispfast.h"


template <class Buffer>
struct IDispatch_fast_t:IDispatchEx
{

	 typedef Buffer buffer_t;
     typedef LONGLONG int64_t; 

	 LONG refcount;
	 
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject){

		if ((riid == IID_IUnknown)||(riid == IID_IDispatch)||(riid == IID_IDispatchEx))
		{
			*ppvObject = (IDispatch*)static_cast<IDispatch*>(this);
			this->AddRef();
			return S_OK;
		}


		if (riid == IID_IMarshal)
		    return E_ACCESSDENIED; 
		    
		return E_NOINTERFACE;
	}

	
	STDMETHOD_(ULONG,AddRef)()	{ return ++refcount;}
	STDMETHOD_(ULONG,Release)()	{	
         
		ULONG l=--refcount;
		if(l==0)		
          delete this;		
		return l;
	}


	inline void dispose()
	{
		if(pholder)
		{
		
		 buffer->unlock();
		 pholder->Release();
		 pholder=0;

		}

	}


	IDispatch_fast_t(buffer_t* _buffer,IUnknown* _pholder)
		:buffer(_buffer),pholder(_pholder),refcount(1)
	{
		pholder->AddRef();
		buffer->lock();
		vbuf.reserve(64);
	

	}

	~IDispatch_fast_t()
	{ 
		dispose(); 
	};



	

	inline	HRESULT impl_GetIDsOfNames( LPOLESTR name, DISPID *pdispid){

		if(_wcsnicmp(name,L"buf",3)==0)
		{
			*pdispid=-27;
			return S_OK;
		}


		if(_wcsnicmp(name,L"gets",3)==0)
		{
			*pdispid=-26;
			return S_OK;
		}
        if(_wcsnicmp(name,L"op",2)==0)
		{
			*pdispid=-25;
			return S_OK;
		}
		if(_wcsnicmp(name,L"dispose",7)==0)
		{
			*pdispid=-22;
			return S_OK;
		}
		if(_wcsnicmp(name,L"length",6)==0)
		{
			*pdispid=-23;
			return S_OK;
		}

		return E_NOTIMPL;


	}

	HRESULT gets(DISPPARAMS dp,VARIANT* pres,EXCEPINFO* ei,IServiceProvider *pspCaller=0)
	{
		HRESULT hr;
		int coff=dp.cNamedArgs,c=dp.cArgs;
		bool fthis=coff&&(dp.rgdispidNamedArgs)&&(*dp.rgdispidNamedArgs==DISPID_THIS);

		if(c<=coff)
			 return E_INVALIDARG;

		struct _closer_t
		{
			IDispatchEx* dx;
			~_closer_t(){ 
				dx&& dx->Release();
			}

		}		holder={0};


		
		if(fthis)
		{

          VARIANT& vcb=dp.rgvarg[c-1];

		  if(!((vcb.vt==VT_DISPATCH)&&vcb.byref))
			  return E_INVALIDARG;

		  //if(FAILED(hr=vcb.pdispVal->QueryInterface(__uuidof(IDispatchEx),(void**)&holder.dx)))
			//  return E_INVALIDARG;
		    IDispatchEx *dex=(IDispatchEx*)vcb.pdispVal;

		    vbuf.resize(c-1);
			VARIANT *pv=&vbuf[0];
			pv[0]=dp.rgvarg[0];
			

			VARIANT *pi=dp.rgvarg,vi={VT_EMPTY};

			dp.cArgs=c-1;
			dp.rgvarg=pv;

			for(int k=0;k<c-2;k++)
			{
				if(FAILED(hr=VariantChangeType(&vi,++pi,0,VT_I8)))
					return E_INVALIDARG;
				if(FAILED(hr=buffer->get_element(vi.llVal,++pv)))
					return hr;

			}
			

			//hr=holder.dx->InvokeEx(DISPID_VALUE,LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD,&dp,pres,ei,0);
			hr=dex->InvokeEx(DISPID_VALUE,LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD,&dp,pres,ei,pspCaller);





		}
		else hr=E_NOTIMPL;




		return hr;


	}

	
	inline	HRESULT impl_Invoke(DISPID did,WORD flags,DISPPARAMS* dp,VARIANT*res,EXCEPINFO* ei=0,IServiceProvider *pspCaller=0){


		HRESULT hr;

		bool fgp=flags&(DISPATCH_PROPERTYGET|DISPATCH_METHOD);
		int off=dp->cNamedArgs;
		int c=dp->cArgs-off;

		if(!(fgp) ) return E_INVALIDARG;	
					

		if(did==0)
		{
			//if(!(fgp&&c) ) return E_INVALIDARG;	


			if(!pholder) return E_ACCESSDENIED;

			if(dp->rgvarg[dp->cArgs-1].vt==VT_DISPATCH)
			{
                   return hr=gets(*dp,res,ei,pspCaller);
			}

			VARIANT vi={};
			int c1=c-1;
			VARIANT* pvl=dp->rgvarg+c1+off;


			if(FAILED(hr=VariantChangeType(&vi,pvl,0,VT_I8)))
				return hr;
			
				//return 0;
				if(c>1)
				{
                 for(int k=0;k<c1;k++)
				 {
					 if(FAILED(hr=buffer->set_element(vi.llVal+k,*(--pvl))))
						 break;     
				 }
				 
				}
				else 
				{

					hr=(res)?buffer->get_element(vi.llVal,res):S_OK;
				}

			  return hr;

		}

// HRESULT gets(DISPPARAMS dp,VARIANT* pres,EXCEPINFO* ei,IServiceProvider *pspCaller=0)
		if((did==-27)&&(fgp))
		{
			if(res&&pholder)
			{
				res->vt=VT_UNKNOWN;
				(res->punkVal=pholder)->AddRef();
			}
			return S_OK;
			

		}

		if((did==-26)&&(fgp))
		{
			return hr=gets(*dp,res,ei,pspCaller);

		}
		if((did==-25)&&(fgp))
		{
			int64_t offset,size;
			IDispatch* disp;
			if(c!=4)  return E_INVALIDARG;	
			if(dp->rgvarg[3].vt!=VT_DISPATCH) 
				 return E_INVALIDARG;	
			else disp=dp->rgvarg[3].pdispVal;
				 
			VARIANT& vcontext=dp->rgvarg[2];
			VARIANT v={};
			if(FAILED(hr=VariantChangeType(&v,&(dp->rgvarg[1]),0,VT_R8))) 
				return hr;
			else offset=v.dblVal;
			v.vt=0;
			if(FAILED(hr=VariantChangeType(&v,&(dp->rgvarg[0]),0,VT_R8))) 
				return hr;
			else size=v.dblVal;		

			vbuf.resize(size+1);
			VARIANT* pv=(&vbuf[0]),*pc=pv+size;

			DISPPARAMS dpcb={pv,0,size+1,0};
			*(pc)=vcontext;

			for(ULONG k=0;k<size;k++)
			{
				
				if(FAILED(hr=buffer->get_element(offset+k,--pc)))
					 return hr;

			}

			hr=disp->Invoke(0,IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD,&dpcb,res,ei,0);			
			
			return hr;
		}

		if((did==-22)&&(fgp))
		{
			dispose();
			return S_OK;
		}
		if((did==-23)&&(fgp))
		{
			if(!pholder) 
				 return E_ACCESSDENIED;

			if(res)
			{
				res->vt=VT_R8;
				res->dblVal=buffer->get_count();
			}
			return S_OK;
		}



       
		return E_NOTIMPL;
	}


			

	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)	{
			return E_NOTIMPL;
	}

	STDMETHOD(GetTypeInfo)( UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo){
			return E_NOTIMPL;
	}

	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,LCID lcid, DISPID* rgdispid)	{
		return impl_GetIDsOfNames(*rgszNames,rgdispid);
	}
	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
		return impl_Invoke(dispidMember,wFlags,pdispparams,pvarResult,pexcepinfo);
	}



	virtual HRESULT STDMETHODCALLTYPE GetDispID( 
		/* [in] */ __RPC__in BSTR bstrName,
		/* [in] */ DWORD grfdex,
		/* [out] */ __RPC__out DISPID *pid) 	{
			
			
			return impl_GetIDsOfNames(bstrName,pid);	

	}



	virtual /* [local] */ HRESULT STDMETHODCALLTYPE InvokeEx( 
		/* [in] */ DISPID id,
		/* [in] */ LCID lcid,
		/* [in] */ WORD wFlags,
		/* [in] */ DISPPARAMS *pdp,
		/* [out] */ VARIANT *pvarRes,
		/* [out] */ EXCEPINFO *pei,
		/* [unique][in] */ IServiceProvider *pspCaller) 	{
            return impl_Invoke(id,wFlags,pdp,pvarRes,pei,pspCaller);
	}


	virtual HRESULT STDMETHODCALLTYPE DeleteMemberByName( 
		/* [in] */ __RPC__in BSTR bstrName,
		/* [in] */ DWORD grfdex) 	{
			return E_NOTIMPL;

	}


	virtual HRESULT STDMETHODCALLTYPE DeleteMemberByDispID( 
		/* [in] */ DISPID id)	{
			return E_NOTIMPL;

	}


	virtual HRESULT STDMETHODCALLTYPE GetMemberProperties( 
		/* [in] */ DISPID id,
		/* [in] */ DWORD grfdexFetch,
		/* [out] */ __RPC__out DWORD *pgrfdex)	{
			return E_NOTIMPL;
	}


	virtual HRESULT STDMETHODCALLTYPE GetMemberName( 
		/* [in] */ DISPID id,
		/* [out] */ __RPC__deref_out_opt BSTR *pbstrName)	{
			return E_NOTIMPL;

	}


	virtual HRESULT STDMETHODCALLTYPE GetNextDispID( 
		/* [in] */ DWORD grfdex,
		/* [in] */ DISPID id,
		/* [out] */ __RPC__out DISPID *pid) 	{
			return E_NOTIMPL;

	}


	virtual HRESULT STDMETHODCALLTYPE GetNameSpaceParent(/* [out] */ __RPC__deref_out_opt IUnknown **ppunk)
	{
		HRESULT  hr;
		return E_NOTIMPL;
	}




	buffer_t* buffer;
	IUnknown* pholder;
	std::vector<VARIANT> vbuf;
	
/*	VARTYPE vt;
	int64_t count;
	long elsize;*/

};


