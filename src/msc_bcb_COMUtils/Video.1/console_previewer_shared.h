#pragma once
//#include "lasercalibrate.h"
//#include "dib_viewers.h"
#include "IShared_Image.h"
#include "wchar_parsers.h"


template <int II=0>
struct shared_image_saver_t
{
   shared_image_helper_t shared_image;
   std::wstring filefmt;
   HRESULT hr;
   int nframe;
   
   shared_image_saver_t(const wchar_t* key,const wchar_t* pfilefmt):shared_image(key),filefmt(pfilefmt) 
   {
        hr=shared_image.hr;
   }

   shared_image_saver_t(ISharedImage* si,const wchar_t* pfilefmt):filefmt(pfilefmt) 
   { 
	   shared_image.reset(si); 
	   hr=shared_image.hr;
   }


   int save(int nframe)
   {
	   wchar_t fnb[2048];
       wsprintf(fnb,(wchar_t *)filefmt.c_str(),nframe);
	   FILE* hf=_wfopen(fnb,L"wb");
	   if(hf)
	   {
		   BITMAPFILEHEADER* pbfh;
		   const int ccc=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
		   int cbimage;
		   if(FAILED(hr=shared_image->GetFrame(&pbfh,&cbimage)))
			   return 0;

		   BITMAPINFOHEADER *pbih=(BITMAPINFOHEADER *)( ((char*)pbfh)+sizeof(BITMAPFILEHEADER));

		   int cbo=0;
		   int coff= pbfh->bfOffBits-ccc;
		   //cbo+=fwrite(pbfh,1,cbimage+coff,hf);
		   cbo+=fwrite(pbfh,1,cbimage,hf);


		   //cbo+=fwrite(pbfh,1,ccc,hf);
		   //
		   //cbo+=fwrite(((char*)pbfh)+coff,1,cbimage+(coff-ccc),hf);
           fclose(hf);  
          return cbo;
	   }
        return 0;

   }

   HRESULT run(float fps=25, int endframe=-1) {
	     
	     int nframe=0;
         double ips=1000/fps;
		 DWORD tic=GetTickCount();
		 while(save(++nframe))
		 {
			 if((endframe>0)&&(nframe>endframe))
				 return S_OK;
			 double t=ips-(GetTickCount()-tic);
			 if(t>5) Sleep(t);
			 tic=GetTickCount();
		 }

         return S_OK;    
   }

};

template <int II=0>
struct console_previewer_shared_image_t
{
    HDC dc;
	double m_fps;
	double shrink;
	v_buf<wchar_t> imagekey;
	v_buf<char> ttt;
	int flipV,smode;
	shared_image_helper_t shared_image;

	template <class CH>
	console_previewer_shared_image_t(CH* _imagekey,int _flipV=0,shared_image_helper_t& sh=shared_image_helper_t()):     
	m_fps(25),smode(HALFTONE),
	shared_image(sh),flipV(_flipV)
	{
		shared_image->Compression(1);
		imagekey.cat(char_mutator<CP_THREAD_ACP>(_imagekey));
		//ttt.resize(1024*1024*16);

	}
	;

static void __stdcall s_preview(console_previewer_shared_image_t* p)
{
	try
	{
		p->preview();
	}
	catch (...){}
   
}

inline  void StretchDIB()
{
	HRESULT hr;
   BITMAPFILEHEADER* pbfh=0;
   if(shared_image->GetFrame(&pbfh)) 
	   return;
   char* p=(char* )pbfh;
   BITMAPINFO* pbmi=(BITMAPINFO*)(p+sizeof(BITMAPFILEHEADER));
   char*pbytes=(p+pbfh->bfOffBits);
   int w=pbmi->bmiHeader.biWidth;
   int h=pbmi->bmiHeader.biHeight;


   //
   if(!pbytes) return ;

   //int ttc=w*h*2;

        //pbytes=  ttt.get();
   int res;
   res=StretchDIBits(dc,0,0,w/shrink,h/shrink,0,(flipV)?h:0,w,(flipV)?-h:h,pbytes,(BITMAPINFO*)pbmi,DIB_RGB_COLORS,SRCCOPY);
   hr=GetLastError();
   //GDI_ERROR

}

inline  void __stdcall preview()
	{
        
		double ips=1000/m_fps;
		DWORD tic=GetTickCount();
		for(;;)
		{
			StretchDIB();
			//camera.locked_update();
			double t=ips-(GetTickCount()-tic);
			if(t>5) Sleep(t);
			tic=GetTickCount();
		}
	};

bool run(bool fasynchro,double _shrink=1,double fps=25)
{
	HWND hwin=GetConsoleWindow();
   
	
   return run_to_hwin(hwin,fasynchro,_shrink,fps);
}

bool run_to_hwin(HWND hwin, bool fasynchro,double _shrink=1,double fps=25)
{

	//HWND hwin=GetConsoleWindow();
	if(!hwin) return false;
	dc=GetDC(hwin);
	if(!dc) return false;
	SetStretchBltMode(dc,smode);

	shrink=_shrink;
	if(shared_image->Init(imagekey))
		return false;

	m_fps=fps;
	if(fasynchro)
		return QueueUserWorkItem((LPTHREAD_START_ROUTINE)&s_preview,this,WT_EXECUTELONGFUNCTION);
	else s_preview(this);
	return 1;
};


};

template<class CH,class Args>
console_previewer_shared_image_t<>* start_console_previewer_shared_image_hwnd(HWND hwin,Args& args,const CH* keyc=0,bool fasyn=true)
{
	const CH* key=(keyc)?keyc:args[char_mutator<>("key")];
	double shrink=args[char_mutator<>("viewer.shrink")].def<double>(0.0);
	double viewer_fps=args[char_mutator<>("viewer.fps")].def<double>(8);
	if((shrink<0.00001)||(viewer_fps<0.00001)) return 0;
	int flip=args[char_mutator<>("viewer.flip")].def<int>(args[char_mutator<>("viewer.flipV")]);

	console_previewer_shared_image_t<>* cp=new console_previewer_shared_image_t<>(key,flip);
	cp->smode=args[char_mutator<>("viewer.smode")].def<int>(HALFTONE);
	if(cp->run_to_hwin(hwin,fasyn,shrink,viewer_fps)) return cp;
	else delete cp;
	return 0;

}

template<class CH,class Args>
console_previewer_shared_image_t<>* start_console_previewer_shared_image(Args& args,const CH* keyc=0,bool fasyn=true)
{
	return start_console_previewer_shared_image_hwnd(GetConsoleWindow(),args,keyc,fasyn);
	/*
	const CH* key=(keyc)?keyc:args[char_mutator<>("key")];
	double shrink=args[char_mutator<>("viewer.shrink")].def<double>(0.0);
	double viewer_fps=args[char_mutator<>("viewer.fps")].def<double>(8);
	if((shrink<0.00001)||(viewer_fps<0.00001)) return 0;
	int flip=args[char_mutator<>("viewer.flip")].def<int>(args[char_mutator<>("viewer.flipV")]);

	console_previewer_shared_image_t<>* cp=new console_previewer_shared_image_t<>(key,flip);
	cp->smode=args[char_mutator<>("viewer.smode")].def<int>(HALFTONE);
	if(cp->run(fasyn,shrink,viewer_fps)) return cp;
	else delete cp;
	return 0;
	*/

}



