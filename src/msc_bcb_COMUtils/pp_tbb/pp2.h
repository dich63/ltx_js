#pragma once

#include "utils/utils.h"
#include "tbb/cache_aligned_allocator.h"
#include "tbb/parallel_for.h"


template <int I = 0>
struct partitioner2_t {
	typedef tbb::affinity_partitioner partitioner;
};

template <>
struct partitioner2_t<1> {
	typedef tbb::static_partitioner partitioner;
};



//template <class It,class  partitioner_t= tbb::affinity_partitioner >
template <class It, int IP = 0  >
struct tbb_parfor2_t {


	typedef typename partitioner2_t<IP>::partitioner partitioner_t;

	template <class Func>
	inline int operator()(bool pp, It b, It e, Func itemfun, size_t grain = 1) {
		//rfun(b, e);

		try {
			if (pp) {
				tbb::parallel_for(tbb::blocked_range<It>(b, e, grain),
					[&](const tbb::blocked_range<It>& r) {
						int err = 0;
						for (auto i = r.begin(); i != r.end(); ++i) {
							if (err = itemfun(i))
								throw err;
						}



					}, partitioner);
			}
			else {
				int err = 0;
				for (auto i = b; i != b; ++i) {
					if (err = itemfun(i))
						throw err;
				}
			}
		}
		catch (int err) {
			return err;
		}
		return 0;
	}

 partitioner_t partitioner;

};
