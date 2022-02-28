// kox.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"






//
#include "ipc_ports/io_port.h"
#include "rc_holder.h"
//#include "ipc_ports/ipc_marshal_def.h"
#include "ipc_ports/lh_marshal.h"
#include "ipc_ports/lh_marshal2.h"

#include "ipc_ports/kox.h"
#include "ipc_ports/istream_pipe.h"

//


//typedef io_port_t<pipe_io_object,io_binder_t> io_pipe_port_t;




//ipc_utils::smart_ptr_t<i_holder_t<io_pipe_port_t>>  sh(new i_holder_ptr_t<DDS>( new DDS),0);

// {C491A6A4-34E3-4584-8079-1D1CE127CAA0}
const IID pipe_port_iid = 
{ 0xc491a6a4, 0x34e3, 0x4584, { 0x80, 0x79, 0x1d, 0x1c, 0xe1, 0x27, 0xca, 0xa0 } };
// {46DC750E-E8CF-48da-9834-0FAFF5FD350A}
static const IID lh_marshal_iid = 
{ 0x46dc750e, 0xe8cf, 0x48da, { 0x98, 0x34, 0xf, 0xaf, 0xf5, 0xfd, 0x35, 0xa } };


//typedef i_holder_t<io_pipe_port_t,__uuidof(IDispatch)> pipe_port_holder_t;



/*
template <class CH>
pipe_port_holder_t(CH* options_zz):io_port_t<pipe_io_object,io_binder_t>()
{
options=opt_t(true,PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT).load(argv_zz<CH>(options_zz));
}*/


struct  __declspec(uuid("{C491A6A4-34E3-4584-8079-1D1CE127CAA0}")) io_pipe_port_t
	:io_port_t<pipe_io_object,io_binder_t>{};

typedef i_holder_t<io_pipe_port_t,__uuidof(io_pipe_port_t)> pipe_port_holder_t;


struct  __declspec(uuid("{46DC750E-E8CF-48da-9834-0FAFF5FD350A}")) marshal_t
	:lh_marshal_t{};


struct  __declspec(uuid("{FA4FCB79-D923-466D-B30D-30A7A3474C50}")) marshal_map_t
	:lh_marshal_map_t{};


typedef i_holder_t<marshal_t,__uuidof(marshal_t)> marshal_holder_t;

typedef i_holder_t<marshal_map_t,__uuidof(marshal_map_t)> marshal_map_holder_t;


extern "C" HRESULT io_pipe_port_connectW(const wchar_t* url,i_kox_stream** ppstream)
{
	HRESULT hr;
	return   hr=io_pipe_port_t::connect(url,ppstream);

}

extern "C" HRESULT io_pipe_port_connectA(const char* url,i_kox_stream** ppstream)
{
	HRESULT hr;
	return   hr=io_pipe_port_t::connect(url,ppstream);
}



template <class CH>
inline HRESULT _pipe_port_create_and_listen(const CH* url,const CH* options_zz,i_kox_obj** ppport,void* pcallback=0,void* pcontext=0)
{
	if(!ppport) return E_POINTER;
	HRESULT hr;


	ipc_utils::smart_ptr_t<pipe_port_holder_t>  port(new pipe_port_holder_t,0);
	port->t.options=io_pipe_port_t::opt_t(true,PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT).load(argv_zz<CH>(options_zz));
	port->t.create(url);
	hr=port->QueryInterface(__uuidof(i_kox_obj),(void**)ppport);
	if((pcallback)&&SUCCEEDED(hr))
		hr=port->t.listen(pcallback,pcontext);


	return hr;
}


extern "C" HRESULT io_pipe_port_createW(const wchar_t* url,const wchar_t* options_zz,i_kox_obj** ppport)
{
	return _pipe_port_create_and_listen(url,options_zz, ppport);
}

extern "C" HRESULT io_pipe_port_createA(const char* url,const char* options_zz,i_kox_obj** ppport)
{
	return _pipe_port_create_and_listen(url,options_zz, ppport);
}


extern "C" HRESULT io_pipe_port_create_runW(const wchar_t* url,const wchar_t* options_zz,void* pcallback,void* pcontext,i_kox_obj** ppport)
{
	if(!pcallback) return E_POINTER;
	return _pipe_port_create_and_listen(url,options_zz, ppport,pcallback,pcontext);
}

extern "C" HRESULT io_pipe_port_create_runA(const char* url,const char* options_zz,void* pcallback,void* pcontext,i_kox_obj** ppport)
{
	if(!pcallback) return E_POINTER;
	return _pipe_port_create_and_listen(url,options_zz, ppport,pcallback,pcontext);
}


template<class T>
inline bool _check_get_object(i_kox_obj* pport,T*& p,HRESULT &hr)
{
	hr=E_POINTER;
	return (pport)&&SUCCEEDED(hr=pport->QueryInterface(__uuidof(T),(void**)&p));		
}



extern "C" HRESULT io_pipe_port_listen(i_kox_obj* pport,void* pcallback,void* pcontext)
{
	HRESULT hr;
	io_pipe_port_t* p=0;
	if(_check_get_object(pport,p,hr))
		hr=p->listen(pcallback,pcontext);
	return hr;
}

extern "C" HRESULT io_pipe_port_shutdown(i_kox_obj* pport,bool force)
{
	HRESULT hr;
	io_pipe_port_t* p=0;
	if(_check_get_object(pport,p,hr))
		p->shutdown(force);
	return hr;
}


extern "C" HRESULT io_pipe_port_get_waitable_event(i_kox_obj* pport,HANDLE* pevent)
{
	HRESULT hr;
	io_pipe_port_t* p=0;
	if(_check_get_object(pport,p,hr))
		hr=p->monitor->get_waitable_event(pevent);
	return hr;
}


extern "C" HRESULT kox_marshal(i_kox_stream* pstream,int region_count, ipc_region_base_t** pp_regions,const GUID* pconn_uuid)
{
      return kox_marshal_to_process(0,pstream, region_count, pp_regions,pconn_uuid);
}
extern "C" HRESULT kox_marshal_to_process(int pid,i_kox_stream* pstream,int region_count, ipc_region_base_t** pp_regions,const GUID* pconn_uuid)
{
	HRESULT hr;
	const GUID& conn_uuid=(pconn_uuid)?(*pconn_uuid):IID_NULL;
	return hr=lh_marshal_t::marshal(region_count,pp_regions,pstream,pid,conn_uuid);
}

extern "C" HRESULT kox_marshal_array(i_kox_stream* pstream,int region_count, ipc_region_base_t* p_regions,const GUID* pconn_uuid)
{
	HRESULT hr;
	const GUID& conn_uuid=(pconn_uuid)?(*pconn_uuid):IID_NULL;
	//v_buf<ipc_region_base_t*> pp_regions(region_count);
	//	v_buf<ipc_region_base_t*,utils::alloc::stack_alloc<ipc_region_base_t*,REGION_STACK_COUNT> > pp_regions(region_count);
	//
	v_buf<ipc_region_base_t* > pp_regions(region_count);
	for(int n=0;n<region_count;++n) pp_regions[n]=p_regions++;
	return hr=lh_marshal_t::marshal(region_count,pp_regions.get(),pstream,0,conn_uuid);
}


extern "C" HRESULT kox_unmarshal(i_kox_stream* pstream,i_kox_obj** pp_koxdata)
{
	HRESULT hr;
	ipc_utils::smart_ptr_t<marshal_holder_t>  marshaler(new marshal_holder_t,0);
	if(SUCCEEDED(hr=marshaler->t.unmarshal(pstream)))
		hr=marshaler->QueryInterface(__uuidof(i_kox_obj),(void**)pp_koxdata);

	return hr;
}

extern "C" HRESULT kox_get_unmarshal_uuid(i_kox_obj* p_koxdata,GUID** ppconn_uuid)
{

	HRESULT hr;
	if(ppconn_uuid==0) return E_POINTER;
	marshal_t * pm;
	if(_check_get_object(p_koxdata,pm,hr))
		*ppconn_uuid=&(pm->pmd->header.conn_uuid);
	return hr;

}
extern "C" HRESULT kox_get_unmarshal_data(i_kox_obj* p_koxdata,int* pcbcount,ipc_region_base_t*** ppp_regions)
{
	HRESULT hr;
	if((pcbcount==0)||(ppp_regions==0)) return E_POINTER;
	marshal_t * pm;
	if(_check_get_object(p_koxdata,pm,hr))
	{
		*pcbcount=pm->get_region_count();
		*ppp_regions=pm->get_regions_ptr();

	}
	return hr;

}

extern "C" long kox_get_region_att(ipc_region_base_t* p_regions)
{
	if(!p_regions) return IPCM_ERROR;
	return p_regions->attr;
}

extern "C" long kox_get_region_size(ipc_region_base_t* p_regions)
{
	if(!p_regions) return 0;
	return p_regions->sizeb;
}

extern "C" char* kox_get_region_ptr(ipc_region_base_t* p_regions)
{
	if(!p_regions) return 0;
	return p_regions->ptr;
}


extern "C" HRESULT kox_get_unmarshal_com_object(ipc_region_base_t* p_regions,void** ppobj,const GUID* piid)
{
	HRESULT hr=E_HANDLE;
	if(!p_regions) return E_POINTER;

	if((p_regions->attr&(IPCM_COMSTUB|IPCM_HANDLE))&&p_regions->pcomstub)
	{
		const GUID& iid=(piid)? *piid:__uuidof(i_kox_obj);
		HGLOBAL hg=*p_regions->pcomstub;
		if(ipc_utils::CheckCOMStub(hg,hr)&8)
			hr=ipc_utils::CoUnmarshalInterfaceFromHGlobal(hg,iid,ppobj);

	}
	return hr;

}

//region_type_auto_detect(region_t& r)
extern "C" long kox_detect_type(void* p)
{
	lh_marshal_t::region_t r;
	r.attr=0;
	r.ptr_v=&p;
	lh_marshal_t::region_type_auto_detect(r);
	return r.attr;
}



extern "C" HRESULT kox_map_unmarshal(i_kox_obj** pp_map,i_kox_stream* pstream)
{
	if(!pp_map) return E_POINTER;
	HRESULT hr;	
	//ipc_utils::smart_ptr_t<pipe_port_holder_t>  port(new pipe_port_holder_t,0);
	ipc_utils::smart_ptr_t<marshal_map_holder_t>  mh(new marshal_map_holder_t,0);


	if((pstream)&&FAILED(hr=mh->t.unmarshal(pstream))) return hr;

	return hr=mh->QueryInterface(__uuidof(i_kox_obj),(void**)pp_map);


}

extern "C" HRESULT kox_map_create(i_kox_obj** pp_map)
{
	return kox_map_unmarshal(pp_map,0);
}


extern "C" HRESULT kox_map_marshal(i_kox_obj* p_map,i_kox_stream* pstream,int pid)
{
	if(!((p_map)&&(pstream))) return E_POINTER;
	HRESULT hr;	
	//ipc_utils::smart_ptr_t<pipe_port_holder_t>  port(new pipe_port_holder_t,0);

	marshal_map_t * pm;



	if(_check_get_object(p_map,pm,hr)){
		if(!pm->check_tid()) return hr=RPC_E_ATTEMPTED_MULTITHREAD;
		hr=pm->marshal(pstream,pid);
	}

	return hr;
}

inline  ipc_region_base_t* set_default_region_size(ipc_region_base_t*& pregion,ipc_region_base_t& r)
{
	if(pregion)
	{
		 r=*pregion;
		if( ((r.attr&(~IPCM_REGIONMAP))==0 )&&(int(r.sizeb)<0)&&(r.ptr)) 
	{
		r.sizeb=(int(r.sizeb)==-2)?safe_len_zerozero(r.ptr):safe_len(r.ptr);
		pregion=&r;
	}

	}
	return pregion;
}

extern "C" HRESULT kox_map_set(i_kox_obj* p_map,const char* name,ipc_region_base_t* pregion)
{
	if(!((p_map)&&(name))) return E_POINTER;



	HRESULT hr;	
	marshal_map_t * pm;
	if(_check_get_object(p_map,pm,hr))
	{      
		   ipc_region_base_t tmp;
		   //set_default_region_size(pregion,tmp);
		if(!pm->check_tid()) return hr=RPC_E_ATTEMPTED_MULTITHREAD;
		pm->set_region(name,pregion);
	}

	return hr;
}

extern "C" HRESULT kox_map_set_data(i_kox_obj* p_map,const char* name,void* ptr,long cbyte,long attr)
{
	if(!((p_map)&&(name))) return E_POINTER;
	HRESULT hr;	
	ipc_region_base_t r;
	r.ptr_v=ptr;
	r.sizeb=(ptr)?cbyte:0;
	r.attr=attr;

	/*
	if( ((attr&(~IPCM_REGIONMAP))==0 )&&(cbyte<0)&&(ptr)) 
	{
		r.sizeb==(cbyte==-2)?safe_len_zerozero(r.ptr):safe_len(r.ptr);

	}
	*/
	return hr=kox_map_set(p_map,name,&r);		

}



extern "C" HRESULT kox_map_get(i_kox_obj* p_map,const char* name,ipc_region_base_t** ppregion)
{
	if(!((p_map)&&(name)&&(ppregion))) return E_POINTER;

	HRESULT hr;	
	marshal_map_t * pm;
	if(_check_get_object(p_map,pm,hr))
	{
		if(!pm->check_tid()) return hr=RPC_E_ATTEMPTED_MULTITHREAD;
		*ppregion=pm->region_map[name];
	}

	return hr;
}

extern "C" HRESULT kox_map_info(i_kox_obj* p_map,char*** pppnames,int* pcb)
{
	if(!(p_map)) return E_POINTER;
	char** tmp;
	int itmp;
	pppnames||(pppnames=&tmp);
	pcb||(pcb=&itmp);


	HRESULT hr;	
	marshal_map_t * pm;
	if(_check_get_object(p_map,pm,hr))
	{
		if(!pm->check_tid()) return hr=RPC_E_ATTEMPTED_MULTITHREAD;
		*pcb=pm->get_names(pppnames);
	}

	return hr;
}

extern "C" HRESULT kox_get_fd(i_kox_stream* ps,HANDLE* ph)
{
	if(!((ps)&&(ph))) return E_POINTER;
	HRESULT hr;	
	i_get_handle_t * gh;
	if(_check_get_object(ps,gh,hr))
		hr=gh->get_handle(ph);
	return hr;
}

extern "C" HRESULT kox_get_pipe_stream(IUnknown* pUnk,HANDLE h_r,HANDLE h_w,i_kox_stream** ppstream)
{
	
        HRESULT hr=ISequentialStreamPipe_Impl<>::CreateInstance(pUnk,h_r,h_w,(ISequentialStream**)ppstream);
    return hr ;  
}
extern "C" HRESULT kox_get_stream_pair(IUnknown* pUnk,i_kox_stream** ppstream1,i_kox_stream** ppstream2)
{
    HRESULT hr=ISequentialStreamPipe_Impl<>::CreateStreamPair(pUnk,(ISequentialStream**)ppstream1,(ISequentialStream**)ppstream2);
	return hr;
}





//