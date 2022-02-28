#pragma once

template <class _F>
struct accum_static_t {
	typedef _F f_t;
	typedef typename void(*proc_t)(size_t N, f_t* xs, f_t** xsm, int m);



	static void accum1(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k];
		}
	};

	static void accum2(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k];
		}
	};
	static void accum3(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k];
		}
	};

	static void accum4(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k];
		}
	};

	static void accum5(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k];
		}
	};
	static void accum6(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k];
		}
	};
	static void accum7(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k];
		}
	};
	static void accum8(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k];
		}
	};
	static void accum9(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k];
		}
	};
	static void accum10(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k];
		}
	};

	static void accum11(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k] + xdm[10][k];
		}
	};
	static void accum12(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k] + xdm[10][k] + xdm[11][k];
		}
	};
	static void accum13(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k] + xdm[10][k] + xdm[11][k] + xdm[12][k];
		}
	};
	static void accum14(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k] + xdm[10][k] + xdm[11][k] + xdm[12][k] + xdm[13][k];
		}
	};
	static void accum15(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k] + xdm[10][k] + xdm[11][k] + xdm[12][k] + xdm[13][k] + xdm[14][k];
		}
	};
	static void accum16(size_t N, f_t* xs, f_t** xdm, int) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k] + xdm[10][k] + xdm[11][k] + xdm[12][k] + xdm[13][k] + xdm[14][k] + xdm[15][k];
		}
	};


	static void accumM(size_t N, f_t* xs, f_t** xdm, int M) {
		for (auto k = 0; k < N; ++k) {
			xs[k] = 0;
			for (auto m = 0; m < M; ++m) {
				xs[k] += xdm[m][k];
			}
		};
	}

	static proc_t get(int M) {
		switch (M)
		{
		case 1: return &accum1;
		case 2: return &accum2;
		case 3: return &accum3;
		case 4: return &accum4;
		case 5: return &accum5;
		case 6: return &accum6;
		case 7: return &accum7;
		case 8: return &accum8;
		case 9: return &accum9;
		case 10: return &accum10;
		case 12: return &accum12;
		case 13: return &accum13;
		case 14: return &accum14;
		case 15: return &accum15;
		case 16: return &accum16;
		}
		return &accumM;
	}

	struct add_t {
		static void accum1(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k];
			}
		};

		static void accum2(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k];
			}
		};
		static void accum3(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k];
			}
		};

		static void accum4(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k];
			}
		};

		static void accum5(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k];
			}
		};
		static void accum6(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k];
			}
		};
		static void accum7(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k];
			}
		};
		static void accum8(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k];
			}
		};
		static void accum9(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k];
			}
		};
		static void accum10(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k];
			}
		};

		static void accum11(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k] + xdm[10][k];
			}
		};
		static void accum12(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k] + xdm[10][k] + xdm[11][k];
			}
		};
		static void accum13(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k] + xdm[10][k] + xdm[11][k] + xdm[12][k];
			}
		};
		static void accum14(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k] + xdm[10][k] + xdm[11][k] + xdm[12][k] + xdm[13][k];
			}
		};
		static void accum15(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k] + xdm[10][k] + xdm[11][k] + xdm[12][k] + xdm[13][k] + xdm[14][k];
			}
		};
		static void accum16(size_t N, f_t* xs, f_t** xdm, int) {
			for (auto k = 0; k < N; ++k) {
				xs[k] += xdm[0][k] + xdm[1][k] + xdm[2][k] + xdm[3][k] + xdm[4][k] + xdm[5][k] + xdm[6][k] + xdm[7][k] + xdm[8][k] + xdm[9][k] + xdm[10][k] + xdm[11][k] + xdm[12][k] + xdm[13][k] + xdm[14][k] + xdm[15][k];
			}
		};


		static void accumM(size_t N, f_t* xs, f_t** xdm, int M) {
			for (auto k = 0; k < N; ++k) {
				//xs[k] = 0;
				for (auto m = 0; m < M; ++m) {
					xs[k] += xdm[m][k];
				}
			};
		}

		static proc_t get(int M) {
			switch (M)
			{
			case 1: return &accum1;
			case 2: return &accum2;
			case 3: return &accum3;
			case 4: return &accum4;
			case 5: return &accum5;
			case 6: return &accum6;
			case 7: return &accum7;
			case 8: return &accum8;
			case 9: return &accum9;
			case 10: return &accum10;
			case 12: return &accum12;
			case 13: return &accum13;
			case 14: return &accum14;
			case 15: return &accum15;
			case 16: return &accum16;
			}
			return &accumM;
		}



	};


	static proc_t get(int M, bool fadd) {

		return (fadd) ? add_t::get(M) : get(M);
	}

};