#pragma once

#include "sparse_mkl.h"
#include <map>
#include <algorithm>
namespace mkl_holder_utils {


    
	template <class N,bool FCOPY=false>
	struct  type_buffer_t
	{
		typedef std::map<const char*,void*> ptrmap_t;


		type_buffer_t():count(0),p(0),ph(0){};

		~type_buffer_t(){		          
			::free(ph);
		}


		template<class O>
		N* link(size_t _count,O* po,bool fcopy=FCOPY){

			count=_count;
			if((!fcopy)&&(typeid(O).name()==typeid(N).name()))
			{ p=(N*)po;
			  if(ph)
			  {
				  free(ph);
				  ph=0;
			  }
			}
			else
			{
				p=ph=(N*)::realloc(ph,(count)*sizeof(N));
				for(size_t k=0;k<count;k++)
					p[k]=N(po[k]);	   		   		   

			}

			return p;		

		};

		N* link_vt(VARTYPE vt,size_t _count,void* po,bool fcopy=FCOPY){

			switch (vt)
			{
			case VT_R8:
				return link( _count,(double*)po,fcopy);
				break;
			case VT_R4:
				return link( _count,(float*)po,fcopy);
				break;
			case VT_I1:
				return link( _count,(signed char*)po,fcopy);
				break;
			case VT_UI1:
				return link( _count,(unsigned char*)po,fcopy);
				break;
			case VT_I2:
				return link( _count,(signed short*)po,fcopy);
				break;
			case VT_UI2:
				return link( _count,(unsigned short*)po,fcopy);
				break;
			case VT_BOOL:
				return link( _count,(VARIANT_BOOL*)po,fcopy);
				break;
			case VT_I4:
			    return link( _count,(signed int*)po,fcopy);
				break;
			case VT_UI4:
				return link( _count,(unsigned int*)po,fcopy);
				break;
			case VT_I8:
				return link( _count,(LONGLONG*)po,fcopy);
				break;
			case VT_UI8:
				return link( _count,(ULONGLONG*)po,fcopy);
				break;

			}
			return 0;
		}


		template<class O>
		O* copy_to(O* po)
		{
          for(size_t k=0;k<count;k++)
			  po[k]=O(p[k]);

            return po;

		}


		bool linked(){ 
			return !ph;
		};    

		operator N*() {	
			return p;
		}
		N* p,*ph;
		char* name;
		size_t count;

	};


	template <class ValueT=double,class IndexT=i_t >
	struct matrix_RM_t
	{
		typedef ValueT value_t;
		typedef IndexT index_t;
		typedef std::map<index_t,value_t> map_t;


		matrix_RM_t(index_t _n=0){

			reset(_n);
			/*
			rows_buf.resize(n=_n);
			rows=(n)?&rows_buf[0]:0;
			rows--;
			*/
		}

		matrix_RM_t(index_t _n,int ntr,int nd,index_t* tri_indexes,value_t* pmatrix){

			set_elements(_n,ntr,nd,tri_indexes,pmatrix);
		}

		void reset(index_t _n=0)
		{
			nnz=-1;
			rows=0;
			if(rows_buf.size()) rows_buf.clear();
			if(n=_n)
			{
				rows_buf.resize(_n);
				rows=&rows_buf[0];
				rows--;
			}
		}


		index_t set_elements(index_t _n,int ntr,int nd,index_t* tri_indexes,value_t* pmatrix){

			  reset(_n);
			  add_elements(ntr,nd,tri_indexes,pmatrix);
              return calc_nnz();
		}


		void add_elements(int ntr,int nd,index_t* tri_indexes,value_t* pmatrix){

			i_t * indexes=tri_indexes;
			int ndq=nd*nd;
			double* pm=pmatrix;
			for(i_t n=0;n<ntr;n++)
			{
				//i_t * indexes=tri_indexes+n*nd;
				//double* pm=pmatrix+n*nd2;
				add_element(nd,indexes,pm); 
				indexes+=nd;
				pm+=ndq;
			}
		}

		void add_element(int nd,index_t* indexes,value_t* pv){

			value_t* p=pv;

			for (int k=0;k<nd;k++)
			{
				index_t i=indexes[k];
				map_t &row=rows[i];
				for (int m=0;m<nd;m++)
				{
					index_t j=indexes[m];
					row[j]+=p[m];
				}
				p+=nd;
			}

		}

		void add_element(int nd,index_t* indexes,value_t* pv,int nf,int stepf=1,int stepfv=1){

			value_t* p;

			for (int k=0;k<nd;k++)
			{
				index_t it=indexes[k];
				
				for (int c=0;c<nf;c++)
				{
					int csf=c*stepf;
					int csfv=c*stepfv;
					index_t i=it+csf;                       
					map_t &row=rows[i];
					for (int m=0;m<nd;m++)
					{
						index_t j=indexes[m];
						row[j]+=p[m+csfv];
					}
				}
				
			}

		}

		index_t calc_nnz()
		{
			index_t c=0;
			for (index_t k=1;k<=n;k++)
				c+=rows[k].size();
			return nnz=c;		
		}

		index_t get_nnz()
		{
			return  (nnz<0)?calc_nnz():nnz;
		}

		index_t convert_to_COO( matrix_COO_t<value_t>** ppcoo,bool ftranspose=false)
		{
			if(!ppcoo) return -1;
			

			matrix_COO_t<value_t>* pc=new matrix_COO_t<value_t>(n,get_nnz());
			*ppcoo=pc;
			

			i_t* colind0=pc->colind;
			i_t* rowind0=pc->rowind;

			if(ftranspose)
			{
				 //colind0=pc->rowind;
				 //rowind0=pc->colind;
				std::swap(colind0,rowind0);
			}

            index_t j=0;

			for (index_t k=1;k<=n;k++)
			{
				map_t &row=rows[k];
				value_t *pv=pc->a+j;
				i_t *colind=colind0+j,*rowind=rowind0+j;

				for (  map_t::iterator i=row.begin();i!=row.end();++i)
				{
					*(rowind++)=k;
					*(colind++)=i->first;
					*(pv++)=i->second;

				};
				j+=row.size();

			}

			return 0;
		}

		index_t convert_to_CRS( matrix_CRS_t<value_t>** ppcrs,bool ftranspose=false)
		{
			index_t err;
			if(!ppcrs) return -1;
			matrix_COO_t<value_t>* pcoo=0;
			if(!(err=convert_to_COO(&pcoo,ftranspose))) 			              
				err=coo2crs(pcoo,ppcrs);
			delete pcoo;		   
			return err;

		}



		index_t nnz,n;
		map_t* rows;

		std::vector<map_t> rows_buf;
	};

	template<class float_t>
	struct rect2D_t
	{
        float_t xb,xe,yb,ye; 
	};

    template<class float_t>
	inline bool in_rect2D(float_t* v,rect2D_t<float_t>& r)
	{
		bool f=(r.xb<v[0])&&(v[0]<=r.xe)&&(r.yb<v[1])&&(v[1]<=r.ye);
        return f;
	}

	template<class float_t>
	inline bool in_rect2D_closed(float_t* v,rect2D_t<float_t>& r)
	{
		bool f=(r.xb<=v[0])&&(v[0]<=r.xe)&&(r.yb<=v[1])&&(v[1]<=r.ye);
		return f;
	}

	template<class float_t>
	inline bool in_rect2D_inner(float_t* v,rect2D_t<float_t>& r)
	{
		bool f=(r.xb<v[0])&&(v[0]<r.xe)&&(r.yb<v[1])&&(v[1]<r.ye);
		return f;
	}


	template<class float_t>
	inline bool tri_in_rect2D(unsigned int* tri,float_t* vts,rect2D_t<float_t>& r)
	{
       float_t* vx0=vts+2*tri[0];
	   float_t* vx1=vts+2*tri[1];
	   float_t* vx2=vts+2*tri[2];
	   bool f;
	   f=in_rect2D(vx0,r);
	   f=f&&in_rect2D(vx1,r);
	   f=f&&in_rect2D(vx2,r);
	   return f;
	}

	template<class float_t>
	inline bool tri_center_in_rect2D(unsigned int* tri,float_t* vts,rect2D_t<float_t>& r)
	{
		float_t* vx0=vts+2*tri[0];
		float_t* vx1=vts+2*tri[1];
		float_t* vx2=vts+2*tri[2];
		bool f;
		float_t vc[2]={(vx0[0]+vx1[0]+vx2[0])/3,(vx0[1]+vx1[1]+vx2[1])/3};
		f=in_rect2D(vc,r);
		return f;
	}

};

// namespace mkl_holder_utils