#pragma once
#include "AviSynth_templates.h"
#include "orderstat.h"
#include <vector>

template <class Version=version>
struct AviSynth_PD_base:public avs_base<Version>
{
	typedef typename AviSynth_PD_base<Version> self_t;

	AVSValue frame_getter;
	AVSValue Params;
    VideoInfo vi_val;
	PVideoFrame frame;
	int pitch;
	int row_size;
	int totsize;
	int handled_count;
	std::vector<DWORD*> vlines;

	inline PVideoFrame&	  current_frame()
	{
		return frame;
	}

	AviSynth_PD_base(PClip _clip,AVSValue _Params=AVSValue())
		:avs_base<Version>(_clip),Params(_Params),
        frame_getter(new CurrentGetter<AviSynth_PD_base>(this)),handled_count(-1)
	{
		VideoInfo tmp={vi.width,vi.height,vi.fps_numerator,vi.fps_denominator,vi.num_frames,VideoInfo::CS_BGR32};
        vi_val=tmp;
		vi_val=vi;
	}


	inline void init_once(int n,IScriptEnvironment* env)
	{
		++handled_count;
		
		//if(n) return ;
		if(InterlockedExchangeAdd(&m_init_flag,0)) return;
		locker_t<IClipMap_impl<Version> > lock(this);
		if(InterlockedIncrement(&m_init_flag)>1) return;

		frame = env->NewVideoFrame(vi_val);
		bool ffw=env->MakeWritable(&frame);
        DWORD* p = (DWORD* )frame->GetWritePtr();

		pitch = frame->GetPitch();
		row_size = frame->GetRowSize();
		int wf=frame->GetHeight();
	
		//env->MakeWritable(&frame);
		int nBPP=vi_val.BitsPerPixel();

		int w=vi_val.width;
		int h=vi_val.height;
         vlines.resize(h);
		int  pitch32=pitch/4;
		 
		 totsize=h*pitch;      
		//memset(p,0,totsize);
		BYTE *pb=(BYTE *)p;
		for(int y=0;y<h;y++,pb+=pitch) 
		{
		 vlines[y]=(DWORD*)pb;
         memset(pb,0,row_size);
		}

	}




	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env)
	{ 

		return frame;
	}
	bool __stdcall GetParity(int n) { return false; }
	const VideoInfo& __stdcall GetVideoInfo() { return vi_val;}
	void __stdcall SetCacheHints(int cachehints,int frame_range) { };
	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { };

};


template <class Version=version>
struct AviSynth_PD_distance:AviSynth_PD_base<Version>
{
    AVSValue clip2;
	AviSynth_PD_distance(PClip _clip,AVSValue _clip2,AVSValue _Params)
		:AviSynth_PD_base<Version>(_clip,_Params),clip2(_clip2){}

	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env)
	{ 
        init_once(n,env);
		if(n<handled_count) 
			return frame;
		DWORD* pdw = (DWORD* )frame->GetWritePtr();
		DWORD* pdw0=pdw;

		PVideoFrame fr1=child->GetFrame(n,env);
		PVideoFrame fr2=clip2.AsClip()->GetFrame(n,env);

		int pitch_0= fr2->GetPitch();
        BYTE * p1=(BYTE *)fr1->GetReadPtr();
		BYTE * p2=(BYTE *)fr2->GetReadPtr();
		int pitch_1= fr1->GetPitch();
		int pitch_2= fr2->GetPitch();
		
		int mult=vi.BitsPerPixel()/8;
		int w=vi_val.width;



		//int rsize=fr1->GetRowSize();
		if(mult==2)
		{

			for(int y=0;y<vi_val.height;y++)
			{
				 DWORD *j=vlines[y];   
				for(int x=0;x<w;x++)  
				{
					//j[x]=128;
					//continue;
					BYTE* i1=p1+2*x;
					BYTE* i2=p2+2*x;
					int v;
					v=int(i1[0])-int(i2[0]);
					j[x] =(v>=0)?v:-v; 
					v=int(i1[1])-int(i2[1]);
					j[x] +=(v>=0)?v:-v; 
					
				}

				p1=ptr_offset(p1,pitch_1);
				p2=ptr_offset(p2,pitch_2);

			}

		}
		else if(mult==3)
		{
			for(int y=0;y<vi_val.height;y++)
			{
				DWORD *j=vlines[y];   
				for(int x=0;x<w;x++)  
				{
                    
					BYTE* i1=p1+3*x;
					BYTE* i2=p2+3*x;
					int v;
					v=int(i1[0])-int(i2[0]);
					j[x] =(v>=0)?v:-v; 
					v=int(i1[1])-int(i2[1]);
					j[x] +=(v>=0)?v:-v; 
					v=int(i1[2])-int(i2[2]);
					j[x] +=(v>=0)?v:-v; 


				}

				p1=ptr_offset(p1,pitch_1);
				p2=ptr_offset(p2,pitch_2);

			}


		} 
  		 else if(mult==4)
		 {
			 int  pitch32=pitch/4;
			 for(int y=0;y<vi_val.height;y++)
			 {
				 DWORD *j=vlines[y];   
				 for(int x=0;x<w;x++)  
				 {
					 //					 j[x]=128;
					 //					 continue;
					 int xb=x<<2;
					 BYTE* i1=p1+xb;
					 BYTE* i2=p2+xb;
					 int v;
					 v=int(i1[0])-int(i2[0]);
					 j[x] =(v>=0)?v:-v; 
					 v=int(i1[1])-int(i2[1]);
					 j[x] +=(v>=0)?v:-v; 
					 v=int(i1[2])-int(i2[2]);
					 j[x] +=(v>=0)?v:-v; 
					 v=int(i1[3])-int(i2[3]);
					 j[x] +=(v>=0)?v:-v; 

				 }
				 
				 p1=ptr_offset(p1,pitch_1);
				 p2=ptr_offset(p2,pitch_2);

				 //p1 += pitch_0;
				 //p2 += pitch_0;
				 //pdw+=pitch32;
			 }

		 }

		DWORD maxv1=0;
		pdw=pdw0;
		for(int y=0;y<vi_val.height;y++)
		{

			for(int x=0;x<w;x++)  
			{
				DWORD d=	pdw[x];
				if(d>maxv1) maxv1=d;
			}


			pdw+=w;
		}

		return frame;
	}

	static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {

		return new AviSynth_PD_distance<Version>(args[0].AsClip(),args[1],args[2]);
	}
};

template <class Version=version>
struct AviSynth_PD:AviSynth_PD_base<Version>
{
	
	DWORD probN;

	AviSynth_PD(PClip _clip,AVSValue _Params,AVSValue key)
		:AviSynth_PD_base<Version>(_clip,_Params)
	{
		/*
		const char* pkey=key.AsString("");
		if(*pkey)
		{
			IClipMap_impl<Version>::global_list()->set_value(pkey,this);//frame_getter);
		}
		*/
	}
~AviSynth_PD()
{
   
}

	
inline DWORD _probN(DWORD* p)
{
	int w=vi_val.width;
	int h=vi_val.height;
	return p[w*h-1]-p[0];
}

       

	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env)
	{ 
		//_cprintf("PD::GetFrame(%d)\n",n);
		init_once(n,env);
		if(n<handled_count) return frame;
		DWORD maxv;
		DWORD* pdw0 = (DWORD* )frame->GetWritePtr();
		DWORD* pdw=pdw0;
        int   pitch32=pitch/4;
		PVideoFrame fr1=child->GetFrame(n,env);
		
		
		DWORD* p1=(DWORD *)fr1->GetReadPtr();
		DWORD* p01=p1;

		int w=vi_val.width;
		int h=vi_val.height;

		CStopwatch cs;

		for(int y=0;y<h;y++)
			{
                DWORD d=0;
				DWORD* j=vlines[y];
				for(int x=0;x<w;x++)  
				{
		          d+=p1[x];
				  j[x]=d;
				}

				p1 =ptr_offset(p1,pitch);
		       	//pdw =ptr_offset(pdw,pitch);
			}

		for(int x=0;x<w;x++)  
		{
			DWORD d=0;
			for(int y=0;y<h;y++)
			{
              DWORD* pv=vlines[y]+x;
			  d+=*pv;
			  *pv=d;
			}
		}

		maxv=vlines[h-1][w-1];

        bool f=(Params.AsFloat(0)!=0 );

       if(f)
	   {
		   pdw=pdw0;
		   for(int y=0;y<h;y++)
		   {
               pdw=vlines[y];
			   for(int x=0;x<w;x++)  
			   {
				   double d=	pdw[x];
				   d=(250.*d)/double(maxv);
				   //d=fabs(d);
				   //d=255;
				   DWORD dn=RGB(d,d,d);
				   pdw[x]=dn;
				   				   
			   }


			   //pdw+=pitch32;
			 //  pdw=ptr_offset(pdw,pitch);
		   }



	   }
       
	   // set values...

          probN=maxv;//_probab(make_rect(0,0,w,h));

           
		 set_value("probN",double(probN));
      log_printf("[%d] PDF millsec=%g\n",n,1000.*cs.Sec());  



		return frame;
	}


    
inline	DWORD _probab(RECT r)
	{
		int w=vi_val.width;
		int h=vi_val.height;
		r=_inrect(r,make_rect(0,0,w,h));
		if(IsRectEmpty(&r)) return 0;
		return vlines[r.bottom][r.right] - vlines[r.top][r.right] - (vlines[r.bottom][r.left]-vlines[r.top][r.left]);
	}
    

inline	POINT median_x(RECT rin,double pl=0.5,double pr=0.5)
	{
		int w=vi_val.width;
		int h=vi_val.height;
		POINT pt={0,w};
		rin=_inrect(rin,make_rect(0,0,w,h));
		if(IsRectEmpty(&rin)) return pt;
		DWORD mfl=_probab(rin)*pl;
		DWORD mfr=_probab(rin)*pr;

		DWORD* pdb=vlines[rin.bottom]; 
		DWORD* pdt=vlines[rin.top];

		for(int n=rin.left;n<=rin.right;n++)
		{

			DWORD d=pdb[n]-pdt[n]; 

			if(d<=mfr) 
			{
				pt.y=n;
				if(d<mfl) pt.x=n;
			}
		}
		return pt;
}

inline	POINT median_y(RECT rin,double pl=0.5,double pr=0.5)
{
	int w=vi_val.width;
	int h=vi_val.height;
	POINT pt={0,h};
	rin=_inrect(rin,make_rect(0,0,w,h));
	if(IsRectEmpty(&rin)) return pt;

	DWORD mfl=_probab(rin)*pl;
	DWORD mfr=_probab(rin)*pr;



	
	DWORD* pdb=vlines[rin.bottom]; 
	DWORD* pdt=vlines[rin.top];

	for(int n=rin.top;n<=rin.bottom;n++)
	{

		DWORD d=vlines[n][rin.right]-vlines[n][rin.left]; 
		if(d<=mfr) 
		{
			pt.x=n;
			if(d<mfl) pt.y=n;
		}
	}

	return pt;
}

double max_interval_x(POINT& pt,RECT r)
{
	int w=vi_val.width;
	int h=vi_val.height;

	POINT pout={r.left,r.right};
     r=_inrect(r,make_rect(0,0,w,h));

	POINT pc0=center(pt);
	DWORD *pf=vlines[r.top];
	DWORD *pl=vlines[r.bottom-1];
	DWORD pbmax=0;
	for(int n=r.left;n<=r.right;++n){
		POINT pc={pc0.x+n,pc0.y+n};
		pc=_ininterval(pc,pout);
		double pb=(pl[pc.y]-pf[pc.y])-(pl[pc.x]-pf[pc.x]);
		if(pb>=pbmax)
		{
			pbmax=pb;
			pt=pc;
		}
	}

	DWORD pbnn=_probab(make_rect(0,0,w,h));
	return (pbnn)?double(pbmax)/double(pbnn):0;
	
}

double max_interval_y(POINT& pt,RECT r)
{
	int w=vi_val.width;
	int h=vi_val.height;

	POINT pout={r.top,r.bottom};
	r=_inrect(r,make_rect(0,0,w,h));

	POINT pc0=center(pt);
	
	DWORD pbmax=0;
	for(int n=r.top;n<=r.bottom;++n){
		POINT pc={pc0.x+n,pc0.y+n};
		pc=_ininterval(pc,pout);
		double pb=(vlines[pc.y][r.right]-vlines[pc.y][r.left])-(vlines[pc.x][r.right]-vlines[pc.x][r.left]);
		if(pb>=pbmax)
		{
			pbmax=pb;
			pt=pc;
		}
	}

	DWORD pbnn=_probab(make_rect(0,0,w,h));
	return (pbnn)?double(pbmax)/double(pbnn):0;
}

double max_rect(RECT& rt,RECT ro)
{
	int w=vi_val.width;
	int h=vi_val.height;

	RECT r={0,0,w,h};
	ro=_inrect(ro,r);

	RECT rc0=center(rt);
	DWORD pbmax=0;
	for(int y=ro.left;y<=ro.right;++y)
        for(int x=ro.top;x<=ro.bottom;++x)
		{
			RECT r=shift(rc0,x,y);
			 r=_inrect(r,ro);
			 DWORD pbb=_probab(r);
			 if(pbmax<=pbb)
			 {
               pbmax==pbb;
			   rt=r;
			 }

		}
	DWORD pbnn=_probab(ro);
	return (pbnn)?double(pbmax)/double(pbnn):0;
};



AVSValue invoke_op(const char* name,int argc,AVSValue args,IScriptEnvironment* env)
{
	int w=vi_val.width;
	int h=vi_val.height;
	char * cmds[]={"median","maxregion"};
	POINT pt;
	RECT r={0,0,w,h};
	RECT rout=r;

	
	if(strcmpi(name,"update")==0)
	{

          int n=AVSArray_def(args,0,0).AsFloat();   
		  GetFrame(n,env);
		return this;
	}

    int flags=AVSArray_def(args,0,3).AsFloat();   
	r.left=AVSArray_def(args,1,rout.left).AsFloat(0);
	r.top=AVSArray_def(args,2,rout.top).AsFloat(0);
	r.right=AVSArray_def(args,3,rout.right).AsFloat(0);
	r.bottom=AVSArray_def(args,4,rout.bottom).AsFloat(0);

	if(strcmpi(name,"probab")==0)
	{
       double pbb;
		   pbb=_probab(r);
		   AVSValue val;
		   if(flags&(probN!=0))
		               val= AVSValue(pbb/double(probN));  
		   else val= AVSValue(pbb);

           set_value("probab",val);

		   return val;

	}


	if(strcmpi(name,"median")==0)
	{
       
	   
	
	   set_value("median.region.left",r.left);
	   set_value("median.region.top",r.top);
	   set_value("median.region.right",r.right);
	   set_value("median.region.bottom",r.bottom);

	   double pl=AVSArray_def(args,5,0.5).AsFloat(0);
	   double pr=AVSArray_def(args,6,1.0-pl).AsFloat(0);

	   set_value("median.pl",pl);
	   set_value("median.pr",pr);

       double cm,cm2,cm1;

	   
	   if(flags&2)
	   {
		   pt=median_y(r,pl,pr);
		   set_value("median.y.left",pt.x);
		   set_value("median.y.right",pt.y);
		   cm=cm2=(pt.x+pt.y)/2;
		   //_cprintf("mdy %d %d\n",pt.x,pt.y);
	   }
	   if(flags&1)
	   {
		   pt=median_x(r,pl,pr);
		   set_value("median.x.left",pt.x);
		   set_value("median.x.right",pt.y);
		   cm=(pt.x+pt.y)/2;
		   //		   _cprintf("mdx %d %d      \r",pt.x,pt.y);
		  // _cprintf("mx %d         \r",cm);
	   }

       return cm;  

	}
	else if(strcmpi(name,"maxregion")==0)
	{
        
        //		double pb=max_region();  
		double pb,cm;

		rout.left=AVSArray_def(args,4+1,rout.left).AsFloat(0);
		rout.top=AVSArray_def(args,4+2,rout.top).AsFloat(0);
		rout.right=AVSArray_def(args,4+3,rout.right).AsFloat(0);
		rout.bottom=AVSArray_def(args,4+4,rout.bottom).AsFloat(0);


		set_value("maxregion.left",rout.left);
		set_value("maxregion.top",rout.top);
		set_value("maxregion.right",rout.right);
		set_value("maxregion.bottom",rout.bottom);
		set_value("maxregion.probab",0);

		
		if(flags==0)
		{
			pb=max_rect(r,rout);

			set_value("maxregion.right",r.right);
			set_value("maxregion.bottom",r.bottom);
			set_value("maxregion.left",r.left);
			set_value("maxregion.top",r.top);
         
		}
		else
		{

		if(flags&2)
		{
			POINT pp={r.top,r.bottom};
			pt=pp;
			pb=max_interval_y(pt,rout);  
			cm=(pt.x+pt.y)/2;
			r.top=pt.x;
			r.bottom=pt.y;
			set_value("maxregion.right",r.right);
			set_value("maxregion.bottom",r.bottom);
		}

		if(flags&1)
		{
			POINT pp={r.left,r.right};
			pt=pp;
			pb=max_interval_x(pt,rout);  
			cm=(pt.x+pt.y)/2;
			r.left=pt.x;
			r.right=pt.y;
			set_value("maxregion.left",r.left);
			set_value("maxregion.top",r.top);
		}
		
		}
        
		 set_value("maxregion.probab",pb);
		return cm;

	}

	     

	return AVSValue();
}



	

	static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {

		return new AviSynth_PD<Version>(args[0].AsClip(),args[1],args[2]);
	}
};
