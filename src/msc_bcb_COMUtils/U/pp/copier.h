#pragma once

template <typename T>
struct refref
{
   refref():m_pp(0){};

template <typename N>
   refref(N** pp,int offs=0){ m_pp=((T**)(pp))+offs;};

	inline T&   operator[](int n) 
	{
		return *(m_pp[n]);
	}
	T** m_pp;
};

template <typename P,bool freverse=false>
struct copier
{
	P& m_p;
	int m_count;
	copier(int count=0,P& p=P()):m_p(p),m_count(count){};

	template <typename _I>
	inline _I operator()(_I i)
	{
		if(freverse)
			for(int n=m_count-1;n>=0;n--)  *(i++)=m_p[n];
		else	for(int n=0;   n<m_count;n++)  *(i++)=m_p[n];
		return i; 
	}

};

template <typename P,typename _I>
_I copy_to_collection(int count,P& p,_I i,bool freverse=false)
{
	if(freverse) return (copier<P,true>(count,p))(i);
	else         return (copier<P,false>(count,p))(i);
};


