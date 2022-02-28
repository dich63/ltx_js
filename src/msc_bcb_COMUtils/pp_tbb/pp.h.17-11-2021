#pragma once

#include "utils/utils.h"
#include "tbb/cache_aligned_allocator.h"
#include "tbb/parallel_for.h"

///*
struct tbb_mem_pool_mng_cache_line_t {
	inline static void* malloc(size_t n_element, size_t element_size) {
		return  tbb::internal::NFS_Allocate(n_element, element_size, nullptr);
	}
	inline static void free(void* p) {
		tbb::internal::NFS_Free(p);
	}
};

//*/

//typedef utils::mem_pool_mng_def_t tbb_mem_pool_mng_cache_line_t;

typedef utils::mem_pool_t<std::mutex, tbb_mem_pool_mng_cache_line_t> tbb_mem_pool_t;
typedef tbb_mem_pool_t cache_line_mem_pool_t;

//

template <int I = 0>
struct partitioner_t {
    typedef tbb::affinity_partitioner partitioner;
};

template <>
struct partitioner_t<1> {
    typedef tbb::static_partitioner partitioner;
};



//template <class It,class  partitioner_t= tbb::affinity_partitioner >
template <class It, int IP=0  >
struct tbb_parfor_t {

    typedef typename partitioner_t<IP>::partitioner partitioner_t;

    template <class RangeFunc>
    inline void operator()(bool pp,It b, It e, RangeFunc rfun, size_t grain = 1) {
        //rfun(b, e);
        
        if (pp) {
            tbb::parallel_for(tbb::blocked_range<It>(b, e, grain),
                [&](const tbb::blocked_range<It>& r) {

                    It nb = r.begin(), ne = r.end();
                    rfun(nb, ne);

                } , partitioner);
        }
        else {
            rfun(b,e);
        }

    }

    template <class RangeFunc>
    inline void operator()( It b, It e, RangeFunc rfun, size_t grain = 1) {
        //rfun(b, e);

        
            tbb::parallel_for(tbb::blocked_range<It>(b, e, grain),
                [&](const tbb::blocked_range<It>& r) {

                    It nb = r.begin(), ne = r.end();
                    rfun(nb, ne);

                }, partitioner);
        

    }

    partitioner_t partitioner;
};
