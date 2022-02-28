//
#pragma once 
#include <stdint.h>

struct nls_data_t {
// run data
	char ftype[8];	

	void* pxx;
	int64_t rep;
	int64_t pp;	

	double g;
	double alpha_r;
	double alpha_i;

// method data
	double dt;
	int64_t nt;


	

	int64_t n;
	int64_t m;

	int64_t omagnus;
	
// create data;
	int64_t N;	

	double alpha_nl_r;
	double alpha_nl_i;

	double w;
	int64_t ncorr;
	int64_t flags;
	// out

	int64_t step_count;
	double step_time;
	int64_t masks;
	void* opts;
	
};

struct pade_step_data_t {
	void* x;
	void* a;
	void* b;
	void* c;
};

struct shuttle_data_t {
	double az_r;
	double az_i;
	void* pa;
	void* pb;
	void* pc;
	void* px;
	void* pxo;
	int64_t N;	
	int64_t M;
};