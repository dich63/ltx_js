#pragma once
#include "video/webcam_capture.h"
#include "video/lasercalibrate.h"
//#include "AviSynth.h"
#include "AviSynth_templates.h"



extern HANDLE hGlobalEventAbort;

class FileMapCap : public IClipMap_impl<> {

	
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

	 _non_copyable::CStopwatch g_cs;
	laser_shared_arrea_t laser_shared_arrea;

	typedef FileMapCap owner_t;
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
      WC_filemap_ext<fake_detector> camCap;         

//	
	  ;


	  

	enum { Hz = 440 } ;

public:

AVSValue getter;


inline PVideoFrame&	  current_frame()
{
   return frame;
}

	~FileMapCap() {

		camCap.stop();

	}

	FileMapCap(IScriptEnvironment* env,const char* _key,float fps,bool _flipV
		,float num_frames,const char* _path,RECT _rcrop,float _fix_fps)
		:cmask(0),camCap(fake_detector()),flipV(_flipV)//camCap(this,ndriv,fdialog)
	{

		crop_r=_rcrop;
		key.cat(char_mutator<CP_THREAD_ACP>(_key));
        path.cat(char_mutator<CP_THREAD_ACP>(_path));
		getter=new CurrentGetter<FileMapCap>(this);

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
        
		{
			BMPMap bm;

			bm.open_key(key,path);
			camCap.open(bm);
		}

	    

		int w, h;

		//if(!camCap.get_bmpmap().check_resize())			env->ThrowError("key mapping  bad or not exists!");

		if(camCap.hr!=0) env->ThrowError("camCap: Error %d ",camCap.hr);

		
		BITMAPINFO * pbmi=camCap.pInfo();

          if(!pbmi) 
			  return false;
         
		  camCap.pInfo_cache(pbmi);

		int nBPP=pbmi->bmiHeader.biBitCount;

		//if((nBPP!=32))			env->ThrowError("camCap:  BPP must be  32");

		//if((nBPP!=32)&&(nBPP!=24))			env->ThrowError("camCap:  BPP must be 24 or 32");

		
		if((nBPP!=24)&&(nBPP!=16)&&(nBPP!=32))			env->ThrowError("camCap:  BPP must be 24 or 16 ? 32");

		if(nBPP==16)
		   vi.pixel_type = VideoInfo::CS_YUY2;
		else if(nBPP==32) 
			vi.pixel_type = VideoInfo::CS_BGR32;

		w=camCap.width();
		h=camCap.height();


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

		int cam_pitch=camCap.line_sizeb();
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
 
		     BMPMap& bmpmap=camCap.get_bmpmap();   

			 
         if(0)
		 {
             locker_t<BMPMap> lock(bmpmap);
			 
               if(!bmpmap.check_resize(camCap.pInfo_cache()))
				   if(!reset_VideoFrame(env)) 
					    return frame;
		 }

          bool f=env->MakeWritable(&frame);
         
		  
		
	//  	if(camCap.update()!=S_OK) env->ThrowError("scrCap: Error %x",camCap.hr);
		  POINT cursor={0,0};
		  bool fupd=true;
		  BMPMap::remote_control_data_t* rcd;

		{
		        	

			locker_t< WC_filemap_ext<fake_detector> > lock(camCap);

			

            if(bmpmap.nframes==0)
			{
				rcd=bmpmap.remote_control_data();
				fupd=rcd->crc32!=make_detach(crc32,rcd->crc32);
				cursor=rcd->cursor;
			}
			
			//update_callback((unsigned char*)camCap.pBits);
			if(fupd)	update_callback(camCap.get_pBits<unsigned char>());
			 //cursor=*((POINT*)camCap.get_bmpmap().pAddData());

		}
		

		if(bmpmap.nframes==0)
		{
	
		set_value("screen.cursor.x",AVSValue(float(cursor.x)));
		set_value("screen.cursor.y",AVSValue(float(cursor.y)));
		}
            //_non_copyable::CStopwatch cs;



      
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


		//env->SetGlobalVar("LASERX",AVSValue(int(mx)));
		//env->SetGlobalVar("LASERY",AVSValue(int(my)));



		//ttf=

		//AVSValue avs=
        

/*
		AVSValue args[3] = { getter,100, 100};

		AVSValue  ares=env->Invoke("BilinearResize", AVSValue(args,3));
         ttf=ares.AsClip()->GetFrame(n,env); 
		const VideoInfo& tvi =ares.AsClip()->GetVideoInfo();

*/
		 //return  ttf;

		return  frame;
	}


		inline  void update_callback(BYTE* pBits)
		{
                
				BYTE* pdest = (BYTE*)frame->GetWritePtr();
				const int linesize_dest = frame->GetPitch();
				const int rowsize = frame->GetRowSize();
				int  linesize_scr=camCap.line_sizeb();
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
//FileMapCap(char* key,float fps,float num_frames,char* path,IScriptEnvironment* env)

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
	//	 */
		RECT rr={0,0,11111111,1111111111};

        //
		return new FileMapCap(env,args[0].AsString(""),fps,flipV,numframes,path,r,fixfps);

		//		return new FileMapCap(env,args[0].AsString(""),args[1].AsFloat(25),args[2].AsFloat(1),args[3].AsFloat(3*107892),args[4].AsString(""),rr);
	}
};
