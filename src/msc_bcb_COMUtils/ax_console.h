#pragma once

#include "free_com_factory.h"
#include "conio.h"
#include "winconsole.h"
#include "singleton_utils.h"
#pragma comment(lib, "winmm.lib")

template <class com_variant,class com_string,class com_exception>
struct QI_moniker_t
{
	HRESULT hrs;
	GUID iid;
	typedef void (__stdcall * raise_error_proc)(wchar_t* smessage,wchar_t* ssource);
	QI_moniker_t(wchar_t* pstr)
	{
		 hrs=(pstr)?CLSIDFromString(pstr,&iid):E_POINTER;
	}
	static com_variant invoke(QI_moniker_t * _this ,raise_error_proc error,  int argc, com_variant* argv,int,void*,HRESULT& hr,unsigned flags)
	{
        HRESULT hhr;  
		com_variant unk;
		VARIANT b;
		if(argc<1) return false;
		if(FAILED(hhr=VariantChangeType(&unk,argv,0,VT_UNKNOWN))) return false;
		IUnknown* punk=0;
         if(FAILED(hhr=unk.punkVal->QueryInterface(_this->iid,(void**)&punk))) return false;
		 punk->Release();
		 return true;
	}
	static void on_exit(QI_moniker_t* p)
	{
		delete p;
	}
};


template <class com_variant,class com_string,class com_exception>
struct console_io_utils
{
  typedef void (__stdcall * raise_error_proc)(wchar_t* smessage,wchar_t* ssource);
   typedef   initializator_singleton::locker locker;

    

    

 inline static HRESULT win_transparency(LONG lhwin,LONG lckey,LONG lalpha)
   {
	   HWND hwin=HWND (lhwin);
	   long ll=GetWindowLong(hwin, GWL_EXSTYLE);
	   if(lalpha==0)
	   {
            if(ll) 
				ll=SetWindowLong(hwin,GWL_EXSTYLE,ll&(~WS_EX_LAYERED));
				return (ll)? S_OK:HRESULT_FROM_WIN32(GetLastError());
	   }
	   bool f1=SetWindowLong(hwin,GWL_EXSTYLE,ll|WS_EX_LAYERED);
	   if(f1)
		    f1=SetLayeredWindowAttributes(hwin, lckey, (255 * (100-lalpha)) / 100,LWA_COLORKEY| LWA_ALPHA);
	   return (f1)? S_OK:HRESULT_FROM_WIN32(GetLastError());
   };

   static com_variant invoke(void*,raise_error_proc error,  int argc, com_variant* argv,int,void*,HRESULT& hr,unsigned flags)
   {
	   wchar_t buf[4096];
	 if(argc<1)  error(L"wrong arguments number!!",0);
     try
     {
		 int ncmd=argv[0];
		 locker lock;
//case 0x100:; AllocConsole();break;		       
//case 0x200: ;FreeConsole();break;
		 DWORD attr=M_CC;
		 if((ncmd==0x501))
		 {
           com_variant v;
		   
		      if(argc>1)
			  {
				  VariantChangeType(&v,argv+1,0,VT_BSTR);
			      if(SysStringLen(v.bstrVal))	
				  {
					  PlaySoundW(v.bstrVal,NULL,SND_FILENAME|SND_ASYNC|SND_LOOP);
					   return com_variant();
				  }
			  }
			   PlaySoundW(NULL,NULL,SND_PURGE);
			 
			 return com_variant();
		 }
		 if(argc>2)
		 {
			 VARIANT v={0};
			 if(SUCCEEDED(VariantChangeType(&v,argv+2,0,VT_I4)))
				 attr=v.intVal;
		 }
		 if((ncmd==0x500)&&((argc>1)))
		 {
              int pid=argv[1];
			  HANDLE hp=OpenProcess(PROCESS_TERMINATE,0,pid);
			  if(hp)
			  {
				   TerminateProcess(hp,-1);
				   CloseHandle(hp);
			  }
		 }
		 if(ncmd==0x400)
		 {
			   GUID g=(argc>1)? uuid_file(bstr_t(argv[1]),1):uuid_generate();
               return com_variant((wchar_t*)(wstr_uuid(g)));

		 }
		 if(ncmd&0x100) AllocConsole();
         if(ncmd&0x200) FreeConsole();
		 ncmd&=0xFF;
		 switch(ncmd)
		 {
		   case 8:
			 
			 *buf=0;
             fgetws(buf,4096,stdin);
             return buf;    
			 break;
           
		   case 1:;case 2:;case 3:case 4:
			   {
			   if(argc<2)  error(L" fputs: wrong arguments number!!",0);
			   com_string s=argv[1];
			   int n=s.length();
			   

			   
			   switch(ncmd)
			   {
			     case 1: fputws(s,stdout);break;
     			 case 2: fputws(s,stderr);break;
				 //case 3: M_CC_cputws(s);break;
                //case 3: conputs_h(attr,s);break;
    			 case 3: console_shared_data_t<wchar_t,0>().puts((wchar_t*)s,attr);break;
                case 4: OutputDebugStringW(s);break;
                
			   };
			   }
		    break;

		   case 0x80: 
			   hr=win_transparency(argv[1],argv[2],argv[3]);
			   ;break;
		   case 0x20: 
			    return   bstr_t(ClipboardText());
			   ;break;
		   case 0x21: 
			    ClipboardText()=bstr_t(argv[1]);
			   ;break;


		   case 0x40: 
			   {
				   int ihwnd=argv[1];
				   int isw=-1;
					   if(argc>2) isw=argv[2];
				   if(ihwnd==-1)
				   {
					   ihwnd=(int) GetConsoleWindow();
					   if(!ihwnd) return 0;
				   }
				   if(ihwnd>=0) ShowWindow(HWND(ihwnd),isw);
				   return ihwnd;
				   
			   }
			   //hr=win_transparency(argv[1],argv[2],argv[3]);
			   ;break;


            case 0x010: 
				coord c=consoleXY();
				if(argc>1) consoleXY()=coord(int(argv[1]));
                 return com_variant(int(DWORD(c)));    
				;break;



/*
					case 0x80:
				     return   bstr_t(ClipboardText());
					break;

			case 0x801:
				ClipboardText()=bstr_t(argv[1]);
				;break;
*/
			



		 }

           return com_variant();    

     }
     catch (com_exception& e)
     {
		 error(com_string(e.ErrorMessage()),0);
     }
          
   }


};
