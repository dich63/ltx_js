#pragma once
#include <limits>
#include "utils/static_constructors_0.h"
#include "utils/stof.h"

#define _L_(...) { __VA_ARGS__ }

//constexpr int NUMM = 21;

//const float_t NaN = std::numeric_limits<float_t>::quiet_NaN();


struct poles_res_data_t {
#define  _FLOAT char*
#define _L_(...) { __VA_ARGS__ }
#define _S( x ) #x
#define _N   "NaN"		

#include "./exp_pade_poles_res_21.inc"


#undef  _FLOAT
#undef _L_
#undef _S
#undef _N


	inline  static poles_res_data_t& get_instance() {
		return class_initializer_T<poles_res_data_t>::get();
	}
	poles_res_data_t() {

	}
};



template <class _FLOAT_TYPE=long double>
struct pade_exp_poles_res_t {

	typedef  _FLOAT_TYPE float_t;

	typedef struct complex_t {
		float_t re;
		float_t im;

	} complex_t, * pcomplex_t;

	inline static bool is_nan(float_t x) { return x != x; }
	//const float_t NaN = std::numeric_limits<float_t>::quiet_NaN();

	

   enum {
	  max_num = poles_res_data_t::NUMM
   };
	static int max_poles_num() {
		//return get().max_num;
		return poles_res_data_t::NUMM;

	}


	static  int   poles_res(int n, int m, const void* p_poles, const void* p_res, float_t e = 1) {
		
		pcomplex_t poles = pcomplex_t(p_poles);
		pcomplex_t res = pcomplex_t(p_res);

		if (m < 0) m = n;

		if ((m >= max_num)|| (n > m))
			return -1;

		poles_res_data_t& prd = poles_res_data_t::get_instance();

		const	 char** spoles_re = &prd.pade_poles_re[m][n][0];
		const	 char** spoles_im = &prd.pade_poles_im[m][n][0];

		const     char** sres_re = &prd.pade_res_re[m][n][0];
		const	  char** sres_im = &prd.pade_res_im[m][n][0];


		e = float_t(1) / e;

		//return 0;

		for (int i = 0; i < m; i++)
		{
			float_t poles_re, poles_im, res_re, res_im;
			
			
			stof(spoles_re[i], &poles_re);
			stof(spoles_im[i], &poles_im);
			stof(sres_re[i], &res_re);
			stof(sres_im[i], &res_im);
			

			if (is_nan(poles_re))
				return -1;
			poles[i].re = e * poles_re;
			poles[i].im = e * poles_im;
			res[i].re = e * res_re;
			res[i].im = e * res_im;
		}

		return m;


	}

	
	static  int   poles_res_half(int n, int m, const void* p_poles, const void* p_res, float_t e = 1) {

		const float_t eps = 1.e-10;
		pcomplex_t poles = pcomplex_t(p_poles);
		pcomplex_t res = pcomplex_t(p_res);

		if (m < 0) m = n;

		if ((m >= max_num) || (n > m))
			return -1;

		poles_res_data_t& prd = poles_res_data_t::get_instance();

		const	 char** spoles_re = &prd.pade_poles_re[m][n][0];
		const	 char** spoles_im = &prd.pade_poles_im[m][n][0];

		const     char** sres_re = &prd.pade_res_re[m][n][0];
		const	  char** sres_im = &prd.pade_res_im[m][n][0];



		int c = 0;

		float_t sc;

		e = float_t(1) / e;

		for (int i = 0; i < m; i++)
		{
			float_t poles_re, poles_im, res_re, res_im;
			stof(spoles_re[i], &poles_re);
			stof(spoles_im[i], &poles_im);
			stof(sres_re[i], &res_re);
			stof(sres_im[i], &res_im);



			if (is_nan(poles_re))
				return -1;
			

			if (poles_im < -eps) continue;


			sc = (poles_im > eps) ? 2 : 1;

			poles[c].re = e * poles_re;
			poles[c].im = e * poles_im;

			res[c].re = sc * e * res_re;
			res[c].im = sc * e * res_im;

			c++;

		}

		return c;
	
	}
	
};



#undef _L_