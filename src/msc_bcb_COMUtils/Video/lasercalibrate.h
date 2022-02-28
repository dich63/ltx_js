#pragma once

#include "calibrate_data.h"
#include "webcam_capture.h"
#include "screencapture.h"
#include "orderstat.h"
#include "prjtransform.h"
#include "singleton_utils.h"

#include "conio.h"
#include "math.h"

#include <utility>

#define RED_C 0
#define GREEN_C 1
#define BLUE_C 2

struct Calibrate
{
	RECT rw,rscr; 
	//HDC dc;
	DC_holder dc;
	//HBRUSH hbg;
	HGDI_holder hbg;
	HGDI_holder  hfg;
    HGDI_holder  hpen,hpen_target;
	HWND hwin;

	Calibrate():hwin(0){}
	Calibrate(RECT rm,DWORD diskColor=RGB(255,255,255),DWORD bkcol=RGB(0,0,0))
	{
		rscr=rm;
		RECT t={0,0,rm.right-rm.left,rm.bottom-rm.top};
		rw=t;

		hwin=Capture_base::wnd_alloc("YYYCalirate00");
		dc.attach(hwin);
		//=GetDC(hwin);
		hbg.attach(CreateSolidBrush(bkcol));
		hfg.attach(CreateSolidBrush(diskColor));
		hpen.attach(CreatePen(PS_SOLID,2,RGB(0,0,64)));
		hpen_target.attach(CreatePen(PS_SOLID,1,RGB(0,255,0)));
		int rrr=SetTextColor(dc,RGB(0,128,0));
		
	};

             
	~Calibrate()
	{
		if(hwin!=0) DestroyWindow(hwin);
	}

	inline      void invert()
	{
		hbg.hobj=make_detach(hfg.hobj,hbg.hobj);
	}

	inline  void show(bool fshow=true)
	{
		HWND hwp=HWND_TOPMOST;//hwp=HWND_TOP;
		//dc=GetDC(hwin);
		if(fshow)
		{
			SetWindowPos(hwin,hwp,rscr.left,rscr.top,rscr.right-rscr.left,rscr.bottom-rscr.top,SWP_SHOWWINDOW);
			clear();
		}
		else hide();
		//ReleaseDC(hwin,dc);

	}
	inline void clear()
	{
		SelectObject(dc,hbg);
		Rectangle(dc,rw.left,rw.top,rw.right,rw.bottom);
	}
	inline   void hide()
	{
		ShowWindow(hwin,0);
	}
	inline  void drawcycle(int x,int y,int rc,bool fclear=false,bool finvert=false)
	{

		RECT r={x-rc,y-rc,x+rc,y+rc};
		if(finvert) invert();

		if(fclear) clear(); 
		if(!IsRectEmpty(&r))
		{
       	SelectObject(dc,hfg);
		Ellipse(dc,r.left,r.top,r.right,r.bottom);
		}
		if(finvert) invert();
	}

	inline  void drawrect(int l,int t,int r,int b,bool fclear=true,bool finvert=false)
	{
          if(finvert) invert();
		if(fclear) clear();         		
		SelectObject(dc,hfg);
		Rectangle(dc,l,t,r,b);
		if(finvert) invert();
	}

	inline  void drawrect(std::vector<int>& vr,bool fclear=true,bool finvert=false)
	{
		  
		  int l=rw.left,t=rw.top,r=rw.right,b=rw.bottom;
		  int s=vr.size();
		  if(s>0) l=vr[0];
		  if(s>1) t=vr[1];
		  if(s>2) r=vr[2];
		  if(s>3) r=vr[3];

          drawrect(l,t,r,b,fclear);
    }

	inline  void drawpolygon(std::vector<LONG>& vr,bool fclear=true,bool finvert=false)
	{
        int c=vr.size();
		if(c&&((c&1)==0))
		{

		POINT* pp=(POINT*)(&vr[0]);
		if(finvert) invert();
		if(fclear) clear();         		
		SelectObject(dc,hfg);
		Polygon(dc,pp,c/2);
		if(finvert) invert();
		}
	}





	template<class float_type>
	void  draw_target(std::complex<float_type> z,float_type rc,int tic=4)
	{

		SelectObject(dc,hpen_target);
		//float rr=100;

		//double rr=(1./5.)*min(w,h),rh=rr/5;
		
		//double rr=(1./tic)*rc,rh=rr/tic;
		double rr=rc,rh=rr/tic;

		int x=z.real();
		int y=z.imag();
		double re=1.2*rr;
		MoveToEx(dc,x-re,y,0);LineTo(dc,x+re,y);
		MoveToEx(dc,x,y-re,0);LineTo(dc,x,y+re);
		//rr-=25;
		//for(;rr>0;rr-=25) 
		//rr-=rh;
		for(;rr>0;rr-=rh)
			Arc(dc,x-rr,y-rr,x+rr,y+rr,x-rr,y-rr,x-rr,y-rr);
		SelectObject(dc,hpen);

	}
	template<class float_type>
	inline  std::pair<std::complex<float_type>,float_type> unitcycle(std::complex<float_type> z,float_type rc,bool fclear=true,bool finvert=false)
	{
		wchar_t buf[256]; 
   //   SelectObject(dc,hpen);
	
		int xc=rw.right/2;
		int yc=rw.bottom/2;
		float_type rs=min(xc,yc);
		float_type ix=xc+rs*z.real();
		float_type iy=yc+rs*z.imag();
		float_type ir=rs*rc;
		drawcycle(ix,iy,ir,fclear,finvert);
		return std::make_pair(std::complex<float_type>(ix,iy),ir);
	}


};

struct previewer_base
{
   DC_holder dc;
   HGDI_holder hpen;
   HGDI_holder hbrush,hbrush2;
   int hshift;
   bool ftest; 
   previewer_base(HWND hwin=HWND(-1),int _hshift=40,HPEN hp=CreatePen(PS_SOLID,2,RGB(0,255,0)))
	   :dc(hwin),hpen(hp),hshift(_hshift),ftest(1)
   {
	    hbrush.attach(CreateSolidBrush(RGB(255,255,0)));
		hbrush2.attach(CreateSolidBrush(RGB(255,255,255)));
   }

inline previewer_base& base_ref(){return *this;};

inline int   operator()(Capture_base* pcap,unsigned char* pbytes,POINT & point,RECT& rect,void* pUserData=0)
{
	HRESULT hr;
	if(dc) 
	{  
		int w=pcap->width();
		int h=pcap->height();
		int hshift=40;
		int nums=StretchDIBits(dc,0,hshift,w,h,0,0,w, h,pbytes,(*pcap),DIB_RGB_COLORS, SRCCOPY);
		int ys=hshift;

		RECT rb={0,0,w,h};
		if(EqualRect(&rect,&rb)) return 0;
		
		//
      RECT rrect={rect.left,rect.top+ys,rect.right,ys+rect.bottom};
		//RECT rrect={200,200,500,500};
		FrameRect(dc,&rrect,(HBRUSH)hbrush.hobj);
		//SleepEx(1000,1);
         //  hr=GetLastError();
	}
	return 0;
}

};




struct RGB24_getter
{

	typedef  float value_type;
	bool fset;

	struct line_type_c
	{
         unsigned char* pbits;
		 RGB24_getter& g;
			 line_type_c(RGB24_getter& gt,unsigned char* p)
				 :g(gt),pbits(p){};
		inline  float operator[](int n)
		{
			value_type v=0,f;
			//*

        if(g.fset)
	    {
			f=pbits[((n<<1)+n)+0]*g.ar;
			v+=f;
			pbits[((n<<1)+n)+0]=f;

			f=pbits[((n<<1)+n)+1]*g.ag;
			v+=f;
			pbits[((n<<1)+n)+1]=f;

			f=pbits[((n<<1)+n)+2]*g.ab;
			v+=f;
			pbits[((n<<1)+n)+2]=f;     
	    }
	    else
	    {
			 f=pbits[((n<<1)+n)+0];
			 v+=f*g.ar;

			 f=pbits[((n<<1)+n)+1];
			 v+=f*g.ag;

			 f=pbits[((n<<1)+n)+2];
			 v+=f*g.ab;
	    }

			return v;
		}
	};

	//typedef typename line_type_c<ncol,tpow> line_type_t;
	float ar,ag,ab;

	RGB24_getter(float _ar=1,float _ag=0,float _ab=0,bool _fset=false):ar(_ar),ag(_ag),ab(_ab),fset(_fset)
	{         
	}
	typedef  line_type_c line_type_t;

     int w;
	 int h;
	 unsigned char* pbits;
	 int linesize_b;


void     init(void*p, int wn,int hn,int lsb)
        {
			 w=wn;
			 h=hn;
			 linesize_b=lsb;
			 pbits=(unsigned char*)p;
        }
		   //:pbits((unsigned char*)p),w(wn),h(hn),linesize_b(lsb){};


	 inline int width(){return w;}
	 inline int height(){return h;}


	// RGB24_colget(void*p, int wn,int hn,int lsb):pbits((unsigned char*)p),w(wn),h(hn),linesize_b(lsb){};

line_type_t	 operator [](int n)
{
	return line_type_t(*this,pbits+n*linesize_b);
}

};




template <class Getter,typename  value_type=typename Getter::value_type >
struct LaserPointDetector
{
      Getter&  getter;
	  int fenabled;
	  
		  LaserPointDetector(Getter&  g):getter(g),fenabled(1){};



  inline void  operator()(Capture_base* pcap,unsigned char* pbytes,POINT & point,RECT& rect,void* pUserData=0)
	  {
		  CALIBRATE_DATA &cd= *((CALIBRATE_DATA*)pUserData);


		  RECT roror=cd.camera.rect;


		  if(!fenabled) return;
		  typedef typename line_type<Getter,typedeffer<Getter>::result >::type_t line_type_t;

		  int w=pcap->width();
		  int h=pcap->height();
          getter.init(pbytes,w,h,pcap->line_sizeb());

          value_type cmax=0,cmaxinner=0;

//     /*

		  CALIBRATE_DATA::camera_t::laser_t& lsr=cd.camera.laser;

		  RECT rcam=make_rect(0,0,w,h);
		  RECT ro = _inrect(lsr.raw_area.outer,rcam);
		  RECT ri =  _inrect(lsr.raw_area.inner,ro);


  

       double mean=0,meaninner=0,ccf=0.001;
	   POINT pi={0,0};
	   POINT po={0,0};

     
	   for(int m=ro.top;m<=ro.bottom;m++) {

		   line_type_t pline=getter[m];
		   
		   bool fy=(ri.top<=m)&&(m<=ri.bottom);

		   for(int n=ro.left;n<=ro.right;n++)	{
			   value_type c=pline[n];
			   ++ccf;

			   
			   bool fi=fy&&(ri.left<=n)&&(n<=ri.right);
			   if(!fi)
			   {
				      meaninner+=c;
					  if(cmaxinner<c)
					  {
						  cmaxinner=c;
                          pi.x=n;
						  pi.y=m;
					  }
			   }

                 mean+=c;

			   if(cmax<c)
			   {
                 cmax=c;
				 po.x=n;
				 po.y=m;
			   }


		   }

	   }

           lsr.image_data.inner.maxval=cmaxinner;
		   lsr.image_data.inner.meanval=meaninner;
		   lsr.image_data.inner.point=pi;

		   lsr.image_data.outer.maxval=cmax;
		   lsr.image_data.outer.meanval=mean/ccf;
		   lsr.image_data.outer.point=po;

		  //
		   point =po;

         ///  */
          

	   // 
		   /*
      cmax=0;
	  for(int m=0;m<h;m++) {

           line_type_t pline=getter[m];

			   	  for(int n=0;n<w;n++)	{

                    value_type c=pline[n];

						if(cmax<c){
						cmax=c;
						point.x=n;
						point.y=m;
						}

				  }
    	  } 
		 // */
  }
	  
};

template <class Getter,typename  value_type=typename Getter::value_type >
struct CamCalibratePrjMatrixDetector
{

	typedef array2D<double> accumulator_t;
	accumulator_t accumulator;

	Getter&  getter;
	int fenabled;


	CamCalibratePrjMatrixDetector(Getter&  g):getter(g),fenabled(0){};

inline      void reset()
	 {
        accumulator.clear();
	 }

	inline void  operator()(Capture_base* pcap,unsigned char* pbytes,POINT & point,RECT& rect,void* pUserData=0)
	{
          if(!fenabled) return;
		typedef typename line_type<Getter,typedeffer<Getter>::result >::type_t line_type_t;

		int w=pcap->width();
		int h=pcap->height();
		getter.init(pbytes,w,h,pcap->line_sizeb());
		accumulator.accumulate(getter,rect);
		
	}

};

struct fake_detector
{

	
	//int fenabled;


	fake_detector(){};

	
	inline void  operator()(Capture_base* pcap,unsigned char* pbytes,POINT & point,RECT& rect,void* pUserData=0)
	{
	
	};

};




template <class CDetect_handler,class CView_handler=previewer_base>
struct WC_laser_base:WebCamCapture<WC_laser_base<CDetect_handler,CView_handler> >
{
	
	typedef typename  CDetect_handler detect_handler_t;
	typedef typename  CView_handler view_handler_t;
	POINT point;
    RECT  rect,rectvm;
	CDetect_handler & detect_handler;
	CView_handler& view_handler;
	int ntic;
	struct  
	{
		int modulo;
	} previewer;

	inline RECT& set_margin_rect(RECT m)
	{
		//RECT m=pcd->camera.margin;
		//	RECT r=make_rect(m.left,m.top,wc-m.right,hc-m.bottom);
		//RECT r=make_rect(m.left,m.bottom,wc-m.right,hc-m.top);
		{
		    RECT r={m.left,m.top,width()-m.right,height()-m.bottom};
			rect=r;
		}
		{
	
		RECT r={m.left,m.bottom,width()-m.right,height()-m.top};
		rectvm=r;
		}
		 return rect;
    	
	};


   

	WC_laser_base(CDetect_handler & d,CView_handler& p,int nc=0,int dlg=0,bool ffcalback=1)
		:WebCamCapture<WC_laser_base<CDetect_handler,CView_handler> >(nc,dlg,ffcalback),
        detect_handler(d),view_handler(p)
	{
		RECT r={0,0,width(),height()};
		previewer.modulo=1;
		rectvm=r;
		rect=RECT();
		point=POINT();
		ntic=0;

		 std::pair<const wchar_t*,const wchar_t*> mapfn=get_cam_map();

		 decompressor.set_mapfilename(mapfn.first,mapfn.second);



	};
	~WC_laser_base(){;}

	inline 	void update_frame(unsigned char* pbytes )
	{
          
        detect_handler(this,pbytes,point,rectvm,pUserData);
		if((previewer.modulo>0)&&((ntic++)%previewer.modulo)==0)
		view_handler(this,pbytes,point,rect,pUserData);

	}

};

struct  WC_view_only:WC_laser_base<fake_detector> 
{
	WC_view_only(previewer_base& pv,int nc=0,int dlg=0)
		:WC_laser_base<fake_detector>(fake_detector(),pv,nc,dlg){}; 
};

struct  WC_filemap_only:WC_laser_base<fake_detector,fake_detector>
{
   WC_filemap_only(int nc=0,wchar_t* prfx=0)
	   :WC_laser_base<fake_detector,fake_detector>(fake_detector(),fake_detector(),nc,0,0)
   {
	   if(prfx)
	   {
        wcscat(decompressor.mapname,prfx);
	    wcscat(decompressor.filename,prfx);
	   }
		   

      decompressor.pre_open();	
	  capDriverDisconnect(hWndC);
   }

inline   wchar_t * get_mapname()
   {
         return decompressor.mapname;
   }
inline   wchar_t * get_filename()
{
	return decompressor.filename;
}

inline   int lock()
   {
     return decompressor.bmpmap.lock() ;
   }
std::pair<BITMAPFILEHEADER*,size_t>   get_image()
{
	return std::make_pair((BITMAPFILEHEADER*)decompressor.bmpmap.pBits(),decompressor.bmpmap.size);
}
inline   int unlock()
{
	return decompressor.bmpmap.unlock() ;
}
 inline BMPMap& get_bmpmap()
 {
   return decompressor.bmpmap;
 }
template <class T>
T* get_pBits()
{
	return get_bmpmap().pBitsT<T>();
}
};

template <class CView_handler=fake_detector>
struct  WC_filemap_ext:WC_laser_base<fake_detector,CView_handler>
{
	WC_filemap_ext(CView_handler& handler,wchar_t* mapname=0,wchar_t* filename=0)
		:WC_laser_base<fake_detector,CView_handler>(fake_detector(),handler,0,0x10000)
	{
		 pmutex=0;
         p_frame_bits=0;
		if(hWndC) capDriverDisconnect(hWndC); 
		open(mapname,filename);
		/*
        decompressor.mapname.clear().cat(mapname);
		decompressor.filename.clear().cat(filename);
		decompressor.pre_open();	
		*/
		
	}

std::pair<BITMAPFILEHEADER*,size_t> open(wchar_t* mapname=0,wchar_t* filename=0)
{
	decompressor.mapname.clear().cat(mapname);
	decompressor.filename.clear().cat(filename);
	return decompressor.pre_open();	
};

std::pair<BITMAPFILEHEADER*,size_t> open(BMPMap& bm)
{
	return open(bm.mapname,bm.filename);
};


	mutex_ref_t* pmutex;
	//BMPMap::frame_info_t* pframe;
	BYTE *p_frame_bits;

	inline   wchar_t * get_mapname()
	{
		return decompressor.bmpmap.mapname;
	}
	inline   wchar_t * get_filename()
	{
		return decompressor.bmpmap.filename;
	}

	inline   int lock()
	{
		//return decompressor.bmpmap.lock() ;
		int ind;
		BMPMap& bmm=get_bmpmap();
		if(bmm.nframes==0)
		{
           p_frame_bits=bmm.pBits();
           bmm.lock();   
		}
		else
		{
		if((ind=bmm.get_last_index())<0) throw -1;
		(pmutex=&bmm.vframe_mutexes[ind])->lock();
    	  p_frame_bits=bmm.vframe_info[ind]->pbits;
		}
	}
	std::pair<BITMAPFILEHEADER*,size_t>   get_image()
	{
		return std::make_pair((BITMAPFILEHEADER*)decompressor.bmpmap.pBits(),decompressor.bmpmap.size);
	}
	inline   int unlock()
	{
		BMPMap& bmm=get_bmpmap();
		if(bmm.nframes==0)
		{
			bmm.unlock();   
		}
		else
		{
	    	if(pmutex) 
			return make_detach(pmutex)->unlock();
		};
		//make_detach(p_frame_bits);
		//return decompressor.bmpmap.unlock() ;
	}
	inline BMPMap& get_bmpmap()
	{
		return decompressor.bmpmap;
	}

	inline int line_sizeb()
	{
		return decompressor.bmpmap.line_sizeb();
	}

inline BITMAPINFO* pInfo_cache(const BITMAPINFO* pbi=0)
{
    if(pbi)
	{
     int headsize=PALSIZE(*pbi)+sizeof(BITMAPINFOHEADER);
	 memcpy(&bip,pbi,headsize);

	}
	return &bip;
}


inline BITMAPINFO* pInfo()
{
	return decompressor.bmpmap.pInfo();
}
inline 	int width()	{
	BITMAPINFOHEADER& bhi=decompressor.bmpmap.pInfo()->bmiHeader;
	return bhi.biWidth;
}
inline 	int height()	{
	BITMAPINFOHEADER& bhi=decompressor.bmpmap.pInfo()->bmiHeader;
	return bhi.biHeight;
}

	template <class T>
	T*  get_pBits()
	{
		//return get_bmpmap().pBits<T>();
		return (T*) p_frame_bits;
	}

	inline 	void update_frame(unsigned char* pbytes )
	{

		//locker_t<BMPMap> lock(get_bmpmap());
		detect_handler(this,pbytes,point,rectvm,pUserData);
		view_handler(this,pbytes,point,rect,pUserData);

	}

	inline void locked_update()
	{
		locker_t<WC_filemap_ext> lock(this);
         update_frame(p_frame_bits);
	}

	//inline 	void update_frame(unsigned char* pbytes )
};



struct laser_handler_base
{
	HANDLE hevent,hevent_s;
	void* pcap;
	laser_handler_base(HANDLE h=0):hevent_s(0),pcap(0)
	{

		hevent=(h)?h:(hevent_s=CreateEvent(0,1,0,0));

	}

	~laser_handler_base() {
		if(hevent_s)
			CloseHandle(hevent_s);
	}

	inline    bool operator()(POINT pc,int tio,void* pdata=0)
	{
		return wait(tio);
	}
	bool wait(int tio)
	{
		return WaitForSingleObject(hevent,tio)==WAIT_OBJECT_0;
	}
};

struct laser_handler_kb:laser_handler_base
{
	DWORD vkCode,vkCodeP;
	int cc;
	laser_handler_kb(DWORD _vkCode=VK_ESCAPE,HANDLE h=0,DWORD _vkCodeP=0x50)
		:laser_handler_base(h),vkCode(_vkCode),vkCodeP(_vkCodeP),cc(0){}
	inline    bool operator()(POINT pc,int tio,void* pdata=0)
	{
		if(kbhit())
		{
			int code=_getch();

			if(vkCodeP==toupper(code))
			{
				if((pcap)&&(pdata))
				{
					
					wchar_t* ap=((CALIBRATE_DATA*)pdata)->appdir;
					wcharbuf buf;
					swprintf(buf,L"%s\\logs\\sh%03d.bmp",ap,cc++);
					((Capture_base*)pcap)->write_to_file(buf); 
				}
					
			}
          if(vkCode==code) return 1;

			  //
		  //if(0x73==code)				  WebCamInfo::source_dlg();
			  
		}
		return wait(tio);
	}
	  
};

template <class WEBCAM,class laser_handler_T>
void laser_detect(WEBCAM &wcld,laser_handler_T & laser_handler,float fps=5,int nframes=-1)
{
   laser_handler.pcap=&wcld;

	DWORD timeout=double(1001)/double(fps);
	int n=0;

	do 
	{
		n++;
		//_cprintf("{%d}",n);
		//Sleep(10);
		if((nframes>=0)&&(n>nframes)) break;
		wcld.update();


	} while (!laser_handler(wcld.point,timeout,wcld.pUserData));

};

struct laser_handler_screen_cursor:laser_handler_base
{
	projective_transform::projmatrix<double> pjm;

	RECT monrect;
	LONG32* pflags;
	laser_handler_screen_cursor(LONG32* _pflags,projective_transform::projmatrix<double> &pj,int nmon=0)
	{

		pflags=_pflags;
		monrect=ScreenCapture(nmon,0).monitor_rect();
		pjm=pj;
	}

	inline    bool operator()(POINT pc,int tio)
	{
		std::complex<double> co(pc.x,pc.y);
		std::complex<double> ci=pjm*co;




		POINT pi={ci.real(),ci.imag()};
		POINT pmm={monrect.left+ci.real(),monrect.top+ci.imag()};
		LONG32 flags=InterlockedExchangeAdd(( volatile LONG *)pflags,0);
		if(flags<0)  SetCursorPos(pmm.x,pmm.y);
		_cprintf("[%04d:%04d:]\r",pi.x,pi.y);
		return wait(tio);
	}
	
};
