#pragma once
#include <math.h>


namespace mid2Dmesh {

	typedef unsigned char byte_t;
	typedef unsigned short uint16_t;
	typedef unsigned int uint32_t;
	typedef long long int64_t;
	typedef unsigned long long uint64_t;


	template<class float_t>
	struct rect2D_t
	{
		float_t xb,xe,yb,ye; 
	};


	template<class F>
	struct matrix3x3_t
	{
		F v[3][3];    
	};

	typedef void (*to_matrix_proc_t)(double* v1,double* v2,double* v3,matrix3x3_t<double>& ma,matrix3x3_t<double>& mv,matrix3x3_t<double>& mc,byte_t fmask,double sc_a,double sc_c);



	template <int VERS=0>
	struct mesh_2D_matrix_procs_t
	{


		static void EM_2D_up_to_matrix(double* v1,double* v2,double* v3,matrix3x3_t<double>& ma,matrix3x3_t<double>& mc,matrix3x3_t<double>& mv,byte_t fmask,double sc_a=1,double sc_c=1)
		{
			const double cnd=double(7)/216,cdd=double(22)/216;

			/*
			double ax=v1[0],ay=v1[1];
			double bx=v2[0],by=v2[1];
			double cx=v3[0],cy=v3[1];
			*/

			double ax=v1[1],ay=v1[0];
			double bx=v2[1],by=v2[0];
			double cx=v3[1],cy=v3[0];


			ax*=ax;
			bx*=bx;
			cx*=cx;

			double dD=ax*by - bx*ay - ax*cy + cx*ay + bx*cy - cx*by;

			double adD=(dD>0)?dD:-dD;

			double dDin=double(1)/(3*adD);

			double Xa=ax,Ya=ay,Xb=bx,Yb=by,Xc=cx,Yc=cy;

			double opx=(ax+bx+cx)/3,opy=(ay+by+cy)/3;

			double apx=(bx+cx)/2,apy=(by+cy)/2;
			double bpx=(ax+cx)/2,bpy=(ay+cy)/2;
			double cpx=(ax+bx)/2,cpy=(ay+by)/2;

			double ahx=(opx+apx)/2,ahy=(opy+apy)/2;
			double bhx=(opx+bpx)/2,bhy=(opy+bpy)/2;
			double chx=(opx+apx)/2,chy=(opy+cpy)/2;

			double Yap=apy,Ybp=bpy,Ycp=apy;

			double Xah=ahx,Xbh=bhx,Xch=chx;

			double b11,b22,b33,b12,b13,b21,b23,b31,b32;

			double Xab=Xa-Xb,Yab=Ya-Yb;
			double Xbc=Xb-Xc,Ybc=Yb-Yc;

			double Xca=Xc-Xa,Yca=Yc-Ya;

			double Yaap=Ya-Yap,Ybbp=Yb-Ybp,Yccp=Yc-Ycp;



			b11=dD*2.5*Ybc + 4*(Xbh*Ybbp - Xch*Yccp)*Ybc + 1.5*Xbc*Xbc;
			b22=dD*2.5*Yca + 4*(Xch*Yccp - Xah*Yaap)*Yca + 1.5*Xca*Xca;
			b33=dD*2.5*Yab + 4*(Xah*Yaap - Xbh*Ybbp)*Yab + 1.5*Xab*Xab;
			b12=dD*(2*Ybbp - 5*Yccp)/3 + 4*(Xbh*Ybbp - Xch*Yccp)*Yca + 1.5*Xbc*Xca;
			b13=dD*(5*Ybbp - 2*Yccp)/3 + 4*(Xbh*Ybbp - Xch*Yccp)*Yab + 1.5*Xbc*Xab;
			b21=dD*(5*Yccp - 2*Yaap)/3 + 4*(Xch*Yccp - Xah*Yaap)*Ybc + 1.5*Xca*Xbc;
			b23=dD*(2*Yccp - 5*Yaap)/3 + 4*(Xch*Yccp - Xah*Yaap)*Yab + 1.5*Xca*Xab;
			b31=dD*(2*Yaap - 5*Ybbp)/3 + 4*(Xah*Yaap - Xbh*Ybbp)*Ybc + 1.5*Xab*Xbc;
			b32=dD*(5*Yaap - 2*Ybbp)/3 + 4*(Xah*Yaap - Xbh*Ybbp)*Yca + 1.5*Xab*Xca;


			b11*=dDin;
			b22*=dDin;
			b33*=dDin;
			b12*=dDin;
			b13*=dDin;
			b21*=dDin;
			b23*=dDin;
			b31*=dDin;
			b32*=dDin;










			double sc_detD=sc_c*adD;	



			mc.v[0][0]=mc.v[1][1]=mc.v[2][2]=cdd*sc_detD;

			{		
				double r=cnd*sc_detD;
				mc.v[0][1]=mc.v[0][2]=mc.v[1][2]=r;
				mc.v[1][0]=mc.v[2][0]=mc.v[2][1]=r;

			}



			ma.v[0][0]=b11;
			ma.v[1][1]=b22;
			ma.v[2][2]=b33;

			ma.v[0][1]=b12;
			ma.v[0][2]=b13;
			ma.v[1][2]=b23;

			ma.v[1][0]=b21;
			ma.v[2][0]=b31;
			ma.v[2][1]=b31;



			if(fmask)
			{
				for(int k=0;k<3;k++)
					if(fmask&(1<<k))
					{
						//
						/*
						ma.v[k][0]= ma.v[k][1]=ma.v[k][2]=0;
						mc.v[k][0]= mc.v[k][1]=mc.v[k][2]=0;
						mc.v[k][k]=1;
						//						*/

						///*

						ma.v[0][k]= ma.v[1][k]=ma.v[2][k]=0;
						mc.v[0][k]= mc.v[1][k]=mc.v[2][k]=0;
						mc.v[k][k]=1;

						//*/


					}
			}




		}



		static void EM_2D_down_to_matrix(double* v1,double* v2,double* v3,matrix3x3_t<double>& ma,matrix3x3_t<double>& mc,matrix3x3_t<double>& mv,byte_t fmask,double sc_a=1,double sc_c=1)
		{
			const double cnd=double(7)/216,cdd=double(22)/216;

			double x1=v1[0],y1=v1[1];
			double x2=v2[0],y2=v2[1];
			double x3=v3[0],y3=v3[1];

			double detD=x1*y2 - x2*y1 - x1*y3 + x3*y1 + x2*y3 - x3*y2;
			if(detD<0) detD=-detD;

			double sc_detD=sc_c*detD;

			double bdetD2=-sc_a/(2*detD);


			double xc=(x1+x2+x3)/3,yc=(y1+y2+y3)/3;
			if(xc<0) xc=-xc;

			//double br=double(1)/sqrt(xc*xc+yc*yc);
			double br=double(1)/xc;

			sc_detD*=br;
			bdetD2*=br;



			mc.v[0][0]=mc.v[1][1]=mc.v[2][2]=cdd*sc_detD;

			{		
				double r=cnd*sc_detD;
				mc.v[0][1]=mc.v[0][2]=mc.v[1][2]=r;
				mc.v[1][0]=mc.v[2][0]=mc.v[2][1]=r;

			}


			mv.v[0][0]=mv.v[1][1]=mv.v[2][2]=cdd*detD*br;

			{		
				double r=cnd*detD*br;
				mv.v[0][1]=mv.v[0][2]=mv.v[1][2]=r;
				mv.v[1][0]=mv.v[2][0]=mv.v[2][1]=r;

			}





			double x12=x1-x2,x23=x2-x3,x31=x3-x1;
			double y12=y1-y2,y23=y2-y3,y31=y3-y1;

			double b00=x23*x23+y23*y23,b11=x31*x31+y31*y31,b22=x12*x12+y12*y12; 
			double b01=x23*x31+y23*y31,b02=x23*x12+y23*y12,b12=x31*x12+y31*y12;

			b00*=bdetD2;
			b11*=bdetD2;
			b22*=bdetD2;

			b01*=bdetD2;
			b02*=bdetD2;
			b12*=bdetD2;

			ma.v[0][0]=b00;
			ma.v[1][1]=b11;
			ma.v[2][2]=b22;

			ma.v[0][1]=ma.v[1][0]=b01;
			ma.v[0][2]=ma.v[2][0]=b02;
			ma.v[1][2]=ma.v[2][1]=b12;	   


			if(fmask)
			{
				
				double r=cnd*detD*br;

				for(int k=0;k<3;k++)
					if(fmask&(1<<k))
					{
						//
						/*
						ma.v[k][0]= ma.v[k][1]=ma.v[k][2]=0;
						mc.v[k][0]= mc.v[k][1]=mc.v[k][2]=0;
						mc.v[k][k]=1;
						//*/

						///*

						ma.v[0][k]= ma.v[1][k]=ma.v[2][k]=0;
						mc.v[0][k]= mc.v[1][k]=mc.v[2][k]=0;
						mc.v[k][k]=1;
						//ma.v[k][k]=-1;

						//*/


					}
			}




		}

		static void EM_2D_down_to_matrix_137(double* v1,double* v2,double* v3,matrix3x3_t<double>& ma,matrix3x3_t<double>& mc,matrix3x3_t<double>& mv,byte_t fmask,double sc_a=1,double sc_c=1)
		{
			const double cnd=double(7)/216,cdd=double(22)/216;

			double x1=v1[0],y1=v1[1];
			double x2=v2[0],y2=v2[1];
			double x3=v3[0],y3=v3[1];

			double detD=x1*y2 - x2*y1 - x1*y3 + x3*y1 + x2*y3 - x3*y2;
			if(detD<0) detD=-detD;

			double sc_detD=sc_c*detD;

			double bdetD2=-sc_a/(2*detD);


			double xc=(x1+x2+x3)/3,yc=(y1+y2+y3)/3;
			if(xc<0) xc=-xc;

			//double br=double(1)/sqrt(xc*xc+yc*yc);
			double br=double(1)/xc;

			sc_detD*=br;
			bdetD2*=br;



			mc.v[0][0]=mc.v[1][1]=mc.v[2][2]=cdd*sc_detD;

			{		
				double r=cnd*sc_detD;
				mc.v[0][1]=mc.v[0][2]=mc.v[1][2]=r;
				mc.v[1][0]=mc.v[2][0]=mc.v[2][1]=r;

			}


			mv.v[0][0]=mv.v[1][1]=mv.v[2][2]=cdd*detD*br;

			{		
				double r=cnd*detD*br;
				mv.v[0][1]=mv.v[0][2]=mv.v[1][2]=r;
				mv.v[1][0]=mv.v[2][0]=mv.v[2][1]=r;

			}





			double x12=x1-x2,x23=x2-x3,x31=x3-x1;
			double y12=y1-y2,y23=y2-y3,y31=y3-y1;

			double b00=x23*x23+y23*y23,b11=x31*x31+y31*y31,b22=x12*x12+y12*y12; 
			double b01=x23*x31+y23*y31,b02=x23*x12+y23*y12,b12=x31*x12+y31*y12;

			b00*=bdetD2;
			b11*=bdetD2;
			b22*=bdetD2;

			b01*=bdetD2;
			b02*=bdetD2;
			b12*=bdetD2;

			ma.v[0][0]=b00;
			ma.v[1][1]=b11;
			ma.v[2][2]=b22;

			ma.v[0][1]=ma.v[1][0]=b01;
			ma.v[0][2]=ma.v[2][0]=b02;
			ma.v[1][2]=ma.v[2][1]=b12;	   


			if(fmask)
			{
				//double r=cnd*sc_detD;
				double r=cnd*detD*br;

				for(int k=0;k<3;k++)
					if(fmask&(1<<k))
					{
						
						//
						/*
						ma.v[k][0]= ma.v[k][1]=ma.v[k][2]=0;
						mc.v[k][0]= mc.v[k][1]=mc.v[k][2]=0;
						mc.v[k][k]=1;
					    //ma.v[k][k]=-1;

						//*/

						///*

						ma.v[0][k]= ma.v[1][k]=ma.v[2][k]=0;
						mc.v[0][k]= mc.v[1][k]=mc.v[2][k]=0;
						mc.v[k][k]=1;

						//*/


					}
			}




		}




		//typedef double[3][3]  double3x3_t;

		static void laplace_2D_to_matrix(double* v1,double* v2,double* v3,matrix3x3_t<double>& ma,matrix3x3_t<double>& mc,matrix3x3_t<double>& mv,byte_t fmask,double sc_a=1,double sc_c=1)
		{
			const double cnd=double(7)/216,cdd=double(22)/216;

			double x1=v1[0],y1=v1[1];
			double x2=v2[0],y2=v2[1];
			double x3=v3[0],y3=v3[1];

			double detD=x1*y2 - x2*y1 - x1*y3 + x3*y1 + x2*y3 - x3*y2;
			if(detD<0) detD=-detD;

			double sc_detD=sc_c*detD;

			double bdetD2=-sc_a/(2*detD);

			mc.v[0][0]=mc.v[1][1]=mc.v[2][2]=cdd*sc_detD;


			{		
				double r=cnd*sc_detD;
				mc.v[0][1]=mc.v[0][2]=mc.v[1][2]=r;
				mc.v[1][0]=mc.v[2][0]=mc.v[2][1]=r;

			}


			mv.v[0][0]=mv.v[1][1]=mv.v[2][2]=cdd*detD;


			{		
				double r=cnd*detD;
				mv.v[0][1]=mv.v[0][2]=mv.v[1][2]=r;
				mv.v[1][0]=mv.v[2][0]=mv.v[2][1]=r;

			}






			double x12=x1-x2,x23=x2-x3,x31=x3-x1;
			double y12=y1-y2,y23=y2-y3,y31=y3-y1;

			double b00=x23*x23+y23*y23,b11=x31*x31+y31*y31,b22=x12*x12+y12*y12; 
			double b01=x23*x31+y23*y31,b02=x23*x12+y23*y12,b12=x31*x12+y31*y12;

			b00*=bdetD2;
			b11*=bdetD2;
			b22*=bdetD2;

			b01*=bdetD2;
			b02*=bdetD2;
			b12*=bdetD2;

			ma.v[0][0]=b00;
			ma.v[1][1]=b11;
			ma.v[2][2]=b22;

			ma.v[0][1]=ma.v[1][0]=b01;
			ma.v[0][2]=ma.v[2][0]=b02;
			ma.v[1][2]=ma.v[2][1]=b12;	   


			if(fmask)
			{
				for(int k=0;k<3;k++)
					if(fmask&(1<<k))
					{
						//
						/*
						ma.v[k][0]= ma.v[k][1]=ma.v[k][2]=0;
						mc.v[k][0]= mc.v[k][1]=mc.v[k][2]=0;
						mc.v[k][k]=1;
						//*/

						///*

						ma.v[0][k]= ma.v[1][k]=ma.v[2][k]=0;
						mc.v[0][k]= mc.v[1][k]=mc.v[2][k]=0;
						mc.v[k][k]=1;

						//*/


					}
			}




		}

		static void laplace_2D_to_matrix0(double* v1,double* v2,double* v3,matrix3x3_t<double>& ma,matrix3x3_t<double>& mc,matrix3x3_t<double>& mv,byte_t fmask,double sc_a=1,double sc_c=1)
		{

			const double cnd=double(0)/216,cdd=double(22+7+7)/216;

			double x1=v1[0],y1=v1[1];
			double x2=v2[0],y2=v2[1];
			double x3=v3[0],y3=v3[1];

			double detD=x1*y2 - x2*y1 - x1*y3 + x3*y1 + x2*y3 - x3*y2;
			if(detD<0) detD=-detD;

			double sc_detD=sc_c*detD;

			double bdetD2=-sc_a/(2*detD);

			mc.v[0][0]=mc.v[1][1]=mc.v[2][2]=cdd*sc_detD;


			{		
				double r=cnd*sc_detD;
				mc.v[0][1]=mc.v[0][2]=mc.v[1][2]=r;
				mc.v[1][0]=mc.v[2][0]=mc.v[2][1]=r;

			}


			mv.v[0][0]=mv.v[1][1]=mv.v[2][2]=cdd*detD;


			{		
				double r=cnd*detD;
				mv.v[0][1]=mv.v[0][2]=mv.v[1][2]=r;
				mv.v[1][0]=mv.v[2][0]=mv.v[2][1]=r;

			}






			double x12=x1-x2,x23=x2-x3,x31=x3-x1;
			double y12=y1-y2,y23=y2-y3,y31=y3-y1;

			double b00=x23*x23+y23*y23,b11=x31*x31+y31*y31,b22=x12*x12+y12*y12; 
			double b01=x23*x31+y23*y31,b02=x23*x12+y23*y12,b12=x31*x12+y31*y12;

			b00*=bdetD2;
			b11*=bdetD2;
			b22*=bdetD2;

			b01*=bdetD2;
			b02*=bdetD2;
			b12*=bdetD2;

			ma.v[0][0]=b00;
			ma.v[1][1]=b11;
			ma.v[2][2]=b22;

			ma.v[0][1]=ma.v[1][0]=b01;
			ma.v[0][2]=ma.v[2][0]=b02;
			ma.v[1][2]=ma.v[2][1]=b12;	   


			if(fmask)
			{
				for(int k=0;k<3;k++)
					if(fmask&(1<<k))
					{
						//
						/*
						ma.v[k][0]= ma.v[k][1]=ma.v[k][2]=0;
						mc.v[k][0]= mc.v[k][1]=mc.v[k][2]=0;
						mc.v[k][k]=1;
						//*/

						///*

						ma.v[0][k]= ma.v[1][k]=ma.v[2][k]=0;
						mc.v[0][k]= mc.v[1][k]=mc.v[2][k]=0;
						mc.v[k][k]=1;

						//*/


					}
			}




		}



		static  to_matrix_proc_t get_matrix_proc(void* p)   
		{
			static to_matrix_proc_t mprocs[4]={&EM_2D_up_to_matrix,&EM_2D_down_to_matrix,&laplace_2D_to_matrix0,&laplace_2D_to_matrix};
			static to_matrix_proc_t m137=&EM_2D_down_to_matrix_137; 
			uint64_t ip=uint64_t(p);
			if(ip==137)
                  return m137;
			if(ip<4)
				return mprocs[ip];
			else return to_matrix_proc_t(p);		  
		}


	};

	template <int VERS=0>
	struct mesh_2D_t
	{

		inline static byte_t get_bc_mask(uint16_t *f,int64_t i1,int64_t i2,int64_t i3){
			if(f)
			{
				byte_t fbc=0;
				if(f[i1]) fbc|=1;
				if(f[i2]) fbc|=2;
				if(f[i3]) fbc|=4;
				return fbc;
			}
			else return 0;
		}

		static void __cdecl  laplace_2D_to_matrix_trimesh(to_matrix_proc_t matrix_proc,uint32_t index_base,uint32_t ntrs,uint32_t *ptrs,double *pa,double *pc,int64_t nvxs,double *pvxs,uint16_t * pfbc
			,matrix3x3_t<double>* pma,matrix3x3_t<double>* pmc,matrix3x3_t<double>* pmv						  )
		{
			if(pfbc) pfbc-=index_base; 
			pvxs-=2*index_base;

			for(uint32_t kt=0;kt<ntrs;kt++) 
			{
				uint32_t i1=ptrs[0];  
				uint32_t i2=ptrs[1];
				uint32_t i3=ptrs[2];

				byte_t fmask=get_bc_mask(pfbc,i1,i2,i3);

				double ra=pa[kt];
				double rc=pc[kt];

				matrix_proc(pvxs+2*i1,pvxs+2*i2,pvxs+2*i3,pma[kt],pmc[kt],pmv[kt],fmask,ra,rc);


				ptrs+=3;
			}

		}
	};

#define mul9(x) ((x<<3)+x)
#define mul2(x) (x<<1)
#define mul3(x) ((x<<1)+x)

	template<int _VERS=0>
	struct projective2D_t
	{
		inline static void transform(matrix3x3_t<double>& m,double* vx,double* vy)
		{
			double g;
			vy[0]=m.v[0][0]*vx[0]+m.v[0][1]*vx[1]+m.v[0][2];
			vy[1]=m.v[1][0]*vx[0]+m.v[1][1]*vx[1]+m.v[1][2];
			g=double(1)/(m.v[2][0]*vx[0]+m.v[2][1]*vx[1]+m.v[2][2]);
			vy[0]*=g;
			vy[1]*=g;
		}

		static void   vxs2D_prjtransform_in_index(uint32_t nvxs,uint32_t* pindex,matrix3x3_t<double>* pm,double* px,double* py)
		{
			//srand(GetTickCount());
			int ii=((GetTickCount()/1000))%7;
			for(uint32_t n=0;n<nvxs;n++)
			{
				uint32_t imm=pindex[n];
				double *vx=px+(n<<1);
				double *vy=py+(n<<1);
				matrix3x3_t<double> &m=pm[imm];
				if(ii&&(m.v[2][2]!=0))
					transform(m,vx,vy);
				else
				{
					vy[0]=vx[0];
					vy[1]=vx[1];
				}
			}
		}

	};



	template<class float_t=double>
	struct rect_utils_t
	{
		typedef bool (*tri_in_rect_proc_t)(uint32_t* tri,float_t* vts,rect2D_t<float_t>& r);
		typedef bool (*vxs_in_rect_proc_t)(float_t* vts,rect2D_t<float_t>& r);


		static inline bool _in_rect2D(float_t* v,rect2D_t<float_t>& r)
		{
			bool f=(r.xb<v[0])&&(v[0]<=r.xe)&&(r.yb<v[1])&&(v[1]<=r.ye);
			return f;
		}
		static bool in_rect2D(float_t* v,rect2D_t<float_t>& r)
		{
			return _in_rect2D(v,r);
		}


		static bool in_rect2D_closed(float_t* v,rect2D_t<float_t>& r)
		{
			bool f=(r.xb<=v[0])&&(v[0]<=r.xe)&&(r.yb<=v[1])&&(v[1]<=r.ye);
			return f;
		}


		static  bool in_rect2D_inner(float_t* v,rect2D_t<float_t>& r)
		{
			bool f=(r.xb<v[0])&&(v[0]<r.xe)&&(r.yb<v[1])&&(v[1]<r.ye);
			return f;
		}



		static bool tri_in_rect2D(uint32_t* tri,float_t* vts,rect2D_t<float_t>& r)
		{
			float_t* vx0=vts+2*tri[0];
			float_t* vx1=vts+2*tri[1];
			float_t* vx2=vts+2*tri[2];
			bool f;
			f=_in_rect2D(vx0,r);
			f=f&&_in_rect2D(vx1,r);
			f=f&&_in_rect2D(vx2,r);
			return f;
		}


		static bool tri_center_in_rect2D(uint32_t* tri,float_t* vts,rect2D_t<float_t>& r)
		{
			float_t* vx0=vts+2*tri[0];
			float_t* vx1=vts+2*tri[1];
			float_t* vx2=vts+2*tri[2];
			bool f;
			float_t vc[2]={(vx0[0]+vx1[0]+vx2[0])/3,(vx0[1]+vx1[1]+vx2[1])/3};
			f=_in_rect2D(vc,r);
			return f;
		}

		static void mesh2D_in_rect_index(uint32_t index_base,uint32_t fcenter,uint32_t ntrs,uint32_t* ptrs,uint32_t*pmasks
			,float_t* pvxs,uint32_t nrgn,uint32_t* prect_indx,rect2D_t<float_t>* prects){


				bool f;
				tri_in_rect_proc_t tri_in_rect_proc=fcenter? &tri_center_in_rect2D:tri_in_rect2D;


				pvxs-=2*index_base;


				for(uint32_t k=0;k<ntrs;k++){

					uint32_t& mask= pmasks[k]; 

					for(uint32_t n=0;n<nrgn;n++)
				 {
					 if(f=tri_in_rect_proc(ptrs,pvxs,prects[n]))
						 mask=prect_indx[n];  
				 }

					ptrs+=3;
			 }





		}

		static void vxs2D_in_rect_index(uint32_t fclosed,uint32_t nvxs,float_t* pvxs,uint32_t*pmasks
			,uint32_t nrgn,uint32_t* prect_indx,rect2D_t<float_t>* prects){


				bool f;
				const vxs_in_rect_proc_t vxs_in_rect_proc[3]={&in_rect2D,&in_rect2D_closed,&in_rect2D_inner};
				vxs_in_rect_proc_t in_rect=vxs_in_rect_proc[fclosed];	 

				for(uint32_t n=0;n<nrgn;n++){

					float_t *p=pvxs;
					uint32_t rect_indx=prect_indx[n];
					rect2D_t<float_t>& r=prects[n];

					for(uint32_t k=0;k<nvxs;k++,p+=2){
						if(f=in_rect(p,r))
							pmasks[k]=rect_indx; 						
					}
					

				}


				/*

				for(uint32_t k=0;k<nvxs;k++){

				uint32_t& mask= pmasks[k]; 

				for(uint32_t n=0;n<nrgn;n++)
				{
				if(f=in_rect(pvxs,prects[n]))
				mask=prect_indx[n];  
				}

				pvxs+=2;
			 }

			 */



		}




	};




}; //namespace trimesh_utils 



