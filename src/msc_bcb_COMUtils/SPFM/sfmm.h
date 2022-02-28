#pragma once
#include "gsmm.h"



#pragma pack(1)

struct SFM_ADDRESS
{
union
{
	struct{ ulong64 m_segoffset;};
	struct{ ulong32 m_segoffset_l;
	        ulong32 m_segoffset_h;
	      };

};



struct SEG_A
{
  SFM_ADDRESS& m_A;
  SEG_A(SFM_ADDRESS& A):m_A(A){};
 operator ulong32()
 {
	 return (m_A.m_segoffset_h)>>8;
 }
inline SFM_ADDRESS& operator =(ulong32 seg)
{
   ulong32 r=(seg<<8)|(m_A.m_segoffset_h&0x000000FF);
   m_A.m_segoffset_h=r;
  return m_A;
}
};

struct OFFSET_A
{
	SFM_ADDRESS& m_A;
inline	OFFSET_A(SFM_ADDRESS& A):m_A(A){};
inline	operator ulong64()
	{
		return m_A.m_segoffset&ulong64(0x000000FFFFFFFFFF);
	}
	inline SFM_ADDRESS& operator =(ulong64 offset)
	{
		ulong64 r=m_A.m_segoffset&ulong64(0xFFFFFF0000000000);
	    m_A.m_segoffset= r|(ulong64(0x000000FFFFFFFFFF)&offset);
		return m_A;
	}
};


inline SEG_A seg()
{
	return SEG_A(*this);
}
inline OFFSET_A offset()
{
	return OFFSET_A(*this);
}


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
	union
	{
	SFM_ADDRESS attribute_SEG_OFF;
      ulong64   attribute_SEG_OFF_64;
	};
	ulong32 link_count;
	ulong32 free_SEG;
    ulong32 prev_SEG;
	ulong32 next_SEG;
 };
struct{ char reserved[1];};
};

inline bool check(ulong32* pcrc32=NULL)
{
	ulong32 c=Crc32(((unsigned char*) this)+sizeof(crc32),sizeof(SFM_SEGMENT)-sizeof(crc32));
	bool f=(crc32==c);
	if(pcrc32) *pcrc32=c;
	return f;
};

};




template <short powN ,short powI=1 >
struct filemapbuffer
{
 struct segment
 {
	 void* ptr;
	 long64 offset;
  segment():ptr(NULL){};
	 //ulong64 size;
 };

 enum
 {
	 Ns=1<<powN,
	 mask=Ns-1
     

 };
  wild_ptr hm;
  segment pseg[Ns];
  long   cursor_index;
  long  sz_item;
  

filemapbuffer():cursor_index(0)
{
	sz_item=system_info::os_granularity()*(1<<powI);
}
~filemapbuffer()
{
	for(long n=0;n<Ns;n++)
     if(pseg[n].ptr) 
		 unmap_ptr(pseg[n].ptr);
}
 
inline void* _find_ptr(ulong64 sindex)
{
	for(int n=0;n<Ns;n++)
	{

	}
};
inline void* find_ptr(ulong64 index,ulong32& off)
{
  ulong64 sindex=(~ulong64(mask))&index;
  
  off=index&mask;
  return NULL;

}
void* get_ptr(long64 index)
{

}

  //
};

template <short powI>
struct filemapbuffer<0,powI>
{
	struct segment
	{
		char* ptr;
		long64 offset;
		segment():ptr(NULL),offset(0){};
		//ulong64 size;
	};

	enum
	{
		Ns=1,
		mask=Ns-1
	};
	wild_handle hm;
	segment seg;
	long  sz_item;
	long  imask;


	filemapbuffer(wild_handle _hm)
	{
		hm=_hm;
		sz_item=system_info::os_granularity()*(1<<powI);
		 imask=sz_item-1;
	}
	~filemapbuffer()
	{
		if(seg.ptr ) unmap_ptr(seg.ptr);
	}

inline void clear()
{
	if(seg.ptr ) 
	{
		unmap_ptr(seg.ptr);
		seg.ptr=0;
	};
}
	inline char* find_ptr(ulong64 index,ulong32& off)
	{
		ulong64 sindex=(~ulong64(imask))&index;
           off=index&imask;
		if((sindex==seg.offset)&&(off<sz_item)&&(seg.ptr))
			 return seg.ptr;
		      unmap_ptr(seg.ptr);
		return  seg.ptr=map_of_view(hm,seg.offset=sindex,sz_item);
	}
	void* get_ptr(long64 index)
	{
      char* p;
	  ulong32 off;
	  if(p=find_ptr(index,off))
              return p+off;
	  return NULL;
	}
};



template <class T,short powI=0,short powN=0>
struct filemapbufferT:filemapbuffer<powN,powI>
{
	filemapbufferT(wild_handle h):filemapbuffer<powN,powI>(h){};
inline T&  operator[](long64 index)
{
 return *((T*)get_ptr(index*sizeof(T)));
}
};





//template <long size >
struct SFM_SEGMENT_stack_base
{

  SFM_SEGMENT bottom[1];
  SFM_SEGMENT segments;
  union 
  {
	  struct {SFM_SEGMENT reserved[62];};
	  struct {SFM_SEGMENT reserved2[61];
	          char eof_label[2*sizeof(SFM_SEGMENT)];};
	  struct 
	  {
		  ulong32 crc32; 
		  union
		  {
		  struct{ 
		  ulong64 space_size;
		  ulong32 segment_max_count;
		  uuid_t static_iid;
		  uuid_t syncro_iid; };

		  struct { char crcdata[2*16+8+4];}; 

		  } ;
		  char objectref[1024];
		 
		  char comments[2*1024];

	  };
  };
};


struct SFM_SEGMENT_stack:SFM_SEGMENT_stack_base
{
 inline bool check(ulong32* pcrc32=NULL)
  {
 	  ulong32 c=Crc32(((unsigned char*) crcdata),sizeof(crcdata));
  	  bool f=(crc32==c);
	  if(pcrc32) *pcrc32=c;
	  return f;
  };

inline void init_new(uuid_t sync,ulong64 sp_size,ulong32 seg_count=(1<<24)-sizeof(SFM_SEGMENT_stack_base))
{
  memset(this,0,sizeof(this));
  space_size=sp_size;
  segment_max_count=seg_count;
  static_iid=get_global_space_id();
  syncro_iid=sync;
  check(&crc32);
  SFM_SEGMENT&  base=bottom[0];
  base.capacity=system_info::offset_align(space_size-seg_count*sizeof(SFM_SEGMENT));
  segments.offset=base.capacity;
  segments.capacity=seg_count*sizeof(SFM_SEGMENT);
}

};





typedef SFM_SEGMENT_stack* PSFM_SEGMENT_stack;

struct sfm_helper
{
  PSFM_SEGMENT_stack pss;

};