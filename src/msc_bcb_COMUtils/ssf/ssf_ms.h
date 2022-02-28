#pragma once 

#include "ssf_nls.h"

template <class _MEMPOOL = mem_pool_t<>>
struct ms_ssf_t {
	typedef _MEMPOOL pool_t;

	typedef typename complex_t complex_t;
	typedef typename complex_t complex64_t;


	ms_ssf_t(size_t _N, double _t = 1.0, double _g = 1.0, complex_t _alpha = complex_t())
		:expLLx(_N, _t, _alpha),
		expLLy(_N, _t, _alpha),
		N(_N), omagnus(0) {

		reset(_t, _g, _alpha);

	}

	ms_ssf_t& reset(double _t = 1.0, double _g = 1.0, complex_t _alpha = complex_t()) {
		g = _g;
		t = _t;
		expLLx.reset_if(t, _alpha);
		expLLy.reset_if(t, _alpha);
		return *this;
	}

	void NL_exp_magnus(complex_t* xb, complex_t* yb) {

		auto gt = g * t;
		auto x = xb;
		auto fx = x[0], fxp = x[1], fxm = x[N - 1];
		auto x0 = fx;

		auto y = yb;
		auto fy = y[0], fyp = y[1], fym = y[N - 1];
		auto y0 = fy;


		//int sgn_magnus = (omagnus > 0) ? 1 : -1;

		auto g1 = double(omagnus)*gt * t;

		for (auto k = 0; k < N - 1; k++) {


			auto  v0 = gt * (abs2(fx)+ abs2(fy));

			fxp = x[k + 1];
			fyp = y[k + 1];


			auto v1 = g1 * (mul_imag_c(fx, fxm + fxp)+ mul_imag_c(fy, fym + fyp));
			auto vm = v0 + v1;

			auto cs = std::cos(vm), ss = std::sin(vm);
			auto  expv = complex_t(cs, ss);
			x[k] *= expv;
			y[k] *= expv;



			fxm = fx;
			fx = fxp;

			fym = fy;
			fy = fyp;

		}

		fxp = x0;
		fyp = y0;
		
		auto  v0 = gt * (abs2(fx) + abs2(fy));
		auto v1 = g1 * (mul_imag_c(fx, fxm + fxp) + mul_imag_c(fy, fym + fyp));
		auto vm = v0 + v1;

		auto cs = std::cos(vm), ss = std::sin(vm);
		auto  expv = complex_t(cs, ss);

		x[N - 1] *= expv;
		y[N - 1] *= expv;

	}


	void NL_exp(complex_t* xb, complex_t* yb) {



		if (g) {
			if (omagnus) {
				NL_exp_magnus(xb,yb);
			}
			else {
				double gt = g * t;
				auto xe = xb + N;
				//auto y = yb;

				for (auto x = xb, y = yb; x != xe; ++x,++y) {					
					
					auto v = gt *(abs2(*x) + abs2(*y));

					auto cs = std::cos(v), ss = std::sin(v);
					auto  expv = complex_t(cs, ss);
					(*x) *= expv;
					(*y) *= expv;
				}
			}
		}


	}



	inline ms_ssf_t& step(complex_t* x, complex_t* y, int rep = 1) {
		for (auto r = 0; r < rep; ++r) {
			NL_exp(x,y);
			expLLx(x);
			expLLy(y);

			if (state())
				break;
		}

		return *this;
	}

	inline long state() {
		return expLLx.st | expLLy.st;
	}

	double g, t;
	size_t N;
	int omagnus;

	expLL_t<pool_t> expLLx, expLLy;
};


template <class _MEMPOOL = mem_pool_t<>>
struct ms_xy_ssf_t :ms_ssf_t<_MEMPOOL> {
	typedef typename complex_t complex_t;
	typedef typename complex_t complex64_t;
	typedef typename ms_ssf_t<_MEMPOOL> base_t;

	ms_xy_ssf_t(size_t _N, double _t = 1.0, double _g = 1.0, complex_t _alpha = complex_t())
		:ms_ssf_t<_MEMPOOL>(_N, _t, _g) {

		auto& pool = this->expLLx.pool;

		pool.alloc(_N, &x);
		std::fill(x, x + _N, complex_t());
		pool.alloc(_N, &y);
		std::fill(y, y + _N, complex_t());

	};


	template<class F>
	void load(F* fx, F* fy) {
		std::copy(fx, fx + this->N, x);
		std::copy(fy, fy + this->N, y);
	}
	template<class F>
	void load(F* f) {
		load(f, f + this->N);
	}

	template<class F>
	void save(F* fx, F* fy) {
		std::copy(x, x + this->N, fx);
		std::copy(y, y + this->N, fy);
	}
	template<class F>
	void save(F* f) {
		save(f, f + this->N);
	}

	long step(int rep = 1) {
		return static_cast<base_t*>(this)->step(x,y, rep).state();
	}

	complex_t *x,*y;
};
