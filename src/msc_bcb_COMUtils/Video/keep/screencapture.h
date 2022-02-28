#pragma once

#include "windows.h"
//
#include "capture_base.h"



#define BYTESPERLINE(Width, BPP) ((WORD)((((DWORD)(Width) * (DWORD)(BPP) + 31) >> 5)) << 2)



struct ScreenCapture
{

	struct __s_crt__
	{
		unsigned long crc_table[256];
		__s_crt__()
		{
			unsigned long crc;
			for (int i = 0; i < 256; i++)
			{
				crc = i;
				for (int j = 0; j < 8; j++)
					crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;

				crc_table[i] = crc;
			};
		};

		inline unsigned long make(void *p, unsigned long len)
		{
			unsigned char *buf=(unsigned char *)p;
			unsigned long crc = 0xFFFFFFFFUL;
			while (len--) 
				crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);

			return crc ^ 0xFFFFFFFFUL;

		};

	};

	struct BIHPAL:BITMAPINFO
	{
		//
		RGBQUAD rr[255];
		BIHPAL()
		{
			memset(this,0,sizeof(BIHPAL));
			bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		}
		BIHPAL* init(int w,int h, int nBPP)
		{
			bmiHeader.biWidth=w;
			bmiHeader.biHeight=h;
			bmiHeader.biBitCount = USHORT( nBPP );
			bmiHeader.biPlanes = 1;
			bmiHeader.biCompression = BI_RGB;
			return this;

		}
	};



    HRESULT hr;
    BIHPAL bip,bipr;
	HDC MemDC;
	HBITMAP hBitmap;
	void* pBits;
	int n_monitor;
	MONITORINFOEXA mi;
	 __s_crt__ s_crc;
	int linesizeb;
	int nc; 
	ULONG32 crc;
    ULONG32 crcold;
	RECT r;
	POINT cursor;
	POINT cursor_g;
	
	

inline HRESULT pre_init(HDC dc)
{
	
	hBitmap = ::CreateDIBSection( 0,&bip, DIB_RGB_COLORS, &pBits, NULL,0);
	if(!hBitmap) 
	{
		hr=HRESULT_FROM_WIN32(GetLastError());
		if(!hr) return hr=E_FAIL;
	}
	MemDC= CreateCompatibleDC(dc);
	if(!MemDC) 
	{
		hr=HRESULT_FROM_WIN32(GetLastError());
		if(!hr) return hr=E_FAIL;
	}

	linesizeb=_line_sizeb();

	HGDIOBJ hold=SelectObject(MemDC,hBitmap);
	if(hold) DeleteObject(hold);
	return S_OK;
}

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

static inline int monitor_count()
{
     return ScreenCapture(0x0FFFFF,false).nc;
}

inline BOOL monitor_update(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor)
{
	if((nc++)!=n_monitor) return true;
    _update(hdcMonitor,*lprcMonitor);
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
			hr=init( hdcMonitor,*lprcMonitor);
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
		HDC hdc = GetDC(NULL);
		void *pthis=this;
		bool f=EnumDisplayMonitors(hdc, NULL,enumproc,LPARAM(pthis));
		//if(!f) hr=GetLastError();
		ReleaseDC(0,hdc);

	  }
	  return hr;
  }


inline HRESULT update()
{
    return start_enum(&s_MonitorEnumUpdate);
}

ScreenCapture(int mon=0,bool fupdate=true):MemDC(0),hBitmap(0),n_monitor(mon),hr(0),pBits(0)
{
	     memset(&mi,0,sizeof(mi));
		 mi.cbSize=sizeof(MONITORINFO);
		 start_enum(&s_MonitorEnumInit);
		 if(fupdate) start_enum(&s_MonitorEnumUpdate);
		 
}




 ~ScreenCapture()
 {
	 if(MemDC) DeleteDC(MemDC);
	 if(hBitmap) DeleteObject(hBitmap);

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
 inline int line_sizebr()
 {
	 return BYTESPERLINE(bipr.bmiHeader.biWidth,bipr.bmiHeader.biBitCount);;
 }

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
	 int siz=line_sizeb()*height();
	 return s_crc.make(pBits,siz);
 }

inline bool crc32_check()
 {
    ULONG32 s=get_crc32();
	crcold=crc;
	if(s==crc) return true;
	    else crc=s;
      return false;
 }

inline bool decrease_toRGB24()
 {
	 if(24==bip.bmiHeader.biBitCount) 
		 return true;
       if(32!=bip.bmiHeader.biBitCount) 
		   return false;
    int line24=BYTESPERLINE(bip.bmiHeader.biWidth,24);

	char *ps=((char*)pBits)+3;
	char *pd=ps;

	int h=height();
    int wb=3*(width()-1);
	
	for(int y=0;y<h;y++)
	{
         char * p=ps;
 
///*
        for(char * i=pd;i<pd+wb;i+=3) 
		{
           *((unsigned*)i)=* ((unsigned*)(++p));
		}
//*/
		 
//
		 /*
		 for(char * i=pd;i<pd+wb;i+=3)
		 {
            *i=*(++p);
			*(i+1)=*(++p);
			*(i+2)=*(++p);
		 }
//*/
/*         
     for(int x=0;x<wb;x+=3)
		{
			++p;
            pd[x]=p[x];
			pd[x+1]=p[x+1];
			pd[x+2]=p[x+2];
            //++p; 
		}
*/       
		pd+=line24;
		ps+=linesizeb;
	}
       bipr.bmiHeader=bip.bmiHeader;
       bipr.bmiHeader.biBitCount=24;
		//_line_sizeb();
  return true;
 }

};

