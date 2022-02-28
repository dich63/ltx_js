#pragma once
//ls_corr.h
#include "./utils.h"
#include <complex>
#include <math.h>    

template <class f_t>
static double subAx(size_t N, f_t* a, f_t* b, f_t* c, f_t* xx, f_t* y) {

	auto xm = xx[N - 1], x = xx[0], xp = xx[1];
	double rm = 0;

	for (auto k = 0; k < N - 1; ++k) {

		xp = xx[k + 1];
		y[k] -= b[k] * xm + a[k] * x + c[k] * xp;
		xm = x;
		x = xp;

		double r = std::abs(y[k]);
		if (rm < r) rm = r;

	}

	xp = xx[0];
	y[N - 1] -= b[N - 1] * xm + a[N - 1] * x + c[N - 1] * xp;

	double r = std::abs(y[N - 1]);
	if (rm < r) rm = r;

	return rm;

};




template <class f_t>
static double subAx(size_t N, f_t az, f_t* a, f_t* b, f_t* c, f_t* xx, f_t* y) {

	auto xm = xx[N - 1], x = xx[0], xp = xx[1];
	double rm = 0;

	for (auto k = 0; k < N - 1; ++k) {

		xp = xx[k + 1];
		y[k] -= b[k] * xm + (a[k]+az) * x + c[k] * xp;
		xm = x;
		x = xp;

		double r = std::abs(y[k]);
		if (rm < r) rm = r;

	}

	xp = xx[0];
	y[N - 1] -= b[N - 1] * xm + (a[N - 1]+az )* x + c[N - 1] * xp;

	double r = std::abs(y[N - 1]);
	if (rm < r) rm = r;

	return rm;

};





template <class _F>
struct ls_corr_t{
	typedef _F f_t;


	
	ls_corr_t() {};

	ls_corr_t(size_t _N, f_t* _y, f_t* _c)  {
		init(_N, _y, _c);
	};

	void init(size_t _N, f_t* _y, f_t* _c) {
		N = _N;
		y = _y;
		cc = _c;
	}


	void set_b(f_t* b, int nrep = 1) {
		if(nrep>0)
			::memcpy(y, b, sizeof(f_t) * N);
	}

	template <class F_subA>
	double residual(F_subA subA, f_t* x) {
		::memcpy(cc, y, sizeof(f_t) * N);
		return err= subA(x, cc);
	}

	template <class F_subA>
	double residual1(F_subA subA, f_t* x) {
		::memcpy(cc, y, sizeof(f_t) * N);
		err = subA(x, cc);
		double r = 0;
		for (auto k = 0; k < N; k++) {
			r += std::abs(cc[k]);
		}
		return r / N;
	}

	template <class F_subA,class F_iA>
	f_t* make(F_subA subA, F_iA iA, f_t* x,int nrep=1) {

		

		for (auto n = 0; n < nrep; ++n) {

			double brm, rm;
			int dm;

			rm = residual(subA, x);



			::frexp(rm, &(dm = 0));
			rm = ::pow(2., dm);
			brm = ::pow(2., -dm);
			//brm = rm = 1;

			for (auto k = 0; k < N; k++) {
				cc[k] *= brm;
			}

			iA(cc);

			for (auto k = 0; k < N; k++) {
				auto c = cc[k];
				c *= rm;
				x[k] += c;
			}
		}

		return x;
	}



	double err;
	f_t* y,*cc;
	size_t N;

};