#ifndef __CRAND__H
#define __CRAND__H
#include <windows.h>
#include <math.h>
#define DUINT UINT64



inline UINT32 pow2majorant(UINT32 c )
{
	if(c==0xFFFFFFFF) return 0; 
	if(!c) return 0;
	register int r=1,rc=c-1;
	while(rc)
	{
		r<<=1;
		rc>>=1;
	};
	return r;
}


struct  crand
{
  UINT32 value;
  UINT32 maxvalue;
  UINT32 mask;
  UINT32 a,b;
  long double b_maxvalue;
  crand(UINT32 _maxvalue, UINT32 bval=GetTickCount() )
  {
  maxvalue=_maxvalue;
  mask=maxvalue-1;
  value=bval;
  double M_PI_4 =atan(1.0);
   long double halfm=maxvalue;
       a=8*(DUINT)(halfm*M_PI_4/8.)+5;
       b=2*(DUINT)(halfm*(0.5-sqrtl(3.)/6.))+1;
  
  };

  inline UINT32 fast_get()
  {
	  value =a*value+b;
	  return value&=mask;
  }

  inline UINT32 get()
  {
    return  fast_get();
  }
  
};


#endif