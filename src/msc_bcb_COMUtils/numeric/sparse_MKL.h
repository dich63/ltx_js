#pragma once


#include "mkl_pardiso.h"
#include "mkl_types.h"
#include "mkl_spblas.h"
#include "mkl.h"
#include <vector>
#include <complex>

namespace mkl_holder_utils {

	typedef MKL_INT i_t;
	typedef unsigned char  byte_t;
	//typedef MKL_Complex16  complex_t;
	typedef std::complex<double> complex16_t;

	enum
	{
		real8_id=0,
		real4_id=1,
		complex8_id=2,
		complex4_id=3
	};

	inline int get_type(double* p){ return real8_id;}
	inline int get_type(float* p){ return real4_id;}
	inline int get_type(MKL_Complex16* p){ return complex8_id;}
	inline int get_type(MKL_Complex8* p){ return complex4_id;}




	template <class ValueT=double >
	struct matrix_DIA_base_t
	{
		enum{
			blank_offs=1
		};
		typedef ValueT value_t;
		value_t *a;	
		i_t *distance;
		i_t n,nnz,ndiags;
		byte_t vt;

	};

	template <class ValueT=double >
	struct matrix_DIA_t:matrix_DIA_base_t<ValueT>
	{
		matrix_DIA_t(i_t _n,i_t _ndiags=0,i_t _nnz=0):matrix_DIA_base_t<ValueT>()
		{
			vt=get_type((value_t*)0);
			n=_n;
			a=0;
			distance=0;
			//nnz=_nnz;
			nnz_rezise(_nnz,_ndiags);
		};


		inline i_t nnz_rezise(i_t _nnz,i_t _ndiags)
		{

			i_t ns=(nnz=_nnz)+blank_offs;
			i_t nsd=(ndiags=_ndiags)+blank_offs;


			if(ns)
			{	
				buf0.resize(ns);						
				a=&buf0[0]; 

		 }
			if(nsd)
			{	
				buf1.resize(nsd);						
				distance=&buf1[0]; 

		 }

			return nnz;

		}



		static void onexit(void* p)
		{
			delete   (matrix_DIA_t*)p;
		}

		std::vector<value_t> buf0;
		std::vector<i_t> buf1;

	};


	template <class ValueT=double >
	struct matrix_COO_base_t
	{
		enum{
			blank_offs=1
		};
		typedef ValueT value_t;
		value_t *a;	
		i_t *colind,*rowind;
		i_t n,nnz;
		byte_t vt;

	};

	template <class ValueT=double >
	struct matrix_COO_t:matrix_COO_base_t<ValueT>
	{
		matrix_COO_t(i_t _n,i_t _nnz=0):matrix_COO_base_t<ValueT>()
		{
			vt=get_type((value_t*)0);
			n=_n;
			//nnz=_nnz;
			nnz_rezise(_nnz);
		};


		inline i_t nnz_rezise(i_t _nnz)
		{

			i_t ns=(nnz=_nnz)+blank_offs;
			if(ns)
			{	
				buf0.resize(ns);		
				buf1.resize(ns);
				buf2.resize(ns);
				a=&buf0[0];
				colind=&buf1[0];
				rowind=&buf2[0];

			}
			return nnz;

		}


		inline void set_identity()
		{
			nnz_rezise(n);  
			for(int k=0;k<n;k++)
			{
				a[k]=1;
				colind[k]=k+1;
				rowind[k]=k+1;
			}

		}


		static void onexit(void* p)
		{
			delete   (matrix_COO_t*)p;
		}

		std::vector<value_t> buf0;
		std::vector<i_t> buf1,buf2;

	};


	template <class ValueT=double >
	struct matrix_CRS_base_t
	{
		enum{
			blank_offs=1
		};

		typedef ValueT value_t;
		value_t *a;	
		i_t *ja,*ia;
		i_t n,nnz;
		byte_t vt;

	};


	template <class ValueT=double >
	struct matrix_CRS_t:matrix_CRS_base_t<ValueT>
	{
		matrix_CRS_t(i_t _n,i_t _nnz=0):matrix_CRS_base_t<ValueT>()
		{
			vt=get_type((value_t*)0);
			n=_n;


			nnz_rezise( _nnz);

			buf2.resize(n+1+blank_offs);
			ia=&buf2[0];
		};

		inline i_t nnz_rezise(i_t _nnz)
		{

			i_t ns=(nnz=_nnz)+blank_offs;
			if(ns)
			{	
				buf0.resize(ns);		
				buf1.resize(ns);			
				a=&buf0[0];
				ja=&buf1[0];

			}
			return nnz;

		}


		static void onexit(void* p)
		{
			matrix_CRS_t*	_this=(matrix_CRS_t*)p;

			delete   _this;
		}

		std::vector<value_t> buf0;
		std::vector<i_t> buf1,buf2;

	};


	inline i_t dia2crs(matrix_DIA_t<double>* pdias,matrix_CRS_t<double>** ppcrs)
	{
		i_t info=0;
		if(!pdias) return info;

		i_t n=pdias->n,ndias=pdias->ndiags,nnz=pdias->nnz;
		i_t job[8]={1,1,1,0,0,0,0};
		double arm[1];
		i_t iarm[1],jarm[1];

		matrix_CRS_t<double>* pcrs=new matrix_CRS_t<double>(n,nnz);
		//mkl_zcsradd
		mkl_dcsrdia(job,&n,pcrs->a,pcrs->ja,pcrs->ia,pdias->a,&n,pdias->distance,&ndias,arm,iarm,jarm,&info);

		if(info==0)
		{
			pcrs->nnz=pcrs->ia[n]-1;
			*ppcrs=pcrs;
		}
		else delete pcrs;

		return info;
	}



	inline i_t coo2crs(matrix_COO_t<double>* pcoo,matrix_CRS_t<double>** ppcrs)
	{
		i_t info=-1;
		if(!pcoo) return info;

		i_t n=pcoo->n,nnz=pcoo->nnz;
		i_t job[8]={1,1,1,0,nnz};



		matrix_CRS_t<double>* pcrs=new matrix_CRS_t<double>(n,nnz);
		//mkl_zcsradd
		mkl_dcsrcoo(job,&n,pcrs->a,pcrs->ja,pcrs->ia,&nnz,pcoo->a,pcoo->rowind,pcoo->colind,&info);
		if(info==0)
			*ppcrs=pcrs;
		else delete pcrs;

		return info;
	}

	inline i_t crs2coo(matrix_CRS_t<double>* pcrs,matrix_COO_t<double>** ppcoo)
	{
		i_t info=-1;
		if(!ppcoo) return info;

		i_t n=pcrs->n,nnz=pcrs->nnz;
		i_t job[8]={0,1,1,0,nnz,3};



		matrix_COO_t<double>* pcoo=new matrix_COO_t<double>(n,nnz);
		//mkl_zcsradd
		mkl_dcsrcoo(job,&n,pcrs->a,pcrs->ja,pcrs->ia,&nnz,pcoo->a,pcoo->rowind,pcoo->colind,&info);
		if(info==0)
			*ppcoo=pcoo;
		else delete pcoo;

		return info;
	}


	inline MKL_Complex16 mul_cc(MKL_Complex16 a,MKL_Complex16 b)
	{
		//complex16_t ca(a.real,a.imag),cb(b.real,b.imag);

		MKL_Complex16 r={a.real*b.real-a.imag*b.imag,a.real*b.imag+a.imag*b.real};

		return r;

	}

	inline MKL_Complex16 div_cc(MKL_Complex16 a,MKL_Complex16 b)
	{
		//complex16_t ca(a.real,a.imag),cb(b.real,b.imag);

		//MKL_Complex16 r={a.real*b.real-a.imag*b.imag,a.real*b.imag+a.imag*b.real};
		double bb=double(1)/(b.real*b.real+b.imag*b.imag);
		MKL_Complex16 r={bb*(a.real*b.real+a.imag*b.imag),bb*(a.imag*b.real-a.real*b.imag)};

		return r;

	}



	/*
	inline i_t expand_to_complex(matrix_COO_base_t<double>* pmr,matrix_COO_t<MKL_Complex16>** ppmc)
	{
	i_t info=0;
	i_t n=pmr->n,nnz=pmr->nnz;

	matrix_COO_t<MKL_Complex16>* pmc=new matrix_COO_t<MKL_Complex16>(n,nnz);
	for(i_t i=0;i<nnz;i++)
	{
	matrix_COO_base_t<double>& mr=pmr[i];
	matrix_COO_base_t<MKL_Complex16>& mc=pmc[i];

	mc.colind[i]=mr.colind[i];
	mc.rowind[i]=mr.rowind[i];
	mc.a[i].real=mr.a[i];
	mc.a[i].imag=0;
	}

	*ppmc=pmc;

	return info;

	}
	*/

	inline i_t complex2real(matrix_CRS_base_t<MKL_Complex16>* pmc,matrix_CRS_t<double>** ppmr,bool fimag=false)
	{
		i_t info=0;
		i_t n=pmc->n,nnz=pmc->nnz;


		matrix_CRS_t<double>* pmr=new matrix_CRS_t<double>(n,nnz);

		matrix_CRS_base_t<double>& mr=*pmr;
		matrix_CRS_base_t<MKL_Complex16>& mc=*pmc;

		if(fimag)
		{
			for(i_t i=0;i<nnz;i++)
			{		
				mr.ja[i]=mc.ja[i];
				mr.a[i]=mc.a[i].imag;				
			}


		}
		else
		{
			for(i_t i=0;i<nnz;i++)
			{		
				mr.ja[i]=mc.ja[i];
				mr.a[i]=mc.a[i].real;
			}

		}



	

	 for(i_t i=0;i<n+1;i++)
	 	mr.ia[i]=mc.ia[i];


	*ppmr=pmr;
	 return info;
	}

inline i_t expand_to_complex(matrix_CRS_base_t<double>* pmr,matrix_CRS_t<MKL_Complex16>** ppmc,MKL_Complex16* alpha=0)
{
	i_t info=0;
	i_t n=pmr->n,nnz=pmr->nnz;



	matrix_CRS_t<MKL_Complex16>* pmc=new matrix_CRS_t<MKL_Complex16>(n,nnz);

	matrix_CRS_base_t<double>& mr=*pmr;
	matrix_CRS_base_t<MKL_Complex16>& mc=*pmc;		
	if(alpha)
	{
		MKL_Complex16 a=*alpha;
		MKL_Complex16 r={0,0};

		for(i_t i=0;i<nnz;i++)
		{
			r.real=mr.a[i];
			mc.ja[i]=mr.ja[i];
			mc.a[i]=mul_cc(r,a);

		}

	}
	else{
		for(i_t i=0;i<nnz;i++)
		{		
			mc.ja[i]=mr.ja[i];
			mc.a[i].real=mr.a[i];
			mc.a[i].imag=0;
		}

	}
	for(i_t i=0;i<n+1;i++)
		mc.ia[i]=mr.ia[i];


	*ppmc=pmc;

	return info;

}


inline void proj2D_transform(double *m,double *vx,double *vy){


	

	//cblas_daxpy(n,alpha,x,1,y,1); 
//	void cblas_dgemv(const enum CBLAS_ORDER order, const enum CBLAS_TRANSPOSE TransA, const int M, const int N, const double alpha, const double *A, const int lda, const double *X, const int incX, const double beta, double *Y, const int incY);
	double x[3]={vx[0],vx[1],1},y[3]={0,0,0};
	//cblas_dgemv(CblasRowMajor,CblasNoTrans,3,3,1,m,3,x,1,0,y,1);
	cblas_dgemv(CblasColMajor,CblasNoTrans,3,3,1,m,3,x,1,0,y,1);
	double g=double(1)/y[2];
	vy[0]=g*y[0];
	vy[1]=g*y[1];

}


inline void vector_add_real(i_t n,double *x,double *y,double alpha=1)
{
	
    
	i_t inc=1;

	cblas_daxpy(n,alpha,x,1,y,1); 


}


inline void vector_add(i_t n,MKL_Complex16 *x,MKL_Complex16 *y,MKL_Complex16* alpha=0)
{
	MKL_Complex16 ta={1,0};

	if(!alpha) alpha=&ta;
	i_t inc=1;

	cblas_zaxpy(n,alpha,x,1,y,1); 


}

inline void matrix_vector_mul(matrix_CRS_t<MKL_Complex16>* pm,MKL_Complex16 *x,MKL_Complex16 *y,MKL_Complex16* alpha=0,MKL_Complex16* beta=0)
{
	MKL_Complex16 ta={1,0},tb={0,0};

	if(!alpha) alpha=&ta;
	if(!beta) beta=&tb;
	char matdescra[]="G??F";

	i_t n=pm->n;

	i_t *pntrb=pm->ia;
	i_t *pntre=pntrb+1;


	/*
	This enables calling a routine that has values, columns, pointerB and pointerE as input parameters for a sparse matrix stored in the format accepted for the direct sparse solvers. For example, a routine with the interface:

	Subroutine name_routine(.... ,  values, columns, pointerB, pointerE, ...)
	can be called with parameters values, columns, rowIndex as follows:

	call name_routine(.... ,  values, columns, rowIndex, rowindex(2), ...).
	*/

	mkl_zcsrmv("N",&n,&n,alpha,matdescra,pm->a,pm->ja,pntrb,pntre,x,beta,y);

}


inline i_t linopcrs(matrix_CRS_t<MKL_Complex16>* pma,matrix_CRS_t<MKL_Complex16>* pmb,matrix_CRS_t<MKL_Complex16>** ppmc,MKL_Complex16* beta=0)
{
	i_t info=-1;

	i_t n=pma->n,job,sort,nzmax=0;

	matrix_CRS_t<MKL_Complex16>* pmc=new matrix_CRS_t<MKL_Complex16>(n);
	MKL_Complex16 t={1,0};
	if(!beta) beta=&t;

	//	  
	mkl_zcsradd("n",&(job=1),&(sort=0),&n,&n,pma->a,pma->ja,pma->ia,beta,pmb->a,pmb->ja,pmb->ia,pmc->a,pmc->ja,pmc->ia,&nzmax,&info); 
	// if(info>=0)
	{
		nzmax=pmc->nnz_rezise(pmc->ia[n]-1);		   
		mkl_zcsradd("n",&(job=2),&(sort=0),&n,&n,pma->a,pma->ja,pma->ia,beta,pmb->a,pmb->ja,pmb->ia,pmc->a,pmc->ja,pmc->ia,&nzmax,&info); 
		if(info==0)
			*ppmc=pmc;
		else delete pmc;

	}


	return info;
}


};