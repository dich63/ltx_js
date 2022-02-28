#pragma once
// #include "pade_nls.h" 
//
#include<complex>



#include "pade/magnus-utils.h"
#include "pade/pade_nls_base.h"

template <template <class > class _COMPLEX = std::complex, class _FLOAT = double, class _MEMPOOL = mem_pool_t<> >
struct pade_magnus_nls_t {
	typedef typename pade_nls_base_t<_COMPLEX, _FLOAT, _MEMPOOL>  pade_t;
	typedef typename pade_magnus_nls_t<_COMPLEX, _FLOAT, _MEMPOOL> self_t;
	typedef typename self_t  *pself_t;
	typedef typename pade_t::float_t float_t;
	typedef typename pade_t::complex_t complex_t;
	typedef typename _COMPLEX<double> complex64_t;
	typedef _MEMPOOL pool_t;
	typedef void (self_t::* NL_proc_t)();


	pade_magnus_nls_t(size_t _N, float_t _g = 0, complex_t _alpha = complex_t())
		:X_nls(_N), N(_N),
		g(_g), alpha(_alpha), alpha_prev(_alpha), omagnus(0), ncorr(0){


		std::tie(vb, ve) = pool.alloc2(_N, &vv, complex_t());
		std::tie(xb, xe) = pool.alloc2(_N, &xx, complex_t());

		
		pool.alloc(_N, &vv1);
		pool.alloc(_N, &mA);
		pool.alloc(_N, &mB);
		pool.alloc(_N, &mBc);		

		set_NaN(alpha_prev);
		set_NaN(g_prev);
	}



	//pade_nls_base_c_t<_COMPLEX, _FLOAT, _MEMPOOL>(_N)

	//pade_nls_base_c_t<_COMPLEX, _FLOAT, _MEMPOOL> X_nls, Y_nls;

	void check_modify() {

		fgmodify = check_reset(g, g_prev) || g;
		fmodify = check_reset(alpha_nl, alpha_nl_prev) || check_reset(alpha, alpha_prev) || fgmodify;

	}

	void preset(void*) {};

	pade_magnus_nls_t& reset(double dt = 1.0, float_t _g = 0, complex_t _alpha = complex_t(), int _nt = 1, std::tuple<int, int> nm = { 8,8 }) {
		int _n, _m;
		g = _g;
		alpha = _alpha;
		std::tie(_n, _m) = nm;
		return reset(_n, _m, dt, _nt);
	}

	pade_magnus_nls_t& reset(int _n, int _m, double _dt = 1.0, int _nt = 1) {

		if (_nt < 1) _nt = 1;

		if (_m < 0) _m = _n;

		//dt = dt / (nt = _nt);
		n = _n;
		m = _m;
		dt = _dt;
		X_nls.reset(_n, _m, dt,ncorr);

		alpha_nl = { 0.,0. };

		set_NaN(alpha_nl_prev);
		set_NaN(alpha_prev);
		set_NaN(g_prev);

		return *this;
	}



	template<class F>
	void load(F* fx) {
		std::copy(fx, fx + this->N, xb);		
	}

	
	template<class F>
	void save(F* fx) {
		std::copy(xb, xe, fx);		
	}
	
	

	

	void set_magnus_NL() {

		auto g = this->g;
		auto x = xx;
		
		
		auto fx = x[0], fxp = x[1], fxm = x[N - 1];
		

		int sgn_magnus = (omagnus > 0) ? 1 : -1;

		//auto g1 = -float_t(sgn_magnus)*g * dt;// / 2.;
		auto g1 = -g * dt;// / 2.;

		auto magnus = [&](auto k, auto fxp) {
		
		};




		for (auto k = 0; k < N - 1; k++) {


			auto  v0 = g * (abs2(fx));




			fxp = x[k + 1];
			



			auto v1 = g1 * (mul_imag_c(fx, fxm + fxp));

			auto vm = v0+ v1;

			

			if (0&&(vm < 0)) {

				char buf[1000];
				sprintf_s(buf, 1000, "vm=%g ; v0=%g ;v1=%g", vm,v0,v1);
				FatalAppExitA(0, buf);
			}

			//vv[k].real(vm);
			mA[k]=vm;
			mB[k] = 1.0;
			mBc[k] = 1.0;

			fxm = fx;
			fx = fxp;

			

		}

		//for (auto k = N - 1; k < N; k++)
		{


			auto  v0 = g * (abs2(fx));



			fxp = x[0];
			

			auto v1 = g1 * (mul_imag_c(fx, fxm + fxp) );


			auto vm = v0 + v1;

			//vv[N - 1].real(vm);
			mA[N - 1]=vm;
			mB[N - 1] = 1.0L;
			mBc[N - 1] = 1.0L;

		}

		//*/
	}


	void set_magnus_NL2() {

		auto g = this->g;
		auto x = xx;


		auto fx = x[0], fxp = x[1], fxm = x[N - 1], fxp2 = x[2], fxm2 = x[N - 2];


		int sgn_magnus = (omagnus > 0) ? 1 : -1;

		//auto g1 = -float_t(sgn_magnus)*g * dt;// / 2.;
		auto g1 = -g * dt;// / 2.;
		auto g2 = -g1 * dt / 3;//6
		auto gc2 = -1.0*g2 / 2;
		//auto gc2 = -g1 * dt / 2;
		//auto g2 = -g1 * dt / 3;

		auto magnus = [&](auto k, auto f) {

			//fxp2 = x[k + 2];
			fxp2 = f;
			auto  v0 = (abs2(fx));


			auto v1 = (mul_imag_c(fx, fxm + fxp));


			auto fpm2 = abs2(fxm2 + fxp2);

			auto v2 = abs2(fxm + fxp) - (2 * v0 + mul_real_c(fx, fxm2 + fxp2));

			auto t0 = abs2(fxm + fxp);
			auto t1 = 2 * v0;
			auto t2 = mul_real_c(fx, fxm2 + fxp2);

			//(mul_imag_c(fx, fxm + fxp));



			auto vm = g * v0 + g1 * v1 + g2 * v2;

			

			//vv[k].real(vm);
			//vv[k] = vm;
			vv1[k] = v1;
			mA[k] = vm;
			mB[k] = 1.0;
			mBc[k] = 1.0;


			fxm2 = fxm;
			fxm = fx;
			fx = fxp;
			fxp = fxp2;

		};



		for (auto k = 0; k < N - 2; k++) {

			magnus(k, x[k + 2]);

		}



		for (auto k = N - 2; k < N; k++) {

			magnus(k, x[k - (N - 2)]);
		}


		if (sgn_magnus > 0) {

			auto vc = vv1[0];
			auto vp = vv1[N - 1];

			complex_t cgc2 = gc2;

			auto  dv = cgc2 * (vc - vp);
			

			mB[0] += dv;
			mBc[N - 1] -= dv;

			vc = vp;

			for (auto k = 1; k < N; k++) {

				vp =  vv1[k];
				dv = cgc2 * (vc - vp);				
				
				mB[k] += dv;
				mBc[k - 1] -= dv;

				vc = vp;
			}


		}

	}

	

	void set_NL() {

		auto g = this->g;


		if (g) {

			
			if (omagnus) {
				if(_abs(omagnus)>1)
					set_magnus_NL2();
				else
					set_magnus_NL();
			}
			else				
				for (auto k = 0;k<N; ++k) {

					//v->real(g*abs2(*x));
					mA[k]=(g*abs2(xx[k]));
					mB[k] = 1.0L;
					mBc[k] = 1.0L;

				}
		}
		else if (fgmodify) {

			//std::fill(vb, ve, complex_t());
			std::fill(mA, mA + N, complex_t((0.0)));
			std::fill(mB, mB + N, complex_t(float_t(1.0)));
			std::fill(mBc, mBc + N, complex_t(float_t(1.0)));

		}


	}

	int step(int pp = 0) {

		check_modify();

		set_NL();
		auto diag = alpha - complex_t(2);


		int ppi = pp & 1;

		

		

		if (fmodify) {
		//	static_cast<base_t*>(this)->step(vv, diag, xx, pp);
			X_nls.step({ mA,mB,mBc }, diag, xx, pp);
		}
		else {
			X_nls.step(xx, pp);
		}
		


		return 0;
	}





	


	pade_t X_nls;		
	size_t N;
	int nt,ncorr;
	int omagnus;
	int n, m;
	double dt;
	double w;
	float_t g, g_prev;
	//complex_t * xy[2];
	complex_t *xb, *xe, *xx;	
	complex_t *vb, *ve, *vv;
	complex_t *vv1;
	

	complex_t *mA, *mB, *mBc;
	complex_t alpha, alpha_prev;
	complex_t alpha_nl, alpha_nl_prev;

	bool fmodify, fgmodify;

	pool_t pool;
	tbb::affinity_partitioner afp;

};


template <template <class > class _COMPLEX = std::complex, class _FLOAT = double, class _MEMPOOL = mem_pool_t<> >
struct pade_magnus_nls_step_t:pade_magnus_nls_t<_COMPLEX, _FLOAT,_MEMPOOL> {

	typedef typename pade_nls_base_t<_COMPLEX, _FLOAT, _MEMPOOL>  pade_t;	
	typedef typename pade_t::float_t float_t;
	typedef typename pade_t::complex_t complex_t;
	typedef typename _COMPLEX<double> complex64_t;

	

	
	void load(complex64_t* d) {

		

		std::copy(d, d + this->N, xb); d += this->N;
		std::copy(d, d + this->N, mA); d += this->N;
		std::copy(d, d + this->N, mB); d += this->N;
		std::copy(d, d + this->N, mBc); d += this->N;		
	}

	void save(complex64_t* d) {
		
		std::copy(x, xb + this->N, d); d += this->N;
		//std::copy(mA, mA + this->N, d); d += this->N;
		//std::copy(mB, mB + this->N, d); d += this->N;
		//std::copy(mBc, mBc + this->N, d); d += this->N;		
	}	
	

	pade_magnus_nls_step_t(size_t _N, float_t _g = 0, complex_t _alpha = complex_t()) :pade_magnus_nls_t(_N,_g,_alpha) {
		
	}

	long step(int pp = 0) {
		return X_nls.step({ mA,mB,mBc }, 0.0, xx, pp);
	}

};