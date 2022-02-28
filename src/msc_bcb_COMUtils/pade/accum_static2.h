#pragma once

template <class matrix_item, class tri_matrix_item >
struct accum_static_diag2_t {

	
	typedef matrix_item mi_t;
	typedef tri_matrix_item ti_t;

	typedef typename void(*proc_t)(size_t N, mi_t*, ti_t**, int m);

	static void accum1(size_t N,  mi_t* mi, ti_t** pti, int) {
		for (auto k = 0; k < N; ++k) {
			mi[k].xy[0] = -mi[k].xy[0] + pti[0][k].x[0];
			mi[k].xy[1] = -mi[k].xy[1] + pti[0][k].x[1];
		}
	};
	static void accum2(size_t N,  mi_t* mi, ti_t** pti, int) {
		for (auto k = 0; k < N; ++k) {
			mi[k].xy[0] += pti[0][k].x[0] + pti[1][k].x[0];
			mi[k].xy[1] += pti[0][k].x[1] + pti[1][k].x[1];
		}
	};
	static void accum3(size_t N,  mi_t* mi, ti_t** pti, int) {
		for (auto k = 0; k < N; ++k) {
			mi[k].xy[0] = -mi[k].xy[0] + pti[0][k].x[0] + pti[1][k].x[0] + pti[2][k].x[0];
			mi[k].xy[1] = -mi[k].xy[1] + pti[0][k].x[1] + pti[1][k].x[1] + pti[2][k].x[1];
		}
	};

	static void accum4(size_t N,  mi_t* mi, ti_t** pti, int) {
		for (auto k = 0; k < N; ++k) {
			mi[k].xy[0] += pti[0][k].x[0] + pti[1][k].x[0] + pti[2][k].x[0] + pti[3][k].x[0];
			mi[k].xy[1] += pti[0][k].x[1] + pti[1][k].x[1] + pti[2][k].x[1] + pti[3][k].x[1];
		}
	};

	static void accumM_odd(size_t N,  mi_t* mi, ti_t** pti, int M) {
		for (auto k = 0; k < N; ++k) {
			auto xy0 = -mi[k].xy[0];
			auto xy1 = -mi[k].xy[1];
			for (auto m = 0; m < M; ++m) {

				xy0 += pti[m][k].x[0];
				xy1 += pti[m][k].x[1];
			}

			mi[k].xy[0] = xy0;
			mi[k].xy[1] = xy1;
		}
	};
	static void accumM_even(size_t N,  mi_t* mi, ti_t** pti, int M) {
		for (auto k = 0; k < N; ++k) {
			auto xy0 = mi[k].xy[0];
			auto xy1 = mi[k].xy[1];
			for (auto m = 0; m < M; ++m) {

				xy0 += pti[m][k].x[0];
				xy1 += pti[m][k].x[1];
			}

			mi[k].xy[0] = xy0;
			mi[k].xy[1] = xy1;
		}
	};

	static proc_t get(int M) {
		switch (M)
		{
		case 1: return &accum1;
		case 2: return &accum2;
		case 3: return &accum3;
		case 4: return &accum4;
		}
		return (M & 1) ? &accumM_odd : &accumM_even;
	}

};

