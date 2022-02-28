
#pragma once 




#include "trimatrix/thomas.h"
#include <tuple>

#define _FAST_DIV_C(a,b) _fast_div(a,b)
#define _FAST_INV_C(a) _FAST_INV(a)

namespace trimatrix {

	template <class _F >
	struct thomas_cycle_base_t {
		
		typedef _F f_t;

		template <class Pool>
		void init(Pool& pool) {
			pool.alloc(n, &F);
		}

		inline void clear_set(f_t* p, f_t vb, f_t ve) {
			::memset(p, 0, sizeof(f_t) * n);
			p[n - 2] = ve;
			p[0] = vb;
			
		}

		thomas_cycle_base_t(size_t N) :n(N), ne(N-1), F(nullptr) {};


		template <class Triplet ,class TriSolver,class XX>
		int inner_factorize(TriSolver& solver, f_t az, Triplet* trm, XX& xx) {

			int err = 0;
			const f_t E = f_t(1);

			auto a = trm->a;
			auto b = trm->b;
			auto c = trm->c;

			//clear_set(F, E, E);
			//
			clear_set(F, -b[0], -c[ne - 1]);

			//clear_set(G, b[ne],c[ne]);	
			Gb = c[ne];
			Ge = b[ne];

			xx_t<f_t, xx.M + 1> xxF;
			xxF.xx[0] = F;
			attach_xx(xxF, xx, 1);

			auto pF = bN_t<f_t, 4>::ptr(F);
			auto px = bN_t<f_t, 4>::ptr(xx.xx[0]);

			err = solver.factorize(trm, xxF,az);

			f_t  D = a[n - 1];

			H = D + Gb * F[0] + Ge * F[n - 2];

			iH = E / H;




			err = _post_solve(xx);
			//*/

			return err;

		};

		template <class XX>
		int _post_solve(XX& xx) {

			auto px = bN_t<f_t, 4>::ptr(xx.xx[0]);

			f_t xm[xx.M+1];

			auto iH = this->iH;
			auto Gb = this->Gb;
			auto Ge = this->Ge;


			xx.op([&](auto x, auto m) {

				/*f_t  Gxn;

				xm[m] = iH*x[n - 1];				
				Gxn = Gb * x[0] + Ge * x[n - 2];
				x[n - 1] = (xm[m] += Gxn);*/
				x[n - 1] = xm[m] = (x[n - 1] - Gb * x[0] - Ge * x[n - 2]) / H;
				
				});		
			
			


			for (auto k = 0; k < n - 1; k++) {

				auto Fk = F[k];

				xx.op([&](auto x, auto m) {
					x[k] += Fk * xm[m];
					});
			}
			
			return 0;
		}

		template <class TriSolver, class XX>
		int inner_solve(TriSolver& solver,XX& xx) {

			return solver.solve(xx) || _post_solve(xx);

		}
		

		f_t *F, iH, H, Gb, Ge;
		int n,ne;
	
	};


	template <class _F, class _MEMPOOL = utils::mem_pool_t<>, bool fcopy_c = false>
	struct thomas_cycle_t :thomas_cycle_base_t<_F> {


		typedef _MEMPOOL pool_t;
		typedef _F f_t;
		typedef  thomas_t<f_t,pool_t,fcopy_c>  solver_t;
		//typedef typename _SOLVER<f_t,>

		thomas_cycle_t(size_t _N) :thomas_cycle_base_t<_F>(_N), solver(_N-1), N(_N) {
			this->init(solver.pool);
		}

		template <class Triplet, class XX>
		inline int factorize(Triplet* t, XX& xx, f_t az = f_t()) {
			return this->inner_factorize(solver, az, t, xx);
		}
		template <class Triplet, class XX>
		inline int factorize(Triplet* t, f_t az = f_t()) {
			xx_t<f_t, 0> xx;
			return this->inner_factorize( solver, az, t, xx);
		}
		template <class XX>
		inline int solve(XX& xx) {
			return this->inner_solve(solver, xx);
		}

		size_t N;
		solver_t solver;
	};

};// namespace trimatrix
