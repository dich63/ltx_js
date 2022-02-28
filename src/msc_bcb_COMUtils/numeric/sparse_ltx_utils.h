#pragma once

#include "sparse_utils.h"
namespace mkl_holder_utils {

   
    
	template <class N,bool FCOPY=false>
	struct  ltx_read_buffer_t:type_buffer_t<N,FCOPY>
	{
		typedef i_mm_region_ptr::int64_t  int64_t;
		HRESULT hr;
		VARTYPE vt;
		typedef std::map<const char*,void*> ptrmap_t;

		ipc_utils::smart_ptr_t<i_mm_region_ptr> region_ptr;
		i_mm_region_ptr::locker_t lock;

		ltx_read_buffer_t(VARIANT v,int64_t dflt_count=0){
			hr=init(v);
			if(dflt_count&&FAILED(hr))
			{
			   count=dflt_count;
               p=ph=(N*)realloc(ph,count*sizeof(N));
			   memset(p,0,count*sizeof(N));
			   
			}
		}
/*
		ltx_read_buffer_t(IUnknown* punk){			
			hr=init(punk);
		}
		*/
		HRESULT init(IUnknown* punk) 
		{
			OLE_CHECK_PTR(punk);
			VARIANT v={VT_UNKNOWN}
			v.punkVal=punk;
			return init(v);
		}
		HRESULT init(VARIANT& v) {

			int64_t c;
			void* ptr;
			OLE_CHECK_hr(region_ptr.reset(v));
			OLE_CHECK_hr(region_ptr->GetElementInfo(&vt,&c));
			OLE_CHECK_hr(region_ptr->GetPtr(&(ptr=0)));
             lock.attach(region_ptr);
			 if(!link_vt(vt,c,ptr))
			 {
               lock.detach();
			   region_ptr.Release();

			   hr=DISP_E_BADVARTYPE;		   
			 }

		    return hr;
		};

		operator HRESULT() { return hr;}	

		operator N*() {	
			return p;
		}


	};



};

// namespace mkl_holder_utils