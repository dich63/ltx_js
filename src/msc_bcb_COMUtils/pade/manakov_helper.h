#pragma  once


#include "pade/manakov_solver.h"
#include "pade/manakov_solver.h"




#include <complex>
#include <vector>
#include "tbb/cache_aligned_allocator.h"


using namespace std::complex_literals;


template <template <class > class allocator= tbb::cache_aligned_allocator>
struct manakov_helper_t  {

	typedef std::complex<double> complex_t;


	


	

	typedef  pade_handle_t::mi_t mi_t;
	typedef  pade_handle_t::ti_t ti_t;

	typedef std::vector<ti_t, allocator<ti_t>> ti_holder_t;
	typedef std::vector<mi_t, allocator<mi_t>> mi_holder_t;
	

	enum {
		NUMM = pade_handle_t::NUMM
	};




	manakov_helper_t(size_t N=0, int M=2, double dz = 1, int pp = 1) :pade_handle() {	
		
			N && reset(N, M, dz, pp);
	
	}

	manakov_helper_t* reset(size_t N, int M=2, double dz = 1, int pp = 1){

		

		reset_pade_handle(&pade_handle, N, dz, M, pp);

		for (auto m = 0; m < M; m++) {
			ti_holders[m].clear();
			ti_holders[m].resize(N);
			pade_handle.pptmi[m] = ti_holders[m].data();
		}

		mi_holder.resize(N);

		return this;		

	}

	void loadV(complex_t* V) {
		for (auto n = 0; n < this->N; n++) {
			mi_holder[n].v = V[n];
		}

	}

	void loadXY(complex_t* X, complex_t* Y) {
		for (auto n = 0; n < pade_handle.N; n++) {
			mi_holder[n].xy[0] = X[n];
			mi_holder[n].xy[1] = Y[n];
		}
	  }


	void saveXY(complex_t* X, complex_t* Y) {
		for (auto n = 0; n < pade_handle.N; n++) {
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
		vx.resize(pade_handle.N);
		vy.resize(pade_handle.N);
		saveXY(vx.data(), vy.data());
	}


		inline pade_handle_t* get_handle() {

			return &pade_handle;
		}

		inline mi_t* get_mi() {

			return mi_holder.data();
		}
	


    pade_handle_t  pade_handle;
	ti_holder_t ti_holders[NUMM];
	mi_holder_t mi_holder;

};

