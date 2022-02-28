#pragma once
#include "lasercalibrate.h"
#include "dib_viewers.h"


template <class ViewerT=DIB_previewer_base,class CameraT=WC_filemap_ext<ViewerT>>
struct console_previewer_image_t
{
    
	double m_fps;
	ViewerT dib;
	CameraT camera;
	HANDLE hevent;
	std::vector<BYTE> vv;
	std::pair<BITMAPFILEHEADER*,size_t> h_s;
	console_previewer_image_t(BMPMap & bmpmap,HWND hwin=GetConsoleWindow()):
     dib(hwin)
    ,camera(dib)
	,m_fps(25)
	{
		if(!IsWindow(hwin)) return;
        h_s=camera.open(bmpmap);
	}
	;

static void __stdcall s_preview(console_previewer_image_t* p)
{
	try
	{
		p->preview();
	}
	catch (...){}
   
}

inline  void __stdcall preview()
	{
        
		double ips=1000/m_fps;
		for(;;)
		{
			camera.locked_update();
			Sleep(ips);
		}
	};

bool run(bool fasynchro,double shrink=1,double fps=25)
{
   if(!h_s.first) return false;
   dib.shrink=shrink;
   
   m_fps=fps;
   if(fasynchro)
       QueueUserWorkItem((LPTHREAD_START_ROUTINE)&s_preview,this,WT_EXECUTELONGFUNCTION);
   else s_preview(this);
}

};


