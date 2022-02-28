// gauss_jordah.h
#pragma once




#include <vector>
template <class float_type>
struct gauss_jordan_t
{

gauss_jordan_t(){};

gauss_jordan_t(long n,long nmax=-1)
{
    reset( n,nmax);
}

gauss_jordan_t(const gauss_jordan_t& s)
{
	reset( s.nrows,s.ncolsmax);
}
gauss_jordan_t( gauss_jordan_t& s)
{
	reset( s.nrows,s.ncolsmax);
}

gauss_jordan_t& operator=(const gauss_jordan_t& s)
{
    
	return reset( s.nrows,s.ncolsmax);
}
gauss_jordan_t& operator=( gauss_jordan_t& s)
{

	return reset( s.nrows,s.ncolsmax);
}


gauss_jordan_t&  reset(long n,long nmax=-1)
	{
		nrows=n;
		ncolsmax=(nmax>0)?nmax:nrows;
		int nc=nrows+ncolsmax;
		buffer.resize(nrows*(nc));
		rowbuffer.resize(nrows);
		rowlbuf.resize(nrows);
		rowrbuf.resize(ncolsmax);
		ppl=&rowlbuf[0];
		ppr=&rowrbuf[0];
		arr2=&rowbuffer[0];
		float_type* p=&buffer[0];   
		for(long k=0;k<nrows;k++,p+=nc)   arr2[k]=p;
		return *this;
	}


	inline bool operator()(float_type** lhs, float_type** rhs, long ncolsrhs)
	{
		return make(lhs,rhs,ncolsrhs);
	}

	inline bool operator()(float_type** lhs, float_type** rhs)
	{
		return make(lhs,rhs,nrows);
	}

	inline bool operator()(float_type** lhs, float_type* rh)
	{
		return make(lhs,&rh,1);
	}

	inline bool operator()(float_type* lhs, float_type* rhs, long ncolsrhs=-1)
	{
            return  make_flat(lhs,rhs,ncolsrhs);
	}

	inline bool inverse(float_type** pps, float_type** ppd=0)
	{
		return  make(pps,(ppd)?ppd:pps,nrows,true);
	}
	inline bool inverse(float_type* pps, float_type* ppd=0)
	{
		return  make_flat(pps,(ppd)?ppd:pps,nrows,true);
	}



protected:



	inline static	void swaprows(float_type** arr, long row0, long row1) {
		float_type* temp;
		temp=arr[row0];
		arr[row0]=arr[row1];
		arr[row1]=temp;
	}

	inline int make_flat(float_type* lhs, float_type* rhs, long ncolsrhs,bool fback=false)
	{
		ncolsrhs=(ncolsrhs>0)?ncolsrhs:nrows;  
		for(int k=0;k<nrows;k++,lhs+=nrows) ppl[k]=lhs;
		for(int k=0;k<ncolsrhs;k++,rhs+=nrows) ppr[k]=rhs;

		return make(ppl,ppr,ncolsrhs,fback);
	}


	int  make(float_type** lhs, float_type** rhs, long ncolsrhs,bool fback=false) {


		for (long row=0; row<nrows; ++row) {
			for (long col=0; col<nrows; ++col) {
				arr2[row][col]=lhs[row][col];
			}
		if(fback)
		  	for (long col=nrows; col<nrows+ncolsrhs; ++col) {

				arr2[row][col]=(row+nrows-col)?0:1;
			}
		else for (long col=nrows; col<nrows+ncolsrhs; ++col) {
				arr2[row][col]=rhs[row][col-nrows];
			   }
		}

		//	perform forward elimination to get arr2 in row-echelon form
		for (long dindex=0; dindex<nrows; ++dindex) {
			//	run along diagonal, swapping rows to move zeros in working position 
			//	(along the diagonal) downwards
			if ( (dindex==(nrows-1)) && (arr2[dindex][dindex]==float_type(0))) {
				return 0; //  no solution
			} else if (arr2[dindex][dindex]== float_type(0)) {
				swaprows(arr2, dindex, dindex+1);
			}
			//	divide working row by value of working position to get a 1 on the
			//	diagonal
			if (arr2[dindex][dindex] == 0.0) {
				return 0;
			} else {
				float_type tempval=arr2[dindex][dindex];
				for (long col=0; col<nrows+ncolsrhs; ++col) {
					arr2[dindex][col]/=tempval;
				}
			}

			//	eliminate value below working position by subtracting a multiple of 
			//	the current row
			for (long row=dindex+1; row<nrows; ++row) {
				float_type wval=arr2[row][dindex];
				for (long col=0; col<nrows+ncolsrhs; ++col) {
					arr2[row][col]-=wval*arr2[dindex][col];
				}
			}
		}

		//	backward substitution steps
		for (long dindex=nrows-1; dindex>=0; --dindex) {
			//	eliminate value above working position by subtracting a multiple of 
			//	the current row
			for (long row=dindex-1; row>=0; --row) {
				float_type wval=arr2[row][dindex];
				for (long col=0; col<nrows+ncolsrhs; ++col) {
					arr2[row][col]-=wval*arr2[dindex][col];
				}
			}
		}

		//	assign result to replace rhs
		for (long row=0; row<nrows; ++row) {
			for (long col=0; col<ncolsrhs; ++col) {
				rhs[row][col]=arr2[row][col+nrows];
			}
		}


		return 1;
	};



	float_type** arr2,**ppl,**ppr;
	int nrows;
	int ncolsmax;
	std::vector<float_type> buffer;
	std::vector<float_type*> rowbuffer,rowlbuf,rowrbuf;


};

/*
  //test ...
  double a[3][3]={{2,-1,0},{-1,2,-1},{0,-1,1}};
  double b[3][3]={{1,0,0},{0,1,0},{0,0,1}};
  double ia[3][3];
  
  

  bool fok;
   fok=gauss_jordan_t<double>(3)((double*)a,(double*)b);	

   gauss_jordan_t<double> gj(3);

     fok=gj.inverse((double*)a,(double*)ia); // inverse to ia
	 fok=gj.inverse((double*)a); // inverse in place


*/

