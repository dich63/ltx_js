#pragma once 

#include "utils/complex-utils.h"

template <class _F> 
struct tri_matrix_item1_t {
	typedef _F f_t;
	
		f_t a;
	    f_t x[2];	
};



/*
    solve 
	| a[0]  1  0...0  0| |???|   =| f[0]  x[0] y[0] |
	|  1  a[1] 0...0 0 | |???| =| f[1]  x[1] y[1] |
	|  .......  ...... | |???| =.........
	|  0 0 ... 1 a[N-1]| |???| =| f[N-1] x[.] y[.] |

*/


template <class _F>
int thomas_2x(size_t N,tri_matrix_item1_t<_F>* pt) {

	typedef _F f_t;

	int err = 0;


  // forward sweep

	auto p = pt + 0;

	f_t* x = p->x;
	f_t iu;

	auto Lkm = _fast_inv(p->a);

	p->a = Lkm;

	auto xm0 = p->x[0];
	auto xm1 = p->x[1];
	

	


//	for (auto k = 1; k < N; k++) {
	for (auto p = pt + 1; p < pt+N; p++) {

		//p = pt + k;
		x = p->x;

		iu = _fast_inv(p->a - Lkm);

		p->a = iu;		

		xm0 = (x[0] -= Lkm * xm0);
		xm1 = (x[1] -= Lkm * xm1);
		

		Lkm = iu;

	}


	

	//backward sweep 
	//p = pt + (N - 1);

	

	auto xp0 = (x[0] *= iu);
	auto xp1 = (x[1] *= iu);
	

	//for (int64_t k = N - 2; k >= 0; k--) {
	for (auto  p =pt+ N - 2; p >= pt; p--) {

		//p = pt + k;
	    x = p->x;

		iu = p->a;

		x[0] = xp0 = iu * (x[0] - xp0);
		x[1] = xp1 = iu * (x[1] - xp1);
		

	}


	return err;
}




/*
	solve
	| a[0]  1  0...0  1| |???|   =|   x[0] y[0] |
	|  1  a[1] 0...0 0 | |???| =|   x[1] y[1] |
	|  .......  ...... | |???| =.........
	|  1 0 ... 1 a[N-1]| |???| =|  x[.] y[.] |


*/
//  all x[0] must be zero
template <class _F>
int thomas_1x_cycle(size_t N, tri_matrix_item1_t<_F>* pt) {

	typedef _F f_t;

	int err = 0;
	pt[  0].x[1] = -_F(1);
	pt[N-2].x[1] = -_F(1);

	auto D = pt[N - 1].a;
	f_t H;

	if (!(err = thomas_2x(N-1,pt))) {

		f_t xm0;


		H = D + pt[0].x[1] + pt[N - 2].x[1];

		pt[N - 1].x[0] = xm0 = (pt[N - 1].x[0] - pt[0].x[0] - pt[N - 2].x[0])/H;		
		pt[N - 1].x[1] = H;

		//for (int64_t k = 0; k < N - 1; k++) {}

		for (auto p = pt; p < pt + N - 1; p++) {
			auto x = p->x;
			
			x[0] += x[1] * xm0;
			
		}

	}

	return err;
}



template <class V>
int thomas_2x(V& v) {
	return thomas_2x(v.size(), v.data());
};

template <class V>
int thomas_1x_cycle(V& v) {
	return thomas_1x_cycle(v.size(), v.data());
}



template <class F>
std::vector<tri_matrix_item1_t<F>> tri_matrix_item1_vector(size_t N, F* a, F* x) {

	std::vector<tri_matrix_item1_t<F>> v(N);

	for (auto n = 0; n < v.size(); n++) {
		v[n].a = a[n];
		v[n].x[0] = x[n];		
	};


	return v;
};


template <class F>
 size_t fill_tri_matrix_item1_vector(size_t N, F* a, F* x, std::vector<tri_matrix_item2_t<F>>&v  ) {

	

	for (auto n = 0; n < v.size(); n++) {
		v[n].a = a[n];
		v[n].x[0] = x[n];		
		v[n].x[1] = F();
	};


	return v.size();
};






