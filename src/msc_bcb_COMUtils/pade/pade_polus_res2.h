#pragma  once

#include "trimatrix/thomas3.h"
#include "pade/accum_static2.h"
#include "pp_tbb/pp.h"

template <class F>
struct matrix_item2_t {
	typedef F f_t;

	f_t v;
	f_t xy[2];	
	f_t notused;
};


template < class _COMPLEX >
	struct pade_data_t {
	
	typedef typename _COMPLEX complex_t;

	typedef typename matrix_item2_t<complex_t> mi_t;
	typedef tri_matrix_item2_t<complex_t> ti_t;

	typedef typename void(*proc_t)(size_t N, mi_t*, ti_t**, int m);


	enum{
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
		NUMM=pade_t::NUMM
	};

	pade_t pade;
	size_t N;
	ti_t* pptmi[NUMM];	
	complex_t mmo; // 1i/dz
	int pp;
};

	



template <class F>
int pade_polus_res2_tic(size_t N, F polus, F res, matrix_item2_t<F>* pmmi, tri_matrix_item2_t<F>* ptmi) {

	for (auto n = 0; n < N; n++) {
		auto& mmi = pmmi[n];
		auto& tmi = ptmi[n];

		tmi.a = mmi.v + polus;
		tmi.x[0] = res * mmi.xy[0];
		tmi.x[1] = res * mmi.xy[1];
		tmi.x[2] = F();
	}

	return thomas_2x_cycle(N, ptmi);
};

template <class F,class ParFor>
int exp_pade_step2(pade_exp_handle_t<F>* handle, matrix_item2_t<F>* pmmi, const ParFor& parfor = tbb_parfor_t<int>()) {
	int err = 0;

	auto pp = handle->pp;
	auto& pade = handle->pade;
	int M = pade.M;
	auto N = handle->N;

	auto accum_proc = handle->pade.accum;
	auto poles = pade.poles;
	auto res = pade.res;

	auto mmo = handle->mmo;

	try {

		parfor(pp, 0, M, [&](auto b, auto e) {

			for (auto m = b; m < e; m++) {
				int e;
				if (e = pade_polus_res2_tic(N, mmo*poles[m], mmo*res[m] pmmi, handle->pptmi[m]))
					throw 1;
			}

			});
	}
	catch (...) {
		return err = 1;
	};


	accum_proc(N, pmmi, handle->pptmi, M);

	return err;
};




