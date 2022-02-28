#pragma once
#include "video/webcam_capture.h"
#include "video/lasercalibrate.h"
#include "video/ishared_image.h"
//#include "AviSynth.h"
#include "AviSynth_templates.h"



extern HANDLE hGlobalEventAbort;

class FileMapCap2 : public IClipMap_impl<> {

	
	VideoInfo vi;
	PVideoFrame frame;
	PVideoFrame ttf;
	IScriptEnvironment* penv;
	unsigned cmask;
	int mx,my;
     bool flipV;
	 int fps_fix;
	 v_buf<wchar_t> key,path;
	 double back_fps_ev,fps_set;
	 ULONG32 crc32;
	 RECT crop_r;
	 int crop_offset;
	 int frames_cache;
	 double lag_effect;

	 _non_copyable::CStopwatch g_cs;
	laser_shared_arrea_t laser_shared_arrea;

    shared_image_helper_t shared_image_tmp;
	shared_image_helper_t shared_image;

	typedef FileMapCap2 owner_t;
/*
      struct WebCamClip:WebCamCapture<WebCamClip>
	  {
            owner_t *powner;
		  WebCamClip(owner_t *t,int ndriv,int fdlg):WebCamCapture<WebCamClip>(ndriv,fdlg),powner(t){}


		  void update_frame(unsigned char* ppBits)
		  {
            //  powner->update_callback(ppBits);
		  }

			  


	  } camCap;
*/
      //WC_filemap_ext<fake_detector> camCap;         

//	
	  ;


	  

	enum { Hz = 440 } ;

public:

AVSValue getter;


inline PVideoFrame&	  current_frame()
{
	return frame;
}


	~FileMapCap2() {

		

	}

	FileMapCap2(IScriptEnvironment* env,const char* _key,float fps,bool _flipV
		,float num_frames,const char* _path,RECT _rcrop,float _fix_fps,float _frames_cache,float time_lag,double _lag_effect=1)
		:cmask(0),flipV(_flipV)//camCap(this,ndriv,fdialog)
	{
        lag_effect=_lag_effect;
		if(time_lag>0)
		{
           _frames_cache=fps*time_lag;

		}

		frames_cache=_frames_cache;
		crop_r=_rcrop;
		key.cat(char_mutator<CP_THREAD_ACP>(_key));
        path.cat(char_mutator<CP_THREAD_ACP>(_path));
		getter=new CurrentGetter<FileMapCap2>(this);

		memset(&vi, 0, sizeof(VideoInfo));
		vi.fps_numerator = fps*1000;
		vi.fps_denominator = 1001;
		vi.num_frames = num_frames;   // 1 hour
		//vi.pixel_type = VideoInfo::CS_BGR32;
		vi.pixel_type = VideoInfo::CS_BGR24;
		reset_VideoFrame(env);
		g_cs.Start();
		back_fps_ev=0;
		fps_set=fps;
		fps_fix=_fix_fps;
		crc32=0;

	}

  bool reset_VideoFrame(IScriptEnvironment* env)
	{
        
		shared_image=shared_image_helper_t(key.get());
		if(!shared_image) env->ThrowError("camCap: Error %d ",shared_image.hr);
		shared_image->SetOptionsA(v_buf<char>().printf("filters.frames.cache=%d\0\0",frames_cache));
		shared_image->SetOptionsA(v_buf<char>().printf("filters.frames.effect=%f\0\0",lag_effect));


        BMP_helper_t bmp(shared_image);

	    

		int w, h;

		//if(!camCap.get_bmpmap().check_resize())			env->ThrowError("key mapping  bad or not exists!");

	
		
		BITMAPINFO * pbmi=bmp.hdr_info();

          if(!pbmi) 
			  return false;
         
		  

		int nBPP=pbmi->bmiHeader.biBitCount;

		//if((nBPP!=32))			env->ThrowError("camCap:  BPP must be  32");

		//if((nBPP!=32)&&(nBPP!=24))			env->ThrowError("camCap:  BPP must be 24 or 32");

		
		if((nBPP!=24)&&(nBPP!=16)&&(nBPP!=32))			env->ThrowError("camCap:  BPP must be 24 or 16 ? 32");

		if(nBPP==16)
		   vi.pixel_type = VideoInfo::CS_YUY2;
		else if(nBPP==32) 
			vi.pixel_type = VideoInfo::CS_BGR32;

		w=bmp.width();
		h=bmp.height();


        if(crop_r.right<=0) crop_r.right+=w;
		if(crop_r.bottom<=0) crop_r.bottom+=h;
		
		

         // crop_r
		crop_r=_inrect_cl(crop_r,make_rect(0,0,w,h));
		

		if(vi.pixel_type == VideoInfo::CS_YUY2)
		{
          crop_r.left= (crop_r.left/2)*2;
		  crop_r.top= (crop_r.top/2)*2;
		  crop_r.right= (crop_r.right/2)*2;
		  crop_r.bottom= (crop_r.bottom/2)*2;
		}

		vi.width = crop_r.right-crop_r.left;
		vi.height = crop_r.bottom-crop_r.top;

		int cam_pitch=bmp.linesize_b();
		//linesize_dest*(h)+crop_offset;
		if(flipV) crop_offset=cam_pitch*(h-crop_r.top)+crop_r.left;
		else crop_offset=cam_pitch*crop_r.top+crop_r.left;

		//vi.width=w;
		//vi.height=h;

      	frame = env->NewVideoFrame(vi);
		return true;
       
	}

	inline  PVideoFrame Update(int n, IScriptEnvironment* env) 
	{

		penv=env;
		        

		if(WaitForSingleObject(hGlobalEventAbort,10)==WAIT_OBJECT_0)
			env->ThrowError("Abort");
 
		     BMP_helper_t bmp(shared_image);

			 int fupd=-1;
         

          bool f=env->MakeWritable(&frame);
         
		  
			  

		  	update_callback(bmp.bits<BYTE>(),bmp.linesize_b());

		

		



      
		{
            char buf[64];
			double t=g_cs.Sec();
            if(t>1e-10)
			{
			//	double fps=1./t;
				back_fps_ev=(1.-(1./25.))*back_fps_ev+(1./25.)*t;
				double fps=safe_back_abs(t),fpse=safe_back_abs(back_fps_ev);
				
				char cc[]="+=";

				//if(0)
				if(fps_fix)
				{
					int ic=1;
					if(fpse>fps_set)
					{
                       double ti=(1.0/fps_set)-(1.0/fpse);
					  
					   if(ti>0) {
						   Sleep(ti);
						   fpse=fps_set;
						   if(fps_fix==2)
							   back_fps_ev=fps_set;


						   ic=0;
					   }

					}

					if(is_console()){	
						sprintf(buf,"force:[%c] <fps>=%5.2f fps=%5.2f upd=%d",cc[ic],fpse,fps,fupd);
						SetConsoleTitleA(buf);
					}

				}
				else
				{
					if(is_console()){	
						sprintf(buf,"<fps>=%5.2f fps=%5.2f upd=%d",fpse,fps,fupd);
						SetConsoleTitleA(buf);
					}
				}

			}
			g_cs.Start();
		}


		


		return  frame;
	}


		inline  void update_callback(BYTE* pBits,int  linesize_scr)
		{
                
				BYTE* pdest = (BYTE*)frame->GetWritePtr();
				const int linesize_dest = frame->GetPitch();
				const int rowsize = frame->GetRowSize();
				//int  linesize_scr=camCap.line_sizeb();
				int h=vi.height;

				BYTE* pd=pdest;
				BYTE* ps=pBits+crop_offset;

				if(flipV)
					for(int y=0;y<h;y++)
					{
						ps-= linesize_scr;
						memcpy(pd,ps,rowsize);
						pd+=linesize_dest;                        
					}
				else
					for(int y=0;y<h;y++)
					{			
						memcpy(pd,ps,rowsize);                     
						pd+=linesize_dest;
						ps+= linesize_scr;
					}	

		}


	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env){ 

		return Update(n, env) ;
	}
	bool __stdcall GetParity(int n) { return false; }
	const VideoInfo& __stdcall GetVideoInfo() { 
		return vi; 
	}
	void __stdcall SetCacheHints(int cachehints,int frame_range) { };

	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { };
//FileMapCap2(char* key,float fps,float num_frames,char* path,IScriptEnvironment* env)

	static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {

		//get_AVSFloat(
     ///*
		double fps=get_AVSFloat(args[1],25);
		int flipV=get_AVSFloat(args[2],0)!=0;
		int numframes=get_AVSFloat(args[3],3*107892);
		const char* path=0;
		if(args[4].IsString()) path=args[4].AsString("");
		 RECT r;
		 r.left=get_AVSFloat(args[5],0);
		 r.top=get_AVSFloat(args[6],0);
		 r.right=get_AVSFloat(args[7],0x0FFFFFFF);
		 r.bottom=get_AVSFloat(args[8],0x0FFFFFFF);
		 float fixfps=get_AVSFloat(args[9],0);
		 float frame_cache=get_AVSFloat(args[10],0);
		 float time_lag=get_AVSFloat(args[11],-111);
		 float lag_effect=get_AVSFloat(args[12],0);
	//	 */
		RECT rr={0,0,11111111,1111111111};

        //
		return new FileMapCap2(env,args[0].AsString(""),fps,flipV,numframes,path,r,fixfps,frame_cache,time_lag,lag_effect);

		//		return new FileMapCap2(env,args[0].AsString(""),args[1].AsFloat(25),args[2].AsFloat(1),args[3].AsFloat(3*107892),args[4].AsString(""),rr);
	}
};
