#pragma once 

#include "utils/utils.h"


#include "trimatrix/trimatrix_base.h"

#define _FAST_DIV(a,b) _fast_div(a,b)
#define _FAST_INV(a) _fast2_inv(a)

namespace trimatrix {
	template <class _F, class _MEMPOOL = utils::mem_pool_t<> ,bool fcopy_c=false >
	struct thomas_t {
		typedef _MEMPOOL pool_t;
		typedef _F f_t;

		thomas_t(size_t _N):N(_N) {

			pool.alloc(_N, &iL);
			pool.alloc(_N, &R);	
			if(fcopy_c)
				pool.alloc(_N, &c);

		}

		template <class XX>
		inline int inner_factorize(f_t bz, f_t* b, f_t* a, f_t* _c, XX& xx) {

			

			auto n = N;
			int err = 0;

			if constexpr (fcopy_c){
				std::copy(_c, _c + N, c);
			}
			else {
				c = _c;
			}

			f_t iBm, Rk = f_t() , iBkAC= f_t();

			auto update = [&](auto k) {

				
				iBm = _FAST_INV((b[k] + bz) - iBkAC);
				iL[k] = iBm;
				R[k] = Rk;
				
				
			};


			update(0);

			f_t xm[xx.M+1];

			xx.op([&](auto x,auto m) {
				xm[m]= x[0] ;
				});

			for (auto k = 1; k < n; k++) {		

				
				
				Rk = iBm * a[k];

				

				xx.op([&](auto x,auto m) {

					xm[m] = ( x[k]-= Rk * xm[m] ) ;				

					});				

				iBkAC = Rk * c[k - 1];
				update(k);


				

				
			}

			
				if(xx.M)
					err = solve_u(xx);

			return err;
		}


		template <class XX>
		inline int solve_u(XX& xx) {

			std::int64_t n = N;
			
			f_t xm[xx.M + 1];

			

			xx.op([&](auto x,auto m) {

				xm[m]=x[n - 1] *= iL[n - 1];
				
				});

			for (auto k = n - 2; k >= 0; k--) {

				auto iLk = iL[k];
				auto ck = c[k];

				xx.op([&](auto x,auto m) {

					  x[k]= xm[m] = iLk * (x[k] - ck * xm[m]);
					
					});
				
			}

			return 0;
		}

		template <class XX>
		inline int solve(XX& xx) {
			if constexpr (xx.M) {

				
				f_t xm[xx.M + 1];

				xx.op([&](auto x, auto m) {
					xm[m] = x[0];
					});

				auto n = N;

				
				for (auto k = 1; k < n; k++) {

					auto Rk = R[k];

					xx.op([&](auto x, auto m) {

						xm[m] = (x[k] -= Rk * xm[m]);

						});
					
				}

				return solve_u(xx);

			}
			else
				return 0;
		}



		
		template <class Triplet ,class XX>
		inline int factorize(Triplet* t, XX& xx, f_t az= f_t()) {
			return inner_factorize(az, t->a, t->b, t->c, xx);
		}
		template <class Triplet, class XX>
		inline int factorize(Triplet* t, f_t az = f_t()) {
			xx_t<f_t, 0> xx;
			return inner_factorize(az, t->a, t->b, t->c, xx);
		}

		template <class XX>
		inline int factorize(f_t _az, f_t* _a, f_t* _b, f_t* _c, XX& xx) {

			return inner_factorize(_az, _a, _b, _c, xx);
		}
		


		size_t N;
		f_t* iL, * R,*c;
		
		pool_t pool;
	};



};
