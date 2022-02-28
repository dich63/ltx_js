#pragma once 
//#include "shuttle/shuttle.h"
//#include "pade/complex-intel.h"


#include "./utils.h"

template <class F>
inline F _fast_back(F a) {
	return F(1) / F(a);
}

template <class F>
inline F _fast_back2(F a) {
	return F(1) / F(a);
}


/*
template <class C>
inline C _fast_back(C a,typename  C::value_type* p=(C::value_type*)nullptr) {

	auto r = a.real();
	auto i = a.imag();
	if ((r < 0 ? -r : +r) > (i < 0 ? -i : +i)) {
		auto b = 1 / r, eps = i * b;
		auto f = b / (eps * eps + 1);
		return C(f, -eps * f);
	}
	else {
		auto b = 1 / i, eps = r * b;
		auto f = b / (eps * eps + 1);
		return C(eps * f, -f);
	}

}

*/

template <class F>
inline std::complex<F> _fast_back(std::complex<F> a) {
	
	F r = a.real();
	F i = a.imag();
	if ((r < 0 ? -r : +r) > (i < 0 ? -i : +i)) {
		auto b = 1 / r, eps = i *b;
		auto f = b / (eps * eps +1);
		return std::complex<F>(f, -eps * f);
	}
	else {
		auto b = 1 / i, eps = r * b;		
		auto f = b / (eps * eps + 1);
		return std::complex<F>(eps * f, - f);
	}

}


template <class F>
inline std::complex<F> _fast_back2(const std::complex<F>& a) {

	F r = a.real();
	F i = a.imag();

	F br = r * r + i * i;
	//br = F(1) / br;
	return { r/br,- i/br };

}



template <class _F,class _MEMPOOL =mem_pool_t<> >
struct rsc_shuttle_t {

	typedef typename _F f_t;
	typedef typename _MEMPOOL pool_t;


	rsc_shuttle_t(int _n) :n(_n) {

		 pool.alloc(n,&iU);
		//iU = pool.alloc< f_t >(n);
		//U = pool.alloc<f_t>(n);
		//L = pool.alloc<f_t>(n);

		//R = (f_t*)::malloc(n * sizeof(f_t));

	};

	

	~rsc_shuttle_t() {

		//::free(R);

		/*
		::free(L);
		::free(U);
		::free(iU);
		*/

	}


	

	

	int factorize2(f_t* am, f_t a, f_t* x1, f_t* x2) {

		int err = 0;
		
		

		const f_t E = f_t(1);

		//		iU[0] = E / (a + am[0]);
		//
		iU[0] = _fast_inv(a + am[0]);

		f_t Lkm;

		Lkm =  iU[0];

		f_t xm1, xm2;
		xm1 = x1[0];
		xm2 = x2[0];

		auto px1 = (bN_t<f_t, 16>*)x1;
		auto px2 = (bN_t<f_t, 16>*)x2;
		auto piU = (bN_t<f_t, 16>*)iU;
		f_t att = a;

		for (auto k = 1; k < n; k++) {

			f_t iu;
			

			//			iu = E / (am[k] + a - Lkm);
			//			iu = _fast_back2((am[k] + a - Lkm));
			//
			iu = _fast_inv((am[k] + a - Lkm));

			iU[k] = iu;



			//auto Lkm1 = - Lkm;
			//Lkm1 += 1;

			//x1[k] = (xm1 *= Lkm1);
			//x2[k] = (xm2 *= Lkm1);

			x1[k] -= Lkm * xm1;	
			//x1[k]=( xm1 -= Lkm * xm1 );
			x2[k] -= Lkm * xm2;
			
			xm1 = x1[k];
			xm2 = x2[k];
						//x2[k] = (xm2 -= Lkm * xm2);


			
			  Lkm = iu;

		}




		x1[n - 1] *= iU[n - 1];
		x2[n - 1] *= iU[n - 1];


		f_t xp1, xp2;
		xp1 = x1[n - 1];
		xp2 = x2[n - 1];


		for (auto k = n - 2; k >= 0; k--) {

			register f_t iUk = iU[k];

			x1[k] = xp1 = iUk * (x1[k] - xp1);
			x2[k] = xp2 = iUk * (x2[k] - xp2);

		}

		return err;
	}


	int factorize1(f_t* am, f_t a, f_t* x1) {

		int err = 0;



		const f_t E = f_t(1);

		//		iU[0] = E / (a + am[0]);
		//
		iU[0] = _fast_inv(a + am[0]);

		f_t Lkm;

		Lkm = iU[0];

		f_t xm1, xm2;
		
		xm1 = x1[0];

		auto px1 = (bN_t<f_t, 16>*)x1;
		
		auto piU = (bN_t<f_t, 16>*)iU;
		f_t att = a;

		for (auto k = 1; k < n; k++) {

			f_t iu;


			//			iu = E / (am[k] + a - Lkm);
			//			iu = _fast_back2((am[k] + a - Lkm));
			//
			iu = _fast_inv((am[k] + a - Lkm));

			iU[k] = iu;



			//auto Lkm1 = - Lkm;
			//Lkm1 += 1;

			//x1[k] = (xm1 *= Lkm1);
			//x2[k] = (xm2 *= Lkm1);

			x1[k] -= Lkm * xm1;
			//x1[k]=( xm1 -= Lkm * xm1 );
			

			xm1 = x1[k];
			
			//x2[k] = (xm2 -= Lkm * xm2);



			Lkm = iu;

		}




		x1[n - 1] *= iU[n - 1];
		


		f_t xp1, xp2;
		xp1 = x1[n - 1];
		


		for (auto k = n - 2; k >= 0; k--) {

			register f_t iUk = iU[k];

			x1[k] = xp1 = iUk * (x1[k] - xp1);
			

		}

		return err;
	}


	int factorize2a(f_t* am, f_t a, f_t* x1, f_t* x2) {

		int err = 0;

		U = iU;

		const f_t E = f_t(1);

		//		iU[0] = E / (a + am[0]);
		//
		

		f_t Ukm;
		Ukm = (a + am[0]);
        U[0]= Ukm;

		f_t xm1, xm2;
		xm1 = x1[0];
		xm2 = x2[0];

		auto px1 = (bN_t<f_t, 16>*)x1;
		auto px2 = (bN_t<f_t, 16>*)x2;
		auto piU = (bN_t<f_t, 16>*)iU;
		f_t att = a;

		for (auto k = 1; k < n; k++) {

			f_t u;


			//			iu = E / (am[k] + a - Lkm);
			//			iu = _fast_back2((am[k] + a - Lkm));
			//
			

			



			//auto Lkm1 = - Lkm;
			//Lkm1 += 1;

			//x1[k] = (xm1 *= Lkm1);
			//x2[k] = (xm2 *= Lkm1);

			x1[k] -= xm1/Ukm;
			//x1[k]=( xm1 -= Lkm * xm1 );
			x2[k] -=  xm2 / Ukm;

			xm1 = x1[k];
			xm2 = x2[k];
			//x2[k] = (xm2 -= Lkm * xm2);

			u = ((am[k] + a - E / Ukm));

			U[k] = Ukm = u;

		}




		x1[n - 1] /= U[n - 1];
		x2[n - 1] /= U[n - 1];


		f_t xp1, xp2;
		xp1 = x1[n - 1];
		xp2 = x2[n - 1];


		for (auto k = n - 2; k >= 0; k--) {

			register f_t Uk = U[k];

			x1[k] = xp1 =  (x1[k] - xp1)/ Uk;
			x2[k] = xp2 = (x2[k] - xp2) / Uk;

		}

		return err;
	}



	int solve(f_t* x) {

		for (auto k = 0; k < n-1; k++) {
			x[k+1] -= iU[k] * x[k];
		}


		//====================================
		x[n - 1] *= iU[n - 1];

		register auto xp = x[n - 1];

		for (auto k = n - 2; k >= 0; k--) {	
			x[k] = xp = iU[k] * (x[k] - xp);
		}

		return 0;
	}





	/*
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
	*/


	
	f_t* iU,*U;
	

	pool_t pool;

	int n;
};

