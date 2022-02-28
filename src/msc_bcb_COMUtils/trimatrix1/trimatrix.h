#pragma once 
//trimatrix

#include "utils/utils.h"
#include "utils/complex-utils.h"
#include "trimatrix/thomas_cycle.h"
#include <memory>

#define _FAST_DIV(a,b) _fast_div(a,b)
//#define _FAST_DIV(a,b) (a/b)

//#define DBG_000_CR

namespace trimatrix {
		
	template <class F>
	struct triplet_t {
		F* a;
		F* b;
		F* c;
	};

	template <class It>
	struct fake_parallel_t {
		template <class RangeFunc>
		inline void operator()(It b, It e, RangeFunc rfun, size_t grain=1) {
			rfun(b, e);
		}
	};


	
	
	template < class _F, int M = 1, class _MEMPOOL = utils::mem_pool_t<>, template <class, class,bool> class _SOLVER = thomas_cycle_t >
	struct trimatrix_cycle_reduction_t :trimatrix_base_t<_F,M, _MEMPOOL> {
		//typedef _COMPLEX<_F> complex_t;
		typedef _F f_t;
		typedef _MEMPOOL pool_t;
		typedef _SOLVER<f_t, pool_t, false> solver_t;


		template <int MM>
		struct factory_t {


		};

		
		template <class XX>
		static void disassemble(size_t N, XX& target, XX& even,  XX& odd) {

			auto& xe = even.xx;
			auto& xo = odd.xx;
			
			for (auto n = 0; n < N; n += 2) {
				
				target.op([&](auto x, auto m) {

					xe[m][n >> 1] = x[n + 0];
					xo[m][n >> 1] = x[n + 1];
				});				
			}

		};

		
		template <class XX>
		inline static void assemble(size_t N, XX& even, XX& odd, XX& sourse) {

			auto& xe = even.xx;
			auto& xo = odd.xx;

			for (auto n = 0; n < N; n += 2) {
				sourse.op([&](auto x, auto m) {					
					x[n + 0] = xe[m][n >> 1];
					x[n + 1] = xo[m][n >> 1];
				});
			}

		}


		template <class Parallel, class XX>		
		static void disassemble_pp(Parallel& parfor,size_t N, XX& target, XX& even, XX& odd) {

			auto& xe = even.xx;
			auto& xo = odd.xx;
			parfor(0, N, [&](auto b, auto e) {
				for (auto n = b; n < e; n += 2) {

					target.op([&](auto x, auto m) {

						xe[m][n >> 1] = x[n + 0];
						xo[m][n >> 1] = x[n + 1];
						});
				}});

		};


		template <class Parallel, class XX>
		inline static void assemble_pp(Parallel& parfor,size_t N, XX& even, XX& odd, XX& sourse) {

			auto& xe = even.xx;
			auto& xo = odd.xx;

			parfor(0, N, [&](auto b, auto e) {
				for (auto n = b; n < e; n += 2) {
					sourse.op([&](auto x, auto m) {
						x[n + 0] = xe[m][n >> 1];
						x[n + 1] = xo[m][n >> 1];
						});
				}
				});
			

		}





		void init() {
			if (level > 0) {
				auto l2 = level - 1;
				auto N2 = this->N >> 1;
				//odd = std::make_unique<trimatrix_cycle_reduction_t>(trimatrix_cycle_reduction_t(N2, l2));
				//even = std::make_unique<trimatrix_cycle_reduction_t>(trimatrix_cycle_reduction_t(N2, l2));			
				if (N2) {
					
					this->even.reset(new trimatrix_cycle_reduction_t(N2, l2));
					this->odd.reset(new trimatrix_cycle_reduction_t(N2, l2));
				}
			}
			else {
				lu.reset(new solver_t(this->N));
			}
		}

		trimatrix_cycle_reduction_t(size_t _N, int _level = 1)
			:trimatrix_base_t<f_t,M, pool_t>(_N), level(_level) {
			init();
			//this->pool.alloc<int>(11);
		}


		trimatrix_cycle_reduction_t(size_t _N,  f_t* _a, f_t* _b, f_t* _c, f_t** _yy, int _level = 1)
			:trimatrix_base_t<f_t, M, pool_t>(_N,_a,_b,_c,_yy), level(_level) {
			init();
		}

		template <class Triplet>
		trimatrix_cycle_reduction_t(size_t _N, Triplet* t,f_t** _yy, int _level = 1)
			:trimatrix_cycle_reduction_t(_N, t->a, t->b, t->c, _yy, _level){}

		template <class TriMatrix>
		trimatrix_cycle_reduction_t(TriMatrix* t, f_t** _yy, int _level = 1)
			: trimatrix_cycle_reduction_t(t->N, t->a, t->b, t->c, _yy, _level) {}

		
		template <class I,class Fun>
		inline bool reindexes(I N2,I& nb, I& ne, Fun prefix) {
			
			if (!nb) {
				prefix();
				++nb;
			}

			if ( ne >= N2 ) {
				--ne;
				return true;
			}
			return false;
		}

		template <class I, class Fun>
		inline static std::tuple<bool, bool>  index_bounds(I N2, I& nb, I& ne) {

			auto fb = (!nb), fe = (ne >= N2);
			if (fb) ++nb;
			if (fe) --ne;
			return {fb, fe};
		}

		template <class I>
		inline static void  index_bounds(I N2, I& nb, I& ne,bool &fb,bool& fe) {

			fb = (!nb);
			fe = (ne >= N2);
			if (fb) ++nb;
			if (fe) --ne;
			
		}

//#define DBG_000_CR
#ifdef DBG_000_CR
#define NDBG 8
#endif

		template<bool fodd,class XX>
		void make_half_range(XX& xxr,size_t nb, size_t ne,f_t az) {

			
			auto N = this->N;
			auto Ne = N - 1;
			auto N2 = N >> 1;

			auto a = this->a, b = this->b, c = this->c;

			auto& xx = xxr.xx;

			//auto [fb, fe] = index_bounds(N2, nb, ne);
			bool fb,fe;

			index_bounds(N2, nb, ne, fb, fe);

			trimatrix_cycle_reduction_t* half;

			//trimatrix_cycle_reduction_t* half = (fodd) ? odd.get() : even.get();

			if constexpr (fodd)
				half = odd.get();
			else
				half = even.get();

			auto ah = half->a, bh = half->b, ch = half->c;
			auto& xxh = half->yy;


			

			bool  ffodd = fodd;

#ifdef DBG_000_CR


			int dbg = 0;
			bN_t<f_t, NDBG> bbb;

			auto pxx = pN_t<NDBG>::ptr(xx[0]);
			auto pxh = pN_t<NDBG>::ptr(xxh.xx[0]);

			auto pa = pN_t<NDBG>::ptr(a);
			auto pb = pN_t<NDBG>::ptr(b);
			auto pc = pN_t<NDBG>::ptr(c);
			
			auto pah = pN_t<NDBG>::ptr(ah);
			auto pbh = pN_t<NDBG>::ptr(bh);
			auto pch = pN_t<NDBG>::ptr(ch);

			f_t dd[NDBG / 2], gg[NDBG / 2];
#endif 
			
			auto make_row = [&](auto n, auto n2, auto n2m, auto n2p, auto n2pp) {
				
				if constexpr ( fodd ){
					//[&] (auto n, auto n2, auto n2m, auto n2p)
					auto d = _FAST_DIV(b[n2p], az + a[n2]);
					auto g = _FAST_DIV(c[n2p], az + a[n2pp]);

					ah[n] = (az + a[n2p]) - (c[n2] * d + b[n2pp] * g);

					bh[n] = - b[n2] * d;
					ch[n] = - c[n2pp] * g;

					xxh.op([&](auto x, auto m) {
						x[n] -= d * xx[m][n2] + g * xx[m][n2pp];
						});
#ifdef DBG_000_CR
					dd[n] = d;
					gg[n] = g;
					dbg++;
#endif 
				}
				else {

					// Ae=D1-B*(1/D2)*C ; xe=ye-B*(1/D2)yo;

					auto d = _FAST_DIV(c[n2], az + a[n2p]);
					auto g = _FAST_DIV(b[n2], az + a[n2m]);

					ah[n] = (az + a[n2]) - (b[n2p] * d + c[n2m] * g);

					bh[n] = - b[n2m] * g;
					ch[n] = - c[n2p] * d;

					xxh.op([&](auto x, auto m) {
						x[n] -= d * xx[m][n2p] + g * xx[m][n2m];
						});
#ifdef DBG_000_CR
					dd[n] = d;
					gg[n] = g;
					dbg++;
#endif 
				}

			};


			

			if (fb)
				make_row(0,0, Ne, 1,2);

			for (auto n = nb; n < ne; ++n) {
				auto n2 = n << 1;
				make_row(n, n2,n2-1,n2 + 1,n2+2);
			}

			if (fe)
				make_row(N2-1,N-2,N-3,N-1,  0);
			         //(    n, n2,n2m,n2p,n2pp)


		}

		template<bool fodd, class XX>
		void make_half(XX& xx, f_t az) {

			make_half_range<fodd>(xx, 0, this->N >> 1, az);
		}


		template <class XX>
		long half_factorize(int fodd,XX& xx, f_t az) {
			if (fodd) {
				make_half<true>(xx, az);
				return odd->factorize();
			}
			else {
				make_half<false>(xx, az);
				return even->factorize();
			}
		}


		long factorize(f_t az = f_t()) {
			return factorize(this->yy, az);
		}

		template <class XX>
		long factorize(XX& xx, f_t az= f_t())	{
			
			//this->a[0] = f_t();
			if (lu) {
				return lu->factorize(this,xx,az);
			}


			disassemble(this->N, xx, even->yy, odd->yy);

			half_factorize(0, xx, az);
			half_factorize(1, xx, az);
			

			assemble(this->N, even->yy, odd->yy, xx);

#ifdef DBG_000_CR
			bN_t<f_t, NDBG> bbb;
			auto pxx = pN_t<NDBG>::ptr(xx.xx[0]);
#endif
			return 0;
		}



		template <class Parallel , class XX>
		long half_factorize_pp(Parallel& parfor, int fodd, XX& xx, size_t grain, f_t az) {

			if (fodd) {			
				///*
				parfor(0, this->N >> 1, [&](auto b, auto e) {					
						make_half_range<true>(xx, b, e, az);					
					}
				, grain);
				//*/
				//make_half_range<true>(xx, 0, this->N >> 1, az);
				return odd->factorize_pp(parfor,grain);
			}
			else {
				///*
				parfor(0, this->N >> 1, [&](auto b, auto e) {
					make_half_range<false>(xx, b, e, az);
					}
				, grain);
				//*/
				//make_half_range<false>(xx, 0, this->N >> 1, az);
				return even->factorize_pp(parfor, grain);
			}
		}

		template <class Parallel>
		long factorize_pp(Parallel& parfor,size_t grain = 1, f_t az = f_t()) {
			return factorize_pp(parfor,this->yy, grain, az);
		}

		template <class Parallel , class XX>
		long factorize_pp(Parallel& parfor,XX& xx,size_t grain=1, f_t az = f_t()) {

			//this->a[0] = f_t();
			if (lu) {
				return lu->factorize(this, xx, az);
			}


					disassemble(this->N, xx, even->yy, odd->yy);

			//disassemble_pp(parfor,this->N, xx, even->yy, odd->yy);

			
			parfor(0, 2, [&](auto b, auto e) {
				for (auto i = b; i < e; ++i) {
					half_factorize_pp(parfor, i, xx, grain, az);
				}				
				}
			,1);

			//
								assemble( this->N, even->yy, odd->yy, xx);
			//			assemble_pp(parfor, this->N,  even->yy, odd->yy, xx);

#ifdef DBG_000_CR
			bN_t<f_t, NDBG> bbb;
			auto pxx = pN_t<NDBG>::ptr(xx.xx[0]);
#endif
			return 0;
		}







		

		int level;
		std::unique_ptr<trimatrix_cycle_reduction_t>  odd;
		std::unique_ptr<trimatrix_cycle_reduction_t>  even;
		
		std::unique_ptr<solver_t> lu;
	};
	

}// trimatrix
#undef DBG_000_CR