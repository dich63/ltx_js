#pragma once

#include "screencapture.h" 
//#include "AviSynth.h"
#include "AviSynth_templates.h"

#include "video/lasercamutils.h" 

class ScreenCap : public IClipMap_impl<>  {
 
	VideoInfo vi;
    PVideoFrame frame;
    ScreenCapture scrCap;
	laser_shared_arrea_t laser_shared_arrea;

 
  enum { Hz = 440 } ;

public:

  ~ScreenCap() {
    
  }

  ScreenCap(int monN, float fps,float num_frames,IScriptEnvironment* env)
	  :scrCap(monN)
  {


	  int w, h;
	  if(scrCap.hr!=0) env->ThrowError("scrCap: Error %x",scrCap.hr);

      int nBPP=scrCap.bip.bmiHeader.biBitCount;

	  if((nBPP!=32))
		  env->ThrowError("scrCap: Screen BPP must be  32");

	  if((nBPP!=32)&&(nBPP!=16))
		  env->ThrowError("scrCap: Screen BPP must be 16 or 32");

	  w=scrCap.width();
	  h=scrCap.height();

       

    memset(&vi, 0, sizeof(VideoInfo));
    vi.width = w;
    vi.height = h;
    vi.fps_numerator = fps*1000;
    vi.fps_denominator = 1001;
    vi.num_frames = num_frames;   // 1 hour
    vi.pixel_type = VideoInfo::CS_BGR32;
    


    frame = env->NewVideoFrame(vi);
    unsigned* p = (unsigned*)frame->GetWritePtr();
    const int pitch = frame->GetPitch()/4;
  }


inline  PVideoFrame Update(int n, IScriptEnvironment* env) 
  {
        //virtual bool __stdcall MakeWritable(PVideoFrame* pvf) = 0;
       if(scrCap.update()!=0) env->ThrowError("scrCap: Error %x",scrCap.hr);

	   int cx=scrCap.cursor.x;
	   int cy=scrCap.cursor.y;

	   env->SetGlobalVar("SCREENCURSORX",AVSValue(cx));
	   env->SetGlobalVar("SCREENCURSORY",AVSValue(cy));

       int flaser= get_value("laser.detect",AVSValue(0)).AsFloat(0);     
  if(flaser)
  {
	   double laserx,lasery;
	   int laser_active; 
	   {
           locker_t<laser_shared_arrea_t> lock(laser_shared_arrea);
		   laser_shared_data* lsd=laser_shared_arrea;
		   laser_active=laser_shared_arrea->camera_laser_detector.factive;
		   laserx=laser_shared_arrea->screen_point[0];
		   lasery=laser_shared_arrea->screen_point[1];
	   }
	   
        set_value("screen.laser.active",AVSValue(laser_active));  
	    set_value("screen.laser.x",AVSValue(laserx));
 		set_value("screen.laser.y",AVSValue(lasery));
  }

        set_value("screen.cursor.x",AVSValue(scrCap.cursor.x));
		set_value("screen.cursor.y",AVSValue(scrCap.cursor.y));
		set_value("screen.cursor_g.x",AVSValue(scrCap.cursor_g.x));
		set_value("screen.cursor_g.y",AVSValue(scrCap.cursor_g.y));


		





	   if((n)&&scrCap.crc32_check())
	   {
		   //_cprintf("Frame %d skipped\n",n);
           return    frame;
	   }

	  bool f=env->MakeWritable(&frame);
	  unsigned* p = (unsigned*)frame->GetWritePtr();
	  const int pitchb = frame->GetPitch();
	  const int pitch=pitchb/4;
      
      

	  
	  int h=frame->GetHeight();
      int rsize=frame->GetRowSize();
	  unsigned* pbegin =p;
	  unsigned* psrc=(unsigned*)scrCap.pline(0);
	  int  rsize_scr=scrCap.line_sizeb();

	  memcpy(p,scrCap.pline(0),vi.height*vi.width*4);
if(0)
	  for(int y=0;y<vi.height;++y)
	  {
          //unsigned* p=pbegin+pitch*y;
		  unsigned* psrc=(unsigned*)scrCap.pline(y);
		  for(int x=0;x<vi.width;++x)
		  {
			  // p[x]=RGB((y%256),2*((y+x)%128),2*(x%128));//y;//psrc[x];
			  //
			 // 
			  p[x]=psrc[x];
		  }
         
		  p=(unsigned*)(((char*)p)+rsize);
		  psrc=(unsigned*)(((char*)psrc)+rsize_scr);
		  // p+=pitch;
	  }



    return  frame;


  }

  //PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return frame; }

PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env){ 

	return Update(n, env) ;
 }
  bool __stdcall GetParity(int n) { return false; }
  const VideoInfo& __stdcall GetVideoInfo() { return vi; }
  void __stdcall SetCacheHints(int cachehints,int frame_range) { };

  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { };
  
  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {
    return new ScreenCap(args[0].AsInt(0),args[1].AsFloat(5),args[2].AsFloat(3*107892), env);
  }
};
