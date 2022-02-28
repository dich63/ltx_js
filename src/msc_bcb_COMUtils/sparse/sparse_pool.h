#pragma once

#include <omp.h>  
#include "klu.h"
#include "cs.h"
#include <complex>

#include "ipc_ports/context_holder.h"
//#include "ipc_ports/ipc_utils_base.h"
#include "invoke_context.h"
//#include "invoke_context/invoke_context.h"
//#include "matrix_context.h"
#include "sparse.h"


typedef cs_complex_t complex_t;

int cmp_struct_context(base_context_t* cs1, base_context_t* cs2);

struct sp_utils{	

	inline static void  _proj(const complex_t& y, double& x) {
		x = y.real();
	}
	inline static void  _proj(const complex_t& y, complex_t& x) {
		x = y;
	}
};

template <class T>
struct holder_base_t:invoke_context_t {

	//void* weak_ref_handle;
	holder_base_t() {
		weak_ref_handle = NULL;
	};
	static void ondestroy(void*p) {

		delete ((T*)p);
	}


	static long  s_invoke(invoke_context_t* _this) {

		return ((T*)_this)->invoke();
	};

	void* create_handle(context_holder_t* pch){
		if (pch) {
			weak_ref_handle&&pch->release(weak_ref_handle);
			pch->create_handle((T*)this, &ondestroy, &(weak_ref_handle=0));
		}
		return weak_ref_handle;
	}

	long invoke() {
		return -1;
	}

	template <class O>
	HRESULT link(context_holder_t* pch,O* o) {
		return (pch) ? pch->link(weak_ref_handle, o->weak_ref_handle) : -1;
	}

};


inline int64_t get_index_size(void* p) {

	return base_context_size_t::get_index_size(p);
		//(p) ? ((base_context_size_t*)p)->index_size : 0;
}

template <class CS_T> 
struct cs_types_t{
};

template <> 
struct  cs_types_t<cs_ci>{
	enum {
		SMP = SMP_CX | SMP_ISI
	};
	typedef cs_ci cs_t;
	typedef  int index_t;
	typedef  complex_t  data_t;
};

template <>
struct  cs_types_t<cs_cl>{
	enum {
		SMP = SMP_CX | SMP_ISL
	};

	typedef cs_cl cs_t;
	typedef long long index_t ;
	typedef  complex_t  data_t;
};

template <>
struct  cs_types_t<cs_di> {
	enum {
		SMP = SMP_RE | SMP_ISI
	};

	typedef cs_di cs_t;
	typedef  int index_t;
	typedef  double  data_t;
};

template <>
struct  cs_types_t<cs_dl> {
	enum {
		SMP = SMP_RE | SMP_ISL
	};
	typedef cs_dl cs_t;
	typedef long long index_t;
	typedef  double  data_t;
};


template <class CS_T = cs_cl>
struct sparse_holder_t :context_impl_T<sparse_holder_t<CS_T> > {
	typedef CS_T cs_t;	
	typedef typename cs_types_t<cs_t>::index_t index_t;
	typedef typename cs_types_t<cs_t>::data_t  data_t;


	/*
	cs_t *spalloc(CS_INT m, CS_INT n, CS_INT nzmax, CS_INT values, CS_INT triplet)
	{
	int offs;
	cs *A = cs_calloc(1, sizeof(cs));    // allocate the cs struct
	if (!A) return (NULL);                 // out of memory
	A->m = m;                              // define dimensions and nzmax
	A->n = n;
	A->nzmax = nzmax = CS_MAX(nzmax, 1);
	A->nz = triplet ? 0 : -1;              // allocate triplet or comp.col
	A->p = cs_malloc(triplet ? nzmax : n + 1, sizeof(CS_INT));
	A->i = cs_malloc(nzmax, sizeof(CS_INT));
	if (nzmax)
	A->i[0] = 77777;
	offs = offsetof(cs, i);
	A->x = values ? cs_malloc(nzmax, sizeof(CS_ENTRY)) : NULL;
	return ((!A->p || !A->i || (values && !A->x)) ? cs_spfree(A) : A);
	}

	static cs_t*  cs_copy(cs_t* s) {
	cs_t* d;


	return d;
	}

	*/

	static bool  is_equ_graph(cs_t* s, cs_t* d) {

		if ((!s) || (s == d) || (!d))
			return true;

		if ((s->nzmax==0) || (d->nzmax == 0))
			return true;

		auto nms = s->nzmax;
		

		bool f = (s->nzmax == d->nzmax) && (s->n == d->n) && (s->m == d->m);		

		f =f&&( (s->i==d->i)|| (memcmp(s->i, d->i, nms * sizeof(nms)) == 0));
		f =f&& ((s->p == d->p) || (memcmp(s->p, d->p, s->n * sizeof(nms)) == 0));

		return f;
	}

	static bool  is_csc(const cs_t* s) {
		return s && (s->nz <= 0);
	}

	static bool  is_null(const cs_t* s) {

		return (!s) || (s->nzmax == 0);
	}

	template<class typex_t, class typey_t>
	static int  gaxpy(const cs_t *A, const typex_t *x, typey_t *y)
	{
		//CS_INT p, j, n, *Ap, *Ai;
		if (!x)	return (1);

		if (is_null(A))	return (1);
		else 
			if (!is_csc(A) || !y) return (0);       /* check inputs */

		index_t p, j, n, *Ap, *Ai;
		data_t  *Ax;
		
		n = A->n; Ap = A->p; Ai = A->i; Ax = A->x;
		for (j = 0; j < n; j++)
		{
			for (p = Ap[j]; p < Ap[j + 1]; p++)
			{
				y[Ai[p]] += Ax[p] * x[j];
			}
		}
		return (1);


		/*

		data_t  *Ax = A->x;
		index_t n = A->n;
		index_t* Ap = A->p;
		index_t* Ai = A->i;

		index_t j = n,p=n;

		for (j = 0; j < n; j++)
		{
			for (p = Ap[j]; p < Ap[j + 1]; p++)
			{
				y[Ai[p]] += Ax[p] * x[j];
			}
		}
		return (1);*/
	}

	inline static cs_ci* sp_add(cs_ci* A, cs_ci* B,cs_complex_t a, cs_complex_t b) {
		return cs_ci_add(A, B, a, b);
	}

	inline static cs_cl* sp_add(cs_cl* A, cs_cl* B, cs_complex_t a, cs_complex_t b) {
		return cs_cl_add(A, B, a, b);
	}

	inline static int sp_dupl(cs_ci* s) {
		return cs_ci_dupl(s);
	}


	inline static int sp_dupl(cs_cl* s) {
		return cs_cl_dupl(s);
	}


	inline static cs_ci* sp_compress(cs_ci* s) {

		return cs_ci_compress(s);
	}

	inline static cs_cl* sp_compress(cs_cl* s) {

		return cs_cl_compress(s);
	}

	inline static cs_ci* sp_alloc(int m, int n, int nzmax, int values, int triplet, cs_ci** ppA) {

		cs_ci* pA;
		pA = cs_ci_spalloc(m, n, nzmax, values, triplet);
		if (ppA) *ppA = pA;
		return pA;
	}

	inline static cs_cl* sp_alloc(int m, int n, int nzmax, int values, int triplet, cs_cl** ppA) {

		cs_cl* pA;
		pA = cs_cl_spalloc(m, n, nzmax, values, triplet);
		if (ppA) *ppA = pA;
		return pA;
	}

	inline static cs_t* sp_alloc(int m, int n, int nzmax, int values, int triplet) {

		return sp_alloc(m, n, nzmax, values, triplet, (cs_t**)NULL);
	}

	inline static int sp_realloc(cs_ci* pA, long long nzmax) {

		return  cs_ci_sprealloc(pA, nzmax);
	}

	inline static int sp_realloc(cs_cl* pA, long long nzmax) {

		return  cs_cl_sprealloc(pA, nzmax);
	}


	//CS_INT cs_sprealloc(cs *A, CS_INT nzmax)
	
	inline static  cs_t* sp_copy(cs_t* c,int value, cs_t** pp_out=NULL)
	{
		cs_t* s;
		if (sp_alloc(c->m, c->n, c->nzmax, c->x != NULL,0, &s)) {

			memcpy(s->i, c->i, sizeof(index_t)*c->nzmax);
			memcpy(s->p, c->p, sizeof(index_t)*(c->n+1));

			if(value&&(c->x != NULL))
				memcpy(s->x, c->x, sizeof(data_t)*(c->nzmax));

			if (pp_out)
				*pp_out = s;
			return s;
		}
		else return 0;
	};
	

	inline static cs_t *spfree(cs_t *A)
	{
		if (!A) return (NULL);     /* do nothing if A already NULL */
		cs_free(A->p);
		cs_free(A->i);
		cs_free(A->x);
		return ((cs_t *)cs_free(A));   /* free the cs struct and return NULL */
	}

	inline cs_t * ptr() {
		return &sm;
	}

	

	template <class CS2>
	static cs_t * sp_copy(CS2*s) {

		if (!s)
			return NULL;
		const size_t si = sizeof(s->m);
		auto m = s->m, n = s->n, nzmax = s->nzmax,nz= s->nz;
		bool t = nz >= 0;

		cs_t *d = sp_alloc(m, n, nzmax,s->x!=NULL,t);
		if (!d)
			return NULL;
		size_t nc = t ?  nzmax : (n + 1);

		for (long long k = 0; k < nc; k++) {
			d->p[k] = s->p[k];
		}

		if(s->x){
			for (long long k = 0; k < nzmax; k++) {
				d->x[k] = s->x[k];
				d->i[k] = s->i[k];
			}
		}
		else 
			for (long long k = 0; k < nzmax; k++) {		
			d->i[k] = s->i[k];
		}

		if (t) {
			cs_t* o = d;
			o->nz = nz;
			d = sp_compress(o);
			spfree(o);
		}
		sp_dupl(d);
		return d;

	}

	static cs_t* to_coo(cs_t* A) {

		auto n = A->n, Ap = A->p, Ai = A->i, Ax = A->x, nzmax = A->nzmax;
		auto j = n, p = n;
		cs_t* R = sp_alloc(m, n, nzmax, !!(Ax), 1);
		auto  Rp = R->p, Ri = R->i, Rx = R->x;

		for (j = 0; j < n; j++)
		{
			for (p = Ap[j]; p < Ap[j + 1]; p++)
			{
				Ri[p] = Ai[p];
				Rp[p] = j;
				//y[Ai[p]] += Ax[p] * x[j];
			}
		}

		R->nz = Ap[n];
		if (Ax)
			memcpy(Rx, Ax, sizeof(*Ax)*nzmax);
		return R;

	}


	template<class cs_T>
	sparse_holder_t(cs_T *s, context_holder_t* pch = NULL) : sm(), sm_res(0), frozen(false) {

		index_size=sizeof(index_t);
		if (s)
			sm = *(sm_res = sp_copy(s));

		 create_handle(pch);
	};



	

	sparse_holder_t* clear() {
		//sm = cs_t();
		spfree(sm_res);
		sm_res = NULL;
		return this;
	}

	sparse_holder_t* attach(cs_t *s) {
		clear();
		if (s)
			sm = *(sm_res = s);
		return this;
	}

	sparse_holder_t*  clone(context_holder_t* pch = NULL, int mode = 0) {

		sparse_holder_t* c = new sparse_holder_t((cs_t*)NULL, pch);
		 c->sm = sm;
		 c->sm_res = cs_calloc(1, sizeof(cs_t));
		 c->sm.x = NULL;
		 data_t *x;
		 index_t nzmax = sm.nzmax;
		 if (mode == 1)
			 x = cs_calloc(nzmax, sizeof(data_t));
		 else 
			 x = cs_malloc(nzmax, sizeof(data_t));

		 if ((mode == 2)&&(sm.x))
			 memcpy(x, sm.x, nzmax * sizeof(data_t));

		 c->sm.x = c->sm_res->x = x;

		 c->link(pch, this);

		 return c;
	}



	template<class typex_t, class typey_t>
	int  gaxpy(const typex_t *x, typey_t *y) {
		if (this)
			return gaxpy(&sm, x, y);
		else
			return 1;
	}

	template <class cs_matrix>
	int loader(sparse_matrix_params_t& smp) {

		typedef cs_types_t<cs_matrix>::index_t index_t;
		typedef cs_types_t<cs_matrix>::data_t data_t;
		cs_matrix cm = {};
		cm.nz = (smp.flags&SMP_FMT) ? -1 : smp.nz;
		cm.nzmax =  smp.nzmax;
		cm.n = smp.n;
		cm.m = smp.m;		
		cm.i = (index_t*)smp.i;
		cm.p = (index_t*)smp.p;
		cm.x = (data_t*)smp.x;
		clear();
		sm = *(sm_res = sp_copy(&cm));
		return 0;
	}

	void rescale(const complex_t& scale) {
		complex_t s = scale;

		data_t* x = sm.x,*xe=x+sm.nzmax;

		while(x!=xe)
			(*x++)*= s;

	};



	long invoke(int64_t icmd, void* params) {

		//sp_maitrix_params_t* prm = (sp_maitrix_params_t*)params;
		if (!params)
			return -1;

		byte_t cmd = byte_t(icmd);
		if (cmd == '_') {
			void** pp = (void**)params;
			*pp = (void*)this;
			return 0;
		}
		else if (cmd == 'q') {
			cs_t** pp = (cs_t**)params;
			*pp = &sm;
			return 0;
		}
		else if (cmd == 'g') {
			sparse_matrix_gaxpy_t* sp = (sparse_matrix_gaxpy_t*)params;
			complex_t* x = (complex_t*)sp->x;
			complex_t* y = (complex_t*)sp->y;
			return !gaxpy(x, y);
		}
		else if (cmd == 'r') {
			//sparse_matrix_rescale_t*params= (sparse_matrix_rescale_t*)
			data_t& scale = *((data_t*)params);
			rescale(scale);
			return 0;
		}
		else if (cmd == 'l'){

			sparse_matrix_params_t* smp = (sparse_matrix_params_t*)params;

			int64_t flags = smp->flags;

			if (flags&SMP_RE) {

				if (flags&SMP_ISL)
					return loader<cs_dl>(*smp);
				else
					return loader<cs_di>(*smp);
				return -1;

			}
			else{

				if (flags&SMP_ISL)
					return loader<cs_cl>(*smp);
				else
					return loader<cs_ci>(*smp);
				return -1;
			}
			
		}
		else if (cmd == 'i') {

			int64_t SMP = cs_types_t<cs_t>::SMP | SMP_COO;

			sparse_matrix_params_t& s= *((sparse_matrix_params_t*)params);
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
	
	

	sparse_holder_t() : sm(), sm_res(0) {
		index_size = sizeof(index_t);
	};

	~sparse_holder_t() {
		spfree(sm_res);
	};



	cs_t sm;
	cs_t *sm_res;
	bool frozen;
	
};





template <class CS_T = cs_cl>
struct sparse_jet_t :context_impl_T<sparse_jet_t<CS_T> >{
	typedef CS_T cs_t;	
	typedef typename cs_types_t<cs_t>::index_t index_t;
	typedef typename cs_types_t<cs_t>::data_t  data_t;
	typedef sparse_holder_t<cs_t> sp_t;
	

	sparse_jet_t(context_holder_t* pch,int _count, sp_t** psp):
		count (_count),n(0),m(0),ctxh(pch),kb(-1),nz(-1) ,f_struct_equival(0){

		index_size = sizeof(index_t);
		create_handle(pch);
		spbuf.resize(count);
		pjet =(count)? &spbuf[0]:NULL;

		if (psp&&pjet) {

			for (int k = 0; k < count; k++) {
				sp_t* p = psp[k];
				if (p&&(!sp_t::is_null(&(p->sm)))) {
					kb = k;					
					break;

				}
			}
			if (kb < 0)
				return;
			


			for (int k = kb; k < count; k++) {
				pjet[k] = psp[k];
			}

			bool f = true;
			sp_t* pb = pjet[kb];
			n = pb->sm.n;
			nz= pb->sm.p[n];
			for (int k = kb + 1; k < count; k++) {
				sp_t* p = pjet[k];
				if (p && (!sp_t::is_null(&(p->sm)))) {
					f = 0==cmp_struct_context(pb, p);
				}
				if (!f)
					break;
			}

			f_struct_equival = f;

			if (pch) {

				for (int k = kb; k < count; k++) {
					link(pch, pjet[k]);
				}
			}
		}
	}

	HRESULT polyval_struct_fast(complex_t& _z, sp_t** pp_sp) {

		int k0 = kb,ke=count;

		sp_t* spb = pjet[k0];

		sp_t* spc;

		if (!(spc = spb->clone(ctxh, 1)))
			return -1;

		cs_t& s=spc->sm;

		
		data_t *x = s.x;
		index_t nzm = s.nzmax;		
		complex_t z = _z;
		complex_t zp = 1;

		for (int k = 0; k < k0; k++) {
			zp *= z;
		}

		for (int k = k0; k < ke; k++) {

			sp_t* jet = pjet[k];
			
			if (jet) {

				cs_t&  a = jet->sm;

				if (!sp_t::is_null(&a)) {

					data_t *xk = a.x;					

					for(index_t i=0;i<nzm;i++)
						x[i]+=zp*xk[i];
				}
			}
			zp *= z;
		}

		
		*pp_sp = spc;
		
		return 0;
	}


	 HRESULT polyval( complex_t& _z, sp_t** pp_sp) {
		 
		 HRESULT hr;
		 int k0 = kb;
		 if (k0 < 0)
			 return -1;

		 if ((!ctxh)||(!pp_sp)||(!pjet))
			 return -1;
		 if (count<1)
			 return -1;

		 if (f_struct_equival)
			 return polyval_struct_fast(_z, pp_sp);

		complex_t zp = 1;
		complex_t z = _z;
		//int kb = -1;
		
		for (int k = 0; k < k0; k++) {
			zp *= z;
		}

		
		  cs_t*  s= &(pjet[k0]->sm),*sr=NULL;
		  n = s->n;
		  m = s->m;
		  bool f = false;
		  for (int k = k0+1; k < count; k++) {

			  zp *= z;
			  sp_t* jet = pjet[k];

			  if (jet) {

				  cs_t*  pa = &(jet->sm);

				  if (!sp_t::is_null(s))
				  {
					  if (!sp_t::is_null(pa)) {
						  sr = sp_t::sp_add(s, pa, 1, zp);
						  if (f)
							  sp_t::spfree(s);
						  s = sr;
						  f = true;
					  }

				  }
				  else {
					  s = pa;
				  }
			  }

			  
		  }

		  sp_t*  sp= new sp_t(s,ctxh);
		  
		  *pp_sp = sp;

		  if (f)
			  sp_t::spfree(s);

		  return 0;

	}

template<class Float>
 static HRESULT make_jet(int n,int nd, complex_t z,Float** fnd, complex_t ** fjet ) {

	 complex_t* ps=fjet[0];
	 
	 complex_t zd = z;
	 int d0 = 0;
	// return 0;
	 if (!ps) {
		 complex_t* ps1;
		 ps1 = fjet[1];
		 Float* f = fnd[0];
		 for (int i = 0; i < n; i++) {
			 ps1[i] = -f[i];
		 }
		 ++d0;
		 ps = ps1;
	 }

	 for (int d= d0; d < nd-1; d++) {
		 complex_t* ps1;
		 ps1 = fjet[d+1];
		 Float* f = fnd[d]; 
		 for (int i = 0; i < n; i++) {
			 ps1[i] = -f[i] + zd*ps[i];
		 }
		 
		 ps = ps1;
		 //zd *= z;
		 }	

		 return 0;
	}

 template<class Float>
  HRESULT make_jet(complex_t& z, Float** fnd, complex_t ** fjet) {
	  return make_jet(n, count,z, fnd, fjet);
 }

  void gaxpy(complex_t ** fjet, complex_t * y) {	  

	  for (int c = 0; c < count; c++) {
		  pjet[c]->gaxpy(fjet[c],y);
	  }
	  
  }


  template<class Float>
   HRESULT gaxpy_jet( complex_t& _z, Float** fnd, complex_t * y, complex_t * ys,complex_t * fbuf) {

	  int nd = count-1;

	  if (nd<=0)
		  return 0;
	  //
	  /*	  /// test
	  Float* f = fnd[0];
	  for (int i = 0; i < n; i++) {
		  y[i] = 2.0* y[i]+0.0*f[i]+3.3;
	  }
	  return 0;
	  //*/
	  complex_t* ps,*ps2 = fbuf;
	  complex_t z = _z;

	  if (ys) {
		  ps = ys;		  
		  pjet[0]->gaxpy(ps, y);		  
		  
	  }
	  else ps = NULL;
	  
	  for (int d = 0; d < nd; d++) {

		  Float* f = fnd[d];

		  if(ps)
			  for (int i = 0; i < n; i++) {
				  ps2[i]= f[i] + z*ps[i];		  
			  }
		  else {
			  
			  for (int i = 0; i < n; i++) {
				  ps2[i] =f[i];
			  }
		  }		
		  
		  pjet[d+1]->gaxpy(ps = ps2, y);
		  
	  }

	  return 0;

  }

  
  

  template<class Float>
  static void lñ(int N,int D, complex_t **vv, complex_t *cc, Float* xout) {

 //#pragma omp parallel for schedule(dynamic)

	  for (int d = 0; d < D; d++) {
		  complex_t *v = vv[d];
		  complex_t c = cc[d];		  
		  Float* x = xout;
		  if(d)
			  for (int n = 0; n < N; n++) {
				  Float f;
				  sp_utils::_proj(c*(v++)[0], f);
				  *(x++)+=f ;
			  }
		  else
			  for (int n = 0; n < N; n++) {				  
				  sp_utils::_proj(c*(v++)[0], *(x++));
			  }			  
	   }
  };



  long invoke(int64_t icmd, void* params) {

	  const complex_t c_e = 1;
	  //sp_maitrix_params_t* prm = (sp_maitrix_params_t*)params;
	  byte_t cmd = byte_t(icmd);
	  if (!params)
		  return -1;

	  if (cmd == '_') {
		  void** pp = (void**)params;
		  *pp = (void*)this;
		  return 0;
	  }
	  else if (cmd == 'x') {

		  sparse_jet_gaxpy_jet_t* jj= (sparse_jet_gaxpy_jet_t*)params;
		  complex_t& z = *((complex_t*)jj->z);
		  complex_t* y = (complex_t*)jj->py;
		  complex_t* ys = (complex_t*)jj->pys;
		  complex_t* buf = (complex_t*)jj->pbuf;

		  if (jj->type[0] == 'r') {
			  double** f = (double**)jj->pp_f;
			  return gaxpy_jet(z, f, y,ys,buf);
		  }
		  else {
			  complex_t** f = (complex_t**)jj->pp_f;
			  return gaxpy_jet(z, f, y, ys, buf);
		  }


	  }
	  else if (cmd == 'j') {
		  sparse_jet_jet_t* jj = (sparse_jet_jet_t*)params;
		  complex_t& z = *((complex_t*)jj->z);
		  complex_t** cjet = (complex_t**)jj->pp_jet;

		  int64_t c = (jj->count!=-1)? jj->count:count;		  
		  int64_t N = (jj->n != -1) ? jj->n : n;
		  
		  if (jj->type[0] == 'r') {
			  double** f=(double**)jj->pp_f;			  
			  return make_jet(N, c, z, f, cjet);
		  }
		  else{
			  complex_t** f = (complex_t**)jj->pp_f;
			  return make_jet(N, c, z, f, cjet);
		  }
		
		  return -1;

	  }
	  else if (cmd == 'g') {

		   sparse_jet_gaxpy_t* sjg = (sparse_jet_gaxpy_t*)params;
		   complex_t** f = (complex_t**)sjg->pp_jet;
		   complex_t* y = (complex_t*)sjg->p_out;
		   gaxpy(f, y);
		   return 0;
	   }
	  else if (cmd == 'p') {
		   sparse_jet_polyval_t* spv = (sparse_jet_polyval_t*)params;
		   base_context_t** pp_out = (base_context_t**)spv->pp_out;
		   if (!pp_out)
			   return -1;
		   long st;
		   complex_t &z = *((complex_t*)spv->z);
		   complex_t s = *((complex_t*)spv->s);

		   if ((!(st=polyval(z, (sp_t**)pp_out)))&&(s!=c_e))
		   {
			   sp_t* sp = *((sp_t**)pp_out);
			   sp->rescale(s);
		   }
		   return st;

	   }
	  else if (cmd == 'i') {

		  sparse_jet_info_t i = { count,n,nz,f_struct_equival,pjet };
		  *((sparse_jet_info_t*)params)=i;
		  
		  return 0;
	  }

	  return -1;
  }

  static HRESULT createInstance(context_holder_t* pch,int64_t count,base_context_t** pp_sp, base_context_t** pp_jet) {
	  if (!pp_jet)
		  return -1;

	  sparse_jet_t* p = new sparse_jet_t(pch, count,(sp_t**) pp_sp);
	  *pp_jet = p;
	  return 0;

  }

	sp_t** pjet;
	int count;
	int64_t n, m,nz;
	int kb;
	bool f_struct_equival;
	std::vector<sp_t*> spbuf;
	context_holder_t* ctxh;

};


inline int cmp_struct_context(base_context_t* cs1, base_context_t* cs2) {

	typedef sparse_holder_t<cs_ci> i_t, *p_i;
	typedef sparse_holder_t<cs_cl> l_t, *p_l;

	if (cs1&&cs2) {
		int64_t s1 = get_index_size(cs1);
		int64_t s2 = get_index_size(cs2);
		if (s1 != s2)
			return -2;
		if (s1 == sizeof(i_t::index_t))
			return i_t::is_equ_graph(&p_i(cs1)->sm, &p_i(cs2)->sm)? 0 : -3;
		else
			return l_t::is_equ_graph(&p_l(cs1)->sm, &p_l(cs2)->sm) ? 0 : -3;
	}

	return -1;
};
