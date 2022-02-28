#pragma once
#include <utility>
#include "morton_hilbert.h"

#define clip3(x, y, z) ((z < x) ? x : ((z > y) ? y : z))

#pragma pack(push)
#pragma pack(1)

struct byte3_t
{
	typedef unsigned char byte_t;
	byte_t r,g,b;
	
};
template <class N,int SZ> 
struct block2D_t{
	N v[SZ][SZ];
};


template <class N> 
struct triplet_t
{
	union{
		struct{ N r,g,b; };
		struct{ N y,u,v; };
		struct{ N c[3]; };
	};
};
#pragma pack(pop)
template <int BPP_Y=8,int BPP_UV=8>
struct yuv_rgb_t
{
	typedef unsigned char byte_t;
	typedef signed char int8_t;
	typedef int int32_t;
	typedef unsigned int uint32_t;
	typedef unsigned short uint16_t;

	typedef  triplet_t<int32_t>  triplet_int32_t;
	typedef  triplet_t<byte_t>  triplet_byte_t;

	enum
	{
		bpp_y=BPP_Y,
        bpp_uv=BPP_UV,
		add_y=8-bpp_y,
		add_uv=8-bpp_uv,		
		pow_hi=0x10,
		pow_hi4=pow_hi+2,
		mul_hi=1<<pow_hi,				
		mul_hi4=1<<pow_hi4,				
		mask_round=1<<(pow_hi-1),
		yr=int(0.256788 *mul_hi),
		yg=int(0.504129 *mul_hi),
		yb=int(0.097906 *mul_hi),

		ur=int(-0.148223 *mul_hi),
		ug=int(-0.290993 *mul_hi),
		ub=int( 0.439216 *mul_hi),

		vr=int(0.439216 *mul_hi),
		vg=int(-0.367788 *mul_hi),
		vb=int(-0.071427*mul_hi),


		ry=int(1.16438242102687 *mul_hi),
		ru=int(0 *mul_hi),
		rv=int( 1.59602644096888*mul_hi)
		,
		gy=int(1.16438522602142 *mul_hi),
		gu=int( -0.391761962130446*mul_hi),
		gv=int(  -0.812967393565921*mul_hi)
		,
		by=int(1.16438427941495 *mul_hi),
		bu=int(2.01723023246196 *mul_hi),
		bv=int( 0*mul_hi)

	};


    

   inline static void  rgb_to_yuv(triplet_byte_t* rgb,triplet_byte_t* yuv){
	   

	   
	         const int32_t off_y=16*mul_hi,off_uv=128*mul_hi;
             int32_t r=rgb->r;
			 int32_t g=rgb->g;
			 int32_t b=rgb->b;

			 int32_t y=yr*r+yg*g+yb*b+off_y;
			 int32_t u=ur*r+ug*g+ub*b+off_uv;
			 int32_t v=vr*r+vg*g+vb*b+off_uv;
			 yuv->y=uint32_t(y)>>(pow_hi+add_y);
			 yuv->u=uint32_t(u)>>(pow_hi+add_uv);
			 yuv->v=uint32_t(v)>>(pow_hi+add_uv);			 
			 
   };


   
   inline static uint32_t  rgb4_to_uv_pack(int32_t r,int32_t g,int32_t b){


	   const int32_t off_uv=128*mul_hi;
	   
	   
	   int32_t u=((ur*r+ug*g+ub*b)>>2)+off_uv;
	   int32_t v=((vr*r+vg*g+vb*b)>>2)+off_uv;
	   
	   uint32_t cu=uint32_t(u)>>(pow_hi+add_uv);
	   uint32_t cv=uint32_t(v)>>(pow_hi+add_uv);

	   return (cu<<(32-bpp_uv))|(cv<<(32-2*bpp_uv));
	   
   };

   template <class T>
   inline static int32_t  rgb_to_y(triplet_t<T>* rgb){


	   
	   int32_t r=rgb->r;
	   int32_t g=rgb->g;
	   int32_t b=rgb->b;

	   int32_t y=yr*r+yg*g+yb*b;
	   return y;

   };



   inline static uint32_t  rgb_to_y_pack(triplet_byte_t* rgb){


	   const int32_t off_y=16*mul_hi;
	   int32_t r=rgb->r;
	   int32_t g=rgb->g;
	   int32_t b=rgb->b;

	   int32_t y=yr*r+yg*g+yb*b+off_y;
	   return uint32_t(y)>>(pow_hi+add_y);

   };






   inline static void  yuv_to_rgb(triplet_byte_t*yuv,triplet_byte_t* rgb ){
               yuv_to_rgb(yuv->y,yuv->u,yuv->v,rgb );
   }
   inline static void  yuv_to_rgb(byte_t y8,byte_t u8,byte_t v8,triplet_byte_t* rgb ){
	   const int32_t off_y=-16,off_uv=-128;
	   int32_t y=uint32_t(y8<<add_y)+off_y;
	   int32_t u=uint32_t(u8<<add_uv)+off_uv;
	   int32_t v=uint32_t(v8<<add_uv)+off_uv;

       int32_t r=ry*y+ru*u+rv*v;
	   int32_t g=gy*y+gu*u+gv*v;
	   int32_t b=by*y+bu*u+bv*v;  

	   r=clip3(0,255*mul_hi,r);
	   g=clip3(0,255*mul_hi,g);
	   b=clip3(0,255*mul_hi,b);

	   rgb->r=uint32_t(r)>>pow_hi;
	   rgb->g=uint32_t(g)>>pow_hi;
	   rgb->b=uint32_t(b)>>pow_hi;  
	   
   };

   inline static void  yuv_to_rgb_pure(int32_t y,int32_t u,int32_t v,triplet_byte_t* rgb ){	   

	   int32_t r=ry*y+ru*u+rv*v;
	   int32_t g=gy*y+gu*u+gv*v;
	   int32_t b=by*y+bu*u+bv*v;  

	   r=clip3(0,255*mul_hi,r);
	   g=clip3(0,255*mul_hi,g);
	   b=clip3(0,255*mul_hi,b);

	   rgb->r=uint32_t(r)>>pow_hi;
	   rgb->g=uint32_t(g)>>pow_hi;
	   rgb->b=uint32_t(b)>>pow_hi;  

   };


};

template <int BPP_Y=5,int BPP_UV=6>
struct yuv_packer_4526_t
{
	typedef unsigned char byte_t;
	typedef signed char int8_t;
	typedef int int32_t;
	typedef unsigned int uint32_t;
	typedef unsigned short uint16_t;

	typedef  triplet_t<int32_t>  triplet_int32_t;
	typedef  triplet_t<byte_t>  triplet_byte_t;
	enum
	{
		bpp_y=BPP_Y,
		bpp_uv=BPP_UV
	};

	typedef yuv_rgb_t<BPP_Y,BPP_UV> yuvrgb_t;


	template <int OFF>
	static inline byte_t get_y(uint32_t u)
	{
           return byte_t(u>>(OFF*bpp_y))&((1<<bpp_y)-1);
	}

	template <int OFF0,int OFF1>
	static inline byte_t get_y(uint32_t u)
	{
		return byte_t(u>>((2*OFF1+OFF0)*bpp_y))&((1<<bpp_y)-1);
	}
//(cu<<(32-bpp_uv))|(cv<<(32-2*bpp_uv));
	static inline byte_t get_u(uint32_t u)
	{
		return byte_t(u>>(32-bpp_uv));
	}

	static inline byte_t get_v(uint32_t u)
	{
		return byte_t(u>>(32-2*bpp_uv))&((1<<bpp_uv)-1);
	}

	inline static uint32_t  uv_pack(uint32_t u,uint32_t v){
		return (u<<(32-bpp_uv))|(v<<(32-2*bpp_uv));
	}

	inline static uint32_t  y_pack(uint32_t y00,uint32_t y01,uint32_t y10,uint32_t y11){
		
		return (y00)|(y01<<(1*bpp_y))|(y10<<(2*bpp_y))|(y11<<(3*bpp_y));

	}


	template <class N>
	inline	static uint32_t rgb4_to_yuv_pack(N* p00,N* p01,N* p10,N* p11)
	{
		return rgb4_to_yuv_pack((triplet_byte_t*)p00, (triplet_byte_t*)p01,(triplet_byte_t*)p10,(triplet_byte_t*)p11);
	}
	inline	static uint32_t rgb4_to_yuv_pack(triplet_byte_t* p00,triplet_byte_t* p01,triplet_byte_t* p10,triplet_byte_t* p11)
	{
		uint32_t rs=p00->r+p01->r+p10->r+p11->r;
		uint32_t gs=p00->g+p01->g+p10->g+p11->g;
		uint32_t bs=p00->b+p01->b+p10->b+p11->b;
		uint32_t yuv;

			yuv=yuvrgb_t::rgb4_to_uv_pack(rs,gs,bs);
			

		uint32_t u00=yuvrgb_t::rgb_to_y_pack(p00);
		uint32_t u01=yuvrgb_t::rgb_to_y_pack(p01);
		uint32_t u10=yuvrgb_t::rgb_to_y_pack(p10);
		uint32_t u11=yuvrgb_t::rgb_to_y_pack(p11);
		
        
       uint32_t res=yuv|(u00)|(u01<<(1*bpp_y))|(u10<<(2*bpp_y))|(u11<<(3*bpp_y));
	   return res;
		

	}

	template <class N>
	inline	static void yuv_to_rgb4_unpack(uint32_t yuvpack,N* p00,N* p01,N* p10,N* p11)
	{
		return yuv_to_rgb4_unpack( yuvpack,(triplet_byte_t*)p00, (triplet_byte_t*)p01,(triplet_byte_t*)p10,(triplet_byte_t*)p11);
	}

	inline	static void yuv_to_rgb4_unpack(uint32_t yuvpack, triplet_byte_t* p00,triplet_byte_t* p01,triplet_byte_t* p10,triplet_byte_t* p11)
	{
		byte_t u=yuvpack>>(32-bpp_uv);
		byte_t v=(yuvpack>>(32-2*bpp_uv))&63;

		byte_t y00=(yuvpack>>(0*bpp_y))&31;
		byte_t y01=(yuvpack>>(1*bpp_y))&31;
		byte_t y10=(yuvpack>>(2*bpp_y))&31;
		byte_t y11=(yuvpack>>(3*bpp_y))&31;
		yuv_rgb_t<bpp_y,bpp_uv>::yuv_to_rgb(y00,u,v,p00);

		/*yuv_rgb_t<bpp_y,bpp_uv>::yuv_to_rgb(y00,u,v,p01);
		yuv_rgb_t<bpp_y,bpp_uv>::yuv_to_rgb(y00,u,v,p10);
		yuv_rgb_t<bpp_y,bpp_uv>::yuv_to_rgb(y00,u,v,p11);
		*/

		
		yuv_rgb_t<bpp_y,bpp_uv>::yuv_to_rgb(y01,u,v,p01);
		yuv_rgb_t<bpp_y,bpp_uv>::yuv_to_rgb(y10,u,v,p10);
		yuv_rgb_t<bpp_y,bpp_uv>::yuv_to_rgb(y11,u,v,p11);
		

	}





};


template <class yuv_packer_T, int bpp_RGBA>
long convert_RGB_to_YUV32_bits(int width,int height,int rowlength,void* pRGBAbits,void* pyuvbits)
{
	typedef unsigned char byte_t;
    typedef  byte_t* pbyte_t;
	typedef unsigned int uint32_t;

    const int offset_s=bpp_RGBA/8,offset_d=4,offset_s2=offset_s*2;

	int w2=width/2,h2=height/2;
	int offset_row=rowlength*2;
//	int byte_width_d=sizeof(uint32_t)*w2;

     pbyte_t  ps0=pbyte_t(pRGBAbits);
	 pbyte_t  ps1=ps0+offset_s*width;
	 uint32_t*  pd=(uint32_t*)(pyuvbits);
	 

	 

	for(int n=0;n<h2;n++)
	{
		pbyte_t  ps00=ps0;
		pbyte_t  ps10=ps1;
				
		for(int m=0;m<w2;m++)
		{
           uint32_t yuv=yuv_packer_T::rgb4_to_yuv_pack(ps00,ps00+offset_s,ps10,ps10+offset_s); 
            pd[m]=yuv;
			ps00+=offset_s2;
			ps10+=offset_s2;
		}
		ps0+=offset_row;
		ps1+=offset_row;
		pd+=w2;
	}


	return 0;
}


template <class yuv_packer_T>
inline long convert_RGB_to_YUV32_bits(int bpp_RGBA, int width,int height,int rowlength,void* pRGBAbits,void* pyuvbits)
{
	if (bpp_RGBA==32) return convert_RGB_to_YUV32_bits<yuv_packer_T,32>( width, height,rowlength, pRGBAbits, pyuvbits);
	else if (bpp_RGBA==24) return convert_RGB_to_YUV32_bits<yuv_packer_T,24>( width, height,rowlength, pRGBAbits, pyuvbits);
	    return -1;
}



template <class yuv_packer_T, int bpp_RGBA>
long convert_YUV32_to_RGB_bits(int width,int height,void* pyuvbits,int rowlength,void* pRGBAbits)
{
	typedef unsigned char byte_t;
	typedef  byte_t* pbyte_t;
	typedef unsigned int uint32_t;

	const int offset_d=bpp_RGBA/8,offset_d2=offset_d*2;

	int w2=width,h2=height;
	int offset_row=rowlength*2;	
	

	pbyte_t  pd0=pbyte_t(pRGBAbits);
	pbyte_t  pd1=pd0+rowlength;
	uint32_t*  ps=(uint32_t*)(pyuvbits);




	for(int n=0;n<h2;n++)
	{
		pbyte_t  pd00=pd0;
		pbyte_t  pd10=pd1;

		for(int m=0;m<w2;m++)
		{
			uint32_t yuv=ps[m];
				yuv_packer_T::yuv_to_rgb4_unpack(yuv,pd00,pd00+offset_d,pd10,pd10+offset_d); 
			;
			pd00+=offset_d2;
			pd10+=offset_d2;
		}
		pd0+=offset_row;
		pd1+=offset_row;
		ps+=w2;
	}


	return 0;
}

template <class yuv_packer_T>
inline long convert_YUV32_to_RGB_bits(int width,int height,void*pyuvbits,int bpp_RGBA,int rowlength, void*  pRGBAbits)
{
	if (bpp_RGBA==32) return convert_YUV32_to_RGB_bits<yuv_packer_T,32>( width, height, pyuvbits, rowlength,pRGBAbits);
	else if (bpp_RGBA==24) return convert_YUV32_to_RGB_bits<yuv_packer_T,24>( width, height, pyuvbits, rowlength,pRGBAbits);
	return -1;
}


template <class N>
struct yuv_equ_default_t
{
	inline static bool isdelta(const N v1,const N v2)
	{
        return v1!=v2;
	}
};


struct yuv_equ_ye_t
{
	typedef yuv_packer_4526_t<> yuvrgb_t;
	typedef  yuvrgb_t::uint32_t uint32_t;

	inline static bool isdelta(const uint32_t v1,const uint32_t v2)
	{

		uint32_t e1=uint32_t(yuvrgb_t::get_y<0>(v1))+uint32_t(yuvrgb_t::get_y<1>(v1))+uint32_t(yuvrgb_t::get_y<2>(v1))+uint32_t(yuvrgb_t::get_y<3>(v1));
		uint32_t e2=uint32_t(yuvrgb_t::get_y<0>(v2))+uint32_t(yuvrgb_t::get_y<1>(v2))+uint32_t(yuvrgb_t::get_y<2>(v2))+uint32_t(yuvrgb_t::get_y<3>(v2));
		return e1!=e2;
	}
};


template <class Pt=uint32_t,class EQUIV=yuv_equ_default_t<Pt> >
struct yuv_comparator_t
{
	typedef unsigned char byte_t;
	typedef Pt pixel_t;
	typedef  pixel_t*  ppixel_t;
	typedef  EQUIV  comp_t;


	template <int nbits>	
	static uint32_t add_diff(bool m,byte_t& b,ppixel_t cf,ppixel_t& pdf){         
		 
		  
		if(m){
			  b|=(1<<nbits);
			  *(pdf++)=*cf;
			  return 1;
		}
		else  if(b&(1<<nbits))
		{
			*(pdf++)=*cf;
			return 1;
		}
		  else return 0;

	}

	static uint32_t set_delta_bitmask(uint32_t sizeb, void* last_frame,void* curr_frame,void* bitmasks_buff,void* diff_frame)
	{
		byte_t* bitmasks=(byte_t*)bitmasks_buff;
		uint32_t count=0;

		//uint32_t s=(size/sizeof(pixel_t))>>3;// size/sizeof(uint32_t)/8
		uint32_t s=sizeb/(sizeof(pixel_t)*8);
		ppixel_t lf=(ppixel_t) last_frame;
		ppixel_t cf=(ppixel_t) curr_frame;
		ppixel_t df= (ppixel_t)diff_frame;


		for(int nb=0;nb<s;nb++)
		{
         
           byte_t& b= bitmasks[nb];	   

		   count+=add_diff<0>(comp_t::isdelta(*lf,*cf),b,cf,df);++lf;++cf;
		   count+=add_diff<1>(comp_t::isdelta(*lf,*cf),b,cf,df);++lf;++cf;
		   count+=add_diff<2>(comp_t::isdelta(*lf,*cf),b,cf,df);++lf;++cf;
		   count+=add_diff<3>(comp_t::isdelta(*lf,*cf),b,cf,df);++lf;++cf;
		   count+=add_diff<4>(comp_t::isdelta(*lf,*cf),b,cf,df);++lf;++cf;
		   count+=add_diff<5>(comp_t::isdelta(*lf,*cf),b,cf,df);++lf;++cf;
		   count+=add_diff<6>(comp_t::isdelta(*lf,*cf),b,cf,df);++lf;++cf;
		   count+=add_diff<7>(comp_t::isdelta(*lf,*cf),b,cf,df);++lf;++cf;                  

		}
		return count;
	}

	static uint32_t update_frame(uint32_t sizeb, void* frame_buff,void* bitmasks_buff,void* diff_frame_buff)
	{
		byte_t* bitmasks=(byte_t*)bitmasks_buff;

		//uint32_t s=(size/sizeof(pixel_t))>>3;
		uint32_t s=sizeb/(sizeof(pixel_t)*8);
		ppixel_t f=(ppixel_t) frame_buff;
		ppixel_t df0= (ppixel_t)diff_frame_buff;
		ppixel_t df= df0;
		


		for(int nb=0;nb<s;nb++)
		{
			int n=nb<<3; 
			byte_t& b= bitmasks[nb];	   
			if(b&(1<<0)) *f=*(df++);
			++f;			
			if(b&(1<<1)) *f=*(df++);
			++f;
			if(b&(1<<2)) *f=*(df++);
			++f;
			if(b&(1<<3)) *f=*(df++);
			++f;
			if(b&(1<<4)) *f=*(df++);
			++f;
			if(b&(1<<5)) *f=*(df++);
			++f;
			if(b&(1<<6)) *f=*(df++);
			++f;
			if(b&(1<<7)) *f=*(df++);
			++f; 

		}
		 return std::distance(df,df0);
		  //return (UINT_PTR(df)-UINT_PTR(df0))/sizeof(pixel_t);
	}

};

