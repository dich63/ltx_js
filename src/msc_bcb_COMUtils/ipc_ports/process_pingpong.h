#pragma once
#include "singleton_utils.h"


#pragma pack(push)
#pragma pack(1)

struct  HANDLE64_t
{
	union{
		struct { HANDLE h;}
      struct _rsvd_t {char c[8];} rsvd;   
	}
   HANDLE64_t()
   {
	   _rsvd_t t={0,0,0,0,0,0,0,0};
       rsvd=t;
   }
   operator HANDLE&(){return h;}
};
struct process_pingpong_t
{
   v_buf<wchar_t> key;
   shared_area_t* parea;
   processinfo_t processinfo;
   int mapsize;
    process_pingpong_t(wchar_t* _key,int _mapsize=0x10000-2*sizeof(HANDLE64_t)):parea(NULL),mapsize(_mapsize)
   {
	   key.cat(_key);

   }

};
#pragma pack(pop)