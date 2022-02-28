#pragma once
#include <atldef.h>
#include <comdef.h>
#include  <memory>
#define MAX_SAFEARRAY_DIM 64
#define SA_VT(psa) sf_vt_type(psa)
//(((psa->fFeatures)&&FADF_HAVEVARTYPE)? *(((int*)psa)-1):-1)
//#define SAFEARRAY_INDEX_C_ORDER_DISABLE
 

  
inline VARTYPE sf_vt_type(SAFEARRAY* psa)
{
  VARTYPE vt=0;
 if(SUCCEEDED(SafeArrayGetVartype(psa,&vt)) ) return vt;
 else return -1;
}



#define _SB safearraybound


inline VARIANT VARIANT_ptr(VARTYPE vt,const void* ptr)
{
   VARIANT v={vt|VT_BYREF,0,0,0};
    v.byref=(void*)ptr;
	return v;
}

inline HRESULT VARIANT_QueryRef(VARIANT &v,void** ptr)
{ 
	if((v.vt)&VT_BYREF)
	{
		*ptr=v.byref;
		return S_OK;
	}
  return S_FALSE;
}

template <class T>
 HRESULT VARIANT_QueryRef_T(VARIANT &v,T& ptr)
{ 

	if((v.vt)&VT_BYREF)
	{
		ptr=((T)(v.byref));
		return S_OK;
	}
  return S_FALSE;
}

 HRESULT VARIANT_QueryRef_T(VARIANT &v,VARIANT*& ptr)
{ 

	if((v.vt)&VT_BYREF)
	{
		ptr=((VARIANT*)(v.byref));
		return S_OK;
	}
  return S_FALSE;
}
///*
 HRESULT VARIANT_QueryRef_T(VARIANT &v,SAFEARRAY*& ptr)
{ 

	if((v.vt)&VT_BYREF)
	{
		ptr=*((SAFEARRAY**)(v.byref));
		return S_OK;
	}
  return S_FALSE;
}
//*/


inline VARIANT VARIANT_Ref(const VARIANT& v)
{
if(VT_BYREF&v.vt) return v;
else return VARIANT_ptr(v.vt|VT_BYREF,&(v.byref));
};



inline VARIANT VARIANT_Ref_VARIANT(const VARIANT& v)
{
if((VT_VARIANT|VT_BYREF)==v.vt) return v;
else return VARIANT_ptr(VT_VARIANT|VT_BYREF,(void*)&v);
	/*
	if(V_VT(pv)&VT_BYREF)
		return *pv;
	else 
		return VARIANT_ptr(V_VT(pv)|VT_BYREF,pv);
		*/
};




class safearraybound :public SAFEARRAYBOUND
{
	public:
inline 		safearraybound(){ this->cElements=1;  this->lLbound=0;}

inline 		safearraybound(const int cb,const int l=0)
{
this->cElements=cb;
this->lLbound=l;
}
/*
safearraybound& operator =(const int cb)
{
  this->cElements=cb;
  return *this;
}
 safearraybound& operator,(const int l)
   {
       
	   this->lLbound=l;
       return *this;
   }		
 */
}; 
inline long SAFEARRAY_GetVTSize(VARTYPE vt)
{
        SAFEARRAY* psa;
		safearraybound sb;
		if(!(psa=SafeArrayCreate(vt,1,&sb) ) ) return 0;
		long e=psa->cbElements;
		SafeArrayDestroy(psa);
		return e;

}
inline VARIANT VARIANT_from_SAFEARRAY(SAFEARRAY* psa)
{
	VARIANT v={0,0,0,0};
	 // VariantInit(&v);
	  //	memset(&v,0,sizeof(VARIANT));
	  v.vt=SA_VT(psa)|VT_ARRAY ;
	  v.parray=psa;
	  return v;
}

template <class T>
inline T&  SafeArrayRefOfIndex(SAFEARRAY* psa,long * pi)
{
	T *pt=NULL;
  
 SafeArrayPtrOfIndex(psa,pi,(void**)&pt); 
 return *pt;
}

inline VARIANT VARIANT_Ref_SafeArrayOfIndex(SAFEARRAY* psa,long * pi)
{
	
  VARIANT V={VT_EMPTY,0,0,0};
	if(psa)
	{
      
      void* ptr;

	 if (S_OK== SafeArrayPtrOfIndex(psa,pi,&ptr))
	 {
	    V.vt=SA_VT(psa)|VT_BYREF;
		V.byref=ptr;
	 }
   	};
  return V;
}


/*
inline VARIANT VARIANT_REF_from_SAFEARRAY(SAFEARRAY* psa)
{
      VARIANT v;
	  VariantInit(&v);
	  v.vt=SA_VT(psa)|VT_ARRAY|VT_BYREF;
	  v.parray=psa;
	  return v;
}
*/
inline size_t SAFEARRAY_total_elements(SAFEARRAY* psa)
{
	size_t res=1;
	long ub,lb;
	for(int n=1;n<=psa->cDims;n++)
	{
          SafeArrayGetUBound(psa,n,&ub);
          SafeArrayGetLBound(psa,n,&lb);
		  res*=( ub-lb+1);
	}
	return res;
}

inline size_t SAFEARRAY_total_size(SAFEARRAY* psa)
{

	return SAFEARRAY_total_elements(psa)*(psa->cbElements);
}


inline SAFEARRAY* SAFEARRAY_reshape(SAFEARRAY* psa,long nd,SAFEARRAYBOUND* prb )
{
int totn=1, n,k;
for( n=0;n<nd;n++) totn*=prb->cElements;
	if(SAFEARRAY_total_elements(psa)!=totn) 
		 return NULL;
for( n=0,k=nd-1;n<nd;n++,k--) 
     psa->rgsabound[n]=prb[k];
psa->cDims=nd; 

return psa;
}

inline SAFEARRAY* SAFEARRAY_reshape1D(SAFEARRAY* psa)
{
		
	long cd=SAFEARRAY_total_elements(psa);
	SAFEARRAYBOUND b={cd,0};
	psa->cDims=1;
	psa->rgsabound[0]=b;
 return psa;
}
inline bool SAFEARRAY_copy_data(SAFEARRAY* dest,SAFEARRAY* src)
{
    size_t cb=SAFEARRAY_total_size(src);
    size_t _cb=SAFEARRAY_total_size(dest);
     if(cb!=_cb) return false;
    SafeArrayLock(src);
     SafeArrayLock(dest);
memmove(dest->pvData,src->pvData,cb);

   SafeArrayUnlock(dest);
   SafeArrayUnlock(src);


   return true;

}

template < class T>
size_t _dist(T* pb,void* pe)
{
	return (size_t(pe)-size_t(pb))/sizeof(T);
}

template < class T >
inline void reflection(T* p,long n)
{
	T* tail=p+n;

	T* plast=p+(n>>1);
	register T t;
	while(p!=plast)
	{
		t=*(--tail);
		*tail=*p;
			*(p++)=t;
	}
	//n>>=1;

}

template < class T >
inline void reflection(T* p,void* vtail)
{
	T* tail=(T*)vtail;
//	size_t n=size_t(tail)-size_t(p);
	
	T* plast=p+size_t((size_t(tail)-size_t(p))/(sizeof(T)<<1));
	register T t;
	while(p!=plast)
	{
		t=*(--tail);
		*tail=*p;
			*(p++)=t;
	}
	//n>>=1;

}

//#define _SB SAFEARRAYBOUND

/*
inline SAFEARRAYBOUND operator  =(int cb)
{
	SAFEARRAYBOUND b={cb,0};
	return b;
};
*/

class safearray
{
	
  public:
	   HRESULT err;
       bool fcanfree;
	  SAFEARRAY* psa;

	  union
	  {
	   struct { SAFEARRAYBOUND rb[MAX_SAFEARRAY_DIM]; };
	   struct { long _ind[MAX_SAFEARRAY_DIM]; };
	  };

      long _pdims[MAX_SAFEARRAY_DIM];

template < class T >
inline void  set_c_array_order(T* p,long n)
{
   #ifndef SAFEARRAY_INDEX_C_ORDER_DISABLE

   reflection(p,n);

   #endif
}
/*
inline void  set_c_array_order(void* p,int n)
{
   #ifndef SAFEARRAY_INDEX_C_ORDER_DISABLE
   reflection((long*)p,n);

   #endif
}
*/

#define _pb _pT<SAFEARRAYBOUND>
#define _pi(a,b) _pT((long*)(&a),&b)

template <class T>
inline T* _pT(T* p,void *t)
{
	long n=(long(t)-long(p))/sizeof(T)+1;
   if(n>ndim) n=ndim;  
   ZeroMemory(rb,sizeof(T)*MAX_SAFEARRAY_DIM);
  // memset(rb,0,); 
   for(int i=0;i<n;i++) ((T*)rb)[i]=((T*)p)[i];
   return (T*)rb;
 }
/*
inline    safearray(VARIANT& V)
{
	fcanfree=0;
	psa=V.parray;
}
*/
//inline    sss(int _vt,_SB b0=_SB(-1)){ return 0;} ;//,_SB b1,_SB b2,_SB b3,_SB b4,_SB b5,_SB b6,_SB b7,_SB b9,_SB bend )

inline	  safearray():psa(NULL),fcanfree(0){};
inline	  safearray(SAFEARRAY* _psa,bool f=0):psa(_psa),fcanfree(f){lock();}
///*
inline	  safearray(VARIANT& a,bool f=0):psa(NULL),fcanfree(f)
{
	//fcanfree=f;

	if(a.vt==(VT_BYREF|VT_VARIANT))
	{
		psa=a.pvarVal->parray;
	}
	else if( a.vt&VT_BYREF)
		psa=*(a.pparray);
	else   psa=a.parray;
   	lock();
};
//*/
//inline	  safearray(int _vt,long i1=-1,long i2=-1,long i3=-1,long i4=-1 ,long i5=-1,long i6=-1 )

inline SAFEARRAY* create2(int _vt,int cx,int cy)
{


	//int nd=0,nmax=(int(pe)-int(pb))/sizeof(_SB);
	//for(nd=0;(nd<nmax)&&(pb[nd].cElements!=UINT(-1));nd++);

	//set_c_array_order(pb,nd);
	//		 std::auto_ptr<safearraybound> tmp(new safearraybound[nmax]);
	//	
	//safearraybound* tmp =new safearraybound[nmax];
	//for(int n=nmax-nd,k=0;n<nmax;n++,k++) tmp[n]=pb[k];
	int nd=2;
	SAFEARRAYBOUND tmp[2]={{cx,0},{cy,0}};
	if(psa=SafeArrayCreate(_vt,nd,tmp))
		lock();
	//	psa->cDims=nd;
	return psa;
	;

}


inline SAFEARRAY* create_ex(int _vt,_SB* pb,_SB* pe,void* pextra=NULL)
{
	
	
	int nd=0,nmax=(int(pe)-int(pb))/sizeof(_SB);
           for(nd=0;(nd<nmax)&&(pb[nd].cElements!=UINT(-1));nd++);
         
		 set_c_array_order(pb,nd);
		 //		 std::auto_ptr<safearraybound> tmp(new safearraybound[nmax]);
         //	
		 safearraybound* tmp =new safearraybound[nmax];
		 for(int n=nmax-nd,k=0;n<nmax;n++,k++) tmp[n]=pb[k];
     	 if(psa=SafeArrayCreateEx(_vt,nmax,tmp,pextra))
			 psa->cDims=nd;
		 delete tmp;
		 return psa;
			 ;
		 

}
   
   
  safearray(int _vt,void* pextra,_SB b0=_SB(-1),_SB b1=_SB(-1),_SB b2=_SB(-1),_SB b3=_SB(-1),
					_SB b4=_SB(-1),_SB b5=_SB(-1),_SB b6=_SB(-1),_SB b7=_SB(-1),_SB bend=_SB(-1))
	  {
        fcanfree=1;
       psa=create_ex(_vt,&b0,&bend,pextra); 
         lock();
	  };


   safearray(int _vt,_SB b0=_SB(-1),_SB b1=_SB(-1),_SB b2=_SB(-1),_SB b3=_SB(-1),
					_SB b4=_SB(-1),_SB b5=_SB(-1),_SB b6=_SB(-1),_SB b7=_SB(-1),
					_SB b8=_SB(-1),_SB b9=_SB(-1),_SB b10=_SB(-1),_SB b11=_SB(-1),_SB b12=_SB(-1),
					_SB b13=_SB(-1),_SB b14=_SB(-1),
					_SB bend=_SB(-1))
	  {
        fcanfree=1;
       psa=create_ex(_vt,&b0,&bend); 
/*	
	    _SB *pb=&b0,*pe=&bend+1;

        	int nd=0,nmax=(int(pe)-int(pb))/sizeof(_SB);
           for(nd=0;(nd<nmax)&&(pb[nd].cElements!=UINT(-1));nd++);
		 //  for(int n=nd;n<nmax;n++) pb[n]=1;
         
		 set_c_array_order(pb,nd);
         safearraybound* tmp =new safearraybound[nmax];
		 for(int n=nmax-nd,k=0;n<nmax;n++,k++) tmp[n]=pb[k];
         

		 psa=SafeArrayCreate(_vt,nmax,tmp);
		 delete tmp;
		 psa->cDims=nd;
*/
         lock();
		// 

	  };

inline      ~safearray(){  clear();}

inline HRESULT redim(_SB b0=_SB(-1),_SB b1=_SB(-1),_SB b2=_SB(-1),_SB b3=_SB(-1),
					_SB b4=_SB(-1),_SB b5=_SB(-1),_SB b6=_SB(-1),_SB b7=_SB(-1),_SB bend=_SB(-1))
{ 
     _SB *pb=&b0,*pe=&bend+1;
        	int nd=0,nmax=_dist(pb,pe);
           for(nd=0;(nd<nmax)&&(pb[nd].cElements!=UINT(-1));nd++);
		  if(nd!=psa->cDims) return  E_INVALIDARG;	
//
		#ifdef SAFEARRAY_INDEX_C_ORDER_DISABLE
        reflection(pb,nd);
        #endif
		 // set_c_array_order(pb,nd);
	unsigned long cLocks=psa->cLocks;
    while(psa->cLocks) unlock();
	err= SafeArrayRedim(psa,pb);
	while(cLocks--) lock();
	return err;
}

inline SAFEARRAY* reshape
(_SB b0=_SB(-1),_SB b1=_SB(-1),_SB b2=_SB(-1),_SB b3=_SB(-1),
_SB b4=_SB(-1),_SB b5=_SB(-1),_SB b6=_SB(-1),_SB b7=_SB(-1),_SB bend=_SB(-1))
{
//SAFEARRAY* 
_SB *pb=&b0,*pe=&bend+1;
        	int nd=0,nmax=_dist(pb,pe);
           for(nd=0;(nd<nmax)&&(pb[nd].cElements!=UINT(-1));nd++);
		   set_c_array_order(pb,nd);
return SAFEARRAY_reshape( psa,nd,pb);
};
inline HRESULT clear()
{

 //SafeArrayUnlock(psa);
 err=S_OK;
 if(fcanfree) err=SafeArrayDestroy(detach());
 else detach();
 return err;
}

	  inline int get_ndim(){return psa->cDims;}
      inline long get_dims(int nd)
	  {
		  long ub=-1,lb;
		  #ifndef SAFEARRAY_INDEX_C_ORDER_DISABLE
           nd=ndim-nd;
          #else
		   nd++;
          #endif
		  SafeArrayGetUBound(psa,nd,&ub);
          SafeArrayGetLBound(psa,nd,&lb);
		  return ub-lb+1;
	  }
 inline long* get_pdims()
 {
    memset(_pdims,0,sizeof(_pdims));
    for(int i=0;i<ndim;i++) _pdims[i]=get_dims(i);
	return _pdims;
 }
	  inline int get_vt(){return SA_VT(psa);}

	  inline size_t get_total_count(){	return  SAFEARRAY_total_elements(psa);}
inline	  SAFEARRAY* detach(VARIANT* pV=NULL)
	  { SAFEARRAY* tmp=psa;
        unlock();
	    psa=0;
		
        if(pV) *pV=VARIANT_from_SAFEARRAY(tmp);
		return tmp;
	  }	  ;

inline	  VARIANT detachV()
{
	VARIANT r;
	detach(&r);
	return r;
}
 inline   int  attach(SAFEARRAY* _psa)
	{  
	    clear();
	    psa=_psa;
		lock();
       	return (psa)? get_ndim():0;
	  }	 
inline   int  attach(VARIANT& a)
{
	SAFEARRAY * _psa;
	if( a.vt&VT_BYREF)
		_psa=*(a.pparray);
	else   _psa=a.parray;
	return attach(_psa);
}
inline VARIANT VARIANT_Array_Ref()
{
	VARIANT r={get_vt()|VT_ARRAY|VT_BYREF,0,0,0};
	r.pparray=&psa;
	return r;
}
inline void* ptr()
{
	if((psa)&&(psa->cLocks)) return psa->pvData;
	return NULL;
}

inline	SAFEARRAY* lock(){SafeArrayLock(psa);return psa;};
inline	SAFEARRAY* unlock(){SafeArrayUnlock(psa);return psa;};

inline _variant_t  get_v(long i1=0,long i2=0,long i3=0
,long i4=0,long i5=0,long i6=0,long i7=0,long ie=0)
{
	set_c_array_order(&i1,get_ndim());
    	void* pval;
		_variant_t res;

    if(SA_VT(psa)==VT_VARIANT)
	{
		
		pval=(VARIANT*)&res;
	}
	else
	{
	res.ChangeType(SA_VT(psa));
     pval=&(res.uintVal);
	}
	SafeArrayGetElement(psa,&i1,pval);
	return res;
		
};

inline void puts(void* pi,const VARIANT& v)
{  

    set_c_array_order((long*)pi,get_ndim());

	void* pval;

    if(SA_VT(psa)==VT_VARIANT)
	{
		
		pval=(VARIANT*)&v;
	}
	else
	{
   _variant_t res,src(v) ;
	//
     res.ChangeType(SA_VT(psa),&src);

    pval=&(res.uintVal);
	}
   err=	SafeArrayPutElement(psa,(long*)pi,pval);

};
inline const _variant_t&  put_v(long ie,const _variant_t& v)
 {
           // 
   	puts(_pi(ie,ie),v);return v;
          //  
	//long* pp=  _pT((long*)(&ie),(&ie));

 }
inline const _variant_t&  put_v(long i1,long ie,const _variant_t& v)
{
             puts(_pi(i1,ie),v);return v;
 }
inline const _variant_t&  put_v(long i1,long i2,long ie,const _variant_t& v)
{
              puts(_pi(i1,ie),v);return v;
 }

inline const _variant_t& put_v(long i1,long i2,long i3,long ie,const _variant_t& v)
{
               puts(_pi(i1,ie),v);return v;
 }

inline void _puts(void* pi,const VARIANT& v)
{  
    set_c_array_order((long*)pi,get_ndim());
    lock();
    
   // VARIANT *pV;


	unlock();
    

};

inline LPSAFEARRAY get_s
(long i1=0,long i2=0,long i3=0,long i4=0,long i5=0,
 long i6=0,long i7=0,long i8=0,long i9=0)
{
	set_c_array_order(&i1,get_ndim());
    VARIANT v;
   //	
	SafeArrayGetElement(psa,&i1,&v);
	return v.parray;
}
/*
#define ARGS_0(t,a)
#define ARGS_1(t,a) ARGS_0(t,a)  t a##1
#define ARGS_2(t,a) ARGS_1(t,a), t a##2
#define ARGS_3(t,a) ARGS_2(t,a), t a##3
#define ARGS_4(t,a) ARGS_3(t,a), t a##4
#define ARGS_5(t,a) ARGS_4(t,a), t a##5
#define ARGS_6(t,a) ARGS_5(t,a), t a##6
#define ARGS_7(t,a) ARGS_6(t,a), t a##7
#define ARGS_8(t,a) ARGS_7(t,a), t a##8
 */
#define ARGS_0(a)
#define ARGS_1(a) ARGS_0(a)   a##1
#define ARGS_2(a) ARGS_1(a),  a##2
#define ARGS_3(a) ARGS_2(a),  a##3
#define ARGS_4(a) ARGS_3(a),  a##4
#define ARGS_5(a) ARGS_4(a),  a##5
#define ARGS_6(a) ARGS_5(a),  a##6
#define ARGS_7(a) ARGS_6(a),  a##7
#define ARGS_8(a) ARGS_7(a),  a##8

#define _REPEAT_MACRO_8(m,proc,proc_p,t,a,vv) \
	m(0,proc,proc_p,t,a,vv);\
	m(1,proc,proc_p,t,a,vv);\
	m(2,proc,proc_p,t,a,vv);\
	m(3,proc,proc_p,t,a,vv);\
	m(4,proc,proc_p,t,a,vv);\
	m(5,proc,proc_p,t,a,vv);\
	m(6,proc,proc_p,t,a,vv);\
	m(7,proc,proc_p,t,a,vv);\
	m(8,proc,proc_p,t,a,vv);
/*
	m(arg,1);\
	m(arg,2);\
	m(arg,3);\
	m(arg,4);\
	m(arg,5);\
	m(arg,6);\
 	m(arg,7);\
	m(arg,8);

//*/

//#define ARGS_XX(t,a,n) ARGS_##n(t,a)
#define DECL_GET_PROP(n,proc,proc_p,t,a) proc_p (ARGS_##n (t a))\
{ return proc(ARGS_##n(a)); };


 _REPEAT_MACRO_8(inline LPSAFEARRAY DECL_GET_PROP,get_s,get_s_p,long,i)
 _REPEAT_MACRO_8(inline _variant_t DECL_GET_PROP,get_v,get_v_p,long,i)
 _REPEAT_MACRO_8(inline VARIANT DECL_GET_PROP,get_V,get_V_p,long,i)

//{ return proc(ARGS_##n(,a)); }	
//DECL_GET_PROP(get_s,get_s_p,long,i,0);
/*
inline LPSAFEARRAY DECL_GET_PROP(get_s,get_s_p,long,i ,0);
inline LPSAFEARRAY DECL_GET_PROP(get_s,get_s_p,long,i,1);
inline LPSAFEARRAY DECL_GET_PROP(get_s,get_s_p,long,i,2);
inline LPSAFEARRAY DECL_GET_PROP(get_s,get_s_p,long,i,3);
inline LPSAFEARRAY DECL_GET_PROP(get_s,get_s_p,long,i,4);
inline LPSAFEARRAY DECL_GET_PROP(get_s,get_s_p,long,i,5);
inline LPSAFEARRAY DECL_GET_PROP(get_s,get_s_p,long,i,6);
inline LPSAFEARRAY DECL_GET_PROP(get_s,get_s_p,long,i,7);
inline LPSAFEARRAY DECL_GET_PROP(get_s,get_s_p,long,i,8);
*/
inline void  put_s(long ie,const LPSAFEARRAY p)
 {

             puts(_pi(ie,ie),VARIANT_from_SAFEARRAY(p));
 }
inline void  put_s(long i1,long ie,const LPSAFEARRAY p)
{
              puts(_pi(i1,ie),VARIANT_from_SAFEARRAY(p));
 }
inline void  put_s(long i1,long i2,long ie,const LPSAFEARRAY p)
{
              puts(_pi(i1,ie),VARIANT_from_SAFEARRAY(p));
 }

inline void put_s(long i1,long i2,long i3,long ie,const LPSAFEARRAY p)
{
              puts(_pi(i1,ie),VARIANT_from_SAFEARRAY(p));
}


inline VARIANT& _putV(long* pi,VARIANT& r)
{
      // 	     if((vt!=r.vt)&&(r.vt&VT_ARRAY)) _com_issue_error(DISP_E_TYPEMISMATCH);
		//	 _com_util::CheckError(DISP_E_TYPEMISMATCH);
	    
		 set_c_array_order(pi,get_ndim());
         void* ptr;
//		
		 _com_util::CheckError(SafeArrayPtrOfIndex(psa,pi,&ptr));

//VARIANT VR=VARIANT_Ref_SafeArrayOfIndex(psa,pi);


///*         
		 if(vt==VT_VARIANT)
		 {
           VariantClear((VARIANT*)ptr);
	        *((VARIANT*)ptr)=r;
			return r;
		
		 }
         if(vt==r.vt)
		 {
           memmove(ptr,r.byref,psa->cbElements);
		   return r;
		 }

 _com_issue_error(DISP_E_TYPEMISMATCH);
          //VARIANT VR=VARIANT_Ref_SafeArrayOfIndex(psa, pi)  
  //*/ 
}

///*
inline VARIANT put_V(long ie,VARIANT r)
{
      return 	_putV(_pi(ie,ie),r);
}
inline VARIANT put_V(long i1,long ie,VARIANT r)
{
      return 	_putV(_pi(i1,ie),r);
}

inline VARIANT put_V(long i1,long i2,long ie,VARIANT r)
{
      return 	_putV(_pi(i1,ie),r);
}

inline VARIANT put_V(long i1,long i2,long i3,long ie,VARIANT r)
{
      return 	_putV(_pi(i1,ie),r);
}

inline VARIANT put_V(long i1,long i2,long i3,long i4, long ie,VARIANT r)
{
      return 	_putV(_pi(i1,ie),r);
}

inline VARIANT put_V(long i1,long i2,long i3,long i4 ,long i5,long ie,VARIANT r)
{
      return 	_putV(_pi(i1,ie),r);
}

inline VARIANT put_V(long i1,long i2,long i3,long i4 ,long i5,long i6,long ie,VARIANT r)
{
      return 	_putV(_pi(i1,ie),r);
}

inline VARIANT get_V(long i1=0,long i2=0,long i3=0,long i4=0 
					 ,long i5=0,long i6=0,long i7=0,long last=0)
{
	 set_c_array_order(&i1,get_ndim());
   return	VARIANT_Ref_SafeArrayOfIndex(psa,&i1);
}




//*/
inline VARIANT& VarRef(long i1=0,long i2=0,long i3=0,long i4=0 ,long i5=0,long i6=0,long last=0)
{
        set_c_array_order(&i1,get_ndim());
      return SafeArrayRefOfIndex<VARIANT>(psa,&i1);   

}




//   void __stdcall put_v(long i1=0,......................

__declspec(property(get=get_total_count)) size_t tc;
__declspec(property(get=get_total_count)) size_t total_count;
__declspec(property(get=get_pdims)) long* pdims;
__declspec(property(get=get_dims)) long dims[];
__declspec(property(get=get_ndim)) int ndim;
__declspec(property(get=get_vt)) int vt;
__declspec(property(get=get_v_p, put=put_v)) _variant_t v[];

__declspec(property(get=get_s_p, put=put_s)) LPSAFEARRAY s[];
__declspec(property(get=get_s_p, put=put_s)) LPSAFEARRAY array[];

//__declspec(property(get=VarRef,put=put_V)) VARIANT V[];
__declspec(property(get=get_V_p,put=put_V)) VARIANT V[];

template <class T>
inline long load_ptr(T* pt,long count=-1)
{
	if(count==-1) count=total_count;
	lock();
	T* ps=(T*)ptr();
	VARIANT VV={SA_VT(psa),0,0,0};
	_variant_t vv(VV);
	for(long k=0;k<count;k++)
	{
	  vv=pt[k];
	  ps[k]=vv;
	};
    unlock();
   return count;
};

template <class T>
inline long save_ptr(T* pt,long count=-1)
{
///*
	if(count==-1) count=total_count;
	lock();
//	T* ps=(T*)ptr();
	VARIANT VV={SA_VT(psa)|VT_BYREF,0,0};
		VV.byref=ptr();
	
	for(long k=0;k<count;k++)
	{
		_variant_t vv(VV);
		VV.pbVal+=psa->cbElements;
	    pt[k]=vv;
	};
    unlock();
   return count;
   //*/
};

inline operator VARIANT() const
 {
    return VARIANT_from_SAFEARRAY(psa) ;
	
 }
inline operator LPSAFEARRAY()
 {
      
	  return psa;
 }

};




// Simplest IMalloc Implementation :
template <VARTYPE VT=VT_I1>
class __declspec(dllexport)
//ATL_NO_VTABLE  
  IMalloc_SA_Impl: public IMalloc
{
public:
   safearray sa;
   
   inline SAFEARRAY* detach()
   {
	   return sa.detach();
   }
   inline VARIANT detachV()
   {
	   return sa.detachV();
   }
	STDMETHOD_(ULONG, AddRef)() {return 0;}
	STDMETHOD_(ULONG, Release)(){return 0;}
	
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{return E_NOINTERFACE;}
		
        virtual void __RPC_FAR *STDMETHODCALLTYPE Alloc( 
            /* [in] */ ULONG cb) 
		{
		  safearray tmp(VT,cb);
		  sa.attach(tmp.detach());
		  return sa.psa->pvData;
		};
        
        virtual void __RPC_FAR *STDMETHODCALLTYPE Realloc( 
            /* [in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb)
		{
			if(!pv) return Alloc(cb);
		    if(sa.psa->pvData==pv)
			{
               
			}
			return NULL;
		};
        
        virtual void STDMETHODCALLTYPE Free( 
            /* [in] */ void __RPC_FAR *pv) 
		{
		  sa.clear();
		};;
        
        virtual ULONG STDMETHODCALLTYPE GetSize( 
            /* [in] */ void __RPC_FAR *pv) 
		{return sa.total_count; };
        
        virtual int STDMETHODCALLTYPE DidAlloc( 
		void __RPC_FAR *pv) 
		{ return !((sa.psa)&&(sa.psa->pvData==pv));};
        
        virtual void STDMETHODCALLTYPE HeapMinimize( void) {};;	

};




