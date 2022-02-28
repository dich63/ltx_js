#pragma once 


//#include "shuttle/cycle_shuttle.h"

#include "./shuttle.h"
#include "./rsc_shuttle.h"

template <class F>
struct matrix_triplet_t {
	F* a;
	F* b;
	F* c;
};

template <class F, class _MEMPOOL = mem_pool_t<> >
struct cycle_shuttle_t {

	typedef F f_t;
	typedef matrix_triplet_t<F> triplet_t;
	typedef _MEMPOOL pool_t;

	cycle_shuttle_t(int _n):
		 A(_n-1)
		,Ac(_n - 1)
		,n(_n),ne(_n-1) {

		//F = (f_t*)::malloc(n * sizeof(f_t));
		//G = (f_t*)::malloc(n * sizeof(f_t));
		F = A.pool.alloc<f_t>(n);

	};

	inline pool_t& get_pool() {
		return A.pool;
	}

	~cycle_shuttle_t() {
		//::free(G);
		//::free(F);
	}

	inline void clear(f_t* p) {
		::memset(p, 0, sizeof(f_t) * n);
	}

	inline void clear_set(f_t* p,f_t vb, f_t ve) {
		::memset(p, 0, sizeof(f_t) * n);
		p[0] = vb;
		p[n-2] = ve;
	}

	int factorize(f_t* a, f_t* b, f_t* c, f_t* x) {

		int err = 0;
		const f_t E = f_t(1);

		clear_set(F,-c[0],-b[ne-1]);
		//clear_set(G, b[ne],c[ne]);	
		Gb = b[ne];
		Ge = c[ne];

		//b5_t* pF = (b5_t*)F;
		//b5_t* px = (b5_t*)x;

		 //memcpy((void*)F, x, sizeof(f_t) *n );

		

		
		err =(x)? A.factorize(a, b, c + 1,F,x): A.factorize(a, b, c + 1, F);

		f_t H,D = a[n - 1];

		//H = D + G[0] * F[0]+ G[n-2] * F[n-2];
		H = D + Gb * F[0] + Ge * F[n - 2];
		
		

		iH = E / H;
		//G[0] *= -iH;
		//G[n-2] *= -iH;

		Gb *= -iH;
		Ge *= -iH;

		err = post_solve(x);


		return err;
	
	}

	int factorize2(f_t* am, f_t* a, f_t* b, f_t* c, f_t* x) {

		int err = 0;
		const f_t E = f_t(1);

		clear_set(F, -c[0], -b[ne - 1]);
		
		Gb = b[ne];
		Ge = c[ne];

		//err = (x) ? A.factorize2(a,am, b, c + 1, F, x) : A.factorize2(a,am, b, c + 1, F);

		err = A.factorize2(am, a, b, c + 1, F, x);

		b5_t* px = (b5_t*)x;

		f_t H, D = a[n - 1]+ am[n - 1];

		//H = D + G[0] * F[0]+ G[n-2] * F[n-2];
		H = D + Gb * F[0] + Ge * F[n - 2];



		iH = E / H;

		Gb *= -iH;
		Ge *= -iH;

		err = post_solve(x);


		return err;

	}


	int factorize_mp(f_t az,const triplet_t& t, f_t* x) {

		return factorize_mp(az, t.a, t.b, t.c, x);
	
	}

	int factorize_mp(f_t az, f_t* a, f_t* b, f_t* c, f_t* x) {

		int err = 0;
		const f_t E = f_t(1);


		auto px = pN_t<4>::ptr(x);
		auto pF = pN_t<4>::ptr(F);

		auto pa = pN_t<4>::ptr(a);
		auto pb = pN_t<4>::ptr(b);
		auto pc = pN_t<4>::ptr(c);

		clear_set(F, -c[0], -b[ne - 1]);

		Gb = b[ne];
		Ge = c[ne];

		//err = (x) ? A.factorize2(a,am, b, c + 1, F, x) : A.factorize2(a,am, b, c + 1, F);

		//
		err = A.factorize_mp(az, a, b, c + 1, F, x);
		//err = Ac.factorize2(a,az, F, x);

		


		f_t  D = a[n - 1] + az;

		//H = D + G[0] * F[0]+ G[n-2] * F[n-2];
		H = D + Gb * F[0] + Ge * F[n - 2];



		iH = E / H;

		//Gb *= -iH;
		//Ge *= -iH;

		err = _post_solve_mp(x);


		return err;

	}


	int factorize2_c(f_t* am, f_t a, f_t b, f_t c, f_t* x) {

		int err = 0;
		const f_t E = f_t(1);

		clear_set(F, -c, -b);

		Gb = b;
		Ge = c;

		b5_t* px = (b5_t*)x;
		b5_t* pF = (b5_t*)F;

		//err = (x) ? A.factorize2(a,am, b, c + 1, F, x) : A.factorize2(a,am, b, c + 1, F);

		err = A.factorize2_c(am, a, b, c, F, x);

		

		f_t H, D = a + am[n - 1];

		//H = D + G[0] * F[0]+ G[n-2] * F[n-2];
		H = D + Gb * F[0] + Ge * F[n - 2];



		iH = E / H;

		Gb *= -iH;
		Ge *= -iH;

		err = _post_solve(x);


		return err;

	}



	
	int solve(f_t* x) {	

		return A.solve(1,&x) || post_solve(x);			
	}

	int solve_mp(f_t* x) {

		//
		return A.solve_mp(1, &x) || _post_solve_mp(x);
		//return A.solve_mp(x) || _post_solve_mp(x);
	}

	inline int operator()(f_t* x) {
		return solve_mp(x);
	}

	
	int _post_solve_mp(f_t* x) {

		/*
		register f_t xm, Gxn;
		xm = x[n - 1];
		xm *= iH;
		//Gxn = G[0] * x[0]+ G[n-2] * x[n-2];
		Gxn = Gb * x[0] + Ge * x[n - 2];

		x[n - 1] = (xm += Gxn);
		*/

		register f_t xm;

		//x[n - 1] = xm = iH * (x[n - 1] - Gb * x[0] - Ge * x[n - 2]);
		x[n - 1] = xm =  (x[n - 1] - Gb * x[0] - Ge * x[n - 2])/H;

		for (auto k = 0; k < n - 1; k++) {
			x[k] += F[k] * xm;
		}

		return 0;
	}


	int _post_solve(f_t* x) {

		
		register f_t xm, Gxn;
		xm = x[n - 1];
		xm *= iH;
		//Gxn = G[0] * x[0]+ G[n-2] * x[n-2];
		Gxn = Gb * x[0] + Ge * x[n - 2];

		x[n - 1] = (xm += Gxn);
		

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


		shuttle_t<f_t,pool_t> A;
		rsc_shuttle_t<f_t, pool_t> Ac;
		int n,ne;
		f_t *F,iH,H,Gb,Ge;
};




