#pragma once

//#include "AviSynth.h"
#include "AviSynth_templates.h"
//#pragma comment (lib,"Extras/avisynth.lib")
#include "thread_splitter.h"
#include "orderstat.h"
#include "ipc.h"
#define avs_fast_call   inline 
//__forceinline 

#define in_interval(left,x,right) ((left<=x)&&(x<right))

template <class Version=version>
struct AviSynthFastOverlay:public IClipMap_impl<Version>
{
	typedef unsigned int uint_t;

	typedef typename AviSynthFastOverlay<Version> base_t;

       

	struct MT_handler_t
	{
		base_t* powner;
		typedef  void  (   base_t::*threat_method)(int it, int nt);
        threat_method method;

		
	inline	void operator()(int it, int nt)
		{
			(powner->*method)(it,nt);
		}
	};



       struct ClipRegionData
	   {
		   IClip* pclip;
		   POINT pos;
		   RECT  crop_rect;
           RECT  rect_absolute;
		   int  w;
		   int  h;
		   int  pitch;
		   PClip clip;
		   VideoFrame* pframe;
		   bool fmasked;
		   BYTE *pBits;
           BYTE *py;

		  inline BYTE * calc_py(int y)
		  {
			  y-=pos.y;
              BYTE *p=pBits+pitch*y-pos.x;
			  return py=p;
		  }
          inline DWORD macropxl(int x)
		  {   
			  //int xx=(x-pos.x);
			 return *((DWORD*)(py+x));
		  }
		   
	   };


	   std::vector<ClipRegionData> vcrd;
	   std::vector<ClipRegionData*> pcrd;
	   int clip_count;







		//uint_t beta_num;
		
        VideoInfo vi; 
        AVSValue frame_getter;
        int handled_count;
		DWORD fill_color,fc;
		PVideoFrame frame;
		_non_copyable::CStopwatch cst;
		BYTE * bits_ptr;
		int pitch;


		MT_handler_t mt_handler;
		thread_splitter_t<MT_handler_t,1>  thread_splitter;
		int thread_mode;


		AviSynthFastOverlay():handled_count(-1){};

	 AviSynthFastOverlay(IScriptEnvironment* env,AVSValue vwidth,AVSValue vheight,AVSValue fillclr,AVSValue vfps,AVSValue vnum_frames)
		 :IClipMap_impl<Version>()
		 
	 {

 		 frame_getter=new CurrentGetter<AviSynthFastOverlay>(this);

		 
		 int w=vwidth.AsFloat(256);
		 int h=vheight.AsFloat(w);

		 fill_color=0x7fFF7fFF;
			 //fillclr.AsFloat();
		 //if(fillclr.IsInt()) fill_color=fillclr.AsInt(0);


          memset(&vi,0,sizeof(vi));
         vi.width=w; 
		 vi.height=h;
		 vi.fps_numerator = vfps.AsFloat(25)*1000;
		 vi.fps_denominator = 1001;
		 vi.num_frames = vnum_frames.AsFloat(3*107892);   // 1 hour
		 //vi.pixel_type = VideoInfo::CS_BGR32;
		 vi.pixel_type = VideoInfo::CS_YUY2;
		 frame = env->NewVideoFrame(vi);
		
	 }

	 inline PVideoFrame&	  current_frame()
	 {
		 return frame;
	 }


void filldesk_MT(int it,int nt)
{

   //BYTE * p=((BYTE* )frame->GetWritePtr())+pitch*it;

	
   int count=vi.height/nt;

	BYTE * p=bits_ptr+pitch*it*count;

   int w2=vi.width/2;
   if(it==nt-1)
	   count=vi.height-(count*it);

   for(int y=0;y<count;y++)
   {
	   DWORD* pdw=(DWORD*)p;
	   for(int x=0;x<w2;x++) pdw[x]=fc;
	   p+=pitch;
   }
   

}



inline void filldesk()
{

	  bits_ptr = (BYTE* )frame->GetWritePtr();
      
	  
	  double t;
       cst.Start();

	   if(thread_mode==0)     filldesk_MT(0,1);
	   else
	   {
		   mt_handler.method=&AviSynthFastOverlay::filldesk_MT;
           mt_handler.powner=this;
		   thread_splitter.run();
	   }
     // memset(pb,0,pitch*vi.height);

/*
    int w2=vi.width/2;
	   BYTE *p=bits_ptr;
	  for(int y=0;y<vi.height;y++)
	  {
		  DWORD* pdw=(DWORD*)p;
          //		  DWORD* pdwe=(DWORD*)p+w2;
		  //		  for(;pdw<pdwe;++pdw) *pdw=fc;
		  //
		  for(int x=0;x<w2;x++) pdw[x]=fc;

		  p+=pitch;
	  }
*/
   

	  t=cst.Sec();
	  _cprintf("copy %g milisec\n",t*1000.);

}


inline void init_clips()
{
  
   ; 
   vcrd.reserve(64);
   pcrd.reserve(64);

   ClipRegionData cr=ClipRegionData();
   cr.pclip=this;
   cr.w=vi.width;
   cr.h=vi.height;
   cr.pitch=pitch;
   cr.crop_rect=make_rect(0,0,vi.width,vi.height);
   cr.rect_absolute=cr.crop_rect;
   cr.pos=POINT();
   cr.pframe=0;
   vcrd.push_back(cr);
  
  for(int n=1;;n++)
  {
      ClipRegionData cr=ClipRegionData();
	  AVSValue ac=get_value(v_buf<char>().printf("clip[%d]",n));
      if(!ac.IsClip()) break;
	  cr.clip=ac.AsClip();
	  cr.pclip=(IClip*)(void*)cr.clip;
	  VideoInfo vi=cr.pclip->GetVideoInfo();
	  cr.pframe=0;
	  cr.w=vi.width;
	  cr.h=vi.height;
	  vcrd.push_back(cr);
  }
  pcrd.resize(vcrd.size());
}


inline 	 void init_once(int n, IScriptEnvironment* env)
	 {
			 ++handled_count;
			 if(n) return ;

			  pitch = frame->GetPitch();

			 int m=get_value("thread.mode",AVSValue(0)).AsInt(0);
			 thread_mode=m;
			 if(m)
			 {
				 int nthread=get_value("thread.num",AVSValue(0)).AsInt(0);
				 if(nthread<=0) nthread=thread_splitter.get_num_processors();
				 thread_splitter.init(&mt_handler,nthread);//,THREAD_PRIORITY_HIGHEST);
			 }

			 bool ffw=env->MakeWritable(&frame);

             init_clips(); 
			 fc=fill_color;
			 filldesk();

	 };


inline AVSValue get_clip_value(int num, const char* name,float dflt,char* buf)
{
     sprintf(buf,"clip[%d].%s",num,name);
	 return get_value(buf,AVSValue(dflt));
}

inline bool into_rect(RECT& r,RECT& ro)
{
   return  (ro.left<=r.left)&&(ro.top<=r.top)&&(r.right<=ro.right)&&(r.bottom<=ro.bottom);  
}
//
static inline int d2d(int n)
{
	return (n>>1)<<1;
}
void  calc_overlay()
{
    char tmp[128];

	int undef=-1024*1024;

   int cb=vcrd.size();
   int r;

   ClipRegionData& crd0=vcrd[0];
   crd0.fmasked=false;   

   for(int n=1;n<cb;n++)
   {
	   
	   ClipRegionData& crd=vcrd[n];
       crd.fmasked=false;   
	    r=get_clip_value(n,"left",0,tmp).AsFloat();
	    crd.pos.x=d2d(r);
        r=get_clip_value(n,"top",0,tmp).AsFloat();;
		crd.pos.y=r;
        
		RECT rc;
        r=get_clip_value(n,"rect.left",0,tmp).AsFloat();
		rc.left=d2d(r);
		r=get_clip_value(n,"rect.top",0,tmp).AsFloat();;
		rc.top=r;
		r=get_clip_value(n,"rect.right",crd.w,tmp).AsFloat();
		rc.right=d2d(r);
		r=get_clip_value(n,"rect.bottom",crd.h,tmp).AsFloat();
		rc.bottom=r;
		crd.crop_rect=rc;
		RECT rr=shift(rc,crd.pos.x,crd.pos.y);
		

		RECT& rb=crd0.rect_absolute;
		RECT ra;
		if(!IntersectRect(&ra,&rb,&rr))
		{
         crd.fmasked=true; 
		 continue;
		}

        crd.rect_absolute=ra;

		for(int nb=n-1;nb>=0;--nb)
		{
           RECT& ran=vcrd[nb].rect_absolute;
		   vcrd[nb].fmasked|=into_rect(ran,ra);        
		}
			   
   }
  
     ClipRegionData** pc=&pcrd[0];
       
	 clip_count=0;
	 for(int n=cb-1;n>=0;--n)
	 { 
         if(!vcrd[n].fmasked)
		 {
             pc[clip_count]=&vcrd[n]; 
			 clip_count++;
		 }
	 }

}

    
 void  make_overlay(int nframe, IScriptEnvironment* env)
   {

	       calc_overlay();
           bits_ptr = (BYTE* )frame->GetWritePtr();
		   std::vector<PVideoFrame> vframes(clip_count);

           ClipRegionData** pc=&pcrd[0];
		   for(int n=0;n<clip_count;++n)
		   {
             if(pc[n]->pclip==(IClip*)this)
			 {
				   pc[n]->pBits=bits_ptr;
				   pc[n]->pitch=pitch;

			 }
			 else
			 {
			   //vframes[n]=pc[n]->pclip->GetFrame(nframe,env);
			   //PVideoFrame& pvi=vframes[n];
				 VideoFrame* pvi=(VideoFrame*)(void*)pc[n]->pclip->GetFrame(nframe,env);
			   pc[n]->pitch=pvi->GetPitch();
			   pc[n]->pBits=(BYTE* )pvi->GetReadPtr();



			 }
				   
					 
				   
		   }
		   //
		             
   ///int syka=0;

// in_interval(left,x,right) ((left<=x)&&(x<right))		   
		   int it=0,nt=1;
		   ClipRegionData* pce=pcrd[0];
		   ClipRegionData* pcb=pcrd[clip_count-1];
		   

		     int count=vi.height/nt;
              
		     BYTE* pdest =  bits_ptr;
		    int pitchnt=(pitch)*nt; 
			int wh=vi.width;
           
         for(int y=it;y<vi.height;y+=nt)
		 {
			 if(1)
			 {
              ClipRegionData* pcy=pce;

			  
			  while(!in_interval(pcy->rect_absolute.top,y,pcy->rect_absolute.bottom)) --pcy;

			  for(ClipRegionData* p=pcy;p>=pcb;p--)
			  {
				  p->calc_py(y);
			  }

              

              DWORD* pdw =(DWORD*)pdest;
			  for(int x=0;x<wh;x+=2)
			  {
                     ClipRegionData* pcx=pcy;
					 while(!in_interval(pcx->rect_absolute.left,x,pcx->rect_absolute.right)) --pcx;
					 if(pcx==pcb) 
					 {
						 fc=7*fc+33;
                         pdw[x>>1]=fc;
					 }
					 else pdw[x>>1]=pcx->macropxl(x>>1);
				 
			  }

			 }
			 else
			 {

			 

			 DWORD* pw =(DWORD*)pdest;
			 for(int x=0;x<wh;x+=2)
			 {
				 
				 //((DWORD*)pw)[x]=0;//pcx->macropxl(x);
				 pw[x>>1]=0;


			 }
			  
			 } 
              //if(!in_interval(left,y,right))

              pdest+=pitchnt;
		 }







   }



	 PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env)
	 { 
		 init_once(n,env);

		 bool ffw=env->MakeWritable(&frame);
		 
         if((n%64)==0) fc=7*fc+33;
		 filldesk();
		 make_overlay(n,env);
		 //
		 
		 
		 return frame;

	 }



	 bool __stdcall GetParity(int n) { return false; }
	 const VideoInfo& __stdcall GetVideoInfo() { 
		 return vi; 
	 }
	 void __stdcall SetCacheHints(int cachehints,int frame_range) { };

	 void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { };


	 static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {
		 return new AviSynthFastOverlay(env,args[0].AsFloat(640),args[1].AsFloat(480),args[2].AsFloat(0),args[3].AsFloat(25),args[4].AsFloat(3*107892));
	 }


 };

