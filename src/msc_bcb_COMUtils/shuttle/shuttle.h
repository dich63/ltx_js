#pragma once 
//#include "shuttle/shuttle.h"



#include "./utils.h"


template <class F, class _MEMPOOL = mem_pool_t<>>
struct shuttle_t {

	typedef F f_t;
	typedef _MEMPOOL pool_t;

	shuttle_t(int _n) :n(_n) {

		/*
		iU = (f_t*)::malloc(n * sizeof(f_t));
		U = (f_t*)::malloc(n * sizeof(f_t));
		L = (f_t*)::malloc(n * sizeof(f_t));
		*/

		iU = pool.alloc<f_t>(n);
		U = pool.alloc<f_t>(n);
		L = pool.alloc<f_t>(n);
		cL = pool.alloc<f_t>(n);
		R = pool.alloc<f_t>(n);

		//R = (f_t*)::malloc(n * sizeof(f_t));

	};

	~shuttle_t() {

		//::free(R);

		/*
		::free(L);
		::free(U);
		::free(iU);
		*/

	}


	int factorize(f_t* a, f_t* b, f_t* c, f_t* x) {

		int err=0;

		//::memcpy(R, b, (n - 1) * sizeof(f_t));
		R = b;
		const f_t E = f_t(1);

		iU[0] = E / a[0];
		L[0] = c[0] * iU[0];

		auto px = (bN_t<f_t, 16>*)x;
		auto piU = (bN_t<f_t, 16>*)iU;

		for (auto k = 1; k < n; k++) {

			f_t iu;

			//iu = E / (a[k] - L[k - 1] * R[k - 1]);
			iu = _fast_inv(a[k] - L[k - 1] * R[k - 1]);
			L[k] = c[k] * iu;
			iU[k] = iu;


			x[k] -= L[k - 1] * x[k - 1];

		}

		x[n - 1] *= iU[n - 1];

		for (auto k = n - 2; k >= 0; k--) {

			x[k] = iU[k] * (x[k] - R[k] * x[k + 1]);
		}

		return err;
	}

	int factorize(f_t* a, f_t* b, f_t* c, f_t* x1, f_t* x2) {

		int err = 0;

		//::memcpy(R, b, (n - 1) * sizeof(f_t));
		R = b;
		const f_t E = f_t(1);

		iU[0] = E / a[0];
		L[0] = c[0] * iU[0];

		f_t Lkm;
		Lkm = L[0];

		f_t xm1, xm2;
		xm1 = x1[0];
		xm2 = x2[0];
		for (auto k = 1; k < n; k++) {

			f_t iu;
			f_t Lk;

			//iu = E / (a[k] - L[k - 1] * R[k - 1]);
			iu = _fast_inv(a[k] - L[k - 1] * R[k - 1]);
			L[k] = Lk = c[k] * iu;
			iU[k] = iu;



			//x[k] -= Lkm * x[k - 1];
			//x2[k] -= Lkm * x2[k - 1];

			x1[k] -= Lkm * xm1;
			xm1 = x1[k];

			x2[k] -= Lkm * xm2;			
			xm2 = x2[k];



			Lkm = Lk;

		}




		x1[n - 1] *= iU[n - 1];
		x2[n - 1] *= iU[n - 1];


		f_t xp1, xp2;
		xp1 = x1[n - 1];
		xp2 = x2[n - 1];


		for (auto k = n - 2; k >= 0; k--) {

			register f_t iUk = iU[k], Rk = R[k];
			
			x1[k]=xp1 = iUk * (x1[k] - Rk * xp1);
			x2[k]=xp2 = iUk * (x2[k] - Rk * xp2);
			
		}

		return err;
	}


	int factorize_mp00(f_t az, f_t* a, f_t* b, f_t* c, f_t* x1, f_t* x2) {

		int err = 0;

		
		R = b;
		const f_t E = f_t(1);

		//iU[0] = E/(a[0] + az);
		iU[0] = _fast_inv(a[0]+az);

		L[0] = c[0] * iU[0];

		f_t Lkm;
		Lkm = L[0];

		f_t xm1, xm2;
		xm1 = x1[0];
		xm2 = x2[0];
		for (auto k = 1; k < n; k++) {

			f_t iu;
			f_t Lk;

			//			iu = E / (a[k] + az - L[k - 1] * R[k - 1]);
			//
			iu = _fast_inv(a[k]+az - L[k - 1] * R[k - 1]);
			L[k] = Lk = c[k] * iu;
			iU[k] = iu;



			//x[k] -= Lkm * x[k - 1];
			//x2[k] -= Lkm * x2[k - 1];

			x1[k] -= Lkm * xm1;
			

			x2[k] -= Lkm * xm2;

			xm1 = x1[k];
			xm2 = x2[k];


			Lkm = Lk;

		}




		x1[n - 1] *= iU[n - 1];
		x2[n - 1] *= iU[n - 1];


		f_t xp1, xp2;
		xp1 = x1[n - 1];
		xp2 = x2[n - 1];


		for (auto k = n - 2; k >= 0; k--) {

			register f_t iUk = iU[k], Rk = R[k];

			x1[k] = xp1 = iUk * (x1[k] - Rk * xp1);
			x2[k] = xp2 = iUk * (x2[k] - Rk * xp2);

		}

		return err;
	}


	int factorize_mp0(f_t az, f_t* a, f_t* b, f_t* c, f_t* x1) {

		int err = 0;


		R = b;
		const f_t E = f_t(1);

		//iU[0] = E/(a[0] + az);
		iU[0] = _fast_inv(a[0] + az);

		L[0] = c[0] * iU[0];

		f_t Lkm;
		Lkm = L[0];

		f_t xm1;
		xm1 = x1[0];
		
		for (auto k = 1; k < n; k++) {

			f_t iu;
			f_t Lk;

			//			iu = E / (a[k] + az - L[k - 1] * R[k - 1]);
			//
			iu = _fast_inv(a[k] + az - L[k - 1] * R[k - 1]);
			L[k] = Lk = c[k] * iu;
			iU[k] = iu;



			//x[k] -= Lkm * x[k - 1];
			//x2[k] -= Lkm * x2[k - 1];

			x1[k] -= Lkm * xm1;


			

			xm1 = x1[k];
			


			Lkm = Lk;

		}




		x1[n - 1] *= iU[n - 1];
		


		f_t xp1;
		xp1 = x1[n - 1];
		


		for (auto k = n - 2; k >= 0; k--) {

			register f_t iUk = iU[k], Rk = R[k];

			//
			x1[k] = xp1 = iUk * (x1[k] - Rk * xp1);		
			//x1[k] = xp1 = iUk * (x1[k] - c[k] * xp1);

		}

		return err;
	}


	int factorize2_c(f_t* am, f_t a, f_t b, f_t c, f_t* x1, f_t* x2) {

		int err = 0;
		
		f_t R = b;

		const f_t E = f_t(1);

		iU[0] = E / (a + am[0]);
		

		f_t Lkm;

		Lkm = c * iU[0];

		f_t xm1, xm2;
		xm1 = x1[0];
		xm2 = x2[0];
		for (auto k = 1; k < n; k++) {

			f_t iu;
			f_t Lk;

			//			iu = E / (am[k] + a - Lkm * R);
			//
			iu =_fast_inv(am[k] + a - Lkm * R);
			iU[k] = iu;



		
			x1[k] -= Lkm * xm1;
			xm1 = x1[k];

			x2[k] -= Lkm * xm2;
			xm2 = x2[k];



			//Lkm = Lk;
			  Lkm = c * iu;

		}




		x1[n - 1] *= iU[n - 1];
		x2[n - 1] *= iU[n - 1];


		f_t xp1, xp2;
		xp1 = x1[n - 1];
		xp2 = x2[n - 1];


		for (auto k = n - 2; k >= 0; k--) {

			register f_t iUk = iU[k], Rk = R;

			x1[k] = xp1 = iUk * (x1[k] - Rk * xp1);
			x2[k] = xp2 = iUk * (x2[k] - Rk * xp2);

		}

		return err;
	}




	int factorize2(f_t* a, f_t* am, f_t* b, f_t* c, f_t* x1, f_t* x2) {

		int err = 0;

		//::memcpy(R, b, (n - 1) * sizeof(f_t));
		R = b;
		const f_t E = f_t(1);

		iU[0] = E / (a[0]+ am[0]);
		L[0] = c[0] * iU[0];

		f_t Lkm;
		Lkm = L[0];

		f_t xm1, xm2;
		xm1 = x1[0];
		xm2 = x2[0];
		for (auto k = 1; k < n; k++) {

			f_t iu;
			f_t Lk;

			//iu = E / (a[k]+ am[k] - Lkm * R[k - 1]);
			iu=_fast_inv(a[k] + am[k] - Lkm * R[k - 1]);
			Lk = c[k] * iu;
			



			//x[k] -= Lkm * x[k - 1];
			//x2[k] -= Lkm * x2[k - 1];

			x1[k] -= Lkm * xm1;
			xm1 = x1[k];

			x2[k] -= Lkm * xm2;
			xm2 = x2[k];



			Lkm = Lk;
			L[k] = Lk;
			iU[k] = iu;
		}




		x1[n - 1] *= iU[n - 1];
		x2[n - 1] *= iU[n - 1];


		f_t xp1, xp2;
		xp1 = x1[n - 1];
		xp2 = x2[n - 1];


		for (auto k = n - 2; k >= 0; k--) {

			register f_t iUk = iU[k], Rk = R[k];

			x1[k] = xp1 = iUk * (x1[k] - Rk * xp1);
			x2[k] = xp2 = iUk * (x2[k] - Rk * xp2);

		}

		return err;
	}






	int factorize(f_t* a, f_t* b, f_t* c, int M, f_t** xx) {

		int err;

		::memcpy(R, b, (n - 1) * sizeof(f_t));
		const f_t E = f_t(1);

		iU[0] = E / a[0];
		L[0] = c[0] * iU[0];

		for (auto k = 1; k < n; k++) {

			f_t iu;

			iu = E / (a[k] - L[k - 1] * R[k - 1]);
			L[k] = c[k] * iu;
			iU[k] = iu;

			for (auto m = 0; m < M; m++) {
				f_t* x = xx[m];
				x[k] -= L[k - 1] * x[k - 1];
			}
		}

		M && (err = solveU(M, xx));

		return err;
	}



	int factorize(f_t* a, f_t* b, f_t* c) {

		int err = 0;

		::memcpy(R, b, (n - 1) * sizeof(f_t));
		const f_t E = f_t(1);

		iU[0] = E / a[0];
		L[0] = c[0] * iU[0];

		for (auto k = 1; k < n; k++) {

			f_t iu;

			iu = E / (a[k] - L[k - 1] * R[k - 1]);
			L[k] = c[k] * iu;
			iU[k] = iu;


		}



		return err;
	}


	int solve(int M, f_t** xx) {

		for (auto m = 0; m < M; m++) {

			f_t* x = xx[m];

			for (auto k = 1; k < n; k++) {
				x[k] -= L[k - 1] * x[k - 1];
			}




			x[n - 1] *= iU[n - 1];

			for (auto k = n - 2; k >= 0; k--) {

				x[k] = iU[k] * (x[k] - R[k] * x[k + 1]);
			}

		}

		return 0;
	}

	int solve(f_t* x) {
		return solve(1, &x);
	}

	int solveU(int M, f_t** xx) {

		for (auto m = 0; m < M; m++) {
			f_t* x = xx[m];

			x[n - 1] *= iU[n - 1];

			for (auto k = n - 2; k >= 0; k--) {

				x[k] = iU[k] * (x[k] - R[k] * x[k + 1]);
			}
		}

		return 0;
	}

	int factorize_mp(f_t az, f_t* a, f_t* b, f_t* c, f_t* x1) {

		//return factorize00(a, b, c, 1, &x1);
		return factorize11(az,a, b, c-1, 1, &x1);
	
	}

	int factorize_mp(f_t az, f_t* a, f_t* b, f_t* c, f_t* x1, f_t* x2) {

		//return factorize00(a, b, c, 1, &x1);
		//
		
		//int factorize2(f_t* a, f_t* am, f_t* b, f_t* c, f_t* x1, f_t* x2) {

		f_t* xx[] = { x1,x2 };
		//
		return factorize11(az, a, b, c - 1, 2, xx);
		//return factorize00(az, a, b, c - 1, 2, xx);

	}


	int solve_mp( f_t* x) {


		int err = 0;
		

			f_t Lp, Rp;

			Lp = L[0];


		
				auto xp=x[0] /= Lp;
		
			for (auto k = 1; k < n; ++k) {

				auto ck = cL[k];
				Lp = L[k];
				auto xc = x[k];				
				x[k] = (xc - ck * xp)/ Lp;	
				xp = xc;


			}
			
			auto x1 = x[n - 1];
			for (auto k = n - 2; k >= 0; --k) {

				auto Rk = R[k];				
				x1=(x[k] += Rk * x1);

			}


		

		return err;
	}



	int solve_mp(int M, f_t** xx) {


		int err = 0;
		if (M) {

			f_t Lp, Rp;

			Lp = L[0];
			

			for (auto m = 0; m < M; m++) {
				f_t* x = xx[m];
				x[0] /= Lp;
			}

			for (auto k = 1; k < n; ++k) {

				auto ck = cL[k];

				Lp = L[k];


				for (auto m = 0; m < M; m++) {
					f_t* x = xx[m];
					x[k] = (x[k] - ck * x[k - 1]) / Lp;
				}

				

			}

			for (auto k = n - 2; k >= 0; --k) {

				auto Rk = R[k];
				for (auto m = 0; m < M; m++) {
					f_t* x = xx[m];
					x[k] += Rk * x[k + 1];
				}

			}


		}

		return err;
	}

	int factorize11(f_t bz, f_t* b, f_t* a, f_t* c, int M, f_t** xx) {
		//   c*x[-1] +b*x[0]+a*x[+1]
		int err = 0;
		f_t Lp, Rp  ;

		L[0] = Lp = b[0] + bz;// +c[0]*R[-1]
		R[0] = Rp =-a[0]/Lp;

		for (auto m = 0; m < M; m++) {
			f_t* x = xx[m];
			x[0] /= Lp;
		}

		for (auto k = 1; k < n; ++k) {

			auto ck =cL[k]=c[k];

			L[k] = Lp = (b[k] + bz + ck * Rp);


			for (auto m = 0; m < M; m++) {
				f_t* x = xx[m];
				x[k] = (x[k] - ck * x[k - 1]) / Lp;
			}

			R[k] = Rp = -a[k] /Lp ;

		}

		

		if (M) {

			
			

			for (auto k = n - 2; k >= 0; --k) {

				auto Rk = R[k];
				for (auto m = 0; m < M; m++) {
					f_t* x = xx[m];
					x[k] += Rk* x[k+1];
				}

			}


		}



		return err;
	}

	int factorize00(f_t* a, f_t* b, f_t* c, int M, f_t** xx) {

		int err = 0;

		//::memcpy(R, b, n - 1);
		const f_t E = f_t(1);

		f_t up=U[0] = a[0];
		f_t Lc;
		//L[0] = c[0] / U[0];

		for (auto k = 1; k < n; k++) {

			Lc = c[k] / up;



			

			

			for (auto m = 0; m < M; m++) {
				f_t* x = xx[m];
				x[k] = x[k] - Lc * x[k - 1];
			}

			up= a[k] - Lc* b[k - 1];

			L[k-1] = Lc;
			U[k] = up;

		}



		for (auto m = 0; m < M; m++) {
			f_t* x = xx[m];

			x[n - 1] /= U[n - 1];

			for (auto k = n - 2; k >= 0; k--) {

				x[k] = (x[k] - b[k] * x[k + 1]) / U[k];
			}
		}


		return err;
	}


	int factorize0(f_t* a, f_t* b, f_t* c, int M, f_t** xx) {

		int err = 0;

		//::memcpy(R, b, n - 1);
		const f_t E = f_t(1);

		U[0] = a[0];
		L[0] = c[0] / U[0];

		for (auto k = 1; k < n; k++) {

			U[k] = a[k] - L[k - 1] * b[k - 1];

			L[k] = c[k] / U[k];

			for (auto m = 0; m < M; m++) {
				f_t* x = xx[m];
				x[k] -= L[k - 1] * x[k - 1];
			}
		}



		for (auto m = 0; m < M; m++) {
			f_t* x = xx[m];

			x[n - 1] /= U[n - 1];

			for (auto k = n - 2; k >= 0; k--) {

				x[k] = (x[k] - b[k] * x[k + 1]) / U[k];
			}
		}


		return err;
	}

	inline pool_t& get_pool() {
		return pool;
	}

	f_t* L;
	f_t* cL;
	f_t* U;
	f_t* iU;
	f_t* R;

	pool_t pool;

	int n;
};

/*
template <class F>
struct cycle_shuttle_t {

	typedef F f_t;

	cycle_shuttle_t(int _n) :A(_n-1),n(_n),ne(_n-1) {

		//F = (f_t*)::malloc(n * sizeof(f_t));
		//G = (f_t*)::malloc(n * sizeof(f_t));
		F = A.pool.alloc<f_t>(n);

	};

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

	int factorize2(f_t* a, f_t* am, f_t* b, f_t* c, f_t* x) {

		int err = 0;
		const f_t E = f_t(1);

		clear_set(F, -c[0], -b[ne - 1]);
		//clear_set(G, b[ne],c[ne]);	
		Gb = b[ne];
		Ge = c[ne];

		//b5_t* pF = (b5_t*)F;
		//b5_t* px = (b5_t*)x;

		 //memcpy((void*)F, x, sizeof(f_t) *n );




		//err = (x) ? A.factorize2(a,am, b, c + 1, F, x) : A.factorize2(a,am, b, c + 1, F);

		err = A.factorize2(a, am, b, c + 1, F, x);

		f_t H, D = a[n - 1]+ am[n - 1];

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

	
	int solve(f_t* x) {	

		return A.solve(1,&x) || post_solve(x);			
	}
	

	int post_solve(f_t* x) {
		int err = 0;

		if (x) {

			register f_t xm, Gxn;
			xm = x[n - 1];
			xm *= iH;
			//Gxn = G[0] * x[0]+ G[n-2] * x[n-2];
			Gxn = Gb * x[0] + Ge * x[n - 2];

			x[n - 1]=(xm+= Gxn);

			for (auto k = 0; k < n - 1; k++) {
				x[k] += F[k] * xm;
			}


		}



		return err;
	}


		shuttle_t<f_t> A;
		int n,ne;
		f_t *F,iH,Gb,Ge;
};

*/



