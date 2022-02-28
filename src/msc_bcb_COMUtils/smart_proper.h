#ifndef __SMART_PROPER__H
#define __SMART_PROPER__H

#include "smart_callers.h"
#include <algorithm>
#include <memory>
//
#define _SP_DUMP 
//
#ifdef _SP_DUMP 
#define SP_TRACE ATLTRACE
#else 
#define SP_TRACE
//(...) 
#endif




#define OVERRIDE_OP_EQU_EX(_Class,_BaseClass,arg,override_feature) \
	template <class A>\
	inline _Class& operator =(A& arg)\
 {\
 override_feature;\
 return  *this; }\
 template <class A>\
 inline _Class& operator =(A const& arg)\
 {\
 override_feature;\
  return  *this;  }

#define OVERRIDE_OP_EQU(_Class,_BaseClass) \
	OVERRIDE_OP_EQU_EX(_Class,_BaseClass,__arg_000137,*((_BaseClass*)this)=__arg_000137 )

/*
#define OVERRIDE_OP_EQU(_Class,_BaseClass,override_feature) \
	template <class A>\
	inline _Class& operator =(A& V)\
 {\
 override_feature;\
 *((_BaseClass*)this)=V;\
 return  *this; }\
 template <class A>\
 inline _Class& operator =(A const& V)\
 {\
 override_feature;\
 *((_BaseClass*)this)=V;\
 return  *this;  }
*/



template<class D>
class _Cast
{

public:
	D d;
	template<class T>
	inline  D& assign(T& a)
	{
		return d=D(a);
	}
	//template<>
	inline  D& assign(D& a)
	{
		return a;
	}

	template<class T>
	inline  T& type_cast(T&t, D& d)
	{

		return t=T(d);
	}
  operator D&()
  {
    return d;
  }
  operator const D&()
  {
	  return d;
  }
};

template<class V,typename Cast=_Cast<V> >
class type_adapter
{


public:
	
	V v;


	 type_adapter(){SP_TRACE(_T("TA costructor: %s\n"),CA2T(typeid( this ).name()));};

	~type_adapter()
	{SP_TRACE(_T("TA destructor: %s\n"),CA2T(typeid( this ).name()));};
/*
	type_adapter(type_adapter& a)
	{
		SP_TRACE(_T("TA  copy costructor: %s\n"),CA2T(typeid( this ).name()));
	}
	type_adapter(type_adapter const & a)
	{
		SP_TRACE(_T("TA  copy costructor: %s\n"),CA2T(typeid( this ).name()));
	} 
*/


	template<class A>
	 type_adapter(A& a)
	{
		v=Cast().assign(a);
	}

	template<class A>
	 type_adapter(A const& a)
	{
		v=Cast().assign(a);
	}
	operator V&()
	{
		return v;
	};

 
};




template<class T,class ValueType,class Cast=_Cast<ValueType> >
//template<class T,class ValueType>
class prop_adapter
{

protected:

	
public:
	

	T* pv;
	bool fdelayget;
	bool fprepare;
         
	prop_adapter(T* pt=NULL ,bool f=1):pv(pt),fdelayget(f)
	{
        //SP_TRACE(_T("default constructor ...\n"));
		SP_TRACE(_T("default constructor: %s\n"),CA2T(typeid( this ).name()));
	};
	//	/*
	prop_adapter(prop_adapter& cc):pv(cc.pv),fdelayget(0)
	{
		SP_TRACE(_T("copy constructor ...%s\n"),CA2T(typeid( this ).name()));
		cc.fdelayget=1;
	};
///*
	prop_adapter(prop_adapter& cc,T* pt):pv(pt),fdelayget(0)
	{
		SP_TRACE(_T("copy constructor ...%s\n"),CA2T(typeid( this ).name()));
		cc.fdelayget=1;
	};
//*/
	inline void init(T* p, bool fdg=1)
	{
		pv=p;
		fdelayget=fdg;
	}
	~prop_adapter()
	{
		SP_TRACE(_T("destructor (fdelayget=%d)  \n"),fdelayget);
		if(!fdelayget)   getVNoResult();
		
	}

	template <class A>
	inline operator A()
	{

		A a;
		return Cast().type_cast(a,getV());
	}

	
	inline operator  ValueType()
	{
        ValueType test;      
		return getV();
	}
	

	template <class A>
	inline prop_adapter& operator =(A& V)
	{
		return putV(Cast().assign(V));
	}

	template <class A>
	inline prop_adapter& operator =(A const& V)
	{
		return putV(Cast().assign(V));
	}
	//template <>
	inline prop_adapter& operator =(ValueType& V)
	{
		return putV(V);
	}
	//template <>
	inline prop_adapter& operator =(ValueType const& V)
	{
		return putV(V);
	}
	inline void  call()
	{
		pv->getV();
	};



	inline ValueType getV()
	{
		Cast cc;
		fdelayget=1;
		if(pv)  pv->get_prop(&(cc.d));
		return cc.d;
	};

	inline void getVNoResult()
	{
		SP_TRACE(_T("prop_adapter::getVNoResult\n"));
		if(pv)   pv->get_prop(NULL);
	};

	inline prop_adapter& putV(const ValueType& v)
	{
		fdelayget=1;
		if(pv)  pv->put_prop(&v);
		return *this;
	};
	void calls()
	{
		SP_TRACE(_T(" prop_adapter::calls...\n"));
	}
//*/
};

template<class T,class ValueType,class Cast=_Cast<ValueType> >
class prop_adapter_T: public T
{

protected:

	//prop_adapter():pv(NULL),fdelayget(0){};
public:
	//friend class T;


	bool fdelayget;
	
	//   	prop_adapter(T& v):pv(&v),fdelayget(1){};
	//

	prop_adapter_T():fdelayget(1)
	{
		SP_TRACE(_T("default constructor ...\n"));   
	};
	prop_adapter_T(prop_adapter_T& cc):T(cc),fdelayget(0)
	{
		SP_TRACE(_T("copy constructor ...\n"));
		//cc.fdelayget=1;
	};
	
	~prop_adapter_T()
	{
		SP_TRACE(_T("destructor fdelayget=%d { \n"),fdelayget);
		if(!fdelayget)   getVNoResult();
		SP_TRACE(_T("}destructor..\n"));
	}

	template <class A>
	inline operator A()
	{

		A a;
		return Cast().type_cast(a,getV());
	}

	// /*
	inline operator  ValueType()
	{

		return getV();
	}
	//*/

	template <class A>
	inline prop_adapter_T& operator =(A& V)
	{
		return putV(Cast().assign(V));
	}

	template <class A>
	inline prop_adapter_T& operator =(A const& V)
	{
		return putV(Cast().assign(V));
	}
	//template <>
	inline prop_adapter_T& operator =(ValueType& V)
	{
		return putV(V);
	}
	//template <>
	inline prop_adapter_T& operator =(ValueType const& V)
	{
		return putV(V);
	}
	inline void  call()
	{
		getV();
	};

	inline ValueType getV()
	{
		Cast cc;
		fdelayget=1;
		T::get_prop(&(cc.d));
		return cc.d;
	};

	inline void getVNoResult()
	{
		SP_TRACE(_T("prop_adapter::getVNoResult\n"));
		T::get_prop(NULL);
	};

	inline prop_adapter_T& putV(const ValueType& v)
	{
		fdelayget=1;
		T::put_prop(&v);
		return *this;
	};
	void calls()
	{
		SP_TRACE(_T(" prop_adapter::calls...\n"));
	}

};


template <typename A>
inline A remove(A& a)
{
	A t=a;
	a=0;
	return t;
}

#define FSTATE_CALL 1
#define FSTATE_PROP 2

template <class T,class V,class Cast=_Cast<V>, bool back_order=0
,class TypeAdapter=type_adapter<V,Cast> ,class A=std::allocator<TypeAdapter> >
class invoke_helper_base:
	      public prop_adapter<T,V>,
		  //public call_helper<invoke_helper_base<T,V>,TypeAdapter, invoke_helper_base<T,V,Cast,back_order,TypeAdapter,A> >  
		// 
		 public call_helper<invoke_helper_base<T,V>,TypeAdapter,prop_adapter<T,V> >  
		
	//	public call_helper<invoke_helper_base<T,V>,TypeAdapter,T> 


{
//
public:
	friend call_helper<invoke_helper_base<T,V>,TypeAdapter,T> ;
    typedef   call_helper<invoke_helper_base<T,V>,TypeAdapter,T> CALL_HELPER;
//
	typedef type_adapter<V> ArgType;
    typedef prop_adapter<T,V> PropType;
   //protected:

   
//    typedef type_adapter<V> RetType;
	unsigned fstate;  
  //
	ArgType* pres;
	ArgType* argv; 
	int   argc;
	T& RefT;
	char* ptt;
		//invoke_helper_base(T& rt):prop_adapter((T*)this),call_helper(*this),RefT(rt),argv(0),argc(0),pres(0){};
	    invoke_helper_base(T* prt):prop_adapter(prt),call_helper(*this),RefT(*prt),argv(0),argc(0),pres(0),fstate(0)
		{
          ptt="INIT";
         SP_TRACE(_T("this=[%p] T=[%p] constructor %s\n "),this,&RefT,(wchar_t*)CA2T(typeid( this ).name()));
		};
		~invoke_helper_base()
		{
           SP_TRACE(_T("this=[%p] T=[%p] destructor %s pres=[%p]\n "),this,&RefT,(wchar_t*)CA2T(typeid( this ).name()),pres);
	       reset_state();	   
		}
//*
		//invoke_helper_base(invoke_helper_base& ih ):prop_adapter(ih),call_helper(*this),RefT(ih.RefT)
	   invoke_helper_base( invoke_helper_base& ih ):prop_adapter(0,0) ,call_helper(*this),RefT(*((T*)this))
	  {
		    prop_adapter::init((T*)this,ih.fdelayget);
             //
			 pres=remove(ih.pres);
			 argv=remove(ih.argv);
			 argc=remove(ih.argc);
			 fstate=remove(ih.fstate);
			 ih.ptt="CLEANed";
			 ptt="COPYed!!!";
			 
	  };
//*/
    OVERRIDE_OP_EQU(invoke_helper_base,prop_adapter);





/*        
invoke_helper_base& operator =(invoke_helper_base& ih)
{
	pres=remove(ih.pres);
	argv=remove(ih.argv);
	argc=remove(ih.argc);

	return *this;
}
*/

void inline reset_state()
{
	if(pres){
		//for(int i=0;i<=argc;i++)  A().destroy(pres+i);
		for(ArgType* p=pres;p!=pres+(argc+1);p++)  A().destroy(p);
		A().deallocate(pres,argc+1);
	}
	remove(pres);
	remove(argv);
	remove(argc);
	remove(fstate);

};


//
inline 	T& prepare_invoke(long  _argc,ArgType* _argv)
//inline 	T prepare_invoke(long  _argc,ArgType* _argv)
	{ 
       //ArgType* aa=(ArgType*)_argv;
		argv=(pres=A().allocate((argc=_argc)+1))+1;
     // ArgType aaa;
	 // aaa=ArgType();
	  //(*pres).vt=0;
		*pres=ArgType();
        if(back_order) 
			std::reverse_copy(_argv,_argv+_argc,argv);  
		else 
			std::copy(_argv,_argv+_argc,argv);  


		//argv=(ArgType*)_argv; 
		;

#ifdef _SP_DUMP
		SP_TRACE(_T(" %s\n "),CA2T(typeid( this ).name()));
		SP_TRACE(_T("prepare_Invoke : argc=%d argv[%p]={ "),argc,argv);

	//	for(int i=0;i<argc;i++)	SP_TRACE(_T("%s "),(wchar_t*)(_bstr_t)(_argv[i]));

		SP_TRACE(_T("}\n"));
#endif
		//return *(prop_adapter::pv);
		fstate|=1;
		//fdelayget=false;
		/*
         T res=*((T*)this);  
		 res.fstate|=1;
		 res.fdelayget=false;
		 
		return res;
		*/
		return *((T*)this);
	}

};




#endif