#pragma once
#include "singleton_utils.h"
#include <math.h>

template <int _DIG=12>
struct decay_filter_t
{
	
	typedef unsigned long ulong_t;
	typedef unsigned char byte_t;
	ulong_t an,bn,w,h;
	ulong_t an2,bn2;
    double t,alpha,beta;
	double t2,alpha2,beta2;
	enum    
	{
		alpha_denom_dig = _DIG ,
		alpha_denom =1<<alpha_denom_dig
	};

inline static 	byte_t blend_pt(const byte_t v_src,ulong_t a,ulong_t b,ulong_t* pdwdest)
	{
		ulong_t v;
		v= (b*(*pdwdest))>>alpha_denom_dig;
		v= v+a*v_src;
		*pdwdest=v; 
		return v>>alpha_denom_dig;
	}


	struct line_t
	{
		int w;
		ulong_t* pdw;
		ulong_t an,bn;
		line_t& init(decay_filter_t& df,int row)
		{
			w=(df.w);
			an=(df.an);
			bn=(df.bn);
         pdw=df.pbuf[row];
		 return *this;
		}
        

		inline      void makeline(byte_t* pdest,byte_t* psrc)  
		{
             for( ulong_t* p=pdw;p<pdw+w;p++,pdest++,psrc++)
				 	          *pdest=blend_pt(*psrc,an,bn,p);
		}

inline      byte_t make(int pos,byte_t bv)  
	  {
		return  blend_pt(bv,an,bn,pdw+pos); 
	  }
inline byte_t operator()(int pos,byte_t v) 
{

	      return  blend_pt(bv,an,bn,pdw+pos); 

};
};

inline     line_t& line(int row)
	 {
		 return _line.init(*this,row);
	 }
	decay_filter_t(double _t=0)
	{
       set_alpha(_t); 
	};

decay_filter_t&   set_alpha(double _t=0,double _t2=0)
{
  t=_t;
  t2=_t2;
  an=alpha_denom;
  bn=0;
  an2=alpha_denom;
  bn2=0;
  if(t>0)
  { 
   //beta=exp(-1./t);
	  alpha=1-beta;
   beta=pow(2,-1./t);
   bn=alpha_denom*beta;
   an=alpha_denom-bn;
      
  }
  if(t2>0)
  { 
	  //beta=exp(-1./t);
	  alpha2=1-beta2;
	  beta2=pow(2,-1./t2);
	  bn2=alpha_denom*beta2;
	  an2=alpha_denom-bn2;

  }
   return *this;
}
decay_filter_t&   set_alpha(std::vector<double> vt)
{    int n=  vt.size();
      if(n)
	  {
        if(n<2) set_alpha(vt[0]);
		else set_alpha(vt[0],vt[1]);
	  }
      return *this;
}
int   set_size(int _w,int _h)
{
	int siz=(w=_w)*(h=_h);
  if(siz!=dwbuf.count())
  { 
   dwbuf.resize(siz);
   pbuf.resize(h);
   for(int n=0;n<h;n++) pbuf[n]=&dwbuf[w*n];
  }

  return siz;
}
operator bool()
{ 
	return bn||bn2;
 }

 v_buf<ulong_t> dwbuf;
 v_buf<ulong_t*> pbuf;
 line_t _line;
 
};

template <int _DIG=12>
struct decay_filter_buf_t:decay_filter_t<_DIG>
{
  v_buf<byte_t> buf;
  std::vector<int> mask;
  decay_filter_buf_t(){ };
byte_t*  make_mix(int _w,int _h,int linesize,byte_t* pbits_src)
  {
	  set_size(_w,_h);
	  int sz= linesize*_h;
	  if(sz!=buf.count()) buf.resize(sz);
	  byte_t* pbits_dest,*p,*psrc=pbits_src;
	  pbits_dest=p=buf.get();
	  int mn=mask.size();
	 std::vector<byte_t> invmask(mn);
	 for(int nn=0;nn<mn;nn++) invmask[nn]=~byte_t(mask[nn]);
	 

	 if(bn2==0)
	 {

	 

      for(int n=0;n<h;n++,p+=linesize,psrc+=linesize)
	  {
       //  line(n).makeline(p,pbits_src);
		  ulong_t* pwl=pbuf[n];
		   
		  for(int m=0;m<w;m++)
		  {
			  byte v=blend_pt(psrc[m],an,bn,pwl++);
			  
			  if(mn)
			  {
				  int minx=m%mn;
				  v&=invmask[minx];

			  }
			  //p[m]=byte_t((*pwl)>>alpha_denom_dig)-v;
			  //p[m]=v-byte_t((*pwl)>>alpha_denom_dig);
			  p[m]=v;

			  
		  }


	  }
	 }
	 else
	 {
		 byte y;
         char uv;
			 
		 if(mn==2)
		 {
			 y=mask[0];
				 uv=mask[1];
		 };
		 for(int n=0;n<h;n++,p+=linesize,psrc+=linesize)
		 {
			 //  line(n).makeline(p,pbits_src);
			 ulong_t* pwl=pbuf[n];

			 for(int m=0;m<w;m++)
			 {   byte v;
				 if(m&1)  v=blend_pt(psrc[m],an2,bn2,pwl++);
					 else v=blend_pt(psrc[m],an,bn,pwl++);

				 if(mn==2)
			  {
				  if(m%mn) 
				  	  v=(int(v)/int(uv))*int(uv);
				  else 
                      v=(unsigned(v)/unsigned(y))*unsigned(y);
				  
					  //v&=invmask[minx];

			  }
				 //p[m]=byte_t((*pwl)>>alpha_denom_dig)-v;
				 //p[m]=v-byte_t((*pwl)>>alpha_denom_dig);
				 p[m]=v;


			 }


		 }

	 }

	  return pbits_dest;

  }

};
