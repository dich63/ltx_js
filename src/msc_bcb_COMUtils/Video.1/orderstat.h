#pragma once

#include <vector>
#include <complex>



template<int ncol,int tpow=0>
struct line_type_c
{
	unsigned char* pbits;
	line_type_c(unsigned char* p):pbits(p){};
	inline  unsigned char operator[](int n)
	{
		unsigned char c=pbits[((n<<1)+n)+ncol];
		;
		return (c>>tpow)<<tpow;
	}
};



//template<int ncol=2,int tpow=1>
template< class line_type_C>
struct RGB24_colget
{

/*
	struct line_type_c
	{
         unsigned char* pbits;
			 line_type_c(unsigned char* p):pbits(p){};
		inline  unsigned char operator[](int n)
		{
			unsigned char c=pbits[((n<<1)+n)+ncol];
			;
			return (c>>tpow)<<tpow;
		}
	};
*/
	//typedef typename line_type_c<ncol,tpow> line_type_t;
	typedef typename line_type_C line_type_t;

     int w;
	 int h;
	 unsigned char* pbits;
	 int linesize_b;



	 inline int width(){return w;}
	 inline int height(){return h;}


	 RGB24_colget(void*p, int wn,int hn,int lsb):pbits((unsigned char*)p),w(wn),h(hn),linesize_b(lsb){};

line_type_t	 operator [](int n)
{
	return line_type_t(pbits+n*linesize_b);
}

};





template <class T> 
struct typedeffer
{

template<class V> static  char is_line(V* t,typename V::line_type_t* pl=0);

template<class V> static   float is_line(void* t);

enum 
{
	result=sizeof(is_line<T>((T*)0))==sizeof(char)
};


};


template <class T,bool f>
struct line_type
{
   typedef typename T type_t;
};

template <class T>
struct line_type<T,true>
{
	typedef typename  T::line_type_t type_t;
};

template <class T>
struct line_type<T*,false>
{
	typedef typename  T type_t;
};


inline static RECT center(RECT rc)
{
//	int w=rc.bottom-rc.top;
//	int h=rc.right-rc.left;

		int h=rc.bottom-rc.top;
		int w=rc.right-rc.left;

	rc.left=-w/2;
	rc.right=+w/2;
	rc.top=-h/2;
	rc.bottom=+h/2;
	return rc;
}

inline static POINT center(POINT pt)
{
	int w=pt.y-pt.x;
	pt.x=-w/2;
	pt.y=+w/2;
	return pt;
}


inline  RECT shift(RECT r,int x,int y)
{
	r.left+=x;
	r.right+=x;
	r.top+=y;
	r.bottom+=y;
	return r;
}

inline RECT make_rect(int l,int t,int r,int b)
{
	RECT rr={l,t,r,b};
	return rr;
}

inline POINT make_point(int x,int y=0)
{
	POINT r={x,y};
	return r;
}

inline static	inline  RECT _inrect(RECT r,RECT& outr)
{
	if(r.left<outr.left) r.left=outr.left;
	if(r.top<outr.top) r.top=outr.top;
	if(r.right>=outr.right) r.right=outr.right-1;
	if(r.bottom>=outr.bottom) r.bottom=outr.bottom-1;
	return r;
}

inline static	inline  RECT _inrect_cl(RECT r,RECT& outr)
{
	if(r.left<outr.left) r.left=outr.left;
	if(r.top<outr.top) r.top=outr.top;
	if(r.right>outr.right) r.right=outr.right;
	if(r.bottom>outr.bottom) r.bottom=outr.bottom;
	return r;
}


inline static	inline  POINT _ininterval(POINT pt,POINT& outpt)
{
	if(pt.x<outpt.x) pt.x=outpt.x;
	if(pt.y>=outpt.y) pt.y=outpt.y-1;
	return pt;
}


inline bool IsRectNull(RECT &r)
{
	return ((&r)==0)||(r.left==0)&&(r.top==0)&&(r.right==0)&&(r.bottom==0);
}

inline static	inline  RECT flipv(RECT r,const RECT& out)
{
	int h=out.bottom-out.top;
	RECT rr={r.left,h-r.bottom,r.right,h-r.top};
	return rr;
}

inline static	inline  RECT from_margin(RECT m,RECT out)
{
	RECT r={out.left+m.left,out.top+m.top,out.right-m.right,out.bottom-m.bottom};
	return r;
}

template <class float_type>
std::complex<float_type> center_point(RECT r)
{
	double x=r.right+r.left,y=r.top+r.bottom;
	return std::complex<float_type>(x/2,y/2);
}

template <class float_type>
struct array2D
{
	int w;
	int h;
	int size;
	std::vector<float_type*> parray;
	std::vector<float_type> pbase;
	float_type fnorm;
	int naccc;

     

	float_type*	reset(int wn=0,int hn=0)
	{
        naccc=0;
        fnorm=0;
		int sizen=wn*hn;
		pbase.reserve(max(size,sizen)); 
		parray.reserve(max(w,wn)); 
		w=wn;
		h=hn;
		pbase.resize(0);
		pbase.resize(size=sizen);
		parray.resize(0);
		parray.resize(h);
		if(size)
		{
		
		float_type *p=&pbase[0];
		for(int n=0;n<h;n++) parray[n]=p+n*w;

		  return &(pbase[0]);
		}
		else return 0;
	};

inline void	clear(){
	         reset(0,0);
            }

	array2D():w(0),h(0),size(0),fnorm(0),naccc(0){};
	array2D(int wn,int hn):size(0),h(0),fnorm(0),naccc(0)
	{
		reset(wn,hn);
	};


	template<class T>
	float_type accumulate(T& a,RECT cutoff=make_rect(0,0,MAXLONG,MAXLONG))
	{
	 	
      return  accumulate(a,a.width(),a.height(),cutoff);
	}


inline bool	is_empty()
	{
		return size==0;
	}

	template<class T>
	int accumulate(T& a,int wn,int hn,RECT cutoff=make_rect(0,0,MAXLONG,MAXLONG))
	{
       typedef typename line_type<T,typedeffer<T>::result >::type_t line_type_t;
	
	   cutoff=_inrect(cutoff,make_rect(0,0,wn,hn));

		if(naccc==0)
		{
         reset(wn,hn);
		}

		++naccc;

		for(int n=cutoff.top;n<=cutoff.bottom;n++)
		{

			
			line_type_t	pline_src=a[n];
			float_type* pline=parray[n]; 
			for(int m=cutoff.left;m<=cutoff.right;m++)
			{
				float_type d=pline[m];
					d+=pline_src[m];
					pline[m]=d;
					if(d>fnorm) fnorm=d;
				
			}


		}
             
		return naccc;
	}
	
	float_type threshold(float_type pt,float_type pb=1.001)
	{
       float_type td=pt*fnorm;
       float_type bd=pb*fnorm;
	      
		  int nt=0;
	   for(int n=0;n<h;n++)
	   {

    	   float_type* pline=parray[n]; 
		   for(int m=0;m<w;m++)
		   {
			   float_type d=pline[m];
			   if(d<td)  pline[m]=0;
			   //else if(fback) pline[m]=1;
			   if(d>=bd) pline[m]=bd;
			   else nt++;
   		   }

	   }

         return float_type(nt)/float_type(size);
	}

	inline int width(){return w;}
	inline int height(){return h;}

	int write_to_file(char* fn)
	{
		FILE* hf=fopen(fn,"wb");
		int cb=write_to_file(hf);
        fclose(hf);
		return cb;
	};
	int write_to_file(wchar_t* fn)
	{
		FILE* hf=_wfopen(fn,L"wb");
		int cb=write_to_file(hf);
		fclose(hf);
		return cb;
	};
	int write_to_file(FILE* hf)
	{
		
		int cb=0;

		cb+=fwrite(&w,sizeof(int),1,hf);
		cb+=fwrite(&h,sizeof(int),1,hf);
		if(size>0)
             cb+=fwrite(&(pbase[0]),sizeof(float_type),size,hf);
		
		return cb;

	};

};

template <class float_type>
struct distrib2D:array2D<float_type>
{
/*
	int w;
    int h;
	int size;
	std::vector<float_type*> parray;
    std::vector<float_type> pbase;
	float_type fnorm;



float_type*	reset(int wn,int hn)
	{
		
		int sizen=wn*hn;
		 pbase.reserve(max(size,sizen)); 
		 parray.reserve(max(w,wn)); 
		 w=wn;
		 h=hn;
		 pbase.resize(0);
         pbase.resize(size=sizen);
         parray.resize(0);
		 parray.resize(h);
       for(int n=0;n<h;n++) parray[n]=pbase+n*w;
	   return &(pbase[0]);
	};

*/
	distrib2D(){};
	distrib2D(int wn,int hn):array2D<float_type>(wn,hn){};

	template<class T>
	distrib2D&  make_from(T& a)
	{
		return make_from(a,a.width(),a.height());
	};

	template<class T>
	distrib2D&  make_from(T& a,int wn,int hn)
	{
		//,T::line_type_t * pl
         typedef typename line_type<T,typedeffer<T>::result >::type_t line_type_t;

         reset(wn,hn);

		 for(int n=0;n<h;n++)
		 {
			 float_type d=0;

			 line_type_t pline_src=a[n];
			 float_type* pline=parray[n]; 
			 for(int m=0;m<w;m++)
			 {
				 d+=pline_src[m];
				 pline[m]=d;
			 }


		 }

		 //float_type dfull=0;

		 for(int m=0;m<w;m++)
		 {
    		 float_type d=0;

           for(int n=0;n<h;n++)
		   {
			   float_type& f=parray[n][m];
                d+=f;
				f=d;
				//dfull+=d;
		   }
		 }

         fnorm=parray[h-1][w-1];
		 return *this;
	};


inline  RECT& inrect(RECT& r)
{
	if(r.left<0) r.left=0;
	if(r.top<0) r.top=0;
    if(r.right>=w) r.right=w-1;
	if(r.bottom>=h) r.bottom=h-1;
	return r;
}


inline float_type _probab(RECT& r )
{
      inrect(r);
	//float_type dl=(r.top<0)?0:parray[r.top][r.right];
    //float_type dt=(r.top<0)?0:parray[r.top][r.right];
  return parray[r.bottom][r.right] - parray[r.top][r.right] - (parray[r.bottom][r.left]-parray[r.top][r.left]);
}
inline float_type probab(RECT r )
{
       float_type p=_probab(r);
	   return p/fnorm;
}

RECT max_rect(RECT rc,float_type* pprm=0,RECT cutoff=RECT(0,0,MAXLONG,MAXLONG))
{
	
	RECT rmax=center(rc);
	inrect(rmax);
	cutoff=inrect(cutoff);
	float_type pbmax=0,pbn;
      float_type fnorm=_probab(cutoff);
	for(int n=cutoff.top;n<=cutoff.bottom;n++)
	{
		
		//float_type* pF=parray[n]; 
		for(int m=cutoff.left;m<=cutoff.right;m++)
		{
			RECT r=shift(rc,m,n);
              r=_inrect(r,cutoff);
			float_type pb=_probab(r);
			if(pbmax<pb)
			{
				
               pbmax=pb;
			   pbn=pbmax/fnorm;
              rmax=r;
			}
			
		}
	}

	if(pprm) *pprm=pbmax/fnorm;
	return rmax;
}

RECT median()
{
	return median(make_rect(0,0,w,h));
}

RECT median(RECT r,float_type hl=0.5,float_type hr=0.5)
{
	float_type mfl=_probab(r)*hl;
	float_type mfr=_probab(r)*hr;
   RECT rm=RECT();
     float_type ppp=probab(r)/2;
   for(int n=r.top;n<=r.bottom;n++)
   {
   
	   float_type d=parray[n][r.right]-parray[n][r.left]; 
	   
	   if(d<=mfr) 
	   {
		   rm.bottom=n;
	       if(d<mfl) rm.top=n;
	   }
   }

   float_type* pdb=parray[r.bottom]; 
   float_type* pdt=parray[r.top]; 
   for(int n=r.left;n<=r.right;n++)
   {

	   float_type d=pdb[n]-pdt[n]; 

	   if(d<=mfr) 
	   {
		   rm.right=n;
		   if(d<mfl) rm.left=n;
	   }
   }

	
   return rm;
}


inline RECT csquare(float_type siz,int x,int y)
{
	RECT r={x-siz,y-siz,x+siz,y+siz};
	return r;
}


RECT min_square_percent_in(RECT rbound,float_type percent,float_type* pprm=0)
{
	int sz=max(rbound.bottom-rbound.top,rbound.right-rbound.left);

//	RECT rc={0,0,sz,sz};
//	center(rc);

		float_type pbb=_probab(rbound);
		float_type pthreshold=pbb*percent;

        inrect(rbound);
    
	float_type	irc=sz,pbc=0;
	int xc=0;
	int yc=0;
	
	float_type pbmax=0,pbn;
	for(int n=0;n<h;n++)
	{

		//float_type* pF=parray[n]; 
		for(int m=0;m<w;m++)
		{

			for(int ir=sz;ir>0;ir--)
            {
				
				RECT r=csquare(ir,m,n);
			float_type pb=_probab(r);
			
			 if(pthreshold>pb)
			 {
                xc=m;
				yc=n;
				irc=ir;
				pbc=pb;
                break;
			  }
			}

		}
	}


	if(pprm)
	{
		
		if(pbb>0)
		  *pprm=pbc/pbb;
		else *pprm=0;
	}
	return csquare(irc,xc,yc);
}

};
