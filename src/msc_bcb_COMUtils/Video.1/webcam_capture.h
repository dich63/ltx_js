#pragma once

#include <windows.h>
#include <complex>
#include <utility>
#include <vector>

#include <vfw.h>
#pragma comment( lib, "Vfw32.lib" )

#include "capture_base.h"
 #include "wchar_parsers.h"



struct decompressor_t:_non_copyable
{
	typedef Capture_base::BIHPAL BIHPAL;
   HIC hic;
   //
   //BIHPAL raw_bip;
   size_t  sizeb_out;
   
    
     std::vector<BIHPAL> vbinfo;
	 std::vector<unsigned char> vbits;


     inline unsigned char* pbits_out()
	 {
		 return (vbits.size())? &vbits[0]:0;
	 };

   decompressor_t():hic(0){};

  bool open(BIHPAL& biraw,BIHPAL& bidecompressed)
   {
	   DWORD* pcctype=(DWORD*)("VIDC");
       close();
	   hic=ICOpen(*pcctype,biraw.bmiHeader.biCompression,ICMODE_DECOMPRESS);
	   if(!hic) return false;
	   if(ICERR_OK==ICDecompressBegin(hic,&biraw,&bidecompressed))
	   {
           vbinfo.resize(2); 
           vbinfo[0]=biraw;
           vbinfo[1]=bidecompressed;
		    return true;
	   }
       close(); 

			//hic = ICOpen( MAKEFOURCC('V','I','D','C'),
			//vfmt0.biCompression, ICMODE_DECOMPRESS );
	   //MAKEFOURCC('V','I','D','C');

   }
  inline void close()
  {
	  if(hic)
	  {
		  ICDecompressEnd(hic);
		  ICClose(hic);
		  hic=0;
		  
		  vbits.clear();
	  }

  };

   ~decompressor_t()
   {
     close();
	  
   }

inline  unsigned char*  operator()(void *pbits_in ) 
 {
	 if(!hic) return 0;
	 DWORD dw;
	 if(vbits.size()==0)
	 {
		 //vbits.resize(vbinfo[1].sizeb());
		 vbits.resize(vbinfo[1].bmiHeader.biSizeImage);
		
	 }

	 dw=ICDecompress(hic,0,&(vbinfo[0].bmiHeader),pbits_in,&(vbinfo[1].bmiHeader),pbits_out());

	 return (dw==ICERR_OK)?pbits_out():0;

 }

  inline operator bool()
  {
	  return hic;
  }
};


struct shared_decompressor_t:_non_copyable
{
	typedef Capture_base::BIHPAL BIHPAL;
	HIC hic;
	bool fok;
	int fcopy;
	
	//
	//BIHPAL raw_bip;
	size_t  sizeb_out;
	v_buf_fix<wchar_t,256> mapname;
	v_buf_fix<wchar_t>	filename;


	std::vector<BIHPAL> vbinfo;
	//std::vector<unsigned char> vbits;
	BMPMap   bmpmap;
    BMPMap   bmpmap_tmp;


	inline unsigned char* pbits_out()
	{
		//return (vbits.size())? &vbits[0]:0;
		return bmpmap.pBits();
	};

inline void	set_mapfilename(const wchar_t* _mapname=0,const wchar_t* _filename=0)
	{
		copy_z(_mapname,mapname);
		copy_z(_filename,filename);

	}
	shared_decompressor_t(const wchar_t* _mapname=0,const wchar_t* _filename=0):hic(0),fok(0)
	{
          set_mapfilename( _mapname, _filename);
	};

	bool open(BIHPAL& biraw,BIHPAL& bidecompressed,bool _fcopy=false)
	{
		
		DWORD* pcctype=(DWORD*)("VIDC");
		close();

		fcopy=_fcopy&&(memcmp(&biraw.bmiHeader,&bidecompressed.bmiHeader,sizeof(BITMAPINFOHEADER))==0);
		if(fcopy)
		{
			vbinfo.resize(2); 
			vbinfo[0]=biraw;
			vbinfo[1]=bidecompressed;
           return fok=true;
		}
		else
		{
             
		}

		hic=ICOpen(*pcctype,biraw.bmiHeader.biCompression,ICMODE_DECOMPRESS);
		if(!hic) return fok=false;
		if(ICERR_OK==ICDecompressBegin(hic,&biraw,&bidecompressed))
		{
			vbinfo.resize(2); 
			vbinfo[0]=biraw;
			vbinfo[1]=bidecompressed;
			return fok=true;
		}
		close(); 

		//hic = ICOpen( MAKEFOURCC('V','I','D','C'),
		//vfmt0.biCompression, ICMODE_DECOMPRESS );
		//MAKEFOURCC('V','I','D','C');

	}
	inline void close()
	{
		if(hic)
		{
			ICDecompressEnd(hic);
			ICClose(hic);
			hic=0;
			

			//vbits.clear();
		}
         fok=false;
		 
	};

	~shared_decompressor_t()
	{
		close();

	}
	  
	std::pair<BITMAPFILEHEADER*,size_t> pre_open()
	  {
        
		  std::pair<BITMAPFILEHEADER*,size_t> pp;
            pp.first=bmpmap.open(mapname,filename);
			pp.second=sizeb_out=bmpmap.size;
			return pp;
	  }

	inline  unsigned char*  operator()(void *pbits_in ) 
	{
		if(!fok) return 0;
		DWORD dw=ICERR_OK;
		/*
		if(vbits.size()==0)
		{
			//vbits.resize(vbinfo[1].sizeb());
			vbits.resize(vbinfo[1].bmiHeader.biSizeImage);

		}
		*/
		
        if(!bmpmap)
		{
			bmpmap.init(vbinfo[1],mapname,filename);
			sizeb_out=bmpmap.size_bits;
		}

		locker_t <BMPMap> lock(bmpmap);
        if(fcopy) memcpy(pbits_out(),pbits_in,sizeb_out);
		else dw=ICDecompress(hic,0,&(vbinfo[0].bmiHeader),pbits_in,&(vbinfo[1].bmiHeader),pbits_out());

		return (dw==ICERR_OK)?pbits_out():0;

	}

	inline operator bool()
	{
		return fok;
	}
};



template <class T>
struct WebCamCapture:Capture_base
{
   HWND hWndC,hWndOwher;
   void* pUserData;
   HRESULT hrGrab;
   wchar_t* grfn;
   bool fmapping;
   
   //decompressor_t decompressor;

    shared_decompressor_t decompressor;
   
    typedef WebCamCapture<T>*  this_t;
    typedef WebCamCapture<T>  self_t;
   



   void update_frame(unsigned char* pbytes )
   {

   }


inline    void frame_callback(void* pbits_in)
   {
          int cbf;
         //pBits=pbits_in;
	   hrGrab=1;

	   T* pt=static_cast<T*>(this);

	   if(decompressor)
	   {
		   
     	   unsigned char* p=decompressor(pbits_in);
		  hrGrab|=2;
           if(p) 
		   { 
			     pBits=p;

				 
			     //pt->update_frame(p);
				  
                 hrGrab|=4;
				 
		         
		   }
	   }
	   else {
         pBits=pbits_in;

		 
		 //pt->update_frame((unsigned char* )pbits_in);
		 hrGrab|=4;
	   }
   }

static   LRESULT __stdcall s_FrameCallback(HWND hWnd,LPVIDEOHDR lpVHdr)
   {
	   int gggg=  VHDR_KEYFRAME;
	   //return 0;
	   if(lpVHdr->dwFlags&VHDR_KEYFRAME)
	   {
		    LPARAM l= capGetUserData(hWnd);
            //this_t p=static_cast<this_t>((void*)(l)); 
			this_t(l)->frame_callback(lpVHdr->lpData);

			/*
			pt->pBits=lpVHdr->lpData;
			pt->update_frame(lpVHdr->lpData);
			*/

		   //return 0;
	   }
	   return 0;
   }


static void __stdcall s_DlgVideoSource(int num)
{
       //capDlgVideoSource(hwin);
	Beep(1000,300);
	HWND hWndC = capCreateCaptureWindowA("",0,0,0,0,0,HWND_MESSAGE,0);
     bool  f;
	 f=capDriverConnect(hWndC,num);
    f=capDlgVideoSource(hWndC);
	 f=capDriverDisconnect(hWndC);
	 DestroyWindow(hWndC);

}


   bool set_BPP_compresion(int bpp=0,int compression=BI_RGB)
   {
	   bool f;
	   f=capGetVideoFormat(hWndC, &bip.bmiHeader, sizeof(bip.bmiHeader));
	   if(f) {
          if(bpp>0)
               bip.bmiHeader.biBitCount=bpp;
            bip.bmiHeader.biCompression=compression;
           f=capSetVideoFormat(hWndC, &bip.bmiHeader, sizeof(bip.bmiHeader));
	   }
	   return f=capGetVideoFormat(hWndC, &bip.bmiHeader, sizeof(bip.bmiHeader));
   }

   static void source_dlg(int ncam=0)
   {
	   Beep(500,300);
       QueueUserWorkItem((LPTHREAD_START_ROUTINE)&s_DlgVideoSource,(void*)ncam,WT_EXECUTELONGFUNCTION);
   }

    WebCamCapture(int num=0,int fdlg=0,bool ffcalback=true,int bppout=24):pUserData(0),grfn(0),fmapping(1)
	{
		bool f;
		hWndC=hWndOwher=0;
		if(fdlg&0x10000)
		{
			hr=S_OK;
			return;
		}

	   ;//wnd_alloc("CamCaps000");
       //hWndC = capCreateCaptureWindowA("",0,0,0,0,0,hWndOwher,0);
//  
	   //WS_CHILD | WS_VISIBLE
	   //hWndC = capCreateCaptureWindowA("",WS_CHILD|WS_VISIBLE,0,0,0,0,hwndParent,0);
	   hWndC = capCreateCaptureWindowA("zz",WS_CHILD,0,0,0,0,HWND_MESSAGE,0);
	  if(!hWndC) {hr=GetLastError();if(!hr) hr=E_FAIL;return;};

	  f=capDriverConnect(hWndC,num);
	  if(!f) {hr=GetLastError();if(!hr) hr=E_FAIL;return;};

	  if(fdlg&4)		  f=capDlgVideoCompression(hWndC);
	  if(fdlg&2)		  f=capDlgVideoFormat(hWndC);
	  if(fdlg&1)
	  {
		  //		  f=capDlgVideoSource(hWndC);
         //	
		 source_dlg(num);
	  }



           int sizfmt=capGetVideoFormatSize(hWndC);
		   
		   f=capGetVideoFormat(hWndC, &bip, min(sizfmt,sizeof(bip)));
		   //bppout=32;

		   _fourdw=bip.bmiHeader.biCompression;
		  // if((bip.bmiHeader.biCompression!=BI_RGB)||(bip.bmiHeader.biBitCount!=bppout))
            // bppout=32;
		   {
			   std::vector<BIHPAL> vraw(1,bip);
			   bip.bmiHeader.biCompression=BI_RGB;
			   bip.bmiHeader.biBitCount=bppout;
			   bip.bmiHeader.biSizeImage=bip.sizeb();
			   if(!decompressor.open(vraw[0],bip,true))
			   {
           		   bip.bmiHeader.biHeight=bip.bmiHeader.biWidth=0;
			   }
   		   }

	  //f=capGetVideoFormat(hWndC, &bip.bmiHeader, min(sizfmt,sizeof(bip)));

	  if(!f) {
		  hr=GetLastError();
		  if(!hr) hr=E_FAIL;return;
	  };



	   bip.bmiHeader.biCompression=BI_RGB;


if(0)
{
	  bip.bmiHeader.biBitCount=32;
      bip.bmiHeader.biCompression=BI_RGB;

	  f=capSetVideoFormat(hWndC, &bip.bmiHeader, sizeof(bip.bmiHeader));
         
	  if(!f) {hr=GetLastError();if(!hr) hr=E_FAIL;return;};
}
	  
	  void* pthis=this;
	  f=capSetUserData(hWndC,LPARAM(pthis));
	  if(!f) {hr=GetLastError();if(!hr) hr=E_FAIL;return;};

	  if(ffcalback)
	  {
		  f=capSetCallbackOnFrame(hWndC,&s_FrameCallback);
		  if(!f) {hr=GetLastError();if(!hr) hr=E_FAIL;return;};
	  }
	  
	  _line_sizeb();
	  
	  


	}

	HRESULT clipbrd_copy()
	{
		bool f=capEditCopy(hWndC);
		return (f)?S_OK:E_FAIL;
	};
 
	HRESULT update()
	 {
		 hrGrab=0;
         bool f=capGrabFrame(hWndC);
		 T* pt=static_cast<T*>(this);
		 pt->update_frame((BYTE*)pBits);
		 capGetUserData(hWndC);
		 		 return ((hrGrab&4)&&f)?S_OK:E_FAIL;
	 }
/*
	void grab_to_file(wchar_t *fn)
	{
		grfn=fn;
		update();

	}
*/

	 void stop()
	 {
		 capCaptureAbort(hWndC);
		 capDriverDisconnect(hWndC);
		 DestroyWindow(hWndC);
		 DestroyWindow(hWndOwher);
		 //
	 }

	~WebCamCapture()
	{
		stop();
	}
};

struct WebCamInfo:WebCamCapture<WebCamInfo>
{
	WebCamInfo(int nd=0,int dlg=0,int bpp=24):WebCamCapture<WebCamInfo>(nd,dlg,bpp){};
};

struct WCW:WebCamCapture<WCW>
{
	WCW(HWND hwin,int nd,int dlg=0):WebCamCapture<WCW>(nd,dlg)
	{
		hdc=GetDC(hw=hwin);
		
	};
	HDC hdc;
	HWND hw;
	~WCW()
	{
		ReleaseDC(hw,hdc);
	}

inline 	void update_frame(unsigned char* pbytes )
	{
		//clipbrd_copy();
		//GDI_ERROR
		//BitBlt( hdc, 0, 0, width(), height(),MemDC, 0, 0, SRCCOPY );

		int r=StretchDIBits(hdc,0,0,width(),height(),0,0,width(), height(),pbytes,*this,DIB_RGB_COLORS, SRCCOPY);
	}
};

