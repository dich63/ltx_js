#pragma once
// matrix_context.h

#define SMP_FMT 0xFF
#define SMP_COO 0x0
#define SMP_CSC 0x1
#define SMP_CRC 0x2

#define SMP_3D 0x20
#define SMP_DS 0x10
#define SMP_SC 0x40
#define SMP_SP 0x00

#define SMP_RE 0x010000
#define SMP_CX 0x000000

#define SMP_ISL 0x0100
#define SMP_ISI 0x0000


#pragma push(pack)
#pragma pack(8)

struct matrix_gaxpy_t {
	int64_t typesxy;
	void* x;
	void* y;
};

struct matrix_rescale_t {
	double zz[2];
};


struct matrix_params_t {

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
	int64_t opts;
	int64_t count;	
	matrix_params_t* mx;
	void* a;// complex value (2*c*sizeof(double))	
};




#pragma pop(pack)