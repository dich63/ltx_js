#pragma once

#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <conio.h>
#include "winconsole.h"
/*
#include "webcam_capture.h"
//#include "wchar_parsers.h"
#include "screencapture.h"
#include "orderstat.h"
#include "prjtransform.h"
#include "conio.h"
#include "math.h"
*/
#include "singleton_utils.h"
#include "lasercalibrate_0_0.h"
//#include "lasercalibrate.h"
#include "video/hssh_desktop_service.h"

#define LASER_HOOK_DIR L"laser_hook"

extern HMODULE g_hModule;

typedef projective_transform::projmatrix_base<double> projmatrix_base;
typedef projective_transform::projmatrix<double> projmatrix;

extern ULONG32 mouse_hook_flags;
extern ULONG32 kbrd_hook_flags;

extern host_singleton_t  host_abortifacient;




struct laser_shared_data
{  
  LONG32  size;
  LONG32 hooker_pid;
  ULONG32  flags;
  ULONG32  kb_state;
  ULONG32  state;
  RECT screen_rect;
  RECT camera_rect;
  double screen_point[3];
  double camera_point[3];
  union{
  struct{ double projective_matrix[3*3];};
  projmatrix_base projm;
  };
  camera_types::camera_t camera_laser_detector;


 inline bool set_state_bit(int num){
      return InterlockedBitTestAndSet((LONG volatile*)&state,num); 
  };
inline  bool clear_state_bit(int num) {
	 return InterlockedBitTestAndReset((LONG volatile*)&state,num); 
 };

inline  ULONG32  get_state()
 {
        return InterlockedExchangeAdd((LONG volatile*)&state,0);
 }
inline  bool  get_bit_state(int n)
{ 
	ULONG32 s=get_state();
   return s&(1<<n);
}
inline  ULONG32  set_state(ULONG32 newstate )
{
	return InterlockedExchange((LONG volatile*)&state,newstate);
}

inline bool set_laser_trace_bit(bool f)
{
	return (f)? set_state_bit(31):clear_state_bit(31);
};

};

struct laser_shared_arrea_t:shared_arrea_t
{
	laser_shared_arrea_t():shared_arrea_t(get_libprrx(),LASER_HOOK_DIR){};
inline	operator laser_shared_data*()
	{
		return (laser_shared_data*)ptr;
	}
inline laser_shared_data*	operator ->()
	{
		return (laser_shared_data*)ptr;
	}
};





struct entry_point_base_t
{
	
	
	argv_ini<wchar_t> args_ini;
	argv_zzbase<wchar_t> args_env;
	argv_file<wchar_t> args_file;
	argv_file<wchar_t> args_file2;
	argv_def<wchar_t> args;
	
    wcharbuf appdir;
	v_buf<wchar_t> venv;

	//args_f<wchar_t>  args_file;
	entry_point_base_t(int argc, wchar_t** ppargs)
		:args_ini(L"default.options",L".ini",g_hModule),
		args(argc,ppargs,L"laser.hook.prefix")	{

        args_env.parent=&args_ini;
		args_file.parent=&args_file2;
		args_file2.parent=&args_env;
		args.parent=&args_file;

		args_file.open(args[L"@"].def<wchar_t*>(0));

		args_file2.open(expand_env(args_file[L"@"].def<wchar_t*>(0)));

		if(args[L"env"].def<bool>(0))
		{
			wchar_t *penv=GetEnvironmentStringsW();
           args_env.set_args(copy_zz(penv,venv));
		   FreeEnvironmentStringsW(penv);

		}
		 appdir=get_app_dir(LASER_HOOK_DIR); 
		 get_app_dir(LASER_HOOK_DIR L"\\logs"); 
		 

		//args.parent=args_file.pargs();
         //float g0= args.getval(L"fps");
	}
	~entry_point_base_t()
	{
		
	}
virtual void operator()(){};
};


struct shared_laser_handler_screen:laser_handler_screen
{
	
	laser_shared_arrea_t& lsa;
	bool fcursor;
	bool fbimax;
	ULONG32 flags;
	bool fremote;
	hssh_desktop_client_control remote_control;

	shared_laser_handler_screen(laser_shared_arrea_t& _lsa ,projective_transform::projmatrix<double> &pj,int nmon=0,ULONG32 _flags=1)
	:laser_handler_screen(pj,nmon,host_abortifacient.hAbortEvent),lsa(_lsa),flags(_flags)
	{
	   fwinrect=flags&1;
	   fcursor=flags&2;
	   fbimax=flags&4;
	   lsa->set_laser_trace_bit(0);
	   fremote=0;

	}

	bool remote_control_init(const wchar_t* host_port,int rcmode=0,const wchar_t* ns_path=_wgetenv(L"laser.hook.hssh_port"))
	{
		if(!host_port) remote_control.close();
		else
		{
			remote_control.reconnect_mode=rcmode;
         if(remote_control.init(host_port,nmon,ns_path))
		      monrect=remote_control.rscr;
		}

		return remote_control;
	}

	//
	inline void set_shared_data(bool f,std::complex<double>& cscreen,std::complex<double>& ccamera,CALIBRATE_DATA* pcd)
	{

		locker_t<laser_shared_arrea_t> lock(lsa);

		lsa->screen_point[0]=cscreen.real();
		lsa->screen_point[1]=cscreen.imag();
		lsa->screen_point[2]=1;

		lsa->camera_point[0]=ccamera.real();
		lsa->camera_point[1]=ccamera.imag();
		lsa->camera_point[2]=1;

		
		if(pcd)
		{

			pcd->camera.factive=f;
			pcd->camera.state=flags;

			lsa->camera_laser_detector=pcd->camera;
		}
		else
		{
			lsa->camera_laser_detector.factive=f;
			lsa->camera_laser_detector.state=flags;
		}
		


	};
     inline void _draw_winrect(POINT& xy,POINT &pi,bool fshow=true)
	 {
		 if(remote_control)
		 {
             remote_control.draw_detection(fshow,xy,pi,false,fwinrect);
			 monrect=remote_control.rscr;

			 //			 draw_winrect(xy,pi,fshow);
		 }
		 else   draw_winrect(xy,pi,fshow);
	 };
	inline void draw_detection(bool f,POINT& xy,POINT &pi)
	{

		if(remote_control)
		{
			remote_control.draw_detection(f,xy,pi,f&&fcursor,fwinrect);
			monrect=remote_control.rscr;

		}
		else
		{
	
		 if(f) {
		 	draw_winrect(xy,pi);
			if(fcursor) SetCursorPos(xy.x,xy.y);
		 }
		 else { 
			hide_winrect();
		 }
		}
	}

	inline void dump_coord(CALIBRATE_DATA& cd,POINT pt,POINT pt_raw)
	{
		if(!is_console()) return;
		wcharbuf buf;
		int nm=-1;
		int factive=0;
		double fmax=0,fmean=0,flo=0,fhi=1;



		
		if(&cd) 
		{
			nm=cd.nmon;
			factive=cd.camera.factive;
			fmax=cd.camera.laser.image_data.outer.maxval;
			fmean=cd.camera.laser.image_data.outer.meanval;
			flo=cd.camera.laser.power_lo;
			fhi=cd.camera.laser.power_hi;


		}
		//coord con_progress_bar(double val,double maxval,int consize,wchar_t f=L'+',wchar_t b='o',DWORD attrf=M_FC|M_BC,DWORD attrb=M_FC|M_BC,coord cxy=consoleXY(),HANDLE hout=GetStdHandle(STD_OUTPUT_HANDLE))
		
		coord cxy=consoleXY();

		const int consiz=57,iys=1;


		if(0)
		{
		
		consoleXY()=coord(1,iys);
		conputs_h(0x74,v_buf<wchar_t>().printf(L"lo=%2.2f%% hi=%2.2f%%     ",flo/2.56,(fhi/2.56)));
		consoleXY()=coord(25,iys);
		con_progress_bar(0,256,consiz,'>','>',0x1a,0x74);
		con_progress_bar(256*flo/fhi,256,(consiz*fhi)/256.,'<','.',0x71,0x72);


		consoleXY()=coord(1,iys+1);
		conputs_h(0x74,v_buf<wchar_t>().printf(L"mean=%2.2f%% max=%2.2f%%    ",fmean/2.56,(fmax/2.56)));
		consoleXY()=coord(25,iys+1);
		if(fmax<flo)
		con_progress_bar(fmax,256,consiz,'|','.',0x1a,0x72);
		else if(fmax>fhi)
		  con_progress_bar(fmax,256,consiz,'|','.',0x4c,0x72);
		    else  con_progress_bar(fmax,256,consiz,'|','.',0x24,0x72);
		}
		else
		{

			consoleXY()=coord(1,iys);
			conputs_h(0x74,v_buf<wchar_t>().printf(L"mean=%2.2f%% max=%2.2f%%    ",fmean/2.56,(fmax/2.56)));

			coord cc(21,iys);
			int i_lo=(double(consiz)*flo)/256.,i_hi=(double(consiz)*fhi)/256.;
			wchar_rep_t wr[3]={{i_lo,0x11},{i_hi-i_lo,7},{consiz-i_hi,0x10}};	
            cons_fill(wr,3,cc);
			consoleXY()=cc;
			if(fmax<flo)
				con_progress_bar(fmax,256,consiz,0,0,0x1a,0x81);
			else if(fmax>fhi)
				con_progress_bar(fmax,256,consiz,0,0,0x0c,0x81);
			else  con_progress_bar(fmax,256,consiz,0,0,0x24,0x72);

		}


			
			/*
			consoleXY()=coord(1,0);
			conputs_h(0xF4,v_buf<wchar_t>().printf(L"<m>=%2.2f%%",fmean/2.56));
			con_progress_bar(fmean,256,60,'|','.',0x4C,0x82);
			*/

		consoleXY()=cxy;

		wsprintf(buf,L"[%d] monitor_%d:[%d,%d;%d,%d] laser_pt=(%d,%d) raw_pt=(%d,%d)",factive,
			nm,monrect.left,monrect.top,monrect.right,monrect.bottom,pt.x,pt.y,pt_raw.x,pt_raw.y);
		SetConsoleTitleW(buf);
	}

	inline    bool operator()(POINT pc,int tio,void* pUserData=0){
		
		CALIBRATE_DATA& cd=*((CALIBRATE_DATA*)pUserData);


		if(fbimax&&(pUserData))
		{
			CALIBRATE_DATA::camera_t::laser_t::image_data_t& img=cd.camera.laser.image_data;  

            bool f;

			double omaxv=img.outer.maxval;
			double imaxv=img.inner.maxval;

			POINT& pout=img.outer.point;
			POINT& pinner=img.inner.point;

			//f=imaxv>omaxv;
			RECT& rin=cd.camera.laser.raw_area.inner;
			RECT& rout=cd.camera.laser.raw_area.outer;
			 
			f=PtInRect(&rin,pout);


			

			//			POINT& pcc=(f)?pinner:pout;
			POINT& pc=pout;

			std::complex<double> co(pc.x,pc.y);
			std::complex<double> ci=pjm*co;
            std::complex<double> co_i(pinner.x,pinner.y);
			


			POINT pi={ci.real(),ci.imag()};
			//POINT pi_i={ci_i.real(),ci_i.imag()};

			cd.raw_pt=pout;
			cd.raw_pt_i=pinner;
			
			POINT pmm={monrect.left+ci.real(),monrect.top+ci.imag()};
			

			draw_detection(f,pmm,pi);
			set_shared_data(f,ci,co,&cd);
			dump_coord(cd,pmm,pc);


		}
		else {

			std::complex<double> co(pc.x,pc.y);
			std::complex<double> ci=pjm*co;


			double dt=pjm.det();
              projective_transform::projmatrix<double> pjmb=pjm.back();
          double dtb=pjmb.det();
			std::complex<double> cbb=pjmb*co;
			

			POINT pi={ci.real(),ci.imag()};
			POINT pmm={monrect.left+ci.real(),monrect.top+ci.imag()};

			int fdetect=0;
			bool f;

			if((fdetect=mouse_hook_flags))
			{
                f= lsa->get_bit_state(31);
     		}
			else

			if((fdetect=(flags&(1<<4))))
			{
				CALIBRATE_DATA::camera_t::laser_t &ld= cd.camera.laser;

				f= cd.camera.factive;
				double fmax=ld.image_data.outer.maxval;
				if(ld.power_lo>fmax)
					f=false;
				else if(ld.power_hi<fmax)
					f=true;
               cd.camera.factive=f;

			}


			if(fdetect){
          
			  
//
			  /*
			  if(f) {
				  draw_winrect(pmm,pi);
				  if(fcursor) SetCursorPos(pmm.x,pmm.y);
				}
			else { 
			  hide_winrect();
		     }
//*/
       // 
			  draw_detection(f,pmm,pi);
            set_shared_data(f,ci,co,&cd);
			dump_coord(cd,pmm,pc);
			

			/*
			locker_t<laser_shared_arrea_t> lock(lsa);

			lsa->screen_point[0]=ci.real();
			lsa->screen_point[1]=ci.imag();
			lsa->screen_point[2]=1;

			lsa->camera_point[0]=co.real();
			lsa->camera_point[1]=co.imag();
			lsa->camera_point[2]=1;
			if(pUserData)
			{
            lsa->camera_laser_detector=cd.camera;
		    }
			*/
		  //


	   }
	   else _draw_winrect(pmm,pi);
	   dump_coord(cd,pmm,pc);
	}
		return wait(tio);
  }


};


inline ULONG32 set_bit(ULONG32 s,int n,bool f=true)
{
    ULONG32 m=(1<<n);
   if(f) return s|m;
   else return s&(~m);
}






#define LASER_DATA_SET_DEF(args,cd,a,b) cd.##a=args[L#a].def(b);
#define LASER_DATA_SET_REDEF(args,cd,a)  LASER_DATA_SET_DEF(args,cd,a,cd.a)

struct entry_point_t:entry_point_base_t
{
	CALIBRATE_DATA cd;
	host_singleton_t host_singleton;
	laser_shared_arrea_t laser_shared_arrea;

	ULONG32 mask;




	entry_point_t(int argc, wchar_t** ppargs)
		:entry_point_base_t(argc,ppargs)
		,host_singleton(get_libprrx(),true)
		,mask(0)
	{
		//

		
         cd=CALIBRATE_DATA();

        int lastmon=ScreenCapture::monitor_count()-1;	
		cd.nmon=lastmon;
        cd.ncam=0;
        cd.fps=25;
		cd.npoint=17;
		cd.rdisk=0.035*5;
		cd.rcycle=1./3.;
		cd.threshold_low=1./3.;
		cd.threshold_high=1.1;//2./3.;
		cd.scan_square_size=1./3.;
		cd.median_epsilon=0.49;
		cd.naccumulate=5;
		cd.pause_time=4;
		cd.time_out=0.1;
		cd.appdir=appdir;
         //return;
        //int g0= args.getval(L"nmon");
		//int gg=args[L"nmon"].def(0);
		LASER_DATA_SET_REDEF(args,cd,nmon);
		LASER_DATA_SET_REDEF(args,cd,ncam);
		LASER_DATA_SET_REDEF(args,cd,fps);
		LASER_DATA_SET_REDEF(args,cd,npoint);
		LASER_DATA_SET_REDEF(args,cd,rdisk);
		LASER_DATA_SET_REDEF(args,cd,rcycle);
		LASER_DATA_SET_REDEF(args,cd,threshold_low);
		LASER_DATA_SET_REDEF(args,cd,threshold_high);
		LASER_DATA_SET_REDEF(args,cd,scan_square_size);

		LASER_DATA_SET_REDEF(args,cd,median_epsilon);
		LASER_DATA_SET_REDEF(args,cd,naccumulate);
		LASER_DATA_SET_REDEF(args,cd,pause_time);
		LASER_DATA_SET_REDEF(args,cd,time_out);
		LASER_DATA_SET_REDEF(args,cd,camera.margin.left);
		LASER_DATA_SET_REDEF(args,cd,camera.margin.right);
		LASER_DATA_SET_REDEF(args,cd,camera.margin.top);
		LASER_DATA_SET_REDEF(args,cd,camera.margin.bottom);

		LASER_DATA_SET_REDEF(args,cd,camera.laser.margin.outer.left);
		LASER_DATA_SET_REDEF(args,cd,camera.laser.margin.outer.right);
		LASER_DATA_SET_REDEF(args,cd,camera.laser.margin.outer.top);
		LASER_DATA_SET_REDEF(args,cd,camera.laser.margin.outer.bottom);

		LASER_DATA_SET_REDEF(args,cd,camera.laser.margin.inner.left);
		LASER_DATA_SET_REDEF(args,cd,camera.laser.margin.inner.right);
		LASER_DATA_SET_REDEF(args,cd,camera.laser.margin.inner.top);
		LASER_DATA_SET_REDEF(args,cd,camera.laser.margin.inner.bottom);

        

		


		read_mask();

	};


void read_mask()
{
   ULONG32 ff;

	mask=ULONG32(args[L"mask"].def(0));

	if(!args)
	{


		ff=args[L"mask.camera.preset"].def(0);
		mask|=(0x0E&ff);

		ff=args[L"mask.camera.preset.format"].def(0);
		if(ff) mask|=4; 
		ff=args[L"mask.camera.preset.source"].def(0);
		if(ff) mask|=2; 

		double ffdeee=args[L"deep.deeep.test.preview"];

		ff=args[L"mask.camera.preset.preview"].def(0);
		if(ff) mask|=8; 


		ff=args[L"mask.calibrate"].def(0);
		if(ff) mask|=0x10;

		ff=args[L"mask.laser"].def(0);
		if(ff) mask|=0x20;
		ff=args[L"mask.console"].def(0);
		if(ff)	 mask|=(0x0F);

	}


};

inline bool is_action()
{
  return ((~mask)&0xFF);
}





virtual void operator()()
{
    wchar_t buf[256];
	ULONG32 ff;


	//if(((~mask)&0xFF)==0)
	 if(!is_action()){host_abortifacient.abort();return;}

	 get_cam_map();

	mutex_t worked_mutex(get_libprrx(),L"_worked_mutex");

	locker_t<mutex_t> lock_worked(worked_mutex);


	HANDLE habort = host_abortifacient.hAbortEvent;

	mask=~mask;
	ULONG32 dlg=(mask&0x7)>>1;

     if(args[L"video.dlg.compression"].def(0)) dlg|=4;


	 int vvcc=sizeof(Capture_base::BIHPAL);
	 bool conspar=args[L"console.parent"].def(0);
     if((0x0F&mask))
	 {
		 if(conspar)
			 SetEnvironmentVariableW(get_libprrx(),L"laser.hook");
			  
		 wchar_t *p=_wgetenv(get_libprrx());
		 if(p)
		 {
			 if(AttachConsole(ATTACH_PARENT_PROCESS)==0)
				 p=0;

		 }

		 if(!p)
		 {
			 AllocConsole();
			 SetEnvironmentVariableW(get_libprrx(),L"laser.hook");
		 }
	 }
        if(is_console())
		{
			
			RECT cr=WebCamInfo(cd.ncam).cam_rect();
            RECT sr=ScreenCapture(cd.nmon,0).monitor_rect();

			wsprintf(buf,L"laser.hook : camera_%d:[%d,%d;%d,%d] monitor_%d:[%d,%d;%d,%d]"
				,cd.ncam,cr.left,cr.top,cr.right,cr.bottom,cd.nmon,sr.left,sr.top,sr.right,sr.bottom);
			SetConsoleTitleW(buf);
			//SetConsoleTitleW(GetCommandLineW());
			if(!conspar)
			{
	
				int w=args[L"console.width"].def(90);
				int h=args[L"console.height"].def(50);
			set_console_size(coord(w,h));
			set_console_attr(args[L"console.attr"].def(0x73));
			 clrscr(1);
			 consoleXY().bottom(0,-2);
			};
			 int fffh;
			 if(args[L"console.cmd"].def(0))
			 {
				 
				 

				//fffh= system("cmd /k set");
				 //fffh= system("cmd /k ");
				 start_process(L"cmd.exe /k \"prompt :$&&set PATHEXT=.lsrhk;%PATHEXT%\"",0);
				//FatalAppExitA(0,"KKKKKKKKKKKKKK");
				//ExitProcess(0);
				
			}

		}

		if(host_abortifacient.signaled()) return;

	 if(0x0F&mask)
	 {

		 previewer_base viewer=GetConsoleWindow();
		   
	    WC_view_only wbcam(viewer,cd.ncam,dlg);

		wbcam.pUserData=&cd;

		RECT cr=wbcam.cam_rect();
		RECT sr=ScreenCapture(cd.nmon,0).monitor_rect();
        RECT mr=wbcam.set_margin_rect(cd.camera.margin);
		char* fcc=wbcam.get_fourcc();

		wsprintf(buf,L"laser.hook : camera{%S}_%d:[%d,%d;%d,%d][%d,%d;%d,%d] monitor_%d:[%d,%d;%d,%d]",
			fcc,cd.ncam,cr.left,cr.top,cr.right,cr.bottom,
			mr.left,mr.top,mr.right,mr.bottom,cd.nmon,sr.left,sr.top,sr.right,sr.bottom);

		SetConsoleTitleW(buf);
      if(0x08&mask) 
	  {
		   Calibrate cal(cd.screen_rect=ScreenCapture(cd.nmon,0).monitor_rect(),RGB(255,255,255));
		   hssh_desktop_client_calibrate rcal;
		   
		  if(args[L"preview.showcalibrate"].def(0))
		  {
			  wchar_t* host_port=args[L"monitor.host_port"];
			  if(!args)
			  {
				    calibrate_static(cal,cd);
			  }
			  else
			  {
				 
				  if(!rcal.init(host_port,cd.nmon,RGB(255,255,255))) return;
				  cd.screen_rect=rcal.rscr;
				 calibrate_static(rcal,cd);
			  }
           
		  }
		  laser_detect(wbcam,laser_handler_kb(args[L"viewer_break"].def(VK_ESCAPE),habort),cd.fps); 
		  ExitProcess(0);
	  }
	  
	 }

	 if(host_abortifacient.signaled()) return;

	 if(0x10&mask)
	 {
		 double ar=args[L"calibrate.alpha.red"].def(1);
         double ag=args[L"calibrate.alpha.green"].def(0);
		 double ab=args[L"calibrate.alpha.blue"].def(0);

         double asum=ar+ag+ab;
		 if(fabs(asum)>0) 
		 {
			 ar/=asum;
			 ag/=asum;
			 ab/=asum;
		 }

		 RGB24_getter calibrate_color_get(ar,ag,ab);

		 CamCalibratePrjMatrixDetector<RGB24_getter> ccpmd(calibrate_color_get);
		 previewer_calibrator pvcc=GetConsoleWindow();
		 WC_laser_base<CamCalibratePrjMatrixDetector<RGB24_getter>,previewer_calibrator> ccwbcam(ccpmd,pvcc,cd.ncam);

		 projmatrix pj;

           
          wchar_t* host_port=args[L"monitor.host_port"];
		  if(!args)
		  {
			  Calibrate cal(cd.screen_rect=ScreenCapture(cd.nmon,0).monitor_rect(),RGB(255,255,255));
			  pj=make_cam_calibrate(cal,ccwbcam,&cd,habort);
		  }
		  else
		  {
			  hssh_desktop_client_calibrate cal(host_port,cd.nmon,RGB(255,255,255));
			  if(!cal) return;
			  cd.screen_rect=cal.rscr;
			  pj=make_cam_calibrate(cal,ccwbcam,&cd,habort);
		  }

		 

		 if(host_abortifacient.signaled()) return;

		 locker_t<laser_shared_arrea_t>  lock(laser_shared_arrea);
		 laser_shared_arrea->projm=pj;
		 laser_shared_arrea->camera_rect=cd.camera.rect;
		 laser_shared_arrea->screen_rect=cd.screen_rect;

		 if(is_console())
		 {
			 set_console_attr(0x1a);
			 v_buf<wchar_t> vcmd;
			 vcmd.printf(L"cmd.exe /c type \"%s\\logs\\calibrate.log.txt\"",cd.appdir);
			 start_process(vcmd.get(),0);
		 }
		 ExitProcess(0);

   	 }
      if(host_abortifacient.signaled()) return;
	 if(0x20&mask)
	 {


          projmatrix pj;

		 {
			 locker_t<laser_shared_arrea_t>  lock(laser_shared_arrea);
			 laser_shared_data* lp=laser_shared_arrea;
			 pj=laser_shared_arrea->projm;
		 }

		   ULONG32 curflags;
		   

		   curflags=args[L"cursor.mode"].def(1);

		   if(!args)
		   {
   		   
		   if(args[L"cursor.mode.cursor"].is())
		   {
                curflags=set_bit(curflags,1,args[L"cursor.mode.cursor"].def<bool>(0));
		   }

		   if(args[L"cursor.mode.rect"].is())
		   {
			   curflags=set_bit(curflags,0,args[L"cursor.mode.rect"].def<bool>(0));
		   }
		   if(args[L"cursor.mode.bimax"].is())
		   {
			   curflags=set_bit(curflags,2,args[L"cursor.mode.bimax"].def<bool>(0));
		   }
		   if(args[L"cursor.mode.power"].is())
		   {
			   curflags=set_bit(curflags,4,args[L"cursor.mode.power"].def<bool>(0));
		   }

		   }


		   double ar=args[L"laser.alpha.red"].def(0);
		   double ag=args[L"laser.alpha.green"].def(1);
		   double ab=args[L"laser.alpha.blue"].def(0);
		   bool  fas=args[L"laser.alpha.set"].def<int>(0);

		   fas=args[L"laser.alpha.show"].def<int>(fas);

		   RGB24_getter laser_color_get(ar,ag,ab,fas);

		   LaserPointDetector<RGB24_getter> lpd(laser_color_get);
		   previewer_laser pv(GetConsoleWindow(),curflags);

		   cd.camera.laser.power_lo=255.9*args[L"laser.ray.power.lo"].def<double>(32./256.);
		   cd.camera.laser.power_hi=255.9*args[L"laser.ray.power.hi"].def<double>(32./256.);

		   //		   laser.ray.power.hi=40
		   //		   laser.ray.power.lo=10


		   WC_laser_base<LaserPointDetector<RGB24_getter>,previewer_laser> wbcam(lpd,pv);

		   wbcam.previewer.modulo=args[L"previewer.modulo"].def(1);

		   cd.laser_regions_setup(wbcam);
		   wbcam.pUserData=&cd;
		   //reinit_conout();
		   _cprintf("start...\n");
/*
		   HANDLE hConout = CreateFileW(L"CONOUT$",GENERIC_WRITE,FILE_SHARE_WRITE,
			   NULL,  OPEN_EXISTING,  FILE_ATTRIBUTE_NORMAL,NULL);
		   SetStdHandle(STD_OUTPUT_HANDLE,hConout);
		    _cprintf("start...\n");
			_cputs()
*/
		   shared_laser_handler_screen lsb(laser_shared_arrea,pj,cd.nmon,curflags);

		   wchar_t* host_port=args[L"monitor.host_port"];
		   if(host_port)
		   {
			   //lsb.remote_control.init(host_port,cd.nmon);
			   lsb.remote_control_init(host_port,args[L"monitor.host_port.reconnect"].def(1));
		   }





		 //laser_handler_screen 

		 // int maxframes=args[L"laser.framecount"].def(-1);
		  int maxframes=args[L"laser.framecount"].def(500);
            
          laser_detect(wbcam,lsb,cd.fps,maxframes); 
		  //SetConsoleTitleA("+++Stop Detect++"); 
		  ExitProcess(0); 
		 
	 }


	SetConsoleTitleA("+++Stop++"); 
	ExitProcess(0);
	host_singleton.abort();

};
};

//QueueUserWorkItem((LPTHREAD_START_ROUTINE)&s_proc,this,WT_EXECUTELONGFUNCTION)
