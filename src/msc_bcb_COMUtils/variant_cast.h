#pragma once

#include <atlcomcli.h>





template<class PT> 
struct variant_cast
{

//typedef *PT T;
	VARIANT m_V;
    VARIANT* m_pV;

template<class S> 
 variant_cast(S &s)
 {
    m_pV= &(m_V=VARIANT());
	CVarTypeInfo<S> svti;
 }

//inline 

};
