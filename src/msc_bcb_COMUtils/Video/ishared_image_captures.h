#pragma once

#define  CP_X CP_THREAD_ACP

#define s_m(s) L#s

#ifdef _DECKLINK_CAPTURE_SI
#include "DeckLinkCaptureSI.h" 
#endif 

#include "video/hssh_desktop_service.h"
#include "video/ishared_image_impl.h"
//#include "video/aviutil.h"
//#include "free_com_factory.h" 
#include "static_constructors_0.h"
#include "ole_share_viewer.h"

//GUID ggg=__uuidof(CDeckLinkIterator);
struct argv_cmdline_S
{

	v_buf<char> flat;
	v_buf<wchar_t> wflat;
	wchar_t* pw;
	char* p;

	argv_cmdline_S()
	{


		argv_cmdline<wchar_t> args(GetCommandLineW());

		pw=copy_zz((wchar_t*)args.flat_str(),wflat).get();
		p=copy_zz((char*)char_mutator<CP_THREAD_ACP>(args.flat_str()),flat).get();

	}


};

extern "C" wchar_t*   get_cmdline_zzW()
{
	return class_initializer_T<argv_cmdline_S>().get().pw;
}

extern "C" char*   get_cmdline_zz()
{
	return class_initializer_T<argv_cmdline_S>().get().p;
}

extern "C" HRESULT  Create_ICaptureImage(ICaptureImage** ppimage);

template <int VERSION=0>
struct ICaptureImage_mon_webcam_impl:ICaptureImage
{

	typedef typename DummyIDispatchAggregator_t<ICaptureImage_mon_webcam_impl,true> dummy_dispatch_t;

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject)
	{ 
		if(!ppvObject) return E_POINTER;
		HRESULT hr=E_NOINTERFACE;

		if (iid == __uuidof(IUnknown))			
		{
			*ppvObject = static_cast< ICaptureImage*>(this);
			AddRef();
			hr=S_OK;
		} 

		ddisp(this,iid,ppvObject,hr);
		return hr;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) 
	{ 
		return InterlockedIncrement((LONG volatile *)&refcount); 
	}

	virtual ULONG STDMETHODCALLTYPE Release(void) 
	{
		LONG l= InterlockedDecrement((LONG volatile *)&refcount); 
		if(l==0) delete this;
		return l;
	}

	virtual HRESULT InitA(const char* options,ICaptureImage::FILEINFO** ppfi,int* pcbimage){
		
		if(ICMDLINE==(ULONG)(options)) return Init(get_cmdline_zzW(),ppfi,pcbimage);
		else return Init(char_mutator<CP_THREAD_ACP>(options),ppfi,pcbimage);
	};

	inline HRESULT update_webcam(ICaptureImage::FILEINFO** ppfi,int* pcbimage=0)
	{
		  HRESULT hr;

	}


inline HRESULT update_screen(ICaptureImage::FILEINFO** ppfi,int* pcbimage=0)
{
       HRESULT hr;
	   int ii;
    hr=pScreen->update();
	if(f24)
	{
		pScreen->bmpmap.decrease_toRGB24();
	}
	BITMAPFILEHEADER* pbmp=pScreen->bmpmap.get_BITMAPFILEHEADER();
	if(fyut2) 
	{
		int cb=pScreen->bmpmap.image_size_b()+128;
		buf.resize(cb);
		char* pbuf=buf;
		if(BMPYUY2_converter::compressYUY2(cb,(char*)pbmp,&cb,(char*)pbuf))
			pbmp=(BITMAPFILEHEADER*)pbuf;
	}
	if(ppfi)
	{
         *ppfi=ICaptureImage::PFILEINFO(pbmp);
	}
	if(SUCCEEDED(hr)&&(pcbimage))
	{
         //ii=*pcbimage=pbmp->bfSize;
		 //ii=
			 *pcbimage=pScreen->bmpmap.image_file_size_b();

	}
	return hr;
}

	virtual HRESULT Init(const wchar_t* options,ICaptureImage::FILEINFO** ppfi,int* pcbimage)
	{
		HRESULT hr=E_NOINTERFACE;
		 //return Init();
		wchar_t* opts=(ICMDLINE==(ULONG)(options))?get_cmdline_zzW():(wchar_t*)options;
         argv_zz<wchar_t> args(opts);
		 wchar_t* capture=args[s_m(capture)];
		 int tmp;
		 ICaptureImage::FILEINFO* pfitmp;
		 if(!pcbimage) pcbimage=&tmp;
		 if(!ppfi) ppfi=&pfitmp;
        
		 
		 
		 if(safe_cmpni(capture,s_m(monitor),7)==0)
		 {

			  f24=args[s_m(f24)].def<int>(0);
              wchar_t* compress=args[s_m(compress)];
             fyut2=safe_cmpni(s_m(yuy2),compress,4)==0;			 
			 int nmon=args[s_m(nmon)].def(0);
			 pScreen=new ScreenCapture(nmon);
			 hr=update_screen( ppfi,pcbimage);
			 if(0) {		 
			 FILE* hf=fopen("c:\\temp\\aa.bmp","wb");
			 fwrite(*ppfi,1,*pcbimage,hf);
			 fclose(hf);
			 }


		 }  
		 else if(safe_cmpni(capture,s_m(webcam),6)==0)
		 {
			 /*
			 pWebCam=new WebCamInfo;
			 
			 pWebCam->update();
			 hr=pWebCam->hr;
			 pbmp=pWebCam->decompressor.bmpmap.get_BITMAPFILEHEADER();
			 */
		 }

		return hr;

	};
	virtual HRESULT GetFrame(ICaptureImage::FILEINFO** ppbmpimage,int* pcbimage)
	{
       //ICaptureImage::PFILEINFO pbmpimage;
        //ppbmpimage||(ppbmpimage=&pbmpimage);

		HRESULT hr;
		if(pScreen)
		      hr=update_screen(ppbmpimage,pcbimage);
		//else if(pWebCam)
		else hr=E_NOINTERFACE;

		

		return hr;
	}

	static HRESULT CreateInstance(ICaptureImage** ppObj)
	{
		if(!ppObj) return E_POINTER;
		//HRESULT hr;
		*ppObj=new ICaptureImage_mon_webcam_impl<0>;
		return S_OK;

	}
	ICaptureImage_mon_webcam_impl():refcount(1),pWebCam(0),pScreen(0),pbmp(0),mode(0){
	  free_com_factory::DLL_AddRef(&Create_ICaptureImage);
	};
	~ICaptureImage_mon_webcam_impl(){
			delete pScreen;
			delete pWebCam;
			free_com_factory::DLL_Release(&Create_ICaptureImage);
	}

	LONG refcount;
	dummy_dispatch_t ddisp;
	WebCamInfo * pWebCam;
	ScreenCapture* pScreen;
	BITMAPFILEHEADER* pbmp;
	int mode;
	int f24,fyut2;
	v_buf<char> buf,buf2;

};

extern "C" HRESULT  Create_ICaptureImage(ICaptureImage** ppimage)
{
	
	return ICaptureImage_mon_webcam_impl<>::CreateInstance(ppimage);

}
//template<class CH,class Args>
//console_previewer_shared_image_t<>* start_console_previewer_shared_image(Args& args,const CH* keyc=0,bool fasyn=true)


extern "C" void*  start_console_previewer_shared_imageA(const char* argszz,const char* key=0,bool fasyn=true)
{
	argv_zz<char> args(argszz);

	//start_console_previewer_shared_image
	return start_console_previewer_shared_image(args,key,fasyn);

}

extern "C" void*  start_console_previewer_shared_imageW(const wchar_t* argszz,const wchar_t* key=0,bool fasyn=true)
{
	argv_zz<wchar_t> args(argszz);

	//start_console_previewer_shared_image
	return start_console_previewer_shared_image(args,key,fasyn);

}


extern "C" void __stdcall  startW(HWND hwnd, HINSTANCE hinst,LPWSTR lpszCmdLine,int nCmdShow)
{

	v_buf<wchar_t> pl= argv_ini<wchar_t>().get_module_path();
	 pl<<L";"<<argv_env<wchar_t>()[L"PATH"];
	 wchar_t* ppl=pl;
	 SetEnvironmentVariableW(L"PATH",pl.get());

	 capture_image_helper_t capture_image;

	
	typedef ISharedImage::byte_t byte_t ;
	{
     //shared_image_helper_t sss;
	}
	v_buf<byte_t> tmp;
	byte_t* pbuf=0;
/*
	char bufcd[1111];
	GetCurrentDirectoryA(1000,bufcd);
	argv_env<wchar_t> ae;
	wchar_t* pecd=ae[L"CD"];
	*/

	HRESULT hr=E_POINTER,hr0,hr_ci=E_POINTER;
    argv_zzs<wchar_t> args;
     int fcons=startup_console_attr(L"console.GUI",L"services.console.",1);
     set_args_fei(argv_cmdline<wchar_t>(lpszCmdLine),args);





	 wchar_t* key=args[s_m(key)];
     wchar_t* capture=args[s_m(capture)];

     bool fcapture=safe_len(capture);
	 bool fshared_key=args[s_m(key.shared)].def<bool>(0);
       

	 if((safe_cmpni(L"net",capture,3)==0)||(safe_cmpni(L"ip",capture,2)==0))
	 {
        //start_shared_desktop_client(argv_cmdline<wchar_t>(lpszCmdLine));
		 start_shared_desktop_client(args);
		return ;
	 }

      if((!fshared_key)&&fcapture) singleton_restarter_t<> sr(key);

	 if(safe_cmpni(L"decklink",capture,8)==0)
	 {
		 #ifdef _DECKLINK_CAPTURE_SI
             double shrink=args[s_m(viewer.shrink)].def<double>(0.0);
              console_previewer_shared_image_t<>* previewer =0;
			 if(shrink>0.0001) 
			 {
				 previewer= new console_previewer_shared_image_t<>(key,args[s_m(viewer.flip)].def<int>(0));
				 previewer->smode=args[s_m(viewer.smode)].def<int>(HALFTONE);

			 }
			 shared_image_helper_t shared_image;
			 hr=shared_image->Init2(key);
			 hr=shared_image->SetOptions(args.flat_str());

			 DeckLinkCapture_t<ISharedImage,console_previewer_shared_image_t<> > DeckLinkCapture(args,shared_image,previewer);

             DeckLinkCapture.run();
           #endif
			 return ;
             

	 }


	 


	// Capture_base* pcapture=0;
	 void * pcapture=0;
	 ScreenCapture* pScreen=0;
	 WebCamInfo* pWebCam=0;

	 int nframes=args[s_m(nframes)].def<int>(1);

	 double fps=args[s_m(fps)].def<double>(8);
     bool f24=args[s_m(f24)].def<int>(0);
     wchar_t* compress=args[s_m(compress)];

	 bool fyut2=safe_cmpni(L"yuy2",compress,4)==0;
	 
	 //f24=0;
      BITMAPFILEHEADER* pbmp=0;

      ISharedImage_impl<>* pshared_image=0;  
	 if(fcapture)
	 {
		 
		 int nmon=args[s_m(monitor.n)].def<int>(args[s_m(nmon)].def<int>(0));

		 if(safe_cmpni(capture,L"webcam",6)==0)
		 {
                pWebCam=new WebCamInfo;
				if(pWebCam->hr)
				{
					 hr=pWebCam->hr;
					_cwprintf(L"err=%d ; %s",hr,(wchar_t*) error_msg(hr));
						Sleep(10000);
				}
                 pWebCam->update();
				 //Sleep(1000);
                 pbmp=pWebCam->decompressor.bmpmap.get_BITMAPFILEHEADER();
				pcapture=pWebCam;
		 }
		 else if(safe_cmpni(capture,L"monitor",7)==0)
		 {
			 if(0){ 
		   pScreen=new ScreenCapture(nmon);
		   
		   
            pScreen->update();
			 if(f24) pScreen->bmpmap.decrease_toRGB24();
			 pbmp=pScreen->bmpmap.get_BITMAPFILEHEADER();
			 if(fyut2) 
			 {
				 int cb=pScreen->bmpmap.image_size_b()+128;
				 tmp.resize(cb);
				 pbuf=tmp;
				 if(BMPYUY2_converter::compressYUY2(cb,(char*)pbmp,&cb,(char*)pbuf))
					 pbmp=(BITMAPFILEHEADER*)pbuf;
			 }
			 }
			 //pcapture=pScreen;
			 hr=hr_ci=capture_image->Init(args.flat_str());
			 pcapture=capture_image;
		 }
		 
		 



         if(pcapture)  
		 {

		 

		   //BITMAPINFO* pbmi= *pcapture;
		   //BITMAPFILEHEADER* 
			 //shared_image_helper_t shared_image;


			 //hr=shared_image->Init2(key,1920,1200,2);

			 

		   pshared_image=new ISharedImage_impl<>;
		   hr=pshared_image->Init(key,pbmp,nframes);
		   hr=pshared_image->SetOptions(args.flat_str());
		 }



	 }

     



	 if(fcons)
	 {
		 double shrink=args[s_m(viewer.shrink)].def<double>(0.0);
		 double viewer_fps=args[s_m(viewer.fps)].def<double>(25);

		 //ISharedImage_impl<>* pimsh=new ISharedImage_impl<>;
         
          //hr0=pimsh->Init(key);

		 if((shrink>0.00001))
		 {
             /*
			 console_previewer_image_t<>* previewer =new console_previewer_image_t<>(pimsh->bmpmap);
			 previewer->dib.fstretchDIB=1;
			 previewer->dib.fflip=args[s_m(viewer.flip)].def<int>(-1);
			 */
			 console_previewer_shared_image_t<>* previewer =new console_previewer_shared_image_t<>(key);
			 previewer->smode=args[s_m(viewer.smode)].def<int>(HALFTONE);
			 previewer->flipV=args[s_m(viewer.flip)].def<int>(previewer->flipV);
			 previewer->flipV=args[s_m(viewer.flipV)].def<int>(previewer->flipV);
			 previewer->run(fcapture,shrink,viewer_fps);
		 }

	 }

            if(safe_cmpni(capture,L"frameseq",7)==0)
			{
                int nframes=args[s_m(frameseq.count)].def<int>(-1);	

				shared_image_saver_t<> sis(key,(wchar_t*)args[s_m(frameseq.fmt)]);
				sis.shared_image.set_options(args);
				sis.run(fps,nframes);

				return   ;
			}




	  if(!hr)
	  {

		  DWORD tio=double(1000)/fps;
		  typedef _non_copyable::CStopwatch  stopwatch_t;
		  stopwatch_t swt;
		  
		  double t,tf=1./fps;
		  double fcbev=0,alpha=tf;
		  double efps=0,_fps=fps;

		  swt.Start();

         for(;;)  
		 {

			 byte_t* pb=0;
			 BITMAPFILEHEADER* pbmp=0;
			 
          //  pcapture->update();

			 if(SUCCEEDED(hr_ci))
			 {
				 ICaptureImage::PFILEINFO pfi=0;
				 int ii,jj=sizeof(ICaptureImage::FILEINFO),kk=sizeof(BITMAPFILEHEADER);
                 if(SUCCEEDED(hr=capture_image->GetFrame(&pfi,&ii)))
					 pbmp=&pfi->bf;
			 }
			 else if(pScreen)
			 {
				 BITMAPINFOHEADER& bmih=pScreen->bmpmap.header<BITMAPINFOHEADER>();
				 pScreen->update();
           
             if(!pScreen-> crc32_check()) 
		    	{
 			      if(f24) pScreen->bmpmap.decrease_toRGB24();
				  if(fyut2) 
				  {
					  pbuf=tmp;
					  int cb=tmp.size_b();
					  char* ps=(char*)pScreen->bmpmap.get_BITMAPFILEHEADER();
					  if(BMPYUY2_converter::compressYUY2(cb,(char*)ps,&cb,(char*)pbuf))
					  pbmp=(BITMAPFILEHEADER*)pbuf;
				  }
				  pb=pScreen->bmpmap.pBits(); 
			 }
		 }
			 else if(pWebCam)
			 {
				 pWebCam->update();
                 
				 pb=pWebCam->decompressor.bmpmap.pBits(); 

			 }
             
            if(pbmp) pshared_image->PushFrame(pbmp,0);
			 else pshared_image->PushFrame(0,pb);


			 double dt=swt.Sec(),fcb=1000000000000,tt=max(dt,tf);

			 if(tt>0) {
				 
				 _fps=1/tt;
			 }
			 
			 efps=(1-alpha)*efps+alpha*(_fps);


			 //SetConsoleTitleA(v_buf<char>().printf("fps::kbits/sec: <%g>::<%g> %g::%g ",efps,fcbev,_fps,fcb));
			 SetConsoleTitleA(v_buf<char>().printf("fps: <%g> %g ",efps,_fps));
			 t=tf-dt;
			 swt.Start();
			 if(t>0) Sleep(t*1000);

			 //Sleep(tio);

		 }

	  }

	

}

