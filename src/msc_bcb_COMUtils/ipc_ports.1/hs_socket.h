#pragma once 

#include "hssh.h"


struct accept_address_data
{   //int cb;
	int local_length;
	SOCKADDR_IN* plocal_address;
	int remote_length;
	SOCKADDR_IN* premote_address;
	//void( *shutdown)();
	void *shutdown;
};

extern "C"  long hss_addref(void *p);
extern "C"  long hss_acquire(void *p);
extern "C"  long hss_release(void *p);
extern "C"  HRESULT hss_QueryInterface(void *p,IID& iid, void** pp);
extern "C"  HRESULT hss_error();
extern "C"  HRESULT set_hss_error(HRESULT hrn);
extern "C" long   hss_size(void *p);
extern "C"  void* hss_alloc(int size);

extern "C"  void* hss_websocket_handshake(SOCKET s);
extern "C"  void* hss_websocket_handshake_ex(SOCKET s,accept_address_data* paad=0);




extern "C"  SOCKET hss_websocket_connect(char* url,char* argszz=0);
extern "C"  HSS_INFO* hss_websocket_connect2(char* host_port,char* ns_path=0,char* argszz=0);
extern "C"  HSS_INFO* hss_websocket_connect2W(wchar_t* host_port,wchar_t* ns_path=0,wchar_t* argszz=0);
extern "C"  SOCKET hss_websocket_connect_ex(char* host_port,char* ns_path=0,char* argszz=0,char** pp_socket_ref=0);
extern "C"  SOCKET hss_websocket_connect_ex_HSS_INFO(char* host_port,char* ns_path,char* argszz,char** pp_socket_ref=0,void* hss=0);

extern "C"  SOCKET hss_websocket_connect_exW(wchar_t* whost_port,wchar_t* wns_path,wchar_t* wargszz,wchar_t** wpp_socket_ref=0);

extern "C"  long hss_send_SOCKET_DATA(void *p);

extern "C"  long hss_send_to_port(char* portURL,SOCKET s,char* host,char *confirm_msg,char* params);

extern "C"  long hss_wait_port_creation(char* portURL,long time_out=INFINITE,bool falertable=false);
extern "C"  long hss_wait_port_creationW(wchar_t* portURL,long time_out=INFINITE,bool falertable=false);

extern "C"  long hss_check_port_existsW(wchar_t* portname);
extern "C"  long hss_check_port_exists(char* portname);

extern "C" void* hss_create_socket_portW(wchar_t* filename,ULONG reply_mode=3, void* pcallback=HSD_IO_PORT_PTR,void * hcontext=HSD_SAME_PTR);
extern "C"  void* hss_create_socket_port(char*  filename,ULONG reply_mode=3,void* pcallback=HSD_IO_PORT_PTR,void * hcontext=HSD_SAME_PTR);

extern "C" void*   hss_SOCKET_DATA_create(char* url=0,SOCKET s=INVALID_SOCKET);

extern "C" HSS_INFO*   hss_socket_descriptor_create(SOCKET s=INVALID_SOCKET,unsigned int freeonclose=1);

extern "C" SOCKET hss_reset_socket(void *psd,int flags=0,SOCKET snew=-1);
extern "C" char*   hss_reset_SOCKET_DATA(void *psd,int n,int flags=0,void* pv=0,int cb=-1);
extern "C" char*   hss_reset_socket_data(void *psd,int n,int flags=0,void* pv=0,int cb=-1);




extern "C"  long hss_port_shutdown(void* p);
extern "C" HRESULT __stdcall  hss_port_shutdown_by_nameW(wchar_t* portname);
extern "C" HRESULT __stdcall  hss_port_shutdown_by_name(char* portname);


extern "C"  long hss_wait_port_shutdown(void* p,int timeout=-1,bool falerable=0,int wakemask=0);
extern "C"  SOCKET hss_accept(void* p, void** ppsd=NULL);
extern "C"  void* hss_accept_descriptor(void* p);

extern "C"  void* hss_create_stdcall_closure(void* proc,void* pcontext=0);

extern "C"  void* hss_weak_ref(void * h, void* proc,void* pcontext=0);

extern "C"  char * hss_info(void * h,int n=0);
extern "C"  void** hss_info_ll(void * h);

extern "C"  void * hss_socket_server_pool(char* addressport,void* proc=0,void* proch=HSD_SAME_PTR);
extern "C"  void * hss_socket_server_pool_chain(char* addressport,void* proc=HSD_IO_PORT_PTR,void* proch=HSD_SAME_PTR);

extern "C" ULONG hss_SOCKET_DATA_state(void *psd);


extern "C"  ULONG hss_websocket_handshake_chain_ex(void* phsd,SOCKET s,void* pevent=0,accept_address_data* paad=0,void** pphsdout=0);
extern "C"  ULONG hss_HTTP_request_headers_chain_ex(void* phsd,SOCKET s,void* pevent=0,accept_address_data* paad=0,void** pphsdout=0);

extern "C"  ULONG hss_websocket_handshake_chain(void* phsd);
extern "C"  ULONG  hss_websocket_handshake_reply(void* phsd);

extern "C"  ULONG hss_HTTP_request_headers_chain(void* phsd);


extern "C"  PROC hss_create_event_closure();
extern "C"  PROC hss_create_signaler();

extern "C"  DWORD hss_wait_event(void* ph,int to=INFINITE);
extern "C"  DWORD hss_wait_signaled(void* ph,int to=INFINITE);

extern "C"  PROC hss_get_signaler(void* ph);

extern "C"  hss_proc_t hss_create_completion_port_closure(HANDLE hport=0,ULONG_PTR key=0);

extern "C"  hss_proc_t hss_get_io_signaler(void* ph);

extern "C"  HSS_INFO* hss_wait_io_completion(void* ph,DWORD* pnbt=0,ULONG_PTR* pkey=0,int to=INFINITE);

extern "C"  HANDLE hss_attach_socket_to_completion_port(void* hport,SOCKET s=INVALID_SOCKET,UINT_PTR key=0);

extern "C"  DWORD hss_wait_group_signaled(int to,bool wall,int num,void* p0,...);
extern "C"  DWORD hss_wait_group_signaled_0(int to,bool wall,int num,void** pp);

extern "C"  HRESULT hss_create_port_ns_aliasW(wchar_t* existing_port,wchar_t* alias);
extern "C"  HRESULT hss_create_port_ns_alias(char* existing_port,char* alias);

extern "C"  HRESULT hss_add_port_ns_alias(void* p,char* alias);
extern "C"  HRESULT hss_remove_port_ns_alias(void* p,char* alias);
extern "C"  HRESULT hss_attach_ns_alias_to_port(void* p,char* alias,char* remoteport);

extern "C"  HRESULT hss_add_port_ns_aliasW(void* p,wchar_t* alias);
extern "C"  HRESULT hss_remove_port_ns_aliasW(void* p,wchar_t* alias);
extern "C"  HRESULT hss_attach_ns_alias_to_portW(void* p,wchar_t* alias,wchar_t* remoteport);

extern "C" HRESULT __stdcall  hss_websocket_services_start(char* lpszCmdLine);
extern "C" HRESULT __stdcall  hss_websocket_services_startW(wchar_t* lpszCmdLine);

extern "C"  SOCKET hss_duplicate_socket(SOCKET s);

extern "C" HRESULT __stdcall  hss_websocket_pipes_serviceW(wchar_t* argzz);
extern "C" HRESULT __stdcall  hss_websocket_pipes_service(char* argzz);
extern "C" long   hss_heap_count();
extern "C"  void* hss_tpl_reset(void* pt=0,int flag=0,void* ptr=0,int cb=-1,void *hsinfo=0,void* pstub=0);

extern "C" int hss_recv(void* phs,void* buf,int cb,int flags=0);
extern "C" int hss_send(void* phs,void* buf,int cb,int flags=0);

extern "C"  ULONG hss_ns_ipc_chain(void* phsd);

extern "C"  PROC hss_websocket_asyn_connect_to_port(char* host_port,char* ns_path,char* argszz,char* portname,void* pcallback,void * hcontext=0,int hsf_flags=HSF_SRD_AUTO);
extern "C"  SOCKET hss_socket_ping_port(char* port,SOCKET s);

extern "C" void* hss_http_sendfileW(void* hsinfo,const wchar_t* filename);
extern "C" void* hss_http_sendfileA(void* hsinfo,const char* filename);
extern "C" char* hss_wbs_send_recv(void* hsinfo,char* sendbuf,int cbsend=-1);
extern "C" int hss_wbs_send_recv_ex(void* hsinfo,char* sendbuf,int cbsend=-1,char** recvbuf=0);

extern "C" char* hss_get_args(char* pzz,char* pname,char* pdef=0);
extern "C" wchar_t* hss_get_argsW(wchar_t* pzz,wchar_t* pname,wchar_t* pdef=0);
extern "C" void* hss_ion_channels(wchar_t* nsport);

extern "C" int hss_wbs_version(void* hsinfo);

extern "C" HRESULT hss_wbs_frame_from_to(void* hs_from,void* hs_to,int size_buf=(8*1024));
extern "C" HRESULT hss_read_wbs_frame(void* hsinfo,int fnext=1,void* buf=0,int* pcb=0,wbs_frame_base_t* pwbs_frame=0);
extern "C" HRESULT hss_write_wbs_frame(void* hsinfo,void* buf,int* pcb,int flags=WSF_FIN|WSF_TXT,int mask=0);
extern "C" HRESULT hss_marshal_data(int pid,HSS_INFO* povl,void* pout,int *pcb=0);
extern "C" HRESULT hss_unmarshal_data(void* p,HSS_INFO** ppovl);
extern "C" HRESULT hss_to_clone_process(HSS_INFO* povl);
extern "C" HRESULT hss_to_clone_process_ex(HSS_INFO* povl,void* callback=0,void *context=0);
extern "C" HRESULT hss_from_parent_process(HSS_INFO** ppovl=0);
extern "C" HRESULT hss_phoenix(ULONG tio);


extern "C" SOCKET  hss_tcp_connect(const char* hostport,HSS_INFO** ppovl=0);
extern "C" SOCKET  hss_tcp_connectW(const wchar_t* hostport,HSS_INFO** ppovl=0);
