#pragma once

#include "windows.h"
//
#include "capture_base.h"

#include <vector>

#define BYTESPERLINE(Width, BPP) ((WORD)((((DWORD)(Width) * (DWORD)(BPP) + 31) >> 5)) << 2)



struct ScreenCapture:Capture_base
{

	

	

//    BIHPAL bipr;
	int n_monitor;
	MONITORINFOEXA mi;
	// __s_crt__ s_crc;

	int nc; 
	ULONG32 crc;
    ULONG32 crcold;
	
	POINT cursor;
	POINT cursor_g;
	struct 
	{
		HDC dc;
		HWND hwin;
		DWORD rop;

	} _enum_data;
	
	


inline HRESULT init(HDC dc,RECT& r)
{
    crcold=0;
	crc=-1;
	int pla=GetDeviceCaps(dc,PLANES);
	int nBPP = GetDeviceCaps(dc,BITSPIXEL)*pla;
    bip.init(r.right-r.left,r.bottom-r.top,nBPP);
	return pre_init(dc);
}

HRESULT _update(HDC dc,RECT& r)
{
	DWORD ff;
	if(hr==S_OK)
	{
		
	    GetCursorPos(&cursor_g);
		cursor.x=cursor_g.x-r.left;
		cursor.y=cursor_g.y-r.top;

		int w=r.right-r.left;
		int h=r.bottom-r.top;
		
	   ff=::BitBlt(MemDC, 0,0, w,h, dc,r.left,r.top, SRCCOPY);
		
	   if(!ff) hr=GetLastError();
	}
 
	return hr;
}

HRESULT _update(HDC dc,RECT& r,DWORD rop)
{
	DWORD ff;
	if(hr==S_OK)
	{

		GetCursorPos(&cursor_g);
		cursor.x=cursor_g.x-r.left;
		cursor.y=cursor_g.y-r.top;

		int w=r.right-r.left;
		int h=r.bottom-r.top;

		//CAPTUREBLT
		//		ff=::BitBlt(MemDC, 0,0, w,h, dc,r.left,r.top, SRCCOPY);
		//
		ff=::BitBlt(MemDC, 0,0, w,h, dc,r.left,r.top,rop);

		//		ff=::PrintWindow(hwin,MemDC,0);


		if(!ff) hr=GetLastError();
	}

	return hr;
}


static inline int monitor_count()
{
     return ScreenCapture(0x0FFFFF,false).nc;
}

inline BOOL monitor_update(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor)
{
	if((nc++)!=n_monitor) return true;
    //_update(hdcMonitor,*lprcMonitor);
	_update(_enum_data.dc,*lprcMonitor,_enum_data.rop);
	return false;
}


inline BOOL monitor_init(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor)
{
	  if((nc++)!=n_monitor) return true;

      BOOL f=GetMonitorInfo(hMonitor,&mi);
	  hr=GetLastError();
	  bool fcompl=true;
          hr=S_OK;
        if(f)
		{
			//r=*lprcMonitor;

			//hr=init( hdcMonitor,*lprcMonitor);
			hr=init( _enum_data.dc,*lprcMonitor);
			return false;

		}
		
		else hr=GetLastError();

	  return fcompl;
}
static BOOL CALLBACK s_MonitorEnumInit(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor,LPARAM dwData)
{

	return ((ScreenCapture*)dwData)->monitor_init(hMonitor,hdcMonitor, lprcMonitor);
}

static BOOL CALLBACK s_MonitorEnumUpdate(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor,LPARAM dwData)
{

	return ((ScreenCapture*)dwData)->monitor_update(hMonitor,hdcMonitor, lprcMonitor);
}

  HRESULT start_enum(MONITORENUMPROC enumproc)
  {  
      if(hr==S_OK)
	  {
        nc=0;
		HWND hdwin=NULL;
		//		HDC hdc = GetDC(NULL);
				//hdwin=GetDesktopWindow();
				HDC hdc = GetDC(hdwin);
                
				_enum_data.dc=hdc;
				_enum_data.hwin=hdwin;

		
		void *pthis=this;
		//		bool f=EnumDisplayMonitors(hdc, NULL,enumproc,LPARAM(pthis));
		//
		bool f=EnumDisplayMonitors(NULL, NULL,enumproc,LPARAM(pthis));
		//if(!f) hr=GetLastError();
		ReleaseDC(0,hdc);

	  }
	  return hr;
  }


inline HRESULT update(bool frop=0)
{
	_enum_data.rop=(frop)?CAPTUREBLT|SRCCOPY:SRCCOPY;
    return start_enum(&s_MonitorEnumUpdate);
}



ScreenCapture(int mon=0,bool fupdate=true):n_monitor(mon)
//MemDC(0),hBitmap(0),n_monitor(mon),hr(0),pBits(0)
{
	     memset(&mi,0,sizeof(mi));
		 mi.cbSize=sizeof(MONITORINFO);
		 start_enum(&s_MonitorEnumInit);
		 if(fupdate) start_enum(&s_MonitorEnumUpdate);
}




 ~ScreenCapture()
 {
	 //if(MemDC) DeleteDC(MemDC);
	 //if(hBitmap) DeleteObject(hBitmap);

 }

 inline int width()
 {
	 return bip.bmiHeader.biWidth;
 };

 inline int height()
 {
	 return bip.bmiHeader.biHeight;
 };



 inline int line_sizeb()
 {
	 return linesizeb;
 }
 /*
 inline int line_sizebr()
 {
	 return BYTESPERLINE(bipr.bmiHeader.biWidth,bipr.bmiHeader.biBitCount);;
 }
*/
 inline int _line_sizeb()
 {
	return  linesizeb=BYTESPERLINE(bip.bmiHeader.biWidth,bip.bmiHeader.biBitCount);
 }
 inline char* pline(int n)
 {
	 
      return ((char*)pBits)+linesizeb*n;
 }

 inline RECT monitor_rect()
 {
	 return mi.rcMonitor;
 }
 inline RECT monitor_work_rect()
 {
	 return mi.rcWork;
 }

inline ULONG32 get_crc32()
 {
	 int siz=bip.sizeb();
		 //line_sizeb()*height();
	 //return s_crc.make(pBits,siz);
	 return bmpmap.get_crc32();
		 //Crc32((char*)pBits,siz);
 }

inline bool crc32_check()
 {
    ULONG32 s=get_crc32();
	crcold=crc;
	if(s==crc) return true;
	    else crc=s;
      return false;
 }

static inline Capture_base& decrease_toRGB24(Capture_base& bmp=Capture_base())
 {
	 BIHPAL& bipr=bmp.bip;
	 if(bipr.bmiHeader.biBitCount!=32) return bmp;
/*
	 if(24==bip.bmiHeader.biBitCount) 
		 return true;
       if(32!=bip.bmiHeader.biBitCount) 
		   return false;
*/

    int line24=BYTESPERLINE(bipr.bmiHeader.biWidth,24);

	char *ps=((char*)bmp.pBits)+3;
	char *pd=ps;

	int h=bmp.height();
    int wb=3*(bmp.width()-1);
	
	for(int y=0;y<h;y++)
	{
         char * p=ps;
 

        for(char * i=pd;i<pd+wb;i+=3) 
		{
           //*((unsigned*)i)=* ((unsigned*)(++p));
			 *i=*(++p);
			 *(i+1)=*(++p);
			 *(i+2)=*(++p);
			 ++p;
		}
		pd+=line24;
		ps+=bmp.linesizeb;
	}
       //bipr.bmiHeader=bip.bmiHeader;
       bipr.bmiHeader.biBitCount=24;
	   //bmp.pBits=pBits;
	   
	   bipr.bmiHeader.biSizeImage=bipr.sizeb();
	   bmp._line_sizeb();
	   
       

  return bmp;
 }


inline Capture_base& decrease_toGray4(int mode=0,Capture_base& bmp=Capture_base())
{
	BIHPAL& bipr=bmp.bip;
	/*
	if(24==bip.bmiHeader.biBitCount) 
		return true;
	if(32!=bip.bmiHeader.biBitCount) 
		return false;
		*/
	int line4=BYTESPERLINE(bip.bmiHeader.biWidth,4);



	unsigned char *ps=((unsigned char*)pBits);
	
   
	   
    if(!bmp.pBits)
    {
	   bipr.bmiHeader=bip.bmiHeader;
	   bipr.bmiHeader.biBitCount=4;
	   //bmp.pBits=pBits;
	   bmp._line_sizeb();


	   bipr.bmiHeader.biSizeImage=0;//bipr.sizeb();

	   COLORREF* pColor=(COLORREF*)bipr.bmiColors;
	   for(int n=0;n<16;++n)
	   {
		   int nn=n*16;
		   pColor[n]=RGB(nn,nn,nn);
	   }

         if(mode==0)  bmp.pre_init(MemDC);
		 else bmp.pBits=pBits; 
    }
   
   


    unsigned char *pd=((unsigned char*)bmp.pBits);

	int h=height();
	int w=width();
	int wb=4*w;

	for(int y=0;y<h;y++)
	{
	  unsigned	char * p=pd;


		for(unsigned char * i=ps;i<ps+wb;i+=8) 
		{
			unsigned ul,uh;
			ul=i[0+0]+i[0+1]+i[0+2];
			ul/=(3*16);
			uh=i[4+0]+i[4+1]+i[4+2];
			uh/=3;
            p[0]=(0x0F&ul)|(0xF0&uh);
			++p;
		}
		pd+=line4;
		ps+=linesizeb;
	}
	

	return bmp;
}

};

struct monitors_t
{
	int nmons;


	std::vector<ScreenCapture* > vmon;

	void clear()
	{
		for(int n=0;n<vmon.size();n++) delete vmon[n];
		vmon.clear();
		nmons=0;
	}

	int  reset()
	{
		clear();  	

		nmons=ScreenCapture::monitor_count();
		vmon.resize(nmons);
		for(int n=0;n<nmons;n++)
			vmon[n]=new ScreenCapture(n,false);

		return nmons;
	}

	monitors_t()
	{
		reset();
	}
	~monitors_t()
	{
		clear();
	}
	inline int count()
	{
		return nmons;
	}
	ScreenCapture& operator[](int n)
	{
		return *(vmon[n]);
	}

};


