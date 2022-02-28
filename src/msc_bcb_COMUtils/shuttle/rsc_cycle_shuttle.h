#pragma once 


//#include "shuttle/rsc_cycle_shuttle.h"

#include "./rsc_shuttle.h"


#include <vector>


template <class _FLOAT,  class _MEMPOOL = mem_pool_t<> >
struct rsc_cycle_shuttle_t {

	typedef _FLOAT f_t;
	typedef _MEMPOOL pool_t;

	rsc_cycle_shuttle_t(int _n) :A(_n-1),n(_n),ne(_n-1) {
		
		A.pool.alloc(n,&F);
		iH = Gb = Ge = 0;

	};

	~rsc_cycle_shuttle_t() {
	}

	
	inline  void clear_set(f_t* p,f_t vb, f_t ve) {
		::memset(p, 0, sizeof(f_t) * n);
		p[0] = vb;
		p[n-2] = ve;
	}


	inline  void init_F() {
		::memset(F, 0, sizeof(f_t) * n);
		F[0] = -1;
		F[n - 2] = -1;
	}




	int factorize2(f_t* am, f_t a, f_t* x) {

		int err = 0;
		const f_t E = f_t(1);

		init_F();

		b5_t* px = (b5_t*)x;
		b5_t* pF = (b5_t*)F;

		//err = (x) ? A.factorize2(a,am, b, c + 1, F, x) : A.factorize2(a,am, b, c + 1, F);

		//
		err = A.factorize2(am, a, F, x);
		//		err = A.factorize2a(am, a, F, x);

		

		f_t  D = a + am[n - 1];

		//H = D + G[0] * F[0]+ G[n-2] * F[n-2];
		H = D + F[0] + F[n - 2];



		iH = E / H;

		//Gb = -iH;
		//Ge = -iH;

		err = _post_solve(x);


		return err;

	}



	
	int solve(f_t* x) {	

		return A.solve(x) || _post_solve(x);			
	}



	
	int _post_solve(f_t* x) {

		register f_t xm;

		//x[n - 1] = xm = iH * (x[n - 1] - x[0] - x[n - 2]);
		x[n - 1] = xm =  (x[n - 1] - x[0] - x[n - 2])/H;



		for (auto k = 0; k < n - 1; k++) {
			x[k] += F[k] * xm;
		}

		return 0;
	}


	int post_solve(f_t* x) {
		int err = 0;

		if (x) {

			err=_post_solve(x);

		}

		return err;
	}


	rsc_shuttle_t<f_t,pool_t> A;
		int n,ne;
		f_t *F,iH,Gb,Ge,H;
};




