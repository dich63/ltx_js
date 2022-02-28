#pragma once


#include <char_parsers.h>
#include <wchar_parsers.h>

#include <windows.h>
#include <windef.h>
#include <shellapi.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atldef.h>
#include <comdef.h>
#include <atlenc.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <conio.h>


#include <dshow.h>
#include "qedit_old.h" 
//#include <mtype.h>


#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"Strmiids.lib")




inline wchar_t* error_msg(DWORD err,bstr_t& bstr=bstr_t())
{
	wchar_t lpMsgBuf[1024]=L"";

	
	FormatMessageW( 
		//FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPWSTR) lpMsgBuf,
		1024,
		NULL 
		);


	bstr_t ret=lpMsgBuf;
	wsprintf(lpMsgBuf,L"0x%08x ;",err);
	
	return bstr_t(lpMsgBuf)+ret;
}

#define _cmerr(hr) ((hr)?(char*)char_mutator<CP_OEMCP>(error_msg(hr)):(char*)"0 ;S_OK")
//CP_THREAD_ACP CP_OEMCP CP_UTF8


#define SETS(prfx,prop) lv=args[#prfx "." #prop];\
	if(!!args){\
	lv=args[#prfx "." #prop];\
	lf=args[#prfx "." #prop ".flags"].def(1);\
	hr=prfx->Set(prfx##_##prop,lv,lf);\
	if(FAILED(hr)) args[#prfx "." #prop ".set.hr"]=_cmerr(hr);\
	};


#define GETS(prfx,prop)  hr=prfx->Get(prfx##_##prop,&lv,&lf);\
	if(FAILED(hr)) args[#prfx "." #prop ".get.hr"]=_cmerr(hr);\
	else {\
	args[#prfx "." #prop]=lv;\
	args[#prfx "." #prop ".flags"]=lf;  }


template <class argv_zrw_t>
inline void set_range_val(argv_zrw_t& args,char* name,long lmin ,long lmax,long lst,long ld,long lf)
{
	char buf[256];
	sprintf(buf,"[%d,%d,%d,%d,%d]",lmin ,lmax,lst,ld,lf);
	args[name]=buf;
}


#define GET_RANGES(prfx,prop,r) \
	hr=prfx->GetRange(prfx##_##prop,&lmin,&lmax,&lst,&ld,&lf);\
	if(FAILED(hr)) args[#prfx "." r "." #prop ".hr"]=_cmerr(hr);\
	else {set_range_val(args,#prfx "." r "." #prop,lmin,lmax,lst,ld,lf);}


#define ALL_GETS(prfx,prop) GETS(prfx,prop);\
	GET_RANGES(prfx,prop,"range")

#define GETSr(prfx,prop) GET_RANGES(prfx,prop,"range")




template <class argv_zrw_t>
void VideoProcAmp_sets(IAMVideoProcAmp* VideoProcAmp,argv_zrw_t& args)
{
	HRESULT hr;
	long lv,lmin ,lmax,lst,ld,lf;
	SETS(VideoProcAmp,Brightness);
	SETS(VideoProcAmp,Contrast);
	SETS(VideoProcAmp,Hue);
	SETS(VideoProcAmp,Saturation);
	SETS(VideoProcAmp,Sharpness);
	SETS(VideoProcAmp,Gamma);
	SETS(VideoProcAmp,ColorEnable);
	SETS(VideoProcAmp,WhiteBalance);
	SETS(VideoProcAmp,BacklightCompensation);
	SETS(VideoProcAmp,Gain);
};



template <class argv_zrw_t>
void VideoProcAmp_gets(IAMVideoProcAmp* VideoProcAmp,argv_zrw_t& args)
{
	HRESULT hr;
	long lv,lmin ,lmax,lst,ld,lf;
	GETS(VideoProcAmp,Brightness);
	GETS(VideoProcAmp,Contrast);
	GETS(VideoProcAmp,Hue);
	GETS(VideoProcAmp,Saturation);
	GETS(VideoProcAmp,Sharpness);
	GETS(VideoProcAmp,Gamma);
	GETS(VideoProcAmp,ColorEnable);
	GETS(VideoProcAmp,WhiteBalance);
	GETS(VideoProcAmp,BacklightCompensation);
	GETS(VideoProcAmp,Gain);


	GETSr(VideoProcAmp,Brightness);
	GETSr(VideoProcAmp,Contrast);
	GETSr(VideoProcAmp,Hue);
	GETSr(VideoProcAmp,Saturation);
	GETSr(VideoProcAmp,Sharpness);
	GETSr(VideoProcAmp,Gamma);
	GETSr(VideoProcAmp,ColorEnable);
	GETSr(VideoProcAmp,WhiteBalance);
	GETSr(VideoProcAmp,BacklightCompensation);
	GETSr(VideoProcAmp,Gain);
};

template <class argv_zrw_t>
void CameraControl_gets(IAMCameraControl* CameraControl,argv_zrw_t& args)
{
	HRESULT hr;
	long lv,lmin ,lmax,lst,ld,lf;


	GETS(CameraControl,Pan);
	GETS(CameraControl,Tilt);
	GETS(CameraControl,Roll);
	GETS(CameraControl,Zoom);
	GETS(CameraControl,Exposure);
	GETS(CameraControl,Focus);

	GETSr(CameraControl,Pan);
	GETSr(CameraControl,Tilt);
	GETSr(CameraControl,Roll);
	GETSr(CameraControl,Zoom);
	GETSr(CameraControl,Exposure);
	GETSr(CameraControl,Focus);

};

template <class argv_zrw_t>
void CameraControl_sets(IAMCameraControl* CameraControl,argv_zrw_t& args)
{
	HRESULT hr;
	long lv,lmin ,lmax,lst,ld,lf;


	SETS(CameraControl,Pan);
	SETS(CameraControl,Tilt);
	SETS(CameraControl,Roll);
	SETS(CameraControl,Zoom);
	SETS(CameraControl,Exposure);
	SETS(CameraControl,Focus);
};



struct CamControl
{

	class COMInit
	{
	public:
		HRESULT m_hr;
		COMInit(DWORD dw= COINIT_MULTITHREADED /*COINIT_APARTMENTTHREADED*/ )
		{
			m_hr=CoInitializeEx(NULL,dw); 
		}
		~COMInit()
		{
			//	   Beep(500,500);
			if(SUCCEEDED(m_hr)) CoUninitialize();
		}
		inline operator HRESULT() { return m_hr;}
	};
	int num;
	HRESULT hr,hrCC,hrVPR;
	COMInit _com;
	variant_t cam_name;
	CComPtr<IBaseFilter> Filter;
	CComPtr<IAMVideoProcAmp> VideoProcAmp;
	CComPtr<IAMCameraControl> CameraControl;
	CComPtr<IAMVideoControl> VideoControl;


	CamControl():num(-1),hr(E_FAIL){};
	CamControl(int _num)
	{
		init( _num);
	}
	CamControl& init(int _num=0)
	{
		VideoControl.Release();
		CameraControl.Release();
		VideoProcAmp.Release();
		Filter.Release();

		num=_num;
		hr=E_FAIL;
		CComPtr<ICreateDevEnum> DevEnum ;
		CComPtr<IEnumMoniker> ClassEnum ;

		CComPtr<IMoniker> Moniker ;
		CComPtr<IPropertyBag> PropBag;

		hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
			__uuidof(ICreateDevEnum), (void **)&DevEnum);
		if(FAILED(hr)) return *this;

		hr = DevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &ClassEnum, 0);
		if(FAILED(hr)) return *this;

		for (int i=0; i <=num; i++)
		{
			ULONG cFetched;
			CComPtr<IMoniker> M ;
			hr=ClassEnum->Next(1,&M, &cFetched);
			if ( hr != S_OK)
			{
				break;
			}
			Moniker=M;
		}
		if(FAILED(hr)) return *this;
		hr = Moniker->BindToStorage(0, 0, IID_IPropertyBag, reinterpret_cast<void**>(&PropBag));
		if(FAILED(hr)) return *this;
		hr = PropBag->Read(L"FriendlyName",cam_name.GetAddress(), 0);
		if(FAILED(hr)) return *this;

		//CameraControlProperty dd;
		//VideoProcAmpProperty 
		hr = Moniker->BindToObject(0, 0,__uuidof(IBaseFilter), (void**)&Filter);
		if(FAILED(hr)) return *this;

		hrCC=Filter.QueryInterface(&CameraControl);
		hrVPR=Filter.QueryInterface(&VideoProcAmp);
		return *this;
	}

	template <class argv_zrw_t>
	HRESULT update_argv(argv_zrw_t& args)
	{

		int nc=args["CameraSetup.num"].def(0);
		int mode=args["CameraSetup.mode"].def(1);
		init(nc);
		if(mode&1)
		{
			
			args["CameraSetup.num"]=nc;
			args["CameraSetup.mode"]=mode;
			args["CameraSetup.hr"]= _cmerr(hr);
			bstr_t bn=cam_name;
		if(bn.length())
			args["CameraSetup.name"]=(char*)bn;
		else args["CameraSetup.name"]="??";
		

		}



		if(FAILED(hr)) 	  return hr;


		if(mode&1) args["CameraControl.hr"]= _cmerr(hrCC);

		if(SUCCEEDED(hrCC))
		{
			if(mode&2)
			{

				CameraControl_sets(CameraControl,args);
			}
			if(mode&1)
			{

				CameraControl_gets(CameraControl,args);
			}
			

		}
		if(mode&1) args["VideoProcAmp.hr"]= _cmerr(hrVPR);

		if(SUCCEEDED(hrVPR))
		{
			if(mode&2)
			{
				VideoProcAmp_sets(VideoProcAmp,args);

			}

			if(mode&1)
			{
				VideoProcAmp_gets(VideoProcAmp,args);

			}
			
		}
	}




};


template <class argv_zrw_t>
inline std::stringstream& CamControl_reset(std::stringstream& stream,argv_zrw_t& argv=argv_zrw_t(),char* delim=0)
{
	CamControl cc;
	cc.update_argv(argv);
	return argv.flat(stream,true,delim);
}




template <class argv_zrw_t>
inline std::string CamControl_reset(argv_zrw_t& argv=argv_zrw_t(),char* delim=0)
{

	//CamControl cc;
	//cc.update_argv(argv);
    return CamControl_reset(std::stringstream(),argv,delim).str();
//	const char* ps=argv.flat_str(true,delim);
//	return ps;
}





template <class CH>
inline std::string CamControl_reset(int argc, CH** ppargv,bool fprepare_only=false,char* delim=0)
{
	argv_zzs<char> args;
	{
		argv_def<CH> a(argc,ppargv);
		argv_file<CH> af(a[char_mutator<>("@")].def<CH*>());
		a.parent=&af;
		args=a;
	}

	return (fprepare_only)? args.flat_str(true,delim):CamControl_reset(args,delim);

}

template <class CH>
inline std::string CamControl_reset(std::vector<CH*>& v,bool fprepare_only=false,char* delim=0)
{
	int c=v.size();
	return CamControl_reset(c,(c)?&v[0]:0,fprepare_only,delim);
}

template <class CH>
inline std::string CamControl_reset(CH* pargvzz,bool fprepare_only=false,char* delim=0)
{
	return CamControl_reset(args_ZZ(pargvzz,0,buf),fprepare_only,delim);
}


inline std::string  CamControl_test(int num)
{

	argv_zzs<char> argv;
	argv["CameraSetup.num"]=num;
	return CamControl_reset(argv,"\n");
}
