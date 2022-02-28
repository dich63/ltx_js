#pragma once

#include <WinSock2.h>

#include "hss_types.h"

#pragma pack(push)
#pragma pack(1)

struct hss_port_types{
struct region_t   {
	union	   {
		struct{long long sign;};
		struct{long pid[2];};
		struct{SOCKET s;long pid_srv;};
		struct   {
			long sizeb;
			long offset;	   
		};

	};	
};

struct HANDSHAKE_DATA_t
{
	enum
	{
		REGION_COUNT =HSS_BUF_COUNT,
           BUFSIZE=2*4096
	};

	//typedef region_t region;
	/*
	inline   static  region& sign(char* s="HS>||<SH")
	{
		//static char s[]="HS>||<SH";
		return *((region*)s);
	}
	*/

	union{

		struct
		{
			long sizeb;
			HRESULT crc32;
			char prefix[8];
			long flags;
			long err_code;
		};
		struct
		{
			region_t crc32_size;
			region_t signature;
			region_t flags_err;
			region_t pids;
			region_t cmd;
			region_t fd_data;

			union{
				struct{
					region_t urn_data;
					region_t host_data;
					region_t params_data;
					region_t replay_data;
					region_t request_data;
					region_t error_data;
					region_t url_list;
				};
				struct{
					region_t regions[REGION_COUNT];
				};

			};
			SOCKET fd,fds;//INVALID_SOCKET 
			union{
				WSAPROTOCOL_INFOW wsadata;
				OVERLAPPED ovl;
				struct{
					HANDLE shared_handles[16];
				};
			};
			char buf[BUFSIZE/2];
		};
		struct{char _c[BUFSIZE];};
	};
};
};
#pragma pack(pop)