#pragma once
//include "file_map_utils.h"

#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>



namespace file_map_utils {

	typedef unsigned long long uint64_t;
	typedef  long long int64_t;
	typedef unsigned long  uint32_t;
	typedef   long int32_t;



	struct page_region_t
	{
		uint64_t begin;
		uint64_t end;

		inline bool into(page_region_t const& ps)
		{
			return (ps.end>=end)&&(ps.begin<=begin);
		}
		inline bool is_empty(){ return begin==end; }
	};


	struct system_info_t:SYSTEM_INFO
	{
		system_info_t(){ GetSystemInfo(this);}
	};


	struct region_aligner_t
	{
		union{
			struct{uint64_t  ialignmask;};
			struct{
				uint32_t  ialignmask_l;
				uint32_t  ialignmask_r;
			};
		};
		uint64_t imask,sz_item;

		region_aligner_t()
		{
			system_info_t si;
			sz_item=si.dwAllocationGranularity;
			imask=sz_item-1;
			ialignmask=~imask;
		}

		inline uint64_t offset_align_b(uint64_t index)
		{
			return  ialignmask&index;
		}
		inline uint64_t offset_align_e(uint64_t index)
		{
			return  ialignmask&(index+imask);
		}

		inline  uint64_t size_align(uint64_t size)
		{
			return ialignmask&(size+imask);
		}

	};






	




} // namespace file_map_utils END
