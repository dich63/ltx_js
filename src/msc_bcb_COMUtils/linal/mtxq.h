#pragma once

#include <mkl.h>


template <class _N>
struct v_t
{

	v_t(_N _v=_N()):v(_v){};

	operator _N&()
	{
		return v;
	}


	_N* operator &()
	{
		return &v;
	}

	_N v;
};


inline v_t<int> get_iwork_size(int mm)
{
	MKL_Complex16 szz;
	int info,iwork=-1;
	zgetri(&mm,0,&mm,0,&szz,&iwork,&info);
	if(info) return -1;
	return int(szz.real);
}


inline int matrix_inverse(float* pmm,int sz,int *ipiv,float* pwork,int szwork)
{
	int info;
	sgetrf(&sz,&sz,pmm,&sz,ipiv,&info);
	if(!info) sgetri(&sz,pmm,&sz,ipiv,pwork,&szwork,&info);
	return info;
};

inline int matrix_inverse(double* pmm,int sz,int *ipiv,double* pwork,int szwork)
{
	int info;
	dgetrf(&sz,&sz,pmm,&sz,ipiv,&info);
	if(!info) dgetri(&sz,pmm,&sz,ipiv,pwork,&szwork,&info);
	return info;
};

//sgemm("n","n",&lda,&lda,&lda,&alpha,a0,&lda,b,&lda,&beta,c,&lda);


inline void matrix_matrix_affine(float* mm_1,float* mm_2,float* mm_3_out,int sz,float alpha=1,float beta=0)
{
	sgemm("n","n",&sz,&sz,&sz,&alpha,mm_1,&sz,mm_2,&sz,&beta,mm_3_out,&sz);
};

inline void matrix_matrix_affine(double* mm_1,double* mm_2,double* mm_3_out,int sz,double alpha=1,double beta=0)
{
	dgemm("n","n",&sz,&sz,&sz,&alpha,mm_1,&sz,mm_2,&sz,&beta,mm_3_out,&sz);
};

inline void matrix_vector_affine(float* mm,float* x,float* y_out,int sz,float alpha=1,float beta=0,int incx=1,int incy=1)
{
	sgemv("n",&sz,&sz,&alpha,mm,&sz,x,&incx,&beta,y_out,&incy);
};


inline void matrix_vector_affine(double* mm,double* x,double* y_out,int sz,double alpha=1,double beta=0,int incx=1,int incy=1)
{
	//
	dgemv("n",&sz,&sz,&alpha,mm,&sz,x,&incx,&beta,y_out,&incy);
	//	int sz1=1;
	//	dgemm("n","t",&sz,&sz1,&sz,&alpha,mm,&sz,x,&sz1,&beta,y_out,&sz);
};


template <class T,class numberT>
struct mtxq_base_t
{
	typedef typename numberT value_t;

	struct dims2_t{int n[2];};
	union{
		struct{int nn[2];};   
		struct{int n,m;};   
		struct{ dims2_t dims;};
		};
	value_t* data; 
/*
	inline number_t* operator[](int l)
	{
		return data[l];
	}
*/
	mtxq_base_t():n(0){};
	mtxq_base_t(const mtxq_base_t& o):n(0)
	{
      assign(o);    
	};
	mtxq_base_t( mtxq_base_t& o):n(0)
	{
       assign(o);
	};

inline bool empty()
{
	return nn[0]==0;
};

inline bool is_null()
{
	return nn[0]==0;
};

inline size_t fullsize(dims2_t siz)
{
	//if(L==2) siz*=siz;

	return siz.n[0]*siz.n[1]*sizeof(value_t);
}
inline static dims2_t setdims(int nn=0,int mm=1)
{
	dims2_t t={nn,mm};
	return t;
}
inline int resize(dims2_t siz)
{
       
	int sizb=fullsize(siz);
	if(sizb)
	{
		if(n) ::free(data);
 		 return n=0;
	}
	if(empty()) 	   
		  data=(value_t*)::malloc(sizb);
	else
	{
		value_t* p=(value_t*)::realloc(data,sizb);
		if(p) data=p;
		else ::free(data);
	}
		dims=(data)?siz:setdims();
	return sizb;
}

inline    int reload(dims2_t n,numberT* pdata=0) 
{
	T* thisT=static_cast<T*>(this);
	int sz=thisT->resize(n);
	if(sz&&pdata) memcpy(data,pdata,sz);
	return sz;
};
inline    mtxq_base_t& assign(int _n,int _m,numberT* pdata=0)
{
	reload(setdims(_n,_m),pdata);
	return *this;
}

template <class Other>
inline     mtxq_base_t& assign(const mtxq_base_t<Other,value_t>& m)
	{
		
/*
		int nn=m.n;
		T* thisT=static_cast<T*>(this);
		 if(nn=thisT->resize(nn))	memcpy(data,m.data,nn);
		 */
         //reload(m.dims,m.data);
		reload(setdims(m.n,m.n),m.data);
		return *this;
	}

inline void clear()
 {
	T* thisT=static_cast<T*>(this);
	thisT->resize(setdims());
 }
~mtxq_base_t()
 {
	clear();
 };
};






template<class numberT,int mem_align=16>
struct mtx_mkl_t:mtxq_base_t<mtx_mkl_t<numberT,mem_align>,numberT>
{
	inline int resize(dims2_t siz)
	{
		int sizb=fullsize(siz);
		if(sizb==0)
		{
			if(n) ::mkl_free(data);
			return n=0;
		}
		

		if(sizb>fullsize(dims))
		{
			if(n) ::mkl_free(data);
			data=(value_t*)::mkl_malloc(sizb,mem_align);
		}

		dims=(data)?siz:setdims();
		return sizb;
	}
	
	mtx_mkl_t(){};
	mtx_mkl_t(int n,int m=1,numberT* pdata=0)
	{
		assign(n,m,pdata);
	}

	template <class Other>
	inline  mtx_mkl_t& operator=(const mtxq_base_t<Other,value_t>& m)
	{
		assign(m);
		return *this;
	}
};


template<class numberT,int mem_align=16>
struct matrix_mkl_t:mtx_mkl_t<numberT,mem_align>
{
	matrix_mkl_t(){};
	matrix_mkl_t(int n,numberT* pdata):mtx_mkl_t(n,n,pdata){};

/*
	inline  matrix_mkl_t& operator=(matrix_mkl_t& m)
	{
		assign(m);
		return *this;
	}

	inline    mtxq_base_t& assign(int _n,numberT* pdata)
	{
          return  assign(_n,_n,pdata);
	}
	*/
	inline  matrix_mkl_t& operator=(const matrix_mkl_t& m)
	{
		assign(m);
		return *this;
	}

//	/*
	template <class Other>
	inline  matrix_mkl_t& operator=(const mtxq_base_t<Other,value_t>& m)
	{
		 assign(m);
		 return *this;
	}
//	*/
};

template<class numberT,int mem_align=16>
struct vector_mkl_t:mtx_mkl_t<numberT,mem_align>
{
	vector_mkl_t(){};
	vector_mkl_t(int n,numberT* pdata):mtx_mkl_t(n,1,pdata){};

	inline     vector_mkl_t& operator=(const vector_mkl_t& m)
	{
		assign(m);
		return *this;
	}

	template <class Other>
	inline     vector_mkl_t& operator=(const mtxq_base_t<Other,value_t>& m)
	{
		 assign(m);
		return *this;
	}

/*
	inline    mtxq_base_t& assign(int _n,numberT* pdata)
	{
		return  assign(_n,1,pdata);
	}
//*/
};

//#include <vector>

template<class mklT,int Size=8>
struct cache_mkl_t
{
	enum	{		size=Size	};

	typedef typename mklT item_t;
	typedef typename mklT value_t;
	typedef typename mklT value_type;

	item_t item[Size];

	inline  item_t& operator [](int n)
	{
		return item[n];
	}
       cache_mkl_t(int N,int M=1)
	   {
          for(int n=0;n<Size;n++)
			  item[n].assign(N,M,0);
	   }
};


template <class number_T,class vector_T,class matrix_T>
struct mkl_matrix_operation_t
{

	typedef typename number_T number_t;
	typedef typename matrix_T matrix_t;
	typedef typename vector_T vector_t;

	inline   static int matrix_invert(matrix_t& a,matrix_t& cache,matrix_t& cache2)
	{
        //inline int matrix_inverse(double* pmm,int sz,int *ipiv,double* pwork,int szwork)
		int n=a.n;
		if(n!=a.m) return -1;
		int szwork=n*n;
		int* ipid=(int*)(cache2.data);
		return ::matrix_inverse(a.data,n,ipid,cache.data,szwork);
		
	}

//	inline void matrix_matrix_affine(double* mm_1,double* mm_2,double* mm_3_out,int sz,double alpha=1,double beta=0)
//	inline void matrix_vector_affine(float* mm,float* x,float* y_out,int sz,float alpha=1,float beta=0,int incx=1,int incy=1)

	inline 	static void matrix_matrix_affine(matrix_t& m1,matrix_t& m2,matrix_t& m_out,number_t alpha=1, number_t beta=0)
	{
		::matrix_matrix_affine(m1.data,m2.data,m_out.data,m1.n,alpha,beta);
	};

	inline 	static void matrix_vector_affine(matrix_t& mm,vector_t& v,vector_t& v_out,vector_t& cache,number_t alpha=1 ,number_t beta=0)
	{
		
		::matrix_vector_affine(mm.data,v.data,v_out.data,mm.n,alpha,beta);
	};

	inline 	static void vector_linear_combination(vector_t& m1,vector_t& m2,vector_t& m_out,number_t alpha=1 ,number_t beta=1)
	{
		//m_out=alpha*m1+beta*m2;   
		for(int i=0;i<m1.n;i++)
			m_out.data[i]=alpha*m1.data[i]+beta*m2.data[i];
	}

	inline 	static void matrix_mul_number(matrix_t& m,matrix_t& m_out,number_t alpha)
	{
		for(int i=0;i<m.n;i++)
			m_out.data[i]=alpha*m.data[i];
	};
	inline 	static void vector_mul_number(vector_t& m,vector_t& m_out,number_t alpha)
	{
		for(int i=0;i<m.n;i++)
			m_out.data[i]=alpha*m.data[i];
	};

	inline 	static void matrix_linear_combination(matrix_t& m1,matrix_t& m2,matrix_t& m_out,number_t alpha=1 ,number_t beta=1)
	{
		for(int i=0;i<m1.n*m1.m;i++)
			m_out.data[i]=alpha*m1.data[i]+beta*m2.data[i];
	}

	inline 	static void vector_add(vector_t& m,vector_t& m_out)
	{
		//m_out+=m;
		for(int i=0;i<m.n;i++)
			m_out.data[i]+=m.data[i];
	};
/*
	inline 	static void vector_affine(vector_t& m,vector_t& m_out,number_t alpha=1, number_t beta=0)
	{
		//number_t r= beta*m_out+alpha*m;
		//m_out=r;
	};
*/

};
