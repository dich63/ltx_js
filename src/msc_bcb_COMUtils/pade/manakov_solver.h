#pragma once

#include "pade/magnus2.h"
#include "pade/pade_exp2.h"

using namespace std::complex_literals;

typedef std::complex<double> complex_t;
typedef pade_exp_handle_t<complex_t> pade_handle_t;
typedef matrix_item2_t<complex_t> manakov_item_t;

/*
  from magnus2.h
  struct nls_data2_t {

	double dz=1;
	double g=0;
	double alpha=0;
	int omagnus=1;
	int pp=1;

};

*/


template <class ParFor2>
int manakov_solver(int rep, nls_data2_t nd, pade_handle_t* handle, manakov_item_t* pmi, ParFor2&& parfor2);
int manakov_solver(int rep, nls_data2_t nd, pade_handle_t* handle, manakov_item_t* pmi);


/// impplementation:



template <class ParFor2>
int manakov_solver(int rep , nls_data2_t nd, pade_handle_t* handle, manakov_item_t* pmi,ParFor2&& parfor2){
	int err = 0;
	auto N = handle->N;

	handle->mmo = -1i / nd.dz;
	handle->pp = nd.pp;
	
	for (auto k = 0; k < rep; k++) {
		magnus(N,nd,pmi);
		if (err = exp_pade_step2(handle, pmi, parfor2))
			return err;
		post_attenuation(N, nd, pmi);
	}
	return err;
}

inline int manakov_solver(int rep, nls_data2_t nd, pade_handle_t* handle, manakov_item_t* pmi) {
	return manakov_solver(rep, nd, handle, pmi, fake_parfor_t<int>());
}
