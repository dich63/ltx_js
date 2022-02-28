#pragma once
#include "ssf/fft.h"

#include <limits>

constexpr double NaN = std::numeric_limits<double>::quiet_NaN();

// imag( conj(f1),f2) 

#include "utils/complex-utils.h"

/*
template <template <class > class _COMPLEX, class _FLOAT>
inline static _FLOAT mul_imag_c(const _COMPLEX<_FLOAT>& fc, const _COMPLEX<_FLOAT>& f) {

	auto xc = fc.real();
	auto yc = fc.imag();
	auto x = f.real();
	auto y = f.imag();
	return xc * y - x * yc;
}

template <template <class > class _COMPLEX, class _FLOAT>
inline static _FLOAT abs2(const _COMPLEX<_FLOAT>& f) {
	auto x = f.real();
	auto y = f.imag();
	return x * x + y * y;
}

*/

template <class _MEMPOOL = mem_pool_t<>>
struct expLL_t {

    typedef _MEMPOOL pool_t;

    expLL_t(size_t _N, double _t = 1.0, complex_t _alpha = complex_t())
        :N(_N), fft(_N) {

        double pi = std::acos(double(-1.));
        complex_t i2pi = complex_t(0., 2. * pi);
        bN = 1. / double(N);
        //e = std::exp(i2pi *bN);
        pool.alloc(N, &expDt);


        p2bN = 2 * pi * bN;

        reset(_t, _alpha);

        alpha_prev = { NaN,NaN };
        t_prev = NaN;
    }

    expLL_t& reset(double _t = 1.0, complex_t _alpha = complex_t()) {
        t = _t;
        alpha = _alpha;
		complex_t d = 1.0-_alpha/2.0;
        complex_t i2t = { 0, 2 * t };
        for (auto k = 0; k < N; ++k) {
            expDt[k] = bN * std::exp(i2t * (std::cos(p2bN * k) - d));
        }

        return *this;
    }

    expLL_t& reset_if(double _t = 1.0, complex_t _alpha = complex_t()) {
        bool fa = check_reset(_alpha, alpha_prev);
        bool ft = check_reset(_t, t_prev);
        if (fa || ft) {
            reset(_t, _alpha);
        }
        return *this;
    }

    inline void make_D(complex_t* x) {
        for (auto k = 0; k < N; ++k) {
            x[k] *= expDt[k];
        }
    }


    inline long make(complex_t* x) {

        auto xv = (bN_t<complex_t, 16>*)(x);
        if (!(st = fft.forward(x))) {
            make_D(x);
            st = fft.backward(x);
        }
        return st;
    }

    inline    complex_t* operator ()(complex_t* x) {
        make(x);
        return x;
    }

    long st;
    complex_t* expDt;
    complex_t alpha, alpha_prev;
    double t, t_prev;
    double bN, p2bN;
    size_t N;
    FFT_t<> fft;
    pool_t pool;
};



template <class _MEMPOOL = mem_pool_t<>>
struct nls_ssf_t {
    typedef _MEMPOOL pool_t;

	typedef typename complex_t complex_t;
	typedef typename complex_t complex64_t;


    nls_ssf_t(size_t _N, double _t = 1.0, double _g = 1.0, complex_t _alpha = complex_t())
        :expLL(_N, _t, _alpha), N(_N),omagnus(0){

        reset(_t, _g, _alpha);

    }

    nls_ssf_t& reset(double _t = 1.0, double _g = 1.0, complex_t _alpha = complex_t()) {
        g = _g;
		t = _t;
        expLL.reset_if(t, _alpha);
		ffirst = true;
        return *this;
    }

	template<class DATA>
	void preset(DATA* data) {

		if ((data->masks & 32)) {
			ffirst = false;			
		}

	};

	void NL_exp_magnus(complex_t* xb) {

		auto gt = g * t;
		auto x = xb;
		auto fx = x[0], fxp = x[1], fxm = x[N - 1];

		auto x0 = fx;

		//int sgn_magnus = (omagnus > 0) ? 1 : -1;

		auto g1 = double(omagnus)*gt * t;

		for (auto k = 0; k < N - 1; k++) {


			auto  v0 = gt * abs2(fx) ;

			fxp = x[k + 1];		


			auto v1 = g1 * mul_imag_c(fx, fxm + fxp);
			auto vm = v0 + v1;

			auto cs = std::cos(vm), ss = std::sin(vm);
			x[k] *= complex_t(cs, ss);			



			fxm = fx;
			fx = fxp;		

		}

		fxp = x0;

		auto v0 = gt * abs2(fx);
		auto v1 = g1 * mul_imag_c(fx, fxm + fxp);
		auto vm = v0 + v1;

		auto cs = std::cos(vm), ss = std::sin(vm);
		x[N-1] *= complex_t(cs, ss);




	}


    void NL_exp(complex_t* xb) {



        if (g) {
			if (omagnus) {
				NL_exp_magnus(xb);
			}
			else {

				auto gt = g * t;

				if (ffirst) {
					ffirst = false;
					gt /= 2.0;
				}

				auto xe = xb + N;
				for (auto x = xb; x != xe; ++x) {
					auto rx = x->real();
					auto ix = x->imag();
					auto v = gt * (rx * rx + ix * ix);
					
					auto cs = std::cos(v), ss = std::sin(v);
					(*x) *= complex_t(cs, ss);
				}
			}
        }


    }



    inline nls_ssf_t& step(complex_t* x, int rep = 1) {
        for (auto r = 0; r < rep; ++r) {
			NL_exp(x);
            expLL(x);
			
            if (state())
                break;
        }

        return *this;
    }

    inline long state() {
        return expLL.st;
    }

    double g,t;
    size_t N;
	int omagnus;
	bool ffirst;

    expLL_t<pool_t> expLL;
};


template <class _MEMPOOL = mem_pool_t<>>
struct nls_x_ssf_t:nls_ssf_t<_MEMPOOL> {
	typedef typename complex_t complex_t;
	typedef typename complex_t complex64_t;
	typedef typename nls_ssf_t<_MEMPOOL> base_t;

	nls_x_ssf_t(size_t _N, double _t = 1.0, double _g = 1.0,complex_t _alpha = complex_t())
		:nls_ssf_t<_MEMPOOL>(_N,_t,_g){

		this->expLL.pool.alloc(_N,&x);
		std::fill(x, x + _N, complex_t());

	};

	template<class F>
	void load(F* f) {
		std::copy(f, f + this->N, x);
	}

	template<class F>
	void save(F* f) {
		std::copy(x, x + this->N, f);
	}

	long step(int rep = 1) {
		return static_cast<base_t*>(this)->step(x, rep).state();
	}

	complex_t *x;
};
