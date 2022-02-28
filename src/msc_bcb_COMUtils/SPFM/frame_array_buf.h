#pragma once

#include "isa_frame.h"
#include "Shlwapi.h" 
inline long GetVTSize(VARTYPE vt)
{
	SAFEARRAY* psa;
	SAFEARRAYBOUND sb={1,0};
	if(!(psa=SafeArrayCreate(vt,1,&sb) ) ) return 0;
	long e=psa->cbElements;
	SafeArrayDestroy(psa);
	return e;

}



inline LONG32	Get_fuck_total_size(SAFEARRAY* psa)
{
	if(psa->cDims<=0) return 0;
	LONG32 e=1;
	for(int n=0;n<psa->cDims;n++)
		e*=psa->rgsabound[n].cElements;
	//e*=m_array.cbElements;
	return e;
}


inline LONG64	Get_fuck_total_size_b(SAFEARRAY* psa)
{

	return (psa->cbElements)*LONG64(Get_fuck_total_size(psa));
}

inline LONG32	Get_fuck_total_size(VARIANT &v)
{
	if((v.vt&VT_ARRAY)==0) return 0;
	return Get_fuck_total_size(v.parray);
}

inline LONG32	Get_fuck_total_size_b(VARIANT &v)
{
	if((v.vt&VT_ARRAY)==0) return 0;
	return Get_fuck_total_size_b(v.parray);
}


struct SA_Loker
{
	SAFEARRAY* psa;
	SA_Loker(SAFEARRAY* p)
	{
		psa=p;
		if(psa) SafeArrayLock(psa);
	}
	~SA_Loker()
	{
		if(psa) SafeArrayUnlock(psa);
	}

};

inline bool IsVarAllocNeed(int vt)
{

	if((0<=vt)&&(vt<=7)) return false; 
	if((10<=vt)&&(vt<=11)) return false; 
	if((14<=vt)&&(vt<=25)) return false; 

	return true;
}

inline HRESULT VARIANT_convert(VARIANT* pvd,VARIANT* pvs,VARTYPE vtd)
{
	HRESULT hr;
	VARIANT tmp;

	VARTYPE vts=pvs->vt;

	if((vts&VT_TYPEMASK)==VT_BSTR)
	{
		bool f=(vtd==VT_I1)&&(vtd==VT_I2)&&(vtd==VT_I4)&&(vtd==VT_I8);   
		f=f&&(vtd==VT_UI1)&&(vtd==VT_UI2)&&(vtd==VT_UI4)&&(vtd==VT_UI8);
		if(f)
		{  
			tmp.vt=VT_I8;
			BSTR bstr=0;

			if((vts&VT_BYREF)==0) bstr=pvs->bstrVal;
			else if(pvs->pbstrVal)
				bstr=*(pvs->pbstrVal);

			if(!StrToInt64Ex(bstr,STIF_SUPPORT_HEX,(LONGLONG*)&tmp.llVal))
				return E_FAIL;
			pvs=&tmp;
		} 
	};

	//VariantClear(pvd);
	//pvd->vt=0;
	hr=VariantChangeType(pvd,pvs,0,vtd);
	return hr;
}

inline HRESULT SAFEARRAY_JSON_decode(VARTYPE vtd,SAFEARRAY* psa,wchar_t* parse_str)
{
	HRESULT hr=S_OK;
	VARIANT vparse={VT_BSTR};
	long count=Get_fuck_total_size(psa);
	return S_OK;

}


inline HRESULT SAFEARRAY_convert(SAFEARRAY* pdest,SAFEARRAY* psrc,bool noalloc)
{

	HRESULT hr;
	VARTYPE vts,vtd;
	if(FAILED(hr=SafeArrayGetVartype(psrc,&vts))) return hr;
	if(FAILED(hr=SafeArrayGetVartype(pdest,&vtd))) return hr;
	SA_Loker sl(psrc),sd(pdest);

	int cs=Get_fuck_total_size(psrc);
	int cd=Get_fuck_total_size(pdest);
	if(cs!=cd) return E_INVALIDARG;

	int size_s=psrc->cbElements;
	int size_d=pdest->cbElements;
	char* ps=(char *) psrc->pvData;
	char* pd=(char *) pdest->pvData;
	VARIANT * pvs=(VARIANT *)psrc->pvData;
	VARIANT * pvd=(VARIANT *)pdest->pvData;



	if(vts==VT_VARIANT)
	{

		if(vtd==VT_VARIANT)
			for(int n=0;n<cs;n++)
			{
				bool falloc=IsVarAllocNeed(pvs->vt);
				if(noalloc&&falloc) 
					return DISP_E_BADVARTYPE;
				if(falloc)
				{
					if(FAILED(hr=VariantCopyInd(pvd++,pvs++))) return hr;
				}
				else *(pvd++)=*(pvs++);


				//if(FAILED(hr=VARIANT_convert(pvd++,pvs++,vtd,vts))) return hr;
			}
		else 
			for(int n=0;n<cs;n++)
			{


				VARIANT vd={VT_EMPTY};
				if(FAILED(hr=VARIANT_convert(&vd,pvs++,vtd))) return hr;
				memcpy(pd,&vd.intVal,size_d);
				pd+=size_d;    

			}
			return S_OK;

	}
	else
	{

		VARIANT vs={vts};

		if(vtd==VT_VARIANT)
		{

			bool falloc=IsVarAllocNeed(vts);
			if(noalloc&&falloc) 
				return DISP_E_BADVARTYPE;

			VARIANT * pvd=(VARIANT *)pdest->pvData;
			for(int n=0;n<cs;n++)
			{

				memcpy(&vs.intVal,ps,size_s);
				if(FAILED(hr=VARIANT_convert(pvd++,&vs,vts))) return hr;
				ps+=size_s;    
			}
		}
		else
			for(int n=0;n<cs;n++)
			{
				VARIANT vd={VT_EMPTY};
				memcpy(&vs.intVal,ps,size_s);
				if(FAILED(hr=VARIANT_convert(&vd,&vs,vtd))) return hr;
				memcpy(pd,&vd.intVal,size_d);
				ps+=size_s;    
				pd+=size_d;    
			}
			return S_OK;

	}

	return S_OK;

};

inline HRESULT VARIANT_array_convert(VARIANT* pvd,VARIANT* pvs,VARTYPE vtd,bool fcreate,bool noalloc)
{
	HRESULT hr=E_FAIL;

	if(!pvd) return E_POINTER;
	if(!pvs) return E_POINTER;
	VARTYPE vts;
	VARIANT tmp={0};
	if(((pvs->vt)&(VT_ARRAY))==0) return E_INVALIDARG;


	if(fcreate)
	{
		SA_Loker ssl(pvs->parray);

		SAFEARRAYBOUND sb[64];
		int nDim=pvs->parray->cDims;
		for(int n=0;n<nDim;n++)
		{
			sb[nDim-1-n]=pvs->parray->rgsabound[n];
		};

		if(!(tmp.parray=SafeArrayCreate(vtd,nDim,sb))) return E_FAIL;
		tmp.vt=VT_ARRAY|vtd;
		*pvd=tmp;
	};

	hr=SAFEARRAY_convert(pvd->parray,pvs->parray,noalloc);
	if(FAILED(hr))
	{
		if(fcreate)	VariantClear(pvd);
	}
	return hr;
}


template < class T >
inline void reflectionT(T* p,long n)
{
	T* tail=p+n;

	T* plast=p+(n>>1);

	while(p!=plast)
	{
		T t=*(--tail);
		*tail=*p;
		*(p++)=t;
	}
}



inline int parse_indexes(wchar_t* pstr,int c,int* ind)
{
	if(!pstr) return 0;
	while(iswspace(*pstr)) ++pstr;
	if(*pstr==L'[') ++pstr;
	wchar_t *p,*pn=pstr,*ptail;
	int n=0;
	for(;n<c;n++)
	{
		p=wcstok_s(pn,L",",&ptail);
		pn=0;
		if(!p) break;
		LARGE_INTEGER ll;
		if(StrToInt64ExW(p,STIF_SUPPORT_HEX,&ll.QuadPart))
		{
			ind[n]=ll.LowPart;
			if(ll.HighPart) ind[++n]=ll.HighPart;
		}
		else	return 0;
	}

	return n;
}

inline int parse_slice(wchar_t*& p,int c,int* ind,wchar_t dlm=L':')
{
	if(!p) return 0;
	int cn=0;
	//    if(p)  while(iswspace( static_cast<unsigned short>(*p)))p++;  
	while(cn<c)
	{
		while(iswspace( static_cast<unsigned short>(*p))||(*p==dlm)) p++;  
		if(*p!=L',') break;  
		else p++;
		ind[cn++]=0;
	}
	return cn;
}




inline HRESULT VARIANT_convert2(VARIANT* pvd,VARIANT* pvs,VARTYPE vtd)
{
	HRESULT hr;
	VARIANT tmp;

	VARTYPE vts=pvs->vt;
	VARTYPE vt=vts&VT_TYPEMASK;


	if((vt==VT_BSTR)||(vt==VT_DISPATCH))
	{
		tmp.vt=VT_I8;
		BSTR bstr=0;

		if(vt==VT_DISPATCH)
		{

		}
		else
		{

			if((vts&VT_BYREF)==0) bstr=pvs->bstrVal;
			else if(pvs->pbstrVal)
				bstr=*(pvs->pbstrVal);
		}

		if(!StrToInt64Ex(bstr,STIF_SUPPORT_HEX,(LONGLONG*)&tmp.llVal))
			return E_FAIL;
		pvs=&tmp;
	};


	pvd->vt=0;
	hr=VariantChangeType(pvd,pvs,0,vtd);
	return hr;
}




struct sindexes 
{
	enum
	{
		maxcount=64
	};
	long c;
	long ind[maxcount];
};

inline bool parse_indexes(wchar_t* pstr,sindexes* pind)
{
	if(!pind) return false;
	return   pind->c=parse_indexes(pstr,pind->maxcount,(int*)pind->ind);
}


struct SB_FUCK_HELPER
{
	struct SVARIANT:VARIANT
	{
		SVARIANT(){vt=0;};
		~SVARIANT(){VariantClear(this);}
	};

	union{
		struct{
			int dim;
			int isb[64];};
			sindexes  indexes;
	};
	HRESULT hr;
	bool frev;
	SB_FUCK_HELPER(VARIANT v,bool freverse=true)
	{
		dim=0;
		frev=freverse;
		hr=E_FAIL;
		if(v.vt==VT_R8)
		{
			LARGE_INTEGER ll;
			ll.QuadPart=v.dblVal;
			int n=0;

			if(ll.HighPart) isb[n++]=ll.HighPart;
			isb[n]=ll.LowPart;
			dim=n+1;
			hr=S_OK;
		}

		if(v.vt&VT_ARRAY)
		{
			// if(v.parray->cDims!=1) return; MATLAB FUCK OFF Too...


			SafeArrayLock(v.parray);
			int siz=Get_fuck_total_size(v.parray);
			if(siz>0)
			{
				char* p=(char*) v.parray->pvData;
				int elsize=v.parray->cbElements;
				if(frev)
					for(int n=siz-1;n>=0;n--)
					{
						VARIANT t={v.vt&VT_TYPEMASK},tt,tr={0};   
						if(t.vt==VT_VARIANT)
							memcpy(&t,p,elsize); 
						else   memcpy(&t.intVal,p,elsize);
						hr=VariantChangeType(&tr,&t,0,VT_I4);
						if(FAILED(hr)) break;
						isb[n]=tr.intVal;
						p+=elsize;

					}  

				else 
					for(int n=0;n<siz;n++)
					{
						VARIANT t={v.vt&VT_TYPEMASK},tt,tr={0};   
						if(t.vt==VT_VARIANT)
							memcpy(&t,p,elsize); 
						else   memcpy(&t.intVal,p,elsize);
						hr=VariantChangeType(&tr,&t,0,VT_I4);
						if(FAILED(hr)) break;
						isb[n]=tr.intVal;
						p+=elsize;

					}
					if(SUCCEEDED(hr))	  dim=siz;
			};  
			SafeArrayUnlock(v.parray);

		}
		else
		{ 


			SVARIANT t;

			//hr=VariantChangeType(&t,&v,0,VT_I4);
			//if(FAILED(hr))
			{

				hr=VariantChangeType(&t,&v,0,VT_BSTR);
				if(FAILED(hr)) return;
				int cc=parse_indexes(t.bstrVal,64,isb);
				if(cc==0) hr=E_FAIL;
				else
				{
					if(frev) reflectionT(isb,cc);
					dim=cc;
					return;
				}


				//VariantClear(&t);

				return;
			}
			dim=1;
			*isb=t.intVal;
		}
	}

};







#pragma pack(push, 1)



template<int ndimmax=64>
struct SAFEARRAY_FUCK
{
	void *pVtbl;
	void* pMBV;
	void* punk; 
	ULONG64 total_size;

	enum
	{
		maxdim=ndimmax
	};
	int vt;
	SAFEARRAY m_array;
	SAFEARRAYBOUND radd[ndimmax-1];


	inline SAFEARRAY* parray()
	{
		return &m_array;
	}

	inline static const GUID& iid()
	{
		// {3B0A4445-3F70-4a9d-8AA0-6CF7A61EFFBF}
		static const GUID sgiud = 
		{ 0x3b0a4445, 0x3f70, 0x4a9d, { 0x8a, 0xa0, 0x6c, 0xf7, 0xa6, 0x1e, 0xff, 0xbf } };
		return sgiud;
	}


	inline 	HRESULT Lock()
	{

		if(pMBV)
		{
			HANDLE hmutex=((HANDLE*)pMBV)[1];      
			WaitForSingleObject(hmutex,INFINITE);
		}
		return S_OK;
	}

	inline 	void Unlock()
	{

		if(pMBV)
		{
			HANDLE hmutex=((HANDLE*)pMBV)[1];      
			ReleaseMutex(hmutex);
		}
	}




	inline static const GUID& iid_noref()
	{
		// {243644BD-E4FA-4434-A743-9CCC38B24FCE}
		static const GUID sgiud = 
		{ 0x243644bd, 0xe4fa, 0x4434, { 0xa7, 0x43, 0x9c, 0xcc, 0x38, 0xb2, 0x4f, 0xce } };
		return sgiud;
	}



	SAFEARRAY_FUCK():vt(VT_EMPTY),pVtbl(0)
	{
		//pVtbl=fake_vtbl::vtbl();
	};


	SAFEARRAY_FUCK(VARTYPE tv, int ndim,int* sb,void* pt=0)
	{
		//
		memset(this,0,sizeof(*this));
		//pVtbl=0;//
		fake_vtbl::vtbl();
		//m_array=SAFEARRAY();

		int sizel=GetVTSize(tv);

		m_array.cDims=ndim;
		m_array.fFeatures=FADF_HAVEVARTYPE;//FADF_FIXEDSIZE;
		if(tv==VT_VARIANT) m_array.fFeatures|=FADF_VARIANT;  
		m_array.cbElements=sizel;
		m_array.pvData=(void*)pt;
		vt=tv;
		SAFEARRAYBOUND tt={1,0};
		for(int n=0;n<ndimmax;n++) 
			m_array.rgsabound[n]=tt;

		if(sb)	for(int n=0;n<ndim;n++) 
			m_array.rgsabound[n].cElements=sb[n];

		total_size=get_total_size();
	};
	operator VARIANT()
	{

		VARIANT v={vt|VT_ARRAY};
		v.parray=&m_array;
		return v;
		//VT
	}

	HRESULT CopyTo(VARIANT* pv)
	{
		HRESULT hr;
		if(!pv) return E_POINTER;
		VARIANT v=*this;
		return hr=VariantCopy(pv,&v);

	};


	inline HRESULT calc_index(VARIANT* pv,int cn,LONG64* pind)
	{
		HRESULT hr;
		cn=min(cn,m_array.cDims);
		LONG64 ind;
		VARIANT v={0};
		hr=VariantChangeType(&v,pv+0,0,VT_I8);
		if(FAILED(hr)) return hr;
		ind= v.llVal;
		for(int n=1;n<cn;n++)
		{
			VARIANT v={0};
			hr=VariantChangeType(&v,pv+n,0,VT_I8);
			if(FAILED(hr)) return hr;
			ind+= v.llVal*LONG64(m_array.rgsabound[n-1].cElements);
		};
		*pind=ind;
		return S_OK;
	}

	inline 	LONG64	get_total_sizeb()
	{
		return Get_fuck_total_size_b(&m_array);
		/*
		if(!m_array.cDims) return 0;
		LONG64 e=1;
		for(int n=0;n<m_array.cDims;n++)
		e*=m_array.rgsabound[n].cElements;
		e*=m_array.cbElements;
		*/

	}
	inline 	LONG64	get_total_size()
	{
		return Get_fuck_total_size(&m_array);
	}

	inline HRESULT get_line_ptr(LONG64 ntag,void** pp)
	{
		HRESULT hr;

		char* p=(char* )m_array.pvData;
		if(!p) return E_POINTER;
		*pp=p+ntag*m_array.cbElements;
		return S_OK;
	}

	template<class T> 
	T* begin() 
	{
		T* p=0;
		get_line_ptr(0,(void**)&p);
		return p;
	}

	template<class T> 
	T* end()
	{ 
		T* p=0;
		get_line_ptr(get_total_size(),(void**)&p);
		return p;
	}	

	inline HRESULT make_index(VARIANT* pv,int cn,long* pind)
	{
		HRESULT hr;
		VARIANT v={0};
		for(int n=0;n<cn;n++)
		{
			VARIANT v={0};
			hr=VariantChangeType(&v,pv+(cn-1-n),0,VT_I4);
			if(FAILED(hr)) return hr;
			pind[n]=v.intVal;
		};

		return S_OK;
	}


	inline HRESULT get_ptr(VARIANT* pargs,int count,void** pp)
	{
		HRESULT hr;
		long ii[maxdim]={0};
		if(count>m_array.cDims) return DISP_E_BADINDEX;
		if(FAILED(hr=make_index(pargs,count,ii))) return DISP_E_BADINDEX;
		//return hr=SafeArrayPtrOfIndex(&m_array,ii,pp);
		return hr=get_ptr(ii,count,pp);
	}

	inline HRESULT get_ptr(long* ii,int count,void** pp)
	{
		HRESULT hr;
		if(count>m_array.cDims)
		{
			for(int n=m_array.cDims;n<count;n++)
				if(ii[n]) return DISP_E_BADINDEX;
		}
		return hr=SafeArrayPtrOfIndex(&m_array,ii,pp);
	}

	inline HRESULT check_bounds(LONG64 n )
	{

		if((n<total_size)&&(n>=0))
			return S_OK;
		else  return  DISP_E_BADINDEX;
	}


};

typedef SAFEARRAY_FUCK<> SAFEARRAY_F;


struct SAFEARRAY_MAP_t
{
	ULONG32 vt;
	ULONG32 cDims;
	ULONG32 cbElements;
	ULONG64 total_size;
	LONG32 indexes[64];
	inline operator SAFEARRAY_F()
	{
		SAFEARRAY_F t(vt,cDims,0,0);
		t.total_size=total_size;
		for(int n=0;n<cDims;++n) t.m_array.rgsabound[n].cElements=indexes[n];
		return t;
	};

	inline void operator = (const SAFEARRAY_F& sf)
	{
		vt=sf.vt;
		cDims=sf.m_array.cDims;
		total_size=sf.total_size;
		cbElements=sf.m_array.cbElements;
		for(int n=0;n<cDims;++n) indexes[n]=sf.m_array.rgsabound[n].cElements;
	};

};



struct REGION_SAFEARRAY
{

	LONG64 begin_off;
	LONG64  end_off;
	//SAFEARRAY_F safearrayf;
	//typedef MultiSAFEARRAY::SAFEARRAY_MAP_t SAFEARRAY_MAP_t;
	SAFEARRAY_MAP_t safearrayf;

};

struct fake_vtbl
{
	struct ForMoronicDelphiVtbl
	{
		void * ptr[4+4];
		ForMoronicDelphiVtbl()
		{
			ptr[0]=&fake_QI;  
			ptr[1]=&fake_addref;
			ptr[2]=&fake_release;
			ptr[3]=&TryLock;
			ptr[4]=&Lock;
			ptr[5]=&Unlock;
			ptr[6]=&SAFEARRAY_ptr;
			ptr[7]=0;
		}
		static long __stdcall fake_QI(SAFEARRAY_FUCK<> *p,REFIID riid, void **ppvObject)
		{
			if(p->punk) return ((IUnknown*)(p->punk))->QueryInterface(riid,ppvObject); 
			return E_NOTIMPL;
		};
		static long __stdcall fake_addref(SAFEARRAY_FUCK<> *p)
		{
			if(p->punk) return ((IUnknown*)(p->punk))->AddRef();
			return 1;
		};
		static long __stdcall fake_release(SAFEARRAY_FUCK<> *p)
		{
			if(p->punk) return ((IUnknown*)(p->punk))->Release();
			return 1;
		};
		static  BOOL __stdcall  TryLock(SAFEARRAY_FUCK<> *p, DWORD dwtimeout=0)
		{
			if(!p->pMBV) return 1;
			PHANDLE ph=PHANDLE(p->pMBV)+1;
			return WAIT_TIMEOUT!=WaitForSingleObject(*ph,dwtimeout);
		}
		static   int __stdcall  Lock(SAFEARRAY_FUCK<> *p,VARIANT* pv)
		{
			TryLock(p,INFINITE);
			if(pv) *pv=*p;      
			return 1;
		}
		static   int __stdcall Unlock(SAFEARRAY_FUCK<> *p,VARIANT* pv)
		{
			if(pv) pv->vt=0;      
			if(!p->pMBV) return 0;
			PHANDLE ph=PHANDLE(p->pMBV)+1;
			ReleaseMutex(*ph);
			return 0;
		};

		static   LPSAFEARRAY __stdcall SAFEARRAY_ptr(SAFEARRAY_FUCK<> *p,VARIANT* pv)
		{
			if(pv) *pv=*p;      
			return &(p->m_array);
		}


	};



	inline static void* vtbl()
	{
		static ForMoronicDelphiVtbl s_fakeVtbl;
		return &s_fakeVtbl;
	}



};

inline SAFEARRAY_F read_sizes(VARIANT& vsizes,int vtype)
{


	SB_FUCK_HELPER sbfh(vsizes);

	return SAFEARRAY_F(vtype,sbfh.dim,sbfh.isb);

}





struct SAFEARRAY_reshaper
{


	bool m_f,m_fnull;
	HRESULT hr;

	SAFEARRAY_F& m_psa;
	SAFEARRAY_F m_pslice;
	LONG64 begin_off,end_off;


	inline operator HRESULT()
	{
		return hr;
	}

	inline bool is_optional(VARIANT& v)
	{
		return (v.vt==VT_EMPTY)||(v.vt==VT_ERROR);
	}
	//inline HRESULT VARIANT_array_convert(VARIANT* pvd,VARIANT* pvs,VARTYPE vtd,bool fcreate,bool noalloc)


	inline int make_reshape(int c,int cs,long *indx)
	{
		char *ptr=0,*pshift=0;
		void* pbase=m_psa.parray()->pvData;

		if((pbase)||m_fnull)
		{
			if(FAILED(hr=m_psa.get_ptr(indx,cs,(void**)&pshift)) ) return -1;
			ptr=pshift;
		}
		else
		{
			m_pslice=m_psa;
			pbase= m_pslice.parray()->pvData=(void*)(0x10000);
			if(FAILED(hr=m_pslice.get_ptr(indx,cs,(void**)&pshift)) ) return -1;
		}

		//reflectionT(indx,cs);

		//HRESULT get_ptr(long* ii,int count,void** pp)
		//reflectionT(isb,cc);
		m_f=true;
		m_pslice=SAFEARRAY_F(m_psa.vt,c,NULL,ptr);


		for(int n=0,k=cs-c;n<c;n++,k++)
		{
			m_pslice.parray()->rgsabound[n]=m_psa.parray()->rgsabound[k];
		}
		m_pslice.total_size=m_pslice.get_total_size();

		LONG64 totsize=m_psa.get_total_sizeb();  
		LONG64 totsize_slice=m_pslice.get_total_sizeb();  
		end_off=totsize_slice-totsize;
		begin_off=LONG64(pshift)-LONG64(pbase);

		return c;
		//m_pslice.parray()->cDims=c;

	}

	inline	int   calc_shift(wchar_t*  pstr,long *indx)
	{

		if(!pstr)		return 0;
		int c=0;

		wchar_t* p=pstr;
		int cs=m_psa.parray()->cDims;
		c=parse_slice(p,cs,(int*)indx,L':');
		if(!c) return 0;
		int cd=cs-c;
		//int cp=parse_indexes(p,cd,(int*)&indx[c]);
		if(cd!=parse_indexes(p,cd,(int*)&indx[c])){ hr=E_INVALIDARG; return -1;}

		return make_reshape(c,cs,indx);



		char *ptr=0,*pshift=0;
		void* pbase=m_psa.parray()->pvData;

		if((pbase)||m_fnull)
		{
			if(FAILED(hr=m_psa.get_ptr(indx,cs,(void**)&pshift)) ) return -1;
			ptr=pshift;
		}
		else
		{
			m_pslice=m_psa;
			pbase= m_pslice.parray()->pvData=(void*)(0x10000);
			if(FAILED(hr=m_pslice.get_ptr(indx,cs,(void**)&pshift)) ) return -1;
		}



		//reflectionT(indx,cs);

		//HRESULT get_ptr(long* ii,int count,void** pp)
		//reflectionT(isb,cc);
		m_f=true;
		m_pslice=SAFEARRAY_F(m_psa.vt,c,NULL,ptr);


		for(int n=0,k=cs-c;n<c;n++,k++)
		{
			m_pslice.parray()->rgsabound[n]=m_psa.parray()->rgsabound[k];
		}
		m_pslice.total_size=m_pslice.get_total_size();

		LONG64 totsize=m_psa.get_total_sizeb();  
		LONG64 totsize_slice=m_pslice.get_total_sizeb();  
		end_off=totsize_slice-totsize;
		begin_off=LONG64(pshift)-LONG64(pbase);

		return c;
		//m_pslice.parray()->cDims=c;


	}

	SAFEARRAY_reshaper(SAFEARRAY_F& src,VARIANT sizes=VARIANT(),bool fnull=true)
		:m_psa(src),m_f(false),m_fnull(fnull),begin_off(0),end_off(0)
	{
		long indx[64]={0};
		hr=S_OK;
		//memset(indx,0,sizeof(indx));
		if(is_optional(sizes)) return;
		SB_FUCK_HELPER::SVARIANT ss;
		if(SUCCEEDED(VariantChangeType(&ss,&sizes,0,VT_BSTR)))
		{		   
			if(SysStringLen(ss.bstrVal)==0)
			{
				sizes.vt=VT_I8;
				sizes.llVal=m_psa.get_total_size();

			}
			else
			{
				if(calc_shift(ss.bstrVal,indx)) 
					return;
			}

		}
		//if()
		hr=E_FAIL;
		m_pslice=read_sizes(sizes,m_psa.vt);
		if(m_pslice.parray()->cDims<=0) return;
		LONG64 c=m_pslice.get_total_size();
		LONG64 cs=m_psa.get_total_size();
		if(c<=cs)
		{
			void* ptr=m_psa.parray()->pvData;
			m_pslice.parray()->pvData=ptr;
			if(m_f=ptr) hr=S_OK;
		}
		LONG64 totsize=m_psa.get_total_sizeb();  
		LONG64 totsize_slice=m_pslice.get_total_sizeb();  
		end_off=totsize_slice-totsize;

	}

	inline  SAFEARRAY_F& saf()
	{
		if(m_f) return m_pslice;
		else return m_psa;
	}


};




template <class TLock>
struct LOCKER_PTR
{
	TLock *p;
	LOCKER_PTR(TLock *pp=0):p(0)
	{
		Lock(pp);
	}
	void Lock(TLock *pp)
	{
		if(p) p->Unlock();
		p=pp;
		if(p) p->Lock();
	}
	~LOCKER_PTR()
	{
		if(p) p->Unlock();
	}
};





struct MultiSAFEARRAY
{  



	union
	{
		struct
		{
			union
			{
				struct{ ULONG64 uisign[2] ;};
				struct{ char csign[16] ;};
				struct{ GUID iid ;};
			};
			LONG64 capacity;
			LONG64 item_size_b;
			LONG64 item_size;
			LONG64 offset;
			union{
				struct{
					volatile   LONG cursor_l;
					volatile   LONG cursor_h; };
					struct{  volatile   LONG64 cursor;  };
			};

			//
			SAFEARRAY_MAP_t 	safearray;
			//SAFEARRAY_F	safearray;
			char comment[32*1024];
		};

		struct { char reserved[64*1024];};
	};


	inline HRESULT check_bounds(LONG64 n )
	{

		if((n<item_size)&&(n>=0))
			return S_OK;
		else  return  DISP_E_BADINDEX;
	}

	static inline DWORD os_granularity()
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return si.dwAllocationGranularity;
	}

	static inline ULONG64 need_size(VARIANT& vsizes,int vtype,ULONG64 frame_count=1)
	{
		SAFEARRAY_FUCK<64> sff=read_sizes(vsizes,vtype);

		static ULONG64 ff= os_granularity()-1;
		ULONG64 fmask=~ff;
		return  fmask&(sff.get_total_sizeb()+sizeof(MultiSAFEARRAY)+ff);
	}


};

#pragma pack(pop)



typedef isa_frame_list<MultiSAFEARRAY>    isa_frame_buf;

struct decommiter_t
{
	isa_frame_buf* m_p;
	decommiter_t(isa_frame_buf* p):m_p(p){};
	~decommiter_t(){if(m_p) m_p->Decommit();}
};


template <class N=double>
struct isa_frame_buf_helper
{

   struct locker_t
   {
      isa_frame_buf_helper fbh;
	  locker_t(isa_frame_buf_helper& _fbh):fbh(_fbh)
	  {
		  fbh.buf().Lock();
	  }
	  ~locker_t(){
           fbh.buf().Unlock();
	  }


   };




	isa_frame_buf* m_pbuf;


	inline static const GUID& iid()
	{
		return isa_frame_buf::iid();
	}

	inline isa_frame_buf& buf()
	{
		return *m_pbuf;
	}
	isa_frame_buf_helper(isa_frame_buf* pbuf=0)
	{
		m_pbuf=pbuf;
	};
	inline isa_frame_buf* operator ->()
	{
		return m_pbuf;
	}
	inline N* operator[](LONG64 ind)
	{
		return (N*)m_pbuf->getFramePtr(ind*sizeof(N));
	}

	inline void** GetAddress()
	{
		m_pbuf=0;
		return (void**)&m_pbuf;
	}
};


/*
inline  Item* operator[](long64 ind)
{
long64 offset=pheader->offset;
long64 isizeb=pheader->item_size_b;

return (Item*)get_ptr(ind*isizeb+offset,isizeb);
}
*/

//__declspec(novtable) struct isa_frame_buf:IUnknown

