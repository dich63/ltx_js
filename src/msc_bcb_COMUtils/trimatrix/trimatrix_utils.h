#pragma once 
//trimatrix

#include "utils/utils.h"
#include "utils/complex-utils.h"
#include "trimatrix/trimatrix_base.h"
#include <memory>
#include <algorithm>
#include <limits>

#undef min;
namespace trimatrix {

	template <class FF, class _Pool>
	FF& alloc_xx(size_t N,FF& ff, _Pool& pool) {

		ff([&](typename FF::f_t*& x) {
			pool.alloc(N, &x);
			});

		return ff;
	}

	template <class FF>
	FF& attach_xx(FF& ff, typename FF::f_t** xx, int M= 0x7fffffff, size_t offset = 0) {

		M =std::min(int(ff.M- offset),M);

		for (auto m = 0; m < M; m++) {
			ff.xx[offset + m] = xx[m];
		}
		return ff;
	}

	
	template <class FFD,class FFS>
	FFD& attach_xx(FFD& dest,  FFS& src, size_t offset = 0) {
		return attach_xx(dest, src.xx, src.M, offset);
	}

	template <class FF>
	FF& copy_xx(FF& ff, typename FF::f_t** xx,size_t N, int M = 0x7fffffff, size_t offset = 0) {

		M = std::min(int(ff.M - offset), M);

		for (auto m = 0; m < M; m++) {
			auto b = xx[m],e=b+N;
			std:copy(b,e,ff.xx[offset + m]);
		}
		return ff;
	}

	template <class FFD, class FFS>
	FFD& copy_xx(FFD& dest, FFS& src, size_t N, size_t offset = 0) {
		return copy_xx(dest, src.xx,N,src.M, offset);
	}

	template <class _F, int _M>
	struct xx_t {
		enum {
			M = _M
		};
		typedef _F f_t;
		f_t* xx[M];

		template <class Func>
		inline void operator()(Func f) {
			for (auto m = 0; m < M; m++) {
				f(xx[m]);
			}
		}

		template <class Func>
		inline void op(Func f) {
			for (auto m = 0; m < M; m++) {
				f(xx[m],m);
			}
		}
	};

	template <class _F>
	struct xx_t<_F, 0> {
		enum {
			M = 0
		};
		typedef _F f_t;
		f_t* xx[0];

		template <class Func>
		inline void operator()(Func ) {};

		template <class Func>
		inline void op(Func ) {};
	};

	template <class _F>
	struct xx_t<_F, 1> {
		enum {
			M = 1
		};
		typedef _F f_t;
		f_t* xx[1];

		template <class Func>
		inline void operator()(Func f) {
			f(xx[0]);
		}

		template <class Func>
		inline void op(Func f) {		
				f(xx[0], 0);
		}
		
	};

	template <class _F>
	struct xx_t<_F, 2> {
		enum {
			M = 2
		};
		typedef _F f_t;
		f_t* xx[2];

		template <class Func>
		inline void operator()(Func f) {
			f(xx[0]);
			f(xx[1]);
		}
		template <class Func>
		inline void op(Func f) {
			f(xx[0], 0);
			f(xx[1], 1);
		}
	};

	template <class _F>
	struct xx_t<_F, 3> {
		enum {
			M = 3
		};
		typedef _F f_t;
		f_t* xx[3];

		template <class Func>
		inline void operator()(Func f) {
			f(xx[0]);
			f(xx[1]);
			f(xx[2]);

		}
		template <class Func>
		inline void op(Func f) {
			f(xx[0], 0);
			f(xx[1], 1);
			f(xx[2], 2);
		}
	};

	template <class _F>
	struct xx_t<_F, 4> {
		enum {
			M = 4
		};
		typedef _F f_t;
		f_t* xx[4];

		template <class Func>
		inline void operator()(Func f) {
			f(xx[0]);
			f(xx[1]);
			f(xx[2]);
			f(xx[3]);
		}
		template <class Func>
		inline void op(Func f) {
			f(xx[0], 0); 
			f(xx[1], 1);
			f(xx[2], 2);
			f(xx[3], 3);
		}
	};

	template < template <int > class  Factory,class Fun>
	bool createInstance(int M, Fun fun) {
		bool f = true;
		switch (M) {
		case 1:
			fun( Factory<1>::createInstance );
			break;
		case 2:
			fun( Factory<2>::createInstance );
			break;
		default:
			f = false;
			break;
		};
		return f;
	}

}// trimatrix