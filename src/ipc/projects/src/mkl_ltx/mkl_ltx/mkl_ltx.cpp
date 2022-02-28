// mkl_ltx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>



#include "stdafx.h"
#include "ltxjs.h"
#include "ipc_ports/ipc_utils.h"
#include "dispid_find.h"
#include <math.h>
#include "numeric/exp_pade_data.h"
#include "numeric/sparse_mkl.h"
#include "numeric/sparse_ltx_utils.h"
#include "numeric/pardiso_obj.h"
#include "numeric/lipa_real.h"
#include "numeric/trimesh.h"
//#include "numeric/lipa.h"


//#pragma comment(lib, "delayimp")

typedef ipc_utils::stopwatch_t stopwatch_t;
typedef ipc_utils::com_scriptor_t javascript_t;
typedef ipc_utils::dispcaller_t<javascript_t::exception_t> caller_t;

typedef javascript_t::value_t jsvalue_t;
typedef i_mbv_buffer_ptr::int64_t int64_t;
typedef ltx_helper::arguments_t<jsvalue_t>  arguments_t;

typedef ltx_helper::mbv_map_reader_t<jsvalue_t>  mbvmap_reader_t;
typedef ltx_helper::mbv_map_writer_t<jsvalue_t>  mbvmap_writer_t;



using namespace mkl_holder_utils;

struct dbgC_t
{
	MKL_Complex16 c[16];
};
typedef dbgC_t* pdbgC_t;
struct dbg_t
{
	double c[16];
};
typedef dbg_t* pdbg_t;


inline HRESULT create_real_vector(i_t n,pcallback_lib_arguments_t lib,i_mm_region_ptr** ppdata)
{
	HRESULT hr;
	VARIANT v={VT_R8};
	v.dblVal=n;
	hr=lib->bind_object_args(L"ltx.bind:mm_buffer:length=#1; type=double",__uuidof(i_mm_region_ptr),(void**)ppdata,1,&v);
	return hr;
}

inline HRESULT create_int32_vector(INT64 n,pcallback_lib_arguments_t lib,i_mm_region_ptr** ppdata)
{
	HRESULT hr;
	VARIANT v={VT_R8};
	v.dblVal=n;
	hr=lib->bind_object_args(L"ltx.bind:mm_buffer:length=#1; type=int32",__uuidof(i_mm_region_ptr),(void**)ppdata,1,&v);
	return hr;
}
inline HRESULT create_uint32_vector(INT64 n,pcallback_lib_arguments_t lib,i_mm_region_ptr** ppdata)
{
	HRESULT hr;
	VARIANT v={VT_R8};
	v.dblVal=n;
	hr=lib->bind_object_args(L"ltx.bind:mm_buffer:length=#1; type=uint32",__uuidof(i_mm_region_ptr),(void**)ppdata,1,&v);
	return hr;
}


inline HRESULT create_complex_vector(INT64 n,pcallback_lib_arguments_t lib,i_mm_region_ptr** ppdata)
{
	HRESULT hr;
	/*
	VARIANT v={VT_R8};
	v.dblVal=2*n;
	hr=lib->bind_object_args(L"ltx.bind:mm_buffer:length=#1; type=double",__uuidof(i_mm_region_ptr),(void**)ppdata,1,&v);
	*/
	hr=create_real_vector(2*n,lib,ppdata);
	return hr;
}


extern "C"  void   scalar_prod
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	ipc_utils::smart_ptr_t<i_mm_cache_buffer_ptr> vec1,vec2;
	int64_t len1,len2;
	OLE_CHECK_VOID_hr(arguments[0].QueryInterface(vec1._address()));
	OLE_CHECK_VOID_hr(arguments[1].QueryInterface(vec2._address()));
	VARTYPE vt1,vt2;

	OLE_CHECK_VOID_hr(vec1->GetElementInfo(&vt1,&len1));
	OLE_CHECK_VOID_hr(vec2->GetElementInfo(&vt2,&len2));
	bool f=(vt1==VT_R8)&&(vt2==VT_R8)&&(len1==len2);
	if(!f)
		OLE_CHECK_VOID_hr(E_INVALIDARG);

	double *pv1,*pv2,s=0;
	if(0)
	{


		for(int64_t i=0;i<len1;i++)
		{
			OLE_CHECK_VOID_hr(vec1->GetPtrOfIndex(i,(void**)&pv1));
			OLE_CHECK_VOID_hr(vec2->GetPtrOfIndex(i,(void**)&pv2));
			s+=(*pv1)*(*pv2);
		}
	}
	else 
	{

		const int64_t bufsize=0x10000,count = bufsize/sizeof(double);

		int lenb=len1*sizeof(double);
		int64_t n=lenb/bufsize,rem=lenb-n*bufsize,offs=0;

		for(int64_t i=0;i<n;i++)
		{
			i_mm_cache_buffer_ptr::ptr_t<double> ptr1(vec1,offs,bufsize);
			i_mm_cache_buffer_ptr::ptr_t<double> ptr2(vec2,offs,bufsize);
			OLE_CHECK_VOID_hr(ptr1);
			OLE_CHECK_VOID_hr(ptr2);
			pv1=ptr1;
			pv2=ptr2;
			for(int k=0;k<count;k++)
				s+=pv1[k]*pv2[k];		

			offs+=bufsize;	
		}
		if(rem)
		{
			i_mm_cache_buffer_ptr::ptr_t<double> ptr1(vec1,offs,rem);
			i_mm_cache_buffer_ptr::ptr_t<double> ptr2(vec2,offs,rem);
			OLE_CHECK_VOID_hr(ptr1);
			OLE_CHECK_VOID_hr(ptr2);
			pv1=ptr1;
			pv2=ptr2;

			for(int k=0;k<rem/sizeof(double);k++)
				s+=pv1[k]*pv2[k];		

		}


	}


	result=s; 

}

extern "C"  void   marshalQItest
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	ipc_utils::smart_ptr_t<i_mm_region_ptr> data;
	OLE_CHECK_VOID_hr(arguments[1].QueryInterface(data._address()));
	result.vt=VT_UNKNOWN;
	result.punkVal=data.detach();
}

template <class Value>
void load_data(i_t nnz,double* pc,Value* pv)
{
	for(i_t i=0;i<nnz;i++)
		pc[i]=pv[i];
}

template <class Value>
void load_index(i_t nnz,i_t* pi,Value* pv)
{
	for(i_t i=0;i<nnz;i++)
		pi[i]=pv[i];
}

template <class Value>
void load_real_vector(i_t n,MKL_Complex16* pc,Value* pv)
{
	if(pv)
		for(i_t i=0;i<n;i++)
		{
			pc[i].real=pv[i];
			pc[i].imag=0;
		}
}

template <class Value>
void load_imag_vector(i_t n,MKL_Complex16* pc,Value* pv)
{
	if(pv)
		for(i_t i=0;i<n;i++)
		{
			pc[i].real=0;
			pc[i].imag=pv[i];
		}
}


template <class Value>
void save_real_vector(i_t n,Value* pv,MKL_Complex16* pc)
{
	if(pv)
		for(i_t i=0;i<n;i++)
		{
			pv[i]=pc[i].real;		
		}
}

template <class Value>
void save_imag_vector(i_t n,Value* pv,MKL_Complex16* pc)
{
	if(pv)
		for(i_t i=0;i<n;i++)
		{
			pv[i]=pc[i].imag;		
		}
}


template <class Value>
void load_real_vector(i_t n,MKL_Complex16* pc,Value* pv,Value* pvi)
{
	if(pvi==0) load_real_vector(n,pc,pv);
	else if(pv==0) load_imag_vector(n,pc,pvi);
	else
		for(i_t i=0;i<n;i++)
		{
			pc[i].real=pv[i];
			pc[i].imag=pvi[i];
		}
}

template <class Value>
void save_real_vector(i_t n,Value* pv,Value* pvi,MKL_Complex16* pc)
{
	if(pvi==0) save_real_vector(n,pv,pc);
	else 
		if(pv==0) save_imag_vector(n,pvi,pc);
		else
			for(i_t i=0;i<n;i++)
			{
				pv[i]=pc[i].real;
				pvi[i]=pc[i].imag;
			}
}


static  void  matrix_CSR_create_callback 
( jsvalue_t& result, matrix_CRS_t<MKL_Complex16>& m,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr,hr0;	
	if(arguments.id()==0)
	{
		int n=m.n,nnz=m.nnz;
		if(arguments.length()==0)
		{
			s_parser_t<wchar_t> sp;

			result=(wchar_t*)sp.printf(L" CSR_matrix[%dx%d] nnz=%d  complex",n,n,nnz);
			hr=0;
			return;
		}
		else{

			int c=arguments[0].def<int>(0);
			if(c==0)
			{
				result=n;
				hr=0;
				return;
			}
			if(c==1)
			{
				result=nnz;
				hr=0;
				return;
			}
			if(c==2)
			{
				double dmx=0;
				MKL_Complex16* pa=m.a;
				for (i_t i=0;i<nnz;i++){
					double re=pa[i].real,im=pa[i].imag,d;
					d=re*re+im*im;
					if(dmx<d) dmx=d;
				}

				result=(dmx>0)?sqrt(dmx):double(0);
				hr=0;
				return;
			}

		}


	}
	hr=E_NOTIMPL;

}





extern "C"  void   matrix_CSR_create
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{

	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr,hr0;

	i_t err; 

	i_t n=arguments[0].def<int>(0);
	if(n<=0) 
		OLE_CHECK_VOID_hr(E_INVALIDARG);


	matrix_COO_t<double> mcoo(n);
	MKL_Complex16 lambda={1,0};
	MKL_Complex16* plambda=0;

	ipc_utils::smart_ptr_t<i_mm_region_ptr> data;

	hr0=arguments[1].QueryInterface(data._address());


	if(FAILED(hr0)&&(arguments.length()>1))
	{
		arguments[1].update(lambda.real); 
		arguments[2].update(lambda.imag);
		plambda=&lambda;
		mcoo.set_identity();
	}




	if(SUCCEEDED(hr0)) 
	{



		ipc_utils::smart_ptr_t<i_mm_region_ptr> row,col;

		//OLE_CHECK_VOID_hr(hr0);
		OLE_CHECK_VOID_hr(arguments[2].QueryInterface(row._address()));
		OLE_CHECK_VOID_hr(arguments[3].QueryInterface(col._address()));

		if((arguments.length()>4))
		{
			arguments[4].update(lambda.real); 
			arguments[5].update(lambda.imag);
			plambda=&lambda;

		}


		i_t nnz;


		VARTYPE vtd,vtr,vtc;

		int64_t c;
		OLE_CHECK_VOID_hr(data->GetElementInfo(&vtd,&c));

		nnz=c;
		if((vtd!=VT_R8)&&(vtd!=VT_R4)) 
			OLE_CHECK_VOID_hr(E_INVALIDARG);

		OLE_CHECK_VOID_hr(row->GetElementInfo(&vtr,&c));

		if((int(c)!=nnz)||((vtr!=VT_I4)&&(vtr!=VT_I2)&&(vtr!=VT_UI4)&&(vtr!=VT_UI2))) 
			OLE_CHECK_VOID_hr(E_INVALIDARG);

		OLE_CHECK_VOID_hr(col->GetElementInfo(&vtc,&c));

		if((int(c)!=nnz)||((vtc!=VT_I4)&&(vtc!=VT_I2)&&(vtc!=VT_UI4)&&(vtc!=VT_UI2))) 
			OLE_CHECK_VOID_hr(E_INVALIDARG);


		mcoo.nnz_rezise(nnz);

		{


			void *pdata=0,*prow=0,*pcol=0;
			i_mm_region_ptr::locker_t lock1(data),lock2(row),lock3(col);

			OLE_CHECK_VOID_hr(data->GetPtr(&pdata));
			OLE_CHECK_VOID_hr(row->GetPtr(&prow));
			OLE_CHECK_VOID_hr(col->GetPtr(&pcol));




			if(vtd==VT_R8)
				load_data(nnz,mcoo.a,(double*)pdata);
			else
				load_data(nnz,mcoo.a,(float*)pdata);


			if((vtr==VT_I2)||(vtr==VT_UI2))
				load_index(nnz,mcoo.rowind,(unsigned short*)prow);
			else 
				load_index(nnz,mcoo.rowind,(unsigned*)prow);

			if((vtc==VT_I2)||(vtc==VT_UI2))
				load_index(nnz,mcoo.colind,(unsigned short*)pcol);
			else 
				load_index(nnz,mcoo.colind,(unsigned*)pcol);

		}

	};

	ipc_utils::local_ptr_t<matrix_CRS_t<double> > mcrs;
	ipc_utils::local_ptr_t<matrix_CRS_t<MKL_Complex16> > mccrs;

	if(err=coo2crs(&mcoo,&mcrs.p))
		OLE_CHECK_VOID_hr(E_ACCESSDENIED);


	expand_to_complex(mcrs,&mccrs.p,plambda);

	if(SUCCEEDED(hr=ltx_helper::wrap_callback<FLAG_CBD_FTM>(&matrix_CSR_create_callback,mccrs,&matrix_CRS_t<MKL_Complex16>::onexit,&result)))
		mccrs.detach();

	//ipc_utils::local_ptr_t<mkl_holder_utils::matrix_CRS_t> pcrs=new mkl_holder_utils::matrix_CRS_t(n)   

}



extern "C"  void   matrix_COO_container_create
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{

	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr,hr0;

	i_t err; 



	ipc_utils::smart_ptr_t<i_context> hms;	
	matrix_CRS_t<MKL_Complex16>* msc;
	OLE_CHECK_VOID_hr(arguments[0].QueryInterface(hms._address()));			
	OLE_CHECK_VOID_hr(hms->GetContext((void**)&(msc=0)));


	bool fimag=arguments[1].def<int>(0);

	//matrix_CRS_t<double>* msr;
	ipc_utils::local_ptr_t< matrix_CRS_t<double> > msr;
	ipc_utils::local_ptr_t< matrix_COO_t<double> > mcoo;
	//inline i_t complex2real(matrix_CRS_base_t<MKL_Complex16>* pmc,matrix_CRS_t<double>** ppmr,bool fimag=false)
	OLE_CHECK_VOID_hr(HRESULT_FROM_WIN32(complex2real(msc,&msr.p,fimag)));
	OLE_CHECK_VOID_hr(HRESULT_FROM_WIN32(crs2coo(msr,&mcoo.p)));

	i_t n=mcoo->n;
	i_t nnz=mcoo->nnz;


	ipc_utils::smart_ptr_t<i_mbv_container> mbv_map;	

	ipc_utils::smart_ptr_t<i_mm_region_ptr> mval,mcol,mrow;


	OLE_CHECK_VOID_hr(lib->bind_object(L"ltx.bind:mbv_map",__uuidof(i_mbv_container),mbv_map._ppQI()));

	OLE_CHECK_VOID_hr(create_int32_vector(nnz,lib,mcol._address()));
	OLE_CHECK_VOID_hr(create_int32_vector(nnz,lib,mrow._address()));
	OLE_CHECK_VOID_hr(create_real_vector(nnz,lib,mval._address()));



	i_mm_region_ptr::ptr_t<i_t> colp(mcol,0,sizeof(int)*nnz);
	i_mm_region_ptr::ptr_t<i_t> rowp(mrow,0,sizeof(int)*nnz);
	i_mm_region_ptr::ptr_t<double> valp(mval,0,sizeof(double)*nnz);

	OLE_CHECK_VOID_hr(colp);
	OLE_CHECK_VOID_hr(rowp);
	OLE_CHECK_VOID_hr(valp);

	memcpy(colp,mcoo->colind,sizeof(int)*nnz);
	memcpy(rowp,mcoo->rowind,sizeof(int)*nnz);
	memcpy(valp,mcoo->a,sizeof(double)*nnz);


	VARIANT vi={VT_I4},vo={VT_UNKNOWN};

	OLE_CHECK_VOID_hr(mbv_map->SetItem(jsvalue_t(L"n"),jsvalue_t(n)));
	OLE_CHECK_VOID_hr(mbv_map->SetItem(jsvalue_t(L"nnz"),jsvalue_t(nnz)));
	OLE_CHECK_VOID_hr(mbv_map->SetItem(jsvalue_t(L"fmt"),jsvalue_t(L"COO")));
	OLE_CHECK_VOID_hr(mbv_map->SetItem(jsvalue_t(L"row"),jsvalue_t((IUnknown*)mrow)));
	OLE_CHECK_VOID_hr(mbv_map->SetItem(jsvalue_t(L"col"),jsvalue_t((IUnknown*)mcol)));
	OLE_CHECK_VOID_hr(mbv_map->SetItem(jsvalue_t(L"data"),jsvalue_t((IUnknown*)mval)));

	//mbv_map.detach()   	
	vo.punkVal=mbv_map.detach();
	result.Attach(vo);



}


extern "C"  void   matrix_DIA2CSR_create
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{

	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr,hr0;

	i_t err; 

	i_t n=arguments[0].def<int>(0);
	if(n<=0) 
		OLE_CHECK_VOID_hr(E_INVALIDARG);



	MKL_Complex16 lambda={1,0};
	MKL_Complex16* plambda=0;

	ipc_utils::smart_ptr_t<i_mm_region_ptr> data;
	ipc_utils::smart_ptr_t<i_mm_region_ptr> distance;

	OLE_CHECK_VOID_hr(arguments[1].QueryInterface(data._address()));
	OLE_CHECK_VOID_hr(arguments[2].QueryInterface(distance._address()));



	matrix_DIA_t<double> mdia(n);


	if((arguments.length()>3))
	{
		arguments[3].update(lambda.real); 
		arguments[4].update(lambda.imag);
		plambda=&lambda;

	}


	i_t nnz,ndiags;


	VARTYPE vtd,vti;

	int64_t c;
	OLE_CHECK_VOID_hr(data->GetElementInfo(&vtd,&c));

	nnz=c;
	if((vtd!=VT_R8)&&(vtd!=VT_R4)) 
		OLE_CHECK_VOID_hr(E_INVALIDARG);

	OLE_CHECK_VOID_hr(distance->GetElementInfo(&vti,&c));
	ndiags=c;
	c*=n;

	if((c!=nnz)&&((vti!=VT_I4)&&(vti!=VT_I2)&&(vti!=VT_UI4)&&(vti!=VT_UI2))) 
		OLE_CHECK_VOID_hr(E_INVALIDARG);




	mdia.nnz_rezise(nnz,ndiags);


	{


		void *pdata=0,*pdistance=0;
		i_mm_region_ptr::locker_t lock1(data),lock2(distance);

		OLE_CHECK_VOID_hr(data->GetPtr(&pdata));
		OLE_CHECK_VOID_hr(distance->GetPtr(&pdistance));




		if(vtd==VT_R8)
			load_data(nnz,mdia.a,(double*)pdata);
		else
			load_data(nnz,mdia.a,(float*)pdata);


		if((vti==VT_I2)||(vti==VT_UI2))
			load_index(ndiags,mdia.distance,(signed short*)pdistance);
		else 
			load_index(ndiags,mdia.distance,(int*)pdistance);			

	}



	ipc_utils::local_ptr_t<matrix_CRS_t<double> > mcrs;
	ipc_utils::local_ptr_t<matrix_CRS_t<MKL_Complex16> > mccrs;

	if(err=dia2crs(&mdia,&mcrs.p))
		OLE_CHECK_VOID_hr(E_ACCESSDENIED);


	expand_to_complex(mcrs,&mccrs.p,plambda);

	if(SUCCEEDED(hr=ltx_helper::wrap_callback<FLAG_CBD_FTM>(&matrix_CSR_create_callback,mccrs,&matrix_CRS_t<MKL_Complex16>::onexit,&result)))
		mccrs.detach();

	//ipc_utils::local_ptr_t<mkl_holder_utils::matrix_CRS_t> pcrs=new mkl_holder_utils::matrix_CRS_t(n)   

}


int set_pardiso_options(i_t*iparm, wchar_t* parg)
{
	wchar_t name[64];
	argv_zz<wchar_t> args(parg,0,L";");
	int c=args.argc;
	if(c>0) 
		for(int k=0;k<64;k++){
			swprintf(name,L"iparm[%d]",k+1);
			args[name].update(iparm[k]);
		}

		return args[L"sync"].def<int>(0);
};


struct pardiso_holder_t:ltx_helper::dispatch_by_name_wrapper_t< pardiso_holder_t,jsvalue_t>
{

	struct dummy_t
	{
		template<class T>
		inline T* operator &(){ 
			return (T*)c;
		}
		char c[32];
	};

	static wchar_t* get_errmsg(i_t err)
	{
		BEGIN_STRUCT_NAMES(pn)
			DISP_PAIR(input inconsistent,-1)
			DISP_PAIR(not enough memory,-2)
			DISP_PAIR(reordering problem,-3)
			DISP_PAIR(zero pivot. numerical factorization or iterative refinement problem,-4)
			DISP_PAIR(unclassified (internal) error,-5)
			DISP_PAIR(reordering failed (matrix types 11 and 13 only),-6)
			DISP_PAIR(diagonal matrix is singular,-7)
			DISP_PAIR(32-bit integer overflow problem,-8)
			DISP_PAIR(not enough memory for OOC,-9)
			DISP_PAIR(error opening OOC files,-10)
			DISP_PAIR(read/write error with OOC files,-11)
			DISP_PAIR((pardiso_64 only) pardiso_64 called from 32-bit library,-12)		
			END_STRUCT_NAMES	

			return (wchar_t*)find_name_def(pn,err,L"unknown error");
	}

	void set_options(wchar_t* parg)
	{
		/*
		wchar_t name[64];
		argv_zz<wchar_t> args(parg,0,L";");
		int c=args.argc;
		if(c<=0) return;
		for(int k=0;k<64;k++){
		swprintf(name,L"iparm[%d]",k+1);
		args[name].update(iparm[k]);
		}*/
		set_pardiso_options(iparm,parg);

	}

	pardiso_holder_t(arguments_t& arguments)
	{

		memset(pt,0,sizeof(pt));
		memset(iparm,0,sizeof(iparm));
		HRESULT &hr=herror;

		bstr_t opts=arguments[0].def<bstr_t>();


		OLE_CHECK_VOID_hr(arguments[1].QueryInterface(holder._address()));		
		OLE_CHECK_VOID_hr(holder->GetContext((void**)&(mcrs=0)));

		mtype = 13;
		nrhs=1;
		maxfct=1;
		mnum   = 1;         /* Which factorization to use. */
		msglvl = 1;         /* Print statistical information  */
		error  = 0;         /* Initialize error flag */

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


		//		memset(iparm,-1,sizeof(iparm));

		set_options(opts);


		n=mcrs->n;
		nnz=mcrs->nnz;

		ia=mcrs->ia;
		ja=mcrs->ja;
		a=mcrs->a;

		xbuf.resize(n);
		bbuf.resize(n);
		x=&xbuf[0];
		b=&bbuf[0];
		t11=-1;
		t22=-1;

		if(!(error=make_phase(11)))
		{
			t11=tlast;
			if(!(error=make_phase(22)))
				t22=tlast;
		}

	}
	~pardiso_holder_t()
	{
		make_phase(-1);
	}

	i_t make_phase(i_t _phase,MKL_Complex16* pb=0,MKL_Complex16* px=0)
	{
		i_t err=0;
		i_t idummy; 
		phase=_phase;
		cs.Start();
		if(!pb) pb=b;
		if(!px) px=x;
		PARDISO (pt, &maxfct, &mnum, &mtype, &phase,&n, a, ia, ja,&idummy, &nrhs,
			iparm, &msglvl,pb,px,&err);
		tlast=cs.Sec();
		return err;
	}

	HRESULT cstep(com_variant_t& result,arguments_t& arguments)
	{
		HRESULT hr;
		csc.Start();
		ipc_utils::smart_ptr_t<i_mm_region_ptr> bdata_c,xdata_c;
		OLE_CHECK_hr(arguments[0].QueryInterface(bdata_c._address()));
		VARIANT voutbuffer=arguments[1];
		OLE_CHECK_hr(xdata_c.reset(voutbuffer));			

		i_t err;
		int64_t n2=(n<<1),c;

		MKL_Complex16 *pb,*px;
		VARTYPE vt;

		OLE_CHECK_hr(bdata_c->GetElementInfo(&vt,&c));				
		if((vt!=VT_R8)&&(c!=n2)) 
			return hr=(E_INVALIDARG);
		OLE_CHECK_hr(xdata_c->GetElementInfo(&vt,&c));				
		if((vt!=VT_R8)&&(c!=n2)) 
			return hr=(E_INVALIDARG);




		i_mm_region_ptr::locker_t lockb(bdata_c),lockx(xdata_c);
		//Sleep(1000);

		OLE_CHECK_hr(bdata_c->GetPtr((void**)&pb));
		OLE_CHECK_hr(xdata_c->GetPtr((void**)&px));

		if(err=make_phase(33,pb,px))
			arguments.raise_error(get_errmsg(err),L"PARDISO");

		result=voutbuffer;

		tlastc=csc.Sec();

		return hr;

	}

	inline  void  on_get(DISPID id,const wchar_t* name,com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper){
		HRESULT &hr=arguments.hr=E_NOTIMPL;

		s_parser_t<wchar_t>::case_t<false> cs(name);

		if(cs(L"op",L"step"))
		{
			ipc_utils::smart_ptr_t<i_mm_region_ptr> bdata,xdata;
			ipc_utils::smart_ptr_t<i_mm_region_ptr> bdata_i,xdata_i;

			bool fskip=arguments[4].def<int>(0);



			OLE_CHECK_VOID_hr(arguments[0].QueryInterface(bdata._address()));

			VARIANT voutbuffer=arguments[1];
			OLE_CHECK_VOID_hr(xdata.reset(voutbuffer));			

			VARTYPE vt;
			int64_t c;
			int err;

			OLE_CHECK_VOID_hr(bdata->GetElementInfo(&vt,&c));				
			if((vt!=VT_R8)&&(c!=n)) 
				OLE_CHECK_VOID_hr(E_INVALIDARG);
			OLE_CHECK_VOID_hr(xdata->GetElementInfo(&vt,&c));				
			if((vt!=VT_R8)&&(c!=n)) 
				OLE_CHECK_VOID_hr(E_INVALIDARG);




			HRESULT hr0;
			if(SUCCEEDED(hr0=arguments[2].QueryInterface(bdata_i._address()))){
				OLE_CHECK_VOID_hr(bdata_i->GetElementInfo(&vt,&c));				
				if((vt!=VT_R8)&&(c!=n)) 
					OLE_CHECK_VOID_hr(E_INVALIDARG);

			}

			if(SUCCEEDED(hr0=arguments[3].QueryInterface(xdata_i._address()))){
				OLE_CHECK_VOID_hr(xdata_i->GetElementInfo(&vt,&c));				
				if((vt!=VT_R8)&&(c!=n)) 
					OLE_CHECK_VOID_hr(E_INVALIDARG);

			}










			double* pfr,*pfi;

			{


				i_mm_region_ptr::locker_t lock(bdata),lock_i(bdata_i);
				pfr=pfi=0;

				OLE_CHECK_VOID_hr(bdata->GetPtr((void**)&pfr));
				if(bdata_i)
					OLE_CHECK_VOID_hr(bdata_i->GetPtr((void**)&pfi));

				load_real_vector(n,b,pfr,pfi);

			}

			//arguments.raise_error(get_errmsg(-7),L"PARDISO");
			if(!fskip)
			{			
				if(err=make_phase(33))
					arguments.raise_error(get_errmsg(err),L"PARDISO");
			}

			{


				i_mm_region_ptr::locker_t lock(xdata),lock_i(xdata_i);

				pfr=pfi=0;

				OLE_CHECK_VOID_hr(xdata->GetPtr((void**)&pfr));
				if(xdata_i)
					OLE_CHECK_VOID_hr(xdata_i->GetPtr((void**)&pfi));

				save_real_vector(n,pfr,pfi,x);

			}

			result=voutbuffer;

			hr=S_OK;
			return;
		}

		if(cs(L"cop",L"cstep"))
		{

			hr=cstep(result,arguments);
			return ;

		}

		if(cs(4,L"last"))
		{

			result=tlast;
			hr=S_OK;
			return;
		}
		if(cs(4,L"clast"))
		{

			result=tlastc;
			hr=S_OK;
			return;
		}

		if(id==0)
		{
			s_parser_t<wchar_t> sp;
			//  GMKB()
			double peekmem=iparm[16-1]+iparm[17-1];
			if(peekmem<iparm[15-1]) peekmem=iparm[15-1];

			result=(wchar_t*)sp.printf(L"PARDISO [%d,%d] phase=%d (peek memory =%s) t11=%gsec t22=%gsec ",n,n,phase,GMKB(peekmem*1024),t11,t22);
			hr=S_OK;
			return;

		}




	};
	inline  void  on_put(DISPID id,const wchar_t* name,com_variant_t& value,arguments_t& arguments,i_marshal_helper_t* helper){
		HRESULT &hr=arguments.hr=E_NOTIMPL;

	};


	wchar_t* errmsg()
	{
		return get_errmsg(error);
	}





	ipc_utils::smart_ptr_t<i_context> holder;

	HRESULT herror;
	void* pt[64];
	i_t iparm[64];
	i_t maxfct, mnum, phase, error, msglvl;
	i_t mtype,nrhs;

	i_t n,nnz;
	i_t *ia,*ja;
	MKL_Complex16 *a,*b,*x;
	matrix_CRS_t<MKL_Complex16>* mcrs;

	//	dummy_t dummy;
	double t11,t22,tlast,tlastc;
	stopwatch_t cs,csc;
	std::vector<MKL_Complex16> xbuf,bbuf;
	s_parser_t<wchar_t> sp;



};




extern "C"  void   linsolver
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;


	ipc_utils::local_ptr_t<pardiso_holder_t > holder=new pardiso_holder_t(arguments);
	if(holder->error)
	{
		arguments.raise_error(holder->errmsg(),L"PARDISO");
	}

	hr=holder->wrap(&result,FLAG_CBD_FTM);
	holder.detach(hr);

}

extern "C"  void   dilation
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	ipc_utils::smart_ptr_t<i_context> hms;	
	matrix_CRS_t<MKL_Complex16>* ms;
	OLE_CHECK_VOID_hr(arguments[0].QueryInterface(hms._address()));		
	OLE_CHECK_VOID_hr(hms->GetContext((void**)&(ms=0)));
	double rv=arguments[1].def<double>(1);
	double iv=arguments[2].def<double>(0);

	i_t n=ms->n,nnz=ms->nnz;

	ipc_utils::local_ptr_t<matrix_CRS_t<MKL_Complex16> > md=new matrix_CRS_t<MKL_Complex16>(n,nnz);

	memcpy(md->ia,ms->ia,sizeof(i_t)*(n+1));
	memcpy(md->ja,ms->ja,sizeof(i_t)*(nnz));
	for(i_t i=0;i<nnz;i++)
	{
		MKL_Complex16& s=ms->a[i];
		MKL_Complex16& d=md->a[i];
		double sr=s.real,si=s.imag;
		d.real=rv*sr-iv*si;
		d.imag=iv*sr+rv*si;
	}


	if(SUCCEEDED(hr=ltx_helper::wrap_callback<FLAG_CBD_FTM>(&matrix_CSR_create_callback,md,&matrix_CRS_t<MKL_Complex16>::onexit,&result)))
		md.detach();

};

extern "C"  void   matrix_add
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	i_t err;
	ipc_utils::smart_ptr_t<i_context> hms1,hms2;	
	matrix_CRS_t<MKL_Complex16>* ms1,*ms2;
	OLE_CHECK_VOID_hr(arguments[0].QueryInterface(hms1._address()));		
	OLE_CHECK_VOID_hr(arguments[1].QueryInterface(hms2._address()));		
	OLE_CHECK_VOID_hr(hms1->GetContext((void**)&(ms1=0)));
	OLE_CHECK_VOID_hr(hms2->GetContext((void**)&(ms2=0)));

	i_t n=ms1->n;
	if(n!=ms2->n)
		OLE_CHECK_VOID_hr(E_INVALIDARG);

	double rv=arguments[2].def<double>(1);
	double iv=arguments[3].def<double>(0);

	MKL_Complex16 beta={rv,iv};


	ipc_utils::local_ptr_t<matrix_CRS_t<MKL_Complex16> > md;

	if(err=linopcrs(ms1,ms2,&md.p,&beta))
		OLE_CHECK_VOID_hr(E_INVALIDARG);



	if(SUCCEEDED(hr=ltx_helper::wrap_callback<FLAG_CBD_FTM>(&matrix_CSR_create_callback,md,&matrix_CRS_t<MKL_Complex16>::onexit,&result)))
		md.detach();

};




extern "C"  void   create_complex
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr,hr0,hr1;
	ipc_utils::smart_ptr_t<i_mm_region_ptr> rdata,idata,cdata;


	//OLE_CHECK_VOID_hr(arguments[0].QueryInterface(rdata._address()));
	bool fi=false,fr=false;

	int64_t cr=-1,ci=-1,c=-1,cc;
	VARTYPE vtr,vti,vtc;
	double cn;

	if((arguments.length()==1)&&((cn=arguments[0].def<double>(-1))>0))
	{
		OLE_CHECK_VOID_hr(create_complex_vector(cn,lib,cdata._address()));
		result=cdata.toVARIANT();
		return;
	}



	if(SUCCEEDED(hr0=arguments[0].QueryInterface(rdata._address())))
	{
		OLE_CHECK_VOID_hr(rdata->GetElementInfo(&vtr,&cr));				
		if((vtr!=VT_R8)) 
			OLE_CHECK_VOID_hr(E_INVALIDARG);
		fr=true;
		c=cr;
	}
	else hr1=hr0;




	if(SUCCEEDED(hr0=arguments[1].QueryInterface(idata._address())))
	{
		OLE_CHECK_VOID_hr(idata->GetElementInfo(&vti,&ci));				
		if((vti!=VT_R8)) 
			OLE_CHECK_VOID_hr(E_INVALIDARG);
		c=ci;
		fi=true;
	}	
	else hr1=hr0;

	if(c<0)
		OLE_CHECK_VOID_hr(hr1);
	if((fr&&fi)&&(cr!=ci)&&(vti!=vtr))
		OLE_CHECK_VOID_hr(E_INVALIDARG);

	if(SUCCEEDED(hr0=arguments[2].QueryInterface(cdata._address())))
	{
		OLE_CHECK_VOID_hr(cdata->GetElementInfo(&vtc,&cc));				
		if((cc!=2*c)||(vtc!=VT_R8)) 
			OLE_CHECK_VOID_hr(E_INVALIDARG);


	}
	else
		OLE_CHECK_VOID_hr(create_complex_vector(c,lib,cdata._address()));

	{

		i_mm_region_ptr::locker_t lock1(rdata),lock2(idata),lock3(cdata);

		double *pr=0,*pi=0;
		MKL_Complex16 *pc=0; 

		if(fr)
			OLE_CHECK_VOID_hr(rdata->GetPtr((void**)&pr));	
		if(fi)
			OLE_CHECK_VOID_hr(idata->GetPtr((void**)&pi));	

		OLE_CHECK_VOID_hr(cdata->GetPtr((void**)&pc));	


		load_real_vector(c,pc,pr,pi);
	}

	result=cdata.toVARIANT();

	//result=voutbuffer;	


};


extern "C"  void   complex_pr
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	ipc_utils::smart_ptr_t<i_mm_region_ptr> cdata,pdata;
	VARIANT voutbuffer;
	OLE_CHECK_VOID_hr(arguments[0].QueryInterface(cdata._address()));
	voutbuffer=arguments[1];
	OLE_CHECK_VOID_hr(pdata.reset(voutbuffer));
	VARTYPE vt,vt2;
	int64_t c,c2,offs;
	long el_size,el_size2;

	OLE_CHECK_VOID_hr(cdata->GetElementInfo(&vt2,&c2));
	OLE_CHECK_VOID_hr(pdata->GetElementInfo(&vt,&c,&el_size));

	if((vt!=VT_R8)&&(vt2!=VT_R8))
		OLE_CHECK_VOID_hr(E_INVALIDARG);
	bool fri=arguments[2].def<int>(0);

	i_mm_region_ptr::locker_t lock1(cdata),lock2(pdata);

	char *pc,*pp,*ppe;

	OLE_CHECK_VOID_hr(cdata->GetPtr((void**)&pc));	
	OLE_CHECK_VOID_hr(pdata->GetPtr((void**)&pp));	

	MKL_Complex16* pcc=(MKL_Complex16*)pc;

	if(c2==(2*c))
	{
		if(fri) save_imag_vector(c,(double*)pp,pcc);
		else  save_real_vector(c,(double*)pp,pcc);

	}
	else if(c2==c)
	{
		int64_t off=(fri)?1:0;

		double* d=(double*)pp;
		double* s=(double*)pc;
		s+=off;
		for(int64_t k=0;k<c;k+=2) d[k]=s[k];

	}
	else 
		OLE_CHECK_VOID_hr(E_INVALIDARG);


	result=voutbuffer;	




};

extern "C"  void   mv_mul
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	ipc_utils::smart_ptr_t<i_context> mc;
	ipc_utils::smart_ptr_t<i_mm_region_ptr> vx,vy;
	VARIANT voutbuffer;
	int ck=0;
	OLE_CHECK_VOID_hr(arguments[ck++].QueryInterface(mc._address()));

	OLE_CHECK_VOID_hr(arguments[ck++].QueryInterface(vx._address()));
	voutbuffer=arguments[ck++];
	OLE_CHECK_VOID_hr(vy.reset(voutbuffer));


	MKL_Complex16 alpha={1,0},beta={0,0};

	arguments[ck++].update(alpha.real);
	arguments[ck++].update(alpha.imag);
	arguments[ck++].update(beta.real);
	arguments[ck++].update(beta.imag);

	matrix_CRS_t<MKL_Complex16>* pmc=0;

	OLE_CHECK_VOID_hr(mc->GetContext((void**)&pmc));

	int64_t n2=2*pmc->n;

	VARTYPE vt;
	int64_t c;



	i_mm_region_ptr::locker_t lock1(vx),lock2(vy);

	MKL_Complex16 *px=0,*py=0;

	OLE_CHECK_VOID_hr(vx->GetElementInfo(&vt,&c));
	if( !((vt==VT_R8)&&(c==n2)))
		OLE_CHECK_VOID_hr(E_INVALIDARG);
	OLE_CHECK_VOID_hr(vy->GetElementInfo(&vt,&c));
	if( !((vt==VT_R8)&&(c==n2)))
		OLE_CHECK_VOID_hr(E_INVALIDARG);


	OLE_CHECK_VOID_hr(vx->GetPtr((void**)&px));
	OLE_CHECK_VOID_hr(vy->GetPtr((void**)&py));




	//matrix_vector_mul(matrix_CRS_t<MKL_Complex16>* pm,MKL_Complex16 *x,MKL_Complex16 *y,MKL_Complex16* alpha=0,MKL_Complex16* beta=0)
	matrix_vector_mul(pmc,px,py,&alpha,&beta);




	result=voutbuffer;
}

extern "C"  void   v_copy
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	ipc_utils::smart_ptr_t<i_mm_region_ptr> vx,vy;
	VARIANT voutbuffer;

	OLE_CHECK_VOID_hr(arguments[0].QueryInterface(vx._address()));

	voutbuffer=arguments[1];
	OLE_CHECK_VOID_hr(vy.reset(voutbuffer));


	VARTYPE vt1,vt2;
	int64_t cb,nb;
	i_mm_region_ptr::locker_t lock1(vy),lock2(vx);
	char *px=0,*py=0;
	OLE_CHECK_VOID_hr(vx->GetPtr((void**)&px,&cb));
	OLE_CHECK_VOID_hr(vy->GetPtr((void**)&py,&nb));

	if(cb!=nb)
		OLE_CHECK_VOID_hr(E_INVALIDARG);

	memcpy(py,px,cb);


	result=voutbuffer;
}


extern "C"  void   v_add_real
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	ipc_utils::smart_ptr_t<i_mm_region_ptr> vx,vy;
	VARIANT voutbuffer;
	int k=0;
	OLE_CHECK_VOID_hr(arguments[k++].QueryInterface(vx._address()));

	voutbuffer=arguments[k++];
	OLE_CHECK_VOID_hr(vy.reset(voutbuffer));




	double alpha=arguments[k++].def<double>(1);






	VARTYPE vt;
	int64_t c,n;



	i_mm_region_ptr::locker_t lock1(vx),lock2(vy);

	double *px=0,*py=0;

	OLE_CHECK_VOID_hr(vx->GetElementInfo(&vt,&n));
	if( !((vt==VT_R8)))
		OLE_CHECK_VOID_hr(E_INVALIDARG);
	OLE_CHECK_VOID_hr(vy->GetElementInfo(&vt,&c));
	if( !((vt==VT_R8)&&(c==n)))
		OLE_CHECK_VOID_hr(E_INVALIDARG);



	OLE_CHECK_VOID_hr(vx->GetPtr((void**)&px));
	OLE_CHECK_VOID_hr(vy->GetPtr((void**)&py));

	vector_add_real(n,px,py,alpha);

	result=voutbuffer;
}


extern "C"  void   v_add
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	ipc_utils::smart_ptr_t<i_mm_region_ptr> vx,vy;
	VARIANT voutbuffer;
	int k=0;
	OLE_CHECK_VOID_hr(arguments[k++].QueryInterface(vx._address()));

	voutbuffer=arguments[k++];
	OLE_CHECK_VOID_hr(vy.reset(voutbuffer));


	MKL_Complex16 alpha={1,0};

	arguments[k++].update(alpha.real);
	arguments[k++].update(alpha.imag);





	VARTYPE vt;
	int64_t c,n2;



	i_mm_region_ptr::locker_t lock1(vx),lock2(vy);

	MKL_Complex16 *px=0,*py=0;

	OLE_CHECK_VOID_hr(vx->GetElementInfo(&vt,&n2));
	if( !((vt==VT_R8)))
		OLE_CHECK_VOID_hr(E_INVALIDARG);
	OLE_CHECK_VOID_hr(vy->GetElementInfo(&vt,&c));
	if( !((vt==VT_R8)&&(c==n2)))
		OLE_CHECK_VOID_hr(E_INVALIDARG);



	OLE_CHECK_VOID_hr(vx->GetPtr((void**)&px));
	OLE_CHECK_VOID_hr(vy->GetPtr((void**)&py));

	vector_add(n2/2,px,py,&alpha);



	result=voutbuffer;
}

//

extern "C"  void   cmp_pattern
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	ipc_utils::smart_ptr_t<i_context> hms1,hms2;	
	matrix_CRS_t<MKL_Complex16>* ms1,*ms2;
	OLE_CHECK_VOID_hr(arguments[0].QueryInterface(hms1._address()));		
	OLE_CHECK_VOID_hr(arguments[1].QueryInterface(hms2._address()));		
	OLE_CHECK_VOID_hr(hms1->GetContext((void**)&(ms1=0)));
	OLE_CHECK_VOID_hr(hms2->GetContext((void**)&(ms2=0)));

	bool fcmp;
	i_t n=ms1->n,nnz=ms1->nnz;

	fcmp=(n==ms1->n)&&(nnz==ms1->nnz);           
	fcmp=fcmp&&(memcmp(ms1->ia,ms2->ia,(n+1)*sizeof(i_t))==0);
	fcmp=fcmp&&(memcmp(ms1->ja,ms2->ja,nnz*sizeof(i_t))==0);


	result=fcmp;

}

extern "C"  void   v_clear
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	ipc_utils::smart_ptr_t<i_mm_region_ptr> data;
	//OLE_CHECK_VOID_hr(arguments[0].QueryInterface(data._address()));
	VARIANT vout=arguments[0];
	OLE_CHECK_VOID_hr(data.reset(vout));
	i_mm_region_ptr::locker_t lock(data);

	char* p;
	int64_t len;	


	OLE_CHECK_VOID_hr(data->GetPtr((void**)&p,&len));
	if(arguments.length()==1){  
		memset(p,0,len);
	}
	else 
	{
		VARTYPE vt;
		int64_t c;
		OLE_CHECK_VOID_hr(data->GetElementInfo(&vt,&c));
		if(vt!=VT_R8)
			OLE_CHECK_VOID_hr(E_INVALIDARG);


		int offs=arguments[1].def<int>(0);
		int step=arguments[2].def<int>(1);
		double* pf=(double* )p;
		for(int i=offs;i<c;i+=step)
			pf[i]=0;


	}
	result=vout;

}

HRESULT attempt_attach_real_buffer(i_t n,VARIANT v,pcallback_lib_arguments_t lib,ipc_utils::smart_ptr_t<i_mm_region_ptr>& region)
{
	HRESULT hr;
	hr=region.reset(v);
	VARTYPE vt;
	i_mm_region_ptr::int64_t count;

	if(FAILED(hr))
	{
		hr=create_real_vector(n,lib,region.address());	
	}
	else
	{
		OLE_CHECK_hr(region->GetElementInfo(&vt,&count));   
		if((vt!=VT_R8)||(count!=n))
			hr=E_INVALIDARG;

	}    
	return hr;

}


struct lipa_real_holder_t:ltx_helper::dispatch_by_name_wrapper_t< lipa_real_holder_t,jsvalue_t>
{
	HRESULT hr;
	typedef lipa_solvers::lipa_real_t<> lipa_solver_t;
	lipa_solver_t *lipa;
	ipc_utils::smart_ptr_t<i_context> hold_md,hold_mc,hold_ml;

	lipa_real_holder_t(arguments_t& arguments):lipa(0)
	{
		HRESULT hr0;
		matrix_CRS_t<MKL_Complex16> *md,*mc,*ml=0;
		int ic=0;
		int np,mp;
		double dt;


		bstr_t opts=arguments[ic++].def<bstr_t>();


		dt=arguments[ic++].def<double>(1);
		np=arguments[ic++].def<int>(2);
		mp=arguments[ic++].def<int>(4);




		OLE_CHECK_VOID_hr(arguments[ic++].QueryInterface(hold_md._address()));
		OLE_CHECK_VOID_hr(hold_md->GetContext((void**)&(md=0)));

		OLE_CHECK_VOID_hr(arguments[ic++].QueryInterface(hold_mc._address()));
		OLE_CHECK_VOID_hr(hold_mc->GetContext((void**)&(mc=0)));	  

		if(SUCCEEDED(hr0=arguments[ic++].QueryInterface(hold_ml._address())))
			OLE_CHECK_VOID_hr(hold_ml->GetContext((void**)&ml));

		i_t iparms[64]={};
		int mode;

		mode=set_pardiso_options(iparms,opts);	  

		lipa= new lipa_solver_t(dt,np,mp,md,mc,ml);

		lipa->set_options(mode,iparms);

		//lipa->fsync=set_pardiso_options(lipa->iparms,opts);	  


	}
	~lipa_real_holder_t(){ delete lipa; }

	inline  void  on_get(DISPID id,const wchar_t* name,com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper)
	{
		HRESULT &hr=arguments.hr=E_NOTIMPL,hr0;


		s_parser_t<wchar_t>::case_t<false> cs(name);

		if(cs(L"op",L"step"))
		{
			if(arguments.argc>0)
			{
				int c=arguments[0].def<int>(1);
				while((c--)>0) 
					lipa->step();

			}
			else		lipa->step();
			hr=S_OK;

			/*
			ipc_utils::smart_ptr_t<i_mm_region_ptr> bdata,xdata,x1data;

			int ic=0;

			int count=arguments[ic++].def<int>(1);
			OLE_CHECK_VOID_hr(arguments[ic++].QueryInterface(bdata._address()));
			OLE_CHECK_VOID_hr(arguments[ic++].QueryInterface(xdata._address()));
			OLE_CHECK_VOID_hr(arguments[ic++].QueryInterface(x1data._address()));


			VARIANT voutbuffer=arguments[1];
			OLE_CHECK_VOID_hr(xdata.reset(voutbuffer));			
			*/
			return ;
		}

		if(cs(L"x"))
		{
			i_t n=lipa->nn;
			ipc_utils::smart_ptr_t<i_mm_region_ptr> region;
			OLE_CHECK_VOID_hr(attempt_attach_real_buffer(n,arguments[0],arguments.plib(),region));		

			double* pf;
			{			
				i_mm_region_ptr::locker_t lock(region);
				OLE_CHECK_VOID_hr(region->GetPtr((void**)&pf)); 
				lipa->get_real_f_f1(pf,0);
			}
			result=region.toVARIANT();


			//attempt_attach_real_buffer(i_t n,VARIANT& v,pcallback_lib_arguments_t lib,ipc_utils::smart_ptr_t<i_mm_region_ptr>& region)

			return ;
		}
		if(cs(L"x1"))
		{
			i_t n=lipa->nn;
			ipc_utils::smart_ptr_t<i_mm_region_ptr> region;
			OLE_CHECK_VOID_hr(attempt_attach_real_buffer(n,arguments[0],arguments.plib(),region));		

			double* pf1;
			{			
				i_mm_region_ptr::locker_t lock(region);
				OLE_CHECK_VOID_hr(region->GetPtr((void**)&pf1)); 
				lipa->get_real_f_f1(0,pf1);
			}
			result=region.toVARIANT();


			//attempt_attach_real_buffer(i_t n,VARIANT& v,pcallback_lib_arguments_t lib,ipc_utils::smart_ptr_t<i_mm_region_ptr>& region)

			return ;


		}
		if(cs(L"x2"))
		{
			i_t n=lipa->nn;
			ipc_utils::smart_ptr_t<i_mm_region_ptr> region;
			OLE_CHECK_VOID_hr(attempt_attach_real_buffer(n,arguments[0],arguments.plib(),region));		

			double* pf2;
			{			
				i_mm_region_ptr::locker_t lock(region);
				OLE_CHECK_VOID_hr(region->GetPtr((void**)&pf2)); 
				lipa->get_real_f2(pf2);
			}
			result=region.toVARIANT();


			//attempt_attach_real_buffer(i_t n,VARIANT& v,pcallback_lib_arguments_t lib,ipc_utils::smart_ptr_t<i_mm_region_ptr>& region)

			return ;


		}
		if(cs(L"init"))
		{
			lipa->init();
			hr=S_OK;

		}



	}

	inline  void  on_put(DISPID id,const wchar_t* name,com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper)
	{
		HRESULT &hr=arguments.hr=E_NOTIMPL,hr0;


		s_parser_t<wchar_t>::case_t<false> cs(name);

		i_mm_region_ptr::int64_t count;
		VARTYPE vt;
		double* pf;


		if(cs(L"x"))
		{
			i_t n=lipa->nn;			
			ipc_utils::smart_ptr_t<i_mm_region_ptr> region;
			//OLE_CHECK_VOID_hr(arguments[0].QueryInterface(region._address()));
			OLE_CHECK_VOID_hr(region.reset(result));
			OLE_CHECK_VOID_hr(region->GetElementInfo(&vt,&count));
			if((vt!=VT_R8)||(count!=n))
				OLE_CHECK_VOID_hr(E_INVALIDARG);

			OLE_CHECK_VOID_hr(region->GetPtr((void**)&pf));

			lipa->set_real_f(pf);

			return ;
		}

		if(cs(L"j"))
		{
			i_t n=lipa->nn;			
			ipc_utils::smart_ptr_t<i_mm_region_ptr> region;
			OLE_CHECK_VOID_hr(region.reset(result));
			OLE_CHECK_VOID_hr(region->GetElementInfo(&vt,&count));
			if((vt!=VT_R8)||(count!=n))
				OLE_CHECK_VOID_hr(E_INVALIDARG);
			OLE_CHECK_VOID_hr(region->GetPtr((void**)&pf));

			lipa->set_real_j(pf);			

			return ;
		}



	}

};

extern "C"  void   lipa_real_solver
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;

	ipc_utils::local_ptr_t<lipa_real_holder_t > holder=new lipa_real_holder_t(arguments);

	OLE_CHECK_VOID_hr(holder->hr);

	hr=holder->wrap(&result,FLAG_CBD_FTM);
	holder.detach(hr);

}
extern "C"  void   triangulation_to_coo_matrix_dupl
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	//typedef ltx_helper::mbv_map_reader_t<jsvalue_t>  mbvmap_reader_t;
	//typedef ltx_helper::mbv_map_writer_t<jsvalue_t>  mbv_map_writer_t;
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	mbvmap_reader_t mr((VARIANT)arguments[0]);
	OLE_CHECK_VOID_hr(mr);
	mbvmap_writer_t mw(mr.mbv);
	OLE_CHECK_VOID_hr(mw);

	ipc_utils::smart_ptr_t<i_mm_region_ptr> r_trs,r_col,r_row;

	if(mr.is(L"tri"))
	{
		OLE_CHECK_VOID_hr(mr[L"tri"].QueryInterface(r_trs._address())); 
	}
	else 
	{
	    OLE_CHECK_VOID_hr(mr[L"trs"].QueryInterface(r_trs._address())); 
	}
	      


	int nd=mr[L"nd"].def<int>(3),ndnd=nd*nd;
	 
	int64_t count;
	long es;
	VARTYPE vt; 
	OLE_CHECK_VOID_hr(r_trs->GetElementInfo(&vt,&count,&es));
	i_mm_region_ptr::ptr_t<UINT32> p_trs(r_trs,-1);
	OLE_CHECK_VOID_hr(p_trs);
	
	mkl_holder_utils::type_buffer_t<UINT32>  trs;
	OLE_CHECK_VOID_hr_cond(trs.link_vt(vt,count,(UINT32*)p_trs),E_INVALIDARG);
	int64_t ctri=count/nd;
	OLE_CHECK_VOID_hr(create_uint32_vector(ndnd*ctri,lib,r_col._address()));
	OLE_CHECK_VOID_hr(create_uint32_vector(ndnd*ctri,lib,r_row._address()));

	i_mm_region_ptr::ptr_t<UINT32> p_col(r_col,-1),p_row(r_row,-1);

	UINT32 *pt=trs,*pc=p_col,*pr=p_row;
	for(int64_t n=0;n<ctri;n++)
	{       
		for(int i=0;i<nd;i++)
		{
            UINT32 c=pt[i];
			for(int j=0;j<nd;j++)
			{
				pc[j]=c;
				pr[j]=pt[j];
			}
			pc+=nd;
			pr+=nd;
		}
		pt+=nd;
	}

	OLE_CHECK_VOID_hr(mw.set_value(L"col",(IUnknown*)r_col));
	OLE_CHECK_VOID_hr(mw.set_value(L"row",(IUnknown*)r_row));
   
	hr=mw.detach(&result);

}

extern "C"  void   mesh_data_to_matrix
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	VARIANT v=arguments[0];
	HRESULT hr0;

	int ic=0;
	int ntr;

	int	nd,nd2;
	int	nvx;
	int nc=1;
	int step_e=1,step_g=1;
	int64_t ct,c;


	bstr_t opts=arguments[ic++].def<bstr_t>();
	//
	wchar_t* popts=opts;
	v_buf<wchar_t> optsbuf;
	optsbuf.cat(popts,popts+opts.length());

	argv_zz<wchar_t> args(optsbuf.get());

	bool ftr=args[L"tr"].def<int>(0);


	nvx=arguments[ic++].def<double>(0);
	nd=arguments[ic++].def<int>(0);
	nd2=nd*nd;




	ipc_utils::smart_ptr_t<i_mm_region_ptr> r_tri,r_m;
	OLE_CHECK_VOID_hr(arguments[ic++].QueryInterface(r_tri._address()));
	OLE_CHECK_VOID_hr(arguments[ic++].QueryInterface(r_m._address()));



	VARTYPE vt;





	OLE_CHECK_VOID_hr(r_tri->GetElementInfo(&vt,&ct));
	if(!((vt==VT_I4)||(vt==VT_UI4)))
		OLE_CHECK_VOID_hr(E_INVALIDARG);

	ntr=ct/nd;

	OLE_CHECK_VOID_hr(r_m->GetElementInfo(&vt,&c));
	if( !((vt==VT_R8)&&(c==nd*nc*ct)))
		OLE_CHECK_VOID_hr(E_INVALIDARG);


	i_t * tri_indexes;
	double* pmatrix;



	ipc_utils::local_ptr_t<matrix_CRS_t<MKL_Complex16> > md;


	i_mm_region_ptr::locker_t lock1(r_tri),lock2(r_m);
	OLE_CHECK_VOID_hr(r_tri->GetPtr((void**)&tri_indexes));
	OLE_CHECK_VOID_hr(r_m->GetPtr((void**)&pmatrix));

	matrix_RM_t<double> mrm(nvx);

	mrm.add_elements(ntr,nd,tri_indexes,pmatrix); 
	/*
	i_t * indexes=tri_indexes;
	double* pm=pmatrix;
	for(i_t n=0;n<ntr;n++)
	{
	//i_t * indexes=tri_indexes+n*nd;
	//double* pm=pmatrix+n*nd2;
	mrm.add_element(nd,indexes,pm); 
	indexes+=nd;
	pm+=nd2;
	}
	*/
	ipc_utils::local_ptr_t<matrix_CRS_t<double> > mdr;

	mrm.convert_to_CRS(&mdr.p,ftr);

	expand_to_complex(mdr,&md.p);	


	hr=ltx_helper::wrap_callback<FLAG_CBD_FTM>(&matrix_CSR_create_callback,md,&matrix_CRS_t<MKL_Complex16>::onexit,&result);
	md.detach(hr);



	//ipc_utils::local_ptr_t<lipa_real_holder_t > holder=new lipa_real_holder_t(arguments);


}

extern "C"  void   set_key_value
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{ 
	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;
	int k=0;
	ltx_read_buffer_t<INT32> key_vector(arguments[k++]);
	OLE_CHECK_VOID_hr(key_vector);
	ltx_read_buffer_t<double> values(arguments[k++]);
	OLE_CHECK_VOID_hr(values);

	ipc_utils::smart_ptr_t<i_mm_region_ptr> value_vector;

	int64_t count=key_vector.count;

	OLE_CHECK_VOID_hr(create_real_vector(count,lib,value_vector._address()));


	i_mm_region_ptr::ptr_t<double> vv(value_vector,-1);

	OLE_CHECK_VOID_hr(vv);

	INT32 *pk=key_vector;
	double *pvk=values;
	double *pvv=vv;

	for(int64_t i=0;i<count;i++)
	{
		pvv[i]=pvk[pk[i]];
	}



	result=value_vector.toVARIANT();

}

extern "C"  void   mesh2D_in_rect_index
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{

	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;

	int k=0;
	ltx_read_buffer_t<UINT32> tri(arguments[k++]);
	OLE_CHECK_VOID_hr(tri);
	ltx_read_buffer_t<double> vtx(arguments[k++]);
	OLE_CHECK_VOID_hr(vtx);


	//double xb,xe,yb,ye;
	rect2D_t<double> rect2D;

	rect2D.xb=arguments[k++].def<double>(0);
	rect2D.xe=arguments[k++].def<double>(0);
	rect2D.yb=arguments[k++].def<double>(0);
	rect2D.ye=arguments[k++].def<double>(0);

	//rect2D_t<double>& rect2D={xb,xe,yb,ye};
	UINT32 nregion =arguments[k++].def<double>(0);

	int fcenter=arguments[k++].def<int>(1);

	int64_t ntri=tri.count/3;

	ipc_utils::smart_ptr_t<i_mm_region_ptr> mask_region;

	//OLE_CHECK_VOID_hr(mask_region.reset(VARIANT(arguments[k++])));

	OLE_CHECK_VOID_hr(arguments[k++].QueryInterface(mask_region._address()));



	i_mm_region_ptr::ptr_t<INT32> mask(mask_region,-1);
	OLE_CHECK_VOID_hr(mask);
	if( mask.ByteLength!=ntri*sizeof(INT32))
		OLE_CHECK_VOID_hr(DISP_E_BADINDEX);






	double *pvts=((double *)vtx)-2;
	UINT32* ptri=tri;
	INT32 *pmask=mask;
	bool f;

	if(fcenter)
		for(int64_t n=0;n<ntri;n++)	{
			f=tri_center_in_rect2D(ptri,pvts,rect2D);
			if(f) 
				pmask[n]=nregion; 
			ptri+=3;
		}
	else
		for(int64_t n=0;n<ntri;n++)	{
			f=tri_in_rect2D(ptri,pvts,rect2D);
			if(f) 
				pmask[n]=nregion; 
			ptri+=3;
		}


		result=(IUnknown*)mask_region;    




};


extern "C"  void   vxs2D_in_rect_index
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{

	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;

	int k=0;

	ltx_read_buffer_t<double> vtx(arguments[k++]);
	OLE_CHECK_VOID_hr(vtx);


	//double xb,xe,yb,ye;
	rect2D_t<double> rect2D;

	rect2D.xb=arguments[k++].def<double>(0);
	rect2D.xe=arguments[k++].def<double>(0);
	rect2D.yb=arguments[k++].def<double>(0);
	rect2D.ye=arguments[k++].def<double>(0);

	//rect2D_t<double>& rect2D={xb,xe,yb,ye};
	UINT32 nregion =arguments[k++].def<double>(0);

	int fcenter=arguments[k++].def<int>(1);

	int64_t nvxs=vtx.count/2;

	ipc_utils::smart_ptr_t<i_mm_region_ptr> mask_region;

	//OLE_CHECK_VOID_hr(mask_region.reset(VARIANT(arguments[k++])));

	OLE_CHECK_VOID_hr(arguments[k++].QueryInterface(mask_region._address()));



	i_mm_region_ptr::ptr_t<INT32> mask(mask_region,-1);
	OLE_CHECK_VOID_hr(mask);
	if( mask.ByteLength!=nvxs*sizeof(INT32))
		OLE_CHECK_VOID_hr(DISP_E_BADINDEX);






	double *pvts=(double *)vtx;

	INT32 *pmask=mask;
	bool f;


	for(int64_t n=0;n<nvxs;n++)	{
		f=in_rect2D_closed(pvts,rect2D);
		if(f) 
			pmask[n]=nregion; 
		pvts+=2;
	}


	result=(IUnknown*)mask_region;    




};

#define mul9(x) ((x<<3)+x)
#define mul2(x) (x<<1)

extern "C"  void   vxs2D_prjtransform_in_rect_index
( jsvalue_t& result, pcallback_lib_arguments_t lib,pcallback_context_arguments_t callback_context)
{

	arguments_t arguments(callback_context);
	HRESULT &hr=arguments.hr;

	int k=0;

	ltx_read_buffer_t<double> mtx(arguments[k++]);
	OLE_CHECK_VOID_hr(mtx);


	ltx_read_buffer_t<double> vtx(arguments[k++]);
	OLE_CHECK_VOID_hr(vtx);
	ltx_read_buffer_t<INT32> mask(arguments[k++],1);

	int fdelta=arguments[k++].def<int>(0);

	//OLE_CHECK_VOID_hr(mask);
	//if(mtx.count!=9*mask.count) 
	//	OLE_CHECK_VOID_hr(DISP_E_BADINDEX);

	int64_t nv=vtx.count/2;

	if(nv!=mask.count) 
		OLE_CHECK_VOID_hr(DISP_E_BADINDEX);



	ipc_utils::smart_ptr_t<i_mm_region_ptr> vout;
	OLE_CHECK_VOID_hr(create_real_vector(vtx.count,lib,vout._address()));

	i_mm_region_ptr::ptr_t<double> vty(vout,-1);


	double * pvx=vtx;
	double * pvy=vty;
	double * pmm=mtx;
	INT32 * pmask=mask;

	if(fdelta)
		for(int64_t n=0;n<nv;n++)
		{
			INT32 imm=pmask[n];
			double * mx= pmm+mul9(imm);


			double * vx= pvx+mul2(n);
			double * vy= pvy+mul2(n);
			if(mx[8]!=0)
			{
				proj2D_transform(mx,vx,vy);
				vy[0]-=vx[0];
				vy[1]-=vx[1];
			}
			


		}
	else 
		for(int64_t n=0;n<nv;n++)
		{
			INT32 imm=pmask[n];
			double * mx= pmm+mul9(imm);


			double * vx= pvx+mul2(n);
			double * vy= pvy+mul2(n);
			if(mx[8]!=0)
				proj2D_transform(mx,vx,vy);
			else {
				vy[0]=vx[0];
				vy[1]=vx[1];
			}


		}


		result=(IUnknown*)vout;    




};