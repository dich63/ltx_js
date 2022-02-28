#pragma once
#include "invoke_context.h"
#include "matrix_invoke/matrix_context.h"

#define SMP_FMT 0xFF
#define SMP_ALL 0xFFFFFF
#define SMP_COO 0
#define SMP_CSC 1
#define SMP_CRC 2

#define SMP_RE 0x010000
#define SMP_CX 0x000000

#define SMP_ISL 0x0100
#define SMP_ISI 0x0000

#pragma push(pack)
#pragma pack(8)

struct perf_times_t {
	double symbolic;
	double numeric;
	double solve;
	double mean_solve;
};

struct klu_params_t {
	void* sp_context;
	void* common;	
};

struct klu_x_solve_t {
	int64_t nrs;
	void* data;
};

struct klu_params_solve_t {
	
	int64_t n; 
	int64_t nrs;
	void** pp_in;
	void** pp_out;

};

struct sparse_matrix_gaxpy_t {
	void* x;
	void* y;
};

struct sparse_matrix_rescale_t {
	double zz[2];
};



struct sparse_matrix_params_t {

	int64_t flags;

	int64_t n;
	int64_t m;
	int64_t nzmax;
	int64_t nz;
	

	void* i;
	void* p;
	void* x;

};

struct matrix_lc_t {
	int64_t c;
	int64_t opts;
	sparse_matrix_params_t* mx;
	double* a;// complex value (2*c*sizeof(double))	
};

struct sparse_jet_create_t {
	int64_t nd;
	base_context_t** ppcsh;
};

struct sparse_jet_info_t {
	int64_t count;
	int64_t n;
	int64_t nz;
	int64_t fse;
	void* pp_sp;
};


struct sparse_jet_jet_t {
	char type[8];
	double z[2];
	int64_t count;	
	int64_t n;
	void* pp_f;
	void* pp_jet;
};

struct sparse_jet_gaxpy_jet_t {

	char type[8];
	double z[2];	
	void* pp_f;
	void* py;
	void* pys;	
	void* pbuf;

};


struct sparse_jet_polyval_t {	
	double z[2],s[2];	
	base_context_t** pp_out;
};

struct sparse_jet_gaxpy_t {
	void *pp_jet;
	void * p_out;
};

struct qp_source_apply_t {
	int64_t mode;
	int64_t n;
	double z[2];
	void* y;
};

struct qp_source_param_t{
	char id[16];
	int64_t is_active;
	int64_t n;
	void* b;
	double z[2];	
	int64_t degQ;
	void *Q;
	void *transform_step;

};

struct qp_source_list_t {
	int64_t count;
	qp_source_param_t** params;
};
#pragma pop(pack)

