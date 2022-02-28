#pragma once
//#include "sm_region.h"
#include "ipc_utils_base.h"
#include "static_constructors_0.h"

namespace sm_region {


	typedef ULONGLONG offset_t;
	typedef  offset_t size_t;


	struct err_holder_t
	{
		HRESULT hr;
		err_holder_t(HRESULT e=0):hr(e)
		{

		}
		~err_holder_t(){
			SetLastError(hr);
		}
		template <class T>
		T& operator()(T t,HRESULT h=GetLastError())
		{
			hr=h; 
			return t;
		}


	};


	struct system_info_t:SYSTEM_INFO
	{


		inline static system_info_t& get_instance()
		{
			return class_initializer_T<system_info_t>().get();
		}

		system_info_t()
		{
			GetSystemInfo(this);
		};

	};

	struct memory_align_info_t
	{
		offset_t offset_mask,offset_co_mask;
		offset_t page_size,size_mask,size_co_mask;
		memory_align_info_t()
		{
			system_info_t& si=system_info_t::get_instance();
			offset_mask=ULONGLONG(si.dwAllocationGranularity-1);
			offset_co_mask=~(offset_mask);
			page_size=si.dwPageSize;
			size_mask=page_size-1;
			size_co_mask=~size_mask;		  

		}

		inline static  memory_align_info_t& get_instance()
		{
			return class_initializer_T<memory_align_info_t>().get();
		}

	};

	struct sm_region_base_t
	{
		ULARGE_INTEGER begin;
		ULARGE_INTEGER end;
		bool in_region(sm_region_base_t& r)
		{
			return (r.begin.QuadPart <= begin.QuadPart)&&(end.QuadPart <= r.end.QuadPart);
		}
		offset_t offset(sm_region_base_t& r)
		{
			return begin.QuadPart-r.begin.QuadPart;
		}

		inline 	size_t size(){
			return end.QuadPart-begin.QuadPart;  
		}

	};


	struct sm_region_t:sm_region_base_t
	{

		sm_region_t():sm_region_base_t(){};
		sm_region_t(ULONGLONG size,ULONGLONG offset=0){
			begin.QuadPart=offset;
			end.QuadPart=offset+size;

		};


	};

	inline  ULARGE_INTEGER size_align(ULARGE_INTEGER s)
	{
		memory_align_info_t& ma=memory_align_info_t::get_instance();		  
		s.QuadPart=(s.QuadPart+ma.size_mask)&ma.size_co_mask;
		return s;
	}

	inline  offset_t region_align(sm_region_base_t& r)
	{
		memory_align_info_t& ma=memory_align_info_t::get_instance();		  

		offset_t b=r.begin.QuadPart;
		offset_t e=r.end.QuadPart;
		offset_t offset=b&ma.offset_mask;

		r.begin.QuadPart=b&ma.offset_co_mask;
		r.end.QuadPart=(e+ma.size_mask)&ma.size_co_mask;

		return offset;
	};


	inline char* map_aligned_region(HANDLE hmap,const sm_region_base_t& r,DWORD DesiredAccess=FILE_MAP_ALL_ACCESS )
	{
		size_t size=r.end.QuadPart-r.begin.QuadPart;
		char*   p=(char*) ::MapViewOfFile(hmap,DesiredAccess,r.begin.HighPart,r.begin.LowPart,size);
		void *pp;
		//if(p) pp=VirtualAlloc(p,size,MEM_COMMIT ,PAGE_READWRITE); 
		return p;
	}



	inline char* map_region(HANDLE hmap,sm_region_base_t r,sm_region_base_t* pra=0,DWORD DesiredAccess=FILE_MAP_ALL_ACCESS )
	{
		offset_t offset=region_align(r);

		char* p;
		if((p=map_aligned_region(hmap,r,DesiredAccess)))
		{

			if(pra) *pra=r;
			return p+offset;
		}
		else return 0;
	};

	inline char* map_region(HANDLE hmap,ULONGLONG size,ULONGLONG offset=0,sm_region_base_t* pra=0,DWORD DesiredAccess=FILE_MAP_ALL_ACCESS )
	{
		sm_region_t r(size,offset);
		return map_region(hmap,(sm_region_base_t)r,pra,DesiredAccess);
	};

	inline BOOL unmap_ptr(const void* p,bool fflush=false)
	{
          if(fflush) 
			   FlushViewOfFile(p,0);

		return ::UnmapViewOfFile(p);
	};


	struct base_ptr_region_t
	{
		sm_region_base_t r;
		char* pbase;
		inline void _unmap(){

			BOOL f;
			HRESULT hr;
			if( pbase)
			{
				//f=VirtualFree(pbase,r.size(),MEM_DECOMMIT);
				//hr=GetLastError();
				f=::UnmapViewOfFile(ipc_utils::make_detach( pbase));
				//hr=GetLastError();
			}
		}

		inline void unmap(){
			_unmap();
			r=sm_region_base_t();
		}

	};

	template <int N=1,DWORD DesiredAccess=FILE_MAP_ALL_ACCESS> 
	struct region_cache_t
	{
	};

	template <DWORD __DesiredAccess>
	struct region_cache_t<1,__DesiredAccess>
	{

		base_ptr_region_t region;
		offset_t offset0;
		HANDLE hmap;
		DWORD DesiredAccess;
		region_cache_t(HANDLE _hmap=0,offset_t _offset0=0,DWORD _DesiredAccess=__DesiredAccess):
		hmap(_hmap),region(),offset0(_offset0),DesiredAccess(_DesiredAccess){}	
		~region_cache_t(){ 
			decommit();
		}

		inline void decommit(){
			region.unmap();			
		}

		void reset_access(DWORD _DesiredAccess=__DesiredAccess)
		{
			decommit();
			DesiredAccess=_DesiredAccess;
		}


		char* recommit(size_t size,offset_t offset){
			offset+=offset0;
			offset_t offs;
			sm_region_t r(size,offset);
			if( region.pbase&&r.in_region(region.r)){

				offs=r.offset(region.r);
				//offset&memory_align_info_t::get_instance().offset_mask;			
			}
			else{
				region.unmap();
				offs=region_align(r);
				region.pbase=map_aligned_region(hmap,r,DesiredAccess );
				region.r=r;


			}

			char* p=region.pbase;
			return (p)?p+offs:0;

		}

		template <class T>
		T* recommit_ptr(size_t size,offset_t offset){
			return (T*) recommit(size, offset);
		}

		template <class T>
		T* detach()
		{
			region.r=sm_region_base_t();
			return (T*)ipc_utils::make_detach(region.pbase);     
		}


	};


	template <DWORD __DesiredAccess>
	struct region_cache_t<2,__DesiredAccess>
	{

		base_ptr_region_t regions[2];
		int cursor;
		offset_t offset0;
		HANDLE hmap;
		DWORD DesiredAccess;
		region_cache_t(HANDLE _hmap=0,offset_t _offset0=0,DWORD _DesiredAccess=__DesiredAccess):
		hmap(_hmap),offset0(_offset0),DesiredAccess(_DesiredAccess),cursor(0){

			memset(regions,0,2*sizeof(base_ptr_region_t));
		}	
		~region_cache_t(){ 
			decommit();
		}

		void reset_access(DWORD _DesiredAccess=__DesiredAccess)
		{
			  decommit();
              DesiredAccess=_DesiredAccess;
		}

		inline void decommit(){
			regions[0].unmap();
			regions[1].unmap();
		}

		char* recommit(size_t size,offset_t offset){
			offset+=offset0;
			offset_t offs;
			sm_region_t r(size,offset);
			base_ptr_region_t* pcr=regions+cursor;

			if( pcr->pbase && r.in_region(pcr->r)){

				offs=r.offset(pcr->r);
				//offset&memory_align_info_t::get_instance().offset_mask;			
			}
			else {

				cursor++;
				cursor&=0x1;
				pcr=regions+cursor;
				if(pcr->pbase && r.in_region(pcr->r)){

					offs=r.offset(pcr->r);
					//offset&memory_align_info_t::get_instance().offset_mask;			
			 }
				else
			 {
				 pcr->unmap();
				 offs=region_align(r);
				 pcr->pbase=map_aligned_region(hmap,r,DesiredAccess );
				 pcr->r=r;


			 }




			}

			char* p=pcr->pbase;
			return (p)?p+offs:0;

		}

		template <class T>
		T* recommit_ptr(size_t size,offset_t offset){
			return (T*) recommit(size, offset);
		}



	};

	template <int CachePower>
	size_t mm_copy(HANDLE hd,HANDLE hs,size_t size,offset_t ofset_d=0,offset_t ofset_s=0)
	{
		err_holder_t err;

		const size_t mb=1<<CachePower;
		size_t n=size/mb;
		size_t rem=size-n*mb;

		region_cache_t<1,FILE_MAP_READ> rcs(hs,ofset_s);
		region_cache_t<1> rcd(hd,ofset_d);
		size_t c=0;
		char* ps,*pd;

		for(size_t k=0;k<n;k++,c+=mb)
		{
			if(!(pd=rcd.recommit(mb,c))) return err(c);
			if(!(ps=rcs.recommit(mb,c))) return err(c);
			memcpy(pd,ps,mb);		
		}
		if(rem)
		{
			if(!(pd=rcd.recommit(rem,c))) return err(c);
			if(!(ps=rcs.recommit(rem,c))) return err(c);
			memcpy(pd,ps,rem);
			c+=rem;
		}

		return c;	
	}


	template <int CachePower>
	size_t mp_set(HANDLE hd,const void *ptr,size_t el_size,offset_t ofset_d=0,size_t count=1,size_t step=1)
	{
		err_holder_t err;
		if(!ptr) return err(0,E_POINTER);

		const size_t mb=1<<CachePower;

		if((!el_size)||(el_size>mb)||(mb%el_size)) return err(0,CERTSRV_E_ALIGNMENT_FAULT);

        size_t size=el_size*count;
		size_t n=size/mb;
		size_t rem=size-n*mb;

		region_cache_t<1> rcd(hd,ofset_d);
		size_t c=0;
		char* ps,*pd;

		step*=el_size;

		
			ps=(char*)ptr;
			for(size_t k=0;k<n;k++,c+=mb)
			{
				if(!(pd=rcd.recommit(mb,c))) return err(c);		
				//memcpy(pd,ps,mb);
				for(char* p=pd;p<pd+mb;p+=step)
				{
					memcpy(p,ps,el_size);
					//for(int i=0;i<el_size;i++) p[i]=ps[i];
				}
				
			}
			if(rem)
			{
				if(!(pd=rcd.recommit(rem,c))) return err(c);		
				//memcpy(pd,ps,rem);
				for(char* p=pd;p<pd+rem;p+=step)
				{
					memcpy(p,ps,el_size);
					//for(int i=0;i<el_size;i++) p[i]=ps[i];
				}
				c+=rem;
			}
		
        return c; 
		
	}

	template <int CachePower>
	size_t mp_copy(HANDLE hd,const void *ptr,size_t size,offset_t ofset_d=0,size_t count=1)
	{
		err_holder_t err;
		if(!ptr) return err(0,E_POINTER);

		const size_t mb=1<<CachePower;
		size_t n=size/mb;
		size_t rem=size-n*mb;

		region_cache_t<1> rcd(hd,ofset_d);
		size_t c=0;
		char* ps,*pd;

		for(int nc=0;nc<count;nc++)
		{
			ps=(char*)ptr;
			for(size_t k=0;k<n;k++,c+=mb)
			{
				if(!(pd=rcd.recommit(mb,c))) return err(c);		
				memcpy(pd,ps,mb);
				ps+=mb;
			}
			if(rem)
			{
				if(!(pd=rcd.recommit(rem,c))) return err(c);		
				memcpy(pd,ps,rem);
				c+=rem;
			}
		}

		return c;	
	}

	template <int CachePower>
	size_t pm_copy(const void *ptr,HANDLE hs,size_t size,offset_t ofset_s=0)
	{
		err_holder_t err;
		if(!ptr) return err(0,E_POINTER);

		const size_t mb=1<<CachePower;
		size_t n=size/mb;
		size_t rem=size-n*mb;

		region_cache_t<1,FILE_MAP_READ> rcs(hs,ofset_s);
		size_t c=0;
		char* pd=(char*)ptr,*ps;

		for(size_t k=0;k<n;k++,c+=mb)
		{
			if(!(ps=rcs.recommit(mb,c))) return err(c);		
			memcpy(pd+c,ps,mb);				
		}
		if(rem)
		{
			if(!(ps=rcs.recommit(rem,c))) return err(c);		
			memcpy(pd+c,ps,rem);				
			c+=rem;
		}

		return c;	
	}



}; //namespace sm_region END