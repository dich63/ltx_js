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
struct AviSynthFastOverlay2:public IClipMap_impl<Version>
{
	typedef unsigned int uint_t;

	typedef typename AviSynthFastOverlay2<Version> base_t;

       

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
		   int num;
		   IClip* pclip;
		   bool fhide;
		   POINT pos;
		   int invoke_mode;

		   RECT  crop_rect;
           RECT  rect_absolute;
		   long cropdw_l,cropdw_r;
		   long  posxdw;

		   int  w;
		   int  h;
		   int  pitch;
		   PClip clip;
		   VideoFrame* pframe;
		   bool fmasked;
		   BYTE *pBits;
           BYTE *py;
		   PClip clip_rect;
		   
		   
		   

		  inline BYTE * calc_py(int y)
		  {
			  //y-=pos.y;
			  //y+=crop_rect.top;
			  y-=rect_absolute.top;
              BYTE *p=pBits+pitch*y;
			  return py=p;
		  }
          inline DWORD macropxl(int x)
		  {   
			  //int xx=(x-pos.x);
			 return *((DWORD*)(py+x));
		  }
		   
	   

inline 	   void copy_line_mt(BYTE* pdest_line,int y)
	   {



		   DWORD* pdwd=((DWORD*)pdest_line)+posxdw;
		   DWORD* pdws=(DWORD*)(pBits+pitch*(y-rect_absolute.top));


		   for(int x=cropdw_l;x<cropdw_r;++x)
		   {
			   pdwd[x]=pdws[x];
		   }


	   }


inline 	   void copy_line(BYTE* pdest_line)
	   {
             //pdest_line+=pos.x;  
		   /*
		     WORD* pwd=((WORD*)pdest_line)+pos.x;
			 WORD* pws=(WORD*)py;

             for(int x=crop_rect.left;x<crop_rect.right;++x)
			 {
               pwd[x]=pws[x];
			 }
		 */

		   DWORD* pdwd=((DWORD*)pdest_line)+posxdw;
		   DWORD* pdws=(DWORD*)py;

		   for(int x=cropdw_l;x<cropdw_r;++x)
		   {
			   pdwd[x]=pdws[x];
		   }


	   }

};
	   //	   typedef mutex_cs_t frame_mutex_t;
	   //
	   typedef mutex_fake_t frame_mutex_t;
	   typedef semaphore_local_t frame_semaphore_t;
	   frame_mutex_t frame_mutex_cs;

	   semaphore_local_t sem_reset_clips;
	   semaphore_local_t sem_fill_desk;


	   std::vector<ClipRegionData> vcrd;
	   std::vector<ClipRegionData*> pcrd;
	   int clip_count;

        volatile long m_reset_clips;

		





		//uint_t beta_num;
		
        VideoInfo vi; 
        AVSValue frame_getter;
        int handled_count;
		DWORD fill_color,fc;
		PVideoFrame frame;//,framebk;
		_non_copyable::CStopwatch cst;
		BYTE * bits_ptr;
		int pitch,pitchbk;
		PClip ClipBk;
		BYTE* bits_ptr_bk;


		MT_handler_t mt_handler;
		thread_splitter_t<MT_handler_t,1>  thread_splitter;
		int thread_mode;
		v_buf<char> func_name;
		char* pfunc_name;


		AviSynthFastOverlay2():handled_count(-1){};

	 AviSynthFastOverlay2(IScriptEnvironment* env,AVSValue vwidth,AVSValue vheight,AVSValue fillclr,AVSValue vfps,AVSValue vnum_frames,AVSValue fcallback,AVSValue template_clip)
		 :IClipMap_impl<Version>()
		 
	 {
             handled_count=-1;
			 m_reset_clips=0x7ffffff;
 		 frame_getter=new CurrentGetter<AviSynthFastOverlay2>(this);
              fill_color=0x7fFF7fFF;

		 if(fcallback.IsString())
		 {
              func_name.cat(fcallback.AsString(0));    
		 }
		 pfunc_name=func_name;

		 if(template_clip.IsClip())
		 {
			 
			 PClip tcl=template_clip.AsClip();
			 vi=tcl->GetVideoInfo();
			 if(vi.pixel_type != VideoInfo::CS_YUY2)
				 env->ThrowError("FastOverlay: tplt clip must be YUY2");
			 set_value("clip[0]",template_clip);

		 }
		 else

		 {

		  memset(&vi,0,sizeof(vi));
		 int w=vwidth.AsFloat(256);
		 int h=vheight.AsFloat(w);

		 
		 if(fillclr.IsString()) 
		   fill_color=get_double(fillclr.AsString());

			 //fillclr.AsFloat();
		 //if(fillclr.IsInt()) fill_color=fillclr.AsInt(0);
 
         vi.width=w; 
		 vi.height=h;
		 //vi.fps_numerator = vfps.AsFloat(25)*1001;
		 //vi.fps_denominator = 1001;
		 reset_fps(vi,vfps.AsFloat(25));
		 vi.num_frames = vnum_frames.AsFloat(3*107892);   // 1 hour
		 //vi.pixel_type = VideoInfo::CS_BGR32;
		 vi.pixel_type = VideoInfo::CS_YUY2;
		 }

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
	if(it==nt-1)
		count=vi.height-(count*it);

	int w2=vi.width/2;


	if(!!ClipBk)
	{
        const VideoInfo& vic=ClipBk->GetVideoInfo();
		int wc=vic.width;
		int hc=vic.height;
		int wc2=wc/2;

		
		for(int y=0;y<count;y++)
		{
          int yc=y%hc; 
         BYTE * pc=bits_ptr_bk+(yc*pitchbk);  

		 DWORD* pdw=(DWORD*)p;
		 DWORD* pdwc=(DWORD*)pc;
		for(int x=0;x<w2;x++)
		{
			int xc=x%wc2;
			pdw[x]=pdwc[xc];
		}
		p+=pitch;
		
		}

       return ; 
	}
   

	

   
   
   for(int y=0;y<count;y++)
   {
	   DWORD* pdw=(DWORD*)p;
	   for(int x=0;x<w2;x++) pdw[x]=fc;
	   p+=pitch;
   }
   

}



inline void filldesk(int n,IScriptEnvironment* env)
{

	  bits_ptr = (BYTE* )frame->GetWritePtr();

	  PVideoFrame framebk;
      
	  AVSValue av=get_value("clip[0]");
	  if(av.IsClip())
	  {
		  ClipBk=av.AsClip();
		  framebk=ClipBk->GetFrame(n,env);
		  pitchbk=framebk->GetPitch();
		  bits_ptr_bk=(BYTE*)framebk->GetReadPtr();
	  }
	  else ClipBk=NULL;

	  
	  double t;
       cst.Start();

       filldesk_MT(0,1);

	   ClipBk=NULL;

/*
	   if(thread_mode==0)   
	   else
	   {
		   mt_handler.method=&AviSynthFastOverlay2::filldesk_MT;
           mt_handler.powner=this;
		   thread_splitter.run();
	   }
*/
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
  locker_t<IClipMap_impl<Version>> lock(this);
   ; 
   vcrd.reserve(64);
   pcrd.reserve(64);
   vcrd.clear();


   ClipRegionData cr={};
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
	  ClipRegionData cr={n};
	  AVSValue ac=get_value(v_buf<char>().printf("clip[%d]",n));
	  AVSValue acr=get_value(v_buf<char>().printf("clip[%d].rect",n));
	  bool fi=0;


      //
	  if(!ac.Defined()) break;

	  if(acr.IsClip())
	  {
		  cr.clip_rect=acr.AsClip();
	  }
	  
	  //	  if(!ac.IsClip()) continue;

      //	  if(!ac.IsClip()) break;

	if(ac.IsClip())
	{
	  cr.clip=ac.AsClip();
	  cr.pclip=(IClip*)(void*)cr.clip;
	  VideoInfo vi=cr.pclip->GetVideoInfo();
	  cr.pframe=0;
	  cr.w=vi.width;
	  cr.h=vi.height;
	}
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

             //init_clips(); 
			 fc=fill_color;
			 //filldesk(n,env);


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
	return (n>>2)<<2;
}
void  calc_overlay(IScriptEnvironment* env)
{
    char tmp[128];

	int undef=-1024*1024;

   locker_t<IClipMap_impl<Version>> lock(this);

   int cb=vcrd.size();
   int r;

   ClipRegionData& crd0=vcrd[0];
   crd0.fmasked=true;   

   

   for(int n=1;n<cb;n++)
   {
	   
	   ClipRegionData& crd=vcrd[n];
       
        
	   ///*
		if(crd.fmasked=!crd.clip)
			continue;
		else
		{
	     r=get_clip_value(n,"disable",0,tmp).AsFloat();
		 crd.fmasked=r;
		 avs_base<Version>::log_printf("[%d,%d]",n,crd.fmasked);
		 //if(n==cb-1) avs_base<Version>::log_printf("\n");
		}

		//
		if(crd.fmasked) continue;
		//*/


	    r=get_clip_value(n,"left",0,tmp).AsFloat();
	    crd.pos.x=d2d(r);
        r=get_clip_value(n,"top",0,tmp).AsFloat();;
		crd.pos.y=r;
        
		RECT rc;

         IClipMap* pclipm;
		if((void*)crd.clip_rect)
		{

           IClipMap* pclipm=IClipMap::get_intf(crd.clip_rect,env);
           r= pclipm->get_value("rect.left",AVSValue(double(0))).AsFloat();
		   rc.left=d2d(r);
		   r=pclipm->get_value("rect.top",AVSValue(double(0))).AsFloat();;
		   rc.top=r;
		   r=pclipm->get_value("rect.right",AVSValue(double(crd.w))).AsFloat();
		   if(r<0) r+=crd.w;
		   rc.right=d2d(r);
		   r=pclipm->get_value("rect.bottom",AVSValue(double(crd.h))).AsFloat();
		   if(r<0) r+=crd.h;
		   rc.bottom=r;

		}  
        else 
		{

		

		r=get_clip_value(n,"rect.left",0,tmp).AsFloat();
		rc.left=d2d(r);
		r=get_clip_value(n,"rect.top",0,tmp).AsFloat();;
		rc.top=r;
		r=get_clip_value(n,"rect.right",crd.w,tmp).AsFloat();
		if(r<0) r+=crd.w;
		rc.right=d2d(r);
		r=get_clip_value(n,"rect.bottom",crd.h,tmp).AsFloat();
		if(r<0) r+=crd.h;
		rc.bottom=r;
		}
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
		crd.crop_rect=shift(ra,-crd.pos.x,-crd.pos.y);

          crd.invoke_mode=get_clip_value(n,"invoke.mode",0,tmp).AsFloat();

		if(1)
		for(int nb=n-1;nb>=0;--nb)
		{  
			ClipRegionData& c=vcrd[nb];
           RECT& ran=c.rect_absolute;
           c.fhide=into_rect(ran,ra); 
		   if(!c.invoke_mode)
		        c.fmasked|=c.fhide;        
		}
			   
   }
  
     ClipRegionData** pc=&pcrd[0];
       
	 clip_count=0;
	 for(int n=0;n<cb;++n)
	 { 
		 ClipRegionData*p=&vcrd[n]; 
		 bool ff=!(p->fmasked);
         if(ff)
		 {
             
			 p->cropdw_l=p->crop_rect.left/2;
			 p->cropdw_r=p->crop_rect.right/2;
			 p->posxdw=p->pos.x/2;
			 pc[clip_count]=p;

			 clip_count++;
		 }
	 }

	 avs_base<Version>::log_printf(": clip_count=%d\n",clip_count);
}



 void make_overlay_MT(int it,int nt)
 {
  
	 if(!clip_count) return;

	 ClipRegionData* pcc=pcrd[0];  
	 int count=vi.height;///nt;

	 BYTE* pdest =  bits_ptr+(pitch)*it;
	 int pitchnt=(pitch)*nt; 

	 ClipRegionData** ppcc=&pcrd[0];  

	 for(int y=it;y<count;y+=nt)
	  {


		 for(int l=0;l<clip_count;++l)
			{
				//ClipRegionData& pl=pcc[l];
				//ClipRegionData& pl=*(pcrd[l]);
				ClipRegionData& pl=*(ppcc[l]);
				if(pl.fhide) continue;

				if(in_interval(pl.rect_absolute.top,y,pl.rect_absolute.bottom))  
				{
					//pl.calc_py(y);
					pl.copy_line_mt(pdest,y);             
				}
			}


		 pdest+=pitchnt;
	 }

 }
    
 void  make_overlay(int nframe, IScriptEnvironment* env)
   {


	       calc_overlay(env);
           bits_ptr = (BYTE* )frame->GetWritePtr();
		   std::vector<PVideoFrame> vframes(clip_count);

           ClipRegionData** pc=&pcrd[0];

		   for(int n=0;n<clip_count;++n)
		   {
			   PVideoFrame& f=vframes[n];
			   f=pc[n]->pclip->GetFrame(nframe,env);
			   pc[n]->pitch=f->GetPitch();
			   pc[n]->pBits=(BYTE* )f->GetReadPtr();
			   pc[n]->pframe=(VideoFrame*)(void*)f;

		   }

		   if(thread_mode==0)
		   {
			   
			   make_overlay_MT(0,1);
			   
		   }  
	       else
			{
			 mt_handler.method=&AviSynthFastOverlay2::make_overlay_MT;
			 mt_handler.powner=this;
			 thread_splitter.run();
			}
		
			    


   }



	 PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env)
	 { 
         //locker_t<frame_mutex_t> lock(frame_mutex_cs);

		 init_once(n,env);

		//long reset_clips=InterlockedExchangeAdd(&m_reset_clips,0);

//		if(reset_clips&(1<<1))	 init_clips(); 
		 if(sem_reset_clips.try_lock())
		 {
			 //locker_t <frame_semaphore_t> lock(sem_reset_clips,false);
			 init_clips(); 
		 }

		 bool ffw=env->MakeWritable(&frame);



		 //make_callback(n,env);

		 if(sem_fill_desk.try_lock())
		 {
			 //locker_t <frame_semaphore_t> lock(sem_fill_desk,false);

			 filldesk(n,env);
		 }
  
		 make_callback(n,env);
		 
			 ;
         
		 //if((n%8)==0) fc=7*fc+33;
		 //filldesk();

		 //locker_t<frame_mutex_t> lock(frame_mutex_cs);
		 make_overlay(n,env);
		 //
		 
		 InterlockedExchange(&m_reset_clips,0);
		 return frame;

	 }


	 inline AVSValue invoke(IScriptEnvironment* env,const char* name,int argc,AVSValue a0=0,AVSValue a1=0,AVSValue a2=0,AVSValue a3=0,AVSValue a4=0) 
	 {
		 AVSValue args[8] = {a0,a1,a2,a3,a4};
		 AVSValue  res=env->Invoke(name, AVSValue(args,argc));
		 return res;
	 }

	 inline AVSValue make_callback(int n,IScriptEnvironment* env)
	 {
		 char *p=func_name.get();
		 if(p)  return invoke(env,p,3,n,frame_getter,int(vcrd.size()));

	 }

	 AVSValue invoke_op(const char* name,int argc,AVSValue args,IScriptEnvironment* env)
	 {
		 if(strcmpi(name,"update.bk")==0)
		 {
			 int n=(argc)?args[0].AsFloat(0):0;

			 sem_fill_desk();

			 //locker_t<frame_mutex_t> lock(frame_mutex_cs);
			 //filldesk(n,env);
			 //InterlockedBitTestAndSet(&m_reset_clips,0);
		 }
		 if(strcmpi(name,"clips.reset")==0)
		 {
			 //locker_t<frame_mutex_t> lock(frame_mutex_cs);
			 //init_clips();
			 //InterlockedBitTestAndSet(&m_reset_clips,1);
			 sem_reset_clips();

		 }

		 return AVSValue();
	 }

	 bool __stdcall GetParity(int n) { return false; }
	 const VideoInfo& __stdcall GetVideoInfo() { 
		 return vi; 
	 }
	 void __stdcall SetCacheHints(int cachehints,int frame_range) { };

	 void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { };


	 static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {
		 return new AviSynthFastOverlay2(env,args[0].AsFloat(640),args[1].AsFloat(480),args[2],args[3].AsFloat(25),args[4].AsFloat(3*107892),args[5],args[6]);
	 }


 };

