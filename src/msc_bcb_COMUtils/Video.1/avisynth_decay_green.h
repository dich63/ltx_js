#pragma once

//#include "AviSynth.h"
#include "AviSynth_templates.h"
//#pragma comment (lib,"Extras/avisynth.lib")
#include "thread_splitter.h"
#include "orderstat.h"

#define avs_fast_call   inline 
#define avs_fast_call_f   __forceinline 

template <class Version=version>
struct AviSynthDecayGreen_base:public avs_base<Version>
{
	typedef unsigned int uint_t;

	typedef typename AviSynthDecayGreen_base<Version> base_t;

	struct MT_handler_t
	{
      base_t* po;
	   BYTE *pdest;
	   BYTE *psrc;
	   int mode;
	   
	  void set_ptr(const BYTE *pd,const BYTE *ps)
	  {
		  pdest=(BYTE *)pd;
		  psrc=(BYTE *)ps;
	  }
	  void operator()(int it, int nt)
	  {
          if(mode==2) po->blend_if_MT2(pdest,psrc,it,nt);
		  else po->blend_if_MT(pdest,psrc,it,nt);
	  }
	};

      

     enum    
	{
		alpha_denom_dig = 16 ,
		alpha_denom =1<<alpha_denom_dig

	};

         int fdisabled;
	     MT_handler_t mt_handler;
		 thread_splitter_t<MT_handler_t,1>  thread_splitter;



        AVSValue Filter;
    	AVSValue Params;
		RECT rc,rc_old;
		PVideoFrame frame;
		uint_t alpha_num;
		uint_t alpha_num_2;

		double alpha_2;
		//uint_t beta_num;
		
		int row_size;
		int pitch;
		int handled_count;
		int frame_modulo;

        AVSValue frame_getter;


		AviSynthDecayGreen_base():handled_count(-1){};
	 AviSynthDecayGreen_base(PClip _clip,AVSValue _Filter,AVSValue _Params)
		 :avs_base<Version>(_clip),Filter(_Filter),Params(_Params),handled_count(-1)
		 
	 {
		 fdisabled=0;
		 alpha_num_2=alpha_denom;
		 alpha_2=1;
		 frame_getter=new CurrentGetter<AviSynthDecayGreen_base>(this);
		 RECT r={0,0,0,0};
		 rc=rc_old=r;
         mt_handler.po=this;
		 mt_handler.mode=0;
		 frame_modulo=1;

	 }

	 inline PVideoFrame&	  current_frame()
	 {
		 return frame;
	 }

	 inline uint_t set_alpha(double alpha)
	 {
               //beta_num=double(alpha_denom)*(1-alpha);
       return alpha_num=double(alpha_denom)*alpha;           
	 }
	 inline uint_t set_alpha2(double alpha2)
	 {
		 //beta_num=double(alpha_denom)*(1-alpha);
		 return alpha_num_2=double(alpha_denom)*alpha2;           
	 }

	 void copy(int n,IScriptEnvironment* env=0)
	 {
		 int height = vi.height;//frame->GetHeight();
		 BYTE* pdest = (BYTE* )frame->GetWritePtr();
		 PVideoFrame frm = child->GetFrame(n,env);
		 const BYTE* psrc = (BYTE* )frm->GetReadPtr();
		 //env->BitBlt(pdest,pitch,psrc,pitch,row_size, height);
		 //blend00(pdest,psrc);

		 for(int y=0;y<vi.height;y++)
		 {
			 //blend_line00(p_dest,p_src);
			 for(int x=0;x<row_size;x++)
				 pdest[x]=psrc[x];

			 pdest += pitch;
			 psrc += pitch;

		 }




	 }
	inline void init_once(int n,IScriptEnvironment* env)
	{
		++handled_count;


         //if(n) return ;
		 if(InterlockedExchangeAdd(&m_init_flag,0)) return;
		 locker_t<IClipMap_impl<Version> > lock(this);
		 if(InterlockedIncrement(&m_init_flag)>1) return;

		 fdisabled=get_value("disabled",AVSValue(0)).AsInt(0);
       
		 if(!fdisabled)
		 {
    		 int m=get_value("thread.mode",AVSValue(0)).AsInt(0);
			 mt_handler.mode=m;
          if(m)
		  {
	    	int nthread=get_value("thread.num",AVSValue(0)).AsInt(0);
		    if(nthread<=0) nthread=thread_splitter.get_num_processors();
		    thread_splitter.init(&mt_handler,nthread);//,THREAD_PRIORITY_HIGHEST);
		  }
		 
		  frame_modulo=get_value("frame.modulo",AVSValue(1)).AsInt(1);
		  if(frame_modulo<1) frame_modulo=1;


         }
		 

		frame = env->NewVideoFrame(vi);
		env->MakeWritable(&frame);
		

		 pitch = frame->GetPitch();
		row_size = frame->GetRowSize();
		copy(n,env);

	}

	inline int handle_once(IScriptEnvironment* env)
	{
        int n=handled_count+1;
		GetFrame(n,env);
		return handled_count;
	}

    
	

	
avs_fast_call	void blend_line00(BYTE *pline_dest,const BYTE *pline_src)
	{
		BYTE *pend=pline_dest+row_size;
		uint_t a=alpha_num, b=alpha_denom-a;

		for(;pline_dest<pend;++pline_dest,++pline_src)
	   {
          uint_t v;
		  v= b*(*pline_dest)+a*(*pline_src);
		  *pline_dest=v>>alpha_denom_dig;
	   }

	}

avs_fast_call void blend_line_if11(BYTE *pline_dest,const BYTE *pline_src,BYTE *pleft,BYTE *pright,BYTE *pleft_old,BYTE *pright_old)
	{
		BYTE *pend=pline_dest+row_size;
		uint_t a=alpha_num, b=alpha_denom-a;
		uint_t a2=alpha_num_2, b2=alpha_denom-a2;
		uint_t v;

		for(;pline_dest<pend;++pline_dest,++pline_src)
		{
			if(in_interval(pleft,pline_dest,pright))
				continue;
			else 
				if(in_interval(pleft_old,pline_dest,pright_old))
				{
                  //*pline_dest=*pline_src;
				   v= b2*(*pline_dest)+a2*(*pline_src);
				  *pline_dest=v>>alpha_denom_dig;
				  continue;
					
				}
				
				v= b*(*pline_dest)+a*(*pline_src);
				*pline_dest=v>>alpha_denom_dig;
			
		}
	}

	avs_fast_call void blend_line_if01(BYTE *pline_dest,const BYTE *pline_src,BYTE *,BYTE *,BYTE *pleft_old,BYTE *pright_old)
	{
		BYTE *pend=pline_dest+row_size;
		uint_t a=alpha_num, b=alpha_denom-a;
		uint_t a2=alpha_num_2, b2=alpha_denom-a2;
		uint_t v;

		for(;pline_dest<pend;++pline_dest,++pline_src)
		{
			
				if(in_interval(pleft_old,pline_dest,pright_old))
				{
					//*pline_dest=*pline_src;
				     v= b2*(*pline_dest)+a2*(*pline_src);
					*pline_dest=v>>alpha_denom_dig;
					 continue;
				}
				
				v= b*(*pline_dest)+a*(*pline_src);
				*pline_dest=v>>alpha_denom_dig;

		}
	}

	avs_fast_call void blend_line_if10(BYTE *pline_dest,const BYTE *pline_src,BYTE *pleft,BYTE *pright,BYTE *,BYTE *)
	{
		uint_t a=alpha_num, b=alpha_denom-a;
		BYTE *pend=pline_dest+row_size;
		for(;pline_dest<pend;++pline_dest,++pline_src)
		{
			if(in_interval(pleft,pline_dest,pright)) continue;
			uint_t v;
            v= b*(*pline_dest)+a*(*pline_src);
			*pline_dest=v>>alpha_denom_dig;
		}
	}


	inline void blend_if_MT(BYTE *p_dest,const BYTE *p_src,int it,int nt)
	{
		bool fm_old=IsRectEmpty(&rc_old);
		bool fm_c=IsRectEmpty(&rc);

		int pitchnt=pitch*nt;
		int pitchit=pitch*it;
		p_dest+=pitchit;
		p_src+=pitchit;


		if(fm_old&fm_c)
		{
			for(int y=it;y<vi.height;y+=nt)
		 {
			 blend_line00(p_dest,p_src);
			 p_dest += pitchnt;
			 p_src += pitchnt;

		 }
			return;
		}
		//
		int mult=vi.BitsPerPixel()/8;

		for(int y=it;y<vi.height;y+=nt)
		{
			short flags=((!fm_c)&&(in_interval(rc.top,y,rc.bottom)))?2:0;
			if((!fm_old)&&(in_interval(rc_old.top,y,rc_old.bottom)))
				flags|=1;

			switch(flags)
			{
			case 0+0:
				blend_line00(p_dest,p_src);
				break;
			case 0+1:
				{
					BYTE* plo=p_dest+rc_old.left*mult;
					BYTE* pro=p_dest+rc_old.right*mult;
					blend_line_if01(p_dest,p_src,0,0,plo,pro);
				}
				break;
			case 2+0:
				{
					BYTE* pl=p_dest+rc.left*mult;
					BYTE* pr=p_dest+rc.right*mult;
					blend_line_if10(p_dest,p_src,pl,pr,0,0);
				}
				break;

			case 2+1:
				{
					BYTE* pl=p_dest+rc.left*mult;
					BYTE* pr=p_dest+rc.right*mult;
					BYTE* plo=p_dest+rc_old.left*mult;
					BYTE* pro=p_dest+rc_old.right*mult;
					blend_line_if11(p_dest,p_src,pl,pr,plo,pro);
				}
				break;

			};


			p_dest += pitchnt;
			p_src += pitchnt;

		}

	}

	inline void blend_if_MT2(BYTE *p_dest,const BYTE *p_src,int it,int nt)
	{
		bool fm_old=IsRectEmpty(&rc_old);
		bool fm_c=IsRectEmpty(&rc);

		int pitchnt=pitch;

        int count=vi.height/nt;

		int pitchit=pitch*(it*count);
		p_dest+=pitchit;
		p_src+=pitchit;

		
		if(it==nt-1)
			count=vi.height-(count*it);


		if(fm_old&fm_c)
		{
			for(int y=0;y<count;++y)
		 {
			 blend_line00(p_dest,p_src);
			 p_dest += pitchnt;
			 p_src += pitchnt;

		 }
			return;
		}
		//
		int mult=vi.BitsPerPixel()/8;

		for(int y=0;y<count;++y)
		{
			short flags=((!fm_c)&&(in_interval(rc.top,y,rc.bottom)))?2:0;
			if((!fm_old)&&(in_interval(rc_old.top,y,rc_old.bottom)))
				flags|=1;

			switch(flags)
			{
			case 0+0:
				blend_line00(p_dest,p_src);
				break;
			case 0+1:
				{
					BYTE* plo=p_dest+rc_old.left*mult;
					BYTE* pro=p_dest+rc_old.right*mult;
					blend_line_if01(p_dest,p_src,0,0,plo,pro);
				}
				break;
			case 2+0:
				{
					BYTE* pl=p_dest+rc.left*mult;
					BYTE* pr=p_dest+rc.right*mult;
					blend_line_if10(p_dest,p_src,pl,pr,0,0);
				}
				break;

			case 2+1:
				{
					BYTE* pl=p_dest+rc.left*mult;
					BYTE* pr=p_dest+rc.right*mult;
					BYTE* plo=p_dest+rc_old.left*mult;
					BYTE* pro=p_dest+rc_old.right*mult;
					blend_line_if11(p_dest,p_src,pl,pr,plo,pro);
				}
				break;

			};


			p_dest += pitchnt;
			p_src += pitchnt;

		}

	}


     inline void blend_if(BYTE *p_dest,const BYTE *p_src)
	 {
		 bool fm_old=IsRectEmpty(&rc_old);
		 bool fm_c=IsRectEmpty(&rc);
		 if(fm_old&fm_c)
		 {
             blend00(p_dest,p_src);
			 return;
		 }
          //
		 int mult=vi.BitsPerPixel()/8;

		 for(int y=0;y<vi.height;y++)
		 {
			 short flags=((!fm_c)&&(in_interval(rc.top,y,rc.bottom)))?2:0;
			 if((!fm_old)&&(in_interval(rc_old.top,y,rc_old.bottom)))
				 flags|=1;

			 switch(flags)
			 {
			   case 0+0:
				   blend_line00(p_dest,p_src);
				   break;
			   case 0+1:
				   {
	   				   BYTE* plo=p_dest+rc_old.left*mult;
					   BYTE* pro=p_dest+rc_old.right*mult;
					   blend_line_if01(p_dest,p_src,0,0,plo,pro);
				   }
				   break;
			   case 2+0:
				   {
					   BYTE* pl=p_dest+rc.left*mult;
					   BYTE* pr=p_dest+rc.right*mult;
					   blend_line_if10(p_dest,p_src,pl,pr,0,0);
				   }
				   break;

			   case 2+1:
				   {
					   BYTE* pl=p_dest+rc.left*mult;
					   BYTE* pr=p_dest+rc.right*mult;
					   BYTE* plo=p_dest+rc_old.left*mult;
					   BYTE* pro=p_dest+rc_old.right*mult;
					   blend_line_if11(p_dest,p_src,pl,pr,plo,pro);
				   }
				   break;

			 };
			 			 

			 p_dest += pitch;
			 p_src += pitch;

		 }

	 }

	 inline void blend00(BYTE *p_dest,const BYTE *p_src)
	 {
         for(int y=0;y<vi.height;y++)
		 {
            blend_line00(p_dest,p_src);
			p_dest += pitch;
			p_src += pitch;
						
		 }

	 }

	 inline void sub(BYTE *p_dest,const BYTE *p_src)
	 {
		 for(int y=0;y<vi.height;y++)
		 {
			 for(int x=0;x<row_size;x++)
			 {
				 int v;
				 v= int(p_dest[x])-int(p_src[x]);
	 			 p_dest[x]=(v<0)?-v/2:v/2;
			 }

			 p_dest += pitch;
			 p_src += pitch;

		 }

	 }

inline	 RECT get_fix_rect(int wi,int hi,RECT rin)
		{
			int w=vi.width;
			int h=vi.height;
			 if(wi<=0) wi=w;
			 if(hi<=0) hi=w;
			 RECT r={0,0,wi,hi};
			 int x=(rin.left+rin.right)/2,d;
			 int y=(rin.top+rin.bottom)/2;
			 r=center(r);
			 r=shift(r,x,y);
			 
			 if(r.left<0)
			 {
				 d=-r.left;
				 r=shift(r,d,0);
			 }

			 if(r.top<0)
			 {
				 d=-r.top;
				 r=shift(r,0,d);
			 }
             d=w-r.right; 
			 if(d<0)
			     r=shift(r,d,0);
    		 d=h-r.bottom; 
			 if(d<0)
				 r=shift(r,0,d);
			 
			 return r;
           //inline  RECT shift(RECT r,int x,int y)
		};
	 AVSValue invoke_op(const char* name,int argc,AVSValue args,IScriptEnvironment* env)
	 {
		 if(strcmpi(name,"update")==0)
		 {
                  handle_once(env);
		 }
		 else if(strcmpi(name,"fix.rect")==0)
		 {
              int rw=AVSArray_def(args,0,AVSValue(-1)).AsFloat(-1);
			  int rh=AVSArray_def(args,1,AVSValue(-1)).AsFloat(-1);
			  RECT r,ro;
			  r.left=get_value("rect.left",AVSValue(0)).AsFloat(0);
			  r.top=get_value("rect.top",AVSValue(0)).AsFloat(0);
			  r.right=get_value("rect.right",AVSValue(0)).AsFloat(0);
			  r.bottom=get_value("rect.bottom",AVSValue(0)).AsFloat(0);
			  ro=get_fix_rect(rw,rh,r);
			  set_value("fix.rect.left",AVSValue(ro.left));
			  set_value("fix.rect.top",AVSValue(ro.top));
			  set_value("fix.rect.right",AVSValue(ro.right));
			  set_value("fix.rect.bottom",AVSValue(ro.bottom));
			  set_value("fix.rect.width",AVSValue(ro.right-ro.left));
			  set_value("fix.rect.height",AVSValue(ro.bottom-ro.top));
  
		 }

		 return this;
	 };
 };

 template <class Version=version>
 struct AviSynthTimeCoarsen:AviSynthDecayGreen_base<Version>
 {
	 int FrameModulo;
	 int Shift;

	 AviSynthTimeCoarsen(PClip _clip,AVSValue Fm,AVSValue _Prm):AviSynthDecayGreen_base<Version>(_clip,Fm,_Prm)
	 {
		 
		 FrameModulo=Fm.AsFloat(1);
         Shift=_Prm.AsFloat(0);
		 if(FrameModulo<=1) FrameModulo=1;
	 }

	 PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env)
	 { 
		 init_once(n,env);
		 if(FrameModulo==1) return child->GetFrame(n,env);
		 if(((n-Shift)%FrameModulo)==0)
		 {
			 copy(n,env);
			 log_printf("<<TimeCoarsen SetFrame %d >>\n",n);
		 }
		 else child->GetFrame(n,env);
		 return frame;

	 }
	 static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {

		 PClip child=args[0].AsClip();   
		 return new AviSynthTimeCoarsen(child,args[1],args[2]);
	 }
 };

 template <class Version=version>
 struct AviSynthDecayGreen_simple:AviSynthDecayGreen_base<Version>
 {
	 int keyF;
	 CStopwatch csg;
	 
	 AviSynthDecayGreen_simple(PClip _clip,AVSValue _Filter,AVSValue _Prm):AviSynthDecayGreen_base<Version>(_clip,_Filter,_Prm)
	 {
		 double f=Filter.AsFloat(1);
		 keyF=Params.AsFloat(0);
		 if(keyF<0) keyF=0;
		 set_alpha(f);
	 }

	 PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env)
	 { 

		 init_once(n,env);


		 if(n<handled_count) return frame;

		 bool fmw=  env->MakeWritable(&frame);

        int cc=0;
		cc++;
		 

		 if((keyF)&&(n))
		 {  
           if((n%keyF)==0)
		   { 
			   CStopwatch cs;
			   copy(n,env);
             log_printf("[%d] DG_ millsec=%g\n",n,1000.*cs.Sec());  
		       return frame;
		   }
		 }

		 

		 BYTE* pdest = (BYTE*)frame->GetWritePtr();
		 PVideoFrame frm = child->GetFrame(n,env);
		 const BYTE* psrc = (BYTE* )frm->GetReadPtr();
		 
		 CStopwatch cs; 
		 
		 //blend00(pdest,psrc);
		 if(fdisabled==0)
		 {


			 if(mt_handler.mode)
			 {
				 mt_handler.set_ptr(pdest,psrc); 
				 thread_splitter.run();
			 }
			 else 	blend00(pdest,psrc);


		 }
         
		 
            if(cc)  log_printf("[%d] DG_ millsec=%g  glob=%g \n",n,1000.*cs.Sec()/double(cc),1000.*csg.Sec()); 
			 csg.Start();

		 
		 return frame;
		 
	 }

	 static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {

           PClip child=args[0].AsClip();   
		 return new AviSynthDecayGreen_simple(child,args[1],args[2]);
	 }

 };

 template <class Version=version>
 struct AviSynthDecayGreen_callback:AviSynthDecayGreen_base<Version>
 {
	 charbuf func_name;
	 ;
	 int keyF;
	 AVSValue handler;
	 

	 AviSynthDecayGreen_callback(PClip _clip,AVSValue _Filter,AVSValue _Prm,AVSValue handler_key):AviSynthDecayGreen_base<Version>(_clip,_Filter,_Prm)
	 {

		 

		 /*
		const char* pkey=handler_key.AsString("");
		 if(*pkey)
		 {
			 handler=IClipMap_impl<Version>::global_list()->get_value(pkey);
			 set_value("handler",handler);
		 }
		 */
		 //int bpp=vi.BitsPerPixel();
		 keyF=0;
		 double f=_Prm.AsFloat(1);
		 set_alpha(f);
		 const char* pn=_Filter.AsString("");
		 strcpy(func_name,pn);

		 set_value("rect.left",0.0);
		 set_value("rect.top",0.0);
		 set_value("rect.right",0.0);
		 set_value("rect.bottom",0.0);




	 }
~AviSynthDecayGreen_callback()
{
	 //(*IClipMap_impl::_global_list())=AVSValue();
}

inline AVSValue invoke(IScriptEnvironment* env,const char* name,int argc,AVSValue a0=0,AVSValue a1=0,AVSValue a2=0,AVSValue a3=0,AVSValue a4=0,AVSValue a5=0,AVSValue a6=0,AVSValue a7=0) 
{
     AVSValue args[8] = {a0,a1,a2,a3,a4,a5,a6,a7};
     AVSValue  res=env->Invoke(name, AVSValue(args,argc));
	 return res;
}

inline AVSValue make_callback(int n,IScriptEnvironment* env)
{
	//return 10;
  
	//AVSValue arect=invoke(env,"ArrayCreate",4,rc_old.left,rc_old.top,rc_old.right,rc_old.bottom);
//Params
	//AVSValue res=invoke(env,func_name,5,child,n,frame_getter,Params,arect);
	//AVSValue res=invoke(env,func_name,5,child,n,frame_getter,Params,0);
          //handler=get_value("pdfh",0);
         //if(handler.IsClip())
		 {
          //   handler.AsClip()->GetFrame(n,env);
		 }


         //set_value("alpha",Params);
	//AVSValue res=invoke(env,func_name,8,child,n,frame_getter,handler,rc_old.left,rc_old.top,rc_old.right,rc_old.bottom);
    	 AVSValue res=invoke(env,func_name,7,n,child,frame_getter,rc_old.left,rc_old.top,rc_old.right,rc_old.bottom);
         
    	
		rc.left=get_value("rect.left",0).AsFloat(0);
		rc.top=get_value("rect.top",0).AsFloat(0);
		rc.right=get_value("rect.right",0).AsFloat(0);
		rc.bottom=get_value("rect.bottom",0).AsFloat(0);

        keyF=get_value("keyF",0).AsInt(0);

		double alpha;
		alpha=get_value("alpha",Params).AsFloat(1);
		set_alpha(alpha);
		alpha=get_value("alpha2",alpha_2).AsFloat(1);
        set_alpha2(alpha);                               
		
        


		
   return res;
}


     PVideoFrame ActualGetFrame(int n, IScriptEnvironment* env)
	 {
              bool fmw=  env->MakeWritable(&frame);
			  BYTE* pdest = (BYTE*)frame->GetWritePtr();
			  PVideoFrame frm = child->GetFrame(n,env);
			  const BYTE* psrc = (BYTE* )frm->GetReadPtr();
			  
			  CStopwatch cs;
			  make_callback(n, env);
			  double t=cs.Sec();
			  cs.Start();

			  if(fdisabled==0)
			  {
				  if(keyF)	  copy(n,env);
				  else 
				  { 

					  if(mt_handler.mode)
					  {
						  mt_handler.set_ptr(pdest,psrc); 
						  thread_splitter.run();
					  }
					  else 	blend_if(pdest,psrc);
				  }

			  }

			rc_old=rc;

     	  log_printf("[%d] DG_callback millsec=%g mkcallback=%g \n",n,1000.*cs.Sec(),1000.*t);  


             return frame;

	 }



	 PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env)
	 { 
          //
		 log_printf("DecayCB::GetFrame(%d)\n",n);
		 init_once(n,env);
		 
		 //int ni=n-handled_count;
		 //if(ni<0) return frame;
		 //
		 if(n<handled_count) return frame;

		 bool fmw=  env->MakeWritable(&frame);
		 
		 BYTE* pdest = (BYTE*)frame->GetWritePtr();

        //for(;handled_count<=n;++handled_count)
		 //CStopwatch cs;
		 int cc=0;
		 int ni=n;

		 for(int ni=handled_count;ni<=n;++ni)
		{
          ++cc;

		  if((ni%frame_modulo)==0)
		         ActualGetFrame(ni,env);

/*
		 PVideoFrame frm = child->GetFrame(ni,env);
		 //
		 log_printf("DecayCB::innerGetFrame(%d)\n",ni);
		 const BYTE* psrc = (BYTE* )frm->GetReadPtr();
		 CStopwatch cs;
		 
		 make_callback(ni, env);
         double t=cs.Sec();
		 cs.Start();

        if(fdisabled==0)
       {

   

		if(keyF)
			copy(ni,env);
		else 
		{ 
          
          if(thread_splitter.nt>1)
		  {
            mt_handler.set_ptr(pdest,psrc); 
			thread_splitter.run();
		  }
		  else 	blend_if(pdest,psrc);
		}

     }
		//	
			//blend00(pdest,psrc);
		 //

//         log_printf("[%d] DG_callback millsec=%g mkcallback=%g \n",n,1000.*cs.Sec(),1000.*t);  


		 		rc_old=rc;
				*/
		
		}
		 handled_count=n;
		 
         

		 return frame;

	 }

	 static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {

		 return new AviSynthDecayGreen_callback(args[0].AsClip(),args[1],args[2],args[3]);
	 }

 };
