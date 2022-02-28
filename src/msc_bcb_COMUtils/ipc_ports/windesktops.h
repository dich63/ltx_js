#pragma once
//windesktops.h
#include <windows.h>
#include "wchar_parsers.h"
#include <list>
#include <string>
#include <vector>

template <int VERS=0>
struct win_desktops_t{

	
	

	

	static BOOL CALLBACK s_EnumDesktopProc(
		wchar_t* lpszDesktop,
		 LPARAM lParam
		){


			return ((win_desktops_t*)lParam)->enum_desktop_proc(lpszDesktop);
	};

	bool enum_desktop_proc(wchar_t* name){

		desks.push_back(std::wstring(name));

		return true;
	}
	int update(){
		desks.resize(0);
		HWINSTA ws=GetProcessWindowStation();
		if(EnumDesktops(ws,&s_EnumDesktopProc,LPARAM(this)))
			return desks.size();
		else
			return 0;

	}
	size_t count(){
		return desks.size();
	}

	wchar_t* operator[](int i){
		int c=count();
		i=c-i;
		if((0<=i)&&(i<c))
			return (wchar_t*)desks[i].c_str();
		return NULL;
	}

	win_desktops_t(){


		desks.reserve(24);
		update();
	}

	std::vector<std::wstring> desks;

};

template<int VERS=0>
struct console_input_t{
   HANDLE hin;
   bool fnc;
 


   console_input_t(size_t szbuf=1024)
	   :maxsize(szbuf),cbout(0){

		   fnc=!GetConsoleWindow();
		   if(fnc)
		   AllocConsole();
		   //hin=CreateFileW(L"CONIN$",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);	   
		   hin = CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE,
			   FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		   vbuf.resize(maxsize);
		   pbuf=&vbuf[0];
		   //hin=GetStdHandle(STD_INPUT_HANDLE);
   }

   ~console_input_t(){
	   if(fnc)
		   FreeConsole();
	 hin&&CloseHandle(hin);
   }

   char* read_console(char* p){	     
		 if(ReadConsoleA(hin,p,maxsize,&(cbout=0),NULL)){
			 p[cbout]=0;
			 return p;
		 }
		 return NULL;
   }

   wchar_t* read_console(wchar_t* p){	   
	   
	   if(ReadConsoleW(hin,p,maxsize,&(cbout=0),NULL)){
		   p[cbout]=0;
		   return p;
	   }
	   return NULL;
   }

   template <class CH>
   CH* getline(bool fnocr=false){
	   DWORD dwm;
	   BOOL f;
	   CH* res=NULL;

	   if(GetConsoleMode(hin,&dwm))
	   {
		   SetConsoleMode(hin,dwm|ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT);		   
		   res=read_console((CH*)pbuf);
		   if(fnocr&&(cbout>1))
			   res[(cbout-=2)]=0;
		   SetConsoleMode(hin,dwm);
	   }
	   return res;
   }

   size_t send_console_input(const wchar_t* pwc)
   {

	   DWORD cb = safe_len(pwc);
	   if (!cb) return 0;

	   //HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
	   if (INVALID_HANDLE_VALUE == hin) return 0;
	   INPUT_RECORD* pinp = (INPUT_RECORD*)::calloc(2 * cb, sizeof(INPUT_RECORD));

	   for (int n = 0; n < cb; n++)
	   {
		   INPUT_RECORD& inr_d = pinp[2 * n];
		   inr_d.EventType = KEY_EVENT;
		   KEY_EVENT_RECORD& ker = inr_d.Event.KeyEvent;
		   ker.bKeyDown = true;
		   ker.wRepeatCount = 1;
		   ker.uChar.UnicodeChar = pwc[n];
		   (pinp[2 * n + 1] = inr_d).Event.KeyEvent.bKeyDown = false;

	   }

	   bool f;

	   if (!WriteConsoleInput(hin, pinp, 2 * cb, &cb))
		   cb = 0;
	   ::free(pinp);
	   return cb;
   };

   DWORD cbout;
   wchar_t* pbuf;
   size_t maxsize;
   std::vector<wchar_t> vbuf;
};

template<int VERS=0>
struct console_output_t{
	HANDLE hout;
	bool fnc;

	static  int vfmt_count(const char* fmt,va_list argptr)
	{
		//
		int cb=_vscprintf(fmt,argptr);
		//int cb=vfprintf(dflt_v<char>::nulFile(),fmt,argptr);
		return cb;
	}



	static  int vfmt_count(const wchar_t* fmt,va_list argptr)
	{
		
		int cb=_vscwprintf(fmt,argptr);
		
		return cb;
	}



	console_output_t(){
			fnc=!GetConsoleWindow();
			if(fnc)
				AllocConsole();
			hout=CreateFileW(L"CONOUT$",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);	
			vbuf.reserve(512);
	}
		

	~console_output_t(){
		if(fnc)
			FreeConsole();
		hout&&CloseHandle(hout);
	}

	int write_console(char* p,size_t cb){	     
		return WriteConsoleA(hout,p,cb,&(cbout=0),NULL)?cbout:0;
	}

	int write_console(wchar_t* p,size_t cb){	     
		return WriteConsoleW(hout,p,cb,&(cbout=0),NULL)?cbout:0;
	}

	DWORD attr(DWORD attv){
	  CONSOLE_SCREEN_BUFFER_INFO csbi;
	  GetConsoleScreenBufferInfo(hout,&csbi);
	  SetConsoleTextAttribute(hout,0x0000FFFF&attv);
	  return csbi.wAttributes ;
	}

	template<class CH>
	inline CH* get_buf(size_t cb){
		vbuf.resize(sizeof(CH)*cb+8);
		return (CH*)(&vbuf[0]);
	}

	
	int printf(const wchar_t* fmt,...)
	{
		va_list argptr;
		va_start(argptr, fmt);
		size_t cb=vfmt_count(fmt,argptr);
		wchar_t* buf=get_buf<wchar_t>(cb);
		cb=vswprintf_s(buf,sizeof(wchar_t)*cb, fmt, argptr);
		//cb=vswprintf(buf, fmt, argptr);
		cb=write_console(buf,cb);
		return cb;
	}




	DWORD cbout;
	
	std::vector<char> vbuf;
};



template <int VERS=0>
struct desktop_switch_t{

	static HDESK getCurrentDesktop(){
		return GetThreadDesktop(GetCurrentThreadId()); 
	}

	struct desktop_keep_t{
		HDESK hdesk_old,hdesk;

		
		desktop_keep_t(HDESK hd=getCurrentDesktop()):hdesk_old(hd),hdesk(0){
			wchar_t t;
			DWORD cb=2048;
		//	exename=(wchar_t*)::calloc(1,2*cb);
		//	cb=GetModuleFileNameW(NULL,exename,cb);

		};

		HRESULT swith_to(wchar_t* name=NULL){

		/*	DWORD DA_ALL = (DESKTOP_READOBJECTS | DESKTOP_CREATEWINDOW | DESKTOP_CREATEMENU |
				DESKTOP_HOOKCONTROL | DESKTOP_JOURNALRECORD | DESKTOP_JOURNALPLAYBACK |
				DESKTOP_ENUMERATE | DESKTOP_WRITEOBJECTS | DESKTOP_SWITCHDESKTOP);
				*/
			hdesk&&CloseDesktop(hdesk);
			hdesk=(name)?OpenDesktopW(name,0,0,GENERIC_ALL):NULL;
			if(hdesk)
				return 	SwitchDesktop(hdesk)?S_OK:HRESULT_FROM_WIN32(GetLastError());
			else return S_OK;

		}

		~desktop_keep_t(){		
			
			SwitchDesktop(hdesk_old);
			hdesk&&CloseDesktop(hdesk);
		}


	};


	bool fork(wchar_t* pn,PROCESS_INFORMATION* pi){		  
            
          wchar_t exename[1024],*cmd;    
          STARTUPINFOW sif=si;	
		  sif.lpDesktop=pn;
		  sif.lpTitle=pn;
		  if(!::GetModuleFileNameW(NULL,exename,1024))
			  return false;
		  cmd=::GetCommandLineW();
		  BOOL f;
		  DWORD CF=CREATE_UNICODE_ENVIRONMENT|CREATE_NEW_CONSOLE;
		  f=::CreateProcessW(exename,cmd,NULL,NULL,false,CF,NULL,NULL,&sif,pi);
		  if(f) CloseHandle(pi->hThread);
		  return f;
	}


	bool is_active(wchar_t* pn){
		if(!(pn&&name))
			return true;
		return (wcscmp(pn,name)==0);			
	}


	HRESULT switch_to(wchar_t* pn){

		HRESULT hr;
		if(!(pn&&name))
			return E_POINTER;

		if(is_active(pn))
			return S_OK;

		PROCESS_INFORMATION pi;

		
		if(fork(pn,&pi)){

			desktop_keep_t dk;
			dk.swith_to(pn);
			WaitForSingleObject(pi.hProcess,INFINITE);
			CloseHandle(pi.hProcess);
			return S_OK;
		}
		
		return S_OK; 


	}




	static	wchar_t* parse_slash(wchar_t* pn){
		wchar_t* p=pn;
		if(p)
          while(*p)		  
			  if(*p++==L'\\')
				  return p;	    
		return pn;
	}

	desktop_switch_t(){		

		si.cb=sizeof(STARTUPINFOW);
		GetStartupInfo(&si);
		name=parse_slash(si.lpDesktop);
		hdesk=getCurrentDesktop();
	}

	STARTUPINFOW si;
	wchar_t* name;
	win_desktops_t<VERS> desktops;
	HDESK hdesk;

};