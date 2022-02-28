#pragma once


#include "webcam_capture.h"
#include "yuy2/bmpYUY2.h" 

struct DIB_previewer_base
{
	DC_holder dc;
	HGDI_holder hpen;
	HGDI_holder hbrush,hbrush2;
	int hshift;
	bool ftest; 
	int nframe;
	HDRAWDIB hdrw;
	double shrink;
	BITMAPINFOHEADER* pbmi;
	Capture_base::BIHPAL _bmi;
	std::vector<BYTE> vbyte;
	int fflip;
	int fstretchDIB;
	int fYUY2;


	DIB_previewer_base(HWND hwin=HWND(-1),double _shrink=1,int _hshift=40,HPEN hp=CreatePen(PS_SOLID,2,RGB(0,255,0)))
		:dc(hwin),shrink(_shrink),hpen(hp),hshift(_hshift),ftest(1),nframe(0),hdrw(0),pbmi(0),fstretchDIB(0),fYUY2(0)
	{
		hbrush.attach(CreateSolidBrush(RGB(255,255,0)));
		hbrush2.attach(CreateSolidBrush(RGB(255,255,255)));
		fflip=-1;
	}

	~DIB_previewer_base()
	{
		clear_DIB();
	}

	inline DIB_previewer_base& base_ref(){return *this;};

	//template <class Capture_T>
	void clear_DIB()
	{
		if(hdrw)
		{
			DrawDibEnd(hdrw);
			DrawDibClose(hdrw);
		}
	}

 inline BYTE * flipV(const BYTE *psrc)
 {
        int h=pbmi->biHeight;
		int linecb=_bmi.line_sizeb();
		BYTE *pd=&vbyte[0],*ps=(BYTE*)psrc;
		BYTE* p=pd+linecb*h;

		for(int i=0;i<h;i++)
		{
            p-=linecb; 
			memcpy(p,ps,linecb);
            ps+=linecb;
			
		}
    return pd;
 }

	template <class Capture_T>
	bool   init_once(Capture_T* pcap,void* pUserData=0)
	{
		 bool f;
		 clear_DIB();

		 hdrw=DrawDibOpen();

		 // pbmi=&(pcap->decompressor.bmpmap.pInfo()->bmiHeader);
		 _bmi.bmiHeader=pcap->decompressor.bmpmap.pInfo()->bmiHeader;
		  pbmi=&_bmi.bmiHeader;
		 //_bmi.bmiHeader.biHeight*=-1;
		 int w=pbmi->biWidth;
		 int h=pbmi->biHeight;
		  vbyte.resize(_bmi.sizeb()+16);
		  int linecb=_bmi.line_sizeb();
		  if(fflip==-1)
		  {
             fflip=!(pbmi->biCompression);
		  }

		   


		 //f=DrawDibBegin(hdrw,dc,pcap->width()/shrink,pcap->height()/shrink,pbmi,pcap->width(),pcap->height(),DDF_SAME_HDC);
		 //f=DrawDibBegin(hdrw,dc,w/shrink,h/shrink,pbmi,w,h,DDF_SAME_HDC);
		 f=DrawDibBegin(hdrw,dc,w/shrink,h/shrink,pbmi,w,h,DDF_JUSTDRAWIT);
		 
			 return f;
		 //DDF_SAME_HDC
		 
	}


	template <class Capture_T>
	int   stretchDIB(Capture_T* pcap,char* pbytes,POINT & point,RECT& rect,void* pUserData=0)
	{
		if(!dc) return 0;
         int res;
		
		BITMAPINFOHEADER*pbmi=&pcap->decompressor.bmpmap.pInfo()->bmiHeader;
		char* pYUY2Image;

		if(nframe==0)
		{
			//init_once(pcap,pUserData);
			
			++nframe;
			res=SetStretchBltMode(dc,HALFTONE);

			
			if(fYUY2=(pbmi->biCompression==*((DWORD*)"YUY2"))) 
			{
				vbyte.resize(2*pcap->decompressor.bmpmap.image_size_b()+16);
				
				/*
				memcpy(p,pcap->decompressor.bmpmap.get_BITMAPFILEHEADER(),sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER));
				pbmi=(BITMAPINFOHEADER*)(p+sizeof(BITMAPFILEHEADER));
				BITMAPFILEHEADER* pfi=(BITMAPFILEHEADER* )p;
				pfi->bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
				pbmi->biCompression=0;
				*/

			}

		}

		//BITMAPINFOHEADER* pbmi=&_bmi.bmiHeader;
		if(1)if(fYUY2)
		{
			int cbsz=pcap->decompressor.bmpmap.image_size_b();
			int cbconvert=vbyte.size();
            pYUY2Image=(char*)pcap->decompressor.bmpmap.get_BITMAPFILEHEADER();
			char* p=(char*)&vbyte[0];
			res=BMPYUY2_converter::decompressYUY2(cbsz,(char*)pYUY2Image,&cbconvert,p,0,(char*)pbytes);
			pbmi=(BITMAPINFOHEADER*)(p+sizeof(BITMAPFILEHEADER));
			BITMAPFILEHEADER* pfi=(BITMAPFILEHEADER* )p;
			 pbytes=p+pfi->bfOffBits;
		}


		  int w=pbmi->biWidth;
		  int h=pbmi->biHeight;
		  //
		  if(!pbytes) return 0;

//int nums=DrawDibDraw(hdrw,dc,0,hshift,w/shrink,h/shrink,pbmi,pbytes,0,0,w,h,DDF_SAME_DRAW);//|DDF_SAME_HDC);

		res=StretchDIBits(dc,0,0,w/shrink,h/shrink,0,0,w,h,pbytes,(BITMAPINFO*)pbmi,DIB_RGB_COLORS,SRCCOPY);


       return 1;
	}

	template <class Capture_T>
	 int   operator()(Capture_T* pcap,unsigned char* pbytes,POINT & point,RECT& rect,void* pUserData=0)
	{
		HRESULT hr;
		
		//Sleep(200);
		if(dc) 
		{  
			 if(fstretchDIB) return stretchDIB(pcap,(char*)pbytes,point,rect,pUserData);


			if(nframe==0)
			{
				init_once(pcap,pUserData);
				++nframe;
			}
			//int w=pcap->width();
			//int h=pcap->height();
			int hshift=0;
			//int nums=StretchDIBits(dc,0,hshift,w,h,0,0,w, h,pbytes,(*pcap),DIB_RGB_COLORS, SRCCOPY);
    		//BITMAPINFOHEADER* pbmi=&(pcap->decompressor.bmpmap.pInfo()->bmiHeader);
			//int w=pbmi->biWidth;
			//int h=pbmi->biHeight;
			//BYTE* pbits=pcap->decompressor.bmpmap.pBitsT<BYTE>();
			int w=pbmi->biWidth;
			int h=pbmi->biHeight;
             //
			if(!pbytes) return 0;
			//
            if(fflip)
			    pbytes=flipV(pbytes);
      //    
			int nums=DrawDibDraw(hdrw,dc,0,hshift,w/shrink,h/shrink,pbmi,pbytes,0,0,w,h,DDF_SAME_DRAW);//|DDF_SAME_HDC);
			int ys=hshift;
             return 0;
			RECT rb={0,0,w,h};
			if(EqualRect(&rect,&rb)) return 0;

			//
			RECT rrect={rect.left,rect.top+ys,rect.right,ys+rect.bottom};
			//RECT rrect={200,200,500,500};
			FrameRect(dc,&rrect,(HBRUSH)hbrush.hobj);
			//SleepEx(1000,1);
			//  hr=GetLastError();
		}
		return 0;
	}

};


