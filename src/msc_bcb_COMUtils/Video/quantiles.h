#pragma once
//#include "quantiles.h"

#include "filters3d.h"

template <int _MantissaDigit=14,class _FType=signed short,class _FTypeEx=signed int>
struct float_fix_t
{
	
	typedef _FType value_t;
	typedef _FTypeEx value_double_t;

	
	value_t v;

	enum    
	{
		mantissa_denom_dig = _MantissaDigit ,
		mantissa_denom =1<<mantissa_denom_dig
	};

static const value_double_t id=(value_double_t(1)<<mantissa_denom_dig)-1;
static const value_t id_v=(1<<mantissa_denom_dig)-1;


float_fix_t(){}
float_fix_t(double d){
   this->operator =(d);
}

inline float_fix_t&	operator=(double d)
{
	     v=d*double((((unsigned long long)1)<<mantissa_denom_dig)-1);
		 return *this;
}

inline float_fix_t&	operator=(const double d) const
{
	v=d*double((((unsigned long long)1)<<mantissa_denom_dig)-1);
	return *this;
}


inline operator double()
{
	 return double((unsigned long long)v)/double(((unsigned long long)1)<<mantissa_denom_dig);
}

inline bool operator<(float_fix_t& f)
{
	return v<f.v;
}

inline bool operator<(const float_fix_t& f)
{
	return v<f.v;
}

inline bool operator>(float_fix_t& f)
{
	return v>f.v;
}

inline bool operator>(const float_fix_t& f)
{
	return v>f.v;
}

inline bool operator<=(float_fix_t& f)
{
	return v<=f.v;
}

inline bool operator<=(const float_fix_t& f) const
{
	return v<=f.v;
}


inline bool operator>=(float_fix_t& f)
{
	return v>=f.v;
}


inline bool operator>=(const float_fix_t& f) const
{
	return v>=f.v;
}


inline bool operator ==(float_fix_t& f)
{
	return v==f.v;
}

inline bool operator ==(const float_fix_t& f) const
{
	return v==f.v;
}

	//blend(const float_fix_t a,const float_fix_t b=value_double_t(mantissa_denom)-a)	{	}


};

template <class T>
inline T& blend_add(T& v,const T& b,typename const T::value_double_t* pfake=0 )
{
	const T::value_double_t id=T::id;//(T::value_double_t(1)<<T::mantissa_denom_dig)-1;
    T::value_double_t bd=b.v;
    T::value_double_t ad=id-bd;
	 //ad=ad<<T::mantissa_denom_dig;

	T::value_double_t vd=(T::value_double_t(v.v)*bd)>>T::mantissa_denom_dig;
	vd+=ad;

	v.v=vd;//|1;
	//vd=vd*bd+ad;
	//vd=id-vd;
      
	//	vd=T::value_double_t(b.v)*(vd-id)+id;
	//v.v=(vd>>T::mantissa_denom_dig);
	return v;
		
}

template <class T>
inline T& blend(T& v,const T& b,typename const T::value_double_t* pfake=0 )
{
	
	T::value_double_t vd=T::value_double_t(v.v);//<<T::mantissa_denom_dig;
	vd=T::value_double_t(b.v)*vd;
	v.v=(vd>>T::mantissa_denom_dig);
	return v;

}


template <class T>
inline T& blend_add(T& v,const T& b,...)
{
  return v=b*(v-T(1))+T(1);
}


template <class T>
inline T& blend(T& v,const T& b,...)
{
	return v=b*v;
}

