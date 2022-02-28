#pragma once
#include <Windows.h>
#include "video/filters2d.h"
#include "video/filters3d.h"

template <int Version=1>
struct filter_median_group_t
{
	typedef filter_base_t<>::byte_t byte_t;
	typedef BMP_io_t<>::bmp_file_t bmp_file_t;
	 int w;
	 int h;	 
	 filter_RGB_grayscale_t<32> fin_gr;
	 filter_median_t<> fmt;
	 filter_median_delta_t<4> fmd;
	 filter_quantizer_t<>  fqz;
	 filter_grayscale_RGB_t<32> frgb;
	 BMP_io_t<> bmpgr,bmpmt,bmpqz,bmpout;
	 byte_t *pgr,*pmt,*pqz,*pout;
	 bmp_file_t* pbmpf;


	 inline static byte_t* qft()
	 {
		 static  byte_t qf[256]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
		 return qf;
	 }

	filter_median_group_t(int width,int height,int rmt=7):w(width),h(height)
		,fin_gr(1,width,height)
		,fmt(1,width,height,rmt)
		,fmd(1,width,height)
		,fqz(1,width,height)
		,frgb(1,width,height)
	{
		fqz.filter_load(qft()).filter_invert();
		bmpgr.create(w,h,8);
		bmpmt.create(w,h,8);
		bmpqz.create(w,h,8);
		bmpout.create(w,h,32);

		 pgr=bmpgr.get_bits();
		 pmt=bmpmt.get_bits();
		 pqz=bmpqz.get_bits();
		 pout=bmpout.get_bits();
		 pbmpf=bmpout.get_buf<bmp_file_t>();

	};


	bmp_file_t* make_frame(byte_t* pbits_in,byte_t** pbits_out=0)
	{


		fin_gr.make_frame(pbits_in,pgr);
		fmt.make_frame(pgr,pmt);
		fmd.make_frame(pgr,pmt);
		fqz.make_frame(pmt,pqz);
		frgb.make_frame(pqz,pout);

		if(pbits_out) *pbits_out= pout;
		return pbmpf;
	}


};