#pragma once 
#include "sparse_pool.h"


/*
template <>
struct  cs_types_t<cs_ci> {
	typedef cs_ci cs_t;
	typedef  int index_t;
	typedef  complex_t  data_t;
};
*/

#include <limits>


//extern 
invoke_context_helper_t g_ic;

template <class CS>
struct klu_types_t {
	typedef CS cs_t;
	typedef  cs_types_t<CS> index_t;
};

template <>
struct klu_types_t<cs_ci> {
	typedef cs_ci cs_t;
	typedef  cs_types_t<cs_t>::index_t index_t;
	typedef  cs_types_t<cs_t>::data_t  data_t;

	typedef  klu_numeric numeric_t;	
	typedef  klu_symbolic symbolic_t;
	typedef  klu_common common_t;

	inline  static 	index_t condest(cs_t* a, numeric_t* n, symbolic_t* s, common_t* c) {
		return klu_z_condest(a->p, (double*)a->x, s, n, c);
	}

	inline static void* lu_malloc(size_t l, size_t s, common_t * c) {
		return klu_malloc(l, s, c);
	}

	inline static void* lu_realloc(size_t nnew, size_t nold, size_t size,void* p, common_t * c) {
		return klu_realloc(nnew,nold,size, p, c);
	}

	inline static void* lu_free(void* p,size_t l, size_t s, common_t * c) {
		return klu_free(p, l, s, c);
	}

	inline  static  int free_symbolic(symbolic_t* Symbolic, common_t *Common) {
		return klu_free_symbolic(&Symbolic, Common);
	};

	inline  static int free_numeric(numeric_t* Numeric, common_t *Common) {
		return klu_free_numeric(&Numeric, Common);
	};
		
	
	inline static index_t set_defaults(common_t* c) {
		return klu_defaults(c);
	}
	inline static symbolic_t* analyze(index_t n, index_t* Ap, index_t* Ai, common_t* common) {
		return klu_analyze(n, Ap, Ai, common);
	}
	inline static numeric_t* factor(index_t n, index_t* Ap, index_t* Ai, data_t* Ax, symbolic_t* s, common_t* common) {
		return klu_z_factor( Ap, Ai,(double*)Ax,s, common);
	}

	inline static index_t solve(symbolic_t* symbolic, numeric_t*  numeric, common_t* common, index_t ldim, data_t* B, index_t nrhs = 1) {
		return klu_z_solve(symbolic, numeric, ldim, nrhs, (double*)B, common);
	}

	
};

template <>
struct klu_types_t<cs_cl> {
	typedef cs_cl cs_t;
	typedef  cs_types_t<cs_t>::index_t index_t;
	typedef  cs_types_t<cs_t>::data_t  data_t;

	typedef  klu_l_numeric numeric_t;
	typedef  klu_l_symbolic symbolic_t;
	typedef  klu_l_common common_t;

	inline  static index_t condest(cs_t* a, numeric_t* n, symbolic_t* s, common_t* c) {
		return klu_zl_condest(a->p, (double*)a->x, s, n, c);
	}

	inline static void* lu_malloc(size_t l, size_t s, common_t * c) {
		return klu_l_malloc(l, s, c);
	}

	inline static void* lu_realloc(size_t nnew, size_t nold, size_t size, void* p, common_t * c) {
		return klu_l_realloc(nnew, nold, size, p, c);
	}
	inline static void* lu_free(void* p, size_t l, size_t s, common_t * c) {
		return klu_l_free(p, l, s, c);
	}

	inline  static 	int free_symbolic (	symbolic_t* Symbolic,common_t *Common) {
		return klu_l_free_symbolic(&Symbolic, Common);
	};

	inline  static  int free_numeric(numeric_t* Numeric, common_t *Common) {
		return klu_l_free_numeric(&Numeric, Common);
	};

	inline static index_t set_defaults(common_t* c) {
		return klu_l_defaults(c);
	}

	inline static symbolic_t* analyze(index_t n, index_t* Ap, index_t* Ai, common_t* common) {
		return klu_l_analyze(n, Ap, Ai, common);
	}
	inline static numeric_t* factor(index_t n, index_t* Ap, index_t* Ai, data_t* Ax, symbolic_t* s, common_t* common) {
		return klu_zl_factor(Ap, Ai, (double*)Ax, s, common);
	}


	inline static index_t solve(symbolic_t* symbolic, numeric_t*  numeric, common_t* common, index_t ldim, data_t* B, index_t nrhs = 1) {
		return klu_zl_solve(symbolic, numeric, ldim, nrhs, (double*)B, common);
			
	}


};

template <class C1, class C2>
static void common2common(C1* Common, C2* Common2,bool fhalf=false)
{
	if (!(Common&&Common2))
		return;
	Common->tol = Common2->tol;
	Common->memgrow = Common2->memgrow;
	Common->initmem_amd = Common2->initmem_amd; /* init. mem with AMD:  c*nnz(L) + n */
	Common->initmem = Common2->initmem;      /* init. mem otherwise: c*nnz(A) + n */
	Common->btf = Common2->btf;        /* use BTF pre-ordering, or not */
	Common->maxwork = Common2->maxwork;       /* no limit to work done by btf_order */
	Common->ordering = Common2->ordering;      /* 0: AMD, 1: COLAMD, 2: user-provided P and Q,

							   /* user-provided function */
	Common->scale = Common2->scale;         /* scale: -1: none, and do not check for errors
							   * in the input matrix in KLU_refactor.
							   * 0: none, but check for errors,
							   * 1: sum, 2: max */
	Common->halt_if_singular = Common2->halt_if_singular;   /* quick halt if matrix is singular */

	if (fhalf)
		return;
									   /* user ordering function and optional argument */
	//Common->user_order = Common2->user_order;
	//Common->user_data = Common2->user_data;

	/* statistics */
	Common->status = Common2->status;
	Common->nrealloc = Common2->nrealloc;
	Common->structural_rank = Common2->structural_rank;
	Common->numerical_rank = Common2->numerical_rank;
	Common->noffdiag = Common2->noffdiag;
	Common->flops = Common2->flops;
	Common->rcond = Common2->rcond;
	Common->condest = Common2->condest;
	Common->rgrowth = Common2->rgrowth;
	Common->work = Common2->work;          /* work done by btf_order */

	Common->memusage = Common2->memusage;
	Common->mempeak = Common2->mempeak;

}

template <class CS>
struct klu_holder_t{
	typedef CS cs_t;	
	typedef klu_types_t<cs_t> klu_t;
	typedef typename klu_t::symbolic_t symbolic_t;	
	typedef typename klu_t::symbolic_t symbolic_t;
	typedef typename klu_t::numeric_t numeric_t;
	typedef typename klu_t::common_t common_t;
	typedef typename klu_t::index_t index_t;
	typedef typename klu_t::data_t data_t;

	common_t common;
	symbolic_t symbolic;
	numeric_t numeric;
	int phase;
	
	struct {
		symbolic_t* symbolic;
		numeric_t* numeric;		
	} resource;

	perf_times_t perf;

	cs_t cs_symbolic;
	cs_t cs_numeric;

	klu_holder_t() :symbolic(), numeric(),resource(), phase(0){

		const double nan = std::numeric_limits<double>::quiet_NaN();
		const perf_times_t pf = { nan,nan,nan,nan };
		klu_t::set_defaults(&common);
		common.btf = 0;
		perf=pf;

	}

	~klu_holder_t() {
		
		klu_t::free_numeric(resource.numeric, &common);		
		klu_t::free_symbolic(resource.symbolic, &common);
	};

	template <class COMMON_T >
	int64_t symbolic_factorize(cs_t* m, COMMON_T* _common) {

        common2common(&common, _common);
		if (phase > 0)
			return 777;
		
		symbolic_t* s = klu_t::analyze(m->n, m->p, m->i, &common);
		index_t status = common.status;
		if (s && (!status)) {

			symbolic = *(resource.symbolic = s);
			cs_symbolic = *m;
			phase = 1;
		}
		return status;
	}

	template <class COMMON_T >
	int64_t factorize(cs_t* m, COMMON_T* _common) {
		index_t status;

		if (phase > 1)
			return 777;

		if (phase <= 0) {
			status = symbolic_factorize(m, _common);
			if (status)
				return status;
		}
		else {

			if (!sparse_holder_t<cs_t>::is_equ_graph(&cs_symbolic, m))
				return -777;

			  common2common(&common, _common,false);
		} 

		//inline static numeric_t* factor(index_t n, index_t* Ap, index_t* Ai, data_t* Ax, symbolic_t* s, common_t* common) {
		index_t *i = cs_symbolic.i, *p = cs_symbolic.p;
		numeric_t* s = klu_t::factor(m->n, p, i, m->x, &symbolic, &common);
		status = common.status;

		if (s && (!status)) {

			/*
			klu_t::lu_free(s->Work,1,s->worksize,&common);
			s->Work = s->Xwork = NULL;			
			s->Iwork = NULL;
			*/

			numeric = *(resource.numeric = s);
			cs_numeric = *m;
			phase = phase | 2;
		}
		return status;
	}

	// inline static index_t solve(symbolic_t* symbolic, numeric_t*  numeric, common_t* common, index_t ldim, data_t* B, index_t nrhs = 1)


	/*
	inline index_t ts_numeric(numeric_t& numeric, numeric_t& ts,size_t s_add=0,void** pp=0) {

		index_t status;
		ts = numeric;
		char* p=0;
		if (status = g_ic.tls_buffer(ts.worksize+ s_add,(void**)&p))
				return status;

		ts.Xwork =ts.Work = p;
		ts.Iwork = NULL;// (index_t*)p;
		if (pp)
			*pp = p + ts.worksize;
		
		return 0;
	}
	*/
	inline index_t ts_numeric_alloc(numeric_t& numeric, numeric_t& ts, size_t s_add = 0, void** pp = 0) {

		index_t status;
		ts = numeric;
		char* p ;
		//if (status = g_ic.tls_buffer(ts.worksize + s_add, (void**)&p))
		p = (char*)::malloc(ts.worksize + s_add);

		if(!p)	return E_OUTOFMEMORY;


		ts.Xwork = ts.Work = p;
		ts.Iwork = NULL;// (index_t*)p;
		if (pp)
			*pp = p + ts.worksize;

		return 0;
	}

	inline void ts_numeric_free(numeric_t& ts) {
		::free(ts.Xwork);
			ts = numeric_t();
	}

	int64_t solve_ex(klu_params_solve_t* p) {

		if (phase < 2)
			return -777;
		index_t status = 0;

		numeric_t ts;

		

		data_t* pdata = 0,*pd ;
		index_t nrs = p->nrs;
		index_t n = p->n;

		int64_t sz_add = n*nrs * sizeof(data_t);

		//if (status = ts_numeric(numeric, ts,sz_add,(void**)&pdata))
			//return status;
		
		if (status = ts_numeric_alloc(numeric, ts, sz_add, (void**)&pdata))
			return status;

		pd = pdata;
		for (index_t k = 0; k < nrs; k++, pd += n)
			memcpy(pd, p->pp_in[k], n * sizeof(data_t));

		if (!klu_t::solve(&symbolic, &ts, &common, cs_symbolic.n, pdata, nrs)) {
			status = (common.status) ? common.status : -1;
		}
		
		pd = pdata;
		for (index_t k = 0; k < nrs; k++, pd += n)
			memcpy(p->pp_out[k],pd, n * sizeof(data_t));

		ts_numeric_free(ts);

		return status;
	
	}

	int64_t solve( data_t* pdata, index_t nrhs=1) {
		if (phase < 2)
			return -777;		
		index_t status=0;
		//
		/*
		numeric_t ts;
		if (status = ts_numeric(numeric, ts))
			return status;
		//*/

		if (!klu_t::solve(&symbolic, &numeric, &common, cs_symbolic.n, pdata, nrhs)){
			status = (common.status) ? common.status : -1;
		}

		//::free(ts.Work);
		return status;
	}

	int64_t clone_to(klu_holder_t& kh) {

		kh = *(this);	
		
		memset(&kh.resource, 0, sizeof(kh.resource));
		common.memusage = common.mempeak = 0;
		if (phase > 1) {
		
			size_t worksize = numeric.worksize;
			void* Work = klu_t::lu_malloc(worksize,1 ,&common);
			if (Work) {
				numeric_t* n= (numeric_t*)klu_t::lu_malloc(sizeof(numeric_t), 1, &common);
				*n = numeric_t();

				kh.numeric.worksize = n->worksize = worksize;
				kh.numeric.Xwork = kh.numeric.Work = n->Work = Work;
				kh.numeric.Iwork = NULL;
				kh.resource.numeric = n;
			}
			else return -1;
		}
		else kh.numeric= numeric_t();

		return phase;
	}

	int64_t condest(double* pvalue=0) {
		if (phase < 2)
			return -777;
		//inline  static 	index_t condest(cs_t* a, numeric_t* n, symbolic_t* s, common_t* c) {
		int64_t f = klu_t::condest(&cs_numeric, &numeric, &symbolic,&common);
		if ((f) && (pvalue))
			*pvalue = common.condest;
		return common.status;
	}
};

struct klu_matrix_context_t :context_impl_T< klu_matrix_context_t> {

	klu_matrix_context_t(base_context_t *pklu_ctx):klu_ctx(pklu_ctx){};


	context_ptr_t<base_context_t> klu_ctx;
};

template <class CS>
struct klu_context_t :context_impl_T< klu_context_t<CS> > {
	typedef CS cs_t;
	typedef sparse_holder_t<cs_t> sp_t;
	typedef klu_types_t<cs_t> klu_t;	
	typedef typename klu_t::common_t common_t;
	typedef typename klu_t::index_t index_t;
	typedef typename klu_t::data_t data_t;
	typedef typename sparse_holder_t<cs_t> sph_t,*psph_t;

	klu_context_t(context_holder_t* _ctx=NULL ):context_impl_T< klu_context_t<CS> >(), sp_link(NULL){
		ctx = _ctx;
		index_size = sizeof(index_t);
		create_handle(ctx);		
	}

	~klu_context_t() {
		reset_context(ctx, sp_link);		
	}

	long invoke(int64_t icmd, void* params) {

		
		typedef klu_types_t<cs_cl>::common_t* pcommonl_t;

		if (!params)
			return -11;

		long state;
		char cmd = (unsigned char)icmd;

		
		if (cmd == 's') {

			return state = klu.solve((data_t*)params);
		}
		else if (cmd == 'x') {
			//klu_x_solve_t* x = (klu_x_solve_t*)params;
			//return state = klu.solve((data_t*)(x->data), x->nrs);
			return state = klu.solve_ex((klu_params_solve_t*)params);
		}
		else if (cmd == 'm') {
			klu_x_solve_t* x = (klu_x_solve_t*)params;
			return state = klu.solve((data_t*)(x->data),x->nrs);
		}
		else if ((cmd == 'a')|| (cmd == 'f')) {

			
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

			if (state=invoke_context_helper_t::fast_call_context(params, '_', &sh))
				return state;

			if (get_index_size(sh) != index_size)
				return -2;

			s = &(sh->sm);

			//psph_t sh = psph_t(params);

			
			//base_context_t* sh = (base_context_t*)params;
			
			

			


			//

			if (cmd == 'a')
				state = klu.symbolic_factorize(s, (common_t*)NULL);
			else
				state = klu.factorize(s, (common_t*)NULL);


			 if ((state == 0) && ctx) {
				 link(ctx,sh);
			 }

			return state;
		}
		else if (cmd == 'c') {
			
			base_context_t** pp = (base_context_t**)params;
			klu_context_t* p = new  klu_context_t(ctx);
			klu.clone_to(p->klu);
			p->link(ctx,this);
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
			return 0;
		}
		else if (cmd == 'o') {
			common2common(&klu.common,pcommonl_t(params) );
			return 0;
		}
		else if (cmd == 'p') {

			*((int64_t*)params)= int64_t(klu.phase);
			return 0;
		}
		else if (cmd == 'e') {
			
			return klu.condest((double*)params);
		}
		


		return -1;
	}

	klu_holder_t<cs_t> klu;
	context_holder_t* ctx;
	sp_t* sp_link;
};
