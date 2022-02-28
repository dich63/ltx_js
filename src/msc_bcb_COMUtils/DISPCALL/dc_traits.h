#pragma once

#include "tls_dll.h"
#include "tls_dllex.h"
#include "call_helper.h"
#include <memory>

enum cheh {srv,dbg,tss,except};
#define DFLT_CHEH tss
//#define DFLT_CHEH except
DECL_TYPE_SELECTOR_PRFX(TS,error_handler)

#define   ERROR_HANDLER_SELECTOR  TYPE_SELECT_IF(TS,error_handler)

//error_handler::check_error(hr).success();

#define CHECK_CEH(a) (error_handler::check_error().success()\
	&&error_handler::check_error(a).success())

struct call_helper_error_handler_base
{
   
struct error_info
{
long scode;
union{
  const	wchar_t* description;
  const	wchar_t* message;
  };
  const	wchar_t* helpfile;
  unsigned  long helpcontext;
  const	wchar_t* source;

  inline bool success(){ return scode>=0;}
  inline bool fail(){ return scode<0;}
  error_info(long hr=0,wchar_t* d=NULL,wchar_t* s=NULL):scode(hr),description(d),source(s)
	  ,helpcontext(0),helpfile(NULL){};

};

};


template <typename innerType,typename ArgInHolder,int mode=cheh::srv>
struct call_helper_error_handler:call_helper_error_handler_base
{
typedef   typename   RefCount_fake<0>	refcounter;

static error_info&	get_error_info()
{
   static static error_info ei=error_info();
   return ei;
}

static error_info&   check_error(long hr)
   {
	    get_error_info().scode=hr;
        return get_error_info();
   }
static  error_info&  check_error()
{
	 return get_error_info();
}


};

#define  DO_IF_NO_ERROR if(error_handler.check_error().success()) 


template <typename V,typename Holder=V>
struct  argintype_converter_base
{
	
	typedef typename Holder VHolder;
	
	inline operator VHolder*()
	{
		return m_pholder;
	}
protected:   
	VHolder* m_pholder;
	inline VHolder*& get_holder()
	{
		return m_pholder;
	};
};


template <typename V,typename Holder=V,class Allocator=std::allocator<void> 
,class ErrorHandler=call_helper_error_handler<V,Holder,cheh::DFLT_CHEH> >
struct argin_converter: public argintype_converter_base<V,Holder>
{
    typedef   typename  Allocator     allocator;
	typedef typename  ERROR_HANDLER_SELECTOR<VHolder,ErrorHandler>::result error_handler;
	

	template <typename T>
	argin_converter( T const& val)
	{
		get_holder()=  ::new( allocate<VHolder,allocator>())  VHolder(val);
	}
     
};


template <typename V,class Converter=argin_converter<V>,class Allocator=Converter::allocator
,class ErrorHandler=call_helper_error_handler<V,Converter::VHolder,cheh::DFLT_CHEH> >
struct ArgIn
{
	
	typedef   typename  Converter::VHolder     VHolder; 
	typedef   typename  Allocator     allocator; 
    typedef   typename  a_free<VHolder,allocator>     deleter; 
	

	typedef typename  ERROR_HANDLER_SELECTOR<Converter,ErrorHandler>::result error_handler;
	typedef   typename      error_handler::refcounter  error_refcounter;

	struct ArgInHolder
	{
		typedef   ArgInHolder HolderType;
		//flipflop_ptr<VHolder,deleter,flipflop_interceptor_rc<VHolder,deleter> > m_vh;
		flipflop_ptr_rc<VHolder,deleter,error_refcounter> m_vh;
		ArgInHolder const & reset(VHolder* pvh=NULL)
		{
			m_vh.reset(pvh);
			return *this;  
		}
		operator V const& ()
		{
			VHolder* vh=m_vh;
			return (*vh);
		}
	};

	typedef   ArgInHolder HolderType;

	//typedef typename  call_helper_error_handler<VHolder,HolderType>   error_handler;

	HolderType m_holder;
	operator HolderType const &()
	{
		return m_holder;
	}
//?//
	ArgIn(){};
/*
    ArgIn(HolderType const& h )
	{
         m_holder=h;
	};
*/

template <typename T>
inline void inner_converter(T const& ,HolderType val)
{
  m_holder=val;
};
template <typename T>
inline void inner_converter(T const& val,...)
{
	m_holder.reset(Converter(val));
};


	template <typename T>
	ArgIn( T const& val)
	{
		_ctdbg("costructor\n");

        inner_converter(val,val);

		//if(is_convertible<T,HolderType>::result)
					//m_holder.reset(Converter(val));
		
	};

	template <typename T>
	ArgIn( T & val)
	{
		_ctdbg("costructor\n");
      inner_converter(val,val);
		/*if(is_convertible<T,HolderType>::result)
		{
			m_holder=HolderType(val);
		}
		else m_holder.reset(Converter(val));
		//m_holder.reset(Converter(val));
		*/

	};

	~ArgIn() 
	{
		_ctdbg("destructor\n");//throw 1;// newer ...!!!
	};


};

template <class ArgType,class  Allocator=ArgType::allocator> 
struct collector
{
	typedef  typename ArgType::HolderType HolderType;
	typedef  typename ArgType::VHolder VHolder;
	typedef   typename  Allocator     allocator; 

	typedef  std::vector<HolderType,allocator> vector_Type;
	//
	typedef   typename vector_Type::iterator iterator;
	//typedef   HolderType* iterator;
	vector_Type m_vect;
	collector(){};
	collector(int argc,ArgType *pargv)
	{
		if(argc>0)
		{
		m_vect.resize(argc);
		std::copy(pargv,pargv+argc,m_vect.begin()); 
		}
	};

	inline size_t size()
	{
		return m_vect.size();
	}
	inline  iterator begin()
	{
		return m_vect.begin();//&m_vect[0];
	}
	inline   iterator end()
	{
		return m_vect.end();//begin()+size();
	};
	inline void clear()
	{
		m_vect.clear();
	}

};


template <class _ArgType,class Collector= collector<_ArgType>,class  Allocator=Collector::allocator,
class ErrorHandler=call_helper_error_handler<_ArgType,_ArgType,cheh::DFLT_CHEH> >
struct collector_delegator
{
	typedef  typename _ArgType    ArgType;
	typedef  typename ArgType::HolderType HolderType;
	typedef  typename Collector*  CollectorPtr;
	typedef  typename ArgType::VHolder VHolder;
     typedef   typename  Allocator     allocator; 
    typedef typename  ERROR_HANDLER_SELECTOR<_ArgType,ErrorHandler>::result error_handler;
	typedef   typename      error_handler::refcounter  error_refcounter;

	flipflop_ptr_rc< Collector,a_free<Collector,allocator>,error_refcounter> m_pcoll;


	collector_delegator(int argc,ArgType *pargv=NULL)
	{
		m_pcoll=::new( allocate<Collector,allocator>())  Collector(argc,pargv);
	}
	collector_delegator(){};
	inline CollectorPtr operator ->()
	{
		return   m_pcoll;
	}
	inline bool operator !()
	{
		return   !m_pcoll;
	}
	inline void clear()
	{
		m_pcoll.reset();
	}
};


template <typename ValueType,typename EN_TYPE=long>
struct ValueTypeConverter_base
{
  typedef EN_TYPE HRES_TYPE;
	HRES_TYPE m_hr;
ValueTypeConverter_base()
{
	m_hr=HRES_TYPE();
}
 operator HRES_TYPE()
 {
	 return m_hr;
 }
};






template <class CollectorDelegator,class PropID=char*>
struct NullDriver
{


typedef typename   CollectorDelegator::error_handler error_handler;

typedef typename  CollectorDelegator::VHolder ValueType;	
typedef typename long FlagType;	
typedef typename PropID prop_id_type;	



member_if<FlagType,false> m_flags;

struct sValueGetTypeConverter: ValueTypeConverter_base<ValueType,long>
	{       
      typedef typename ValueType ValueGetType;  

template <typename A>       
      HRES_TYPE operator()(ValueGetType* pval,A* pa)
	  {
		  *pa=A(*pval);
		  return m_hr;
	  };
HRES_TYPE operator()(ValueGetType* pval,NullDriver<CollectorDelegator>* pa)
{
       /// ------------
       *pa=NullDriver<CollectorDelegator>();
	  return m_hr;
}

  	};
struct sValuePutTypeConverter: ValueTypeConverter_base<ValueType,long>
{       
	typedef typename ValueType ValuePutType;  
	template <typename A>       
	HRES_TYPE operator()(A* pa,ValuePutType* pval)
	{
		*pval=ValuePutType(*pa);
		return m_hr;
	};


};

typedef sValueGetTypeConverter ValueGetTypeConverter;
typedef sValuePutTypeConverter ValuePutTypeConverter;

typedef typename ValueGetTypeConverter::ValueGetType ValueGetType;
typedef typename ValuePutTypeConverter::ValuePutType ValuePutType;

	inline long do_get_no_result(CollectorDelegator cd,long flag)
	{
		if(!cd) return 0;
		return do_get(cd,flag);
	};
	long do_get(CollectorDelegator cd,long flag,ValueGetType* pv=NULL)
	{

	return 0;
	}

	long do_put(CollectorDelegator cd,long flag,ValuePutType* v)
	{
	return 0;
	};

	long   get_prop_driver(prop_id_type name,NullDriver* pd)
	{
		*pd=NullDriver<CollectorDelegator>();
		return 0;
	};
 long	get_driver_from_getval(ValueType* val,NullDriver* pd)
	{
		return 0;
	}

};


//template <class RetType,class ArgType,class CollectorDelegator=collector_delegator<ArgType>,
//class _Driver=NullDriver<CollectorDelegator > >
/*
template <class _RetType,class _ArgType,class CollectorDelegator=collector_delegator<_ArgType>
,class _Driver =NullDriver<CollectorDelegator> >
struct invoker_base
{
   typedef typename _Driver Driver;
   typedef typename _ArgType ArgType;
   Driver m_driver;
    
	inline	_RetType  operator()(int argc,ArgType* pargv)
	{
       CollectorDelegator    cd(argc,pargv);

		return _RetType(cd,get_driver());
	}

 virtual  Driver& get_driver()
 {
  return m_driver;
 }

};
*/
template <class RetType,class _ArgType>
struct invoker_base_1
{
	//typedef typename _Driver Driver;
	typedef typename _ArgType ArgType;

	inline	RetType  operator()(int argc,ArgType* pargv)
	{
		collector_delegator<ArgType>    cd(argc,pargv);

		return RetType(cd);
	}
	/*
	virtual  Driver get_driver()
	{
	return Driver();
	}
	*/
};




template <class CollectorDelegator,class _Driver=NullDriver<CollectorDelegator>
,typename ValueGetTypeConverter=_Driver::ValueGetTypeConverter
,typename ValuePutTypeConverter=_Driver::ValuePutTypeConverter
,typename DriverFlagType=_Driver::FlagType,class  Allocator=CollectorDelegator::allocator>
struct RetType_base
{
  friend class	caller_friend_base;

  typedef typename  ValueGetTypeConverter::ValueGetType  ValGetType;
  typedef typename  ValuePutTypeConverter::ValuePutType  ValPutType;
  typedef typename   CollectorDelegator::VHolder  VHolder;
  typedef typename   CollectorDelegator::HolderType  HolderType;
  typedef typename   CollectorDelegator::ArgType  ArgType;

  typedef   typename  Allocator     allocator;   
  typedef   typename  _Driver       Driver; 

  typedef typename   CollectorDelegator::error_handler error_handler;
  typedef   typename      error_handler::refcounter  error_refcounter;
  
inline bool  check() const
{
  return error_handler::check_error().success();
}
inline bool  check(long hr) const
{
	return error_handler::check_error(hr).success();
}

enum ops
{   fempty=0,
	froot=1 };

  struct RHolder
  {

    DriverFlagType m_flags; 
	CollectorDelegator m_cd;
    Driver m_driver;
    ValGetType m_getval;
    //ValPutType m_putval;
	unsigned long m_options;
	RHolder():m_options(0){};
	RHolder(CollectorDelegator const& cd,Driver const& d,unsigned long options=false)
	{	
		m_options=options;
		m_cd=cd;
		m_driver=d;
		m_flags=m_driver.m_flags;
	};   

    ~RHolder()
	{
		if( (!!m_cd)&&check())  
			check(m_driver.do_get_no_result(m_cd,m_flags));
	}

	inline long do_get()
	{
		return m_driver.do_get(m_cd,m_flags,&m_getval);
	};

	inline  bool  check() 
	{
		return error_handler::check_error().success();
	}
	inline  bool  check(long hr) 
	{
		return error_handler::check_error(hr).success();
	}
    inline  long   get_driver_from_getval(Driver* pdr)
	{
		return m_driver.get_driver_from_getval(&m_getval,pdr);
	} 
  };


  template <class N>
  struct cast_pp
  {
      N m_n;  
	  //
	  //RetType_base &const m_da;
	  //
	  RetType_base* m_pda;
	  //	  cast_pp(RetType_base& const da):m_da(da){};
	  //
	  cast_pp(RetType_base* pda):m_pda(pda){};
	  //cast_pp(cast_pp& const cc):m_da(cc.m_da){};

      inline RetType_base& da()
	  {
         return *m_pda;
	  }
///*




   template <class A> 
	  inline operator A() 
	  {
		  
		  //m_n=m_da;
          def(m_n);
		  return  A(m_n);
	  }
	  

//*/
  inline operator N()
	  {
 
		  return *m_pda ;
	  }

  inline operator ArgType() const
  {
	  //return ArgType(N(*m_pda));
	  //
	  return ArgType(N());
  }

  inline operator HolderType() const
  {
	  return HolderType(ArgType(N(*m_pda)));
  }

  inline  HolderType in()
  {
	  return HolderType(ArgType(N(*m_pda)));
  }

//  /*
inline operator HolderType() 
{
	return HolderType(ArgType(N(*m_pda)));
}
//*/

	  inline N def(N nd)
	  {
       
		 // m_r.Clear();
		  //
		   error_handler::error_info& ei=error_handler::check_error();
			   ei.set_defflag();
		  if(ei.success()&&da().do_get()>=0)
		  {
			  try
			  {
				  HRESULT hr=ValueGetTypeConverter()(&(da().m_rh->m_getval),&m_n);
				  if(error_handler::check_error().success())
    				  return m_n;
	    			  else ei.set_defflag();

			  }
			  catch (...){}
		  }
		 
		  return nd;

	  };
	  inline cast_pp& operator =(N n)
	  {
           *m_pda=n;
		  return *this;
	  };

  };


//typedef   typename      error_handler::refcounter  error_refcounter;
//flipflop_ptr< Collector,a_free<Collector,allocator> > m_pcoll;
 flipflop_ptr_rc<RHolder,a_free<RHolder,allocator>,error_refcounter> m_rh;



	// ResultHolder;
	
	RetType_base(CollectorDelegator cd,Driver driver=Driver(),unsigned long options=0)
	{
		m_rh= ::new ( allocate<RHolder,allocator>() ) RHolder(cd,driver,options);
		/*
		m_rh->m_flags=driver.m_flags;
		m_rh->m_cd=cd;
		m_rh->m_driver=driver;
		*/
	};
	~RetType_base()
	{
		
	}

inline long do_get() 
{
  return m_rh->do_get();
}

template <typename A>       
inline operator A()
{
   A a=A();
  //ValueGetTypeConverter v=ValueGetTypeConverter();
if(check())
{
  if(check(do_get()))
   check(ValueGetTypeConverter()(&(m_rh->m_getval),&a));
}
   return a;   
}

inline operator HolderType() const
{
   
	if(check())
	{
		check( const_cast<RetType_base*>(this)->do_get());
			//h.reset(const_cast<RetType_base*>(this)->m_rh->m_getval);
	}
  ArgType argin(const_cast<RetType_base*>(this)->m_rh->m_getval);
  return argin;
}


template <class A>
inline RetType_base& operator =(A a)
{
if(check())
{
  ValPutType putval;

   if(check(ValuePutTypeConverter()(&a,&putval)))
   check(m_rh->m_driver.do_put(m_rh->m_cd,m_rh->m_flags,&putval));
}
   	return *this;
};

inline RetType_base& mask(DriverFlagType mask)
{
  m_rh->m_flags&=(~mask);
  return *this;
}

template <class N>
inline cast_pp<N> as()
{
 return cast_pp<N>(this);
}
template <class N>
inline cast_pp<N> as() const
{
	return cast_pp<N>(this);
}

template <class N>
inline N def(N dv) 
{
	return cast_pp<N>(this).def(dv);
}


protected:
	//
   RetType_base(){};
	//RetType_base(RetType_base const & s)	{     m_rh=s.m_rh;	};

};

template <typename CollectorDelegator,class _Driver=NullDriver<CollectorDelegator>
,typename ValueGetTypeConverter=_Driver::ValueGetTypeConverter
,typename ValuePutTypeConverter=_Driver::ValuePutTypeConverter
,typename DriverFlagType=_Driver::FlagType,class  Allocator=CollectorDelegator::allocator,
typename ArgType=CollectorDelegator::ArgType>
struct RetType: 
public tss_wild_caller_base < RetType<CollectorDelegator,_Driver,
ValueGetTypeConverter,ValuePutTypeConverter,DriverFlagType,Allocator,ArgType>
,ArgType>, 
//invoker_base<RetType<CollectorDelegator,_Driver,
//ValueGetTypeConverter,ValuePutTypeConverter,DriverFlagType,Allocator,ArgType>, ArgType,
 //CollectorDelegator,_Driver> >,
public RetType_base<CollectorDelegator,_Driver,
ValueGetTypeConverter,ValuePutTypeConverter,
DriverFlagType,Allocator>
{
    typedef typename CollectorDelegator::ArgType ArgType;
	typedef typename _Driver::prop_id_type prop_id_type;


OVERRIDE_OP_EQU(RetType,RetType_base);

//
/*

	typedef typename invoker_base<RetType<CollectorDelegator,_Driver,
		ValueGetTypeConverter,ValuePutTypeConverter,DriverFlagType,Allocator,ArgType>, ArgType,
		CollectorDelegator,_Driver>    invoke_type;


    
	struct invoke_invoke : invoke_type
	{
        RHolder* prt;
		virtual  Driver& get_driver()
		{
            prt->do_get();
           
           ValueGetTypeConverter()(&(prt->m_getval),&m_driver);
           	return m_driver;
		}
	};


       invoke_invoke m_invoke_invoke;
//*/


 inline  Driver get_prop_driver(prop_id_type idname)
	   {
		   Driver dp;
		   if(check())
		   {
			   //if(check(do_get()))
                  Driver d=get_driver_from_getval();
				   check(d.get_prop_driver(idname,&dp));
		   }
		   return dp;
	   }

 inline  Driver get_driver_from_getval()
 {
	 Driver driver;
	 
	 if(check())
	 {
		 if(check(do_get()))
			 check( m_rh->get_driver_from_getval(&driver));
			// check(ValueGetTypeConverter()(&(m_rh->m_getval),&driver));

	 }
	 return driver;
 }

inline RetType	pp(   prop_id_type idname )
{
   return prop(idname);
}
inline RetType operator[](prop_id_type idname)
{
  return prop(idname);
}

#ifdef   _MSC_VER
// so VS 2008 is  sranii bagodrom:
inline RetType operator[]( int  idname)
{
	return (*this)[prop_id_type(idname)];
};
#endif


inline RetType	prop(   prop_id_type idname )
{
 	  Driver dp=get_prop_driver(idname);
      return RetType(CollectorDelegator(0),dp,ops::froot);
}

	   virtual RetType inner_invoke(size_t argc,ArgType* plist)
	   {
		   CollectorDelegator cd(argc,plist);
		    if(m_rh->m_options&ops::froot)
		   {
			   m_rh->m_cd.clear();
               return RetType(cd,m_rh->m_driver);
		   }
		   else
		   {
	         Driver driver= get_driver_from_getval();
             return RetType(cd,driver);
		   };

		  // ValueGetTypeConverter()(&(prt->m_getval),&m_driver);
		   //return (m_invoke_invoke)(argc, plist);
	   }

       RetType()
	   {
		  // m_invoke_invoke.prt=this; 
	   };

	   RetType(Driver driver,CollectorDelegator cd=CollectorDelegator(),unsigned long options=0)
		   :RetType_base(cd,driver,options)
	   {		 
	   };
	   RetType(CollectorDelegator cd,Driver driver=Driver(),unsigned long options=0)
		   :RetType_base(cd,driver,options)
	   {  
	   };
	  //	  ,tss_wild_caller((invoke_type*)&m_invoke_invoke){};
};

