#pragma once
//sparse_frob_utils.h
#include "klu_context.h"
#include "algorithms/gauss_jordan.h"
template <class D>
struct cache_buffer_t {
	D* p;
	size_t cb;
	cache_buffer_t() :p(0),cb(0) {};

	cache_buffer_t(size_t _cb, D* d=0):cache_buffer_t() {
		reset(_cb, d);
	};

	

	 D* reset(size_t _cb, D* d=0){

        size_t sz = _cb * sizeof(D);
		if (cb != _cb) {
			cb = _cb;
			p = (D*)::realloc(p, sz);
		}
		if (d)
			memcpy(p, d, sz);
		return p;
	}

	~cache_buffer_t() { 
		::free(p);
	}
	inline operator D*() { return p; }
};

template <class CS>
struct sparse_frob_utils_t{
	typedef CS cs_t;
	typedef klu_types_t<cs_t> klu_t;
	typedef typename klu_t::symbolic_t symbolic_t;
	typedef typename klu_t::symbolic_t symbolic_t;
	typedef typename klu_t::numeric_t numeric_t;
	typedef typename klu_t::common_t common_t;
	typedef typename klu_t::index_t index_t;
	typedef typename klu_t::data_t data_t;
	typedef typename klu_holder_t<cs_t> lu_solver_t;
	typedef typename index_t (*fsearch_t)(index_t i, index_t j, data_t d);
	typedef typename cache_buffer_t<data_t> data_buffer_t;
	

	struct flat_to_mat_t {
		data_t** pp;

		flat_to_mat_t() :pp(0) { };
		flat_to_mat_t(data_t* d, index_t n, index_t m) :pp(0) {
			reset(d, n, m);
		}
		data_t** reset(data_t* d, index_t n, index_t m)
		{   
			//::free(pp);
			pp = (data_t**)::realloc(pp,sizeof(data_t)*n);
			for (index_t k = 0; k < n;k++)
				pp[k] = d + m*k;
			return pp;
		}
		~flat_to_mat_t() {
			::free(pp); 
		}
		inline operator data_t**() {
			return pp;
		}
	};

	struct mat_t {
		data_t** pp;
		data_t* pb;
		index_t n, m;
		flat_to_mat_t f2m;
		size_t size;
		data_buffer_t buffer;

		mat_t() :pb(0) {}

		static void s_transpose(index_t n, index_t m, data_t**ppt, data_t**pps) {
			for (index_t i = 0; i < n; i++) {				
				data_t* pt = ppt[i];
				for (index_t j = 0; j < m; j++) {
					pt[j] = pps[j][i];
				}
				
			}
		
		}

		mat_t(index_t _n, index_t _m, data_t* _d = 0) :mat_t()
		{
			reset(_n, _m,_d);
		}

		data_t** reset(index_t _n, index_t _m, data_t* d=0,bool fzero=true) {

			
			size = _n*_m ;
			size_t szb = size * sizeof(data_t);

			pb = (data_t*)::realloc(pb,szb);
			if (d)
				memcpy(pb, d, szb);
			else
				fzero&&memset(pb, 0, szb);

			

			pp = f2m.reset(pb, n=_n, m=_m);

			

			return pp;
		}

		data_t** reset(mat_t& s,bool ftransp=false) {
			if (ftransp) {				
				reset(s.m, s.n, 0, false);
				s_transpose(n, m, pp, s.pp);				
			}
			else {
				reset(s.n, s.m, s.pb);
			}
			return pp;
		}

	inline	mat_t& operator =(mat_t& m) {
			reset(m);
			return *this;
		}

		data_t** like(mat_t& s, bool fzero = true) {
			return reset(s.n, s.m, 0,fzero);
		}


		~mat_t() {
			::free(pb);
		}

		mat_t& scale(data_t s) {
			for (index_t k = 0; k < size; k++) {
				pb[k] *= s;
			}
			return *this;
		}

		inline operator data_t**() {
			return pp;
		}

		inline data_t* to_flat(data_t* pd) {
			memmove(pd, pb, size* sizeof(data_t));
			return pd;
		}

		data_t* gaxpy(data_t* px, data_t* py) {
			data_t* x = px;
			for (index_t i = 0; i < n;i++) {
				data_t y = 0;
				data_t* p = pp[i];
				for (index_t j = 0; j < m; j++) {
					y += p[j] * x[j];
				}
				py[i] += y;
			}

			return py;
		}

		data_t* gaxpy(data_t* px, data_t* py, data_t sc) {
			data_t* x = px;
			for (index_t i = 0; i < n; i++) {
				data_t y = 0;
				data_t* p = pp[i];
				for (index_t j = 0; j < m; j++) {
					y += sc*p[j] * x[j];
				}
				py[i] += y;
			}

			return py;
		}

		

		index_t mul(mat_t& A, mat_t& B, data_t sc = 1.) {

			reset(A.n, B.m, 0, true);
			return addmul(A,B,sc);
		}

		index_t addmul(mat_t& A, mat_t& B,data_t sc=1.) {
			index_t k = A.m;

			if(k!=B.n)
				return -1;
			int nn = 0;
			mat_t tB;
			tB.reset(B, true);

			for (index_t i = 0; i < n; i++) {

				data_t*	 p = pp[i];
				data_t* pa = A.pp[i];
				tB.gaxpy(pa, p, sc);
				nn++;
			
			}

			return 0;
		}


		index_t em(data_t* px) {			
			if (n != m)
				return -1;

			//data_t* x=(tmp)?tmp:buffer.reset(n);
			//memcpy(x, px, sizeof(data_t)*n);

			data_t* x =  buffer.reset(n,px);


			for (index_t i = 0; i < n; i++) {
				data_t y = 0;
				data_t* p = pp[i];
				for (index_t j = 0; j < n; j++) {
					y += p[j] * x[j];
				}
				px[i] = y;
			}
			return 0;
		}


	};




	//static index_t s_fkeep(cs *A, fsearch_t fkeep, void *other){
	template <class F>
	static index_t s_fkeep(cs_t *A, F fkeep) {
		index_t j, p, nz = 0, n, *Ap, *Ai;
		data_t *Ax;
		//if (!CS_CSC(A) || !fkeep) return (-1);    /* check inputs */
		if (!CS_CSC(A))
			return (-1);

		n = A->n; Ap = A->p; Ai = A->i; Ax = A->x;
		if (Ax) {
			for (j = 0; j < n; j++)
			{
				p = Ap[j];                        /* get current location of col j */
				Ap[j] = nz;                       /* record new location of col j */
				for (; p < Ap[j + 1]; p++)
				{
					if (fkeep(Ai[p], j, Ax[p]))
					{
						Ax[nz] = Ax[p];  /* keep A(i,j) */
						Ai[nz++] = Ai[p];
					}
				}
			}
		}
		else {
			for (j = 0; j < n; j++)
			{
				p = Ap[j];                        /* get current location of col j */
				Ap[j] = nz;                       /* record new location of col j */
				for (; p < Ap[j + 1]; p++)
				{
					if (fkeep(Ai[p], j, 1))
					{			
						Ai[nz++] = Ai[p];
					}
				}
			}

		}
		Ap[n] = nz;                           /* finalize A */		
		return (nz);
	};

	static index_t	to_dense(cs_t*A, data_t** pp) {
		auto n = A->n, m = A->m;

		return s_fkeep(A, [&](index_t i, index_t j, data_t d) {
			pp[i][j] = d;
			return 1;
		});
	}

	static index_t	to_dense(cs_t*A, data_t* pd) {
		flat_to_mat_t f2m(pd, A->n, A->m);
		return to_dense(A, f2m);
	}

	static index_t	to_flat(cs_t*A, data_t* pd) {
		return to_dense(A, pd);
	}

	template<class EndoMorphism>
	static long em_p_mat(EndoMorphism op,mat_t& mm,bool ftrans=true) {

		long err;
		mat_t sm;
		data_t** ppb=sm.reset(mm, ftrans);
		err = op(ppb[0], mm.n, mm.m);
		mm.reset(sm, ftrans);
		return err;
	}

	static index_t inverse(mat_t& mm) {
		if (mm.n != mm.m)
			return -1;
		bool fok;
		gauss_jordan_t<data_t> gj(mm.n);
		fok = gj.inverse(mm.pp);
		return fok?0:-2;
	}


	struct sd_block_mat_t {

		static index_t split(index_t n, cs_t*A, data_t** B, data_t** C, data_t** D) {


			return  s_fkeep(A, [&](index_t i, index_t j, data_t d) {

				bool fi = (i < n), fj = (j < n);

				if (fi&&fj)
					return 1;

				

				if (!(fi || fj))
				{
					index_t im = i - n;
					index_t jm = j - n;
					D[im][jm] = d;
				}
				else if(fi) {
					index_t jm = j - n;
					B[i][jm] = d;
				}
				else {
					index_t im = i - n;
					C[im][j] = d;
				}

				
				
				return 0;
			});

			

			
		}

		sd_block_mat_t(cs_t*_A,index_t _m ):A(_A),m(_m) {
			n = A->n-m;
			B = mB.reset(n, m);
			C = mC.reset(m, n);
			D = mD.reset(m, m);
			split(n, A, B, C, D);
			A->n = A->m = n;
		}

		index_t to_dense(data_t* pA, data_t* pB, data_t* pC, data_t* pD) {
			to_flat(A, pA);
			mB.to_flat(pB);
			mC.to_flat(pC);
			mD.to_flat(pD);
			return 0;
		}

		cs_t*A;
		mat_t mB, mC, mD;
		data_t **B, **C, **D;
		index_t n,m;

	};

	

};


template <class LUS,class CS>
struct frobenius_solver_t {
	typedef LUS LU_solver_t;
	typedef CS cs_t;
	typedef typename sparse_frob_utils_t<cs_t> utils_t;
	typedef typename utils_t::mat_t mat_t;
	typedef typename utils_t::index_t index_t;
	typedef typename utils_t::data_t data_t;

	index_t opA(data_t* b, index_t n, index_t m = 1) {
		return luA->solve(b, m);
	}
	frobenius_solver_t() :luA(0),err(-1) {}
	frobenius_solver_t(LU_solver_t* _luA, mat_t& B, mat_t& C, mat_t& D) :frobenius_solver_t() {
		init(_luA, B, C, D);
	}

	long init(LU_solver_t* _luA, mat_t& B, mat_t& C, mat_t& D)
	{
		luA=_luA;
		m = D.m;
		nf = B.n;
		F = B;
		F.scale(-1.0);

		if (err = utils_t::em_p_mat([&](data_t* b ,index_t n, index_t m) {
			return luA->solve(b, m);
		}, F))			
			return err;

		mat_t &H = iH;
		H = D;
		H.addmul(C, F);

		err=utils_t::inverse(H);
		if (err)
			return  err;

		G.mul(iH, C,-1);
		return err;
	}

	long solve(data_t* xn, data_t* xm) {
		if (err)
			return err;
		long e;
		if (e = luA->solve(xn,1))
			return e;

		iH.em(xm);

		G.gaxpy(xn,xm);
		F.gaxpy(xm, xn);	
		return 0;
	}

	long solve(data_t* x) {
		return solve(x, x + nf);
	}


	LU_solver_t* luA;
	mat_t iH, F, G;
	index_t m,nf;

	long err;
};