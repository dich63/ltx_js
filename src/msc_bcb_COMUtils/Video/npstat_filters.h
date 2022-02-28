#pragma once
//#include "npstat_filters.h"

#include "filters3d.h"
#include "quantiles.h"

template <int _NValue=14,class _FType=float,class _ValueType=unsigned char>
struct PD_base_t
{

	typedef unsigned char byte_t;
	typedef _FType float_t;
	typedef _ValueType value_t;
	enum{
		count=  _NValue,
		last=count-1
	};
	float_t prob[count];
		//float_t beta;
	value_t cursor,median,left,right;
	float_t ci_l,ci_r;
	

	static inline value_t min2(value_t a ,value_t b)
	{
		return (a<b)?a:b;
	}
	static inline value_t max2(value_t a ,value_t b)
	{
		return (a>b)?a:b;
	}

	inline value_t update_quantiles()
	{
		byte_t flags=(1<<0)|(1<<1)|(1<<2);
		for(byte_t k=0;k<count;k++)
			update_quantiles(prob[k], k, flags);
		return median;
	}


	inline byte_t update_quantiles(const  float_t f ,const value_t k,byte_t& flags)
	{
		const			float_t ci_m=0.5;
		if(flags)
		{
			if((flags&(1<<0))&&((f>=ci_l)))
			{
				left=k; 
				flags&=~(1<<0);
				if(f>=ci_m)
				{
					median=k; 
					flags&=~(1<<1);

					if(f>=ci_r)
					{
						right=k; 
						flags&=~(1<<2);

					}

				}


			}
			else
				if((flags&(1<<1))&&((f>=ci_m)))
				{
					median=k; 
					flags&=~(1<<1);

					if(f>=ci_r)
					{
						right=k; 
						flags&=~(1<<2);

					}

				}
				else 
					if((flags&(1<<2))&&((f>=ci_r)))
					{
						right=k; 
						flags&=~(1<<2);
					}

		}


		return flags;

	}

	inline 	float_t mix_distribution_and_prob(value_t pos,const float_t beta)
	{
		value_t k=0;
		for(;k<pos;k++) blend(prob[k],beta);
		
		for(;k<last;k++) blend_add(prob[k],beta);
        return prob[cursor];
	}


	inline 	value_t median_left(value_t pos)
	{
		const			float_t ci_m=0.5;
		for(value_t k=pos;k<last;k++)
			if(prob[k]>ci_m) 
				 return median=k;
		/*int k=int(pos)+1;
		while(prob[k]<=ci_m) ++k;
		;*/
		return  median=last-1;
	}
	inline 	value_t median_right(value_t pos)
	{
		const			float_t ci_m=0.5;
		value_t k,m=0;
         for( k=pos;k>0;k--)
		 {
			 if(prob[k]<=ci_m) 
			 {
               m=k;
			   break;
			 }

				 
		 }

		return median=m;
		/*
		int k=int(last)-1;
		//
		//while(k&&(prob[k]>=ci_m)) --k; 
		 //while(k&&(ci_m<=prob[k])) --k; 
          k=0;
		//
		  while((k<last)&&(prob[k]<=ci_m)) ++k; 
		  //while((k<=int(pos))&&(prob[k]<=ci_m)) ++k; 
		  return median=k;
		  */
		
	}
	inline 	value_t median_get(value_t pos)
	{
		const			float_t ci_m=0.5;
		
		if(pos>median)
		{
           int k=median;   
		   while((k<last)&&(prob[k]<=ci_m))++k;
		   median=k;
		}
		else if(pos<median)
		{
			int k=median;   
			while((k>0)&&(prob[k]>ci_m)) --k;
			median=k;
		}

       //while(prob[k]<=ci_m)++k;
	   return  median;

	}

	inline 	value_t median_getr()
	{
		const			float_t ci_m=0.5;
		value_t pos=0;
		while(prob[pos]<=ci_m)++pos;
		return  pos;

	}

	inline 	value_t mix_distribution2(value_t pos,const float_t beta)
	{
		
		float_t p=mix_distribution_and_prob(pos,beta);
		
		//if(p<ci_l)        cursor=median_left(cursor);
		//else if(p>=ci_r)  cursor=median_right(cursor);

		//if(p<ci_l)        cursor=median_getr();
		//else if(p>=ci_r)  cursor=median_getr();
		if((p<ci_l)||(p>=ci_r))
			   cursor=median_getr();

			return cursor;

	}
	inline 	value_t mix_distribution3(value_t pos,const float_t beta)
	{

		float_t p=mix_distribution_and_prob(pos,beta);

		 update_quantiles();
		 if((cursor<left)||(right<cursor)) cursor=median;

		return cursor;

	}



	inline 	bool mix_distribution_and_check(value_t pos,const float_t beta)
	{
		float_t p=mix_distribution_and_prob(pos,beta);
		return (p<ci_l)||(p=>ci_r);

	}


	inline 	value_t mix_distribution(value_t pos,const float_t beta)
	{

		//value_t l=0,r=count-1,m=0;
		byte_t flags=(1<<0)|(1<<1)|(1<<2);

		value_t k=0;

		for(;k<pos;k++){

			//prob[k]*=beta;
			blend(prob[k],beta);
			if(!update_quantiles(prob[k],k,flags))
			{++k;break;}			
		}

		if(!flags)
			for(;k<pos;k++){
				//prob[k]*=beta;
				blend(prob[k],beta);
			}

			if(flags)
				for(;k<last;k++){
					//prob[k]=beta*(prob[k]-1)+1;
					blend_add(prob[k],beta);
					if(!update_quantiles(prob[k],k,flags))
						{++k;break;}
				}

				if(!flags)
					for(;k<last;k++){
						//prob[k]=beta*(prob[k]-1)+1;
						blend_add(prob[k],beta);
					}

					if((cursor<left)||(right<cursor)) cursor=median;

					/*
					if (cursor==0)
					{ 
						__asm{  int 3};

					}
					else if(cursor==last)
					{
                          //__asm{  int 3};
					}
					*/

					return cursor;
	}


  inline value_t update_left(value_t k,value_t pos,const float_t beta)
	{
		for(;k<pos;k++){

			blend(prob[k],beta);

		}
		for(;k<count;k++){
			//prob[k]=beta*(prob[k]-1)+1;
			blend_add(prob[k],beta);

		}
		return cursor;

	};
  inline value_t update_right(value_t k,value_t pos,const float_t beta)
  {
	  
	  for(;k>=pos;k--){
		  blend_add(prob[k],beta);
		  //prob[k]=beta*(prob[k]-1)+1;
		  
	  }

	  for(;k>=0;k--){

		  //prob[k]*=beta;
		  blend(prob[k],beta);

	  }

	  return cursor;

  };

  inline value_t update_median_left(value_t k,value_t pos,const float_t beta)
  {
       value_t m=median;
	  for(;k<pos;k++){

		  blend(prob[k],beta);

	  }
	  for(;k<count;k++){
		  //prob[k]=beta*(prob[k]-1)+1;
		  blend_add(prob[k],beta);

	  }
	  return cursor;

  };

  inline value_t clamp(value_t pos)
  {
	  return (pos<count)?pos:last; 
  }
  inline 	value_t mix(value_t pos, float_t beta)
  {
	       
           return mix_distribution2(clamp(pos),beta);
  }

	inline 	value_t fast_mix_distribution(value_t pos,value_t old_cursor, float_t beta)
	{
		//
		const			float_t ci_m=0.5;
          // 	
		//
		//

		//			return update_left(0,pos, beta);
		//		return mix_distribution_only(pos,beta);
				return mix_distribution2(pos,beta);

		value_t mold=median;
		if(pos>mold)
		{
			value_t k=0;
			for(;k<mold;k++)
			{

			}

		}
		else
		{

		}
		


	}

};

template <int _NValue=14,class _FType=PD_base_t<>::float_t,class _ValueType=unsigned char>
struct PD_t: PD_base_t< _NValue,_FType,_ValueType>
{
	PD_t(double l=0.25,double r=0.75):PD_base_t< _NValue,_FType,_ValueType>(){
		const double ds= double(1)/double(count);
		double s=0;
		ci_l=l;
		ci_r=r;
		for(int l=0;l<count;l++) prob[l]=(s+=ds);  		
		//prob[last]=1;

		cursor=update_quantiles();

	}  

inline PD_t& setPD(uint32_t c=last,double l=0.25,double r=0.75)
{
	if(c>last) c=last;
	double s=0;
	ci_l=l;
	ci_r=r;
	for(int l=0;l<c;l++) prob[l]=double(0);  		
	for(int l=c;l<count;l++) prob[l]=double(1); 
	cursor=update_quantiles();

	return *this;
}
 inline PD_base_t< _NValue,_FType,_ValueType> get_random(int ff=0)
 {
    
	
	double r=double(rand())/double(RAND_MAX)*count;
	if(ff)
	{
        cursor=r;
		return *this;
	}
		PD_base_t< _NValue,_FType,_ValueType> t;
	t.ci_l=ci_l;
	t.ci_r=ci_r;
	int ir=r;
	for(int l=0;l<ir;l++) t.prob[l]=0;  			
	for(int l=ir;l<count;l++) t.prob[l]=double(1); 
	t.cursor=t.update_quantiles();
	return t;
 }

};
template <int Ver=0>
struct  filter_npstat_decay_base_t:filter_base_t<Ver>
{
	filter_npstat_decay_base_t(int _nthread, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_base_t(_nthread,_width,_height,_src_byte_width,_dest_byte_width){};

 virtual    filter_npstat_decay_base_t& reset_buffer(double l=25,double r=75,int frandv=0)=0;
 virtual ~filter_npstat_decay_base_t(){};

};

template <int _BitsPerByte=3, int Ver=0>
struct  filter_npstat_decay_t:filter_npstat_decay_base_t<Ver>
{
	enum{
		bipby=_BitsPerByte,
		bpsh=8-bipby,
		npstat_count=(1<<bipby)		

	};

    //	typedef float float_pd_t;
	//
	//
	typedef float_fix_t<14,uint16_t> float_pd_t;
	//	typedef float_fix_t<16,uint32_t> float_pd_t;
		//	typedef float_fix_t<8,uint8_t> float_pd_t;
	
	typedef  PD_base_t<npstat_count,float_pd_t> npstat_base_t;
	typedef  PD_t<npstat_count,float_pd_t> npstat_t;




	npstat_base_t* pbuffer;
	std::vector<npstat_base_t> buffer;
	int buffer_width;
	double alpha;
	int nbyte_mask;
	int maxcolor;
	int testcolor;

	




	inline bool check_mask(int l)
	{
		return (nbyte_mask<0)|((l&3)==nbyte_mask );
	}


	//inline 
		static  void blend_value(const byte_t* src,byte_t* dest,const double a,npstat_base_t* nsp,byte maxcol=255)
	{
        float_pd_t b;
				//
		//__asm {			int 3  		};
		byte_t s=*src;
		if(s>maxcol) s=maxcol;
		b=1-a;
		          //
		//if(nsp->mix_distribution_and_check(s>>bpsh,b))		  *dest=*src;
   //     
		//*dest=nsp->mix_distribution2(s>>bpsh,b)<<bpsh;

		//
		*dest=nsp->fast_mix_distribution(s>>bpsh,(*dest)>>bpsh,b)<<bpsh;
	}


virtual    filter_npstat_decay_base_t& reset_buffer(double l=25,double r=75,int frandv=0)
	{
           npstat_t nps(l/100.,r/100.);
		   //int bso=buffer.size();
		   buffer.resize(buffer_width*height);
		   //int bsn=buffer.size();
		   srand(GetTickCount());
		   if(frandv<0)
		   {
			   uint32_t u=int(-frandv);
                frandv=0;
               nps.setPD(u);
		   }
		   if(frandv) 
		   {
			   frandv=frandv>>1;
			   for(int k=0;k<buffer.size();k++) buffer[k]=nps.get_random(frandv);
		   }
		   else std::fill(buffer.begin(),buffer.end(),nps);
		   pbuffer=buffer.size()?&buffer[0]:0;
		   return *this;
	}

	filter_npstat_decay_t(int _nthread,double _alpha, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_npstat_decay_base_t(_nthread,_width,_height,_src_byte_width,_dest_byte_width)
		,alpha(_alpha),testcolor(0),maxcolor(255),buffer_width(0)
	{
		//if(!src_byte_width) 
		//reset_buffer();
		nbyte_mask=-1;
		
	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		//byte_t* prs=psrc;
		//byte_t* prd=pdest;
		/*
		int nx=pfad->nx,ny=pfad->ny;
		const uint16_t wblock=alpha_dynamic_t::wblock;
		const uint16_t hblock=alpha_dynamic_t::hblock;
		const uint16_t hpow=alpha_dynamic_t::hpow;*/
		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;			
			npstat_base_t *pbuf=pbuffer+n*width;

			byte_t *ps_end=ps+src_byte_width;





			for(int l=0;l<width;l++,ps++,pd++,pbuf++)
				if(check_mask(l))					
					blend_value(ps,pd,alpha,pbuf);

			//	blend_pt(ps,pd,a,b,pbuf);
		}





	};
};



template <int _BitsPerByte=3, int Ver=0>
struct  filter_npstat_decay_dynamic_t:filter_npstat_decay_t<_BitsPerByte,Ver>
{

	typedef filter_alpha_dynamic_base_t<Ver> alpha_dynamic_t;
	typedef typename alpha_dynamic_t::alpha_t alpha_t;
	alpha_dynamic_t* pfad;


	filter_npstat_decay_dynamic_t(int _nthread,filter_alpha_dynamic_base_t<Ver>* _pfad, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_npstat_decay_t(_nthread,0,_width,_height,_src_byte_width,_dest_byte_width),pfad(_pfad)
	{
		buffer_width=_width;
	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		//byte_t* prs=psrc;
		//byte_t* prd=pdest;
		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;			
			npstat_base_t *pbuf=pbuffer+n*width;

			byte_t *ps_end=ps+src_byte_width;

			alpha_t* palpha=pfad->palpha_row(n);
			byte_t maxcol=byte_t(maxcolor);



			if(testcolor)
			{
              for(int l=0;l<width;l++,ps++,pd++)  
				    *pd=((*ps)>>bpsh)<<bpsh;
				  ;

			}
			else 
			for(int l=0;l<width;l++,ps++,pd++,pbuf++)
				if(check_mask(l))					
				{
					double a=pfad->alpha_ref(palpha,l).decay;
					blend_value(ps,pd,a,pbuf,maxcol);
				}
				//	blend_pt(ps,pd,a,b,pbuf);
		}





	};

	static filter_npstat_decay_base_t<Ver>* create(int BitsPerByte,int _nthread,filter_alpha_dynamic_base_t<Ver>* _pfad, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
	{
		switch(BitsPerByte)
		{
		case 1:  return new filter_npstat_decay_dynamic_t<1,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		case 2:  return new filter_npstat_decay_dynamic_t<2,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		case 3:  return new filter_npstat_decay_dynamic_t<3,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		case 4:  return new filter_npstat_decay_dynamic_t<4,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		case 5:  return new filter_npstat_decay_dynamic_t<5,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		case 6:  return new filter_npstat_decay_dynamic_t<6,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		case 7:  return new filter_npstat_decay_dynamic_t<7,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		case 8:  return new filter_npstat_decay_dynamic_t<8,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		}

		return 0;

	}
};


template <int _BitsPerByte=3, int Ver=0>
struct  filter_npstat_decay_dynamic_RGB_t:filter_npstat_decay_t<_BitsPerByte,Ver>
{

	typedef filter_alpha_dynamic_base_t<Ver> alpha_dynamic_t;
	typedef typename alpha_dynamic_t::alpha_t alpha_t;
	alpha_dynamic_t* pfad;


	filter_npstat_decay_dynamic_RGB_t(int _nthread,filter_alpha_dynamic_base_t<Ver>* _pfad, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_npstat_decay_t(_nthread,0,3*_width,_height,_src_byte_width,_dest_byte_width),pfad(_pfad)
	{	
		width=_width;
		height=_height;
		buffer_width=3*_width;
		src_byte_width=(_src_byte_width)?_src_byte_width:4*width;
      	dest_byte_width=(_dest_byte_width)?_dest_byte_width:4*width;


	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		//byte_t* prs=psrc;
		//byte_t* prd=pdest;
		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;			
			npstat_base_t *pbuf=pbuffer+3*n*width;

			byte_t *ps_end=ps+src_byte_width;

			alpha_t* palpha=pfad->palpha_row(n);
			byte_t maxcol=byte_t(maxcolor);

			if(testcolor)
			{
				for(int l=0;l<width;l++,ps+=4,pd+=4)  
				{
				pd[0]=((ps[0])>>bpsh)<<bpsh;
				pd[1]=((ps[1])>>bpsh)<<bpsh;
				pd[2]=((ps[2])>>bpsh)<<bpsh;
				pd[3]=0;
				}
				
				;

			}
			else 
			for(int l=0;l<width;l++,ps+=4,pd+=4,pbuf+=3)				
				{
					double a=pfad->alpha_ref(palpha,l).decay;
					blend_value(ps+0,pd+0,a,pbuf+0,maxcol);
					blend_value(ps+1,pd+1,a,pbuf+1,maxcol);
					blend_value(ps+2,pd+2,a,pbuf+2,maxcol);
					pd[3]=0;
					
				}
				//	blend_pt(ps,pd,a,b,pbuf);
		}





	};

  static filter_npstat_decay_base_t<Ver>* create(int BitsPerByte,int _nthread,filter_alpha_dynamic_base_t<Ver>* _pfad, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
  {
     switch(BitsPerByte)
	 {
		 case 1:  return new filter_npstat_decay_dynamic_RGB_t<1,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
	     case 2:  return new filter_npstat_decay_dynamic_RGB_t<2,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		 case 3:  return new filter_npstat_decay_dynamic_RGB_t<3,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		 case 4:  return new filter_npstat_decay_dynamic_RGB_t<4,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
    	 case 5:  return new filter_npstat_decay_dynamic_RGB_t<5,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		 case 6:  return new filter_npstat_decay_dynamic_RGB_t<6,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		 case 7:  return new filter_npstat_decay_dynamic_RGB_t<7,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		 case 8:  return new filter_npstat_decay_dynamic_RGB_t<8,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
	 }

	  return 0;

  }
};
