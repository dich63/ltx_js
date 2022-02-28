#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <strstream>
#include <Shlwapi.h>

#define STRUCT_SET_DFLT(a,s,field,dv) s.field=a[char_mutator<>(#field)].def(dv)
#define PSTRUCT_SET_DFLT(a,p,field,dv) STRUCT_SET_DFLT(a,(*p),field,dv)

#define STRUCT_SET(a,s,field)  STRUCT_SET_DFLT(a,s,field,s.field)
#define PSTRUCT_SET(p,s,field) PSTRUCT_SET_DFLT(p,s,field,s->field)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


// WILDCARD SEARCH FUNCTIONS

// Returns:1 on match, 0 on no match.
//
inline    char* safe_strstr(const char* pd,const char* ps)
{

	if((!pd)||(!ps)) return 0;
	return (char*)strstr(pd,ps);
}
inline    wchar_t* safe_strstr(const wchar_t* pd,const wchar_t* ps)
{

	if((!pd)||(!ps)) return 0;
	return (wchar_t*)wcsstr(pd,ps);
}

inline    char* safe_strstri(const char* pd,const char* ps)
{

	if((!pd)||(!ps)) return 0;
	return (char*)StrStrIA(pd,ps);
}
inline    wchar_t* safe_strstri(const wchar_t* pd,const wchar_t* ps)
{

	if((!pd)||(!ps)) return 0;
	return (wchar_t*)StrStrIW(pd,ps);
}


inline  const   char* safe_strchr(const char* pd,int c)
{
	if(pd)
	{
		return strchr(pd,c);
	}
	return 0;
}


template <class CH>
bool wildcmp( CH *string,CH *wild) {

	if(!wild) return true;
    CH *cp, *mp;
        while ((*string) && (*wild != '*')) {
            if ((*wild != *string) && (*wild != '?')) {
            return false;
        }

        wild++;
        string++;
    }

while (*string) {
            if (*wild == '*') {
                if (!*++wild) {
                return true;
            }

            mp = wild;
            cp = string+1;
            } else if ((*wild == *string) || (*wild == '?')) {

            wild++;
            string++;
            } else {

            wild = mp;
            string = cp++;
        }

    }

while (*wild == '*') {
        wild++;
    }

return !*wild;
}


template <class CH>
bool wildcmp( CH *string,CH **ppwilds)
{
	if(!ppwilds) return true;

	while((*ppwilds)||(**ppwilds))
	{
       if(wildcmp(string,*ppwilds)) return true;  
	}
	return false;
}



template <class T, class T2>
T make_detach(T& dst, T2 n)
{  
	T t=dst;
	dst=(T)n;
	return t;
};

template <class T>
T make_detach(T& dst)
{  
	T t=dst;
	dst=T();
	return t;
};

inline bool make_detach(bool dst)
{
	return make_detach(dst,false);
}


template <class T>
T interlocked_detach(T& dst, T n=T())
{  
	
	T t=(T)InterlockedExchange((LONG volatile*)&dst,LONG(n));
	return t;
};

template <typename CH>
struct dflt_v
{

inline	static FILE* nulFile()
	{
         static  FILE* sf=fopen("nul","wb");
		 return sf;
	}

	static const CH* digits(bool fupper=false)
	{
		return (!fupper)? "0123456789abcdefx,.[];\"\'":"0123456789ABCDEFX,.[];\"\'";
	};

	 static CH* cat(CH*d,const CH*s)
	{
		return strcat(d,s);
	}


	enum{
		equ='=' 
	};




	static  int vfmt_count(const char* fmt,va_list argptr)
	{
		//
		int cb=_vscprintf(fmt,argptr);
		//int cb=vfprintf(dflt_v<char>::nulFile(),fmt,argptr);
		return cb;
	}
	static  int vs_printf(char* buf,const char* fmt,va_list argptr)
	{
		return vsprintf(buf, fmt, argptr);
	}




	static	const char*	 empty()
	{
		return "";
	}
};
template <>
struct dflt_v<wchar_t>
{


	enum{
		equ=L'=' 
	};

 static	wchar_t* cat(wchar_t*d,const wchar_t*s)
	{
		return wcscat(d,s);
	}

	static  int vfmt_count(const wchar_t* fmt,va_list argptr)
	{
		//
		int cb=_vscwprintf(fmt,argptr);
		//int cb=vfwprintf(dflt_v<char>::nulFile(),fmt,argptr);
		return cb;
	}
	static  int vs_printf(wchar_t* buf,const wchar_t* fmt,va_list argptr)
	{
		return vswprintf(buf, fmt, argptr);
	}



	static const wchar_t* digits(bool fupper=false){

		return (!fupper)? L"0123456789abcdefx,.[];\"\'":L"0123456789ABCDEFX,.[];\"\'";
	};

	static	const wchar_t*	 empty()
	{
		return L"";
	}
};


inline int safe_len(const char* p,bool fwith_last_zero=false)
{	
	int cadd=(fwith_last_zero)?1:0;
	return (p)?strlen(p)+cadd:0;
}

template <class CH>
inline    CH* safe_copy(CH* pd,const CH* ps,int cb,bool fwith_last_zero=true)
{
	if(cb)
	{
		if(fwith_last_zero) ++cb;
		//memcpy(pd,ps,(++cb)*sizeof(CH));
		memcpy(pd,ps,(cb)*sizeof(CH));
	}
	return pd+cb;
}

template <class T,class Allocator=std::allocator<T> >
struct v_buf
{
	enum{
     tail=2
	};
	typedef typename T value_t;
	typedef typename T value_type;
	std::vector<T,Allocator> v;
	inline bool empty()
	{
		return v.size()<=tail;
	}
	
	v_buf():v(tail){};
   v_buf(size_t cb)
   {
	   
	   v.resize(cb+tail);
   }

   v_buf(const T* b,const T* e)
   {
       cat(b,e); 
   }



inline v_buf& operator =(const T* p)
{
	return clear().cat(p);
}
inline v_buf& resize(int cb)
{
         v.resize(cb+tail);
		 return *this;
}
inline size_t size_b()
{
	return count()*sizeof(T);
}

inline size_t count()
{
	int c=v.size();
	return (c>tail)?(c-tail):0;
}

inline T& operator[](int n)
{
	return v[n];
}
inline 	  T* get()
{
     return (!empty())? &v[0]:0;
}
inline 	 operator T*()
{
	return get();
};
 inline v_buf& clear(){
     if(!v.empty())
	 {
		   v.clear();
		   v.resize(tail);
	 }

	return *this;
 }

v_buf& vprintf(const T * fmt,va_list argptr)
{
	int cb=dflt_v<T>::vfmt_count(fmt,argptr);
	resize(cb);
     dflt_v<T>::vs_printf(get(),fmt,argptr);
	//vswprintf_s(buffer,4096, fmt, argptr);
	va_end(argptr);
	return *this;
}
v_buf& printf(const T * fmt,...)
{
	va_list argptr;
	va_start(argptr, fmt);
	return vprintf(fmt,argptr);
}


v_buf& cat(const T* b,const T* e)
{
	int cb;
	T* pt;
	if(e==0) return cat(b);
	if(b<e)
	{
		int cb0=count();
		//if(cb0>0) cb0--;
		cb=cb0+std::distance(b,e);
		resize(cb);
		 pt=get();
		// 
		T* pd=pt+cb0;
		for(const T* p=b;p<e;p++,pd++)
		{*pd=*p;}
		   // *pd=0;
	}
  return *this;
}

v_buf& cat(const T* str)
{

    int cb=safe_len(str);
	if(cb){
    cb+=count()+1;
	resize(cb);
	dflt_v<T>::cat(get(),str);
	}
	return *this;
}

v_buf& operator<<(const T* str)
{
	return cat(str);
}

/*
v_buf& operator<<(const v_buf& v)
{
	return cat(v.get());
}
*/

v_buf& undecorate(const T* str,const T bra=dflt_v<T>::digits()[22],const T ket=dflt_v<T>::digits()[22])
{
    int cb=safe_len(str);
	if(cb)
	{
		int nl=(str[0]==bra)?1:0;
        int nr=(str[cb-1]==ket)?cb-1:cb;
		cb=nr-nl;
		resize(cb);
		T*p=safe_copy(get(),str+nl,cb,false);	
	}
	return *this;
}
};

template <class T,int N=1024,class Allocator=std::allocator<T> >
struct v_buf_fix:v_buf<T,Allocator>
{
	v_buf_fix(int sz=N):v_buf<T>(sz){};

	inline T& operator[](int n)
	{
		return v[n];
	}
	inline 	 operator T*()
	{
			return get();
	};
	
};



template <class T,int N=256>
struct bufT
{
	typedef typename T value_t;
	typedef typename T value_type;
	enum
	{
		count=N,
		size=N*sizeof(T)
	};
	T p[N];
	inline 	 operator T*()
	{
		return p;
	};

	inline 	  T* get()
	{
		return p;
	}

	bufT(T t=T())
	{
		p[0]=t;   
	};

	bufT& printf(const T * fmt,...)
	{
		va_list argptr;
		va_start(argptr, fmt);
		dflt_v<T>::vs_printf(p,fmt,argptr);
		return *this;
	}

};

typedef  bufT<char> charbuf;


template <class T>
struct dummy:bufT<T,1>
{
dummy(T t=T()):bufT<T,1>(t){};

inline T* operator&()
{
	return get();
}

};


template <class CH>
inline    int safe_len_zerozero(const CH* pb,bool fwith_last_zero=false)
{

	if(!pb) return 0;
	const CH* p=pb;
	while(*p){p+=safe_len(p)+1;};
	 int cb=(int(p)-int(pb))/sizeof(CH);
	 return (fwith_last_zero)?cb+1:cb;
};


template <class CH>
v_buf<CH>& copy_zz( CH* ps,   v_buf<CH>&buf=v_buf<CH>())
{
	buf.v.clear();
	int sz=safe_len_zerozero(ps);
	buf.v.resize(sz+2);
    safe_copy(buf.get(),ps,sz); 
	return buf;
}

template <class CH>
v_buf<CH>& add_zz( CH* ps,CH* ps2,   v_buf<CH>&buf=v_buf<CH>())
{
	buf.v.clear();
	int sz=safe_len_zerozero(ps);
	int sz2=safe_len_zerozero(ps2);
	buf.v.resize(sz2+sz+2);
	safe_copy(buf.get(),ps,sz,false);
	safe_copy(buf.get()+sz,ps2,sz2,true);
	return buf;
}



template <class CH>
v_buf<CH>& copy_z(const CH* ps,   v_buf<CH>&buf=v_buf<CH>(),int sz=-1,bool fwith_last_zero=false)
{
	buf.v.clear();
	if(sz==-1) sz=safe_len(ps,fwith_last_zero);
	buf.v.resize(sz+1);
	safe_copy(buf.get(),ps,sz,fwith_last_zero); 
	return buf;
}

inline    int safe_cmpni(const char* pd,const char* ps,int maxcb)
{
	if(pd==ps) return 0;
	if(pd==0) return -1;
	if(ps==0) return 1;
	return strnicmp(pd,ps,maxcb);
}


static inline bool is_space(char c)
{
	return  iswspace( static_cast<unsigned short>(c));
}
static inline bool is_space(wchar_t c)
{
	return  isspace( static_cast<unsigned short>(c));
}

template<class CH>
static  CH*  _ltrim(const CH* p) 
{
	if(p)  while(is_space((*p)))p++;  
	return (CH*)p;
};


template <class CH>
inline std::vector< CH*>& args_ZZs(const CH* delim,const CH* pzz,v_buf<CH>& buf=v_buf<CH>(),std::vector<CH*>& psv=std::vector<CH*>(),int cinc=8)
{
    
	int l,sz,sd,lp;	
	CH* p,*ps;

	   sd=safe_len(delim);
	   sz=safe_len(pzz);

	  if(!(sz)) return psv;

      ps=copy_z(pzz,buf,sz).get();
	    p=ps+sz-sd;
	  if((sz>sd)&&(p!=safe_strstr(p,delim)))
		   ps=buf.cat(delim);

	  


	
	while(p=safe_strstr(ps,delim))
	{
		//p=_ltrim(p);

		int n=psv.capacity();
		if((n%cinc)==0) {
			cinc*=2; 
			psv.reserve(n+cinc);
		};
		*p='\0';
		psv.push_back((CH*)_ltrim(ps));
		
		ps=p+sd;

	}


	return psv;
};


template <class CH>
inline std::vector< CH*>& args_ZZ(const CH* pzz,size_t* plength=0,std::vector<CH*>& psv=std::vector<CH*>(),int cinc=8)
{
	
	
		int l;	
		
		while((l=safe_len(pzz))&&(*pzz))
		{
          int n=psv.capacity();
		  if((n%cinc)==0) {
			cinc*=2; 
			psv.reserve(n+cinc);
			};

		  psv.push_back((CH*)pzz);
		  int lp=l+1;
	      if(plength) (*plength)+=lp;
		  pzz+=lp;
			
		}
	
	
	return psv;
};
//





inline double safe_atof(const char* str)
{
	return (str)?atof(str):0;
}
inline double safe_atof(const wchar_t* str)
{
	return (str)?_wtof(str):0;
}

inline double safe_strtod(const wchar_t* str,wchar_t** pend=0)
{
     wchar_t*p;
	if(pend==0) pend=&p;
	return (str)?wcstod(str,pend):0;
}
inline double safe_strtod(const char* str,char** pend=0)
{
	char *p;
	if(pend==0) pend=&p;
	return (str)?strtod(str,pend):0;
}



template <typename CH>
int str_bin(const CH* str, unsigned long& val)
{
	CH* p=(CH*) str;
	int cb=safe_len(p);
	const CH * digs=dflt_v<CH>::digits();

	CH prx[3]={digs[0],digs[11],0};

	if(safe_cmpni(p,prx,min(cb,2))) return 0;
	p+=2;
	cb=min(cb-2,32);
	int n=0;
	val=0;

	for(;n<cb;++n)
	{
		
		int flag=0;
		if(p[n]==dflt_v<CH>::digits()[0]) flag=1; 
		if(p[n]==dflt_v<CH>::digits()[1]) flag=2;
		if(!flag) break;
		 val<<=1;
		if(flag==2) val|=1; 

	}

	return n+1;
}

template <typename CH>
int  ind_dig(CH c)
{
	const CH * low=dflt_v<CH>::digits();
	for(int n=0;n<16;n++)
		if(low[n]==c) return n;
	const CH * upp=dflt_v<CH>::digits(1);
	for(int n=0;n<16;n++)
		if(upp[n]==c) return n;
	return -1;
}

template <typename CH>
int str_hex(const CH* str, unsigned long& val)
{
	CH* p=(CH*) str;
	int cb=safe_len(p);
	const CH * digs=dflt_v<CH>::digits();

	CH prx[3]={digs[0],digs[16],0};

	if(safe_cmpni(p,prx,min(cb,2))) return 0;
	p+=2;
	cb=min(cb-2,32);
	int n=0;
	val=0;

	for(;n<cb;++n){

		int c=ind_dig(p[n]);
		if(c<0) break;
		val<<=4;
		val|=c; 
	}

	return n+1;
}


template <class CH>
inline 	 double get_double(const CH* pval)
{
	unsigned long res;

	if(str_hex(pval,res)) return res;
	if(str_bin(pval,res)) return res;
	return  safe_atof(pval);
}

template <typename CH,typename F>
std::vector<F>& safe_get_vector(const CH* pstr,std::vector<F>& v=std::vector<F>(),F dfv=F())
{ 

	typedef std::basic_string<CH, std::char_traits<CH>,
		std::allocator<CH> > tstring;
	typedef std::basic_stringstream<CH, std::char_traits<CH>,
		std::allocator<CH> > tstringstream;


	const CH* cdig=dflt_v<CH>::digits();
	const CH cq=cdig[17];
	const CH cl=cdig[17+2];
	const CH cr=cdig[17+3];

	v.clear();

	if(!pstr) return v;

	CH* pb=(CH*)pstr;
	//while((*pb)&&(*pb!=cl)) ++pb;
	for(;(*pb)&&(*pb!=cl);++pb);  
	if(*pb==0) 
	{
		if(pstr)
		{
			tstringstream stream(pstr); 
			v.resize(1);
			stream>>v[0]; 

		}
		return v;
	}
	++pb;
	CH* pe=pb;
	int nq=0;
	for(;(*pe)&&(*pe!=cr);++pe)
	{
		if(*pe==cq)   ++nq;
	};

	//while((*pe)&&(*pe!=cr)) ++pe;

	if(pe<=pb) return v;
	int nc=nq+1;
	v.resize(nc);
	CH* p=pb;   
	for(int n=0;n<nc;n++)
	{

	    bool ffail=true;
		try
		{
         tstring sp(p);    
		 int nn=sp.find_first_of(cq);
		 if(nn!=tstring::npos) sp[nn]=0;
		 if(n==(nc-1))
		 {
			 int nn=sp.find_first_of(cr);
			 if(nn!=tstring::npos) sp[nn]=0;
		 }
         tstringstream stream(sp);

         stream>>v[n]; 
		 ffail=stream.fail();
		}
		catch (...){}
		if(ffail) v[n]=dfv;
         
		//v[n]=safe_atof(p);
		for(;(*p!=cr)&&(*p!=cq);++p);
		if(*p) ++p;
	}



	return v;
}


template <class CH>
inline CH* get_named_value(const CH* name,CH* pstr,const CH sep=dflt_v<CH>::equ,int ln=-1)
{
	CH *p=(CH *)safe_strchr(pstr,sep);
	if(p)
	{
		int l=int(p)-int(pstr);
		l/=sizeof(CH);
		ln=(ln<0)?safe_len(name):ln;

		if((l==ln)&&(safe_cmpni(name,pstr,l)==0)) 
			return p+1; 
	}
	return 0;
}


template <class LIST,class CH>
inline int find_named_value(const CH* name,int argc, LIST const& argv,CH** ppout=0,const CH sep=dflt_v<CH>::equ)
{
	CH *p;
	int sl=safe_len(name);
//	if(sl) 
     if(name) 
		for(int n=0;n<argc;n++)
		{ 
			if(p=get_named_value(name,argv[n],sep,sl))
			{
				if(ppout) *ppout=p;
				return n;
			}
		}

		return -1;
}


template <class LIST,class CH>
inline int find_named_value(const CH* name, LIST const& argv,CH** ppout=0,const CH sep=dflt_v<CH>::equ)
{
	return find_named_value(name,argv.size(),argv,ppout,sep);
}

template <class LIST,class CH>
inline const CH* named_value(const CH* name, LIST const& argv,const CH*  def=dflt_v<CH>::empty(),const CH sep=dflt_v<CH>::equ)
{
	char *pp=(char*)def;
   find_named_value(name,argv.size(),argv,&pp,sep);
   return pp;
}

template <class CH>
struct fake_setter
{

	fake_setter(const CH* _name){};
template <typename O,typename V>
void   operator ()(O* o,V& v){};

};

template <class CH,typename LIST=CH** ,typename Setter=fake_setter<CH> >
struct argv_def
{
  typedef CH char_type;
  typedef typename argv_def<CH,LIST,Setter> argv_t;
  typedef std::basic_string<CH, std::char_traits<CH>,
	  std::allocator<CH> > tstring;

  typedef std::basic_stringstream<CH, std::char_traits<CH>,
	  std::allocator<CH> > tstringstream;


   const LIST* pargv;
   int argc;
   int lastindex,lastindex_0;
   CH* prfx;
   CH equsep;
   int prfxlen;
   bufT<CH> prx_buf;
   const void* _pv_;
   argv_t * parent;
   tstring _flat_string;


inline const CH* args(int n)
{
   return   (*pargv)[n];
}



virtual const  CH* init(const CH* _ppargv=0,bool fcopy=false){return 0;};

tstringstream& _flat(tstringstream& stream,bool fsubtree=false,const CH* delim=(CH*)0)
{
	const CH termz=CH(0);

	for(int n=0;n<argc;n++)
	{
		const CH* p=args(n);
		if((p)&&(*p)) 
		{
		  	stream<<p;
			if(delim) stream<<delim;
			else stream<<termz;
		}
	}
	if(fsubtree&&(parent))
		parent->_flat(stream,fsubtree,delim);

	return stream;
}

tstringstream& flat(tstringstream& stream=tstringstream(),bool fsubtree=false,const CH* delim=(CH*)0)
{
	_flat(stream,fsubtree,delim);
	  return stream;
}

const CH* flat_str(bool fsubtree=false,const CH* delim=(CH*)0,int* pcbf=0)
{
	tstringstream stream_flat;
//  stream_flat.str(dflt_v<CH>::empty());
  _flat_string=flat(stream_flat,fsubtree,delim).str();
  if(pcbf)
  {
	  *pcbf=_flat_string.length();
  }
  return _flat_string.c_str();
}


v_buf<CH>& names(int n,v_buf<CH>& vbuf=v_buf<CH>())
{
    CH* pb=(*pargv)[n];
	CH *pe=(CH *)safe_strchr(pb,equsep);
	 //int sz=(pe)? int(pe)-int(pb)+1:-1;
	int sz=(pe)? (int(pe)-int(pb))/sizeof(CH):-1;
	 return copy_z(pb,vbuf,sz,false);
}

inline int _get_value(const CH* name,CH** ppval)
{
	CH* p;
	bufT<CH> buf;
  if(argc>0)  {

	if(prfxlen==0) p=(CH*)name;
	else
	{
		 p=buf;
		 int l=safe_len(name);
         safe_copy(p,prfx,prfxlen);
         safe_copy(p+prfxlen,name,l);
		 p[l+prfxlen]=0;	 
	}

  	lastindex_0=lastindex=find_named_value(p,argc,*pargv,ppval,equsep);
  }
  else lastindex_0=lastindex=-1;

	if((parent)&&(lastindex<0))
	{
		lastindex=parent->_get_value(name,ppval);
	}

	return lastindex;

}

inline int index_of(const CH* name)
{
       CH* p;
	return    _get_value(name,&p);
};

inline bool is(const CH* name)
{
	return index_of(name)>=0;
};

inline bool found()
{
	return lastindex>=0;
}

inline  bool operator!()
{
	return lastindex<0;
}

inline CH& nv_separator()
{
	return equsep;
}



 
struct cval
{
	
     Setter setter;
	 CH* pval;
	 const CH *pname;
	 int lastindex;
	 argv_t* po;
	 
	 cval(const CH* name,argv_t* o):po(o),setter(name),pval(0),pname(name)
	 {
	    lastindex=o->_get_value(name,&pval);
	 };
 
inline int index_of()
{
	return lastindex;
}
inline bool is()
{
    return index_of()>=0;
}

inline  bool operator!()
{
	return lastindex<0;
}
inline 	 operator  CH*()
	 {
		 return pval;
	 }

template<class F>
inline 	operator std::vector<F>()
{
	return safe_get_vector(pval,std::vector<F>());
}

template <class V>
inline cval& operator=(V v)
{
	setter(po,v);
	lastindex=po->_get_value(pname,&pval);
	return *this;
}

inline 	 operator double()
{
  return get_double(pval);
}


inline CH* c_str()
{
	return pval;
}


CH* update(CH*& value)
{
   if (is()) value=pval;
	   return value;
}

template<class D>
D& update(D& value)
{
   if (is()) value=D(*this);
     return value;
}

template<class D>
D def(D dflt=D())
{
  if(pval) return D(*this);
  else return dflt;
}

};

  inline const CH* set_prefix(const CH* name=0)
  {
	  prfxlen=0;
	  if(safe_len(name)==0)
		  return dflt_v<CH>::empty();

	  prfx=(CH*)dflt_v<CH>::empty();
      _get_value(name,&prfx);
     
     prfxlen=safe_len(prfx);
	 return prfx;
  }

  inline const CH* set_prefix_name(const CH* _prefix)
  {
	  prfxlen=safe_len(_prefix);
	  safe_copy(prx_buf.get(),_prefix,prfxlen);
	  prfx=prx_buf;
      prfx[prfxlen]=0;
      return prfx;
  }


   argv_def(int _argc, const LIST & _argv,const CH* _prfxname=dflt_v<CH>::empty())
	   :argc(_argc),prfxlen(0),_pv_(_argv),parent(0) {
	       
        pargv=(const LIST *)&_pv_;
        set_prefix(_prfxname);
		equsep=dflt_v<CH>::equ;
   };

  argv_def(LIST const& _argv,const CH* _prfxname=dflt_v<CH>::empty())
	  :pargv(&_argv),prfxlen(0),parent(0){
	  argc=pargv->size();
      set_prefix(_prfxname);
	  equsep=dflt_v<CH>::equ;
     }


  inline cval getval(CH* name)
  {
	  return cval(name,this);
  }

  inline cval operator[](CH* name)
  {
	  return getval(name);
  }
  inline cval operator[](const CH* name)
  {
	  return getval((CH*)name);
  }

/*
template<typename N>
N  val(CH* name,N defval=N())
  {
     
  }
*/
};



template <class Stream,class Getter>
int get_recv_line(Stream& s,const Getter& g)
{
	char l;
	int c=0,e;
	for(;;){
		e=g(&l,1);

		if(e<=0){
			return (e)?-1:0;
		};
		
		s<<l;
		c++;
		if(l=='\n') break;
	}
	return c;
};

template <class Stream,class Getter>
int get_recv_lines(Stream& s,const Getter& g)
{
	char l;
	bool f=true;
	int c=0,e;
	for(;;){
		e=g(&l,1);

		if(e<=0){
			return (e)?-1:0;
		};

		s<<l;
		c++;
		if(l=='\n'){
			if(f) break;
			else f=true;
		}
		if(!isspace(l)) f=false;
	}
	return c;
};

struct wbs_data
{
	long long size;
    char* p;
	long mode;
};


template <class Getter>
long long get_ws_size(const Getter& g)
{
	long long sz;
    char l;
	int c;
	c=g(&l,1);
	if(c!=1) return -1;

    //for(int )
	
	
	return sz;
};



template <class Stream,class Getter>
int get_wbs_string(Stream& s,const Getter& g)
{
	unsigned char c;
	//unsigned char cc[10]={0};int nn;
	int n=0;
	 
	if(1!=g(&c,1,2)) return -1;//MSG_PEEK
	if(c!=0) return -2;
	//nn=g(&cc,7);
    if(1!=g(&c,1)) return -1;
	//if(c!=0) return -2;
	while(g(&c,1)==1)
	{
	   if(c==0xFF) 
	   {
		    s<<'\0'<<'\0';
			return (n)?n+2:0;
	   }
       s<<c;  
	   ++n;
	}
	return -1;
}

template <class Stream,class Getter>
int get_wbs_string_fast(Stream& s,const Getter& g)
{
	 char c[1024];
	//unsigned char cc[10]={0};int nn;
	int n=0;
	if(1!=g(c,1,2)) return -1;
	if(c[0]!=0) return -2;
	//nn=g(&cc,7);
	if(1!=g(c,1)) return -1;
	//if(c!=0) return -2;
	int cb;
	while((cb=g(c,1024))>0)
	{
		//if(c==0xFF) 
		for(int j=0;j<cb;j++)
			if(c[j]==0xFF)
			{
				cb=j;
				n+=cb;
				s.write(c,cb);
				s<<'\0'<<'\0';
				return (n)?n+2:0;
			}
        
      n+=cb;
      s.write(c,cb);
	}
	return -1;
}


inline bool to_hex(unsigned char& i)
{
	if(('0'<=i)&&(i<='9')) return i+=0-'0',true;
	 else if(('a'<=i)&&(i<='f')) return i+=10-'a',true;
		else if(('A'<=i)&&(i<='F')) return i+=10-'A',true;
  return false;
}
inline char* url_unescape(char* src,int cb,char* dest_buf,int* pbufsize=0)
{
	if(cb==-1) cb=safe_len(src);
	
	int n=0;

   for(int i=0;i<cb;++i)
   {
	   char *p=src+i;
	   if(*p=='%')
	   {
     	     unsigned char h,l;
            if(i+3>cb) return 0;
			h=p[1];l=p[2];
    		//_strlwr(buf);
			
			if(!to_hex(h)) return 0;
			if(!to_hex(l)) return 0;
			unsigned char c= (h<<4)+l;	
         dest_buf[n]=c;
		 i+=2;
	   }
	   else if(*p=='+') dest_buf[n]=' ';
	      else dest_buf[n]=*p;
		  ++n;
   }
     dest_buf[n]=0;

     if(pbufsize)
		 *pbufsize=n;

	 return dest_buf;

}



inline char* url_escape2(char* src,int cb,char* dest_buf,int* pbufsize=0)
{
	if(cb==-1) cb=safe_len(src);
	else if(cb==-2) cb=safe_len_zerozero(src);

	int cbbuf=3*cb+1;

	int& cbout=(pbufsize)?*pbufsize:cbbuf;

	if(cbout<cbbuf) return 0;

	char b[4]={'\%',0,0,0};
	char bb[4]={'\%',0,0,0};
	char fhex[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	std::strstream s(dest_buf,cbout);
	s.seekp(0);
	cbout=0;
	for(int n=0;n<cb;n++)
	{
		unsigned char c=(unsigned char)src[n];
		
		if((isalnum(c))||c=='='||c=='_'||c=='-'||c=='/'||c==':'||c==';'||c=='?'||c=='.')
				s<<c;	
		else
		{
			//sprintf(b,"%02x",c);
			bb[1]=fhex[int(c>>4)];
			bb[2]=fhex[int(0x0F&c)];//
			s<<bb;	
		}
	}
	s<<'\0';
	cbout=s.pcount()-1;
	return dest_buf;
}


inline char* url_escape(char* src,int cb,char* dest_buf,int* pbufsize=0,int fan=1)
{
	 if(cb==-1) cb=safe_len(src);
	 else if(cb==-2) cb=safe_len_zerozero(src);

	 int cbbuf=3*cb+1;

	 int& cbout=(pbufsize)?*pbufsize:cbbuf;
	 
     if(cbout<cbbuf) return 0;
	 
	 char b[4]={'\%',0,0,0};
	 char bb[4]={'\%',0,0,0};
	 char fhex[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	 std::strstream s(dest_buf,cbout);
	 cbout=0;
     for(int n=0;n<cb;n++)
	 {
		 unsigned char c=(unsigned char)src[n];
		 
		 
	 if(fan&&(isalnum(c))||c=='='||c=='_'||c=='-')
	         s<<c;
 		 else
			 {
				 //sprintf(b,"%02x",c);
				 bb[1]=fhex[int(c>>4)];
				 bb[2]=fhex[int(0x0F&c)];//
				 s<<bb;	
	      }
      }
     s<<'\0';
	 cbout=s.pcount()-1;
	 return dest_buf;
}


#include <iostream>
#include <sstream>
#include <fstream>

template <class CH>
struct args_f
{
	typedef std::basic_string<CH, std::char_traits<CH>,
		std::allocator<CH> > tstring;
	typedef std::basic_fstream<CH, std::char_traits<CH> > tfstream;
	std::vector<tstring> vst;
	std::vector<CH*> pst;
	argv_def<CH>* pargv_def;

	args_f(CH* fn=0,const CH* _prfxname=dflt_v<CH>::empty()):pargv_def(0)
	{
		if(fn)	open(fn,_prfxname);
	}
args_f<CH>& open(CH* fn,const CH* _prfxname=dflt_v<CH>::empty())
	{
		delete pargv_def;
         pargv_def=0;

		

		if(!fn) return *this; 

		

		tfstream fss(fn);
            
      if(fss.good())
		try
		{

         v_buf_fix<wchar_t> vbuf; 

		 wchar_t* buf=vbuf;
		
		tstring s;
		vst.reserve(1024);
		pst.reserve(1024);
		while(!fss.eof())
		{
			
			fss.getline(buf,1023);
			int c=fss.gcount();
			if(c==0) break;
			buf[c]=0;
			vst.push_back(buf);
			pst.push_back((CH*)vst.back().c_str());
			
		}
		if(pst.size())
		  pargv_def= new argv_def<CH>(pst.size(),&pst[0],_prfxname);
		}
		catch(...){};
		fss.close();
		return *this;
	}


	~args_f(){delete pargv_def;};
inline    argv_def<CH>* pargs()
{
	return pargv_def; 
}

	inline int size()
	{
		return vst.size();
	};
	inline  CH*& operator[](int n)
	{
		return (CH*)(pst[n]);
	}
	inline CH** ptr()
	{
		return &pst[0];
	}
};

template <class CH>
struct argv_file :argv_def<CH>
{


	inline static __int64 get_file_size(const wchar_t* fn)
	{
		__int64 sz=-1LL;
		HANDLE hf=CreateFileW(fn,FILE_READ_ATTRIBUTES,0,0,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,0);
		if(hf!=INVALID_HANDLE_VALUE)
		{
			ULARGE_INTEGER ll={};
			ll.LowPart=GetFileSize(hf,&ll.HighPart);
			sz=ll.QuadPart;
			CloseHandle(hf);

		}
		return sz;
	}

	
	typedef std::basic_fstream<CH, std::char_traits<CH> > tfstream;
	std::vector<tstring> vst;
	std::vector<CH*> pst;
	
	argv_file(CH* fn=0,const CH* _prfxname=dflt_v<CH>::empty()):argv_def<CH>(0,(CH**)0){
       		 open(fn,_prfxname);
	}
	argv_file<CH>& open(CH* fn,const CH* _prfxname=dflt_v<CH>::empty())	{
		

		if(!fn) return *this; 

		__int64 sz=get_file_size(fn);
         if(sz<0) return *this; 
		v_buf<CH> vfn;
		CH q=((CH*)char_mutator<>(L"\""))[0];
		vfn.undecorate(fn,q,q);
		if(safe_len(vfn.get())==0) return *this; 

		tfstream fss(vfn);

		if(fss.good())
			try
		{

			
            int k=sz ;
			v_buf<CH> vbuf(k);
			//vbuf.resize(size_t(sz)); 

			CH* buf=vbuf.get();

			tstring s;
			vst.reserve(1024);
			pst.reserve(1024);
			while(!fss.eof())
			{

				fss.getline(buf,vbuf.count()-1);
				int c=fss.gcount();
				if(c==0) break;
				buf[c]=0;
				vst.push_back(buf);
				pst.push_back((CH*)vst.back().c_str());

			}

			argc=pst.size();
			if(argc){
				_pv_=&pst[0]; 
			     set_prefix(_prfxname);
			}
			
		}
		catch(...){};
		fss.close();
		return *this;
	}

};


inline char* find_iter(const char* ps,int len, char* pb, char* pe) 
{
	char* p=pb;
	while(p<pe){
		if(*ps==*p)
		{  
			int n=min(len,int(pe)-int(p));
			if(strncmp(ps,p,n)==0) return p;
		}
		++p;
		;}
	return 0;


}
template <class CH>
inline CH* find_iter(const CH c, CH* pb, CH* pe) 
{
	CH* p=pb;
	while(p<pe){
		if(c==*p) return p;
		++p;
		;}
	return 0;
}

template <class region>
inline char* __get_header(char* hn,region r,int* pcb)
{
	int cb=strlen(hn);
	char* p=r.ptr;
	if(!p) return 0;
	char* pe=r.ptr+r.sizeb;//-cb;
	for(p;p<pe;++p)
	{
		if(strnicmp(hn,p,cb)==0)
		{
			int &c=*pcb;
			char *pv=p+cb,*pc=pv;
			while((!isspace(*pc))&&(pc<pe)) ++pc;

			c=int(pc)-int(pv);
			return pv;
		}
	}
	return 0;
}


#define _GET_HEADER(a,r,pi) __get_header("\n" #a ": ",r,pi)
#define _HTTP_END_HEADER "\r\n\r\n"

inline char* find_http_end(char* p,int cb)
{
    char* pe=p+cb-4;
	 unsigned int heh=*(( unsigned int*)_HTTP_END_HEADER);
	for(p;p<pe;p++)
	{
        if( *(( unsigned int*)p)==heh) return p+4;
	}

	return 0;
}

