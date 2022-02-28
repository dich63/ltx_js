#pragma once
#include <windows.h>

struct camera_types
{
	struct camera_t{

		ULONG32 factive;
		ULONG32 state;
		RECT rect,margin;
		struct laser_t{

			double power_lo;
			double power_hi;

			struct detect_t {

				POINT point; 
				double maxval;
				double meanval;
			};
			struct  
			{
				RECT inner,outer;
			} margin;

			struct  
			{
				RECT inner,outer;
			} area;
			struct  
			{
				RECT inner,outer;
			} raw_area;
			struct image_data_t
			{
				detect_t inner;
				detect_t outer;

			} image_data;

		}laser;

	};
};

struct CALIBRATE_DATA
{
	int nmon;
	int ncam;
	int npoint;
	double fps;
	double rcycle;
	double rdisk;
	double threshold_low;
	double threshold_high;
	double scan_square_size;
	double median_epsilon;
	int naccumulate;
	double pause_time;
	double time_out;
	RECT screen_rect;
	POINT raw_pt,raw_pt_i;
	//RECT camera_rect;
    
	typedef camera_types::camera_t camera_t;
	camera_t camera;
	void* pwebcam;


/*	
	struct camera_t{
		RECT rect,margin;
		struct laser_t{

			struct detect_t {
				
                POINT point; 
				double maxval;
				double meanval;
			};
			struct  
			{
				RECT inner,outer;
			} margin;

			struct  
			{
				RECT inner,outer;
			} area;
			struct  
			{
				RECT inner,outer;
			} raw_area;
			struct image_data_t
			{
             detect_t inner;
			 detect_t outer;

			} image_data;

		}laser;

	} camera;
*/
	wchar_t *appdir;



	template<class WEBCAM>
	inline void laser_regions_setup(WEBCAM& wc)
	{
		RECT wr={0,0,wc.width(),wc.height()};

		camera.rect=wr;
		camera_t::laser_t& lr=camera.laser;


		lr.area.outer=from_margin(lr.margin.outer,wr);
		lr.area.inner=from_margin(lr.margin.inner,lr.area.outer);  	   

		lr.raw_area.inner=flipv(lr.area.inner,wr);
		lr.raw_area.outer=flipv(lr.area.outer,wr);

	}


};

#define LASER_HOOK_DIR L"laser_hook"
inline  wchar_t* get_libprrx()
{
	return L"{40A48AB3-9D4F-4d4f-B28A-8ADBB1E2BA7D}";
}

#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <conio.h>
//#include "video/webcam_capture.h"
//#include "video/orderstat.h"
#include "wchar_parsers.h"
#include <utility>



inline std::pair<const wchar_t*,const wchar_t*> get_cam_map()
{
  static struct _st
  {
	wcharbuf buf;
    _st()
	{



	GetEnvironmentVariableW(L"APPDATA",buf,buf.size);
	wcscat(buf,L"\\");
	wcscat(buf,LASER_HOOK_DIR); 
	int res=SHCreateDirectoryExW(NULL,buf,0);
	if(0==GetEnvironmentVariableW(L"hssh.namespaces[laser.hook]",0,0))
     	SetEnvironmentVariableW(L"hssh.namespaces[laser.hook]",buf);
	SetEnvironmentVariableW(L"laser.hook.hssh_port",L"laser.hook::remote_desktop.port");
	wcscat(buf,L"\\webcam.bmp");
	};
   } st;


	 return std::make_pair(L"{40A48AB3-9D4F-4d4f-B28A-8ADBB1E2BA7D}_webcam",st.buf.get());
};


