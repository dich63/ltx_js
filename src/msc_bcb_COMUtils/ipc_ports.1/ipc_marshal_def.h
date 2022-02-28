#pragma once

#include <Winsock2.h>
#include <windows.h>


#pragma pack(push)
#pragma pack(1)

#define  KB 1024
#define  MB (KB*KB)
#define  GB (KB*MB)

struct region_op_t
{
};

#define IPCM_COPY 0x10000000
#define IPCM_REGIONMAP 0x01000000
#define IPCM_REGIONQUERY 0x02000000
#define IPCM_ERROR 0x80000000
#define IPCM_HANDLE 1
#define IPCM_SOCKET 2
#define IPCM_HGLOBAL 4
#define IPCM_KO (IPCM_HANDLE|IPCM_SOCKET|IPCM_HGLOBAL)
#define IPCM_COM 8
#define IPCM_COMSTUB (IPCM_COM|IPCM_HGLOBAL)

//
typedef IStream i_kox_stream;
//typedef ISequentialStream i_kox_stream;
typedef IUnknown i_kox_obj;


struct ipc_region_base_t
{
	typedef  long long int64_t;
	typedef unsigned long long uint64_t;
	typedef unsigned long  uint32_t;
	typedef long  int32_t;
	typedef unsigned short  uint16_t;
	typedef signed short  int16_t;
	typedef unsigned char  uint8_t;
	typedef unsigned char  byte_t;
	typedef  char  int8_t;

	typedef GUID uuid_t;
	union{
		struct {	
			union{
				void* ptr_v;
				char* ptr;
				byte_t* ptru;
				int64_t state;
				int64_t offset;
				wchar_t* wptr;
				HGLOBAL* pcomstub;
				IUnknown** ppunknown;
				HANDLE* phandle;
				SOCKET* psocket;
				HRESULT hr;
			};
			int64_t sizeb; 	  
			union{
				uint64_t attribute;
				struct{
					uint32_t attr;
					uint32_t attr_h;
				};
			};


			struct{
				HRESULT hr_marshal;
				HRESULT hr_unmarshal;
			};
		};
		struct header_t
		{
			uuid_t uuid;
			int32_t marshal_pid;
			int32_t unmarshal_pid;
			int32_t region_count;
			uint32_t resereved1;
			union{
				HANDLE  unmarshal_handle; 
				uint64_t resereved2;
			};
		};
	};

};
typedef ipc_region_base_t* ipc_region_ptr_t;
#pragma pack(pop)