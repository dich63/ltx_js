#pragma once

#include "matrix_sweep.h"
#include "mtxq.h"


template <class number_T,class matrix_T=number_T,bool fpacked=false>
struct affine_matrix_t
{
	enum
	{
		mask_a=1,
		mask_b=2,
		mask_d=4
	};

	long flags;
	typedef typename number_T number_t;
	typedef typename matrix_T matrix_t;
	typedef typename matrix_T vector_t;

	matrix_t a,b;
	number_t d;
	affine_matrix_t():flags(0),d(1){};

	inline void clear()
	{
		if(fpacked)
		{
			clear_matrix(a);
			clear_matrix(b);
			clear_matrix(d);
		} 

		//flags=mask_a|mask_b|mask_d;
	}

	template <class M1,class M2>
	affine_matrix_t(const M1& m1,const M2& m2,number_t _d=1):flags(0),d(_d)
	{
		a=m1;
		b=m2;
		/*
		if(is_null(m1)) flags|=mask_a;
		else a=m1;
		if(is_null(m2)) flags|=mask_b;
		else b=m2;
		if(is_null(_d)) flags|=mask_d;
		*/
	};

	template <typename Other>
	affine_matrix_t& operator=(const Other& o)
	{

		flags=o.flags;
		a=o.a;
		 b=o.b;
		 d=o.d;


		return *this;
	}
	affine_matrix_t& assign(int N,int M=1,number_t* pa=0,number_t* pb=0,number_t _d=0)
	{
		//flags=mask_a|mask_b|mask_d;
		
		d=_d;
		a.assign(N,M,pa);
		b.assign(N,M,pb);
		return *this;

	}

};


template <class AffineMatrixT,class AffineVectorT,class Matrix_opT>
struct affine_matrix_op_t
{
	typedef typename  Matrix_opT op_t;
	typedef typename AffineMatrixT matrix_t;
	typedef typename AffineVectorT vector_t;
	typedef typename  op_t::number_t number_t;
	typedef typename  op_t::matrix_t matrix_block_t;

	inline   static int matrix_invert(matrix_t& a,matrix_t& cache,matrix_t& cache2)
	{ 
		long flags=a.flags;
		
		int err;
		if(err=op_t::matrix_invert(a.a,cache.a,cache2.a)) 
			return err;

		number_t d=1/a.d;
		a.d=d;

			op_t::matrix_matrix_affine(a.a,a.b,cache.a,-d);
			a.b=cache.a;
		

		return 0;
	}


	inline 	static void matrix_matrix_affine(matrix_t& m1,matrix_t& m2,matrix_t& m_out,number_t alpha=1,number_t beta=0)
	{
		
    		op_t::matrix_matrix_affine(m1.a,m2.a,m_out.a,alpha,beta);

		     m_out.d=beta*m_out.d+alpha*m1.d*m2.d;
			
			number_t beta_mul=beta;

			op_t::matrix_matrix_affine(m1.a,m2.b,m_out.b,alpha,beta_mul);
			beta_mul=1;			  

			op_t::matrix_linear_combination(m1.b,m_out.b,m_out.b,alpha*m2.d,beta_mul);


	};

	inline 	static void matrix_vector_affine(matrix_t& mm,vector_t& v,vector_t& v_out,vector_t& cache,number_t alpha=1 ,number_t beta=0)
	{
     
		op_t::matrix_vector_affine(mm.a,v.a,v_out.a,cache.a,alpha,beta);
		op_t::matrix_vector_affine(mm.b,v.b,v_out.a,cache.a,alpha,1);
		op_t::vector_linear_combination(v.b,v_out.b,v_out.b,alpha*mm.d,beta);
	};

	inline 	static void vector_add(vector_t& m,vector_t& m_out)
	{
		
		op_t::vector_add(m.a,m_out.a);
		op_t::vector_add(m.b,m_out.b);
		
	};


};






