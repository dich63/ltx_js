#pragma once 

#include <Winsock2.h>
#include <stdlib.h>
#include "hss_types.h"

#pragma comment(lib ,"Ws2_32.lib")
/*
#define HSS_ENV_PROCESS_PORT_BY_FACTORY      "hssh.__PROCESS_PORT_BY_FACTORY__"
#define HSS_ENV_PROCESS_PORT_BY_FACTORYW      L"hssh.__PROCESS_PORT_BY_FACTORY__"
#define REF_ID_DEMAND "__socket_ref_demand__"
#define HSS_REF_ID_DEMAND REF_ID_DEMAND

#define HS_PROCESS_PORT_BY_FACTORY     ((char*)(777))    
#define HS_PROCESS_PORT_BY_FACTORY_W     ((wchar_t*)(777))

#define HSD_SAME_PTR     ((void*)(-1))    
#define HSD_IO_PORT_PTR  ((void*)(0x512))     
#define HSD_STATE (-1)
#define HSD_REPLY_STATE (-2)
#define HSD_SIGNALER 0x100
#define HSD_USER_DATA 0x800
#define HSD_SOCKET 31
#define HSD_URL 0
#define HSD_HOST 1
#define HSD_PARAMS 2
#define HSD_REPLY 3
#define HSD_REQUEST 4
#define HSD_ERROR 5
#define HSD_SOCKET_REF 6
#define HSD_LOCAL_INFO 7
#define HSD_REMOTE_INFO 8
#define HSD_PROTOCOL 9
#define HSD_REQUEST_HTTP 10
#define HSD_HTTP_URL_BASE 11
#define HSD_REQUEST_POS 12
#define HSD_TRANSMIT_PACKET_ADD 14
#define HSD_CONN_PORT 15
#define HSD_UUID 16
#define HSD_HTTP_FILE 17
#define HSD_HTTP_VIRTUAL_ROOT 18
#define HSD_HTTP_VIRTUAL_FILE 19
#define HSD_PROCESS_MMAP 20
#define HSD_WS_VERSION 21
#define HSD_WS_EXTENSION 22
#define HSD_BODY_LENGTH 23
#define HSD_PROCESS_INFO 24

#define HSF_GET 0
#define HSF_SET 1
#define HSF_DETACH 2
#define HSF_REPLY 4
#define HSF_DUP 8
#define HSF_CLOSED 16
#define HSF_FAST_CLOSED 32
#define HSF_SRD 0x1 
#define HSF_SRD_AUTO (0x2|HSF_SRD)
#define HSF_SRD_VIRTUAL 0x100
#define HSF_SRD_CREATE_PATH (0x1<<16)
#define HSF_REPLY_SRD MAKELONG(HSF_REPLY,HSF_SRD)
#define HSF_REPLY_SRD_AUTO MAKELONG(HSF_REPLY,HSF_SRD_AUTO)

#define HSF_REPLAY_SRD   HSF_REPLY_SRD
#define HSF_REPLAY_SRD_AUTO HSF_REPLY_SRD_AUTO


#define HTPL_CLEAR 0
#define HTPL_FILE 1
#define HTPL_MEMORY 2
#define HTPL_REGION 4
#define HTPL_EOF 32
#define HTPL_POOL 64
#define HTPL_SYNCHRO 0x200
#define HTPL_SEND (HTPL_POOL|HTPL_SYNCHRO)
#define HTPL_BUFFER_AMOUNT 128



#define HSS_BUF_COUNT 32

*/

#define WSF_FIN  0x80
#define WSF_TXT  0x01
#define WSF_BIN  0x02
#define WSF_CLO  0x08
#define WSF_CLS  WSF_CLO
#define WSF_PIN  0x09
#define WSF_PON  0x0A
#define WSF_CNT  0x00
#define WSF_MASK 0x8000

#define WSF_BEG  0x00
#define WSF_END  0x01
#define ERR_EOM ((FACILITY_INTERNET<<16)|ERROR_HANDLE_EOF) 
#define ERR_DCN  HRESULT_FROM_WIN32(ERROR_GRACEFUL_DISCONNECT)


#pragma pack(push)
#pragma pack(1)

struct HSS_INFO:OVERLAPPED
{

	HANDLE hkObjects[2];
	int hkcount; 
	HRESULT   error_code;
    ULONG_PTR user_data;
	SOCKET s;
	ULONG_PTR free_flags;
	ULONG_PTR reply_state;
	//DWORD pid_dest;
	
	WSABUF buffers[HSS_BUF_COUNT];

};

struct wbs_frame_base_t
{

	typedef unsigned char uint8_t;
	typedef unsigned int uint32_t;
	typedef unsigned short uint16_t;
	typedef unsigned long long uint64_t;
	typedef  long long int64_t;

	struct wbs_frame_t_0
	{
		union{
			struct{
				uint8_t opcode:4;
				uint8_t rsv3:1;
				uint8_t rsv2:1;
				uint8_t rsv1:1;
				uint8_t fin:1;
			};
			struct{ uint8_t fields;};
		};
	};
	struct wbs_frame_t_1
	{
		union{
			struct{
				uint8_t len:7;
				uint8_t mask:1;
			};
			struct{ uint8_t fields;};
		};

	};

	struct wbs_frame_t_01
	{
		wbs_frame_t_0 f0;
		wbs_frame_t_1 f1;
	};

	union
	{
		wbs_frame_t_01 f01;
		uint16_t fields;

	};

	uint8_t  inner_buf[12];
	uint8_t cbsz;
	uint8_t opcode;
    uint8_t state;



	uint64_t frame_count;
	union 
	{
		struct{ uint8_t maskb[4];};
		struct{  uint32_t mask;};
	};
	union 
	{
		struct{ uint8_t lenb[8];};
		struct{ uint64_t len;};
	};

	union 
	{
		struct{ uint8_t posb[8];};
		struct{ uint64_t pos;};
	};
};



#pragma pack(pop)
typedef int ( __stdcall * hss_proc_t)(HSS_INFO* ,DWORD);

//typedef void ( *hss_abort_proc_t)();
typedef  PROC hss_abort_proc_t;


inline void* _hssh_set_dll_proc(HMODULE h,void** fp,char* name)
{
	return (h)?(*fp=(void*)GetProcAddress(h,name)):NULL;
}

#define SET_PROC_ADDRESS_prfx(h,a,prfx) _hssh_set_dll_proc(h,(void**)&prfx##a,#a)
#define SET_PROC_ADDRESS(h,a) _hssh_set_dll_proc(h,(void**)&a,#a)
#define SET_PROC_ADDRESS_NS(h,ns,a) _hssh_set_dll_proc(h,(void**)&ns##::##a,#a)
#define SET_PROC_ADDRESS_aprfx(h,a,prfx) _hssh_set_dll_proc(h,(void**)&a,prfx #a)
#define SET_PROC_ADDRESS_hss(h,a) _hssh_set_dll_proc(h,(void**)&a,"hss_" #a)





struct hssh_t
{





 HMODULE hlib;
 bool fok;
 HRESULT hr;

HRESULT (*error)();
long (*addref)(void *p);
long (*release)(void *p);
char*   (*reset_SOCKET_DATA)(void *psd,int n,int flags,void* pv,int cb);
DWORD (*wait_signaled)(void* ph,int to);
void * (*create_socket_port)(char*  portname,ULONG reply_mode,void* pcallback,void * hcontext);
void * (*create_socket_portW)(wchar_t*  wportname,ULONG reply_mode,void* pcallback,void * hcontext);
SOCKET (*websocket_connect_ex)(char* host_port,char* ns_path,char* argszz,char** pp_socket_ref);
HSS_INFO*  (*websocket_connect2)(char* host_port,char* ns_path,char* argszz);
HSS_INFO*  (*websocket_connect2W)(wchar_t* host_port,wchar_t* ns_path,wchar_t* argszz);
SOCKET (*websocket_connect_exW)(wchar_t* host_port,wchar_t* ns_path,wchar_t* argszz,wchar_t** pp_socket_ref);
void* (*create_stdcall_closure)(void* proc,void* pcontext);
HSS_INFO* (*wait_io_completion)(void* ph,DWORD* pnbt,ULONG_PTR* pkey,int timeout);
SOCKET (*accept)(void* p, void** ppsd);
void* (*accept_descriptor)(void* p);
HRESULT (*websocket_services_start)(char* );
HRESULT (*websocket_services_startW)(wchar_t* );
long   (*heap_count)();
char* (*info)(void * h,int n);
void** (*info_ll)(void * h);
void* (*tpl_reset)(void* pt,int flag,void* ptr,int cb,void *hsinfo,void* pstub);

ULONG (*ns_ipc_chain)(void* phsd);
ULONG (*HTTP_request_headers_chain)(void* phsd);
ULONG (*websocket_handshake_chain)(void* phsd);

ULONG (*websocket_handshake_reply)(void* phsd);

SOCKET (*reset_socket)(void *psd,int flags,SOCKET snew);
long (*wait_port_creation)(char* portURL,long time_out,bool falertable);
long (*wait_port_creationW)(wchar_t* portURL,long time_out,bool falertable);
int (*wbs_send_recv_ex)(void* hsinfo,char* sendbuf,int cbsend,char** recvbuf);
char* (*wbs_send_recv)(void* hsinfo,char* sendbuf,int cbsend);
int (*send)(void* phs,void* buf,int cb,int flags);
int (*recv)(void* phs,void* buf,int cb,int flags);
int (*wbs_version)(void* hsinfo);
HRESULT (*read_wbs_frame)(void* hsinfo,int fnext,void* buf,int* pcb,wbs_frame_base_t* pwbs_frame);
HRESULT (*write_wbs_frame)(void* hsinfo,void* buf,int* pcb,int flags,int mask);
DWORD (*wait_group_signaled)(int to,bool wall,int num,void* p0,...);
DWORD (*wait_group_signaled_0)(int to,bool wall,int num,void** pp);

HRESULT (*to_clone_process)(HSS_INFO* povl);
HRESULT (*to_clone_process_ex)(HSS_INFO* povl,void* callback,void* context);
HRESULT (*from_parent_process)(HSS_INFO** ppovl);
void* (*ion_channels)(wchar_t* nsport);
SOCKET (*tcp_connect)(const char* hostport,HSS_INFO** ppovl);
SOCKET (*tcp_connectW)(const wchar_t* hostport,HSS_INFO** ppovl);



struct hssh_ptr_t
{
	hssh_t* plib;
	void* ptr;


};

inline bool is_clone_process()
{
	return S_OK==from_parent_process(NULL);
}

hssh_t(HMODULE _hlib):fok(false),hlib(0)
{
	//hlib=_hlib;
	
	if(GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,LPCTSTR(_hlib),&hlib))
	   init();
}
hssh_t(wchar_t* libname=0):fok(false),hlib(0)
 {
 if(libname&&(*libname)) hlib=LoadLibraryW(libname);
 else
 {
   hlib=LoadLibraryW(L"hs_socket.dll");
   if(!hlib)
   {
	   //wchar_t* p=_wgetenv(L"hssh.lib");
	    wchar_t p[1024];
	    if(GetEnvironmentVariableW(L"hssh.lib",p,1024))
           hlib=LoadLibraryW(p);
   }
 }
 init();
}

inline bool init()
{
   fok=hlib;
   
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,error)); 
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,addref));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,release));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,reset_SOCKET_DATA));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,wait_signaled));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,create_socket_port));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,create_socket_portW));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,websocket_connect_ex));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,websocket_connect2));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,websocket_connect2W));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,websocket_connect_exW));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,create_stdcall_closure));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,wait_io_completion));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,accept_descriptor));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,accept));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,websocket_services_start));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,websocket_services_startW));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,heap_count));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,info_ll));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,info));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,tpl_reset));

   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,ns_ipc_chain));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,HTTP_request_headers_chain));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,websocket_handshake_chain));
   

   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,websocket_handshake_reply));

   
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,reset_socket));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,wait_port_creation));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,wait_port_creationW));

   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,wbs_send_recv));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,wbs_send_recv_ex));

   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,send));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,recv));

   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,wbs_version));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,read_wbs_frame));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,write_wbs_frame));

   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,wait_group_signaled));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,wait_group_signaled_0));

   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,to_clone_process));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,to_clone_process_ex));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,from_parent_process));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,ion_channels));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,tcp_connectW));
   fok=fok&&(SET_PROC_ADDRESS_hss(hlib,tcp_connect));
   
   
   
   
	   

   
   hr=(fok)?S_OK:HRESULT_FROM_WIN32(GetLastError());
     return fok; 
 }


inline HMODULE  lock_forever()   {
	HMODULE t=0;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,	LPCTSTR(hlib),&t);
	  return hlib;
  }

 ~hssh_t(){FreeLibrary(hlib);}

 inline operator bool()
 {
	 return fok;
 }

inline  SOCKET reset_handshake_socket(void* hovl,unsigned int flags=HSF_REPLAY_SRD_AUTO)
 {
         ULONG st=websocket_handshake_chain(hovl);
		//SOCKET s=(SOCKET)reset_SOCKET_DATA(hovl,HSD_SOCKET,flags,(void*)(-1),0);
		 SOCKET s=(SOCKET)reset_socket(hovl,flags,INVALID_SOCKET);
		return s;
 };

 bool asyn_websocket_services_start(char* cmd)
 {
	 struct _i_ccc
	 {
		    char* fn; 
			hssh_t hss;
           //HRESULT (*websocket_services_start)(char* fn);
		   _i_ccc(HMODULE hlib):hss(hlib){}
		   ~_i_ccc(){ free(fn);}
		   
	   static   void __stdcall s_proc(_i_ccc* p)
	   {
		   try
		   {
			   p->hss.websocket_services_start(p->fn);
		   }
		   catch (...){}
		     delete p;
  	   }
	 } ;

     _i_ccc* pcc=new _i_ccc(hlib);
      pcc->fn= (cmd)?strdup(cmd):0;
	  //pcc->websocket_services_start=websocket_services_start;
	 return QueueUserWorkItem((LPTHREAD_START_ROUTINE)&_i_ccc::s_proc,pcc,WT_EXECUTELONGFUNCTION);
 };

};

#include <vector> 

extern "C"  char *  hss_info(void * h,int n);
struct hs_socket_delay_loader
{

	hs_socket_delay_loader(const wchar_t* libpfn=L"hssh.lib")
	{
		std::vector<wchar_t> ve(GetEnvironmentVariableW(libpfn,0,0));      
		if(ve.size())
		{ 
			wchar_t* plib=&ve[0];
			if(GetEnvironmentVariableW(libpfn,plib,ve.size()))
			{

				HMODULE hl=LoadLibraryW(plib);
				if(hl) hss_info(0,0);
				FreeLibrary(hl);

			}
		}
	}

};
