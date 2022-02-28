#pragma once
#include <limits>
//#include "utils/static_constructors_0.h"
//#include "utils/stof.h"

//#define _L_(...) { __VA_ARGS__ }

//constexpr int NUMM = 21;




struct poles_res_data_float64_t {

const double NaN = std::numeric_limits<double>::quiet_NaN();

#define  _FLOAT double
#define _L_(...) { __VA_ARGS__ }
#define _S( x ) x
#define _N   NaN		

#include "./exp_pade_poles_res_21.inc"


#undef  _FLOAT
#undef _L_
#undef _S
#undef _N


	inline  static poles_res_data_float64_t& get_instance() {
		static poles_res_data_float64_t d;
		return d;
	}
	poles_res_data_float64_t() {

	}
};


template <class _FLOAT=double>
struct pade_exp_poles_res_float64_t {

	typedef _FLOAT float_t;

	typedef struct complex_t {
		float_t re;
		float_t im;

	} complex_t, * pcomplex_t;

	inline static bool is_nan(double x) { return x != x; }
	

	

   enum {
	  max_num = poles_res_data_float64_t::NUMM
   };
	static int max_poles_num() {		
		return poles_res_data_float64_t::NUMM;
	}


	static  int   poles_res(int n, int m, const void* p_poles, const void* p_res, float_t e = 1) {
		
		pcomplex_t poles = pcomplex_t(p_poles);
		pcomplex_t res = pcomplex_t(p_res);

		if (m < 0) m = n;

		if ((m >= max_num)|| (n > m))
			return -1;

		poles_res_data_float64_t& prd = poles_res_data_float64_t::get_instance();

		const	 double* poles_re = &prd.pade_poles_re[m][n][0];
		const	 double* poles_im = &prd.pade_poles_im[m][n][0];

		const     double* res_re = &prd.pade_res_re[m][n][0];
		const	  double* res_im = &prd.pade_res_im[m][n][0];


		e = float_t(1) / e;

		
		for (int i = 0; i < m; i++)
		{
			
			
			

			if (is_nan(poles_re[i]))
				return -1;
			poles[i].re = e * poles_re[i];
			poles[i].im = e * poles_im[i];
			res[i].re = e * res_re[i];
			res[i].im = e * res_im[i];
		}

		return m;


	}
	
};



#undef _L_