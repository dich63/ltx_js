#pragma once
//#include "filters2d.h"

#include "filters2d_base.h"
#include "fast_median2d.h"

template <int Ver=0>
struct  filter_alpha_dynamic_base_t:filter_base_t<Ver>
{
	struct alpha_t
	{
		uint32_t alpha;
		uint32_t alpha_old;
		uint32_t decay_num;	
	};
alpha_t* palpha;
int pow_w,pow_h;

int nx,ny;


double gamma;
double decay;
uint32_t decay_num,gamma_num;	         
uint32_t decay_num_max;

filter_alpha_dynamic_base_t(int wpow,int hpow,int _nthread,float _gamma,float _decay,int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
:filter_base_t(1,_width,_height,_src_byte_width,_dest_byte_width)
,pow_w(wpow),pow_h(hpow)
,gamma(_gamma),decay(_decay),palpha(0)
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

    std::vector<alpha_t> buffer;
	frgba_t rgb;
	


	filter_alpha_dynamic_t(int _nthread,float _gamma,float _decay,int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_alpha_dynamic_base_t(wpow,hpow, _nthread, _gamma, _decay,_width,_height,_src_byte_width,_dest_byte_width)
		
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


		 {
         
		 //
			 alpha_t a={1,0,decay_num};
		 //alpha_t a={0,0,alpha_denom};
		 std::vector<alpha_t> t(nx*ny,a);
         buffer=t;
		 palpha=&buffer[0];
		 }
		 
	}

         void blend_decay(uint32_t& d)
		 {
			 d=uint32_t(gamma*double(decay_num)+(1-gamma)*double(d));
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

			alpha_t* pa=palpha+y*nx;

			

			for(int x=0;x<nx;x++)
			{
				byte_t* psr=ps+(x<<wpow)*offsB;
				alpha_t& at=pa[x];

				uint32_t alphaold=at.alpha,alpha=0;


				for(int n=0;n<hblock;n++)
				{		   
					byte_t* p=psr+n*src_byte_width+3;
					for(int m=0;m<wblock;m+=offsB)	
						alpha+=p[m];
					                              
				}


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
			   if(alpha)
			   {
				   at.decay_num=0*alpha_denom;
			   }
			   else{
				   if(alphaold)
				   {
					   at.decay_num=alpha_denom;

				   }
				   else	blend_decay(at.decay_num);                     

			   }
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
				pd[m]=r;

			}
		}
	}





};


template <int _WPow=4,int _HPow=4,int Ver=0>
struct  filter_decay_dynamic_t:filter_base_t<Ver>
{

	typedef filter_alpha_dynamic_t<_WPow,_HPow,Ver> alpha_dynamic_t;
	typedef typename alpha_dynamic_t::alpha_t alpha_t;

	uint32_t* pbuffer;
	std::vector<uint32_t> buffer;



	alpha_dynamic_t* pfad;

	
	
	filter_decay_dynamic_t(int _nthread,filter_alpha_dynamic_t<_WPow,_HPow,Ver>* _pfad, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
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
			int pow=pfad->wpow;




			for(int l=0;l<width;l++,ps++,pd++,pbuf++)
			{
				uint32_t a=pfad->alpha_ref(palpha,l).decay_num;

				blend_byte(*ps,*pd,a,*pbuf);
			}
			//	blend_pt(ps,pd,a,b,pbuf);
		}

		
		


	};
};

template <int _WPow=4,int _HPow=4,int Ver=0>
struct  filter_decay_dynamic_test_t:filter_base_t<Ver>
{
	enum{
		hpow=_HPow,
		wpow=_WPow,
		wblock=1<<wpow,
		hblock=1<<hpow,
		offsB=4
	};

	typedef filter_alpha_dynamic_t<_WPow,_HPow,Ver> alpha_dynamic_t;
	typedef typename alpha_dynamic_t::alpha_t alpha_t;

	uint32_t* pbuffer;
	std::vector<uint32_t> buffer;



	alpha_dynamic_t* pfad;

	
	
	filter_decay_dynamic_test_t(int _nthread,filter_alpha_dynamic_t<_WPow,_HPow,Ver>* _pfad, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_base_t(_nthread,_width,_height,_src_byte_width,_dest_byte_width),pfad(_pfad)
	{
		//if(!src_byte_width) 
		buffer.resize(width*height);
		pbuffer=buffer.size()?&buffer[0]:0;
		dest_byte_width=width*4;


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

			



			for(int l=0;l<width;l++,ps++,pd+=4,pbuf++)
			{

				alpha_t& alp=pfad->alpha_ref(palpha,l);
				
					double dmin,dmax,d;
				 d=alp.decay_num;
				 dmin=pfad->decay_num;
				 dmax=alpha_denom;


				//alpha_t* pp=palpha+l/wblock;
				//a=pp->alpha;

				//a=(255*a)/alpha_denom;
				 byte_t aa=0;
				 if(d>=dmin)
				 {
				  aa=128*(d-dmin)/(dmax-dmin)+64;
				 }
				 else 
				 {
					 d=64*(d/dmin);
				 }
				 /*
				 if( pfad->decay_num_max)
				 {
					 aa=255*double(d)/double( pfad->decay_num_max);
				 }
				 */




				 pd[0]=64;    
				 pd[1]=0*(alp.alpha)?64:0;//*((l>(width/2))&&(n>(height/2)))?128:0;
				 pd[2]=aa;
				 
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
