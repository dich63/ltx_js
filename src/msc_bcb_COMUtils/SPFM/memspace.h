#pragma once

#pragma pack(push)
#pragma pack(1)

#include "gsmm.h"

namespace   sparse_memory_heap
{

	enum ssm:ulong32
	{
          heap=1<<0,  
          over=1<<1,
	      code=1<<2,
          attr=1<<3,
		  root=1<<4,
		  hang=1<<5,
		  dead=1<<6,
		  fctr=1<<7,
		  syss=1<<8
	};


	struct SFM_SEGMENT
	{
		union
		{
			struct
			{
				ulong32 crc32;  
				ulong32 flags;
				ulong32 state;
				ulong32 link_SEG;
				union	{
					page_segment segment;
					struct{
						ulong64 offset;
						ulong64 size;
					};
				};
				ulong64 capacity;
				//---------------------------
					
				ulong64   attribute_SEG_OFF;
	
				ulong32 link_count;
				ulong32 free_SEG;
				ulong32 prev_SEG;
				ulong32 next_SEG;
			};
			struct{ char reserved[4];};
		};

		inline ulong32 set_crc()
		{
			 ulong32 lcrc32;
            check(&lcrc32);
           return InterlockedExchange((volatile LONG* )&crc32,lcrc32);
		}
		inline bool check(ulong32* pcrc32=NULL)
		{
			ulong32 c=Crc32(((unsigned char*) this)+sizeof(crc32),sizeof(SFM_SEGMENT)-sizeof(crc32));
			bool f=(crc32==c);
			if(pcrc32) *pcrc32=c;
			return f;
		};

	};

typedef SFM_SEGMENT* PSFM_SEGMENT;

struct attrib_base
{
	size_t cb; 
};
typedef attrib_base* pattrib_base;
//
enum ss_mode
{
   map_exist=1,
   create_new=2,
   open_exist=4
};


struct ISparseSpace:IUnknown
{
	 //virtual long __stdcall AddRef()=0;
	 //virtual long __stdcall Release()=0;

     virtual long __stdcall get_handle(void** phmap)=0;
	 virtual long __stdcall get_segment(ulong32 hseg,void** ppseg)=0;
     virtual long __stdcall get_uuid(uuid_t* puuid)=0;
	 virtual long __stdcall get_segment_count(ulong32* pcount)=0;
     virtual long __stdcall create_segment(ulong32 hparent_seg,ulong32 flags,long64 capacity,long64 size,ulong32* psegment)=0;
     virtual long __stdcall select_segment(ulong32 hparent_seg,ulong32 flags,long64 offset,long64 capacity,long64 size,ulong32* psegment)=0;
     virtual long __stdcall free_segment(ulong32 hsegment,ulong32 flags)=0;
     virtual long __stdcall set_segment_name(ulong32 hseg,char* pname)=0;
	 virtual long __stdcall get_segment_name(ulong32 hseg,char* pname)=0;
	 virtual long __stdcall find_segment_by_name(char* ,ulong32* hseg)=0;
};


inline wild_ptr map_segment(ISparseSpace* pss,ulong32 hseg,ulong32 mode=FILE_MAP_READ|FILE_MAP_WRITE)
{
	HRESULT hr;
	wild_ptr hm(0);
	hr=pss->get_handle(hm.address());
	PSFM_SEGMENT ps;
	hr=pss->get_segment(hseg,(void**)&ps);
	char* p=map_of_view(hm,ps->offset,ps->size,mode);
	return p;
}


extern "C"
{
	 long __stdcall create_memory_space(wchar_t* pfnint,ISparseSpace** ppsm,ulong64* psize=NULL,int oflag=O_CREAT|O_RDWR,int shflag=SH_DENYNO,int pmode=S_IWRITE,int sparse_compress=1);

}

//errno_t
};
#pragma pop(push)



