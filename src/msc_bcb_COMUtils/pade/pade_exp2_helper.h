#pragma  once

#include "pade/pade_exp2.h"


#include <complex>
#include <vector>
#include "tbb/cache_aligned_allocator.h"


using namespace std::complex_literals;


template <template <class > class allocator= tbb::cache_aligned_allocator>
struct pade_exp_handle_helper_t :pade_exp_handle_t<std::complex<double>> {

	typedef std::complex<double> complex_t;

	typedef pade_exp_handle_t <complex_t> base_t;

	typedef  base_t::mi_t mi_t;
	typedef  base_t::ti_t ti_t;

	typedef std::vector<ti_t, allocator<ti_t>> ti_holder_t;
	typedef std::vector<mi_t, allocator<mi_t>> mi_holder_t;
	

	enum {
		NUMM = base_t::pade_t::NUMM
	};

	pade_exp_handle_helper_t(size_t N=0, int M=2, double dz = 1, int pp = 1) :pade_exp_handle_t() {
	
		if (N) {
			reset(N, M, dz, pp);
		}
	
	}

	pade_exp_handle_helper_t& reset(size_t N, int M=2, double dz = 1, int pp = 1){

		

		reset_pade_handle(this, N, dz, M, pp);

		for (auto m = 0; m < M; m++) {
			ti_holders[m].clear();
			ti_holders[m].resize(N);
			this->pptmi[m] = ti_holders[m].data();
		}

		mi_holder.resize(N);

		return *this;		

	}

	void loadV(complex_t* V) {
		for (auto n = 0; n < this->N; n++) {
			mi_holder[n].v = V[n];
		}

	}

	void loadXY(complex_t* X, complex_t* Y) {
		for (auto n = 0; n < this->N; n++) {
			mi_holder[n].xy[0] = X[n];
			mi_holder[n].xy[1] = Y[n];
		}
	  }


	void saveXY(complex_t* X, complex_t* Y) {
		for (auto n = 0; n < this->N; n++) {
			X[n] = mi_holder[n].xy[0] ;
			Y[n] = mi_holder[n].xy[1] ;
		}
	}

	template <class VX,class VY>
	void loadXY(VX& vx, VY& vy) {
		loadXY(vx.data(), vy.data());
	}

	template <class VX, class VY>
	void saveXY(VX& vx, VY& vy) {
		vx.resize(this->N);
		vy.resize(this->N);
		saveXY(vx.data(), vy.data());
	}


		inline base_t* get_handle() {

			return static_cast<base_t*>(this);
		}

		inline mi_t* get_mi() {

			return mi_holder.data();
		}
	



	ti_holder_t ti_holders[NUMM];
	mi_holder_t mi_holder;


};

