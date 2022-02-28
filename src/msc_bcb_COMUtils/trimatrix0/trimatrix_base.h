#pragma once 
//trimatrix

#include "utils/utils.h"
#include "utils/complex-utils.h"
#include "trimatrix/trimatrix_utils.h"
#include <memory>

namespace trimatrix {

	

	


template <class _F,  int M = 1, class _MEMPOOL= utils::mem_pool_t<> >
struct  trimatrix_base_t {
	/*
	[ a0, c0,  0,  0,  0,  0,  0, b0]
	[ b1, a1, c1,  0,  0,  0,  0,  0]
	[  0, b2, a2, c2,  0,  0,  0,  0]
	         ................
	[  0,  0,  0,  0, b5, a5, c5,  0]
	[  0,  0,  0,  0,  0, b6, a6, c6]
	[ c7,  0,  0,  0,  0,  0, b7, a7]
	*/

	typedef _MEMPOOL pool_t;
	
	typedef _F f_t;

	trimatrix_base_t():N(0),a(nullptr), b(nullptr), c(nullptr) {
		repeat_M([&](auto m) { yy[m] = nullptr;  });
	}

	trimatrix_base_t(size_t _N) :N(_N) {

		pool.alloc(N,&a);
		pool.alloc(N, &b);
		pool.alloc(N, &c);

		alloc_xx(N, yy, pool);

	}
	
	trimatrix_base_t(size_t _N, f_t* _a, f_t* _b, f_t* _c, f_t** _yy) :N(_N),a(_a), b(_b), c(_c) {

		attach_xx(yy, _yy, M);
	}
	
	inline void mul(f_t* x, f_t* y, f_t az=f_t()) {

		auto ne = N - 1; 	

		y[0] = b[0] * x[ne] + (a[0]+az) * x[0] + c[0] * x[0+1];

		for(auto n=1;n< ne;n++){
			y[n] = b[n] * x[n - 1] + (a[n] + az) * x[n] + c[n] * x[n+1];
		}		
		y[ne] = b[ne] * x[ne-1] + (a[ne] + az) * x[ne] + c[ne] * x[0];
	}

	size_t N;
	f_t* a, * b, * c;
	xx_t<f_t,M> yy;
	//utils::loop_op_t<M> repeat_M;
	pool_t pool;
};



};
