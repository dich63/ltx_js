#pragma once
#include "hss_port.h"
#include "wchar_parsers.h"

struct hss_asyn_connector_t
{
	typedef socket_port<> port_t;

	HRESULT hr;
	v_buf<char> host_port,ns_path;
	
	argv_zzs<char> argszz;
	
	ptr_holder<port_t::SOCKET_DATA_h> hs;
	handle_holder habortevent;
	ptr_holder< port_t > inner_port;
	



  hss_asyn_connector_t(char* _host_port,char* _ns_path,char* _argszz,char* portname,void* pcallback,void * hcontext,int hsf_flags=HSF_SRD_AUTO)
  {
           //port.attach(hport,1);
	    hr=E_FAIL;

		v_buf<char> vportname;
		
		if(_host_port)
		{
         host_port.cat(_host_port);
		 ns_path.cat(_ns_path);
		}
		else copy_zz(_ns_path,ns_path);

		argszz.init(_argszz,true);

		habortevent.attach(CreateEvent(0,1,0,0));

		if(!portname)
		{
			vportname.cat("::").cat(uuid_gen());
           portname=vportname;
			inner_port=port_t::create_NS(portname,HSF_SRD_VIRTUAL| hsf_flags,pcallback,hcontext,CP_UTF8);
			if(inner_port) inner_port->port.synchro_list->fport_destroy_notify=0;
			else return;
		}
		else    
	   {
		   hr=hss_check_port_exists(portname);
		   if(hr!=0) return;
	   }
		 hs=shared_heap<port_t::SOCKET_DATA_h>().allocate_holder(); 
		 //hs->set_region(HSD_CONN_PORT,portname);
		 hs->set_region(HSD_URL,portname);
		 hs->hkcount=2;
		 hs->hkObjects[0]=habortevent.attach(CreateEvent(0,1,0,0));
		 hr=S_OK;
        
  }


inline  HRESULT connect()
{
	HRESULT hr=E_FAIL;
	ptr_holder<char> hparams;
	SOCKET s=hss_websocket_connect_ex_HSS_INFO(host_port,ns_path,(char*)argszz.flat_str(),hparams.address(),hs);
	if(s!=INVALID_SOCKET) 
	{
    	if(hs->get_region(HSD_CONN_PORT))
	  {

		closesocket(s);
		return hr=S_OK;
	  }
		
		
	}

	hs->hkcount=1;
	hs->attach_region(HSD_SOCKET_REF,hparams.detach());
	hs->s=s;
	hs->fclose_after=3;
    //hr=hss_send_SOCKET_DATA(hs);
	socket_port<> sp;
	hr=sp.send_NS(hs,1,0);
	//if((inner_port)&&(hr==0))		inner_port->port.synchro_list->pinterceptor_context=0;
	          
			   //interceptor_proc
	
	return hr;

}

inline void operator()()
  { 
	 connect();
     //SetEvent(habortevent);
  }

inline static PROC create_connector(char* host_port,char* ns_path,char* argszz,char* portname,void* pcallback,void * hcontext,int hsf_flags=HSF_SRD_AUTO)
{
	 
     hss_asyn_connector_t* pac=new hss_asyn_connector_t(host_port,ns_path,argszz,portname,pcallback,hcontext,hsf_flags);

	 hss_port_base::LastErrorSaver lerr(pac->hr);

	 if(pac->hr) return 0;
	 ptr_holder<event_signaler> evnt=make_event_signaler(pac->habortevent);
	 HRESULT err=asyn_call(pac);
	 lerr =err;
     if(err) return 0;
	 void* p=evnt.detach();
	 return PROC(p);
}

};