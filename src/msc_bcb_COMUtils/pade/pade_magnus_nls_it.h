#pragma once
// #include "pade_nls.h" 
//
#include<complex>



#include "pade/magnus-utils.h"
#include "pade/pade_nls_base.h"
#include "pade/nls_data.h"

namespace it {
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
			g(_g), alpha(_alpha), alpha_prev(_alpha), omagnus(0), ncorr(0), flags(0){


			std::tie(vb, ve) = pool.alloc2(_N, &vv, complex_t());
			std::tie(xb, xe) = pool.alloc2(_N, &xx, complex_t());

			pool.alloc(_N, &xx_keep);

			pool.alloc(_N, &vvL);
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

		void preset(nls_data_t* nls_data) {
			flags = nls_data->flags;
		};

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
			X_nls.reset(_n, _m, dt, ncorr);

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

				auto  v0 = g * (abs2(fx));




				auto v1 = g1 * (mul_imag_c(fx, fxm + fxp)); //  ((g*dt)/2) * 2 Im(f*(fp+fm))

				auto vm = v0 + v1;
				auto vL = vm + v1;


				if (0 && (vm < 0)) {

					char buf[1000];
					sprintf_s(buf, 1000, "vm=%g ; v0=%g ;v1=%g", vm, v0, v1);
					FatalAppExitA(0, buf);
				}

				vv1[k] = v1;
				vvL[k] = vL;				
				mA[k] = vv[k] = vm;
				mB[k] = 1.0;
				mBc[k] = 1.0;

				fxm = fx;
				fx = fxp;



			};




			for (auto k = 0; k < N - 1; k++) {

				magnus(k, x[k + 1]);

			}

			magnus(N - 1, x[0]);

		}





		void set_NL() {

			auto g = this->g;


			if (g) {


				if (omagnus) {
					
						set_magnus_NL();
				}
				else
					for (auto k = 0; k < N; ++k) {

						//v->real(g*abs2(*x));
						mA[k] = (g*abs2(xx[k]));
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


		void set_NL_pass_2() {
			

			if (!((g) && (omagnus)))
				return;

			

			for (auto k = 0; k < N; ++k) {
				auto v0 = (g*abs2(xx[k]));
				mA[k] += (v0 - vvL[k]) / complex_t(3);
			}



			//return;

			if (flags) {

				int sgn = flags > 0 ? 1 : -1;
				complex_t s2 = complex_t(0, sgn*dt / 6.0);

				auto vp = vv1[0];



				for (auto k = 1; k < N; ++k) {

					auto v = vv1[k];

					auto dv = complex_t(1.0) + s2 * (vp - v);

					mBc[k] = dv;
					mB[k - 1] = std::conj(dv);
					vp = v;

				}

				auto dv = complex_t(1.0) + s2 * (vp - vv1[0]);
				mBc[0] = dv;
				mB[N - 1] = std::conj(dv);
			}

		}



		int step(int pp = 0) {

			check_modify();

			fmodify = true;


			
			auto diag = alpha - complex_t(2);


			int ppi = pp & 1;


			set_NL();

			keep_xx();
			X_nls.step({ mA,mB,mBc }, diag, xx, pp);

			set_NL_pass_2();
			restore_xx();
			X_nls.step({ mA,mB,mBc }, diag, xx, pp);
				//	static_cast<base_t*>(this)->step(vv, diag, xx, pp);
				
			


			return 0;
		}



		void keep_xx() {
			memcpy(xx_keep, xx, N * sizeof(complex_t));
		}

		void restore_xx() {
			memcpy(xx, xx_keep, N * sizeof(complex_t));
		}



		pade_t X_nls;
		size_t N;
		int nt, ncorr;
		int omagnus;
		int n, m;
		
		double dt;
		double w;
		float_t g, g_prev;
		//complex_t * xy[2];
		complex_t *xb, *xe, *xx,*xx_keep;
		complex_t *vb, *ve, *vv;
		complex_t *vv1,*vvL;


		complex_t *mA, *mB, *mBc;
		complex_t alpha, alpha_prev;
		complex_t alpha_nl, alpha_nl_prev;

		bool fmodify, fgmodify;
		int64_t flags;
		pool_t pool;
		tbb::affinity_partitioner afp;

	};


	template <template <class > class _COMPLEX = std::complex, class _FLOAT = double, class _MEMPOOL = mem_pool_t<> >
	struct pade_magnus_nls_step_t :pade_magnus_nls_t<_COMPLEX, _FLOAT, _MEMPOOL> {

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


		pade_magnus_nls_step_t(size_t _N, float_t _g = 0, complex_t _alpha = complex_t()) :pade_magnus_nls_t(_N, _g, _alpha) {

		}
	long	step(int pp = 0) {
			return X_nls.step({ mA,mB,mBc }, 0.0, xx, pp);
		}

	};
};// it