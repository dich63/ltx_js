#pragma once
#include "numeric/pardiso_obj.h"
#include "numeric/exp_pade_data.h"
#include "ipc_ports/thread_pool_job.h"

namespace mkl_holder_utils {	
	namespace lipa_solvers {


		template <class FieldType=MKL_Complex16,class RealFieldType=double>
		struct lipa_real_t	{

			typedef FieldType field_t;
			typedef RealFieldType real_t;
			typedef matrix_CRS_t<field_t> matrix_t;
			typedef matrix_t* pmatrix_t;

			struct job_t
		 {
			 pmatrix_t ma,mc,ml;
			 int n;
			 int nn;
			 lipa_real_t* owner;
			 field_t pt,rt,prt,ptb,prt2;
			 pardiso_object_t<field_t> pardiso;
			 v_buf<field_t> x_buf,bz_buf;
			 v_buf<field_t> x1_buf,bz1_buf;
			 field_t *x,*bz;
			 field_t *x1,*bz1;
			 i_t err;



			 job_t(lipa_real_t* _owner,int nnode):owner(_owner),ma(0),mc(0),ml(0),n(nnode),err(0){

				 pt=owner->p_poles_dt[n];
				 rt=owner->p_res_dt[n];			  
				 prt=owner->p_poles_res_dt[n];
				 prt2=owner->p_poles_res_2_dt[n];
				 field_t e={1,0};
				 ptb=div_cc(e,pt);

			 };


			 ~job_t(){

				 matrix_t::onexit(ma);
				 //pmatrix_t::onexit(mc);

			 };


			 i_t create_A()
			 {

				 pmatrix_t md=owner->md,mc=owner->mc,ml=owner->ml;

				 mc=owner->mc;
				 ml=owner->ml;

				 nn=md->n;

				 field_t z,z2;
				 z.real=-pt.real;
				 z.imag=-pt.imag;

				 x=x_buf.resize(mc->n).get();
				 bz=bz_buf.resize(mc->n).get();


				 //i_t linopcrs(matrix_CRS_t<MKL_Complex16>* pma,matrix_CRS_t<MKL_Complex16>* pmb,matrix_CRS_t<MKL_Complex16>** ppmc,MKL_Complex16* beta=0)
				 err=linopcrs(md,mc,&ma,&z);
				 if((err==0)&&ml)
				 {
					 x1=x1_buf.resize(mc->n).get();
					 bz1=bz1_buf.resize(mc->n).get();

					 z2=mul_cc(z,z);
					 pmatrix_t t=ma;
					 ma=0;					  
					 err=linopcrs(t,ml,&ma,&z);
					 matrix_t::onexit(t);

				 }


				 return err;

			 }

			 i_t init(){



				 err= create_A();

				 if(!err)
				 {
					 pardiso.init(ma,owner->iparms);
					 err=pardiso.make_phase(11);
					 if(!err)
						 err=pardiso.make_phase(22);


				 }

				 return err;

			 }

			 i_t step()
			 {
				 if(!err){

					 memcpy(bz,owner->b,sizeof(field_t)*nn);
					 vector_add(nn,owner->j,bz,&ptb);
					 err=pardiso.make_phase(33,bz,x);
				 }
				 return err;
			 }

			 i_t step_complete()
			 {
				 if(!err){
					 
					 //vector_add(i_t n,MKL_Complex16 *x,MKL_Complex16 *y,MKL_Complex16* alpha=0)
					 //vector_add(ma->n,x,owner->x,&rt);
					 //vector_add(ma->n,x,owner->x1,&prt);
					 double r_r=rt.real,r_i=rt.imag;
					 double pr_r=-prt.real,pr_i=-prt.imag;
					 double pr2_r=-prt2.real,pr2_i=-prt2.imag;

					 for(int i=0;i<nn;i++)
					 {
						 field_t r=x[i];
						 field_t& rx=owner->x[i];
						 field_t& rx1=owner->x1[i];
						 field_t& rx2=owner->x2[i];
						 double f=r_r*r.real-r_i*r.imag;
						 rx.real+=f;
						 rx1.real+=pr_r*r.real-pr_i*r.imag;
						 rx2.real+=pr2_r*r.real-pr2_i*r.imag;
					 }

				 }
				 return err;
			 }






			};






			lipa_real_t(double _dt,int n,int m,pmatrix_t _md,pmatrix_t _mc,pmatrix_t _ml=0)
				:dt(_dt),np(n),mp(m),md(_md),mc(_mc),ml(_ml),fsync(0)
		 {

			 exp_pade_helper_t eph;
			 OLE_CHECK_VOID_hr(HRESULT_FROM_WIN32(eph.hr));

			 nn=md->n;

			 p_poles_dt=poles_buf.resize(mp).get();
			 p_res_dt=res_buf.resize(mp).get();
			 p_poles_res_2_dt=poles_res_2_buf.resize(mp).get();
			 p_poles_res_dt=poles_res_buf.resize(mp).get();


			 delta_pade=mp-np;

			 nPade=eph.poles_res_half(np,mp,p_poles_dt,p_res_dt);
			 if(nPade<0) {hr=E_INVALIDARG;return;}

//
			 /*// fuck
			 p_res_dt[0].real=-2.256958744418149e0;
			 p_res_dt[0].imag= 1.110883163787594e1;

			 p_res_dt[1].real= 2.256958744418162e0;
			 p_res_dt[1].imag=-3.963308700050185e1;
//*/




			 b=b_buf.resize(md->n).get();
			 x=x_buf.resize(md->n).get();
			 x1=x1_buf.resize(md->n).get();
			 x2=x2_buf.resize(md->n).get();
			 j=j_buf.resize(md->n).get();



			 for(int k=0;k<nPade;k++){

				 p_poles_dt[k].real/=dt;
				 p_poles_dt[k].imag/=dt;
				 p_res_dt[k].real/=dt;
				 p_res_dt[k].imag/=dt;
				 p_poles_res_dt[k]=mul_cc(p_poles_dt[k],p_res_dt[k]);
				 p_poles_res_2_dt[k]=mul_cc(p_poles_dt[k],p_poles_res_dt[k]);
				 jobs[k]=new job_t(this,k);

			 }

			 set_pardiso_iparms_def(iparms);
			
				 //thread_pool_job.reset(nPade);


		 }

		int	set_options(int mode,i_t * iprms=0)
		{
			if(iprms) 
				 memcpy(iparms,iprms,sizeof(i_t)*64);

			if((fsync=mode)!=1)
				thread_pool_job.reset(nPade);

			return nPade;
		}


			~lipa_real_t(){


				for(int k=0;k<nPade;k++)
					delete jobs[k];


		 }


			static  int init_job_proc(void* p,int n)
			{

				return ((lipa_real_t*)p)->jobs[n]->init(); 

			}

			static  int step_job_proc(void* p,int n)
			{

				return ((lipa_real_t*)p)->jobs[n]->step(); 
			}


			static  int step_job_complete_proc(void* p,int n)
			{			


				return ((lipa_real_t*)p)->jobs[n]->step_complete();
			}



			i_t first_error()
			{
				i_t err;
				for(int k=0;k<nPade;k++)
				{
					if(err=jobs[k]->err)
						break;
				}
				return err;
				
			}



            i_t init()
			{ 				
				if(fsync==1)
				{
					for(int i=0;i<nPade;i++)
						init_job_proc(this,i);


				}
				else   thread_pool_job.start_jobs(&init_job_proc,this);
				i_t err=first_error();
				return err;

			}

			void get_real_f2(real_t* f2)
			{
				if(f2)
					for (int i=0;i<nn;i++)
							f2[i]=x2[i].real;

			}


			void get_real_f_f1(real_t* f,real_t* f1)
			{	
				
				

				if(f&&f1)
				{

				
				for (int i=0;i<nn;i++)
				{
					f[i]=x[i].real;
					f1[i]=x1[i].real;
				}
				}
				else 
				{
					
					if(f==0)
					{
						for (int i=0;i<nn;i++)
							f1[i]=x1[i].real;
					}
					else {
						for (int i=0;i<nn;i++)
							f[i]=x[i].real;
					}


				}
				
			}


			void set_real_f(real_t* f)
			{
				if(f==0)
				{
					memset(b,0,sizeof(field_t)*nn);
				}
				else 
				for (int i=0;i<nn;i++)
				{
					x[i].real=f[i];
					x[i].imag=0;					
				}
			}


			void set_real_j(real_t* f)
			{
				
				if(f==0)
				{
					memset(j,0,sizeof(field_t)*nn);
				}
				else 
					for (int i=0;i<nn;i++)
					{
						j[i].real=f[i];
						j[i].imag=0;					
					}
			}


			inline field_t * zero_field(field_t *f)
			{
              memset(f,0,sizeof(field_t)*nn);
              return f;
			}


			i_t step()
			{
				i_t err=0;
				if(!err)
				{				
				 //(delta_pade<2) ||
					 
				//memset(b,0,sizeof(field_t)*nn);
				 zero_field(b);
				//matrix_vector_mul(matrix_CRS_t<MKL_Complex16>* pm,MKL_Complex16 *x,MKL_Complex16 *y,MKL_Complex16* alpha=0,MKL_Complex16* beta=0)
				 matrix_vector_mul(mc,x,b);
				 
				 //if(ml)					 matrix_vector_mul(ml,x,b);


				 if(delta_pade!=0) 
					      zero_field(x);

				  zero_field(x1);
				  zero_field(x2);

				 if(fsync==1)
				 {
					 for(int i=0;i<nPade;i++)
					 {
						 step_job_proc(this,i);
						 step_job_complete_proc(this,i);

					 }


				 }
				 else if(fsync==2)
				 {
					 thread_pool_job.start_jobs(&step_job_proc,this);
					 for(int i=0;i<nPade;i++)
					 {					
						 step_job_complete_proc(this,i);
					 }
				 }
				 else
					 thread_pool_job.start_jobs(&step_job_proc,this,&step_job_complete_proc);
     				 err=first_error();
				}
				return err;

			}






			HRESULT hr;
			pmatrix_t mc,md,ml;
			int np,mp,nPade,nn;
			int delta_pade;
			double dt;
			i_t iparms[64];
			int fsync;

			job_t* jobs[64];


			v_buf<field_t> b_buf,x1_buf,x2_buf,x_buf,j_buf;
			field_t *b,*x,*x1,*x2,*j;



			MKL_Complex16 *p_poles_dt,*p_res_dt;
			MKL_Complex16 *p_poles_res_dt,*p_poles_res_2_dt;
			v_buf<MKL_Complex16> poles_buf,res_buf,poles_res_buf,poles_res_2_buf;
			thread_pool_job_t<> thread_pool_job;



		};

	};// lipa_solver
};// mkl_holder_utils