#pragma once

//#import "libid:D864517A-EDD5-466D-867D-C819F1C052BB" version("1.0") 
#include "DeckLinkAPI_h.h"
#include "free_com_factory.h"
#include "AtlExt/MBVLocalHost.h"
#include "wchar_parsers.h"
#include "singleton_utils.h"
//#include "static_com.h"

//#include "os_utils.h"
//#include "IShared_Image.h"
/* lcid("9")
[uuid(D864517A-EDD5-466D-867D-C819F1C052BB),
version(1.0), helpstring("DeckLink API Library")]*/
 
//#define 
#define s_m(s) L#s
#define s_pairm(s) {L#s,s}

#define BNS (1000000000)
#define  NS 1./double(BNS)


template<class _SharedImage,class _Previewer>
struct DeckLinkCapture_t
{
      
	

	class CStopwatch
	{
	public:
		CStopwatch() {
			QueryPerformanceFrequency(&m_liPerfFreq);
			Start(); 
		}

		inline	 void Start() {
			QueryPerformanceFrequency(&m_liPerfFreq);
			QueryPerformanceCounter(&m_liPerfStart); 
		}

		inline long double Sec()
		{
			LARGE_INTEGER liPerfNow;
			
			QueryPerformanceCounter(&liPerfNow);
			return (long double)( liPerfNow.QuadPart - m_liPerfStart.QuadPart ) / (long double)(m_liPerfFreq.QuadPart);
		}
		inline LONGLONG Tic()
		{
			LARGE_INTEGER liPerfNow;
			QueryPerformanceCounter(&liPerfNow);
			return (LONGLONG)( liPerfNow.QuadPart - m_liPerfStart.QuadPart );
		}
		inline LONGLONG Frec()
		{
			QueryPerformanceFrequency(&m_liPerfFreq);
			return  m_liPerfStart.QuadPart;
		}
	private:
		LARGE_INTEGER m_liPerfFreq;
		LARGE_INTEGER m_liPerfStart;
	};


       typedef _SharedImage shared_image_t;
	   typedef _Previewer previewer_t;
	   //typedef  MBVLocalHost::com_ptr com_ptr_t;

       previewer_t* viewer;
	   shared_image_t* shared_image;
	   free_com_factory::COMInit cominit;
	   MBVLocalHost::com_ptr<IDeckLinkIterator> Iterator; 
	   MBVLocalHost::com_ptr<IDeckLinkInput>  input;
	   long long frameCount,framemodulo; 
	   double shrink,viewer_fps,fps,tic,tfps,fpse,afps;
	   CStopwatch cs;
	   console_shared_data_t<wchar_t,0> csd;

  
	   HRESULT hr;


	   struct cb_delegate_t:IDeckLinkInputCallback //<cb_delegate_t,&__uuidof(IDeckLinkInputCallback)>
	   {


		   
		   DeckLinkCapture_t* owner;
		   cb_delegate_t(DeckLinkCapture_t* _owner):owner(_owner){};

		   STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
		   {
			   if ((riid == IID_IUnknown) || (riid == __uuidof(IDeckLinkInputCallback)))
			   {
				   *ppvObject = this;
				   AddRef();
				   return S_OK;
			   }
			   return E_NOINTERFACE;
		   }
		   STDMETHOD_(ULONG,AddRef)() {		   return 2;   }
		   STDMETHOD_(ULONG,Release)() {	   return 2;   }
		   

		   virtual HRESULT STDMETHODCALLTYPE	VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode* newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags){
			   return S_OK;
		   };
		   virtual HRESULT STDMETHODCALLTYPE	VideoInputFrameArrived(IDeckLinkVideoInputFrame* pArrivedFrame, IDeckLinkAudioInputPacket*)
		   {
                return owner->OnCaptureFrame(pArrivedFrame);
		   };


	   };

      cb_delegate_t cb_delegate;


template <class CH>
BMDDisplayMode getDisplayModeDef(CH* name,BMDDisplayMode dflt=bmdModeHD1080p25) 
{
	struct p2_t
	{
      CH* n;
     BMDDisplayMode m;
	};
	p2_t p2[]=
	{	s_pairm(bmdModeNTSC	),
		s_pairm(bmdModeNTSC2398	),
		s_pairm(bmdModePAL),
		s_pairm(bmdModeNTSCp),
		s_pairm(bmdModePALp	),
		s_pairm(bmdModeHD1080p2398),
		s_pairm(bmdModeHD1080p24),
		s_pairm(bmdModeHD1080p25),
		s_pairm(bmdModeHD1080p2997),
		s_pairm(bmdModeHD1080p30),
		s_pairm(bmdModeHD1080i50),
		s_pairm(bmdModeHD1080i5994),
		s_pairm(bmdModeHD1080i6000),
		s_pairm(bmdModeHD1080p50),
		s_pairm(bmdModeHD1080p5994),
		s_pairm(bmdModeHD1080p6000),
		s_pairm(bmdModeHD720p50	),
		s_pairm(bmdModeHD720p5994),
		s_pairm(bmdModeHD720p60),
		s_pairm(bmdMode2k2398),
		s_pairm(bmdMode2k24),
		s_pairm(bmdMode2k25),
		s_pairm(bmdModeUnknown)
		};

	int i=0;
	while(p2[i].m!=bmdModeUnknown)
	{
        if(safe_cmpni(name,p2[i].n,20)==0)
			return p2[i].m;

			i++;
	}
	

  return dflt;
}

template<class Agrs>
DeckLinkCapture_t(Agrs& args, shared_image_t* shi,previewer_t* pv):
shared_image(shi),viewer(pv),cb_delegate(this),frameCount(0)
{
    
	 shrink=args[s_m(viewer.shrink)].def<double>(0.0);
	 viewer_fps=args[s_m(viewer.fps)].def<double>(25);
	 fpse=fps=args[s_m(fps)].def<double>(25);
	 afps=1./fps;
	 tfps=1./fps;
	 framemodulo=args[s_m(DeckLink.fpsdiv)].def<double>(1);
	 tic=GetTickCount();

   MBVLocalHost::com_ptr<IDeckLink> dl;
   int ncard=args[L"DeckLink.ncard"].def(0);
   hr=CoCreateInstance(__uuidof(CDeckLinkIterator),NULL,CLSCTX_ALL,__uuidof(IDeckLinkIterator),Iterator.ppQI());
   int i=0; 
   if(FAILED(hr)) return;
   hr=E_NOTIMPL;
   do{
	   if(Iterator->Next(&dl.p) != S_OK)
		    return;
	   if(i==ncard)
	   {
		   if((hr=dl->QueryInterface(__uuidof(IDeckLinkInput),input.ppQI()))!=S_OK)
			    return;
		   BMDDisplayMode displayMode=getDisplayModeDef(args[L"DeckLink.mode"].def(L"bmdModeHD1080p25"));

           bstr_t mn,dn;   
		   hr=dl->GetModelName( mn.GetAddress());	 
		   hr=dl->GetDisplayName( dn.GetAddress());			   
		   wchar_t* pmn=mn,* pdn=dn;
      
		   csd.printf(M_CC,L"ncard=%d model=\"%s\" display=\"%s\"\n",i,pmn,pdn);
		   hr = input->SetCallback(&cb_delegate);
		   if(hr==S_OK)
		   hr=input->EnableVideoInput(displayMode, bmdFormat8BitYUV, 0);

		   break;
	   }
       ++i;

   }while(1);


}

~DeckLinkCapture_t(){  if(hr==S_OK) hr = input->StopStreams();  }

HRESULT run()
{
	if(FAILED(hr)) return hr;
   
   if(hr==S_OK) hr = input->StartStreams();
   if(hr==S_OK) SleepEx(-1,true);
}


HRESULT OnCaptureFrame(IDeckLinkVideoInputFrame* videoFrame)
{
	HRESULT hr=0;
	unsigned char*					frameBytes;
	long rsize,height,width;
	if(!videoFrame) return hr;
/*
	if ( (videoFrame->QueryInterface(IID_IDeckLinkVideoFrame3DExtensions, (void **) &threeDExtensions) != S_OK) ||
		(threeDExtensions->GetFrameForRightEye(&rightEyeFrame) != S_OK))
	{
		rightEyeFrame = NULL;
	}

	if (threeDExtensions)
		threeDExtensions->Release();
*/
    DWORD fgf;    
	if ((fgf=videoFrame->GetFlags()) & bmdFrameHasNoInputSource)
	{
		//fprintf(stderr, "Frame received (#%lu) - No input signal detected\n", frameCount);
		csd.printf(M_CC,L"Frame received (%d) - No input signal detected\n", frameCount);
		return S_OK;
	}


	hr=videoFrame->GetBytes((void**)&frameBytes);
	if(hr) return hr;
	rsize=videoFrame->GetRowBytes();
	height=videoFrame->GetHeight();
	width=videoFrame->GetWidth();
	if(frameCount==0)
	{
		tic=GetTickCount();
		if(hr=shared_image->Init2(NULL,width,height,2))
			return hr;
		if((viewer)&&(shrink>0.0001)&&(viewer_fps>0))
		{
			viewer->run(true,shrink,viewer_fps);
		}

	}
	frameCount++;
	if(frameCount%framemodulo==0) 
	hr= shared_image->PushFrame(0,frameBytes+3);
	return hr;
  //   double t= GetTickCount();
	 //t-=tic;
	 double t=cs.Sec();
	 double fpsc=1./t;
	 double dt=tfps-t; 
	 fpse+=afps*(fpsc-fpse);
	 SetConsoleTitleA(bufT<char>().printf("[%d]fpsc=%3.3g <%3.3g> [%3.3g]",GetCurrentThreadId(),fpsc,fpse,fps));
	  cs.Start();
	 if(dt>0.005)
	 {
		 //fpsc=double(1)/t;	 
		 int idt=1000*dt;
		 Sleep(idt);	       
	 }
    //
	 
	   return hr;
};

};