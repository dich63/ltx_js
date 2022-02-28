// ltx_websocket.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"


#include "ltxjs.h"
#include "ipc_ports/ipc_utils.h"
#include "wchar_parsers.h"
#include "singleton_utils.h"
#include "ipc_ports/sha1.h"
#include "ipc_ports/wbs_buffer.h"
#include "ipc_ports/websocket_handler.h"
#include <strstream>


typedef ipc_utils::com_scriptor_t javascript_t;
typedef ipc_utils::dispcaller_t<javascript_t::exception_t> caller_t;

typedef javascript_t::value_t jsvalue_t;
typedef i_mbv_buffer_ptr::int64_t int64_t;
typedef ltx_helper::arguments_t<jsvalue_t>  arguments_t;
typedef i_mm_region_ptr::int64_t int64_t;

LPFN_CONNECTEX ConnectEx=ipc_utils::WSA_initerfiniter::get_instance().ConnectEx;
#define WBSOCKET_UUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define  CHUNK_SIZE (4*1024*1024)
  
struct HSSOVL:OVERLAPPED
{
	HANDLE hAbortEvent;
	int hkcount; 
	HRESULT   error_code;
	ULONG_PTR user_data;
	SOCKET s;
	HSSOVL(SOCKET _s,HANDLE _hAbortEvent):OVERLAPPED(),error_code(0),user_data(0),hkcount((_hAbortEvent)?2:1),hAbortEvent(_hAbortEvent),s(_s)
	{
		;	
      hEvent=CreateEvent(0,true,0,0); 
	}
	~HSSOVL(){
		CloseHandle(hEvent);
	}


};


struct socket_data_helper_t
{
  HRESULT& hr;
  arguments_t& arguments;
  pshared_socket_data_t socket_data;
  ipc_utils::smart_ptr_t<i_mbv_socket_context> socket_context;
  socket_data_helper_t(arguments_t& _arguments,int n=0):arguments(_arguments),hr(_arguments.hr),socket_context(0)
  {
	  OLE_CHECK_VOID(hr=arguments[n].QueryInterface(socket_context._address()));
	  OLE_CHECK_VOID(hr=socket_context->GetContext((void**)&socket_data));
  }
  inline operator HRESULT(){ return hr;}

  inline operator pshared_socket_data_t(){ return socket_data;}
  inline pshared_socket_data_t operator ->(){ return socket_data;}


};


struct sockinfo_t
{
	char peername[64];
	char sockname[64];
	char report[128];
	HRESULT hr;
	sockinfo_t(SOCKET s)
	{
		strcpy(peername,"??.??.??.??:??");
		strcpy(sockname,"??.??.??.??:??");
		sockaddr_in soi;
		char* p;
		unsigned long pn;
		int sz;
		sz=sizeof(sockaddr_in);
		HRESULT  err,hr;
		err=getpeername(s,(sockaddr*)&soi,&sz);
		if(err==SOCKET_ERROR)
		{OLE_CHECK_VOID(hr=HRESULT_FROM_WIN32(GetLastError()));}		
		else  sprintf(peername,"%s:%d",inet_ntoa(soi.sin_addr),htons(soi.sin_port));
		sz=sizeof(sockaddr_in);
		if(0!=getsockname(s,(sockaddr*)&soi,&sz))
		{OLE_CHECK_VOID(hr=HRESULT_FROM_WIN32(GetLastError()))}				
		else  sprintf(sockname,"%s:%d",inet_ntoa(soi.sin_addr),htons(soi.sin_port));

		sprintf(report,"sockname=%s ;peername=%s",sockname,peername);

	}
};


struct host_port_resolver_t
{

	

	char* host;
	char* port;
	char* ip;
	int portn;
	struct hostent* hs;
	sockaddr_in ai;	
	HRESULT hr0;



	host_port_resolver_t():hr0(E_FAIL){};


	template <class CH>
	host_port_resolver_t(const CH* _host,const CH* _port)
	{
       init(_host,_port);
	}
	template <class CH>
	HRESULT init(const CH* _host,const CH* _port)
   {
	   
	   //:host(_host?_host:"0.0.0.0"),port(_port?_port:"8080")
	   host=sp.get_cache_encode(_host);
	   port=sp.get_cache_encode(_port);
	   portn=atoi(port);
	   hs=gethostbyname(host);
	   if(!hs)	   	   
		   return hr0=HRESULT_FROM_WIN32(WSAGetLastError());
	   
	   memset(&ai,0,sizeof(ai));
	   ip= inet_ntoa (*(struct in_addr *)*hs->h_addr_list);
	   ai.sin_family=AF_INET;	   
	   ai.sin_addr.s_addr =*(u_long *) hs->h_addr_list[0];
	   ai.sin_port=htons(portn);
	   return hr0=S_OK;
   }

	inline operator HRESULT(){return hr0;}

 HRESULT connect(SOCKET s,HANDLE hAbortEvent)
 {
	 DWORD dw;
	 HRESULT hr,err;
	 BOOL f;
	 OLE_CHECK_WIN32_hr(hr0);

	 
	 LPFN_CONNECTEX ConnectEx;
	 GUID wiid=WSAID_CONNECTEX;
	 OLE_CHECK_WIN32_hr(WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,&wiid,sizeof(wiid),&ConnectEx,sizeof(ConnectEx),&dw,0,0));

	 sockaddr_in sa={};	 
	 sa.sin_family=ai.sin_family; 	 
	 if(::bind( s,(SOCKADDR*) &sa,sizeof(sa)))
	  return hr=HRESULT_FROM_WIN32(WSAGetLastError());

	 struct handle_holder_t{

		 HANDLE h;

		 ~handle_holder_t(){CloseHandle(h);}
	 } handle_holder={CreateEvent(0,1,0,0)};



	 OVERLAPPED ovl={};
	 ovl.hEvent=handle_holder.h;
	 HANDLE hh[2]={ovl.hEvent,hAbortEvent};

     int tio,r,tior,sz;

	 f=ConnectEx(s,(struct sockaddr*)&ai,sizeof(ai),0,0,&dw,&ovl);
	 if((!f)&&((err=WSAGetLastError())==ERROR_IO_PENDING))
	 {
		 DWORD ws;
		 ws=WaitForMultipleObjects(2,hh,false,INFINITE); 
		 if(ws!=WAIT_OBJECT_0) hr=err=E_ABORT;
		 else
		 {
			 f=GetOverlappedResult(HANDLE(s),&ovl,&dw,1);
			 if(f&&(0==setsockopt( s, SOL_SOCKET,SO_UPDATE_CONNECT_CONTEXT,NULL,0)))
			 		 hr=S_OK;
			 else return hr=HRESULT_FROM_WIN32(err=WSAGetLastError());

			 
			 r=getsockopt(s, SOL_SOCKET,SO_SNDTIMEO,(char*)(&(tio=11)),&(sz=sizeof(tio)));
			 r=getsockopt(s, SOL_SOCKET,SO_RCVTIMEO,(char*)(&(tior=111)),&(sz=sizeof(tior)));


		 }
		 
	 }
	 else   hr=err;

	 
     

	 return HRESULT_FROM_WIN32(hr);
 }

	 

 


   s_parser_t<char> sp;
   
};


template <class Stream >
Stream& set_websocket_GET_8(Stream& stream,wbs_sh1_encoder_t& wbsh1,char* host,char* url,char* argszz=0)
{

	size_t lb=0;
	std::vector< char*> vargs=args_ZZ(argszz,&lb);
	v_buf<char> buf(lb=256+3*lb);
	char *p=url;

	stream<<"GET /";
	while(*p=='/') ++p;
	for(;(*p)&&(!isspace(*p));++p)
		stream<<(*p);

	int cb;
	int narg=vargs.size();
	int n=0;
	p=buf;

	if(narg)
	{

		if(url_escape(vargs[0],-1,buf,&(cb=lb))){
			stream<<"?"<<(char*)buf;
			for(n=1;n<narg;++n)	{
				if(url_escape(vargs[n],-1,buf,&(cb=lb)))
					stream<<"&"<<(char*)buf;
			} 
		}
	}


	stream<<" HTTP/1.1\r\n";
	stream<<"Upgrade: WebSocket\r\n";
	stream<<"Connection: Upgrade\r\n";
	stream<<"Host: "<< host<<"\r\n";

	stream<<"Sec-WebSocket-Key: ";
	stream<<wbsh1.base64code<<"\r\n";

	stream<<"Sec-WebSocket-Version: 13\r\n";

	stream<<"Origin: null\r\n";

	stream<<"\r\n";


	return stream;
}




extern "C"  void   tcp_connect
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	socket_data_helper_t sd(arguments);
	OLE_CHECK_VOID(hr=sd);
	if(sd->state&shared_socket_data_t::CONNECTED)
		OLE_CHECK_VOID(hr=HRESULT_FROM_WIN32(WSAEISCONN));
	host_port_resolver_t hpr((wchar_t*)arguments[1].def<bstr_t>(L"localhost"),(wchar_t*)arguments[2].def<bstr_t>(L"7777"));
	OLE_CHECK_VOID(hr=hpr.connect(sd->sock,sd->habort_event));
	sd->state=shared_socket_data_t::CONNECTED;


    argv_zzs<char> args;
	args["host"]=hpr.ip;
	args["port"]=hpr.port;

	sockinfo_t si(sd->sock);		
		
        
		args["peername"]=si.peername;
		args["sockname"]=si.sockname;

		SysFreeString(sd->header);
		int cb;
		const char* pargs=args.flat_str(0,0,&cb);
		sd->header=SysAllocStringLen(NULL,cb);
		char_mutator<CP_UTF8> cm(pargs,0,true);
		memcpy(sd->header,(wchar_t*)cm,sizeof(wchar_t)*(cb+1));
        



	
	result=(wchar_t*) char_mutator<CP_UTF8>(si.report);
//	ipc_utils::smart_ptr_t<i_mbv_socket_context> socket_context;

}

extern "C"  void   ws_connect
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{


	struct sockgetter_t
	{
		SOCKET s;
		sockgetter_t(SOCKET _s):s(_s){};
		inline int operator()(void* p,int count,int flags=0) const
		{
			return ::recv(s,(char*)p,count,flags);
		}
	
		
	};
	struct region_ptr_t
	{
		long sizeb;
		char* ptr;

	};



	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;	





	socket_data_helper_t sd(arguments);
	OLE_CHECK_VOID(hr=sd);
	if(sd->state!=shared_socket_data_t::CONNECTED)	
		OLE_CHECK_VOID(hr=HRESULT_FROM_WIN32(WSAENOTCONN));

	s_parser_t<char> sp;

    jsvalue_t vargs;
	VARIANT v=arguments[2];
	OLE_CHECK_VOID(hr=VariantChangeType(&vargs,&v,0,VT_BSTR));

	

	argv_zzs<wchar_t> wargs_sock(sd->header);

	

	 char* host=sp.get_cache_encode((wchar_t*)wargs_sock[L"host"],CP_UTF8);

	 char* url=sp.get_cache_encode((wchar_t*)arguments[1].def<bstr_t>(L""),CP_UTF8);
	

	argv_zz<wchar_t> wargs(vargs.bstrVal);

	argv_zzs<char> args;

	args.convert(wargs,CP_UTF8);

	char* argszz=(char*)args.flat_str();

	std::strstream stream;
    wbs_sh1_encoder_t wbs_sh1(WBSOCKET_UUID);	

	set_websocket_GET_8(stream, wbs_sh1,host,url,argszz);

	int len=stream.pcount(),c;
	stream<<'\0';
	char * psend=stream.str();

	c=::send(sd->sock,psend,len,0);	

     //char buf[2048];
	 //c=::recv(sd->sock,buf,2048,0);
	 std::strstream ins;
	 sockgetter_t sg(sd->sock);
	 c=get_recv_lines(ins,sg);
	 int len2=ins.pcount();
	 ins<<'\0';
    char *p,* precv=ins.str();
	region_ptr_t r={len2,precv};

	  char *pwsb=_GET_HEADER(Sec-WebSocket-Accept,r,&c);
	 if(!wbs_sh1.check(pwsb,c))
	 {
		 ::closesocket(sd->sock);
		 SetEvent(sd->habort_event);
		 arguments.raise_error(L"Sec-WebSocket-Accept Error");
	 }

	 SysFreeString(sd->header);

	 wargs_sock[L"handshake"]=char_mutator<CP_UTF8>(psend);
	 wargs_sock[L"reply"]=char_mutator<CP_UTF8>(precv);
	 const wchar_t* pwargs=wargs_sock.flat_str(0,0,&c);
	 sd->header=SysAllocStringLen(NULL,c);
	 memcpy(sd->header,pwargs,sizeof(wchar_t)*(c+1));
	 p=_GET_HEADER(Upgrade,r,&c);
     if(p) p[c]=0;
	 result=(wchar_t*)char_mutator<CP_UTF8>(p);
	//	ipc_utils::smart_ptr_t<i_mbv_socket_context> socket_context;

}



extern "C"  void   ws_socket2socket
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{


	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	socket_data_helper_t sdr(arguments,0),sds(arguments,0);
	OLE_CHECK_VOID(hr=sdr);
	OLE_CHECK_VOID(hr=sds);

}

extern "C"  void   ws_send
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{


	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	socket_data_helper_t sd(arguments);
	OLE_CHECK_VOID(hr=sd);

	

	


	HSSOVL hovl(sd->sock,sd->habort_event);
	wbs_setter_t<HSSOVL> setter(&hovl);

	 wbs_frame_io_t wbs_frame_io;
	 char* pbuf;
	 

	 int64_t len=0;
	 const int64_t bufsize=CHUNK_SIZE;
	 bool feom;
	 int cb,cba,cba2;


	 VARIANT v=arguments[1]; 
	 ipc_utils::smart_ptr_t<i_mm_region_ptr> region_ptr;

	 wbs_frame_io_t::uint8_t data_type=WSF_BIN;

	 if(v.vt==VT_BSTR)
	 {
       data_type=WSF_TXT;

	   BSTR pw=v.bstrVal;
	   cb=SysStringLen(pw);
	   cba=WideCharToMultiByte(CP_UTF8,0,pw,cb,0,0,0,0);	
	   

	   len=cba;
	   VARIANT v2=arguments[2];
	   if(FAILED(hr=region_ptr.reset(v2))){
		   s_parser_t<wchar_t> sp;
		   wchar_t* monstr=sp.printf(L"ltx.bind:sm_buffer: length=%d; type=uint8;",cba);
		   OLE_CHECK_VOID(hr=::CoGetObject(monstr,0,__uuidof(i_mbv_buffer_ptr),region_ptr._ppQI()));
	   }

	   
	   OLE_CHECK_VOID(hr=region_ptr->GetPtr((void**)&pbuf));
	   cba2=WideCharToMultiByte(CP_UTF8,0,pw,cb,pbuf,cba,0,0);
	   if(cba!=cba2) OLE_CHECK_VOID(hr=E_INVALIDARG);

	 }
	 else
	 {
		 OLE_CHECK_VOID(hr=region_ptr.reset(v));
		 OLE_CHECK_VOID(hr=region_ptr->GetTotalSize(&len));
	 }


     int fcontinue=arguments[3].def<int>(0x03);

	
    	
     if(fcontinue&0x02)  
      OLE_CHECK_VOID(hr=wbs_frame_io.write_frame(setter,0,0,0,data_type,0));

	  int64_t count=len/bufsize;
	  int64_t rem=len-count*bufsize;

	  int64_t offset=0;

	  

	  for(int64_t n=0;n<count;n++)
	  {

          int64_t szb=bufsize;       
		  i_mm_region_ptr::ptr_t<char> ptr(region_ptr,offset,szb);
		   OLE_CHECK_VOID(hr=ptr);
		   pbuf=ptr; 		   
		  OLE_CHECK_VOID(hr=wbs_frame_io.write_frame(setter,pbuf,szb,&cb,WSF_CNT,0));	  
		  offset+=bufsize;

	  }
	  if(rem)
	  {
		  int64_t szb=rem;       
		  i_mm_region_ptr::ptr_t<char> ptr(region_ptr,offset,szb);
		  OLE_CHECK_VOID(hr=ptr);
		  pbuf=ptr; 		  
		  OLE_CHECK_VOID(hr=wbs_frame_io.write_frame(setter,pbuf,szb,&cb,WSF_CNT,0));	  
		  
	  }

        if(fcontinue&0x01)  
           OLE_CHECK_VOID(hr=wbs_frame_io.write_frame(setter,0,0,&cb,WSF_FIN,0));

		  result=double(len);	



	//	ipc_utils::smart_ptr_t<i_mbv_socket_context> socket_context;

}



extern "C"  void   ws_recv
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	const int64_t gb=(1024*1024*1024);
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	socket_data_helper_t sd(arguments);
	OLE_CHECK_VOID(hr=sd);

	VARIANT v=arguments[1]; 
	ipc_utils::smart_ptr_t<i_mm_region_ptr> region_ptr;

	int64_t capacity,length,c;

    if(arguments.argc>1){
          OLE_CHECK_VOID(hr=region_ptr.reset(v));
	}
	else
	{
		const wchar_t* monstr=L"ltx.bind:sm_buffer:length=128;unit=1073741824;type=uint8;flags.sparse=1";
	     OLE_CHECK_VOID(hr=::CoGetObject(monstr,0,__uuidof(i_mbv_buffer_ptr),region_ptr._ppQI()));

	}
	
	OLE_CHECK_VOID(hr=region_ptr->GetTotalSize(&capacity));





	HSSOVL hovl(sd->sock,sd->habort_event);
	wbs_getter_t<HSSOVL> getter(&hovl);

	wbs_frame_io_t wbs_frame_io;

	wbs_frame_io_t::wbs_frame_t* pwbs_frame=(wbs_frame_io_t::wbs_frame_t*)&sd->wbs_frame_read;
    wbs_frame_io_t::wbs_frame_t& wbs_frame=*pwbs_frame;

	int fnext=arguments[3].def<int>(1);
	int flink=arguments[2].def<int>(0);

	hr=wbs_frame.read_frames(getter,fnext);
	wbs_frame_io_t::uint8_t optcode=wbs_frame.opcode;
	wbs_frame_io_t::uint8_t data_type=optcode&(WSF_TXT|WSF_BIN);
	//if(hr!=ERR_EOM) 
	 //OLE_CHECK_VOID(hr);
	
	int64_t offset=0;
	const int64_t chunk_size=CHUNK_SIZE;
	int cb=0;
	while(hr!=ERR_EOM)
	{
		OLE_CHECK_VOID(hr);
		i_mm_region_ptr::ptr_t<char> ptr(region_ptr,offset,chunk_size);
		OLE_CHECK_VOID(hr=ptr);
		char*p=ptr; 		  
		hr=wbs_frame.read_frames(getter,0,p,chunk_size,&cb);
		offset+=cb;
	}

	

	if(data_type&WSF_TXT)
	{
		int len=offset;
		i_mm_region_ptr::ptr_t<char> ptr(region_ptr,0,len);
		OLE_CHECK_VOID(hr=ptr);
		char*putf8=ptr; 		  
		int cb=MultiByteToWideChar(CP_UTF8,0,putf8,len,0,0);
		BSTR bstr=SysAllocStringLen(0,cb);
		int cb2=MultiByteToWideChar(CP_UTF8,0,putf8,len,bstr,cb);
		if(cb2!=cb) {
			SysFreeString(bstr);
			OLE_CHECK_VOID(hr=(cb2==0)?HRESULT_FROM_WIN32(GetLastError()):E_INVALIDARG);	
		}
		result.vt=VT_BSTR;
		result.bstrVal=bstr;

		
	}
	else {
		int64_t len=offset;

		VARIANT r={VT_DISPATCH};
		//DataViewAs(BSTR type,BOOL link,int64_t ByteOffset,int64_t ByteLength,REFIID iid,void** ppObj)=0;
		OLE_CHECK_VOID(hr=region_ptr->DataViewAs(L"uint8",flink,0,len,__uuidof(IDispatch),&r.byref));
		result.Attach(r);

	}
	


	//	ipc_utils::smart_ptr_t<i_mbv_socket_context> socket_context;

}

extern "C"  void   ws_sendrecv
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{

     jsvalue_t r;
    ws_send(r,lib,callback_context);
	OLE_CHECK_VOID(*(callback_context->phr));
	int narg=callback_context->argc;
	std::vector<VARIANT> vv(narg);
	vv[narg-1]=callback_context->argv[narg-1];
    for(int k=0;k<narg-2;k++)
		vv[1+k]=callback_context->argv[k];
	callback_context->argv=&vv[1];
	callback_context->argc=narg-1;
	ws_recv(result,lib,callback_context);

	//	ipc_utils::smart_ptr_t<i_mbv_socket_context> socket_context;

}


struct event_handler_t
{
     HANDLE hevent,hko;
	 ipc_utils::smart_GIT_ptr_t<IDispatch> dispevent;
	 event_handler_t(IDispatch*pdisp,HANDLE _hevent):
	 dispevent(pdisp),hko(0),hevent(ipc_utils::dup_handle(_hevent)){}
	~event_handler_t(){


		HRESULT hr;

		{
			ipc_utils::COMInitF_t ci;

			ipc_utils::smart_ptr_t<IDispatch> disp;
			if(SUCCEEDED(dispevent.unwrap(disp)))
			{
				DISPPARAMS dp={};
				jsvalue_t r;
				hr=disp->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_METHOD,&dp,&r,0,0);
			}



		}
		

      CloseHandle(hevent);  
      hko&&UnregisterWait(hko);  
	}


  static void __stdcall  s_proc (void* p, BOOLEAN f){
        
	  delete ((event_handler_t*)p);
   }
	;   

};



extern "C"  void   set_socket_close_event
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{

	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	socket_data_helper_t sd(arguments);
	OLE_CHECK_VOID(hr=sd);
	if(SOCKET_ERROR==WSAEventSelect(sd->sock,sd->habort_event,FD_CLOSE))
		hr=HRESULT_FROM_WIN32(WSAGetLastError());	
}

extern "C"  void   add_socket_close_event_handler
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{

	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	socket_data_helper_t sd(arguments);
	OLE_CHECK_VOID(hr=sd);

	ipc_utils::smart_ptr_t<IDispatch> disp;
	OLE_CHECK_VOID(hr=arguments[1].QueryInterface(disp._address()));

	event_handler_t* peh=new event_handler_t(disp,sd->habort_event);

	if(!RegisterWaitForSingleObject(&peh->hko,peh->hevent,&event_handler_t::s_proc,peh,INFINITE,WT_EXECUTELONGFUNCTION|WT_EXECUTEONLYONCE))
	{
		hr=HRESULT_FROM_WIN32(GetLastError());
		delete peh;
	}



}

struct process_killer_t
{
	DWORD tio,pid;
	HANDLE hevent,hko;
	
	
	process_killer_t(HANDLE h,DWORD t=200,DWORD _pid=GetCurrentProcessId())
		:tio(t),hevent(ipc_utils::dup_handle(h)),pid(_pid){};
	static void __stdcall  s_proc (void* p, BOOLEAN f){
 
        ((process_killer_t*)p)->proc();
		
		//TerminateProcess(GetCurrentProcess(),ERROR_GRACEFUL_DISCONNECT);
	}
	void proc(){
		DWORD ti=tio,pi=pid;


	//	
		//Sleep( ti);
		//FreeConsole();

		ipc_utils::process_handle_t ph(pi);
		//return;
		WaitForSingleObject(ph.get_handle(PROCESS_TERMINATE|SYNCHRONIZE),ti);

		//FatalAppExitW(0,L"Ebxnmazm");
		ph.terminate(ERROR_GRACEFUL_DISCONNECT);
		delete this;
	}

	void operator()()
	{
        WaitForSingleObject(hevent,INFINITE);
		proc();

	}

	~process_killer_t(){ 

	 CloseHandle(hevent);
	 hko&&UnregisterWait(hko);  
	}
};


extern "C"  void   terminate_process_on_socket_close
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{

	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	socket_data_helper_t sd(arguments);
	OLE_CHECK_VOID(hr=sd);

	if(!ipc_utils::check_socket(HANDLE(sd->sock)))
		OLE_CHECK_VOID(hr=HRESULT_FROM_WIN32(GetLastError()));

	if(!ipc_utils::check_handle(sd->habort_event))
		OLE_CHECK_VOID(hr=HRESULT_FROM_WIN32(GetLastError()));


	int tio= arguments[1].def<int>(1000);
	
	process_killer_t* pk=new process_killer_t(sd->habort_event,tio);

	asyn_call(pk);


/*
	//if(!RegisterWaitForSingleObject(&pk->hko,pk->hevent,&process_killer_t::s_proc,pk,INFINITE,WT_EXECUTELONGFUNCTION|WT_EXECUTEONLYONCE))
	if(!RegisterWaitForSingleObject(&pk->hko,pk->hevent,&process_killer_t::s_proc,pk,INFINITE,WT_EXECUTEONLYONCE))
	{
		hr=HRESULT_FROM_WIN32(GetLastError());
		delete pk;
	}
*/


}


extern "C"  void   socket_error
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{

	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	socket_data_helper_t sd(arguments);
	OLE_CHECK_VOID(hr=sd);
	char c[2];
	HRESULT errcode=0;
	int nerr;
	DWORD cbr=0;
	WSABUF buf={0,c};
	if(0)
	{
		sockaddr_in soi;
		int sz;
		sz=sizeof(sockaddr_in);

		nerr=getpeername(sd->sock,(sockaddr*)&soi,&sz);
		//if(nerr==SOCKET_ERROR)

	}
	//if(nerr=WSARecv(sd->sock,&buf,1,&cbr,0,0,0))
	//	
	if(nerr=recv(sd->sock,c,0,0))
		errcode=WSAGetLastError();
	result=errcode;
}