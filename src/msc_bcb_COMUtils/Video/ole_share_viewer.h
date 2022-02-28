#pragma once
//#include "ole_share_viewer.h"
#include "IShared_Image.h"
#include "ipc_ports/callback_context_arguments.h"
#include "ipc_ports/ltx_call_helper.h"
#include "singleton_utils.h"


template <int Version=0>
struct ole_viewer_holder_t: ltx_helper::dispatch_wrapper_t<ole_viewer_holder_t<Version>,variant_t>
{
	event_local_t evnt;
	mutex_cs_t& mutex;
	
	HWND hwin;
	ole_viewer_holder_t(HANDLE h,HWND hw,mutex_cs_t& m):evnt(h),hwin(hw),mutex(m){}
     
	inline  void  oncallback(com_variant_t& result,arguments_t& arguments){		
          evnt.signal(); 
		  InvalidateRect(hwin,0,1);
		  UpdateWindow(hwin);
		  locker_t<mutex_cs_t> lock(mutex);


	};

};

template <int Version=0>
struct shared_hwnd_viewer_t {

	 HWND hwin;
	 HRESULT hr;
	 HBRUSH hbr;
	 //ipc_utils::smart_ptr_t<ISharedImage> shimg;
	 shared_image_helper_t shared_image;
	 event_local_t evnt;
	 mutex_cs_t mutex;
	 float fps;
	 shared_hwnd_viewer_t(IUnknown* olecontrol,wchar_t* argzz):hwin(0), hbr(0)
	 {
		 ipc_utils::smart_ptr_t<IOleWindow> olewindow;
		 hr=olecontrol->QueryInterface(__uuidof(IOleWindow),olewindow._ppQI());
		 if(FAILED(hr)) return;
		 hr=olewindow->GetWindow(&hwin);
		 if(FAILED(hr)) return;

		 new (this) shared_hwnd_viewer_t(hwin,argzz);
	 }
	 shared_hwnd_viewer_t(HWND hwnd,wchar_t* argzz):hwin(hwnd),hr(S_OK), hbr(0)
	 {
		  //hr=ISharedImage::Create(shimg._address());
		   argv_zz<wchar_t> args(argzz);
		   shared_image=shared_image_helper_t(args[L"key"]);
		  if(FAILED(hr)) return;
		  shared_image->SetOptions(argzz);
		  
		  fps=args[L"fps"].def<double>(25.0);
		  if(fps<=0.5) fps=0.5;
		  hbr=CreateSolidBrush(RGB(0,0,64));

	 }

      ~shared_hwnd_viewer_t()
	  {
		  hbr||DeleteObject(hbr);
	  }



	  //inline POINT get_win


      
	 inline bool draw()
	 {
		 int scline;
		 RECT rect;
		 PAINTSTRUCT  ps;
		 HDC dc;
		 if(!(GetClientRect(hwin,&rect)))
			 return false;
		 if(!(dc=GetDC(hwin))) 
			 return false;

		 int ix=GetDeviceCaps(dc,ASPECTX);
		 int iy=GetDeviceCaps(dc,ASPECTY);


		 
		 //ClientToScreen(hwin,((POINT*)&rect));
		 //ClientToScreen(hwin,((POINT*)&rect)+1);

		 //DPtoLP(dc, (LPPOINT) &rect, 2); 

		 double ww=rect.right-rect.left,hw=rect.bottom-rect.top;

		 BMP_helper_t bmp(shared_image);
		 int w=bmp.width(),h=bmp.height();

		 double scw=double(ww)/double(w),sch=double(hw)/double(h); 

		 double wsw=ww,wsh=hw,osw=0,osh=0;
		 double sc=(scw>sch)?sch:scw;

		 if(scw<sch)
		 {			 
			 
			 wsh=h*scw;
			 double dd=(hw-wsh)/2;
			 //wsh+=dd;
			 osh+=dd;
		 }
		 else {
				 
				 
				 wsw=w*sch;
				 double dd=(ww-wsw)/2;
				 //wsw+=dd;
				 osw+=dd;
		 }





		 BITMAPINFO* pbi=bmp.hdr_info();
		 char * pbits=bmp.bits<char>();
		 if(bmp.hr) 
			 return false;
		 //if(!(dc=BeginPaint(hwin,&ps))) 
		//	 return false;
		 

		 //FillRect(dc, &rect, hbr);
		 SetStretchBltMode(dc,HALFTONE);
		 
		 //
		 scline=StretchDIBits(dc,osw,osh,wsw,wsh,0,0,w,h,pbits,pbi,DIB_RGB_COLORS,SRCCOPY);
		  //scline=StretchDIBits(dc,0,0,ww/2,hw*scw,0,0,w,h,pbits,pbi,DIB_RGB_COLORS,SRCCOPY);
		 //		 scline=StretchDIBits(dc,0,0,ww,h*scw/2,0,0,w,h,pbits,pbi,DIB_RGB_COLORS,SRCCOPY);
         //scline=StretchDIBits(dc,0,0,w,h,osw,osh,wsw,wsh,pbits,pbi,DIB_RGB_COLORS,SRCCOPY);
		 //		 scline=StretchDIBits(dc,0,0,w,h,0,0,w,h,pbits,pbi,DIB_RGB_COLORS,SRCCOPY);
		 
		 //EndPaint(hwin,&ps);
		 ReleaseDC(hwin,dc);

		 
		 return true;
	 }

	 void view()
	 {

		 //HDC dc=GetDC(hwin);
		 RECT rect;
		 double ips=1000/fps;
		 ULONGLONG tic=GetTickCount64();
		 DWORD tw=0;
         locker_t<mutex_cs_t> lock(mutex);

		 while((WaitForSingleObject(evnt.hko,tw)==WAIT_TIMEOUT)&&draw() )
		 {

             ULONGLONG tx=GetTickCount64();
             double t=ips-double(tx-tic);
			 tic=tx;             		 
			 tw=(t>0)?t:0;
		 }



	 }

	 static void __stdcall s_view(shared_hwnd_viewer_t* p)
	 {
		 try
		 {
			 p->view();
		 }
		 catch (...){
            delete p;   
		 }

	 }



	 HRESULT start(VARIANT* res)
	 {
		 if(!res) hr=E_POINTER;
		 mutex.unlock();
		 if(SUCCEEDED(hr))
		 {
                if(!QueueUserWorkItem((LPTHREAD_START_ROUTINE)&s_view,this,WT_EXECUTELONGFUNCTION))
				{hr=HRESULT_FROM_WIN32(GetLastError());}
		 }

		 if(hr) 
			 delete this;
		 else
		 {
			 ole_viewer_holder_t<>* oh=new ole_viewer_holder_t<>(evnt.hko,hwin,mutex);
			 hr=oh->wrap(res);
		 }
		 return hr;
	 }

};


extern "C"  void __cdecl  ltx_ole_viewer
( variant_t* presult, pcallback_lib_arguments_t pltx,pcallback_context_arguments_t pcca)
{


	ltx_helper::arguments_t<variant_t> arg(pcca);
	HRESULT &hr=arg.hr;
	VARIANT voptions=arg[1];
	wchar_t* poptions=(voptions.vt==VT_BSTR)?voptions.bstrVal:0;
	ipc_utils::smart_ptr_t<IOleWindow> olewindow;
	if(FAILED(hr=arg[0].QueryInterface(olewindow._address())))
		return;
	HWND hwin;
	if(FAILED(hr=olewindow->GetWindow(&hwin)))
		return; 



	hr=(new shared_hwnd_viewer_t<>(hwin,poptions))->start(presult); 
    

}