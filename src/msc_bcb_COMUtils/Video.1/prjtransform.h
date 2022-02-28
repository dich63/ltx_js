#pragma once

#include <complex>

namespace projective_transform {


/******************************************************************************/
/* Perform Gauss-Jordan elimination with row-pivoting to obtain the solution to 
 * the system of linear equations
 * A X = B
 * 
 * Arguments:
 * 		lhs		-	left-hand side of the equation, matrix A
 * 		rhs		-	right-hand side of the equation, matrix B
 * 		nrows	-	number of rows in the arrays lhs and rhs
 * 		ncolsrhs-	number of columns in the array rhs
 * 
 * The function uses Gauss-Jordan elimination with pivoting.  The solution X to 
 * the linear system winds up stored in the array rhs; create a copy to pass to
 * the function if you wish to retain the original RHS array.
 * 
 * Passing the identity matrix as the rhs argument results in the inverse of 
 * matrix A, if it exists.
 * 
 * No library or header dependencies, but requires the function swaprows, which 
 * is included here.
 */



//  swaprows - exchanges the contents of row0 and row1 in a 2d array
template <class float_type>
void swaprows(float_type** arr, long row0, long row1) {
    float_type* temp;
    temp=arr[row0];
    arr[row0]=arr[row1];
    arr[row1]=temp;
}

//	gjelim 
template <class float_type>
void gjelim(float_type** lhs, float_type** rhs, long nrows, long ncolsrhs=1) {

    //	augment lhs array with rhs array and store in arr2
    float_type** arr2=new float_type*[nrows];
    for (long row=0; row<nrows; ++row)
        arr2[row]=new float_type[nrows+ncolsrhs];

    for (long row=0; row<nrows; ++row) {
        for (long col=0; col<nrows; ++col) {
            arr2[row][col]=lhs[row][col];
        }
        for (long col=nrows; col<nrows+ncolsrhs; ++col) {
            arr2[row][col]=rhs[row][col-nrows];
        }
    }

    //	perform forward elimination to get arr2 in row-echelon form
    for (long dindex=0; dindex<nrows; ++dindex) {
        //	run along diagonal, swapping rows to move zeros in working position 
        //	(along the diagonal) downwards
        if ( (dindex==(nrows-1)) && (arr2[dindex][dindex]==0)) {
            return; //  no solution
        } else if (arr2[dindex][dindex]==0) {
            swaprows(arr2, dindex, dindex+1);
        }
        //	divide working row by value of working position to get a 1 on the
        //	diagonal
        if (arr2[dindex][dindex] == 0.0) {
            return;
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

    for (long row=0; row<nrows; ++row)
        delete[] arr2[row];
    delete[] arr2;
};


template <class float_type>
struct vector3D
{
	float_type r[3];
	inline   operator float_type*()
	{
		return r;
	};

};

template <class float_type>
struct projmatrix_base
{
	union
	{
		struct{ float_type a,b,e,c,d,f,k,l,m;};
		struct{ float_type rr[3*3]; };
	};
};


template <class float_type>
struct projmatrix:projmatrix_base<float_type>
{
	
inline projmatrix<float_type>& load(projmatrix_base<float_type>* ps)
{
  projmatrix_base<float_type>* pd=this;
  *pd=*ps;
  return *this;

}
projmatrix(projmatrix_base<float_type>& s)
{
    load(&s);
}

inline projmatrix<float_type>& operator=(projmatrix_base<float_type>& s)
{
  return load(&s);
};

inline float_type det()
{
	return a*(d*m-l*f)-c*(b*m-l*e)+k*(b*f-d*e);
}

projmatrix(bool fE=false)
{
   memset(rr,0,8*sizeof(float_type));
   if(fE)
   {
	   a=d=1;
   }
   m=1;
}

projmatrix back(bool fn=true)
{
	//float_type* ppa[8]={a_xi2[0],a_xi2[1],a_xi2[2],a_xi2[3],a_xi2[4],a_xi2[5],a_xi2[6],a_xi2[7]};
	//float_type* ppb[8]={b_xi2,b_xi2+1,b_xi2+2,b_xi2+3,b_xi2+4,b_xi2+5,b_xi2+6,b_xi2+7};
    projmatrix rb(true);
	float_type* ppa[3]={rr,rr+3,rr+3+3};
	float_type* ppb[3]={rb.rr,rb.rr+3,rb.rr+3+3};
	gjelim(ppa,ppb,3);
	if(fn&&(rb.m!=1))
	{
     rb.k/=rb.m;
	 rb.l/=rb.m;
     rb.m=1;
	}
	return rb;
}

inline   operator float_type*()
{
	return rr;
}
inline vector3D<float_type> transform(vector3D<float_type>& i)
{
   vector3D<float_type> o;
   o[0]=a*i[0]+b*i[1]+e*i[2];
   o[1]=c*i[0]+d*i[1]+f*i[2];
   o[2]=k*i[0]+l*i[1]+m*i[2];
   return o;
}

inline std::complex<float_type> transform(std::complex<float_type>& i)
{
      vector3D<float_type> t;
	  t[0]=i.real();
	  t[1]=i.imag();
	  t[2]=1;
	  t=transform(t);

      return std::complex<float_type>(t[0]/t[2],t[1]/t[2]);

}
template <typename T>
T  operator *(T& c)
{
  return transform(c);
}

};

template <class float_type>
projmatrix<float_type> make_proj_xi2(int nsize, std::complex<float_type> *i, std::complex<float_type> *o)
{
        projmatrix<float_type> prj;
		
        float_type a_xi2[8][8];
		float_type* b_xi2=prj.rr;
         memset(a_xi2[0],0,8*8*sizeof(float_type));
         //memset(b_xi2[0],0,8*sizeof(float_type));

		 for(int n=0;n<nsize;n++){
 
             float_type i0=i[n].real();
			 float_type i1=i[n].imag();
			 float_type o0=o[n].real();
			 float_type o1=o[n].imag();

			 float_type r0[8]={i0,i1,1,0,0,0,-o0*i0,-o0*i1};
			 float_type r1[8]={0,0,0,i0,i1,1,-o1*i0,-o1*i1};
               
			 
              for(int x=0;x<8;x++){

                   float_type t=o0*r0[x]+o1*r1[x];
			     b_xi2[x]+=t;	  
				 
                  for(int y=0;y<8;y++){

                    float_type t=r0[y]*r0[x]+r1[y]*r1[x];
                     a_xi2[y][x]+=t;

					}
		      }

		  }
     
//gjelim(float_type** lhs, float_type** rhs, long nrows, long ncolsrhs)	
		 float_type* ppa[8]={a_xi2[0],a_xi2[1],a_xi2[2],a_xi2[3],a_xi2[4],a_xi2[5],a_xi2[6],a_xi2[7]};
		 float_type* ppb[8]={b_xi2+0,b_xi2+1,b_xi2+2,b_xi2+3,b_xi2+4,b_xi2+5,b_xi2+6,b_xi2+7};
        gjelim(ppa,ppb,8);
		return prj;
}


};