#pragma once 
//sa_utils.h
#include "ipc_utils.h"

namespace sa_utils
{

	inline LONG64	SAFEARRAY_total_length(int cDims, SAFEARRAYBOUND* rgsabound)
	{
		if (cDims <= 0) return 0;
		LONG64 e = 1;
		for (int n = 0; n < cDims; n++)
			e *= rgsabound[n].cElements;
		//e*=m_array.cbElements;
		return e;
	};
	/*
	inline LONG64	SAFEARRAY_total_length(SAFEARRAY* psa)
	{
		if(psa->cDims<=0) return 0;
		LONG64 e=1;
		for(int n=0;n<psa->cDims;n++)
			e*=psa->rgsabound[n].cElements;
		//e*=m_array.cbElements;
		return e;
	}*/

	inline LONG64	SAFEARRAY_total_length(SAFEARRAY* psa)
	{
		return SAFEARRAY_total_length(psa->cDims, psa->rgsabound);
	}


	inline LONG64	SAFEARRAY_total_bytelength(SAFEARRAY* psa)
	{

		return (psa->cbElements)*LONG64(SAFEARRAY_total_length(psa));
	}

	inline VARIANT* PVARIANT_Ref2Ind(VARIANT* pv)
	{
		if (pv&&(pv->vt) == (VT_BYREF | VT_VARIANT)) {
			pv = pv->pvarVal;			
		}
		return pv;
	}


	struct safe_array_ptr_t
	{
		HRESULT hrl;
		char* p;
		SAFEARRAY* psa;
		LONG64 length;
		LONG64 sizeb;
		long  element_size;

		safe_array_ptr_t() :hrl(E_POINTER),p(NULL),psa(NULL) {
		};

		safe_array_ptr_t(SAFEARRAY* _psa ) :p(0), psa(0) {
			hrl = init(_psa);
		}
		HRESULT init(SAFEARRAY* _psa)
		{
			HRESULT &hr = hrl;
			if (psa) SafeArrayUnlock(ipc_utils::make_detach(psa));

			if (!_psa) {

				OLE_CHECK_hr(E_POINTER);
			}

			psa = _psa;

			long ll[64];
			memset(ll, 0, sizeof(ll));
			OLE_CHECK_hr(hrl = SafeArrayLock(psa));
			sizeb = (length = SAFEARRAY_total_length(psa))*(element_size = psa->cbElements);
			//OLE_CHECK_hr(SafeArrayPtrOfIndex(psa,ll,(void**)&p));
			OLE_CHECK_PTR(p = (char*)(psa->pvData));
			return hr;
		}

		HRESULT init(VARIANT* pv,VARTYPE vtc=0) {
			HRESULT &hr = hrl;
			SAFEARRAY** ppsa;

			OLE_CHECK_PTR_hr(pv=PVARIANT_Ref2Ind(pv));

			VARTYPE vt = pv->vt;
			if (0 == (vt&VT_ARRAY))
				return hr = E_INVALIDARG;

			ppsa = ((vt&VT_BYREF)==0) ? &pv->parray : pv->pparray;

			if ((vtc) && (vtc != (VT_TYPEMASK&vt))) {
				return hr = E_INVALIDARG;
			}

			OLE_CHECK_PTR_hr(ppsa);		

			return init(*ppsa);
		}



		~safe_array_ptr_t() { if (SUCCEEDED(hrl)) SafeArrayUnlock(psa); }


		HRESULT clone_reshaped(VARTYPE vt, int nDims, SAFEARRAYBOUND* indxs, SAFEARRAY** psa)
		{
			HRESULT hr;
			OLE_CHECK_hr(hrl);
			return hr;


		}



		template<class N>
		inline operator N*() { return (N*)p; }
		inline operator char*() { return p; }
		inline operator HRESULT() { return hrl; };



	};






}//sa_utils