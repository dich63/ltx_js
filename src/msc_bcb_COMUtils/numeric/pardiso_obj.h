#pragma once
#include "sparse_mkl.h"

namespace mkl_holder_utils {


	struct dummy_t
	{
		template<class T>
		inline T* operator &(){ 
			return (T*)c;
		}
		char c[32];
	};

template <class _field_type=MKL_Complex16>
struct pardiso_object_t
{
	typedef _field_type field_t;

     matrix_CRS_t<field_t>* mcsr;

	 pardiso_object_t(matrix_CRS_t<field_t>* _mcsr=0,i_t * _iparms=0){

		     memset(pt,0,sizeof(void*)*64);                 
              if(_mcsr) 
				  init(_mcsr,_iparms);
	 }


	~pardiso_object_t()
	 {
		 if((pt[0])&&(mcsr))
			   make_phase(-1);  
	 }


	 void clear()
	 {

		 if((pt[0])&&(mcsr))
		 {
			 make_phase(-1);  
			 memset(pt,0,sizeof(void*)*64);

		 }

	 }

	 void init(matrix_CRS_t<field_t>* _mcsr,i_t * _iparms=0)
	 {

         clear();  

		 mcsr=_mcsr;

		 memset(iparm,0,sizeof(i_t)*64);

		 if(mcsr) {
		 

		 if( _iparms) memcpy(iparm,_iparms,sizeof(i_t)*64);

		 
          
		  //memset(pt,0,sizeof(void*)*64);
		  mtype = 13;
		  nrhs=1;
		  maxfct=1;
		  mnum   = 1;         /* Which factorization to use. */
		  msglvl = 0;         /* Print statistical information  */
		  error  = 0; 

		 }
		  


	 }
     
	 i_t make_phase(i_t _phase,field_t* pb=0,field_t* px=0)
	 {
		 i_t err=0;
		 i_t idummy; 
		 field_t blank;

		 phase=_phase;
		 //cs.Start();
		 if(!pb) pb=&blank;
		 if(!px) px=&blank;
		 PARDISO (pt, &maxfct, &mnum, &mtype, &phase,&mcsr->n, mcsr->a, mcsr->ia, mcsr->ja,&idummy, &nrhs,
			 iparm, &msglvl,pb,px,&err);
		 //tlast=cs.Sec();
		 return err;
	 }

	 
	 void* pt[64];
	 i_t iparm[64];
	 i_t maxfct, mnum, phase, error, msglvl;
	 i_t mtype,nrhs;
};

void set_pardiso_iparms_def(i_t* iparm)
{
	memset(iparm,0,sizeof(i_t)*64);
	iparm[0] = 1; /* No solver default */
	iparm[1] = 2; /* Fill-in reordering from METIS */
	/* Numbers of processors, value of OMP_NUM_THREADS */
	iparm[2] = 1;
	iparm[3] = 0; /* No iterative-direct algorithm */
	iparm[4] = 0; /* No user fill-in reducing permutation */
	iparm[5] = 0; /* Write solution into x */
	iparm[6] = 0; /* Not in use */
	iparm[7] = 2; /* Max numbers of iterative refinement steps */
	iparm[8] = 0; /* Not in use */
	iparm[9] = 13; /* Perturb the pivot elements with 1E-13 */
	iparm[10] = 1; /* Use nonsymmetric permutation and scaling MPS */
	iparm[11] = 0; /* Not in use */
	iparm[12] = 1; /* Maximum weighted matching algorithm is switched-on (default for non-symmetric) */
	iparm[13] = 0; /* Output: Number of perturbed pivots */
	iparm[14] = 0; /* Not in use */
	iparm[15] = 0; /* Not in use */
	iparm[16] = 0; /* Not in use */
	iparm[17] = -1; /* Output: Number of nonzeros in the factor LU */
	iparm[18] = -1; /* Output: Mflops for LU factorization */
	iparm[19] = 0; /* Output: Numbers of CG Iterations */
};

void f_set_pardiso_iparms_def(i_t* i){
	
	memset(i,0,sizeof(i_t)*64);
	i_t* iparm=i-1;
	iparm[ 1] = 1;
	iparm[ 2] = 2;
	iparm[ 3] = 0;
	iparm[ 4] = 0;
	iparm[ 5] = 0;
	iparm[ 6] = 0;
	iparm[ 7] = 0;
	iparm[ 8] = 0;
	iparm[ 9] = 0;
	iparm[10] = 13;
	iparm[11] = 1;
	iparm[12] = 1;
	iparm[13] = 1;
	iparm[14] = 0;
	iparm[15] = 0;
	iparm[16] = 0;
	iparm[17] = 0;
	iparm[18] = -1;
	iparm[19] = 0;
	iparm[20] = 0;
	iparm[21] = 1;
	iparm[22] = 0;
	iparm[23] = 0;
	iparm[24] = 0;
	iparm[25] = 1;
	iparm[26] = 0;
	iparm[27] = 0;
	iparm[28] = 0;
	iparm[29] = 0;
	iparm[30] = 0;
	iparm[31] = 0;
	iparm[32] = 0;
	iparm[33] = 0;
	iparm[34] = 0;
	iparm[35] = 0;// one based
	//iparm[36] = 0 <===>	iparm[55] = 0;

	iparm[56] = 0;
	//iparm[57] = 0 <===>   iparm[59] = 0;
	iparm[60] = 0;
	//iparm[61] = 0;	iparm[62] = 0;
	iparm[63] = 0;
	iparm[64] = 0;

};

} //namespace mkl_holder_utils end