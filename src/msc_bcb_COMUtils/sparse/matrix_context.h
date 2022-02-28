#pragma once
// matrix_context.h


#include "matrix_context_base.h"
#include "sparse.h"
template <class T>
struct matrix_context_t :context_impl_T<T> > {

	inline long gaxpy(matrix_gaxpy_t& mg) {
		return -1;
	}

	inline long rescale(void* sc) {
		return -1;
	}

	inline long is_null(int64_t* pf) {
		return -1;
	}

	inline long template_mc(void* params) {
		return -1;
	}


	inline long loader(matrix_params_t* mp) {
		//matrix_lc_t m = { 1,spm,NULL };
		//return mlc(&m);
		return -1;
	}

	inline long mlc(matrix_lc_t* mp) {
		return -1;
	}
	

	long invoke(int64_t icmd, void* params) {
		T& self = *static_cast<T*>(this);
		byte_t cmd = icmd;


		if (!params)
			return -1;

		if (cmd == 'g') {

			matrix_gaxpy_t* sp = (matrix_gaxpy_t*)params;			
			return self.gaxpy(*sp);
		}
		else if (cmd == '_') {
			void** pp = (void**)params;
			*pp = (void*)&self;
			return 0;
		}		
		else if (cmd == 'n') {
			//matrix_lc_t* mp = (matrix_lc_t*)params;
			return self.is_null((int64_t*)params);
		}
		else if (cmd == 'r') {
			//sparse_matrix_rescale_t*params= (sparse_matrix_rescale_t*)
			
			return self.rescale(params);
		}
		else if (cmd == 't') {
			//sparse_matrix_rescale_t*params= (sparse_matrix_rescale_t*)						
			return self.template_mc(params);
		}
		else if (cmd == 'l') {

			matrix_params_t* mp = (matrix_params_t*)params;

			return self.loader(mp);			

		}
		else if (cmd == 'i') {

			//int64_t SMP = cs_types_t<cs_t>::SMP | SMP_COO;

			matrix_params_t& s = *((matrix_params_t*)params);
			s.flags = SMP;
			s.n = sm.n;
			s.m = sm.m;
			s.nzmax = sm.nzmax;
			s.i = sm.i;
			s.p = sm.p;
			s.x = sm.x;
			return 0;

		}
		return -11;

	}

	int64_t SMP;
};