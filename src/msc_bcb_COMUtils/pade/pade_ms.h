#pragma once
#include "./pade_nls.h"

template <template <class > class _COMPLEX = std::complex, class _FLOAT = double, class _MEMPOOL = mem_pool_t<> >
struct pade_ms_t {
	typedef typename pade_nls_base_c_t<_COMPLEX, _FLOAT, _MEMPOOL>  pade_t;
	typedef typename pade_ms_t<_COMPLEX, _FLOAT, _MEMPOOL> self_t;
	typedef typename self_t  *pself_t;
	typedef typename pade_t::float_t float_t;
	typedef typename pade_t::complex_t complex_t;
	typedef typename _COMPLEX<double> complex64_t;
	typedef _MEMPOOL pool_t;

	
	pade_ms_t(size_t _N, float_t _g = 0, complex_t _alpha = complex_t())
		:X_nls(_N),Y_nls(_N),N(_N),
		g(_g), alpha(_alpha), alpha_prev(_alpha), omagnus(0),ncorr(0)	{


		std::tie(vb, ve) = pool.alloc2(_N, &vv);
		std::fill(vb, ve, complex_t());	

		std::tie(xb, xe) = pool.alloc2(_N, &xx);
		std::fill(xb, xe, complex_t());

		std::tie(yb, ye) = pool.alloc2(_N, &yy);
		std::fill(yb, ye, complex_t());

		XY_nls[0] = &X_nls;
		XY_nls[1] = &Y_nls;

		xy[0] = xx;
		xy[1] = yy;

		set_NaN(alpha_prev);
		set_NaN(g_prev);
	}



	//pade_nls_base_c_t<_COMPLEX, _FLOAT, _MEMPOOL>(_N)

	//pade_nls_base_c_t<_COMPLEX, _FLOAT, _MEMPOOL> X_nls, Y_nls;

	void check_modify() {

		fgmodify = check_reset(g, g_prev) || g;
		fmodify = check_reset(alpha, alpha_prev) || fgmodify;
	
	}

	void preset(void*) {};

	pade_ms_t& reset(double dt = 1.0, float_t _g = 0, complex_t _alpha = complex_t(), int _nt = 1, std::tuple<int, int> nm = { 8,8 }) {
		int _n, _m;
		g = _g;
		alpha = _alpha;
		std::tie(_n, _m) = nm;
		return reset(_n, _m, dt, _nt);
	}

	pade_ms_t& reset(int _n, int _m, double _dt = 1.0, int _nt = 1) {

		if (_nt < 1) _nt = 1;

		if (_m < 0) _m = _n;

		//dt = dt / (nt = _nt);
		n = _n;
		m = _m;
		dt = _dt;
		X_nls.reset(_n, _m, dt);
		Y_nls.reset(_n, _m, dt);


		set_NaN(alpha_prev);
		set_NaN(g_prev);

		return *this;
	}

	

	template<class F>
	void load(F* fx, F* fy ) {	
		std::copy(fx, fx + this->N, xb);
		std::copy(fy, fy + this->N, yb);
	}

	template<class F>
	void load(F* f) {
		load(f, f + this->N);
	}

	template<class F>
	void save(F* fx, F* fy) {
		std::copy(xb, xe, fx);
		std::copy(yb, ye, fy);
	}
	template<class F>
	void save(F* f) {		
		save(f, f + this->N);
	}


	inline void make(int b, int e, complex_t* pv, complex_t diag ,int pp=false) {
		if(pv)
			for (auto k = b; k < e; k++) {
				XY_nls[k]->step(pv, diag, xy[k], pp);
			}
		else 
			for (auto k = b; k < e; k++) {
				XY_nls[k]->step(xy[k], pp);
			}

	}

	void set_magnus_NL() {

		auto g = this->g;
		auto x = xx;
		auto y = yy;

		auto fx = x[0], fxp = x[1], fxm = x[N - 1];
		auto fy = y[0], fyp = y[1], fym = y[N - 1];

		int sgn_magnus = (omagnus > 0) ? 1 : -1;

		//auto g1 = -float_t(sgn_magnus)*g * dt;// / 2.;
		auto g1 = -g * dt;// / 2.;



		for (auto k = 0; k < N - 1; k++) {

			
			auto  v0 = g * (abs2(fx) + abs2(fy));

			


			fxp = x[k + 1];
			fyp = y[k + 1];

			

			auto v1 = g1 * (mul_imag_c(fx, fxm + fxp) + mul_imag_c(fy, fym + fyp));


			auto vm = v0 + v1;
			

			vv[k].real(vm);
			


			fxm = fx;
			fx = fxp;

			fym = fy;
			fy = fyp;

		}

		//for (auto k = N - 1; k < N; k++)
		{


			auto  v0 = g * (abs2(fx) + abs2(fy));



			fxp = x[0];
			fyp = y[0];

			auto v1 = g1 * (mul_imag_c(fx, fxm + fxp) + mul_imag_c(fy, fym + fyp));


			auto vm = v0 + v1;

			vv[N-1].real(vm);		

		}


	}


	void set_NL() {

		auto g = this->g;


		if (g) {

			///*
			if (omagnus) {
				set_magnus_NL();
			}
			else
			//*/
				for (auto x = xb, y = yb, v = vb; x != xe; ++x,++y, ++v) {
					
					v->real(g*(abs2(*x)+ abs2(*y)));
				}
		}
		else if (fgmodify) {

			std::fill(vb, ve, complex_t());
		}


	}

	int step(int pp = 0) {

		check_modify();

		set_NL();
		auto diag = alpha - complex_t(2);


		int ppi = pp&1;

		complex_t * pv = (fmodify) ? vv : nullptr;

		if (pp & 2) {
			tbb::parallel_for(tbb::blocked_range<int>(0, 2),
				[&](const tbb::blocked_range<int>& r) {
					make(r.begin(), r.end(), pv,diag, ppi);
				}, afp);
		}
		else make(0,2, pv, diag, ppi);



		/*
		if (fmodify) {
			static_cast<base_t*>(this)->step(vv, diag, xx, pp);			
		}
		else {
			static_cast<base_t*>(this)->step(xx, pp);
		}
		*/
		

		return 0;
	}
	







	pade_t X_nls, Y_nls;
	pade_t* XY_nls[2];
	//pade_t nls[2];
	size_t N;
	int nt, ncorr;
	int omagnus;
	int n, m;
	double dt;
	double w;
	float_t g, g_prev;
	complex_t * xy[2];
	complex_t *xb, *xe, *xx;
	complex_t *yb, *ye, *yy;
	complex_t *vb, *ve, *vv;
	complex_t alpha, alpha_prev, alpha_nl;

	bool fmodify, fgmodify;

	pool_t pool;
	tbb::affinity_partitioner afp;

};