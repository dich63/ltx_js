#pragma once

#include "dummy_dispatch_aggregator.h"

template <class T ,class RefCounter=RefCounterMT_t<1>,class Mutex=mutex_fake_t,class IDispatchX=IDispatch>
struct args_base_t:IDispatch_base_t<T,IDispatchX,RefCounter>
{
	typedef void (* raise_error_proc_t)(wchar_t* smessage,wchar_t* ssource);
	typedef typename Mutex mutex_t;
	enum {
		offset_id=17,
		id_len=-2,
		id_def=-3,
		id_dflt=-4,
		id_error=-5
	};

	mutex_t mutex;
	bool freverse;   
	int argc;
	VARIANT *pargv;
	raise_error_proc_t raise_error;
	args_base_t():raise_error(0){};
	args_base_t(int c,VARIANT *pv,bool f_reverse=true,raise_error_proc_t re=0):argc(c),pargv(pv),freverse(f_reverse),raise_error(re){};

	args_base_t(DISPPARAMS* pdp,bool f_reverse=true):argc(pdp->cArgs),pargv(pdp->rgvarg),freverse(f_reverse),raise_error(0){};


	inline void reset(int c=0,VARIANT *pv=0)
	{
		locker_t<mutex_t> lock(mutex);
		argc=c;
		pargv=pv;

	}

	inline void clear()
	{
		locker_t<mutex_t> lock(mutex);
		argc=0;
	}



	inline int check(int i)
	{
		return (0<=i)&&(i<argc);
	}




	inline HRESULT findname(LPOLESTR name,DISPID& dispid){
		HRESULT hr;
		int i;
		static  BEGIN_STRUCT_NAMES(pn)										
			DISP_PAIR_IMPLC_LEN(length,id_len,3)												
			DISP_PAIR_IMPLC_LEN(def,id_dflt,3)
			DISP_PAIR_IMPLC(error,id_error)
			END_STRUCT_NAMES;
		hr=find_id(pn,name,&dispid);
		if((FAILED(hr))&&StrToIntExW(name,STIF_SUPPORT_HEX,&i))
		{
			dispid=check(i)?(i+offset_id):id_def;			
			return S_OK;
		}
		return hr;
	}

	inline VARIANT to_variant(VARTYPE vt=VT_EMPTY,void* p=0)
	{
		VARIANT r={vt};
		r.byref=p;
		return r;
	}


	template<class N>
	inline HRESULT to_variant(VARTYPE vt,N n,VARIANT* pv)
	{
		if(pv) *pv=to_variant(vt,(void*)n);
		return S_OK;
	}

	/*
	inline	HRESULT impl_Result(DISPID id,WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi)
	{
	return E_INVALIDARG;
	}*/


	inline	HRESULT impl_Invoke(DISPID id,REFIID riid,LCID,WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi, UINT*){



		if(!res) return E_POINTER;

		HRESULT hr=E_INVALIDARG;
		int i;

		//    if(id==id_result) return hr=static_cast<T*>(this)->impl_Result(id,flags,dp,res,pexi);


		locker_t<mutex_t> lock(mutex);

		if(!is_get(flags)) 
		{
			return hr;
		}
		if(check(i=id-offset_id))
		{
			if(freverse) i=(argc-1)-i;
			return hr=VariantCopyInd(res,pargv+i);			   
		}  
		else 
			if(id==id_len)		   		   	   	
				return hr=to_variant(VT_I4,argc,res);  			   
			else {
				if(id==id_def)
					return hr=to_variant(VT_EMPTY,int(0),res);
				else
					if((id==id_dflt)&&(dp->cArgs>0))
					{
						VARIANT vi={};
						if(FAILED(VariantChangeType(&vi,dp->rgvarg,0,VT_I4)))
							return hr;
						if(check(i=vi.intVal))
							return hr=VariantCopyInd(res,pargv+i);
						else{

							if(dp->cArgs>1)
								return hr=VariantCopyInd(res,dp->rgvarg);
							else return hr=to_variant(VT_EMPTY,int(0),res);

						}

					}
					else if(id==id_error)
					{
						if(!raise_error) return E_NOTIMPL;
						int c=dp->cArgs;
						if(c==0) raise_error(0,0);
						if(c==1)
						{
							variant_cast_t<VT_BSTR> v1(dp->rgvarg[0]);
							raise_error((v1)?v1->bstrVal:0,0);
								
						}
						
						if(c>1)
						{
							variant_cast_t<VT_BSTR> v1(dp->rgvarg[c-1]);
							variant_cast_t<VT_BSTR> v2(dp->rgvarg[c-2]);
							raise_error((v1)?v1->bstrVal:0,(v2)?v2->bstrVal:0);
							
						}




					}

			}



			return hr;              

	}

};
