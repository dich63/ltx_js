#pragma once

#include "ipc_marshal_def.h"

//typedef ISequentialStream i_kox_stream;

///
/*
#ifdef EXPORT_DLL 
#define KOX_API extern "C" __declspec(dllexport)
#else 
#define KOX_API extern "C" __declspec(dllimport)
#endif
//*/

//

#define KOX_API extern "C"


KOX_API HRESULT io_pipe_port_connectW(const wchar_t* url,i_kox_stream** ppstream);
KOX_API HRESULT io_pipe_port_connectA(const char* url,i_kox_stream** ppstream);
KOX_API HRESULT io_pipe_port_createW(const wchar_t* url,const wchar_t* options_zz,i_kox_obj** ppport);
KOX_API HRESULT io_pipe_port_createA(const char* url,const char* options_zz,i_kox_obj** ppport);
KOX_API HRESULT io_pipe_port_create_runW(const wchar_t* url,const wchar_t* options_zz,void* pcallback,void* pcontext,i_kox_obj** ppport);
KOX_API HRESULT io_pipe_port_create_runA(const char* url,const char* options_zz,void* pcallback,void* pcontext,i_kox_obj** ppport);
KOX_API HRESULT io_pipe_port_listen(i_kox_obj* pport,void* pcallback,void* pcontext);
KOX_API HRESULT io_pipe_port_shutdown(i_kox_obj* pport,bool force=false);
KOX_API HRESULT io_pipe_port_get_waitable_event(i_kox_obj* pport,HANDLE* pevent);

KOX_API HRESULT kox_marshal(i_kox_stream* pstream,int region_count, ipc_region_base_t** pp_regions,const GUID* pconn_uuid=NULL);
KOX_API HRESULT kox_marshal_to_process(int pid,i_kox_stream* pstream,int region_count, ipc_region_base_t** pp_regions,const GUID* pconn_uuid=NULL);
KOX_API HRESULT kox_marshal_array(i_kox_stream* pstream,int region_count, ipc_region_base_t* p_regions,const GUID* pconn_uuid=NULL);
KOX_API HRESULT kox_unmarshal(i_kox_stream* pstream,i_kox_obj** pp_koxdata);
KOX_API HRESULT kox_get_unmarshal_uuid(i_kox_obj* p_koxdata,GUID** ppconn_uuid);
KOX_API HRESULT kox_get_unmarshal_data(i_kox_obj* p_koxdata,int* pcbcount,ipc_region_base_t*** ppp_regions);
KOX_API HRESULT kox_get_unmarshal_com_object(ipc_region_base_t* p_regions,void** ppobj,const GUID* piid=0);
KOX_API  long kox_detect_type(void* p);

KOX_API long kox_get_region_att(ipc_region_base_t* p_regions);
KOX_API long kox_get_region_size(ipc_region_base_t* p_regions);
KOX_API char* kox_get_region_ptr(ipc_region_base_t* p_regions);

KOX_API HRESULT kox_map_unmarshal(i_kox_obj** pp_map,i_kox_stream* pstream=0);
KOX_API HRESULT kox_map_create(i_kox_obj** pp_map);
KOX_API HRESULT kox_map_marshal(i_kox_obj* pp_map,i_kox_stream* pstream,int pid=0);
KOX_API HRESULT kox_map_set(i_kox_obj* p_map,const char* name,ipc_region_base_t* pregion);
KOX_API HRESULT kox_map_set_data(i_kox_obj* p_map,const char* name,void* ptr,long cbyte=-1,long attr=0);
KOX_API HRESULT kox_map_get(i_kox_obj* p_map,const char* name,ipc_region_base_t** ppregion);
KOX_API HRESULT kox_map_info(i_kox_obj* p_map,char*** pppnames=0,int* pcb=0);

KOX_API HRESULT kox_get_fd(i_kox_stream* ps,HANDLE* ph);


KOX_API HRESULT kox_get_pipe_stream(IUnknown* pUnk,HANDLE h_r,HANDLE h_w,i_kox_stream** ppstream);
KOX_API HRESULT kox_get_stream_pair(IUnknown* pUnk,i_kox_stream** ppstream1,i_kox_stream** ppstream2);



#if defined(__cplusplus) 

#include <vector>
struct kox_delay_loader_t
{
	
	kox_delay_loader_t(const wchar_t* libpfn=L"kox.lib")
	{
		if(!GetModuleHandleW(L"kox.dll"))
		{
		std::vector<wchar_t> ve(GetEnvironmentVariableW(libpfn,0,0));      
        if(ve.size())
		{ 
          wchar_t* plib=&ve[0];
		 if(GetEnvironmentVariableW(libpfn,plib,ve.size()))
		 {			
			HMODULE hl=LoadLibraryW(plib);
			if(hl) kox_get_region_att(0);
			FreeLibrary(hl);			
		 }
		}
		}
	}

};

struct kox_helper_base_t
{
	struct region_t:ipc_region_base_t
	{

		inline operator HANDLE()
		{
		  if((attr&IPCM_ERROR)||((attr&IPCM_HANDLE)==0) )return INVALID_HANDLE_VALUE;
          return *phandle;
		}
		inline operator SOCKET()
		{
			if((attr&IPCM_ERROR)||((attr&IPCM_SOCKET)==0) )return INVALID_SOCKET;
			return *psocket;
		}

		template<class H>
		inline bool detach_to(H& h)
		{
             if((attr&IPCM_ERROR)||((attr&IPCM_KO)==0) )return false;
			 H& hsrc=*((H*)ptr_v);
			 h=hsrc;
			 hsrc=H(0);

			 return true;
		}


		inline HRESULT unwrap_com_object(VARIANT* pv)
		{
			if(!pv) return E_POINTER;
			VARIANT r={VT_UNKNOWN};			
			HRESULT herr;
			//return herr=kox_get_unmarshal_com_object(this,&r.byref,& __uuidof(i_kox_obj));
			if(S_OK==(herr=unwrap_com_object(&r.punkVal))){
				VariantChangeType(&r,&r,0,VT_DISPATCH);
				*pv=r;
			}

			return herr;
	   	}

		template<class Inf>
		inline HRESULT unwrap_com_object(Inf** ppobj)
		{
			HRESULT herr;
            return herr=kox_get_unmarshal_com_object(this,(void**)ppobj,& __uuidof(Inf));
	   	}
		
		region_t(){};

		
		inline void fill(void* _ptr,long _cb,long _attr)
		{
			ptr_v=(void*)_ptr;
			sizeb=_cb;
			attr=_attr;
		}
		region_t(void* _ptr,long _cb=-1,long _attr=0)
		{
			fill(_ptr,_cb,_attr);
		}

		region_t(SOCKET* s)
		{
			fill((void*)s,sizeof(*s),IPCM_SOCKET);
		}
		region_t(HANDLE* h)
		{
			fill((void*)h,sizeof(*h),IPCM_HANDLE);
		}
		template <class I>
		region_t(I** p,const uuid_t& u=__uuidof(I))
		{
			fill((void*)p,sizeof(*p),IPCM_COM);
		}
	} ;


	struct ipc_region_null_t:region_t
	{
		ipc_region_null_t()
		{
		  memset(this,0,sizeof(ipc_region_null_t));
		  attr=IPCM_ERROR;
		  ptr=buf;
		}
		char buf[8];
	} region_null;


  inline const IID& uid()
  {
	    return (pconn_iid)? *pconn_iid:IID_NULL;
  }


	kox_helper_base_t():p_kox(0),pconn_iid(0),hr(E_POINTER),hrlast(0){};

inline void clear(){
		if( p_kox) p_kox->Release();
		p_kox=0;
	}

virtual	~kox_helper_base_t()
	{
		clear();
	}

	i_kox_obj* p_kox;
	GUID* pconn_iid;
	HRESULT hr,hrlast;

};

struct kox_unmarshal_helper_t: kox_helper_base_t
{
	
	//kox_delay_loader_t kxl;



	
	
	inline HRESULT reset(i_kox_obj* p_koxdata=0){
		//if(p_koxdata==p_kox) return hr;

		if(p_koxdata)  p_koxdata->AddRef();          
		if(p_kox) p_kox->Release(); 

		p_kox=p_koxdata;
		if(p_kox){

			hr=kox_get_unmarshal_uuid(p_kox,&pconn_iid);
			if(SUCCEEDED(hr))
				hr=kox_get_unmarshal_data(p_kox,&count,&pp_regions);
		}

             return hr;
	}

	inline HRESULT reset(i_kox_stream* pstream){

		    if(!pstream) 
				return reset();
			
            if(SUCCEEDED(hr=kox_unmarshal(pstream,&p_kox)))
				return reset(p_kox);

	}
	kox_unmarshal_helper_t(i_kox_obj* p_koxdata=0):pp_regions(0),count(0)
	{
		//if( p_koxdata) (p_kox= p_koxdata)->AddRef();
    	       
		 reset(p_koxdata);
		 

	}
	kox_unmarshal_helper_t(i_kox_stream* pstream):pp_regions(0),count(0)
	{
		/*
		if((pstream)&&SUCCEEDED(hr=kox_unmarshal(pstream,&p_kox))
			&&SUCCEEDED(hr=kox_get_unmarshal_uuid(p_kox,&pconn_iid)))
					hr=kox_get_unmarshal_data(p_kox,&count,&pp_regions);
					*/
		reset(pstream);

				
	}

	

	
	inline operator HRESULT()
	{
		return hr;
	}
	inline	bool operator!()
	{
	   return FAILED(hr);
	}

inline region_t& operator[](int n)
{ 
        if(SUCCEEDED(hr)&&(0<=n)&&(n<count))
			return *((region_t*)pp_regions[n]);
		else	return region_null;
}


    ipc_region_base_t** pp_regions;
	
	int count;
	

};

struct kox_map_helper_t: kox_helper_base_t
{
	inline void reset(i_kox_stream* pstream=0){
	    	clear();
			hr=hrlast=kox_map_unmarshal(&p_kox,pstream);
	}

	inline void reset(i_kox_obj* p){
		     if(p) p->AddRef();			
	    	clear();			
			p_kox=p;
	}

	kox_map_helper_t(i_kox_stream* pstream=0){
		reset(pstream);
	}

	kox_map_helper_t(i_kox_obj* p){
		reset(p);
	}
	inline int count()
	{
		
		if(SUCCEEDED(hrlast=kox_map_info(p_kox,&ppnames,&cb)))
		return cb;
		else return -1;
	}
	inline char** names()
	{
		if(SUCCEEDED(hrlast=kox_map_info(p_kox,&ppnames,&cb)))
		return ppnames;
		else return NULL;
	}

	inline region_t& get(const char* name)
	{ 
		region_t* pr;
		if(SUCCEEDED(hrlast=kox_map_get(p_kox,name,(ipc_region_base_t**)&pr)))  
			return *pr;
		else return region_null;
	}

	inline kox_map_helper_t& get(const char* name,region_t& r)
	{ 
		region_t* pr;
		if(SUCCEEDED(hrlast=kox_map_get(p_kox,name,(ipc_region_base_t**)&pr)))  
			r= *pr;
		else r= region_null;
		return *this;
	}
	inline kox_map_helper_t&  set(const char* name,region_t r)
	{ 
		hrlast=kox_map_set(p_kox,name,&r);
		return *this;
	}
	inline  kox_map_helper_t&  erase(const char* name)
	{ 
		hrlast=kox_map_set(p_kox,name,0);
		return *this;
	}
	inline bool marshal(i_kox_stream* pstream)
	{
		return SUCCEEDED(hrlast=kox_map_marshal(p_kox,pstream));
	}

	inline kox_map_helper_t& operator >>(i_kox_stream* pstream)
	{
		marshal( pstream);
		return *this;
	}
	inline kox_map_helper_t& operator <<(i_kox_stream* pstream)
	{
		reset( pstream);
		return *this;
	}

protected:
	int cb;
	char** ppnames;

};




#endif