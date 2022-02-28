#pragma once

#include "utils/complex-utils.h"
#include "pade/magnus-utils.h"
#include "pade/pade_exp2.h"


struct nls_data2_t {

	double dz=1;
	double g=0;
	double alpha=0;
	int omagnus=1;
	int pp=1;

};

template < class COMPLEX >
void magnus(size_t N, nls_data2_t& nd, matrix_item2_t<COMPLEX>* pmi);

template < class COMPLEX >
void post_attenuation(size_t N, nls_data2_t& nd, matrix_item2_t<COMPLEX>* pmi);


/// implementation;



template < class COMPLEX >
void magnus0(size_t N, nls_data2_t& nd,matrix_item2_t<COMPLEX>* pmi ){
	

	auto dec = -2.0*nd.alpha;
	auto g = nd.g;
	auto dt = nd.dz;

	auto gt0= g * texp_1_0(dec * dt);

	for (auto p = pmi; p <pmi+N; p++) {
		p->v.real(gt0* (abs2(p->xy[0]) + abs2(p->xy[1])));
	}
}




template < class COMPLEX >
void magnus1(size_t N, nls_data2_t& nd, matrix_item2_t<COMPLEX>* pmi) {


	auto dec = -2 * nd.alpha;
	auto g = nd.g;
	auto dt = nd.dz;

	auto gt0 = g * texp_1_0(dec * dt);

	auto gt1 = -g * dt * 2 * texp_1_1(dec * dt);


	auto fx = pmi[0].xy[0], fxm = pmi[N - 1].xy[0];
	auto fy = pmi[0].xy[1], fym = pmi[N - 1].xy[1];

	auto cycle_boby = [&](auto k, auto fxp, auto fyp) {

		auto v0 = gt0 * (abs2(fx) + abs2(fy));

		auto v1 = gt1 * (mul_imag_c(fx, fxm + fxp) + mul_imag_c(fy, fym + fyp));

		pmi[k].v.real(v0 + v1);

		fxm = fx;
		fym = fy;
		fx = fxp;
		fy = fyp;
	};

	for (auto k = 0; k < N - 1; k++) {

		auto xy = pmi[k + 1].xy;
		cycle_boby(k, xy[0], xy[1]);
	}

	cycle_boby(N - 1, pmi[0].xy[0], pmi[0].xy[1]);

};


template < class COMPLEX >
void magnus(size_t N, nls_data2_t& nd, matrix_item2_t<COMPLEX>* pmi) {
	if (nd.omagnus) {
		magnus1(N, nd, pmi);
	}
	else {
		magnus0(N, nd, pmi);
	}
};


template < class COMPLEX >
void post_attenuation(size_t N, nls_data2_t& nd, matrix_item2_t<COMPLEX>* pmi) {

	if (!is_nearly_null(nd.alpha)) {

		auto dt = nd.dz;
		auto dump = std::exp(-nd.alpha * dt);

		for (auto p = pmi; p < pmi + N; ++p) {
			p->xy[0] *= dump;
			p->xy[1] *= dump;
		}
	}

};





