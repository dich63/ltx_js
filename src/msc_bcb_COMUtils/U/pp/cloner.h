#pragma once

template <typename T>
struct ClonerT
{

static inline void duplicate(T* d,T const * s)
{
	memcpy(d,s,sizeof(T)); 	
};

inline void clone_to(T* pd) const
{
  	T *  t= thisT();
	 t->duplicate(pd,t);
}

inline void close(){};

ClonerT(){};



ClonerT( ClonerT<T> const & cl)
 {
   	 cl.thisT()->clone_to(thisT());
 }
~ClonerT()
 {
         thisT()->close();
 }
T& operator=(ClonerT<T> & cl)
{
   T* t=thisT();
   t->close();
   cl.thisT()->clone_to(t);
   
 return *t;
}
protected:

inline T* thisT() const
{
	ClonerT* p= const_cast<ClonerT<T>*>(this);
	T* pT = static_cast<T*>(p);
	return pT;
}

};

