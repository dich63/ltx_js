#pragma  once

#include "trimatrix/thomas3.h"
#include "pade/accum_static2.h"
#include "pade/exp_pade_poles_res_float64.h"
//#include "pp_tbb/pp2.h"

template <class F>
struct matrix_item2_t {
	typedef F f_t;

	f_t v;
	f_t xy[2];
	//f_t notused;	
};


template < class _COMPLEX >
struct pade_data_t {

	typedef typename _COMPLEX complex_t;

	typedef typename matrix_item2_t<complex_t> mi_t;
	typedef tri_matrix_item2_t<complex_t> ti_t;

	typedef typename void(*proc_t)(size_t N, mi_t*, ti_t**, int m);


	enum {
		NUMM = 21
	};



	int L, M;

	proc_t accum;
	complex_t poles[NUMM], res[NUMM];

};


template < class _COMPLEX >
struct pade_exp_handle_t {

	typedef  _COMPLEX complex_t;
	typedef typename pade_data_t<complex_t>  pade_t;
	typedef typename pade_t::ti_t  ti_t;
	typedef typename pade_t::mi_t  mi_t;

	enum {
		NUMM = pade_t::NUMM
	};

	pade_t pade;
	size_t N;
	ti_t* pptmi[NUMM];
	complex_t mmo; // 1i/dz
	int pp;
};



template < class PadeHandle>
inline void reset_pade_handle(PadeHandle* ph, size_t N, double dt = 1, int M = 2, int pp = 1);
template <class F, class ParFor2>
int exp_pade_step2(pade_exp_handle_t<F>* handle, matrix_item2_t<F>* pmmi, ParFor2&& parfor2);

template <class F>
int exp_pade_step2(pade_exp_handle_t<F>* handle, matrix_item2_t<F>* pmmi);




template < class PadeData>
inline void set_accum_proc(PadeData& ph) {

	ph.accum = accum_static_diag2_t<PadeData::mi_t, PadeData::ti_t>::get(ph.M);
}


template < class PadeHandle>
inline void reset_pade_handle(PadeHandle* ph,size_t N,double dt, int M,int pp) {

	typedef  PadeHandle::complex_t complex_t;

	ph->pade.M = M;
	pade_exp_poles_res_float64_t<>::poles_res(M, M, ph->pade.poles, ph->pade.res);
	ph->pade.accum = accum_static_diag2_t<PadeHandle::mi_t, PadeHandle::ti_t>::get(M);

	ph->N = N;
	ph->mmo = -complex_t(0.0, 1.0) / dt;
	ph->pp = pp;
	set_accum_proc(ph->pade);
}



template < class PadeData>
inline auto get_accum_proc(PadeData& ph) {
	if (!ph.accum) {
		set_accum_proc(ph);
	}
	return ph.accum;
}


/*
template <class F>
int pade_polus_res2_tic(size_t N, F polus, F res, matrix_item2_t<F>* pmmi, tri_matrix_item2_t<F>* ptmi) {

	for (auto n = 0; n < N; n++) {
		auto& mmi = pmmi[n];
		auto& tmi = ptmi[n];

		tmi.a = mmi.v - polus;
		tmi.x[0] = res * mmi.xy[0];
		tmi.x[1] = res * mmi.xy[1];
		tmi.x[2] = F();
	}

	return thomas_2x_cycle(N, ptmi);
};
*/

template <class It>
struct fake_parfor_t {
	template <class Func>
	inline int operator()(bool pp, It b, It e, Func itemfun, size_t grain = 1) {
		int err;
		for (auto i = b; i != e; ++i) {
			if (err = itemfun(i)) {
				return err;
			}
		}
		return 0;
	}
};


template <class F, class ParFor2>
int exp_pade_step2(pade_exp_handle_t<F>* handle, matrix_item2_t<F>* pmmi, ParFor2&& parfor2) {

	int err = 0;

	auto pp = handle->pp;
	auto& pade = handle->pade;
	int M = pade.M;
	auto N = handle->N;

	auto accum_proc = get_accum_proc(handle->pade);
	auto poles = pade.poles;
	auto residues = pade.res;

	auto rn = handle->mmo;

	parfor2(pp, 0, M, [&](auto m) {

		auto ptmi = handle->pptmi[m];
		auto rnXpolus = rn * poles[m];
		auto rnXres = rn * residues[m];


		for (auto n = 0; n < N; n++) {
			auto& mmi = pmmi[n];
			auto& tmi = ptmi[n];

			tmi.a = mmi.v - rnXpolus;
			tmi.x[0] = rnXres * mmi.xy[0];
			tmi.x[1] = rnXres * mmi.xy[1];
			tmi.x[2] = F();
		}

		return thomas_2x_cycle(N, ptmi);
		//return pade_polus_res2_tic(N, rn * poles[m], rn * residues[m], pmmi, handle->pptmi[m]);
		});


	if (0 == err) {

		accum_proc(N, pmmi, handle->pptmi, M);
	}


	return err;
};

template <class F>
int exp_pade_step2(pade_exp_handle_t<F>* handle, matrix_item2_t<F>* pmmi) {

	return exp_pade_step2(handle, pmmi, fake_parfor_t<int>());
}



