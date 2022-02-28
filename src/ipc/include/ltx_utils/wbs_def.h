#pragma once
#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>




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



#pragma pack(push)
#pragma pack(1)

#pragma pack(pop)
