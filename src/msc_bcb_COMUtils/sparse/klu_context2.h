//klu_context2.h 
#pragma once
#include "sparse_frob_utils.h"
#include "klu_context.h"

#define CHECK_ERR(e) { long e01001=(e); if(e01001) return e01001;};

template <class CS>
struct fro_holder_t {
	typedef CS cs_t;
	
	typedef klu_types_t<cs_t> klu_t;
	typedef typename sparse_frob_utils_t<cs_t> spfu_t;
	
	typedef typename klu_t::symbolic_t symbolic_t;
	typedef typename klu_t::symbolic_t symbolic_t;
	typedef typename klu_t::numeric_t numeric_t;
	typedef typename klu_t::common_t common_t;
	typedef typename klu_t::index_t index_t;
	typedef typename klu_t::data_t data_t;
	typedef typename klu_holder_t<cs_t> lu_solver_t;
	//typedef typename frobenius_solver_t<lu_solver_t,cs_t> frobenius_solver_t;
	

	index_t n,m;
	

};

template <class CS>
struct klu_context2_t :context_impl_T< klu_context2_t<CS> > {
	typedef CS cs_t;
	typedef sparse_holder_t<cs_t> sp_t;
	typedef klu_types_t<cs_t> klu_t;
	typedef typename klu_t::common_t common_t;
	typedef typename klu_t::index_t index_t;
	typedef typename klu_t::data_t data_t;
	typedef typename sparse_holder_t<cs_t> sph_t, *psph_t;

	typedef typename sparse_frob_utils_t<cs_t> spfu_t;

	klu_context2_t(context_holder_t* _ctx = NULL) :context_impl_T< klu_context2_t<CS> >(), sp_link(NULL),mn(0) {
		ctx = _ctx;
		index_size = sizeof(index_t);
		create_handle(ctx);
	}

	~klu_context2_t() {
		reset_context(ctx, sp_link);
	}

	long invoke(int64_t icmd, void* params) {


		typedef klu_types_t<cs_cl>::common_t* pcommonl_t;

		if (!params)
			return -11;

		long state;
		char cmd = (unsigned char)icmd;


		if (cmd == 's') {

			return state = fr_solver.solve((data_t*)params);
		}
		else if (cmd == 'x') {
			//klu_x_solve_t* x = (klu_x_solve_t*)params;
			//return state = klu.solve((data_t*)(x->data), x->nrs);
			return state = klu.solve_ex((klu_params_solve_t*)params);
		}
		else if (cmd == 'm') {
			klu_x_solve_t* x = (klu_x_solve_t*)params;
			return state = klu.solve((data_t*)(x->data), x->nrs);
		}
		else if ((cmd == 'a') || (cmd == 'f')) {

			if (klu.phase <= 0) {
				mn = klu.common.numerical_rank;
				//klu.common.numerical_rank = -1;
			}



			/*
			if (!params)
			return -1;
			klu_params_t* pku = (klu_params_t*)params;


			if (get_index_size(pku->sp_context) != index_size)
			return -2;
			psph_t sh = psph_t(pku->sp_context);
			cs_t* s = &(sh->sm);
			if(cmd == 's')
			state= klu.symbolic_factorize(s, (common_t*)pku->common);
			else
			state = klu.factorize(s, (common_t*)pku->common);
			*/

			sph_t* sh;
			cs_t* s;

			if (state = invoke_context_helper_t::fast_call_context(params, '_', &sh))
				return state;

			if (get_index_size(sh) != index_size)
				return -2;

			s = &(sh->sm);

			//psph_t sh = psph_t(params);


			//base_context_t* sh = (base_context_t*)params;






			//

			if (cmd == 'a')
				state = 0;// klu.symbolic_factorize(s, (common_t*)NULL);
			else {


				spfu_t::sd_block_mat_t sbm(s, mn);

				CHECK_ERR(state = klu.factorize(s, (common_t*)NULL));

				CHECK_ERR(state = fr_solver.init(&klu, sbm.mB, sbm.mC, sbm.mD));


				

			}


			if ((state == 0) && ctx) {
				link(ctx, sh);
			}

			return state;
		}
		else if (cmd == 'c') {

			base_context_t** pp = (base_context_t**)params;
			klu_context2_t* p = new  klu_context2_t(ctx);
			klu.clone_to(p->klu);
			p->link(ctx, this);
			*pp = p;
			return 0;
		}
		/*
		else if ((cmd == 'i')|| (cmd == 'o')) {

		klu_types_t<cs_cl>::common_t* pc = (klu_types_t<cs_cl>::common_t*)params;
		if(cmd == 'i')
		common2common(pc,&klu.common );
		else
		common2common(&klu.common,pc);
		return 0;
		}*/
		else if (cmd == 'i') {
			common2common(pcommonl_t(params), &klu.common);
		}
		else if (cmd == 'o') {
			common2common(&klu.common, pcommonl_t(params));
		}
		else if (cmd == 'p') {

			*((int64_t*)params) = int64_t(klu.phase);
			return 0;
		}
		else if (cmd == 'e') {

			return klu.condest((double*)params);
		}



		return -1;
	}

	frobenius_solver_t<klu_holder_t<cs_t> , cs_t> fr_solver;
	klu_holder_t<cs_t> klu;
	context_holder_t* ctx;
	sp_t* sp_link;
	index_t mn;
};
