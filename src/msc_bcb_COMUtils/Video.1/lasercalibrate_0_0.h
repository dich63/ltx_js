#pragma once

#include "lasercalibrate.h"
#include "calibrate_data.h"

inline bool check_signaled(HANDLE h)
{
	return (h)&&(WaitForSingleObject(h,0)==WAIT_OBJECT_0);
}

template <class float_type> 
std::complex<float_type> detect_center(float_type** ppd,int w,int h,RECT cutoff,float_type rca,float_type eps=float_type())
{

	distrib2D<float_type> F2D;
	float_type probmax;
	F2D.make_from(ppd,w,h);
	RECT rt={0,0,rca,rca};
	RECT r=F2D.max_rect(rt ,&probmax,cutoff);
	RECT rm=F2D.median(r,0.5-eps,0.5+eps);

	std::complex<double> ip=center_point<double>(rm);

	return ip;

}

template <class Calibrate>
void calibrate_static(Calibrate& cal,CALIBRATE_DATA& cd)
{
	int npoint =cd.npoint/4;
	cal.show();
	for (int l=0;l<=npoint;l++)
	{

		double pi=3.14159265358979323846;
		double fi=2*pi*double(l-1)/(npoint-1);

		std::complex<double> pc;
		double rcyc=cd.rdisk ;
		if(l)	pc=std::polar(cd.rcycle,fi);
	
		std::pair<std::complex<double>,double >  o= cal.unitcycle(pc,rcyc,0);
	}
}

//Calibrate cal(cd.screen_rect=ScreenCapture(cd.nmon,0).monitor_rect(),RGB(255,255,255));
template <class Calibrate,class WebCamAccumulator>
projective_transform::projmatrix<double> make_cam_calibrate(Calibrate& cal,WebCamAccumulator& wca,  CALIBRATE_DATA* pcd,HANDLE hAbortEvent=0)
{
       

/*
	HANDLE hConout = CreateFileW(L"CONOUT$",GENERIC_WRITE,FILE_SHARE_WRITE,
		NULL,  OPEN_EXISTING,  FILE_ATTRIBUTE_NORMAL,NULL);
	SetStdHandle(STD_OUTPUT_HANDLE,hConout);
	_cprintf("start...\n");
*/

	CALIBRATE_DATA& cd=*pcd;
	
	struct logger_t
	   {
		   FILE* hf;
		   logger_t( CALIBRATE_DATA& cd,wchar_t* fname){
			   hf=0;
			   wcharbuf dumpf;
			   wsprintf(dumpf,L"%s\\logs\\%s",cd.appdir,fname);
			   hf=_wfopen(dumpf,L"wb");

		   }       
		   operator FILE*()
		   {
			   return hf;
		   }
		   ~logger_t(){if(hf) fclose(hf);}
	   } hf_log(cd,L"calibrate.log.txt"),hf_log2(cd,L"dump.log.txt");


	
	projective_transform::projmatrix<double> projm,projmb;
    projective_transform::projmatrix<double> empty;
	//ScreenCapture scc(1,0);
	  


	
	cal.show();
	int npoint=cd.npoint;
	 ;//0.175
    double err_ev=0,err_max=-1;

	std::vector<std::complex<double> > vo(npoint);
	std::vector<std::complex<double> > vi(npoint);

	WebCamAccumulator::detect_handler_t& detector=wca.detect_handler;
	WebCamAccumulator::detect_handler_t::accumulator_t& accumulator=detector.accumulator;
	WebCamAccumulator::view_handler_t& viewer=wca.view_handler; 



	POINT pointC=POINT();  

for(int fmode=0;fmode<2 ;fmode++)
{



	for (int l=0;l<npoint;l++)
	{
        
		double pi=3.14159265358979323846;
		double fi=2*pi*double(l-1)/double(npoint-1);

		std::complex<double> pc;
         double rcyc=cd.rdisk ;
		if(l)	pc=std::polar(cd.rcycle,fi);
		//else rcyc=cd.rdisk*1.5 ;

		accumulator.reset();

		detector.fenabled=false; 

		//WC_accumulator<line_type_c<RED_C> > wca(ncam,dlg);

		std::pair<std::complex<double>,double >  o= cal.unitcycle(pc,rcyc,1);
		
		{
			
			double ix=o.first.real(), iy=o.first.imag(),ir=o.second;
				fprintf(hf_log2,"%d:%d (ix,iy,ir)=(%g;%g%;%g) ",fmode,l,ix,iy,ir);
				fprintf(hf_log2," (x,y,r)=(%g;%g%;%g)",pc.real(),pc.imag(),rcyc);
		}
		
		        //cal.draw_target(o.first,o.second*1.2);
        viewer.ftest=0;
		 if(!fmode) vo[l]=o.first;

		 {
			 double ix=vo[l].real(), iy=vo[l].imag();
			 fprintf(hf_log2," (vx,vy)=(%g;%g%)",ix,iy);
		 }
		wca.point=pointC;
		double tio;
		tio=cd.pause_time*1000./40.;//



		for(int n=1;n<40;n++)
		{
			//Sleep(100);
			if(check_signaled(hAbortEvent)) return empty;
			Sleep(tio);
			wca.update();
		}


		int wc=wca.width();
		int hc=wca.height();
		RECT cutoff;
		cd.camera.rect=make_rect(0,0,wc,hc);
		{
			RECT m=pcd->camera.margin;
			wca.set_margin_rect(m);
			RECT r=wca.rectvm;
		//	RECT r=make_rect(m.left,m.top,wc-m.right,hc-m.bottom);
			//RECT r=make_rect(m.left,m.bottom,wc-m.right,hc-m.top);
			cutoff=r;
		}

		{
			wcharbuf dumpf;
			wsprintf(dumpf,L"%s\\logs\\clb%d%03d.bmp",cd.appdir,fmode,l);
			wca.write_to_file(dumpf);

		}        

		detector.fenabled=true;
		tio=cd.time_out*1000.;
		for(int n=1;n<cd.naccumulate;n++)
		{
			if(check_signaled(hAbortEvent)) return empty;
			wca.update();
			//Sleep(200);
			// Beep(n*200,100);
			
			Beep(n*200,tio);
		}

             if(accumulator.is_empty())
			 {
				 SetEvent(hAbortEvent);
				 throw 1;
			 }

		


		//	
		double dolya= accumulator.threshold(cd.threshold_low,cd.threshold_high);

		{
			wcharbuf dumpf;
			wsprintf(dumpf,L"%s\\logs\\accum%03d.bi",cd.appdir,l);
			accumulator.write_to_file(dumpf);

		}
		//wca.pp.write_to_file("o:/accum.bi");
		
		int w=accumulator.width();
		int h=accumulator.height();

		double** ppd= &(accumulator.parray[0]);
		//
		
	    double rca=cd.scan_square_size*min(w,h);
		double eps=cd.median_epsilon;

/*

        distrib2D<double> F2D;
		F2D.make_from(ppd,w,h);

		//F2D.write_to_file("o:/distr.bi");
		double ppp=F2D.probab(make_rect(0,0,w,h));
		double ppcut=F2D.probab(cutoff);

		double probmax;
		//double rca=(1./3.)*min(w,h);
		
		RECT rt={0,0,rca,rca};
		//RECT rt={0,0,rca*w,rca*h};
		RECT r=F2D.max_rect(rt ,&probmax,cutoff);
		RECT rm=F2D.median(r,0.5-eps,0.5+eps);


		std::complex<double> ip=center_point<double>(rm);
*/

//std::complex<float_type> detect_center(float_type** ppd,int w,int h,RECT cuttoff,float_type rca,float_type eps=float_type())

              std::complex<double> ip=detect_center(ppd,w,h,cutoff,rca,eps);


			  {
				  double ix=ip.real(), iy=ip.imag();
				  fprintf(hf_log2," (cx,cy)=(%g;%g%)\r\n",ix,iy);
			  }

		if(!fmode) vi[l]=ip;
		else
		{
            std::complex<double> ott=projm*ip;
			cal.draw_target(ott,o.second);
			double rs=vo[l].real(),is=vo[l].imag();
			double rd=ott.real(),idd=ott.imag();
			double err=std::abs(vo[l]-ott);
			if(err>err_max) err_max=err;
			err_ev+=err;
			fprintf(hf_log,"[%2d] z_screen=(%g; %g) z_detect=(%g; %g) err=%g\r\n",l,rs,is,rd,idd,err);
			Sleep(1000);

		}



		int ox=vo[l].real(),oy=vo[l].imag(),ix=ip.real(),iy=ip.imag();  
		pointC.x=ix;pointC.y=iy;
		
		_cprintf("screen: %d %d camera: %d %d         \r",ox,oy,ix,iy);

		        viewer.ftest=1;
				detector.fenabled=false; 
				if(!fmode)
				{

				   
				tio=cd.time_out*1000.;
				cal.unitcycle(pc,rcyc,1);
				wca.point=pointC;
				for(int n=1;n<cd.naccumulate;n++)
				{
					if(check_signaled(hAbortEvent)) return empty;
					wca.update();
					//Sleep(200);
					// Beep(n*200,100);
					Beep((cd.naccumulate-n)*200,tio);
				}
				}

		//getchar();




	}


	std::complex<double> *i=&vi[0];
	std::complex<double> *o=&vo[0];

       if(check_signaled(hAbortEvent)) return empty;

  if(!fmode)
  {
	projm=projective_transform::make_proj_xi2(npoint, i, o);
      
	fprintf(hf_log,"projective_matrix:\r\n");     
	fprintf(hf_log," %g %g %g\r\n",projm.a,projm.b,projm.e);     
	fprintf(hf_log," %g %g %g\r\n",projm.c,projm.d,projm.f);     
	fprintf(hf_log," %g %g %g\r\n",projm.k,projm.l,projm.m);     
	fprintf(hf_log,"det:=%g\r\n",projm.det());     

	/*
	_cprintf("projective_matrix:\r\n");     
	_cprintf(" %g %g %g\r\n",projm.a,projm.b,projm.e);     
	_cprintf(" %g %g %g\r\n",projm.c,projm.d,projm.f);     
	_cprintf(" %g %g %g\r\n",projm.k,projm.l,projm.m);     
	_cprintf("det:=%g\r\n",projm.det());     
	*/
	


//	projmb=projm.back();


  }

}	//
         err_ev/=double(npoint);
         double dl=double(cal.rw.bottom+cal.rw.right)/2;
		 double err_ep=100*err_ev/dl,err_mp=100*err_max/dl;     
        fprintf(hf_log,"||err||_1=%g (%5.3f%%)  ; ||err||_inf=%g (%5.3f%%)\r\n",err_ev,err_ep,err_max,err_mp);    
	//	_cprintf("||err||_1=%g (%5.3f%%)  ; ||err||_inf=%g (%5.3f%%)\r\n",err_ev,err_ep,err_max,err_mp);    

	return projm;

};



struct laser_handler_screen:laser_handler_base
{
	projective_transform::projmatrix<double> pjm;

	HWND hwT;
	RECT monrect;
	bool fwinrect;
	int nmon;
	
	laser_handler_screen(projective_transform::projmatrix<double> &pj,int _nmon=0,HANDLE habort=0)
		:laser_handler_base(habort),fwinrect(1),nmon(_nmon)
	{
	
		monrect=ScreenCapture(nmon,0).monitor_rect();
		hwT=Capture_base::wnd_alloc("ffAq",0,0,WS_POPUPWINDOW,CreateSolidBrush(RGB(0,255,0)));
		pjm=pj;
	}

	~laser_handler_screen(){ DestroyWindow(hwT);}


	inline    bool operator()(POINT pc,int tio)
	{
		std::complex<double> co(pc.x,pc.y);
		std::complex<double> ci=pjm*co;

           POINT pi={ci.real(),ci.imag()};
		   POINT pmm={monrect.left+ci.real(),monrect.top+ci.imag()};

        draw_winrect(pmm,pi);
        return wait(tio);
	}
  
   inline    void  hide_winrect()
   {
	   if(fwinrect) ShowWindow(hwT,SW_HIDE);
   }
   inline    void  draw_winrect(POINT pmm,POINT pi,bool fshow=true)
	{

	       if(!fwinrect) return ;	 
		   if(!fshow) {hide_winrect();return;}

		//unsigned short sks=GetKeyState(VK_LCONTROL);
		//unsigned short sksw=GetKeyState(0x5b);

		
			
			
			SetWindowPos(hwT,HWND_TOPMOST,pmm.x-15,pmm.y-15,15,15,SWP_SHOWWINDOW);

			


			double det=pjm.det();
			_cprintf("[%04d:%04d:] |pjm|=%g\r",pi.x,pi.y,det);
		
		
	}

};

struct previewer_calibrator:previewer_base
{
    
	POINT pold;
	previewer_calibrator(HWND hwin=HWND(-1),int hshift=40,HPEN hp=CreatePen(PS_SOLID,1,RGB(0,255,0)))
		:previewer_base(hwin,hshift,hp)
	{
		pold=POINT();
		
	}

	inline void   operator()(Capture_base* pcap,unsigned char* pbytes,POINT & point,RECT& rect,void* pUserData=0)
	{
		base_ref()(pcap,pbytes,point,rect);
		if(dc)
		{

			
             int ys=hshift;
			 /*
			 RECT rrect={rect.left,rect.top+ys,rect.right,ys+rect.bottom};
   			FrameRect(dc,&rrect,(HBRUSH)hbrush.hobj);
			*/
            if(!ftest) return;


			int w=pcap->width();
			int h=pcap->height();

			//StretchDIBits(dc,0,ys,w,h,0,0,w, h,pbytes,*this,DIB_RGB_COLORS, SRCCOPY);
			
			SelectObject(dc,hpen);
			//float rr=100;

			//double rr=(1./5.)*min(w,h),rh=rr/5;
			double fw=fabs(double(rect.right-rect.left));
			double fh=fabs(double(rect.bottom-rect.top));
			double rr=(1./5.)*min(fw,fh),rh=rr/5;
			
			int x=point.x;
			int y=ys+h-point.y;
			MoveToEx(dc,x-rr,y,0);LineTo(dc,x+rr,y);
			MoveToEx(dc,x,y-rr,0);LineTo(dc,x,y+rr);
			//rr-=25;
            //for(;rr>0;rr-=25) 
			rr-=rh;
			for(;rr>0;rr-=rh)
			Arc(dc,x-rr,y-rr,x+rr,y+rr,x-rr,y-rr,x-rr,y-rr);





/*
			MoveToEx(dc,point.x-rr,ys+h-point.y,0);
			LineTo(dc,point.x+rr,ys+h-point.y);
			MoveToEx(dc,point.x,ys+h-point.y-rr,0);
			LineTo(dc,point.x,ys+h-point.y+rr);
*/
		}
	}

};



struct previewer_laser:previewer_base
{

	POINT pold,p_in,p_out;

	int flags;
	previewer_laser(HWND hwin=HWND(-1),int _flags=0,int hshift=40,HPEN hp=CreatePen(PS_SOLID,1,RGB(128,128,128)))
		:previewer_base(hwin,hshift,hp),flags(_flags)
	{
		
		pold=POINT();
	}

	inline void   operator()(Capture_base* pcap,unsigned char* pbytes,POINT & point,RECT& rect,void* pUserData=0)
	{
		const RECT NullRect={0,0,0,0};
		if(ftest&&(dc))
		{
			int w=pcap->width();
			int h=pcap->height();
			int ys=hshift;

			//StretchDIBits(dc,0,ys,w,h,0,0,w, h,pbytes,*this,DIB_RGB_COLORS, SRCCOPY);
			
			//if(0)
			if(!pUserData)
				base_ref()(pcap,pbytes,point,rect);
			else
			{
				CALIBRATE_DATA& cd=*((CALIBRATE_DATA*)pUserData);
				CALIBRATE_DATA::camera_t::laser_t& lsr=cd.camera.laser;
				RECT er={0,0,0,0};
                 base_ref()(pcap,pbytes,point,er);
				 bool fi,fo;
				if((fo=!IsRectNull(lsr.margin.outer)))
				{
					RECT &r=lsr.area.outer;
					RECT rrect={r.left,r.top+ys,r.right,ys+r.bottom};
					FrameRect(dc,&rrect,(HBRUSH)hbrush.hobj);
				}
				if((fi=!IsRectNull(lsr.margin.inner)))
				{
					RECT &r=lsr.area.inner;
					RECT rrect={r.left,r.top+ys,r.right,ys+r.bottom};
					FrameRect(dc,&rrect,(HBRUSH)hbrush2.hobj);
				}

				if((flags&4)&&fi&&fo)
				{
              
					SelectObject(dc,hpen);
					MoveToEx(dc,cd.raw_pt_i.x,hshift+h-cd.raw_pt_i.y,0);
					LineTo(dc,cd.raw_pt.x,hshift+h-cd.raw_pt.y);
					return;

				}

			}
			//else
				

			{
				SelectObject(dc,hpen);
				MoveToEx(dc,pold.x,hshift+h-pold.y,0);
				LineTo(dc,point.x,hshift+h-point.y);
				pold=point;
			}
			

			
			
		}
	}

};




