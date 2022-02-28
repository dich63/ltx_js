#pragma once

#include "convert_yuy2.h"


#define BYTESPERLINE(Width, BPP) ((WORD)((((DWORD)(Width) * (DWORD)(BPP) + 31) >> 5)) << 2)

struct BMPYUY2_converter
{


static	ULONG compressYUY2(int scb,char *sbuf,int* pdcb,char* dbuf, void * phandle=0,char* s_pbits=0,char* d_pbits=0)
	{
		char* pyuy="YUY2";
		DWORD* pcctype=(DWORD*)("VIDC");
		DWORD Compression=*((DWORD*)pyuy);

		BITMAPFILEHEADER tmp;

		




		BITMAPFILEHEADER& fih_s= *((BITMAPFILEHEADER*)sbuf);
		BITMAPFILEHEADER& fih_d= (dbuf)?*((BITMAPFILEHEADER*)dbuf):tmp;



		fih_d=fih_s;
		BITMAPINFO& bi_s=*((BITMAPINFO*)(sbuf+sizeof(BITMAPFILEHEADER)));
		BITMAPINFO& bi_d=*((BITMAPINFO*)(dbuf+sizeof(BITMAPFILEHEADER)));


		bi_d=bi_s;
		bi_d.bmiHeader.biCompression=Compression;
		bi_d.bmiHeader.biBitCount=16;
		bi_d.bmiHeader.biWidth=(bi_d.bmiHeader.biWidth>>2)<<2;


		DWORD cb_d= BYTESPERLINE(bi_d.bmiHeader.biWidth,bi_d.bmiHeader.biBitCount)*bi_d.bmiHeader.biHeight;
		DWORD cb_s= BYTESPERLINE(bi_s.bmiHeader.biWidth,bi_s.bmiHeader.biBitCount)*bi_s.bmiHeader.biHeight;
		DWORD imsize_s=cb_s+fih_s.bfOffBits;
		DWORD imsize_d=cb_d+fih_d.bfOffBits;
		fih_d.bfSize=imsize_d-sizeof(BITMAPFILEHEADER);
		bi_d.bmiHeader.biSizeImage=cb_d;

		char* pbits_s=(s_pbits)?s_pbits:sbuf+fih_s.bfOffBits;
		char* pbits_d=(d_pbits)?d_pbits:dbuf+fih_d.bfOffBits;
		int err;

		int w=bi_d.bmiHeader.biWidth;
		int h=bi_d.bmiHeader.biHeight;

		int bpps=bi_s.bmiHeader.biBitCount;

		int mul=bpps/8;
		int srcStride=w*mul;
		srcStride=BYTESPERLINE(bi_s.bmiHeader.biWidth,bi_s.bmiHeader.biBitCount);
		int dstStride = w * 2;
		dstStride=BYTESPERLINE(bi_d.bmiHeader.biWidth,bi_d.bmiHeader.biBitCount);
		int srcSize = imsize_s;
		int dstSize = imsize_d;

		int iMatrix = 0;//0=rec601, 1=rec709, 3=PC_601, 7=PC_709


		switch(bpps)
		{
		case 24:
			Tmmx_ConvertRGBtoYUY2<true, false>::mmx_ConvertRGBtoYUY2((BYTE*)pbits_s,(BYTE*)pbits_d, srcStride, dstStride, w, h, iMatrix); 
			*pdcb=imsize_d;
			return 1;
		case 32:
			Tmmx_ConvertRGBtoYUY2<false, false>::mmx_ConvertRGBtoYUY2((BYTE*)pbits_s,(BYTE*)pbits_d, srcStride, dstStride, w, h, iMatrix); 
			*pdcb=imsize_d;
			return 1;
		};

		return 0;   

	}


static	ULONG decompressYUY2(int scb,char *sbuf,int* pdcb,char* dbuf, void * phandle=0,char* s_pbits=0,char* d_pbits=0)
{
	char _tt[sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+64];
	BITMAPFILEHEADER& tmp= *((BITMAPFILEHEADER*)_tt);
	char* pyuy="YUY2";
	DWORD* pcctype=(DWORD*)("VIDC");
	DWORD Compression=*((DWORD*)pyuy);

	int bppd=24;
	if(phandle)
		bppd=*((int*)phandle);

	BITMAPFILEHEADER& fih_s= *((BITMAPFILEHEADER*)sbuf);
	BITMAPFILEHEADER& fih_d=(dbuf)? *((BITMAPFILEHEADER*)dbuf):tmp;



	fih_d=fih_s;
	BITMAPINFO& bi_s=*((BITMAPINFO*)(sbuf+sizeof(BITMAPFILEHEADER)));
	//BITMAPINFO& bi_d=*((BITMAPINFO*)(dbuf+sizeof(BITMAPFILEHEADER)));
	BITMAPINFO& bi_d=*((BITMAPINFO*)((&fih_d)+1));


	bi_d=bi_s;
	bi_d.bmiHeader.biCompression=0;
	bi_d.bmiHeader.biBitCount=bppd;
	


	DWORD cb_d= BYTESPERLINE(bi_d.bmiHeader.biWidth,bi_d.bmiHeader.biBitCount)*bi_d.bmiHeader.biHeight;
	DWORD cb_s= BYTESPERLINE(bi_s.bmiHeader.biWidth,bi_s.bmiHeader.biBitCount)*bi_s.bmiHeader.biHeight;
	DWORD imsize_s=cb_s+fih_s.bfOffBits;
	DWORD imsize_d=cb_d+fih_d.bfOffBits;
	fih_d.bfSize=imsize_d-sizeof(BITMAPFILEHEADER);

	//char* pbits_s=sbuf+fih_s.bfOffBits;
	//char* pbits_d=dbuf+fih_d.bfOffBits;
	char* pbits_s=(s_pbits)?s_pbits:sbuf+fih_s.bfOffBits;
	char* pbits_d=(d_pbits)?d_pbits:dbuf+fih_d.bfOffBits;
	int err;

	int w=bi_d.bmiHeader.biWidth;
	int h=bi_d.bmiHeader.biHeight;

	int bpps=bi_s.bmiHeader.biBitCount;

	int mul=bpps/8;
	int srcStride=w*mul;
	srcStride=BYTESPERLINE(bi_s.bmiHeader.biWidth,bi_s.bmiHeader.biBitCount);
	int dstStride = w * 2;
	dstStride=BYTESPERLINE(bi_d.bmiHeader.biWidth,bi_d.bmiHeader.biBitCount);
	int srcSize = cb_s;
	int dstSize = cb_d;

	int iMatrix = 0;//0=rec601, 1=rec709, 3=PC_601, 7=PC_709


	switch(bppd)
	{
	case 24:
		Tmmx_ConvertYUY2toRGB<false, false>::mmx_ConvertYUY2toRGB((BYTE*)pbits_s,(BYTE*)pbits_d,((BYTE*)pbits_s)+cb_s,srcStride, dstStride, w*2, iMatrix); 
		*pdcb=imsize_d;
		return 1;
	case 32:
		 Tmmx_ConvertYUY2toRGB<false, true>::mmx_ConvertYUY2toRGB((BYTE*)pbits_s,(BYTE*)pbits_d,((BYTE*)pbits_s)+cb_s,srcStride, dstStride, w*2, iMatrix); 
		*pdcb=imsize_d;
		return 1;
	};

	return 0;   

}




};

