#pragma once
#include "video/webcam_capture.h"
#include "video/lasercalibrate.h"
//#include "AviSynth.h"
#include "AviSynth_templates.h"



extern HANDLE hGlobalEventAbort;

class WebCamCap : public IClipMap_impl<> {

	
	VideoInfo vi;
	PVideoFrame frame;
	PVideoFrame ttf;
	IScriptEnvironment* penv;
	unsigned cmask;
	int mx,my;

	

	typedef WebCamCap owner_t;
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
      WC_filemap_only camCap;         

//	
	  ;


	  

	enum { Hz = 440 } ;

public:

AVSValue getter;


inline PVideoFrame&	  current_frame()
{
   return frame;
}

	~WebCamCap() {

		camCap.stop();

	}

	WebCamCap(int ndriv, float fps,float num_frames,int fdialog,int colormask,IScriptEnvironment* env)
		:cmask(colormask),camCap(ndriv)//camCap(this,ndriv,fdialog)
	{

        
		 getter=new CurrentGetter<WebCamCap>(this);

		int w, h;
		if(camCap.hr!=0) env->ThrowError("camCap: Error %d ",camCap.hr);

		int nBPP=camCap.bip.bmiHeader.biBitCount;

		//if((nBPP!=32))			env->ThrowError("camCap:  BPP must be  32");

		if((nBPP!=32)&&(nBPP!=24))
			env->ThrowError("camCap:  BPP must be 24 or 32");

		w=camCap.width();
		h=camCap.height();



		memset(&vi, 0, sizeof(VideoInfo));
		vi.width = w;
		vi.height = h;
		vi.fps_numerator = fps*1000;
		vi.fps_denominator = 1001;
		vi.num_frames = num_frames;   // 1 hour
		//vi.pixel_type = VideoInfo::CS_BGR32;
		vi.pixel_type = VideoInfo::CS_BGR24;



		frame = env->NewVideoFrame(vi);
		unsigned* p = (unsigned*)frame->GetWritePtr();
		const int pitch = frame->GetPitch()/4;
	}

	
	inline  PVideoFrame Update(int n, IScriptEnvironment* env) 
	{

		if(WaitForSingleObject(hGlobalEventAbort,10)==WAIT_OBJECT_0)
			env->ThrowError("Abort");

		 penv=env;
		 bool f=env->MakeWritable(&frame);
	//  	if(camCap.update()!=S_OK) env->ThrowError("scrCap: Error %x",camCap.hr);

		{
			BMPMap::locker_t<WC_filemap_only> lock(camCap);
			//update_callback((unsigned char*)camCap.pBits);
			update_callback(camCap.get_pBits<unsigned char>());
		}
		

		env->SetGlobalVar("LASERX",AVSValue(int(mx)));
		env->SetGlobalVar("LASERY",AVSValue(int(my)));



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


		inline  void update_callback(unsigned char* ppBits)
		{
                //IScriptEnvironment* env=penv;
				//bool f=env->MakeWritable(&frame);
				unsigned* p = (unsigned*)frame->GetWritePtr();
				const int pitchb = frame->GetPitch();
				const int pitch=pitchb/4;
     			//int h=frame->GetHeight();
				int rsize=frame->GetRowSize();
				unsigned* pbegin =p;
				unsigned* psrc=(unsigned*)ppBits;//camCap.pline(0);
				int  rsize_scr=camCap.line_sizeb();
				int nBPP=vi.BitsPerPixel();
				unsigned mask=cmask;
				int w=vi.width;
				int h=vi.height;
				int totsize=h*w*(nBPP/8);
				if(mask==0xFFFFFFFF)
					 memcpy(p,psrc,totsize);
				else
				{


					unsigned char maxgreeen=0;
					 mx=0;
					 my=h-1;
					 
                  if(nBPP==24)
				  {
					  unsigned char* begin=(unsigned char*)p;
					  unsigned char* i=begin;
					  unsigned char* is=(unsigned char*)psrc;
					  unsigned char* end=i+totsize;
					  unsigned char* imax=begin;

					  for(;i<end;is+=3,i+=3)
					  {
						  i[0]=is[0];
						  i[1]=is[1];
						  i[2]=is[2];
						  if(i[1]>maxgreeen)
						  {
							  maxgreeen=i[1];
							  imax=i;
						  }
					  }
                        
					  int mlen=(int(imax)-int(begin))/3;
					  mx=mlen%w;
					  my= h-1- (mlen/w);

				  }
				  else                     
                  if(nBPP==32)
					for(int y=0;y<h;++y)
					{
						//unsigned* p=pbegin+pitch*y;
						//unsigned* psrc=(unsigned*)camCap.pline(y);
						for(int x=0;x<w;++x)
						{
							// p[x]=RGB((y%256),2*((y+x)%128),2*(x%128));//y;//psrc[x];
							//
							// 	p[x]=(0x0000FF00)&psrc[x];
							unsigned src=psrc[x];
							unsigned char gr=((0x0000FF00)&src)>>8;
							if(gr>maxgreeen)
							{
                                maxgreeen=gr;
								mx=x;
								my=h-y-1;
							}



							//p[x]=;
								p[x]=mask&src;
						}

						p=(unsigned*)(((char*)p)+rsize);
						psrc=(unsigned*)(((char*)psrc)+rsize_scr);
						// p+=pitch;
					}
                      
				}


		}

		/*
		//virtual bool __stdcall MakeWritable(PVideoFrame* pvf) = 0;
		

		if((n)&&camCap.crc32_check())
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
		unsigned* psrc=(unsigned*)camCap.pline(0);
		int  rsize_scr=camCap.line_sizeb();

		memcpy(p,camCap.pline(0),vi.height*vi.width*4);
		if(0)
			for(int y=0;y<vi.height;++y)
			{
				//unsigned* p=pbegin+pitch*y;
				unsigned* psrc=(unsigned*)camCap.pline(y);
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

			

			*/


	//PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return frame; }

	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env){ 

		return Update(n, env) ;
	}
	bool __stdcall GetParity(int n) { return false; }
	const VideoInfo& __stdcall GetVideoInfo() { return vi; }
	void __stdcall SetCacheHints(int cachehints,int frame_range) { };

	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { };

	static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {
		return new WebCamCap(args[0].AsInt(0),args[1].AsFloat(5),args[2].AsFloat(3*107892),args[3].AsInt(0),args[4].AsInt(0xFFFFFFFF), env);
	}
};
