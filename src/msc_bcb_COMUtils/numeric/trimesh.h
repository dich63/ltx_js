#pragma once
#include "ltxjs.h"
#include "sparse_ltx_utils.h"
#include <math.h>


namespace trimesh_utils {

	typedef ipc_utils::com_scriptor_t javascript_t;
	typedef ipc_utils::dispcaller_t<javascript_t::exception_t> caller_t;

	typedef javascript_t::value_t jsvalue_t;
	typedef i_mbv_buffer_ptr::int64_t int64_t;
	typedef ltx_helper::arguments_t<jsvalue_t>  arguments_t;
	typedef ltx_helper::mbv_map_reader_t<jsvalue_t>  mbvmap_reader_t;
	typedef ltx_helper::mbv_map_writer_t<jsvalue_t>  mbv_map_writer_t;
	typedef i_mm_region_ptr::int64_t int64_t;
	typedef unsigned char byte_t;



	struct locker_cache_t
	{
		std::list<i_mm_region_ptr::locker_t> lst;
		locker_cache_t& operator <<(i_mm_region_ptr* p)
		{
			lst.push_back(i_mm_region_ptr::locker_t());
			i_mm_region_ptr::locker_t& l=lst.back();
			l.attach(p);
			return *this;
		}

	};
	/*
	inline static HRESULT create_real_vector(int64_t n,pcallback_lib_arguments_t lib,i_mm_region_ptr** ppdata)
	{
	HRESULT hr;
	VARIANT v={VT_R8};
	v.dblVal=n;
	hr=lib->bind_object_args(L"ltx.bind:mm_buffer:length=#1; type=double",__uuidof(i_mm_region_ptr),(void**)ppdata,1,&v);
	return hr;
	}

	*/

	template<class F>
	struct matrix3x3_t
	{
		F v[3][3];    
	};


	inline HRESULT create_buffer(const wchar_t* typ,int64_t n,pcallback_lib_arguments_t lib,i_mm_region_ptr** ppdata)
	{
		HRESULT hr;
		ipc_utils::bstr_c_t<128> bs(typ);
		VARIANT vv[]={{VT_I8},{VT_BSTR}};
		vv[0].llVal=n;
		vv[1].bstrVal=bs;
		hr=lib->bind_object_args(L"ltx.bind:mm_buffer:length=#1; type=#2",__uuidof(i_mm_region_ptr),(void**)ppdata,2,vv);
		return hr;
	}

	inline HRESULT create_buffer(VARTYPE vt,int64_t n,pcallback_lib_arguments_t lib,i_mm_region_ptr** ppdata)
	{
		HRESULT hr;		
		VARIANT vv[]={{VT_I8},{VT_I4}};
		vv[0].llVal=n;
		vv[1].intVal=vt;
		hr=lib->bind_object_args(L"ltx.bind:mm_buffer:length=#1; type=#2",__uuidof(i_mm_region_ptr),(void**)ppdata,2,vv);
		return hr;
	}




	template <int VERS=0>
	struct mesh_2D_t
	{
		typedef void (*to_matrix_proc_t)(double* v1,double* v2,double* v3,matrix3x3_t<double>& ma,matrix3x3_t<double>& mv,matrix3x3_t<double>& mc,byte_t fmask,double sc_a,double sc_c);

		static void EM_2D_up_to_matrix(double* v1,double* v2,double* v3,matrix3x3_t<double>& ma,matrix3x3_t<double>& mc,matrix3x3_t<double>& mv,byte_t fmask,double sc_a=1,double sc_c=1)
		{
			const double cnd=double(7)/216,cdd=double(22)/216;

			double ax=v1[0],ay=v1[1];
			double bx=v2[0],by=v2[1];
			double cx=v3[0],cy=v3[1];

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
						//*/

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


		static void __cdecl  laplace_2D_to_matrix_trimesh
			( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t callback_context)
		{
			arguments_t arguments(callback_context);
			HRESULT &hr=arguments.hr;
			bool flag_bc=false, f_sc,f_sa;

			//ipc_utils::smart_ptr_t<i_mbv_container> mbvmap;
			//OLE_CHECK_VOID_hr(arguments[0].QueryInterface(mbvmap._address()));

			//VARIANT vm=arguments[0];

			mbvmap_reader_t map_in=VARIANT(arguments[0]);

			OLE_CHECK_VOID_hr(map_in);

			ipc_utils::smart_ptr_t<i_mm_region_ptr> r_triangles,r_vertexes,r_bc,r_sc_c,r_sc_a;

			mkl_holder_utils::type_buffer_t<UINT32> trs;
			mkl_holder_utils::type_buffer_t<double> vxs;
			mkl_holder_utils::type_buffer_t<bool> fbc;

			mkl_holder_utils::type_buffer_t<double> fc,fa;

			VARTYPE vt_trs,vt_vxs,vt_fbc;

			int64_t ntrs,nvxs,nbc=0,nsc=0,nsa=0,ntc,nta;
			void *ptrs=0,*pvxs=0,*pbc=0,*pscc=0,*psca=0;


			i_mm_region_ptr::locker_t lock1,lock2,lock3,lockbc,lock_sc_a,lock_sc_c,lock_vol;

			int index_base=map_in[ jsvalue_t(L"index_base")].def<int>(1);
			double rm=map_in[ jsvalue_t(L"meter_unit")].def<double>(1);

			  rm=double(1)/rm;

			double l_unit=rm,s_unit=rm*rm,v_unit=rm*s_unit;


			//map_in["index_base"].def<int>(1);




			OLE_CHECK_VOID_hr(map_in[ jsvalue_t(L"trs")].QueryInterface(r_triangles._address()));
			lock1.attach(r_triangles);
			OLE_CHECK_VOID_hr(r_triangles->GetElementInfo(&vt_trs,&ntrs));
			OLE_CHECK_VOID_hr(r_triangles->GetPtr(&ptrs));
			if(!trs.link_vt(vt_trs,ntrs,ptrs))
				OLE_CHECK_VOID_hr(E_INVALIDARG);





			OLE_CHECK_VOID_hr(map_in[jsvalue_t(L"vxs")].QueryInterface(r_vertexes._address()));
			lock2.attach(r_vertexes);
			OLE_CHECK_VOID_hr(r_vertexes->GetElementInfo(&vt_vxs,&nvxs));
			OLE_CHECK_VOID_hr(r_vertexes->GetPtr(&pvxs));
			if(!vxs.link_vt(vt_vxs,nvxs,pvxs))
				OLE_CHECK_VOID_hr(E_INVALIDARG);




			if((flag_bc=!map_in[jsvalue_t(L"fbc")].is_empty())){
				OLE_CHECK_VOID_hr(map_in[jsvalue_t(L"fbc")].QueryInterface(r_bc._address()));
				lock3.attach(r_bc);
				OLE_CHECK_VOID_hr(r_bc->GetElementInfo(&vt_fbc,&nbc));
				OLE_CHECK_VOID_hr(r_bc->GetPtr(&pbc));
				if(!fbc.link_vt(vt_fbc,nbc,pbc))
					OLE_CHECK_VOID_hr(E_INVALIDARG);
			}

			if((f_sc=!map_in[jsvalue_t(L"fc")].is_empty())){

				VARTYPE vt;
				void*p;
				
				OLE_CHECK_VOID_hr(map_in[jsvalue_t(L"fc")].QueryInterface(r_sc_c._address()));
				lock_sc_c.attach(r_sc_c);
				OLE_CHECK_VOID_hr(r_sc_c->GetElementInfo(&vt,&ntc));
				OLE_CHECK_VOID_hr(r_sc_c->GetPtr(&p));
				if(!fc.link_vt(vt,nsc,p))
					OLE_CHECK_VOID_hr(E_INVALIDARG);

			}

			if((f_sa=!map_in[jsvalue_t(L"fa")].is_empty())){

				VARTYPE vt;
				void*p;
				
				OLE_CHECK_VOID_hr(map_in[jsvalue_t(L"fa")].QueryInterface(r_sc_a._address()));
				lock_sc_a.attach(r_sc_a);
				OLE_CHECK_VOID_hr(r_sc_a->GetElementInfo(&vt,&nta));
				OLE_CHECK_VOID_hr(r_sc_a->GetPtr(&p));
				if(!fa.link_vt(vt,nsa,p))
					OLE_CHECK_VOID_hr(E_INVALIDARG);

			}



			int64_t nt=ntrs/3,nv=nvxs/2;

			matrix3x3_t<double> *pma,*pmc,*pmv;



			ipc_utils::smart_ptr_t<i_mm_region_ptr> mta,mtc;
			ipc_utils::smart_ptr_t<i_mm_region_ptr> mtbc,mtv;

			OLE_CHECK_VOID_hr(create_real_vector(9*nt,plib,mta._address()));
			OLE_CHECK_VOID_hr(create_real_vector(9*nt,plib,mtc._address()));
			OLE_CHECK_VOID_hr(create_real_vector(9*nt,plib,mtv._address()));
			i_mm_region_ptr::locker_t lock4(mta),lock5(mtc),lockv(mtv);

			OLE_CHECK_VOID_hr(mta->GetPtr((void**)&pma));
			OLE_CHECK_VOID_hr(mtc->GetPtr((void**)&pmc));
			OLE_CHECK_VOID_hr(mtv->GetPtr((void**)&pmv));

			UINT32* pt=trs;
			double* pvx=vxs-2*index_base;
			bool* f=fbc-index_base;
			byte_t* ftbc;

			double *pfc=fc,*pfa=fa,*pvol;

			to_matrix_proc_t mprocs[4]={&laplace_2D_to_matrix0,&laplace_2D_to_matrix,&EM_2D_down_to_matrix,&EM_2D_up_to_matrix};

			 int nscheme=map_in[jsvalue_t(L"scheme")].def<int>(0);


			 to_matrix_proc_t matrix_proc=mprocs[nscheme+1];

          /*

			 OLE_CHECK_VOID_hr(create_buffer(VT_R8,nt,plib,mvolumes._address()));
			 lock_vol.attach(mvolumes);
			 OLE_CHECK_VOID_hr(mvolumes->GetPtr((void**)&pvol));

			 */






			if(flag_bc)
			{			
				OLE_CHECK_VOID_hr(create_buffer(VT_UI1,nt,plib,mtbc._address()));
				lockbc.attach(mtbc);
				OLE_CHECK_VOID_hr(mtbc->GetPtr((void**)&ftbc));



				for(int64_t kt=0;kt<nt;kt++) 
				{
					UINT32 i1=pt[0];  
					UINT32 i2=pt[1];
					UINT32 i3=pt[2];

					byte_t fmask=(int(f[i1])<<0)|(int(f[i2])<<1)|(int(f[i3])<<2);

					double ra=(pfa)?s_unit*pfa[kt]:s_unit;
					double rc=(pfc)?v_unit*pfc[kt]:v_unit;

					matrix_proc(pvx+2*i1,pvx+2*i2,pvx+2*i3,pma[kt],pmc[kt],pmv[kt],fmask,ra,rc);

					ftbc[kt]=fmask;

					pt+=3;
				}

			}
			else{

				for(int64_t kt=0;kt<nt;kt++) 
				{
					UINT32 i1=pt[0];  
					UINT32 i2=pt[1];
					UINT32 i3=pt[2];			

					double ra=(pfa)?s_unit*pfa[kt]:s_unit;
					double rc=(pfc)?v_unit*pfc[kt]:v_unit;

					matrix_proc(pvx+2*i1,pvx+2*i2,pvx+2*i3,pma[kt],pmc[kt],pmv[kt],0,ra,rc);				

					pt+=3;
				}



			}





			mbv_map_writer_t map_out;
			OLE_CHECK_VOID_hr(map_out.set_value(jsvalue_t(L"scheme"),double(nscheme)));
            OLE_CHECK_VOID_hr(map_out.set_value(jsvalue_t(L"nt"),double(nt)));
			OLE_CHECK_VOID_hr(map_out.set_value(jsvalue_t(L"nd"),int(3)));
			OLE_CHECK_VOID_hr(map_out.set_value(jsvalue_t(L"nv"),double(nv)));
			OLE_CHECK_VOID_hr(map_out.set_value(jsvalue_t(L"at"),mta.p));
			OLE_CHECK_VOID_hr(map_out.set_value(jsvalue_t(L"ct"),mtc.p));
			OLE_CHECK_VOID_hr(map_out.set_value(jsvalue_t(L"vt"),mtv.p));
			OLE_CHECK_VOID_hr(map_out.set_value(jsvalue_t(L"tri"),(IUnknown*)r_triangles));
			
			
			if(flag_bc)
				OLE_CHECK_VOID_hr(map_out.set_value(jsvalue_t(L"fbct"),mtbc.p));





			OLE_CHECK_VOID_hr(map_out.detach(&result)); 



		}

/*
		static void __cdecl  scale_factors_to_matrix_trimesh
			( variant_t& result, pcallback_lib_arguments_t plib,pcallback_context_arguments_t callback_context)
		{
			arguments_t arguments(callback_context);
			HRESULT &hr=arguments.hr;
			bool flag_bc=false;

			//ipc_utils::smart_ptr_t<i_mbv_container> mbvmap;
			//OLE_CHECK_VOID_hr(arguments[0].QueryInterface(mbvmap._address()));

			//VARIANT vm=arguments[0];

			mbvmap_reader_t map_in=VARIANT(arguments[0]);

			OLE_CHECK_VOID_hr(map_in);
		}
*/



	};



	




}; //namespace trimesh_utils 




struct external_callbacks_t
{
	const char* name;
	void* proc;
	int attr;
};
#define  PROC_LIST_BEGIN(_external_callbacks) extern "C" external_callbacks_t _external_callbacks[]={
#define  PROC_ITEM(_proc) {#_proc,&_proc},
#define  PROC_ITEM_STRUCT(_struct,_proc) {#_proc,&_struct::_proc},
#define  PROC_LIST_END {0,0}};

PROC_LIST_BEGIN(mesh_utils)
PROC_ITEM_STRUCT(trimesh_utils::mesh_2D_t<>,laplace_2D_to_matrix_trimesh)
PROC_LIST_END