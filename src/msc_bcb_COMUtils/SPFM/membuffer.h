#pragma once
#include "gsmm.h"

#pragma pack(push)
#pragma pack(1)
struct page_region
{
	long64 beg_offset;
	long64 end_offset;

	inline bool into(page_region const& ps)
	{
		return (ps.end_offset>=end_offset)&&(ps.beg_offset<=beg_offset);
	}
};

struct map_segment
{

	union
	{
		struct{
			page_region ps;
			char* ptr;
			long32 flag;
		};
		struct{ ulong64 reserved[4];};
	};

	map_segment():ptr(NULL)
	{
		ps.beg_offset=-1;
		ps.end_offset=-1;
		flag=0;
	};

	inline void unmap()
	{
		if(ptr) {
			unmap_ptr(ptr);
			ptr=0;
			
			;
		}
	};
	//segment* prev;
	//segment* next;
	//ulong64 size;
};

struct map_segment_holder
{
      map_segment ms;
	  ~map_segment_holder()
	  {
		  ms.unmap();
	  };
void operator =(map_segment s)
{
	if(ms.ptr!=s.ptr) ms.unmap();
    ms=s;
	
};
};

template <short powN,class Derived>
struct file_map_buffer_base
{

	typedef map_segment segment;

	enum
	{
		Ns=1<<powN,
		mask=Ns-1

	};
	wild_handle hm;
	segment pseg[Ns];
	ulong32   cursor_index;
	ulong32  sz_item;
	ulong32  imask; 
	union{
		struct{ulong64  ialignmask;};
		struct{
			ulong32  ialignmask_l;
			ulong32  ialignmask_r;
		};
	};



	file_map_buffer_base(wild_handle h):cursor_index(0)
	{
		hm=h;
		//
		sz_item=system_info::os_granularity();
		//sz_item=system_info::page_size();
		
		imask=sz_item-1;
		ialignmask=~ulong64(imask);
	}
	~file_map_buffer_base()
	{
		for(long n=0;n<Ns;n++)
			if(pseg[n].ptr) 
				unmap_ptr(pseg[n].ptr);
	}

	inline ulong64 offset_align(ulong64 index)
	{
		return  ialignmask&index;
	}
	inline  ulong32 size_align(ulong32 size)
	{
		return ialignmask_l&(size+imask);
	}

	inline char* find_ptr(ulong64 index,ulong32 siz,ulong32& off)
	{
		off=imask&index;
		page_region ps={offset_align(index),offset_align(index+siz+imask)};
		return   static_cast<Derived*>(this)->remap_ptr(ps);

	}
	inline char* get_ptr(ulong64 index,ulong32 siz)
	{
		ulong32 off;
		char *p=static_cast<Derived*>(this)->find_ptr(index,siz,off);
		if(p) p+=off;
		return p;
	}

};



struct file_map_buffer_fixed_0:file_map_buffer_base<1,file_map_buffer_fixed_0 >
{


	file_map_buffer_fixed_0():file_map_buffer_base<1,file_map_buffer_fixed_0>(0){};

	inline wild_ptr init(wild_handle h,ulong64 offset,ulong32 size)
	{
		hm=h;
        page_region ps={offset_align(offset),offset_align(offset+size+imask)}; 
		ulong32 siz=ps.end_offset -ps.beg_offset;
		segment& s0=pseg[0];
		char*ptr =map_of_view(hm,ps.beg_offset,siz);
		if(ptr)
		{
			s0.ps=ps;
			s0.ptr=ptr;
		}
		return get_ptr(offset,size);
	}


	inline char* remap_ptr(page_region& ps)
	{
	   	
		segment& s0=pseg[0];
		if(ps.into(s0.ps))
			return s0.ptr;
		
        segment& s=pseg[1];
	//
		if(ps.into(s.ps))			return s.ptr;
		if(s.ptr) unmap_ptr(s.ptr);
		ulong32 siz=ps.end_offset -ps.beg_offset;
		char*ptr =map_of_view(hm,ps.beg_offset,siz);
		if(ptr)
		{
			s.ps=ps;
			s.ptr=ptr;
		}
		else {s.ps.end_offset=-1;s.ptr=NULL;}
		return ptr;
	}

inline void decommit()
{
	pseg[1].unmap();
	pseg[1].ps.beg_offset=-1;
	pseg[1].ps.end_offset=-1;
}
    
inline segment	detach_current_region()
   {
       segment tmp=pseg[1];
	   pseg[1]=segment();
	   return tmp;
   }
};

template<class Header,class Item=char>
   struct item_frame_buf:file_map_buffer_fixed_0
  {
	      Header* pheader;

		  item_frame_buf():file_map_buffer_fixed_0(),pheader(0){};

    inline wild_ptr init(wild_handle h,ulong64 offset=0)
	{

		pheader=file_map_buffer_fixed_0::init(h,offset,sizeof(Header));
			return pheader;
	}

           item_frame_buf(wild_handle h,ulong64 offset=0)
		   {
                 init(h,offset);
		   }
  inline operator Header&()
  {
	  return *pheader;
  }
  /*
  inline  Item* operator[](long64 ind)
  {
	    long64 offset=pheader->offset;
        long64 isizeb=pheader->item_size_b;

        return (Item*)get_ptr(ind*isizeb+offset,isizeb);
  }
*/
  };


template <short powN >
struct file_map_buffer:file_map_buffer_base<powN,file_map_buffer<powN> >
{

	


};




template <>
struct file_map_buffer<0>:file_map_buffer_base<0,file_map_buffer<0> >
{

	file_map_buffer(wild_handle h):file_map_buffer_base(h){};
	~file_map_buffer()
	{

	}

	inline char* remap_ptr(page_region& ps)
	{
		segment& s=pseg[0];
	//			if(0) 
			if(ps.into(s.ps))
			return s.ptr;
		if(s.ptr) unmap_ptr(s.ptr);
		ulong32 siz=ps.end_offset -ps.beg_offset;
		char*ptr =map_of_view(hm,ps.beg_offset,siz);

		if(ptr)
		{
			s.ps=ps;
			s.ptr=ptr;
		}
		else {s.ps.end_offset=-1;s.ptr=NULL;}

		return ptr;
	}
};

template <>
struct file_map_buffer<1>:file_map_buffer_base<1,file_map_buffer<1> >
{

	file_map_buffer(wild_handle h):file_map_buffer_base(h){};
	~file_map_buffer()
	{

	}

	inline char* cursor_ptr(page_region& ps)
	{
		segment& s=pseg[cursor_index];
		if(s.ptr) unmap_ptr(s.ptr);
		ulong32 siz=ps.end_offset -ps.beg_offset;
		char*ptr =map_of_view(hm,ps.beg_offset,siz);

		if(ptr)
		{
			s.ps=ps;
			s.ptr=ptr;
		}
		else {s.ps.end_offset=-1;s.ptr=NULL;}

		return ptr;

	}

	inline char* cursor_ptr_check(page_region& ps)
	{
		segment& s=pseg[cursor_index];
		if(ps.into(s.ps))
			return s.ptr;
		else if(s.ps.into(ps))
			return  cursor_ptr(ps);

		return NULL;
	}


	inline char* remap_ptr(page_region& ps)
	{


		char *p;
		if(p=cursor_ptr_check(ps)) return p;
		else 
		{
			cursor_index= (cursor_index+1)&1;
			if(p=cursor_ptr_check(ps)) return p;
		}

		return cursor_ptr(ps);

	}
};



template <class T,short powN=0>
struct file_map_bufferT:file_map_buffer<powN>
{
	file_map_bufferT(wild_handle h):file_map_buffer<powN>(h){};
	inline T&  operator[](long64 ind)
	{
		return *((T*)get_ptr(ind*sizeof(T),sizeof(T)));
	}
};




#pragma pop(push)

