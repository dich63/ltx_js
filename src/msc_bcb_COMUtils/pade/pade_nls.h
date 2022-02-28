#pragma once
// #include "pade_nls.h" 
//
#include<complex>

#include <algorithm>
//#include "pade/complex-intel.h"
//#include "pade/pade_static.h"
#include "pade/exp_pade_poles_res.h"
#include "pade/accum_static.h"

#include "shuttle/rsc_cycle_shuttle.h"
#include "shuttle/rsc_shuttle.h"
#include "tbb/parallel_for.h"




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



//template <class _MEMPOOL = mem_pool_t<> >
template <template <class > class _COMPLEX=std::complex, class _FLOAT=double,  class _MEMPOOL = mem_pool_t<> >
struct pade_nls_base_c_t{
	
	typedef _FLOAT float_t;
	typedef typename _COMPLEX<_FLOAT> complex_t;

	typedef typename accum_static_t<complex_t >::proc_t  accum_proc_t;
	typedef _MEMPOOL pool_t;


	enum {
		NUMM= pade_exp_poles_res_t<float_t>::max_num
	};

	static constexpr complex_t ie = complex_t(0, 1);


	


	//template <class complex_t>
	struct poles_solver_t {
		
		
		

		
		poles_solver_t(size_t _N, complex_t _polus, complex_t _res):N(_N),lu(_N),polus(_polus),res(-_res){

			xz = pool.alloc(N,xz);
		}
		
		inline complex_t* loadxz(complex_t* x0) {
			for (auto k = 0; k < N; ++k) {
				xz[k] = res * x0[k];
			}

			return xz;
		}
		
		inline int factorize2(complex_t* am, complex_t a, complex_t* x0) {
			int err;
			//::memcpy(xz, x0, sizeof(complex_t) * N);
			loadxz(x0);
			complex_t ap = a;
			ap += polus;
			auto xzN = (bN_t<complex_t,16>*)xz;
			err=lu.factorize2(am, ap , xz);
			return err;
		}

		inline int step(complex_t* x0) {
			//::memcpy(xz, x0, sizeof(complex_t) * N);
			loadxz(x0);
			return lu.solve( xz);			
		}

		size_t N;
		complex_t* xz;
		rsc_cycle_shuttle_t<complex_t, pool_t> lu;
		complex_t polus, res;
		pool_t pool;
		
	};


	inline void make(int b, int e, complex_t* am, complex_t a, complex_t* x0) {
		for (auto k = b; k < e; k++) {
			solvers[k].factorize2(am,a,x0);
		}
	}

	inline void make(int b, int e,complex_t* x0) {
		for (auto k = b; k < e; k++) {
			
			solvers[k].step(x0);
		}
	}



	inline void makeall(complex_t* am, complex_t a, complex_t* x0, bool pp = 0) {

		if (pp) {
			tbb::parallel_for(tbb::blocked_range<int>(0, m),
				[&](const tbb::blocked_range<int>& r) {
					make(r.begin(), r.end(), am,a,x0);
				},afp);
		}
		else make(0, m, am, a, x0);
	
	}


	inline void makeall(complex_t* x0, bool pp = 0) {

		if (pp) {
			tbb::parallel_for(tbb::blocked_range<int>(0, m),
				[&](const tbb::blocked_range<int>& r) {
					make(r.begin(), r.end(), x0);
				}, afp);
		}
		else make(0, m,x0);

	}

	inline void step(complex_t* am, complex_t a, complex_t* x0, bool pp = 0) {
		makeall(am, a, x0, pp);
		accum(N, x0, xz, m);
	}

	inline void step(complex_t* x0, bool pp = 0) {
		makeall(x0,pp);
		accum(N, x0, xz, m);
	}



	pade_nls_base_c_t(size_t _N) :N(_N), n(-1), m(-1), dt(0), accum(nullptr){};

	pade_nls_base_c_t& reset(int _n,int _m, double _dt=1.0) {

		complex_t poles[NUMM], res[NUMM];

		dt = _dt;


		pade_exp_poles_res_t<float_t>::poles_res(n = _n, m = _m, poles, res);
		

		accum = accum_static_t<complex_t >::get(m,m==n);
		  
		
		

		auto jf =  ie / float_t(dt);

		solvers.init(m, [&](size_t k) {

			auto p = new poles_solver_t(N, jf * poles[k], jf * res[k]);
			xz[k] = p->xz;
			return p;
			});

		return *this;
	}


	
	accum_proc_t accum;
	complex_t* xz[NUMM];
	tbb::affinity_partitioner afp;
	pool_t pool;
	c_array_t<poles_solver_t> solvers;

	double dt;	
	size_t N;
	int n, m,nt;

};





template <template <class > class _COMPLEX = std::complex, class _FLOAT = double, class _MEMPOOL = mem_pool_t<> >
struct pade_nls_t :pade_nls_base_c_t<_COMPLEX, _FLOAT, _MEMPOOL> {
	typedef typename pade_nls_base_c_t<_COMPLEX, _FLOAT, _MEMPOOL> base_t;
	typedef typename pade_nls_t<_COMPLEX, _FLOAT, _MEMPOOL> self_t;
	typedef typename self_t  *pself_t;
	typedef typename base_t::float_t float_t;
	typedef typename base_t::complex_t complex_t;
	typedef typename _COMPLEX<double> complex64_t;

	


	pade_nls_t(size_t _N, float_t _g=0, complex_t _alpha = complex_t())
		:pade_nls_base_c_t<_COMPLEX, _FLOAT, _MEMPOOL>(_N)
		,g(_g), alpha(_alpha), alpha_prev(_alpha), omagnus(0), ncorr(0){

		//auto base = (base_t*)this;

		std::tie(vb,ve)= this->pool.alloc2(_N, &vv);
		std::fill(vb, ve, complex_t());

		std::tie(vLb, vLe) = this->pool.alloc2(_N, &vvL);
		std::fill(vLb, vLe, complex_t());

			

		std::tie(xb, xe) = this->pool.alloc2(_N, &xx);
		std::fill(xb, xe, complex_t());	
		

	}

	template<class F>
	void load(F* f) {
		std::copy(f,f+this->N,xb);
	}

	template<class F>
	void save(F* f) {
		std::copy(xb, xe, f);
	}


	void post_phase() {
		//if (!(omagnus && (_abs(omagnus) > 1)))
			//return;

		//auto pvL = pN_t<16>(vvL);
		

		auto g = this->g;

		//int sgn = (omagnus > 0) ? 1 : -1;
		int sgn = 1;
		float_t dz = this->dt;

		float_t sm = float_t(sgn)* dz / float_t(3.0);

		for (auto k = 0; k < N; k++) {

			auto vL = vvL[k].real();
			auto  vnew = g * abs2(xx[k]);
			double dv2 = sm * (vnew - vL);

			auto cs = std::cos(dv2), ss = std::sin(dv2);

			xx[k] *= complex_t(cs, ss);
		}

	}


	void set_magnus_NL() {

		auto g = this->g;
		auto x = xx;
		auto f=x[0],fp=x[1],fm=x[N-1];

		int sgn_magnus = (omagnus > 0) ? 1 : -1;

		//auto g1 = -float_t(sgn_magnus)*g * dt;// / 2.;
		auto g1 = -g * dt;// / 2.;



		for (auto k = 0; k < N - 1; k++) {
			
			auto rx = f.real();
			auto ix = f.imag();
			auto  v0 = g * (rx*rx + ix * ix);

			auto fc = std::conj(f);


			fp = x[k + 1];

			auto tmp = fc * (fm + fp);

			auto v1 = g1*(tmp).imag();

			auto vm = v0 + v1;
			auto vL = vm + v1;

			vv[k].real(vm);
			vvL[k].real(vL);


			fm = f;
			f = fp;
			
		}		

		//for (auto k = N - 1; k < N; k++)
		{
			

			auto rx = f.real();
			auto ix = f.imag();
			auto  v0 = g * (rx*rx + ix * ix);

			auto fc = std::conj(f);

			fp = x[0];

			auto v1 = g1 * (fc*(fm + fp)).imag();

			auto vm = v0 + v1;
			auto vL = vm + v1;

			vv[N - 1].real(vm);
			vvL[N - 1].real(vL);

				
		}

	
	}


	void set_magnus_NL2() {

		auto g = this->g;
		auto x = xx;


		auto fx = x[0], fxp = x[1], fxm = x[N - 1], fxp2 = x[2], fxm2 = x[N - 2];


		int sgn_magnus = (omagnus > 0) ? 1 : -1;

		//auto g1 = -float_t(sgn_magnus)*g * dt;// / 2.;
		auto g1 = -g * dt;// / 2.;
		auto g2 = g1 * dt/ 6/2;
		//auto g2 = g1 * dt / 3;

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
			vv[k] = vm;
			

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


	}



	void set_NL() {		

		auto g = this->g;
		
		
		if (g) {

			if (omagnus) {
				if (_abs(omagnus) > 1)
					set_magnus_NL2();
				else {
					set_magnus_NL();					
				}
			}
			else 
				for (auto x = xb, v = vb; x != xe; ++x, ++v) {
					
					/*
					auto rx = x->real();
					auto ix = x->imag();
					v->real(g*(rx*rx + ix * ix));
					*/
					*v=(g*abs2(*x));
				}
		}
		else if (fgmodify) {

			std::fill(vb, ve, complex_t());
		}


	}

	

	pade_nls_t& reset(double dt = 1.0, float_t _g = 0, complex_t _alpha = complex_t(), int _nt = 1, std::tuple<int, int> nm = {8,8}) {
		int _n, _m;
		g = _g;
		alpha = _alpha;
		std::tie(_n, _m) = nm;
		return reset(_n, _m, dt, _nt);
	}

	void preset(void*) {};
	pade_nls_t& reset(int _n, int _m, double _dt = 1.0,int _nt = 1) {
		
		if (_nt < 1) _nt = 1;

		if (_m < 0) _m = _n;

		dt = _dt / (nt = _nt);

		

		static_cast<base_t*>(this)->reset(_n, _m, dt);


		//set_NaN(alpha_prev);
		//set_NaN(g_prev);

		g_prev = std::numeric_limits<double>::infinity();
		alpha_prev = { g_prev ,g_prev };
	
		return *this;
	}

	

	
	void check_modify() {

		fgmodify = check_reset(g, g_prev) || g;
		fmodify = check_reset(alpha,alpha_prev) || fgmodify;

		/*
		fgmodify = g || (g != g_prev);
		//fgmodify =  (g != g_prev);
		fmodify = fgmodify || (alpha != alpha_prev);

		
			alpha_prev = alpha;
			g_prev = g;		
			*/
		
	}




	int step(bool pp=false) {

		check_modify();

		set_NL();
		auto diag= alpha - complex_t(2);

		

		if(fmodify){
			static_cast<base_t*>(this)->step(vv, diag,xx,pp);
			if (omagnus==-1) 				  post_phase();
		}
		else 
			static_cast<base_t*>(this)->step(xx, pp);

		for (auto k = 1; k < nt; ++k) {
			static_cast<base_t*>(this)->step(xx, pp);
		}	

		
		return 0;
	}




	int nt;
	int omagnus;
	float_t g,g_prev;
	double w;
	complex_t *xb,*xe,*xx;
	complex_t *vb,*ve,*vv,*vvL, *vLb, *vLe;
	complex_t alpha, alpha_prev,alpha_nl;
	
	bool fmodify,fgmodify;
	int ncorr;
};
