#pragma once
#include "yuv4526.h"

template <int _Version=0>
struct yuv_packer_1755_t
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
		bpp_y=7,
		bpp_h0=bpp_y,
        bpp_uv=5,
		bpp_hi=bpp_uv,
	};

	typedef yuv_rgb_t<bpp_y,bpp_uv> yuvrgb_t;


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
			

		int32_t y00=yuvrgb_t::rgb_to_y(p00);
		int32_t y01=yuvrgb_t::rgb_to_y(p01);
		int32_t y10=yuvrgb_t::rgb_to_y(p10);
		int32_t y11=yuvrgb_t::rgb_to_y(p11);

		const int shift0=yuvrgb_t::pow_hi+2-bpp_h0;
		const int shifti=yuvrgb_t::pow_hi+2-bpp_hi;
		const int32_t offset=(1<<bpp_hi)/2; 
		const int sh1=32-2*bpp_uv-0*bpp_hi;
		const int sh2=32-2*bpp_uv-1*bpp_hi;
		const int sh3=32-2*bpp_uv-2*bpp_hi;



		int32_t h0=(y00+y01+y10+y11)>>shift0;
		int32_t h1=offset+(y00+y01-y10-y11)>>shifti;
		int32_t h2=offset+(y00-y01+y10-y11)>>shifti;;
		int32_t h3=offset+(y00-y01-y10+y11)>>shifti;
      

		
        
       uint32_t res=yuv|(h1<<sh1)|(h2<<sh2)|(h3<<sh3)|h0;
	   //|(u00)|(u01<<(1*bpp_y))|(u10<<(2*bpp_y))|(u11<<(3*bpp_y));
	   return res;
		

	}

	template <class N>
	inline	static void yuv_to_rgb4_unpack(uint32_t yuvpack,N* p00,N* p01,N* p10,N* p11)
	{
		return yuv_to_rgb4_unpack( yuvpack,(triplet_byte_t*)p00, (triplet_byte_t*)p01,(triplet_byte_t*)p10,(triplet_byte_t*)p11);
	}

	inline	static void yuv_to_rgb4_unpack(uint32_t yuvpack, triplet_byte_t* p00,triplet_byte_t* p01,triplet_byte_t* p10,triplet_byte_t* p11)
	{

		const int32_t offset=(1<<bpp_hi)/2; 
		const int sh1=32-2*bpp_uv-0*bpp_hi;
		const int sh2=32-2*bpp_uv-1*bpp_hi;
		const int sh3=32-2*bpp_uv-2*bpp_hi;

		byte_t u=yuvpack>>(32-bpp_uv);
		byte_t v=(yuvpack>>(32-2*bpp_uv))&32;


		




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
