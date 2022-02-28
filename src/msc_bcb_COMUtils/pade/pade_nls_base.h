#pragma once
// #include "pade_nls.h" 
//
#include<complex>

#include <algorithm>
//#include "pade/complex-intel.h"
//#include "pade/pade_static.h"
#include "pade/exp_pade_poles_res.h"
#include "pade/accum_static.h"
//#include "pade/accum_static.h"
//#include "pade/magnus-utils.h"

#include "shuttle/rsc_cycle_shuttle.h"
#include "shuttle/rsc_shuttle.h"

#include "shuttle/cycle_shuttle.h"
#include "shuttle/shuttle.h"

#include "shuttle/ls_corr.h"

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
struct pade_nls_base_t{
	
	typedef _FLOAT float_t;
	typedef typename _COMPLEX<_FLOAT> complex_t;

	typedef typename accum_static_t<complex_t >::proc_t  accum_proc_t;
	typedef _MEMPOOL pool_t;

	typedef cycle_shuttle_t<complex_t, pool_t> LU_solver_t;
	typedef typename LU_solver_t::triplet_t triplet_t;

	typedef rsc_cycle_shuttle_t<complex_t, pool_t> rsc_LU_solver_t;
	


	enum {
		NUMM= pade_exp_poles_res_t<float_t>::max_num
	};

	static constexpr complex_t ie = complex_t(0, 1);


	


	//template <class complex_t>
	struct poles_solver_t {
		
		
		

		
		poles_solver_t(size_t _N, complex_t _polus, complex_t _res,int _ncorr=0)
			:N(_N),lu(_N)
			,luc(_N)
			,polus(_polus),res(-_res), ncorr(_ncorr){

			xz = pool.alloc(N,xz);
			if (ncorr > 0) {
				xb = pool.alloc(N, xb);
				xbuff = pool.alloc(N, xbuff);
				ls_corr.init(N, xb, xbuff);

			}

		}
		
		inline complex_t* loadxz(complex_t* x0) {
			
			for (auto k = 0; k < N; ++k) {
				xz[k] = res * x0[k];
			}
			
			return xz;
		}
		
		inline int factorize_step(const triplet_t& trt, complex_t dia_c, complex_t* x0) {
			int err;
			//::memcpy(xz, x0, sizeof(complex_t) * N);
			loadxz(x0);

			
			
			

			auto xzN = (bN_t<complex_t,16>*)xz;	
			auto dia_c_polus = dia_c + polus;



			ls_corr.set_b(xz, ncorr);

			err=lu.factorize_mp(dia_c_polus,trt, xz);


			if (ncorr > 0) {

				auto subp = [&](auto x, auto c) {
					//static double subAx(size_t N, f_t az, f_t * a, f_t * b, f_t * c, f_t * xx, f_t * y) 
					return subAx(N, dia_c_polus, trt.a, trt.b, trt.c, x, c);
				};

				ls_corr.make(subp, [&](auto x) { return lu.solve_mp(x); }, xz, ncorr);

			}


			//err = luc.factorize2(trt.a, dia_c + polus,  xz);

			return err;
		}

		inline int step(complex_t* x0) {
			//::memcpy(xz, x0, sizeof(complex_t) * N);
			loadxz(x0);
			return lu.solve_mp( xz);			
		}

		size_t N;
		complex_t* xz,*xb,*xbuff;
		LU_solver_t lu;
		
		rsc_LU_solver_t luc;
		ls_corr_t<complex_t> ls_corr;

		complex_t polus, res;
		pool_t pool;
		int ncorr;
		
		
	};


	inline void make(int b, int e,const triplet_t& trt, complex_t dia_c, complex_t* x0) {
		for (auto k = b; k < e; k++) {
			solvers[k].factorize_step(trt, dia_c,x0);
		}
	}

	inline void make(int b, int e,complex_t* x0) {
		for (auto k = b; k < e; k++) {
			
			solvers[k].step(x0);
		}
	}



	inline void makeall(const triplet_t& trt, complex_t dia_c, complex_t* x0, bool pp = 0) {

		if (pp) {
			tbb::parallel_for(tbb::blocked_range<int>(0, m),
				[&](const tbb::blocked_range<int>& r) {
					make(r.begin(), r.end(),trt,dia_c,x0);
				},afp);
		}
		else make(0, m,trt, dia_c,  x0);
	
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

	inline void step(const triplet_t& trt, complex_t dia_c, complex_t* x0, bool pp = 0) {
		makeall(trt, dia_c, x0, pp);
		accum(N, x0, xz, m);
	}

	inline void step(complex_t* x0, bool pp = 0) {
		makeall(x0, pp);
		accum(N, x0, xz, m);
	}



	pade_nls_base_t(size_t _N) :N(_N), n(-1), m(-1), dt(0), accum(nullptr){};

	pade_nls_base_t& reset(int _n,int _m, double _dt=1.0,int _ncorr=0) {

		complex_t poles[NUMM], res[NUMM];

		dt = _dt;


		pade_exp_poles_res_t<float_t>::poles_res(n = _n, m = _m, poles, res);
		

		accum = accum_static_t<complex_t >::get(m,m==n);
		  
		ncorr = _ncorr;
		

		auto jf =  ie / float_t(dt);

		solvers.init(m, [&](size_t k) {

			auto p = new poles_solver_t(N, jf * poles[k], jf * res[k], ncorr);
			xz[k] = p->xz;
			
			return p;
			});

		return *this;
	}


	
	accum_proc_t accum;
	complex_t* xz[NUMM];
	tbb::affinity_partitioner afp;
	mem_pool_t<> pool;
	c_array_t<poles_solver_t> solvers;

	double dt;	
	size_t N;
	int n, m,nt,ncorr;

};






/*

template <template <class > class _COMPLEX = std::complex, class _FLOAT = double, class _MEMPOOL = mem_pool_t<> >
struct pade_magnus_nls_t :pade_nls_base_t<_COMPLEX, _FLOAT, _MEMPOOL> {
	typedef typename pade_nls_base_t<_COMPLEX, _FLOAT, _MEMPOOL> base_t;
	typedef typename pade_nls_t<_COMPLEX, _FLOAT, _MEMPOOL> self_t;
	typedef typename self_t  *pself_t;
	typedef typename base_t::float_t float_t;
	typedef typename base_t::complex_t complex_t;
	typedef typename _COMPLEX<double> complex64_t;

	


	pade_magnus_nls_t(size_t _N, float_t _g=0, complex_t _alpha = complex_t())
		:pade_nls_base_t<_COMPLEX, _FLOAT, _MEMPOOL>(_N)
		,g(_g), alpha(_alpha), alpha_prev(_alpha), omagnus(0){


		std::tie(vb,ve)=this->pool.alloc2(_N, &vv);
		std::fill(vb, ve, complex_t());

		std::tie(vLb, vLe) = this->pool.alloc2(_N, &vvL);
		std::fill(vLb, vLe, complex_t());

			

		std::tie(xb, xe) =this->pool.alloc2(_N, &xx);
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
		if (!(omagnus && (_abs(omagnus) > 1)))
			return;


		auto g = this->g;

		int sgn = (omagnus > 0) ? 1 : -1;

		float_t sm = float_t(sgn)* this->dt / float_t(3.0);

		for (auto k = 0; k < N; k++) {

			auto vL = vvL[k].real();
			auto  vnew = g * abs2(xx[k]);
			double dv2 = -sm * (vnew - vL);

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

	void set_NL() {		

		auto g = this->g;
		
		
		if (g) {

			if (omagnus) {
				set_magnus_NL();
			}
			else 
				for (auto x = xb, v = vb; x != xe; ++x, ++v) {
					
					
					v->real(g*abs2(*x));
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

	pade_nls_t& reset(int _n, int _m, double _dt = 1.0,int _nt = 1) {
		
		if (_nt < 1) _nt = 1;

		if (_m < 0) _m = _n;

		dt = _dt / (nt = _nt);

		

		static_cast<base_t*>(this)->reset(_n, _m, dt);

		set_NaN(alpha_prev);
		set_NaN(g_prev);
	
		return *this;
	}

	

	
	void check_modify() {

		fgmodify = check_reset(g, g_prev) || g;
		fmodify = check_reset(alpha,alpha_prev) || fgmodify;

			
	}




	int step(bool pp=false) {

		check_modify();

		set_NL();
		auto diag= alpha - complex_t(2);

		

		if(fmodify){
			static_cast<base_t*>(this)->step(vv, diag,xx,pp);
			post_phase();
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
	complex_t *xb,*xe,*xx;
	complex_t *vb,*ve,*vv,*vvL, *vLb, *vLe;
	complex_t alpha, alpha_prev;
	
	bool fmodify,fgmodify;
};
*/