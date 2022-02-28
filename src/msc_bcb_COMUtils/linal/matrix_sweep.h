#pragma once


/*
void solveMatrix (int n, double *a, double *b, double *c, double *v, double *x)
{
        ///**
         //* n - число уравнений (строк матрицы)
         //* a - главная диагональ матрицы A
         //* b - диагональ, лежащая под главной
         //* c - диагональ, лежащая над главной
         //* v - правая часть (столбец)
         //* x - решение, массив x будет содержать ответ
         //
        for (int i = 1; i < n; i++)
        {
                double m = b[i]/a[i-1];
                a[i] = a[i] - m*c[i-1];
                v[i] = v[i] - m*v[i-1];
        }
 
        x[n-1] = v[n-1]/a[n-1];  
 
        for (int i = n - 2; i >= 0; i--)  
                x[i]=(v[i]-c[i]*x[i+1])/a[i];
}
*/



template<class T>
struct is_value_t {
	typedef char (&yes)[1];
	typedef char (&no)[2];

	template<typename O>
	static yes use(O*,typename O::value_t* p=0);
	static no use(...);
	enum { result=sizeof( use((T*)0))==sizeof(yes) };
};
template<typename T,typename D=T,typename bool f=is_value_t<T>::result>
struct value_t_of 
{
	typedef typename D value_t;
};
template<typename T,typename D>
struct value_t_of<T,D,true> 
{
	typedef typename T::value_t value_t;
};


template<class T>
struct is_value_type {
	typedef char (&yes)[1];
	typedef char (&no)[2];

	template<typename O>
	static yes use(O*,typename O::value_type* p=0);
	static no use(...);
	enum { result=sizeof( use((T*)0))==sizeof(yes) };
};


template<typename T,typename D=T,typename bool f=is_value_type<T>::result>
struct value_type_of 
{
	typedef typename D value_type;
	typedef typename value_type value_t;
};

template<typename T,typename D>
struct value_type_of<T*,D,false> 
{
	typedef typename T value_type;
	typedef typename value_type value_t;
};


template<typename T,typename D>
struct value_type_of<T,D,true> 
{
	typedef typename T::value_type value_type;
	typedef typename value_type value_t;
};



//template<class T,class Matrix_T,class Vector_T,template<class> class MatrixArray,template<class,class> class Operation>


template<class T,class Matrix_T,class Vector_T,class MatrixArray,class Operation>
struct sweep_base_t
{
	typedef typename Matrix_T matrix_t;
	typedef typename Vector_T vector_t;
	//typedef typename Operation<Matrix_T,Vector_T> op_t;
	//typedef typename MatrixArray<Matrix_T> matrix_array_t;
	typedef typename Operation op_t;
	typedef typename MatrixArray matrix_array_t;
	matrix_array_t* psweep_data;

	sweep_base_t(matrix_array_t* pmao=0):psweep_data(pmao){};

	template<class BandT,class MatrixCacheT>
	int initialize(int n,BandT& band,MatrixCacheT& cache)
	{
		T* _this=static_cast<T*>(this);
		return _this->inner_initialize(band,n,cache);
	}

	template<class BandT,class MatrixCacheT>
	int initialize(BandT& band,MatrixCacheT& cache)
	{
		T* _this=static_cast<T*>(this);
		return _this->inner_initialize(band,band.size(),cache);
	}

	template<class VectorArrayT, class VectorCacheT>
	int make(VectorArrayT& v,VectorCacheT& cache)
	{
		T* _this=static_cast<T*>(this);
		return _this->inner_make(v,cache);
	}
	template<class VectorArrayT, class VectorCacheT>
	int inner_make(VectorArrayT& xx,VectorCacheT& cache)
	{
		//typedef VectorCacheT::value_t temp_vector_t;
		//typedef vector_t temp_vector_t;
		typedef typename value_type_of<VectorCacheT,vector_t>::value_type temp_vector_t;

		matrix_array_t& sweep_data=*psweep_data;

		int n=sweep_data.size();

		temp_vector_t& v0=cache[0];
		temp_vector_t& xcia=cache[0];
		temp_vector_t& xia=cache[3];
		
		temp_vector_t& vm=cache[1];
		temp_vector_t& v=cache[2];
		temp_vector_t& x=cache[1];
		temp_vector_t& xp=cache[2];

		temp_vector_t& tmp=cache[4];
		
		v0=xx[0];
		vm=v0;

		for (int i = 1; i < n; i++)
		{
			//xx[i] = xx[i] - m*xx[i-1];
			v=xx[i];
			op_t::matrix_vector_affine(sweep_data[i].m,vm,v,tmp,-1,1);
			
    		xx[i]=v;
			vm=v;
		}

        xp=xx[n-1];
		op_t::matrix_vector_affine(sweep_data[n-1].ia,vm,xp,tmp);
		xx[n-1]=xp;

		for (int i = n - 2; i >= 0; i--)  
		{
			// xca= (c[i]/a[i])*x[i+1]
            op_t::matrix_vector_affine(sweep_data[i].cia,xp,xcia,tmp);
            x=xx[i];
			op_t::matrix_vector_affine(sweep_data[i].ia,x,xia,tmp);
			op_t::vector_add(xia,xcia);
			xx[i]=xcia;
			if(i>0) xp=xcia;
		}
			//x[i]=(v[i]-c[i]*x[i+1])/a[i];


		
		return 0;
	}


	template<class BandT,class MatrixCacheT>
	int inner_initialize(BandT& band,int n,MatrixCacheT& cache)
	{
		int err=0;
		typedef BandT band_t;
		//typedef MatrixCacheT::value_t temp_matrix_t;
		//typedef matrix_t temp_matrix_t;
		typedef typename value_type_of<MatrixCacheT,matrix_t>::value_type temp_matrix_t;

		const int index_a=1,index_b=0,index_c=2;

		matrix_array_t& sweep_data=*psweep_data;
		
		//int n=band.size();
		sweep_data.resize(n);


        temp_matrix_t &a=cache[0];
		temp_matrix_t &m=cache[1];
		temp_matrix_t &c=cache[2];
		temp_matrix_t &tmp2=cache[3];
		temp_matrix_t &tmp=cache[4];



          
		  a=band[0][index_a];
		  tmp=a;
		  tmp2=tmp;
          m=tmp;
    	  
          
		if(err=op_t::matrix_invert(a,tmp,tmp2)) return err;

		sweep_data[0].ia=a;


		for(int i=1;i<n;i++)
		{
			
               c=band[i-1][index_c];

			   op_t::matrix_matrix_affine(c,a,tmp,-1);
			   sweep_data[i-1].cia=tmp;

			//  m=b[1]/a[i-1]

			


			op_t::matrix_matrix_affine(band[i][index_b],a,m);

			sweep_data[i].m=m;


			
			// a=band[i].a
			a=band[i][index_a];
		

			// a = a - m*c[i-1];
			op_t::matrix_matrix_affine(m,c,a,-1,1);
		
			// a=1/a
			if(err=op_t::matrix_invert(a,tmp,tmp2)) return err;
			// 		   

			   sweep_data[i].ia=a;

		   //		sweep_data[0].cia=a;

		

		}



       return err;
	}



};

template <class L>
bool is_null(const L& m,void  (  L::*ptm)()=0)
{
	return m.is_null();
}
template <class L>
bool is_null(const L& m,...)
{
	return m==0;
}

template <class L>
void clear_matrix(L& m,void  (  L::*ptm)()=0)
{
	 m.clear();
}
template <class L>
void clear_matrix( L& m,...)
{
	 m=0;
}


#include <vector>

template <class number_T,class vector_T=number_T,class matrix_T=number_T>
struct overloaded_matrix_op_t
{

	typedef typename number_T number_t;
	typedef typename matrix_T matrix_t;
	typedef typename vector_T vector_t;

	inline   static int matrix_invert(matrix_t& a,matrix_t& cache,matrix_t& cache2) { a=1/a;return 0;}
	inline 	static void matrix_matrix_affine(matrix_t& m1,matrix_t& m2,matrix_t& m_out,number_t alpha=1, number_t beta=0)
	{
		number_t r= beta*m_out+alpha*(m1*m2);
		m_out=r;
	};


	inline 	static void matrix_vector_affine(matrix_t& m1,vector_t& m2,vector_t& m_out,vector_t& cache,number_t alpha=1 ,number_t beta=0)
	{
		number_t r= beta*m_out+alpha*(m1*m2);
		m_out=r;
	};

	inline 	static void vector_linear_combination(vector_t& m1,vector_t& m2,vector_t& m_out,number_t alpha=1 ,number_t beta=1)
	{
        m_out=alpha*m1+beta*m2;   
	}

	inline 	static void matrix_linear_combination(matrix_t& m1,matrix_t& m2,matrix_t& m_out,number_t alpha=1 ,number_t beta=1)
	{
		m_out=alpha*m1+beta*m2;   
	}

	inline 	static void matrix_mul_number(matrix_t& m,matrix_t& m_out,number_t alpha)
	{
		m_out=alpha*m;
	};
    inline 	static void vector_mul_number(vector_t& m,vector_t& m_out,number_t alpha)
	{
        m_out=alpha*m;
	}

	inline 	static void vector_add(vector_t& m,vector_t& m_out)
	{
		m_out+=m;
	};
/*
	inline 	static void vector_affine(vector_t& m,vector_t& m_out,number_t alpha=1, number_t beta=0)
	{
		number_t r= beta*m_out+alpha*m;
		m_out=r;
	};
*/

};

template <class number_T,class vector_T=number_T,class matrix_T=number_T
,typename OperationT=overloaded_matrix_op_t<number_T,vector_T,matrix_T>>
struct sweep_matrix_simple_base_t
{

	typedef typename number_T number_t;
	typedef typename matrix_T matrix_t;
	typedef typename vector_T vector_t;
	typedef typename OperationT op_t;

	struct band_triplet_t
	{
		matrix_t r[3];
		inline matrix_t& operator[](int n){ return r[n];};
		band_triplet_t(){};
		band_triplet_t(const matrix_t& b,const matrix_t& a,const matrix_t& c)
		{
			r[0]=b;
			r[1]=a;
			r[2]=c;
		}




	} ;

	struct matrix_triplet_t
	{
		matrix_t m;
		matrix_t ia;
		matrix_t cia;
	} ;

	typedef std::vector<band_triplet_t> band_t;
	typedef std::vector<matrix_triplet_t> matrix_array_t;


	//std::vector< vector_t> vector_cache;
	//



	struct sweep_t:sweep_base_t<sweep_t,matrix_t,vector_t,matrix_array_t,op_t>{};

	sweep_t sweep;
	matrix_array_t sweep_data;
	int err;

	sweep_matrix_simple_base_t()
	{
		err=-1;
		//sweep_data.resize(N);

		//vector_cache.resize(8);
		sweep.psweep_data=&sweep_data;
	}

	template <class BandT,class Cache>
	sweep_matrix_simple_base_t& initialize(int n,BandT& band,Cache& matrix_cache)
	{

		//matrix_t  tmp[4];

		err=sweep.initialize(n,band,matrix_cache); 

		return *this;
	}

	template <class BandT,class Cache>
	sweep_matrix_simple_base_t& initialize(BandT& band,Cache& matrix_cache)
	{

		err=sweep.initialize(band,matrix_cache); 
		return *this;
	}

	template <class BandT>
	sweep_matrix_simple_base_t& initialize(int n,BandT& band)
	{

		//matrix_t  tmp[4];
		std::vector< matrix_t> matrix_cache(8);
		return initialize(n,band,matrix_cache);
	}

	template <class BandT>
	sweep_matrix_simple_base_t& initialize(BandT& band)
	{
		std::vector< matrix_t> matrix_cache(8);
		return initialize(band,matrix_cache);
	}

	template <class VArrayT,class Cache>
	bool operator()(VArrayT& vv,Cache& cache,int count=1)
	{


		for(int n=0;n<count;n++)
		{
			if(err) break;
			err=sweep.make(vv,cache); 
		}
		return err==0;
	}

	template <class VArrayT>
	bool operator()(VArrayT& vv,int count=1)
	{
		vector_t  tmp[8];
		return (*this)(vv,tmp,count);
	}

	operator bool(){return err==0;};
};

template <class number_T,class vector_T=number_T,class matrix_T=number_T,template<class,class,class> class OperationT=overloaded_matrix_op_t >
struct sweep_matrix_simple_t
{

typedef typename number_T number_t;
typedef typename matrix_T matrix_t;
typedef typename vector_T vector_t;
typedef typename OperationT<number_t,vector_t,matrix_t> op_t;

   struct band_triplet_t
   {
	   matrix_t r[3];
	   inline matrix_t& operator[](int n){ return r[n];};
	   band_triplet_t(){};
	   band_triplet_t(const matrix_t& b,const matrix_t& a,const matrix_t& c)
	   {
		   r[0]=b;
		   r[1]=a;
		   r[2]=c;
	   }




   } ;

   struct matrix_triplet_t
   {
	   matrix_t m;
	   matrix_t ia;
	   matrix_t cia;
   } ;

   typedef std::vector<band_triplet_t> band_t;
   typedef std::vector<matrix_triplet_t> matrix_array_t;

   
   //std::vector< vector_t> vector_cache;
   //



   struct sweep_t:sweep_base_t<sweep_t,matrix_t,vector_t,matrix_array_t,op_t>{};

   sweep_t sweep;
   matrix_array_t sweep_data;
   int err;

   sweep_matrix_simple_t()
   {
	   err=-1;
      //sweep_data.resize(N);
	  
	  //vector_cache.resize(8);
	  sweep.psweep_data=&sweep_data;
    }

   template <class BandT,class Cache>
   sweep_matrix_simple_t& initialize(int n,BandT& band,Cache& matrix_cache)
   {

	  //matrix_t  tmp[4];
	  
      err=sweep.initialize(n,band,matrix_cache); 

	  return *this;
   }

   template <class BandT,class Cache>
   sweep_matrix_simple_t& initialize(BandT& band,Cache& matrix_cache)
   {
	   
	   err=sweep.initialize(band,matrix_cache); 
	   return *this;
   }

   template <class BandT>
   sweep_matrix_simple_t& initialize(int n,BandT& band)
   {

	   //matrix_t  tmp[4];
	   std::vector< matrix_t> matrix_cache(8);
	   return initialize(n,band,matrix_cache);
   }

   template <class BandT>
   sweep_matrix_simple_t& initialize(BandT& band)
   {
	   std::vector< matrix_t> matrix_cache(8);
	   return initialize(band,matrix_cache);
   }

   template <class VArrayT,class Cache>
   bool operator()(VArrayT& vv,Cache& cache,int count=1)
   {
	   
	   
	   for(int n=0;n<count;n++)
	   {
		 if(err) break;
	     err=sweep.make(vv,cache); 
	   }
	   return err==0;
   }

   template <class VArrayT>
   bool operator()(VArrayT& vv,int count=1)
   {
          vector_t  tmp[8];
		  return (*this)(vv,tmp,count);
   }

   operator bool(){return err==0;};
 };




