#pragma once
//#include "filters3d.h"

#include "filters2d_base.h"
#include "fast_median2d.h"
#include <math.h>


struct dynamic_t
{
	double decay;
	double gamma;
	double co_gamma;
	double radius;
	int wbp;
	int hbp;

};


template <int Ver=0>
struct  filter_alpha_dynamic_base_t:filter_base_t<Ver>
{
	struct alpha_t
	{
		uint32_t alpha;
		uint32_t alpha_old;
		uint32_t alpha_inner;
		uint32_t alpha_inner_old;
		uint32_t decay_num;	
		double decay;

	};


	struct alpha_ptr_t
	{
		filter_alpha_dynamic_base_t* owner;
		alpha_t* p;
	
		alpha_ptr_t(int n,filter_alpha_dynamic_base_t* o):owner(o)
		{
            p=o->palpha_row(n);  
		}	

	  inline 	alpha_t& operator [](int m)
	  {
		   return owner->alpha_ref(p,m);
	  }		

	};



	alpha_t* palpha;
	int pow_w,pow_h;

	int nx,ny;


	double gamma,co_gamma;
	double decay;

	double alpha_transp;
	frgba_t rgb;

	filter_alpha_dynamic_base_t(int wpow,int hpow,int _nthread,float _gamma,float _decay,int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_base_t(_nthread,_width,_height,_src_byte_width,_dest_byte_width)
		,pow_w(wpow),pow_h(hpow)
		,gamma(_gamma),decay(_decay),palpha(0),co_gamma(1)
	{


	}


	inline alpha_t* palpha_row(int n)
	{ 
		return palpha+(n>>pow_h)*nx;

	}

	inline alpha_t& alpha_ref(alpha_t* p,int m)
	{
		return *(p+(m>>pow_w));
	}
    
	



};


template <int _WPow=4,int _HPow=4,int Ver=0>
struct  filter_alpha_dynamic_t:filter_alpha_dynamic_base_t<Ver>
{
	enum{
		hpow=_HPow,
		wpow=_WPow,
		wblock=1<<wpow,
		hblock=1<<hpow,
		offsB=4
	};

	
	//int nx,ny;
	
	
	//double gamma;
	//double decay;

	std::vector<alpha_t> buffer;
	//alpha_t* palpha;

	uint32_t decay_num,gamma_num;	         
	uint32_t decay_num_max;
	


	filter_alpha_dynamic_t(int _nthread,float _gamma,float _decay,int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_alpha_dynamic_base_t<Ver>(wpow,hpow,1,_gamma,_decay,_width,_height,_src_byte_width,_dest_byte_width)
		//:filter_base_t(1,_width,_height,_src_byte_width,_dest_byte_width)
		//,gamma(_gamma),decay(_decay),palpha(0)
	{


		src_byte_width=(_src_byte_width)?_src_byte_width:offsB*width;
		dest_byte_width=(_dest_byte_width)?_dest_byte_width:width;


		{
			frgba_t  t={1./3.,1./3.,1./3.};
			rgb=t;
		}


         nx=width/wblock;
		 ny=height/hblock;

		 width=nx*wblock;
		 height=ny*hblock;
		 decay_num=alpha_denom*decay;	         
		 gamma_num=alpha_denom*gamma;	

		 alpha_transp=1;

		 {
         
		 //
			 alpha_t a={};
			 a.decay_num=decay_num;
			 a.decay=decay;
		 //
			 //alpha_t a={0,0,0,0.5};
			 //alpha_t a={0,0,0,0.1};
			 //alpha_t a={0,0,0,1};
		 //std::vector<alpha_t> t(nx*ny,a);
         //buffer=t;
			 buffer.resize(nx*ny,a);
		 palpha=&buffer[0];
		 }
		 
	}

         void blend_decay(double& d)
		 {
			 //d=gamma*decay+(1-gamma)*d;
			 d=gamma*(decay-d)+d;
		 }
		 void co_blend_decay(double& d)
		 {
			 //d=gamma*decay+(1-gamma)*d;
			 if(co_gamma>=1) d=1;
			 else  d=co_gamma*(1-d)+d;
		 }

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		//byte_t* prs=psrc;
		//byte_t* prd=pdest;
if(0){
		for(int y=0;y<ny;y++)
		{
			

			for(int x=0;x<nx;x++)
			{
				alpha_t* pa=palpha+y*nx+x;
             //alpha_t& at=pa[x];
			  if ((x<(nx/2))&&(y<(ny/2))) 
				 pa->alpha=1;
			}
		}
     }   	else{

		  decay_num_max=0;
		for(int y=0;y<ny;y++)
		{
			
            byte_t* ps=psrc+(y<<hpow)*src_byte_width;
			byte_t* ps_u=(y)?psrc+((y-1)<<hpow)*src_byte_width:ps;

			alpha_t* pa=palpha+y*nx;

			
              int nx1=nx-1;
			for(int x=0;x<nx;x++)
			{
				
				byte_t* psr=ps+(x<<wpow)*offsB;
				
				byte_t* psr_u=(x)?psr-(1<<wpow)*offsB:psr;
				byte_t* psr_d=(x<nx1)?psr+(1<<wpow)*offsB:psr;
				
				



				/*

				byte_t* psr_l=(x)?ps+((x-1)<<wpow)*offsB:psr;

				byte_t* psr_u=ps_u+(x<<wpow)*offsB;
				byte_t* psr_ul=(x)?ps_u+((x-1)<<wpow)*offsB:psr_u;
				*/
				
				

				
				

				alpha_t& at=pa[x];

				uint32_t alphaold=at.alpha,alpha=0;

                     const int hblock1= hblock-1;
				for(int n=0;n<hblock;n++)
				{		   
					   int offset=n*src_byte_width+3;
					byte_t* p=psr+offset;	
					byte_t* pu=psr_u+offset;	
					byte_t* pd=psr_d+offset;	

					byte_t* pl=(n)?p-src_byte_width:p;
					byte_t* pr=(n<hblock1)?p+src_byte_width:p;

					byte_t* plu=(n)?pu-src_byte_width:pu;
					byte_t* pru=(n<hblock1)?pu+src_byte_width:pu;

					byte_t* pld=(n)?pd-src_byte_width:pd;
					byte_t* prd=(n<hblock1)?pd+src_byte_width:pd;



					/*
					byte_t* pl=psr_l+offset;
					byte_t* pu=psr_u+offset;
					byte_t* pul=psr_ul+offset;
					*/
					
					

					//for(int m=0;m<wblock;m+=offsB)	
						//if(alpha|=(p[m]|pl[m]|pr[m]|pu[m]|pd[m]|plu[m]|pru[m]|pld[m]|prd[m])) goto block_calc;

					for(int m=0;m<wblock;m+=offsB)	
					if((alpha|=p[m])) goto block_calc;
					                              
				}

block_calc:

/*
           byte_t* pb=ps+3;

               for(int n=0;n<hblock;n++)
			   {		   
                    for(byte_t* p=pb;p<pb+wblock;p++)
						alpha+=*p;
					pb+=src_byte_width;                                      
			   }
*/

			   at.alpha=alpha;
			   at.alpha_old=alpha;
			   ///*
			   if(alpha)
			   {
				   //at.decay_num=alpha_denom;
				   at.decay=alpha_transp;
			   }
			   else{
				   if(alphaold)
				   {
					   //at.decay_num=0*alpha_denom;
					   //at.decay=1;
					   co_blend_decay(at.decay);

				   }
				   else	blend_decay(at.decay);



			   }

			   at.decay_num=alpha_denom*at.decay;	         

			   //*/

			   //blend_decay(at.decay_num);                     
			   //at.decay_num=decay_num;

			   if(decay_num_max<at.decay_num) decay_num_max=at.decay_num;
			   //if ((x<(nx/2))&&(y<(ny/2))) 				   at.alpha=1;
			   //if (x>10) at.decay_num=alpha_denom;

			}

		}

		};//ifff
		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;

			for(int m=0;m<width;m++,ps+=offsB)
			{
				uint16_t r=rgb.b*uint16_t(ps[0])+rgb.g*uint16_t(ps[1])+rgb.r*uint16_t(ps[2]);
				pd[m]=(r<251)?r:250;
				//pd[m]=250;

			}
		}
	}

/*
	inline alpha_t* palpha_row(int n)
	{ 
		   return palpha+(n>>hpow)*nx;

	}

	static inline alpha_t& alpha_ref(alpha_t* p,int m)
	{
		 return *(p+(m>>wpow));
	}
*/

};


template <int _HPow,int Ver>
struct factory_filter_alpha_dynamic_WPow_t
{

	static filter_alpha_dynamic_base_t<Ver>* create(int WPow,int nthread,float gamma,float decay,int width,int height,int src_byte_width=0,int dest_byte_width=0)
	{
		switch(WPow)
		{
  		case 0:	return new filter_alpha_dynamic_t<0,_HPow,Ver>(nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 1:	return new filter_alpha_dynamic_t<1,_HPow,Ver>(nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 2:	return new filter_alpha_dynamic_t<2,_HPow,Ver>(nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 3:	return new filter_alpha_dynamic_t<3,_HPow,Ver>(nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 4:	return new filter_alpha_dynamic_t<4,_HPow,Ver>(nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 5:	return new filter_alpha_dynamic_t<5,_HPow,Ver>(nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 6:	return new filter_alpha_dynamic_t<6,_HPow,Ver>(nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 7:	return new filter_alpha_dynamic_t<7,_HPow,Ver>(nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
    	case 8:	return new filter_alpha_dynamic_t<8,_HPow,Ver>(nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 9:	return new filter_alpha_dynamic_t<9,_HPow,Ver>(nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		
		}

		return 0;

	}

};

template <int Ver=0>
struct factory_filter_alpha_dynamic_t
{

	static filter_alpha_dynamic_base_t<Ver>* create(int WPow,int HPow,int nthread,float gamma,float decay,int width,int height,int src_byte_width=0,int dest_byte_width=0)
	{
		switch(HPow)
		{
		case 0:	return factory_filter_alpha_dynamic_WPow_t<0,Ver>::create(WPow,nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 1:	return factory_filter_alpha_dynamic_WPow_t<1,Ver>::create(WPow,nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 2:	return factory_filter_alpha_dynamic_WPow_t<2,Ver>::create(WPow,nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 3:	return factory_filter_alpha_dynamic_WPow_t<3,Ver>::create(WPow,nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 4:	return factory_filter_alpha_dynamic_WPow_t<4,Ver>::create(WPow,nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 5:	return factory_filter_alpha_dynamic_WPow_t<5,Ver>::create(WPow,nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 6:	return factory_filter_alpha_dynamic_WPow_t<6,Ver>::create(WPow,nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 7:	return factory_filter_alpha_dynamic_WPow_t<7,Ver>::create(WPow,nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
		case 8:	return factory_filter_alpha_dynamic_WPow_t<8,Ver>::create(WPow,nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);
    	case 9:	return factory_filter_alpha_dynamic_WPow_t<9,Ver>::create(WPow,nthread,gamma,decay,width,height,src_byte_width,dest_byte_width);

		}

		return 0;

	}

};




template <int Ver=0>
struct  filter_decay_dynamic_t:filter_base_t<Ver>
{

	typedef filter_alpha_dynamic_base_t<Ver> alpha_dynamic_t;
	typedef typename alpha_dynamic_t::alpha_t alpha_t;

	uint32_t* pbuffer;
	std::vector<uint32_t> buffer;



	alpha_dynamic_t* pfad;

	
	
	filter_decay_dynamic_t(int _nthread,filter_alpha_dynamic_base_t<Ver>* _pfad, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_base_t(_nthread,_width,_height,_src_byte_width,_dest_byte_width),pfad(_pfad)
	{
		//if(!src_byte_width) 
		buffer.resize(width*height);
		pbuffer=buffer.size()?&buffer[0]:0;


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
			uint32_t *pbuf=pbuffer+n*width;

			byte_t *ps_end=ps+src_byte_width;

			alpha_t* palpha=pfad->palpha_row(n);



			for(int l=0;l<width;l++,ps++,pd++,pbuf++)
			{
				uint32_t a=pfad->alpha_ref(palpha,l).decay_num;

				blend_byte(*ps,*pd,a,*pbuf);
			}
			//	blend_pt(ps,pd,a,b,pbuf);
		}

		
		


	};




};

template <int Ver=0>
struct  filter_decay_dynamic_test_t:filter_base_t<Ver>
{
	

	typedef filter_alpha_dynamic_base_t<Ver> alpha_dynamic_t;
	typedef typename alpha_dynamic_t::alpha_t alpha_t;

	uint32_t* pbuffer;
	std::vector<uint32_t> buffer;



	alpha_dynamic_t* pfad;

	
	
	filter_decay_dynamic_test_t(int _nthread,filter_alpha_dynamic_base_t<Ver>* _pfad, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_base_t(_nthread,_width,_height,_src_byte_width,_dest_byte_width),pfad(_pfad)
	{
		//if(!src_byte_width) 
		buffer.resize(width*height);
		pbuffer=buffer.size()?&buffer[0]:0;
		dest_byte_width=width*4;


	}

	double logbase(double a, double base=2)
	{
		return log(a) / log(base);
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
			uint32_t *pbuf=pbuffer+n*width;

			byte_t *ps_end=ps+src_byte_width;

			//alpha_t* palpha=pfad->palpha+(n/hblock)*pfad->nx;
			alpha_t* palpha=pfad->palpha_row(n);

			double dec0=pfad->decay;



			for(int l=0;l<width;l++,ps++,pd+=4,pbuf++)
			{

				alpha_t& alp=pfad->alpha_ref(palpha,l);
				
					double dmin,dmax,d,l2d;
				    
					d=alp.decay;
				 

                      //
					l2d=logbase(d+1,2);
				//alpha_t* pp=palpha+l/wblock;
				//a=pp->alpha;

				//a=(255*a)/alpha_denom;
				     pd[0]=0;    
    	  			 pd[1]=255;
	    			 pd[2]=0;


				 if(d>dec0+0.0001)
				 {
		             pd[2]=0;    
    	  			 pd[1]=0;
	    			 pd[0]=64+(125+64)*(1-d)/(1-dec0);
				 }
				 else if(d<dec0-0.0001)
				 {
					 pd[0]=0;    
					 pd[1]=0;
					 pd[2]=255*d/dec0;
				 }

				 


				 /*
				 if( pfad->decay_num_max)
				 {
					 aa=255*double(d)/double( pfad->decay_num_max);
				 }
				 */




				 
                //pd[0]=0*128;    
				//pd[1]=0*((l>(width/2))&&(n>(height/2)))?128:0;
				//pd[2]=aa;
				
			}
			//	blend_pt(ps,pd,a,b,pbuf);
		}

		
		


	};
};

template <int Ver=0>
struct  filter_decay_t:filter_base_t<Ver>
{

	uint32_t alpha_num;
	//int dw_byte_width;
	uint32_t* pbuffer;
	std::vector<uint32_t> buffer;


	filter_decay_t(int nthread,int _width,int _height,double alpha=0.5,int _src_byte_width=0)
		:filter_base_t(nthread,_width,_height,_src_byte_width){
			alpha_num=alpha_denom*alpha;			 
			buffer.resize(width*height);
			pbuffer=buffer.size()?&buffer[0]:0;

	};


	inline void blend_pt(const byte_t* pline_src,byte_t* pline_dest,uint32_t a,uint32_t b,uint32_t* pdwdest)
	{  
		uint32_t v;
		v= (b*(*pdwdest))>>alpha_denom_dig;
		v= v+a*(*pline_src);
		*pdwdest=v; 
		*pline_dest=v>>alpha_denom_dig;

	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){


		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;			
			uint32_t *pbuf=pbuffer+n*width;

			byte_t *ps_end=ps+src_byte_width;

			uint32_t a=alpha_num;
			uint32_t b=alpha_denom-alpha_num;



			for(;ps<ps_end;ps++,pd++,pbuf++)
				blend_byte(*ps,*pd,a,*pbuf);
				//blend_pt(ps,pd,a,b,pbuf);
				//
				//
				


		}

	};
};

template < template< int _WPow,int _HPow > class F >
struct Filter3DFactory
{

};