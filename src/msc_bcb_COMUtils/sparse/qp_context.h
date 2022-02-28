#pragma once
//qp_context.h

#include "ipc_ports/context_holder.h"
#include "sparse.h"
#include <vector>
#include <math.h>

template <class Float>
inline Float to_float(double &r) {
	return r;
}

template <class Float>
inline Float to_float(complex_t &r) {
	return r.real();
}


struct qp_source_t  {

	
	const complex_t c_id=1,c_null=0;
	
	qp_source_param_t* pQ;
	complex_t *Qc,*Qct;

	bool is_last_active;

	qp_source_t() :Qc(NULL), Qct(NULL), pQ(NULL), is_last_active(false) {}

	inline complex_t* getQc() {
		return (Qc) ? Qc : ((complex_t*)pQ->Q);
	}

	
	inline complex_t& get_z() {

		return *((complex_t*)pQ->z);
	}

	inline complex_t* get_transform() {

		return (complex_t*) pQ->transform_step;
	}
	inline	complex_t laplace(complex_t z) {

		complex_t z0 = get_z();
		complex_t lz = c_id / (z - z0),lzn=lz;		
		complex_t* q = getQc();
		complex_t res = q[0]*lzn;

		for (int k = 1; k < pQ->degQ + 1; k++) {
			lzn *= lz;// *double(k);
			res += q[k] * lzn;
		}
		return res;
	}
/*
	template<class Float>
	inline	void add_laplace(complex_t& z, int64_t n, Float* y) {
		complex_t l = laplace(z);
		complex_t* bs = (complex_t*)b;
		for (int64_t k = 0; k < n; k++) {
			complex_t v = bs[n];
			y[k] += to_float<Float>(l*v);			
		}
	}
	*/

	inline	void apply_laplace(complex_t& z, int64_t n, complex_t* y) {
		if (!pQ->is_active) {
			return;
		}
		complex_t l = laplace(z);
		complex_t* bs = (complex_t*)pQ->b;
		for (int64_t k = 0; k < n; k++) {
			complex_t v = bs[k];
			y[k] += l*v;
		}
	}

	inline	void apply_laplace_first(complex_t& z, int64_t n, complex_t* y) {

		if (!pQ->is_active) {
			return;
		}
		complex_t l = laplace(z);
		complex_t* bs = (complex_t*)pQ->b;
		for (int64_t k = 0; k < n; k++) {
			complex_t v = bs[k];
			y[k] = l*v;
		}
	}

	inline	void current( int64_t n, complex_t* y) {
		  complex_t q0 = getQc()[0];
		  complex_t* bs = (complex_t*)pQ->b; 
		  for (int64_t k = 0; k < n; k++) {
			  y[k]+=q0*bs[k];
		  }		  
	}

	inline	void reset() {

		int N = pQ->degQ+1;
		Qc&&memcpy(Qc, pQ->Q, sizeof(complex_t)*N);
		
	}

	inline	void transform() {

		if (pQ->is_active) {

			complex_t* t = get_transform();
			
			
			

			if (1&&t) {
				int N = pQ->degQ + 1;
				complex_t* tn = t;
				for (int n = 0; n < N; n++, tn += N) {
					complex_t qt = 0; 					
					for (int m = 0; m < N; m++) {
						qt += tn[m] * Qc[m];
					}
					Qct[n]=qt;
					
				}

				memcpy(Qc, Qct, sizeof(complex_t)*(N));
			}
			is_last_active = pQ->is_active;

		}
		else {
			if (is_last_active)
			{
				reset();
				is_last_active = false;
			}
		}
		
	}
};


template <int Vers=0>
struct qp_context_t :context_impl_T<qp_context_t<Vers> > {
	  
	qp_context_t(context_holder_t* pch = NULL):count(0), qps(NULL){
		create_handle(pch);
	}

	void init(qp_source_list_t*qpl)  {
		count = 0;
		int c = qpl->count;
		vQc.resize(c);
		vQct.resize(c);
		vqp.resize(c);

		qps = vqp.data();
		
		for (int k = 0; k < c; k++) {
			qp_source_param_t* p = qpl->params[k];
			qp_source_t& qp = qps[count];

			if (1&&p) {
				qp.pQ = p;
				//memcpy(&qp, p, sizeof(qp_source_param_t));
				if (qp.pQ->transform_step) {

					int sz = qp.pQ->degQ + 1;
					buffer_t<complex_t>& bQ = vQc[count];
					buffer_t<complex_t>& bQt = vQct[count];
					bQ.resize(sz);
					bQt.resize(sz);
					qp.Qc = bQ;
					qp.Qct = bQt;
					qp.reset();
				}
				else
					qp.Qc = NULL;

				count++;
			}
		}

		
	}

	inline	void transform() {
		
		for (int k = 0; k < count; k++)
			qps[k].transform();
	}

	inline	void current(int64_t n, complex_t* y) {
		for (int k = 0; k < count; k++)
			qps[k].current(n,y);
	}
	/*
	template<class Float>
	inline	void add_qp(int64_t mode,complex_t&z,int64_t n, Float* y) {

		if (!mode) {
			memset(y, 0, sizeof(*y)*n);
		}

		for (int k = 0; k < count; k++)
			qps[k].add_laplace(z, n, y);
	}
	*/

	inline	void apply_qp(int64_t mode, complex_t&z, int64_t n, complex_t* y) {
		//return ;
		int k = 0;
		if (mode) {
			if (!count) {
				memset(y, 0, sizeof(*y)*n);
				return;
			}
			else {
				qps[k].apply_laplace_first(z, n, y);
				k++;
			}

		}

		for (; k < count; k++)
			qps[k].apply_laplace(z, n, y);
	}
	inline	void reset() {
		for (int k = 0; k < count; k++)
			qps[k].reset();
	}


	long invoke(int64_t icmd, void* params) {
		
		byte_t cmd = byte_t(icmd);
		if (cmd == 'a') { // set
			if (!params)
				return -1;
			
			qp_source_apply_t& ss = *((qp_source_apply_t*)params);
			
			complex_t z = complex_t(ss.z[0], ss.z[1]);

			apply_qp(ss.mode, z, ss.n, (complex_t*)ss.y);
			/*
			if (ss.type[0] == 'r')
				add_qp(z, ss.n, (double*)ss.y);
			else
				add_qp(z, ss.n, (complex_t*)ss.y);
				*/

			return 0;
		}
		else if (cmd == 'g') { // set
			if (!params)
				return -1;

			qp_source_apply_t& ss = *((qp_source_apply_t*)params);		

			current( ss.n, (complex_t*)ss.y);

			return 0;
		}		
		else if  (cmd == 't') {
			transform();
			return 0;
		}
		else if (cmd == 'r') {
			reset();
			return 0;
		}
		else if ((cmd == 'l')||(cmd == 's')) {
			
			if (!params)
				return -1;
			qp_source_list_t*qpl = (qp_source_list_t*)params;			
			init(qpl);
			return 0;			
		}
		return -1;
	}

	~qp_context_t() {

	};
	qp_source_t* qps;	
	std::vector< buffer_t<complex_t> > vQc,vQct;	
	std::vector< qp_source_t > vqp;
	
	int count;
};

