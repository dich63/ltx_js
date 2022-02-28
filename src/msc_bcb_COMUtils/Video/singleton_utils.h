#pragma once

#include <Winsock2.h>
#include <Mswsock.h>

#include "windows.h"
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <comutil.h>
#include <conio.h>
#include <process.h>
#include "handle_array.h"

#pragma pack( push ,8)
#include <Tlhelp32.h>
#pragma pack( pop)

#include "winconsole.h"
#include "static_constructors_0.h" 
//#include "video/webcam_capture.h"
//#include "video/orderstat.h"
#include "wchar_parsers.h"
#include <stdio.h>


#define OLE_CHECK_PTR(p) if(!(p)) return E_POINTER;
#define OLE_CHECK_VOID(a) if(FAILED((a))) return;
#define OLE_CHECK_hr(a) {if(FAILED( hr=(a) ) ) return hr;}
#define OLE_CHECK_if_hr(f,h) if(!!(f)) return HRESULT_FROM_WIN32(h);
#define OLE_CHECK_if_lasterror(f) OLE_CHECK_if_hr(f,GetLastError());
#define OLE_CHECK(a) ;{ HRESULT hr__0000;if(FAILED( hr__0000=(a) ) ) return hr__0000;};


#define SET_PROC_ADDRESS_prfx(h,a,prfx) __set_dll_proc(h,(void**)&prfx##a,#a)
#define SET_PROC_ADDRESS(h,a) __set_dll_proc(h,(void**)&a,#a)
#define SET_PROC_ADDRESS_NS(h,ns,a) __set_dll_proc(h,(void**)&ns##::##a,#a)

///*
	inline void* __set_dll_proc(HMODULE h,void** fp,char* name)
	{
		return (h)?(*fp=(void*)GetProcAddress(h,name)):NULL;
	}
//*/


inline  int SetForegroundProcess(DWORD pid=GetCurrentProcessId(),DWORD tio=1000);

template<class N>
inline void* to_ptr(N n)
{
	union {
		void* p;
		N n;

	} uu;
	uu.n=n;
	return uu.p;
}

inline void* __siu_set_dll_proc(HMODULE h,void** fp,char* name)
{
	return (h)?(*fp=(void*)GetProcAddress(h,name)):NULL;
}

#define SET_PROC_ADDRESS(h,a) __siu_set_dll_proc(h,(void**)&a,#a)


inline v_buf<WCHAR>& expand_env(const WCHAR* str,v_buf<WCHAR>& buf=v_buf<WCHAR>())
{
	if(safe_len(str)==0) buf.clear();
	else
	{
		int cb=ExpandEnvironmentStringsW(str,0,0);
		ExpandEnvironmentStringsW(str,buf.resize(cb),cb);
	}
	return buf;
};

inline v_buf<WCHAR>& full_path_name(const WCHAR* str,v_buf<WCHAR>& buf=v_buf<WCHAR>())
{
	int cb=safe_len(str);
	WCHAR* p;
	if(cb==0) buf.clear();
	else
  {  
		buf.resize(cb+=2048);
		cb=GetFullPathNameW(str,cb,buf,&p);
		if(cb) buf.resize(cb+1);
		else buf.clear();
	}
	return buf;
};

struct console_GUI_t
{

	
	BOOL (WINAPI	*GetConsoleScreenBufferInfoEx)(HANDLE,PCONSOLE_SCREEN_BUFFER_INFOEX);
	BOOL (WINAPI	*SetConsoleScreenBufferInfoEx)(HANDLE,PCONSOLE_SCREEN_BUFFER_INFOEX);
	HANDLE hout;
	//HWND hwin;
	console_GUI_t()
	{
		HMODULE hk=GetModuleHandleA("kernel32.dll");
		SET_PROC_ADDRESS(hk,GetConsoleScreenBufferInfoEx);
		SET_PROC_ADDRESS(hk,SetConsoleScreenBufferInfoEx);
		hout=GetStdHandle(STD_OUTPUT_HANDLE);
		//hwin=GetConsoleWindow();		
	}

operator CONSOLE_SCREEN_BUFFER_INFOEX()
{
	CONSOLE_SCREEN_BUFFER_INFOEX csbi={sizeof(CONSOLE_SCREEN_BUFFER_INFOEX)}; 
	if(console_GUI_t::GetConsoleScreenBufferInfoEx)
		console_GUI_t::GetConsoleScreenBufferInfoEx(hout,&csbi);
	else ::GetConsoleScreenBufferInfo(hout,(PCONSOLE_SCREEN_BUFFER_INFO)&csbi.dwSize);

	return csbi;

};


console_GUI_t& operator=(CONSOLE_SCREEN_BUFFER_INFOEX csbi)
{
	if(console_GUI_t::SetConsoleScreenBufferInfoEx)
		console_GUI_t::SetConsoleScreenBufferInfoEx(hout,&csbi);
	//else ::SetConsoleScreenBufferInfo(hout,(PCONSOLE_SCREEN_BUFFER_INFO)&csbi.dwSize);

	return *this;
}

};

struct console_info_t
{
	CONSOLE_SCREEN_BUFFER_INFOEX csbi;

	struct coord:COORD
	{
      coord(int x,int y=0)
	  {
		  X=x;Y=y;
	  };

	};

	struct rect:SMALL_RECT
	{
		rect(int l,int t,int r,int b)
		{
			Left=l;Top=t;Right=r;Bottom=b;
		};
		rect(int w,int h)
		{
			Left=0;Top=0;Right=w;Bottom=h;
		};

	};

	console_GUI_t cg;
	bool fd;
	console_info_t(bool fdirect=false):fd(fdirect)
	{

      csbi=cg;
	}
	~console_info_t()
	{
		csbi.srWindow.Right++;
		csbi.srWindow.Bottom++;
		cg=csbi;
	}

inline console_info_t& update()
{
         csbi=cg=csbi;
		 return *this;
}
inline CONSOLE_SCREEN_BUFFER_INFOEX& get()
{
	if(fd)  update();
	
	return csbi;
}

 inline CONSOLE_SCREEN_BUFFER_INFOEX* operator ->()
 {
	      return &get();
 }

inline  operator CONSOLE_SCREEN_BUFFER_INFOEX* ()
 {
	 return &get();
 }

inline  operator CONSOLE_SCREEN_BUFFER_INFOEX()
{
	return get();
}

inline  static void simple_set_alpha(double alpha){

	BYTE balpha= double(255*alpha)/100.0;

	//COLORREF c=0;

	HWND hWnd = GetConsoleWindow();
	SetWindowLong(hWnd, GWL_EXSTYLE,GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd,0,balpha,LWA_ALPHA);


}

inline  console_info_t& set_alpha(double alpha,int indx=-1)
{
  LONG lRet;
   DWORD flags=0;
   if((alpha>=0)&&(alpha<=100)) flags|=LWA_ALPHA;
   else alpha=100;
   if(indx==33) indx=csbi.wAttributes&0x0F;
   if(indx==32) indx=(csbi.wAttributes&0xF0)>>4;

   if((0<=indx)&&(indx<16))
   {
	 flags|=LWA_COLORKEY;

   }
  
   else indx=0;
   if(flags)
   {
	   BYTE balpha= double(255*alpha)/100.0;
	   
	   HWND hWnd = GetConsoleWindow();
  	   lRet = SetWindowLong(hWnd, GWL_EXSTYLE,GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	   lRet = SetLayeredWindowAttributes(hWnd,csbi.ColorTable[indx] ,balpha,flags);
   }
  
   return *this;

}


};

inline int startup_console_attr(wchar_t* consGUI=L"console.GUI",wchar_t* consini=L"services.console.",int fconsdef=0)
{
   HRESULT hr;
   int fcons,fhide;
	argv_env_r<wchar_t> env;
	argv_ini<wchar_t> ini(consGUI,L".ini");

	env.set_prefix_name(consini);
    fcons=env[L"mode"].def(ini[L"mode"].def( fconsdef));
	fhide=env[L"hide"].def(ini[L"hide"].def(0));

	BOOL faa;
	if(faa=env[L"asfw"].def(ini[L"asfw"].def(1)))		
	{
		faa=AllowSetForegroundWindow(GetCurrentProcessId());
	   //faa=SetForegroundProcess();
	}
	//WaitForInputIdle(GetCurrentProcess(),5000);
	//Sleep(1000);

	if(fhide)
	{
         AttachConsole(ATTACH_PARENT_PROCESS);
		 HWND hwin=GetConsoleWindow();
		 if(hwin) ShowWindow(hwin,SW_HIDE);
			 return 0;
	}



	env.parent=&ini;


	BOOL frc;
	if(fcons)
	{
		if(fcons<0)
		{

			frc=AttachConsole(ATTACH_PARENT_PROCESS);
		}
		frc=AllocConsole();
		hr=GetLastError();


		_cputs("");
		//system("cmd.exe /C set hssh");


		console_info_t ci;

		ci->wAttributes=env[L"attr"].def<int>(ci->wAttributes);

		int ifg=int(ci->wAttributes);

		ci->ColorTable[ifg&0xF]=env[L"fc"].def<int>(ci->ColorTable[ifg&0xF]);
		ci->ColorTable[(ifg&0xF0)>>4]=env[L"bc"].def<int>(ci->ColorTable[(ifg&0xF0)>>4]);

		ci.set_alpha(env[L"alpha"].def(-1.0),env[L"tc"].def(-1.0));     
	}

	return fcons;

}




inline HRESULT file_make_dir_check(const WCHAR* name,bool fsplit=true,v_buf<WCHAR>& buf=v_buf<WCHAR>())
{
	v_buf_fix<WCHAR> tmp;
	WCHAR* pdir,*p;
	int cb=0;
	expand_env(name,buf);
	cb=safe_len(buf);
	if(cb<=0) return E_POINTER;

	pdir=buf;

	if(fsplit) 
		if((buf[cb-1]!=L'\\')&&(buf[cb-1]!=L'/'))
		{
			copy_z(buf.get(),tmp,-1,1);
			p=pdir=tmp;
			for(int n=0;n<cb;++n)
			{
				if(tmp[n]==L'/') tmp[n]=L'\\';
			}
			p=StrRChrW(pdir,pdir+cb,L'\\');
			if(p) *p=0;
		}

		int res=SHCreateDirectoryExW(NULL,pdir,0);
		
			return res;
}

inline bool file_dir_delete(const WCHAR* name,bool f=false)
{
    v_buf<WCHAR> buf;
	WCHAR* p=(WCHAR*) name,*pl;
	int cb=safe_len(p);
	pl=p+cb-1;
	if(cb&&( (*pl=='/')||(*pl=='\\')))
	{
		 p=buf.cat(p,pl).get();

	}

	SHFILEOPSTRUCTW shfo={0,FO_DELETE,p,NULL,FOF_NOERRORUI|FOF_NOCONFIRMATION|FOF_SILENT};
    return !SHFileOperationW(&shfo);
};

inline v_buf<WCHAR>& file_make_dir(const WCHAR* name,bool fsplit=true,v_buf<WCHAR>& buf=v_buf<WCHAR>())
{
	HRESULT hr=file_make_dir_check(name,fsplit,buf);
	if((ERROR_SUCCESS!=hr)&&(ERROR_ALREADY_EXISTS!=hr))
		buf.clear();
     return buf;
}

inline GUID  uuid_generate()
{
	GUID g;
  CoCreateGuid(&g);
  return g;    
}

inline charbuf str_uuid(GUID g= uuid_generate())
{
	DWORD *ii=(DWORD *)&g;
	return charbuf().printf("%08x%08x%08x%08x",ii[0],ii[1],ii[2],ii[3]);    
}
inline wcharbuf wstr_uuid(GUID g= uuid_generate())
{
	DWORD *ii=(DWORD *)&g;
	return wcharbuf().printf(L"%08x%08x%08x%08x",ii[0],ii[1],ii[2],ii[3]);    
}



inline GUID  uuid_file(HANDLE hf,bool fattr=false)
{           
	
	BY_HANDLE_FILE_INFORMATION fi;
	if(GetFileInformationByHandle(hf,&fi))
	{
		struct {DWORD rr[4];}
		tt={fi.nFileIndexLow,fi.dwVolumeSerialNumber,fi.nFileIndexHigh,(fattr)?fi.dwFileAttributes:0};
		return *((GUID*)&tt);
	}
    GUID r={};
	return r;
}



   
inline GUID uuid_file(const wchar_t* fn,bool fattr=false)
{
	GUID r={};
	HANDLE hf=CreateFileW(fn,FILE_READ_ATTRIBUTES,0,0,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,0);
	if(hf!=INVALID_HANDLE_VALUE)
	{
    r=uuid_file(hf,fattr); 
	 CloseHandle(hf);
	}
	return r;
}


inline v_buf<wchar_t>& file_uuid(HANDLE hf,v_buf<wchar_t>& buf=v_buf<wchar_t>())
{
     buf.clear();
	 BY_HANDLE_FILE_INFORMATION fi;
	 if(GetFileInformationByHandle(hf,&fi))
	         return buf.printf(L"%08x%08x%08x",fi.nFileIndexLow,fi.dwVolumeSerialNumber,fi.nFileIndexHigh);
}

inline v_buf<wchar_t>& file_uuid(const wchar_t* fn,v_buf<wchar_t>& buf=v_buf<wchar_t>())
{
	HANDLE hf=CreateFileW(fn,FILE_READ_ATTRIBUTES,0,0,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,0);
	if(hf==INVALID_HANDLE_VALUE) buf.clear();
	else {
	 file_uuid(hf,buf);
	CloseHandle(hf);
	}
	return buf;
}



inline wcharbuf get_app_dir(const WCHAR* name)
{
	wcharbuf buf;
	
	GetEnvironmentVariableW(L"APPDATA",buf,buf.count);
	wcscat(buf,L"\\");
	wcscat(buf,name); 
	int res=SHCreateDirectoryExW(NULL,buf,0);
	if((ERROR_SUCCESS==res)||(ERROR_ALREADY_EXISTS==res))
			return buf;
	else return wcharbuf();
}

struct path_saver_t
{
	wchar_t* path_ex,*pt;
	argv_env<wchar_t> env;

static inline v_buf<wchar_t>& reparser(v_buf<wchar_t>& libspath)
{
	v_buf<wchar_t> buf(2048);
	argv_zz_trunc<wchar_t> tmp(libspath.get(),L';');
	wchar_t* p;
    libspath.clear();
	for(int n=0;n<tmp.argc;n++)
	{
		const wchar_t* pn=tmp.args(n);
      if((pn)&&GetFullPathNameW(pn,2048,buf,&p))
		  libspath.cat(buf.get()).cat(L";");
	}
	return libspath;

};

	path_saver_t(wchar_t* libspath)
	{
		v_buf<wchar_t> libspath_exp;
		v_buf<wchar_t> test_buf(2048);
		int f;
		HRESULT hr;
		path_ex=expand_env(libspath,libspath_exp);
		path_ex=reparser(libspath_exp);

		if(path_ex)
		{
			path_ex=libspath_exp.cat(L";").cat(env[L"PATH"].def(L""));
			f=SetEnvironmentVariableW(L"PATH",libspath_exp.get());
			hr=GetLastError();
			f=SetEnvironmentVariableW(L"XXPATH",libspath_exp.get());
			hr=GetLastError();
			GetEnvironmentVariableW(L"PATH",test_buf,2048);
            path_ex=test_buf.get(); 
			hr=GetLastError();
		}
		argv_env<wchar_t> test;
		argv_zz<wchar_t> test2(GetEnvironmentStringsW());

		pt=test[L"PATH"];
	}
	~path_saver_t()
	{
		if(path_ex)
			SetEnvironmentVariableW(L"PATH",env[L"PATH"].def(L""));

	}

};


template <class MUTEX>
struct locker_t
{
	MUTEX* r;
	
	locker_t(MUTEX* pc=0,bool flock=true):r(pc)
	{
		if(flock&&r) r->lock();
	}
	locker_t(MUTEX& c,bool flock=true):r(&c)
	{
		if(flock&&r) r->lock();
	}
	~locker_t()
	{
		if(r) r->unlock();
	}
 inline void  attach(MUTEX* pc=0,bool flock=true)
	{
          if(flock&&pc) pc->lock();
		  detach();
			  r=pc;
	}
inline 	MUTEX* detach(bool funlock=true)
	{
	   if((funlock)&&r) r->unlock();
	   return make_detach(r);
	}
};


template <class MUTEX>
struct unlocker_t
{
	MUTEX* r;

	unlocker_t(MUTEX* pc=0):r(pc)
	{
		if(r) r->unlock();
	}
	unlocker_t(MUTEX& c):r(&c)
	{
		if(r) r->unlock();
	}
	~unlocker_t()
	{
		if(r) r->lock();
	}
	
};


inline HANDLE new_event(const wchar_t* name,const wchar_t* prfx=L"_e")
{   
	wcharbuf buf;
	wsprintfW(buf,L"%s%s",name,prfx);
	return CreateEventW(0,1,0,buf);
}

inline HANDLE new_mutex(const wchar_t* name,const wchar_t* prfx=L"_m")
{   
	wcharbuf buf;
	wsprintfW(buf,L"%s%s",name,prfx);
	return CreateMutexW(0,0,buf);
}

inline HANDLE new_semaphore(const wchar_t* name,const wchar_t* prfx=L"_s")
{   
	wcharbuf buf;
	wsprintfW(buf,L"%s%s",name,prfx);
	return CreateSemaphoreW(0,1,0x7fffffff,buf);
}

struct mutex_fake_t
{
	inline void lock(){}
	inline void unlock(){}
    inline bool try_lock(int t=0){return true;}
};
struct mutex_cs_t
{
	CRITICAL_SECTION m_cs;
	mutex_cs_t(unsigned int sc=0){

		if(!sc) InitializeCriticalSection(&m_cs);
		else InitializeCriticalSectionAndSpinCount(&m_cs,sc);
	}
	~mutex_cs_t()
	{
		DeleteCriticalSection(&m_cs);
	} // on  	  DLL_PROCESS_DETACH
	inline void lock(){ EnterCriticalSection(&m_cs);}
	inline void unlock(){ LeaveCriticalSection(&m_cs);}

	
    inline bool try_lock(int t=0)
	{
     return  TryEnterCriticalSection(&m_cs);
	}
	
	inline unsigned int set_spin_count(unsigned int sc)
	{
        return SetCriticalSectionSpinCount(&m_cs,sc);
	}

};


struct kobj_t
{
   HANDLE hko;
   kobj_t(HANDLE h=0):hko(h){};
   inline int lock(int tiot=INFINITE)
   {
	   return (hko)?WaitForSingleObject(hko,tiot):WAIT_FAILED;
   };
   inline bool try_lock(int tiot=0)
   {
	   return WAIT_TIMEOUT!=lock(tiot);
   }

   operator HANDLE()
   {
	   return hko;
   }

   void reset(HANDLE h=0)
   {
	   if(hko) CloseHandle(hko);
	   hko=h;
   }
};


struct kobj_holder_t:kobj_t
{
	kobj_holder_t(HANDLE h=0):kobj_t(h){};
	~kobj_holder_t(){ reset();}
};

struct mutex_ref_t:kobj_t
{
	
	mutex_ref_t(HANDLE h=0):kobj_t(h){};
  
   inline int unlock()
   {
	   return ReleaseMutex(hko);
   };
};

struct semaphore_ref_t:kobj_t
{

	semaphore_ref_t(HANDLE h=0):kobj_t(h){};

	inline int unlock()
	{
		return inc();
	};
	inline int inc(int c=1)
	{
		LONG l;
        if(ReleaseSemaphore(hko,c,&l))
		 return l+c;
		else return -1;
	};
	inline int operator()(int c=1)
	{
		return inc(c);
	}

};


struct event_ref_t:kobj_t
{

	event_ref_t(HANDLE h=0):kobj_t(h){};

	
	inline void signal(int c=1)
	{
		if(c) SetEvent(hko);
		else ResetEvent(hko);
		
	};
	inline void operator()(int c=1)
	{
		return signal();
	}

	inline int unlock()
	{
       return 1;
	}


};




struct semaphore_t:semaphore_ref_t,_non_copyable_
{
	semaphore_t(HANDLE h=0):semaphore_ref_t(h){};
	semaphore_t(const wchar_t* name,const wchar_t* prfx=L""):semaphore_ref_t(new_semaphore(name,prfx)){};
	~semaphore_t(){if(hko) CloseHandle(hko);};
};

struct semaphore_local_t:semaphore_t
{
	
semaphore_local_t():semaphore_t(CreateSemaphore(0,1,0x7fffffff,0)){};
	
};


struct event_local_t:event_ref_t,_non_copyable_
{
	event_local_t(bool fmanual=true,bool fsignaled=0,bool fempty=false):event_ref_t((fempty)?0:CreateEvent(0,fmanual,fsignaled,0)){};
	event_local_t(HANDLE h,bool fdup=true)
	{
		if(fdup) DuplicateHandle(GetCurrentProcess(),h,GetCurrentProcess(),&hko,0,FALSE,DUPLICATE_SAME_ACCESS);
		else hko=h;
		
	}

	
	~event_local_t(){ reset(); }
};

/*
struct event_shared_t:event_ref_t,_non_copyable_
{
	event_shared_t():event_ref_t(){};

     bool reset(wchar_t* name,bool fmanual=true,bool fsignaled=0)
	 {          
       HANDLE h=CreateEventW(0,fmanual,fsignaled,name);	  
	   reset(h);
	   bname=name;	   
	   return h;
	 }
	~event_shared_t(){ reset(); }
	v_buf<wchar_t> bname;
};
*/

struct mutex_ko_t:mutex_ref_t,_non_copyable_
{
	mutex_ko_t(HANDLE h=0):mutex_ref_t(h){};
	mutex_ko_t(const wchar_t* name,const wchar_t* prfx=L""):mutex_ref_t(new_mutex(name,prfx)){};
	~mutex_ko_t(){if(hko) CloseHandle(hko);};
};

struct mutex_def_t:mutex_ko_t
{
	mutex_def_t():mutex_ko_t(CreateMutex(0,0,0)){};
};

typedef mutex_ko_t mutex_t;

#define shared_arrea_t shared_area_t
struct shared_area_t:mutex_ko_t
{

	enum{
	     CRDIS=OPEN_ALWAYS,
		OPMOD=GENERIC_READ|GENERIC_WRITE,
		SHAREMOD=FILE_SHARE_READ|FILE_SHARE_WRITE,
		FILEATTR=0
	};

	struct pair_mmn
	{
		wchar_t filename[MAX_PATH*2];
		wchar_t* mapname;
		pair_mmn(wchar_t* fn):mapname(NULL)
		{
			*filename=0;
			if(!fn) return ;
			wcscpy(filename,fn);
			//wchar_t*
			wchar_t* p;
			if(p=wcsstr(filename,L"::"))
			{
				*p=0;
				mapname=p+2;
			}

		};
		inline operator bool()
		{
			return filename[0];
		}
	};

	//HANDLE mutex;
	//mutex_t mutex;
	void *ptr;
	int size;
	HANDLE hmap;
	HRESULT hr;
	bool f_already_exists;
	
	/*
	inline int lock()
	{
		return WaitForSingleObject(mutex,INFINITE);
	};
	inline int unlock()
	{
		return ReleaseMutex(mutex);
	};
	*/

	shared_area_t():hr(E_POINTER),hmap(NULL),ptr(NULL),size(0){}
    
	

	
     HRESULT open(const wchar_t *_prfx) 
	 {
        hmap=OpenFileMappingW(FILE_MAP_ALL_ACCESS,false,wcharbuf().printf(L"%sa_fm",_prfx));
		hr=(hmap)?S_OK:GetLastError();
		if(hr) return hr;
		hko=new_mutex(_prfx,L"_mtx");
		//CreateMutexW(0,0,buf);
	     ptr=MapViewOfFile(hmap,FILE_MAP_WRITE,0,0,4);
		 hr=(ptr)?S_OK:GetLastError();
		 if(ptr)  size=*PLONG32(ptr);		
		 return hr;
	 }

	shared_area_t(const wchar_t *_prfx,const wchar_t * _appname,int _size=4*1024):size(_size),ptr(0),hr(0)
	{
           wcharbuf buf;
		   HANDLE hf=INVALID_HANDLE_VALUE;
		   hmap=0;


		   hko=new_mutex(_prfx,L"_mtx");
		   if(size==0) return;



       if(_appname)  
	   {
 		   wcharbuf fn=get_app_dir(_appname);
		   wsprintfW(buf,L"%s\\%s.dat",(wchar_t*)fn,_prfx);
		   hf=CreateFileW(buf,OPMOD,SHAREMOD,NULL,CRDIS,FILEATTR,NULL);
		   if(hf==INVALID_HANDLE_VALUE) {hr=GetLastError(); return;}
	   }


           

           wsprintfW(buf,L"%sa_fm",_prfx);
		    hmap=CreateFileMappingW(hf,0,PAGE_READWRITE,0,size,buf);
		     bool fa=ERROR_ALREADY_EXISTS==(hr=GetLastError());
			 f_already_exists=fa;
		     if(hmap==0)  return;
		    CloseHandle(hf);

		   //wsprintf(buf,L"%sa_mtx",_prfx);
		    
			   //CreateMutexW(0,0,buf);


		   ptr=MapViewOfFile(hmap,FILE_MAP_WRITE,0,0,size);
		  // CloseHandle(hmap);
		   if(!ptr) {hr=GetLastError(); return;}
		   else hr=0;
            

			locker_t<shared_area_t> lock(this);

			ULONG32& sd=  *((ULONG32*)ptr);
				if(sd==0) sd=size;


	}

	  inline void* commit(int siz)
	  {     
		  
		  if(hr) return 0;

            void* p=MapViewOfFileEx(hmap,FILE_MAP_WRITE,0,0,siz,ptr);

			if(!p)
			{   UnmapViewOfFile(ptr);
				ptr=MapViewOfFile(hmap,FILE_MAP_WRITE,0,0,siz);
			}
			else ptr=p;
			return p;
	  }
       inline void* commit()
	   {
		    return commit(size);
	   }
       inline void decommit()
	   {
          UnmapViewOfFile(make_detach(ptr));
	   }
       ~shared_area_t()
	   {
		   //unlock();
		   if(hmap)
		   {
           unlock();
		   UnmapViewOfFile(ptr);
		    CloseHandle(hmap);
		   }
		   //CloseHandle(mutex);
		   
	   }
       
	   

};

template <class CH,int Size=8*1024>
struct console_shared_data_t
{

	shared_area_t sha;
	static v_buf<wchar_t>& _prefix(v_buf<wchar_t>& buf=v_buf<wchar_t>())
	{
		HWND hwin=GetConsoleWindow();
		DWORD pid=0;
		GetWindowThreadProcessId(hwin,&pid);
		//
		return buf.printf(L"console_shared_data_[%x:%x]",hwin,pid);
		//return buf.printf(L"console_shared_data_[%x]",hwin);
	}

	template<class CHO>
	inline int log(const CHO* pstr,DWORD attr=M_CC)
	{
        return puts(pstr,attr);
	}

	template<class CHO>
	inline int puts(const CHO* pstr,DWORD attr=M_CC)
	{
		 //int ll=safe_len()
		try
		{     
			if(!is_console()) return 0;
			char_mutator<CP_THREAD_ACP> cm(pstr);
			locker_t<shared_area_t> lock(sha);
			return conputs_hn(attr,(wchar_t*)cm,-1);
		}
		catch (...){}
		
		

    }



	template <class CHO>
	void printf(DWORD attr,const CHO * fmt,...)
	{

		va_list argptr;
		va_start(argptr, fmt);

		try
		{     
			if(!is_console()) return ;
			v_buf<CHO> buf;
			puts(buf.vprintf(fmt,argptr).get(),attr);
		}
		catch (...){}
		
	}




	console_shared_data_t(wchar_t* p=_prefix()):sha(p,0,Size){};
	inline CH* str()
	{
		char* p=((char* )sha.ptr)+sizeof(ULONG32);
		return (CH*)p;
	}


	inline	argv_zzs<CH> args()
	{
		locker_t<shared_area_t> lock(sha);
		return argv_zzs<CH>(str());
	}

	inline  operator 	argv_zzs<CH>()
	{
		locker_t<shared_area_t> lock(sha);
		return argv_zzs<CH>(str());
	}

	void operator =( argv_zzs<CH>& args)
	{
		const CH* pp= args.flat_str();
		int len=safe_len_zerozero(pp,true);
		locker_t<shared_area_t> lock(sha);
		CH* pd=str();
		memcpy(pd,pp,len*sizeof(CH));
		//inline    int safe_len_zerozero(const CH* pb,bool fwith_last_zero=false)
	}

	inline int lock(int tio=INFINITE)
	{
		return sha.lock(tio);
	}
	inline int unlock()
	{
		return sha.unlock();
	}



};

/*
struct shared_console_t
{ 
	//class_initializer_T<console_shared_data_t<wchar_t,0> > cons;

template<class CHO>
inline	static int puts(const CHO* str)
	{
		console_shared_data_t<wchar_t,0>& cons= class_initializer_T<console_shared_data_t<wchar_t,0> >().get(); 
		return cons.puts(str);
	};
template<class CH>
inline	static int log(const CH* fmt,...)
{
	if(!is_console()) return 0;

}

};
*/

struct host_singleton_t:_non_copyable_
{
	
//typedef  v_buf_fix<wchar_t> wcharbuf;
//typedef  bufT<wchar_t,128> wcharbuf;

	HANDLE hAbortEvent;
    HANDLE hMutex;
	wcharbuf prfx;



   host_singleton_t(const wchar_t*_prfx,bool fabortonly=false):hMutex(0)
  {      
         wcscpy(prfx,_prfx);  
		  hAbortEvent=new_event(prfx);
		  if(!fabortonly)
		  {
			hMutex=new_mutex(prfx);
			SignalObjectAndWait(hAbortEvent,hMutex,INFINITE,false); 
			ResetEvent(hAbortEvent);
		  };
  }
	~host_singleton_t()
	{
		if(hMutex)
		{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		}
		CloseHandle(hAbortEvent);
	}

static	inline int loop_event(HANDLE handle,int timeout=-1,int wakeMask=QS_ALLINPUT,int flags=MWMO_INPUTAVAILABLE||MWMO_ALERTABLE)
	{
		flags&=(~MWMO_WAITALL); 
		DWORD ws=MsgWaitForMultipleObjectsEx(1,&handle,timeout,wakeMask,flags);
		if(ws==WAIT_OBJECT_0) return 0;
		else if(ws==(WAIT_OBJECT_0+1)) return 1;
		else if(ws==WAIT_IO_COMPLETION) return 2;
		else return -1;
	}


static inline bool  process_msg()
{
	MSG msg;

	while(PeekMessage(&msg,0,0,0,PM_REMOVE))
	{
		if(msg.message==WM_QUIT) return 0;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 1;
};

static inline int  loop_event_msg(HANDLE hko,int timeout=-1,bool fquit=true)
{
	DWORD ws;


	while(ws=loop_event(hko,timeout))
	{
		try
		{
			MSG msg;
			//if(ws<0)
			while(PeekMessage(&msg,0,0,0,PM_REMOVE))
			{
				if((fquit)&&(msg.message==WM_QUIT)) return 0;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		catch (...){}

	}


	return ws;
}
inline int abort()
{
  return SetEvent(hAbortEvent);
}
inline int wait()
{
  return loop_event_msg(hAbortEvent);
}
inline bool signaled()
{
	return WaitForSingleObject(hAbortEvent,0)==WAIT_OBJECT_0;
}
};


//QueueUserWorkItem((LPTHREAD_START_ROUTINE)&s_proc,this,WT_EXECUTELONGFUNCTION)

/*
template <typename L>
inline static void _l_free(L* pt,void  ( __stdcall   L::*ptm)()=0){
	delete pt;  }
*/



namespace asyn_su
{



	template <typename L>
	inline static void _l_free(L* pt,void  (  L::*ptm)()=0)
	{
		delete pt;  
	}

	//template <typename L>
	//inline static void _l_free(L* pt,...){	};
	inline static void _l_free(...){
	};


inline	HANDLE dup_handle(HANDLE h,HANDLE hr=GetCurrentProcess())
	{
		HANDLE r=0;
		if(h&&(h!=INVALID_HANDLE_VALUE))
		{
			DuplicateHandle(GetCurrentProcess(),h,hr,&r,0,0,DUPLICATE_SAME_ACCESS);
		}
		return r;
	};


template<class T,bool ff>
struct _pool
{
	DWORD tid; 
	T* pt;
	_pool(T* p):pt(p){};

    struct ko_t
	{
       T* pt;
	   HANDLE hkr;
	   HANDLE ho;
	   ko_t(HANDLE h,T* p):pt(p),hkr(0),ho(dup_handle(h)){};
	   ~ko_t() {    
		   CloseHandle(ho);
		   UnregisterWait(hkr);  
	   }

	   inline void operator()() { (*pt)(); }
   
	};


	
	static void proc( T* p)
	{
		try{
			(*p)();
		}catch(...){

		};
		if(ff)
			_l_free(p);
	}
	

	static		long __stdcall s_proc( T* p)
	{
		//int* pii=(int* )0xEBAB;
		//return *pii;
	
		 proc(p);
		return 0;
		__try
		{
			proc(p);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			ExitProcess(2);
		}
		return 0;
	};

  static		void __stdcall s_wtc_proc( ko_t* kp,BOOLEAN twf)
  {
	 // return;
	  __try
	  {
		  s_proc(kp->pt);
		  delete kp;	 
	  }
	  __except(EXCEPTION_EXECUTE_HANDLER)
	  {};
      
  }

inline 	long pool()
	{

		if(QueueUserWorkItem((LPTHREAD_START_ROUTINE)&s_proc,pt,WT_EXECUTELONGFUNCTION))
			return 0;
		return GetLastError();
	}

inline  HRESULT register_kobject(HANDLE h,int tio=INFINITE)
{
	ko_t * pko=new ko_t(h,pt);
	if(RegisterWaitForSingleObject(&pko->hkr,h,(WAITORTIMERCALLBACK)&s_wtc_proc,pko,tio,WT_EXECUTELONGFUNCTION|WT_EXECUTEONLYONCE))
		return S_OK;
	delete pko;
	return GetLastError();
      
}
inline 	long thread_os(bool fwait=false)
	{
		DWORD tid=-1;
		HANDLE hth=CreateThread(0,0,(LPTHREAD_START_ROUTINE)&s_proc,pt,0,&tid);

		if(hth)
		{
			if(fwait) WaitForSingleObject(hth,INFINITE);
			CloseHandle(hth);
			return tid;
		}
		return -1;
	}
inline    long call_in_thread(HANDLE hthread)
	{
		long f=::QueueUserAPC((PAPCFUNC)&s_proc,hthread,ULONG_PTR(pt));
		return f;

	}
inline    long call_in_thread(int tid)
{
	HANDLE hthread=OpenThread(THREAD_SET_CONTEXT,0,tid);
	if(!hthread) return 0;
	long f=call_in_thread(hthread);
		CloseHandle(hthread);
		return f;
}

inline 	long thread_crt(bool fwait=false)
	{

		typedef unsigned (__stdcall * StartAddress_t) (void *);
		unsigned tid=-1;
		HANDLE hth=(HANDLE)_beginthreadex(0,0,(StartAddress_t)&s_proc,pt,0,&tid);

		if(hth)
		{
			if(fwait) WaitForSingleObject(hth,INFINITE);
			CloseHandle(hth);
			return 0;
		}
		return GetLastError();
	}
	//
	inline int run(int mode)
	{
		if(mode==1) return pool();
		else {
			if(mode==2) return thread_crt();
			else
			{ if(mode==4) thread_os();
			else return s_proc(pt);
			}
		}

		return -1;   
	}
};

struct thread_queue
{
	union
	{
		struct{ HANDLE h[3];};
		struct{ HANDLE ht,habort,habortex;};
	};
	


static void __stdcall s_proc(thread_queue* p)
 {
	 HANDLE* ph=p->h+1;
	 int nh=(ph[1])?2:1;
	 while(WAIT_IO_COMPLETION==WaitForMultipleObjectsEx(nh,ph,0,INFINITE,true));

 };
    thread_queue()
	{
		memset(h,0,sizeof(HANDLE)*3);
	}
	thread_queue& init(HANDLE habortevent=0) 
	{
		clear();
		typedef unsigned (__stdcall * StartAddress_t) (void *);
		unsigned tid=-1;
		habort=CreateEvent(0,1,0,0);
		habortex=dup_handle(habortevent);
			
		 ht=(HANDLE)_beginthreadex(0,0,(StartAddress_t)&s_proc,this,0,&tid);
		 return *this;
	}


inline	void clear()
	{
		if(habort)
		{
    		SetEvent(habort);
	    	WaitForSingleObject(ht,INFINITE);
		    
		}
		for(int n=0;n<3;n++) if(h[n]) CloseHandle(make_detach(h[n]));
	}

  ~thread_queue()
  {  
	  clear();
  }
  inline bool terminate(int err=0)
  {
       return TerminateThread(ht,err);
  }
  inline HANDLE thread_handle()
  {
      return ht;
  }

  template<class T >
  int asyn_call(T* p,bool ffree=true)
  {
	  if(ht)
	  {
	     if(ffree) return _pool<T,true>(p).call_in_thread(ht);
		 else return _pool<T,false>(p).call_in_thread(ht);
	  }
	  else  return 0;
  }

  inline operator bool()
  {
	  return ht;
  }
};
};


template<class T >
HRESULT pool_on_ko_signaled(HANDLE h,T* p,int tio=-1)
{
	return asyn_su::_pool<T,true>(p).register_kobject(h,tio);
}

template<class T >
int asyn_call(T* p, int mode=1,bool ffree=true)
{
	if(mode==-2)
		return (ffree)?asyn_su::_pool<T,true>(p).thread_crt(true):asyn_su::_pool<T,false>(p).thread_crt(true);

	return (ffree)?asyn_su::_pool<T,true>(p).run(mode):asyn_su::_pool<T,false>(p).run(mode);
};

//#define METHOD_PTR(cl,mm) __method_ptr<cl> ((__method_ptr<cl>::TMT)&cl::mm)


template<class T, class N>
int asyn_call(T* p, typename void  (  T::*run)(), typename N (  T::*release)())
{

	struct asyn_t
	{
		 
		T *p;
		void  (  T::*run)();
		N (  T::*release)();

		asyn_t(T * _p,void  (  T::*_run)(),N (  T::*_release)())
			:p(_p),run(_run),release(_release){};
		inline void operator()()
		{
			(p->*run)();
			(p->*release)();
		}
		~asyn_t(){}

	};

	if(!p) return E_POINTER;
        asyn_t* pap= new asyn_t(p,run,release);
		int res;
   if(res=asyn_call(pap,1,1))
	    delete pap;
   return res;

};
#if defined(_WIN64)
#else
template<class T, class N>
int asyn_call(T* p, typename void  (  T::*run)(), typename N ( __stdcall  T::*release)())
{

	struct asyn_t
	{

		T *p;
		void  (  T::*run)();
		N ( __stdcall T::*release)();

		asyn_t(T * _p,void  (  T::*_run)(),N (__stdcall  T::*_release)())
			:p(_p),run(_run),release(_release){};
		inline void operator()()
		{
			(p->*run)();
			(p->*release)();
		}
		~asyn_t(){}

	};

	asyn_t* pap= new asyn_t(p,run,release);
	int res;
	if(res=asyn_call(pap,1,1))
		delete pap;
	return res;

};
#endif
inline bool is_console()
{
 //bool f; DWORD cmo;
 //return f=GetConsoleDisplayMode(&(cmo=-1)); 
	return !!GetConsoleWindow();
};


inline int set_console_attr(int attr=0x73)
{
  return SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),attr);
}

/*
extern intptr_t _confh;   

intptr_t reinit_conout()
{
   if(_confh<0)
   {
	   _confh = (intptr_t)CreateFileW(L"CONOUT$",
		   GENERIC_WRITE,
		   FILE_SHARE_READ | FILE_SHARE_WRITE,
		   NULL,
		   OPEN_EXISTING,
		   0,
		   NULL );
   }
   return _confh;
}
*/

/*
inline void clrscr(bool cc=false,HANDLE hout=GetStdHandle(STD_OUTPUT_HANDLE))
{
	;
	CONSOLE_SCREEN_BUFFER_INFO scbi;
	GetConsoleScreenBufferInfo(hout,&scbi);
	DWORD len=scbi.dwSize.X*scbi.dwSize.Y;
	WORD attr=scbi.wAttributes;
	DWORD wr;
	COORD c={0,0};
	FillConsoleOutputCharacterW(hout,L' ', len, c, &wr);
	FillConsoleOutputAttribute(hout,attr, len, c, &wr);
	if(cc) SetConsoleCursorPosition (hout,c);
};
*/


struct processinfo_t:PROCESS_INFORMATION
{
	processinfo_t()
	{
		PROCESS_INFORMATION* pp=this;
		*pp=PROCESS_INFORMATION();
	}
	~processinfo_t()
	{
		if(hProcess) CloseHandle(hProcess);
		if(hThread) CloseHandle(hThread);
	}

	

	inline PROCESS_INFORMATION	Detach()
	{
		PROCESS_INFORMATION tmp=*this;
		hProcess=hThread=0;
		return tmp;
	}
	inline DWORD exit_code()
	{
          DWORD ec;
		  BOOL f=GetExitCodeProcess(hProcess,&ec);
		  return f? ec: GetLastError();
	}

};

inline HRESULT elevate_run(const wchar_t* ApplicationName,const wchar_t* CommandLine,PROCESS_INFORMATION* ppi=NULL,DWORD nShow = SW_SHOWNORMAL,bool frunas=true)
{

	SHELLEXECUTEINFOW Shex;
	ZeroMemory( &Shex, sizeof( SHELLEXECUTEINFOW ) );
	Shex.cbSize = sizeof( SHELLEXECUTEINFOW );
	Shex.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
	//Shex.lpVerb = L"runas";
	Shex.lpFile = ApplicationName;
	Shex.lpParameters = CommandLine;
	Shex.nShow = nShow;

	/*
	// obsolette 
		OSVERSIONINFOW os=OSVERSIONINFOW();
		os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
		if(!GetVersionExW(&os)) return -1;
					
        frunas&=os.dwMajorVersion>5;

		Shex.lpVerb = (frunas)?L"runas":L"open";
		*/
	Shex.lpVerb = L"runas";

	HRESULT hr;
	if ( ! ShellExecuteExW( &Shex ) )
		return	hr =  HRESULT_FROM_WIN32(GetLastError());


	if (!  ppi )
	{
		WaitForSingleObject( Shex.hProcess, INFINITE );
		CloseHandle( Shex.hProcess );
		return S_OK;
	}
	ppi->dwProcessId=GetProcessId(ppi->hProcess=Shex.hProcess);



	return  S_OK;
}

inline HRESULT shell_run(const wchar_t* ApplicationName,const wchar_t* CommandLine=0,DWORD nShow = SW_SHOWNORMAL,bool frunas=false)
{
   return elevate_run(ApplicationName,CommandLine,&processinfo_t(),nShow,frunas);
}

#ifndef GET_MODULE_HANDLE_EX_FLAG_PIN
#define GET_MODULE_HANDLE_EX_FLAG_PIN                 (0x00000001)
#define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT  (0x00000002)
#define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS        (0x00000004)
#endif

inline HMODULE __stdcall su__getHinstance(void* p)
{
	HMODULE h=0;
	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)p,&h);
	return h;
};


inline HRESULT registry_module_path(void *p,bool finnstall)
{
	HRESULT hr;
	  wchar_t *pname=L"";
	  v_buf_fix<wchar_t> fn,buf;
      GetFullPathNameW(argv_ini<wchar_t>::get_module_name(su__getHinstance(p)),fn.count(),fn,&pname);
 wchar_t *fmt;
   if(finnstall)
	 fmt=L"/C REG ADD \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s\" /f /ve /d \"%s\"";		  	
     else fmt=L"/C REG DELETE \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s\" /f";
	 

   wsprintfW(buf,fmt,pname,fn.get());
 return hr=elevate_run(L"cmd.exe",buf);
// if(FAILED(hr))
//	 FatalAppExitW(0,error_msg(hr));

}

inline wcharbuf error_msg(HRESULT hr=GetLastError())
{

	const DWORD lang=  MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US);  //MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
	wcharbuf buf;
	int n=	FormatMessageW( 
		//FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		hr,
		lang, // Default language
		(LPWSTR) buf,
		buf.size,
		NULL 
		);
	if(n)
	{

	
	buf[n--]=0;
	for(;n>=0;n--) 
		if(iswspace(buf[n])) buf[n]=0;
		else break;  
	}
	return buf;
}


struct break_point_t {

	break_point_t(bool f=false)
	{
         if(!f) return;
		 //__asm{ int 3;};
	}
};


struct debugger_point_t
{
	debugger_point_t(bool f=false)
	{
		 if(!f) return;
		while(!IsDebuggerPresent())
			Sleep(500);
        break_point_t(1);
	}
};


inline void safe_get_GetStartupInfo(STARTUPINFOW* psi)
{
	__try{
		psi->cb=sizeof(*psi);
		GetStartupInfoW(psi);

	}__except(EXCEPTION_EXECUTE_HANDLER){}

}

inline int clone_process(PROCESS_INFORMATION* ppi=NULL,DWORD crflags=CREATE_SUSPENDED,void* penv=0,bool finher=0)
{
	DWORD lerr;
	STARTUPINFOW si={sizeof(si),0,0,0,0,0,0,0,0,0,0,STARTF_USESHOWWINDOW,SW_SHOW}; 
		
	//safe_get_GetStartupInfo(&si);

	int pid;
	{
	processinfo_t pi;
	//if(!lpCmdLine.length()) return false;
	wchar_t  app[2048],*cmd=GetCommandLineW();
	bool f;
	if(!(f=GetModuleFileNameW(NULL,app,2048)))
		return 0;
	 
	

	f=CreateProcessW(app,cmd,
		NULL,	// pointer to process security attributes 
		NULL,	// pointer to thread security attributes 
		finher,	// handle inheritance flag 
		crflags,	// creation flags 
		penv,	// pointer to new environment block 
		0,	// pointer to current directory name 
		&si,	// pointer to STARTUPINFO 
		&pi);

	//lerr=GetLastError();
	 pid=pi.dwProcessId;
	if(f) if(ppi) *ppi=pi.Detach(); 
	}

   //SetLastError(lerr);
   return pid;
}


inline int start_process_desktop_ex(bstr_t desktop,const wchar_t* pexefile,bstr_t lpCmdLine,DWORD waittime=INFINITE,PROCESS_INFORMATION* ppi=NULL,DWORD crflags=0,void* penv=0,bool finher=0,STARTUPINFOW* psi=0)
{
	HRESULT lerr;
	wchar_t* pdesk=desktop;
	STARTUPINFOW si={sizeof(si),0,pdesk,0,0,0,0,0,0,0,0,STARTF_USESHOWWINDOW,SW_SHOW}; 
	if(!psi) psi=&si;
	processinfo_t pi;
	//if(!lpCmdLine.length()) return false;
	bool f=
		CreateProcessW(pexefile,lpCmdLine,
		NULL,	// pointer to process security attributes 
		NULL,	// pointer to thread security attributes 
		finher,	// handle inheritance flag 
		crflags,	// creation flags 
		penv,	// pointer to new environment block 
		0,	// pointer to current directory name 
		psi,	// pointer to STARTUPINFO 
		&pi);

	lerr=GetLastError();

	if(f)
	{
		WaitForSingleObject(pi.hProcess,waittime);

		if(ppi)
			*ppi=pi.Detach();


		return pi.dwProcessId;
	}
	SetLastError(lerr);
	return 0;

};

inline int start_process_desktop(bstr_t desktop,bstr_t lpCmdLine,DWORD waittime=INFINITE,PROCESS_INFORMATION* ppi=NULL,DWORD crflags=0,void* penv=0,bool finher=0,STARTUPINFOW* psi=0)
{
  return start_process_desktop_ex(desktop,(wchar_t*)0,lpCmdLine,waittime,ppi,crflags,penv,finher);
}

inline int start_process(bstr_t lpCmdLine,DWORD waittime=INFINITE,PROCESS_INFORMATION* ppi=NULL,DWORD crflags=0,void* penv=0,bool finher=0)
{
   return start_process_desktop((wchar_t*)0,lpCmdLine,waittime,ppi,crflags,penv,finher);
}

inline DWORD __stdcall GetParentProcessId(DWORD cid=GetCurrentProcessId())
{
	// if(cid==DEFCID) cid=GetCurrentProcessId();
	HRESULT hr;
	HANDLE hp= ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,NULL);
	PROCESSENTRY32W pe = { sizeof(PROCESSENTRY32W) };
	//MODULEENTRY32W me32 = { sizeof(MODULEENTRY32W) };
	DWORD pid=0;
	bool fnp=::Process32FirstW(hp,&pe);
	hr = GetLastError();
	while(fnp)
	{
		if (cid==pe.th32ProcessID)
		{
			pid=pe.th32ParentProcessID;
			break;
		}
		fnp=::Process32NextW(hp,&pe);
	};
	CloseHandle(hp);
	return pid;
};


template <class CH>
HRESULT append_to_file(CH* fn,char* msg,int cb=-1)
{
     if((fn==0)||(msg==0)||(cb==0)) return S_OK;
	HRESULT hr;
	HANDLE hf;
/*
	hf= CreateFileW(char_mutator<CP_THREAD_ACP>(fn), // open Two.txt
		FILE_APPEND_DATA,         // open for writing
		FILE_SHARE_READ,          // allow multiple readers
		NULL,                     // no security
		OPEN_ALWAYS,              // open or create
		FILE_ATTRIBUTE_NORMAL,    // normal file
		NULL);                    // no attr. template
*/

	hf= CreateFileW(char_mutator<CP_THREAD_ACP>(fn),GENERIC_WRITE|GENERIC_READ,
		FILE_SHARE_READ,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);//|FILE_FLAG_DELETE_ON_CLOSE

	if(INVALID_HANDLE_VALUE==hf)
		 return hr=HRESULT_FROM_WIN32(GetLastError());

	if(cb<0) cb=safe_len(msg,true);
	
	char *p=(char *)msg;
	LARGE_INTEGER ln={};


	if(SetFilePointerEx(hf,ln,&ln,FILE_END))
	{
		DWORD cbl=cb;
       if(LockFile(hf,ln.LowPart,ln.HighPart,cbl,0))
	   {
		   DWORD cbt;
		   hr=(WriteFile(hf,p,cb,&cbt,NULL))?S_OK:HRESULT_FROM_WIN32(GetLastError());
		   UnlockFile(hf,ln.LowPart,ln.HighPart,cbl,0);
	   }
	   else    hr=HRESULT_FROM_WIN32(GetLastError());
	}
	else    hr=HRESULT_FROM_WIN32(GetLastError());


	CloseHandle(hf);
	return hr;

}



struct win_t
{
	DWORD pid;
	DWORD tio;
	HANDLE hp;

	win_t(DWORD _pid,DWORD _tio):pid(_pid),tio(_tio)
	{
		hp=OpenProcess(SYNCHRONIZE,0,pid);
	}
	~win_t(){CloseHandle(hp);};

	inline bool wait(DWORD t)
	{
          //return WaitForInputIdle(hp,t);
		Sleep(t);
		return 1;
	}

	inline BOOL  ewp(HWND hwin)
	{
		DWORD wpid,ff;
		if(GetWindowThreadProcessId(hwin,&wpid)&&(wpid==pid))
		{
			wait(tio);
			ff=SetForegroundWindow(hwin);
			//BringWindowToTop(hwin);
			
		}

		return true;

	}
	static BOOL __stdcall s_ewp(HWND hwin,LPARAM lp)
	{
		return ((win_t*)lp)->ewp(hwin);
	}

	void operator()()
	{
		BOOL ff;
		if(tio>0) wait(tio);
		ff=EnumWindows(&s_ewp,LPARAM(this));
	}



};



inline  int SetForegroundProcess(DWORD pid,DWORD tio)
 {

	 if(AllowSetForegroundWindow(pid))
       return asyn_call(new win_t(pid,tio))==0; 
	   else  return 1;

 }


inline void fatal_assert(HRESULT hr)
{
	if(FAILED(hr))
	{

		wcharbuf err=error_msg(hr),buf;
		buf.printf(L"error=[%x]: %s\n",hr,(wchar_t*)err);

		if(is_console())
			_cwprintf(L"%s",buf.get());
		 else FatalAppExitW(0,buf);
		
	}
}

inline void run_installer(LPWSTR lpszCmdLine)
{
	HRESULT hr;
	startup_console_attr();
	//v_buf<wchar_t> 
	//SYSTEM_INFO si;
	//GetSystemInfo(&si);


	v_buf<wchar_t> cmdline;
	cmdline.cat(L"/C ").cat(lpszCmdLine);
	v_buf<wchar_t> cmd=expand_env(L"%ComSpec%");
	processinfo_t pi;
	hr=elevate_run(cmd,cmdline,0);
	fatal_assert(hr);
};


inline static  wchar_t* GMKB( double v ,bufT<wchar_t>& buf=bufT<wchar_t>())
{

	const double kB=1024,mB=kB*kB,gB=mB*kB,tB=gB*kB;;
	const  wchar_t* prfx[]={L"B",L"KB",L"MB",L"GB",L"TB"};

	double av=(v>=0)?v:-v;


	int n=0;
	if(av>=tB) { av/=tB; n=4;}
	else if(av>=gB) { av/=gB; n=3;}
	else if(av>=mB) { av/=mB; n=2;}
	else if(av>=kB) { av/=kB; n=1;}
	return buf.printf(L"%g %s",av,prfx[n]);
}


template <class Args,class CH>
inline argv_zzs<CH>& set_args_fei(const Args& a,argv_zzs<CH>& argsCH,const CH* section=char_mutator<CP_UTF8>(L"default.options"))
{



	argv_zzs<wchar_t> args;

	args=a;
	argv_zzs<wchar_t>  argfile_b;
	argv_zzs<wchar_t>  argfile;
	argv_zzs<wchar_t>   argini;
	argv_zzs<wchar_t>   argenv;

	argfile_b.parent=&args;
	args.parent=&argfile;
	argfile.parent=&argenv;
	argenv.parent=&argini;


	if(args[L"@@"].is())
		argfile_b = argv_file<wchar_t>(args[L"@@"]);
	if(args[L"@"].is())
		argfile = argv_file<wchar_t>(args[L"@"]);
	if(args[L"env"].def(0))
		argenv=argv_env<wchar_t>();

	argini=argv_ini<wchar_t>((wchar_t*)char_mutator<CP_UTF8>(section),L".ini");

	argsCH=argfile_b;
	return argsCH; 
}



/*
template <class Args,class CH>
inline argv_zzs<CH>& set_args_fei(const Args& a,argv_zzs<CH>& argsCH,const CH* section=char_mutator<CP_UTF8>(L"default.options"))
{


	argv_zzs<wchar_t> args;
	args=a;
	argv_zzs<wchar_t>  argfile;
	argv_zzs<wchar_t>   argini;
	argv_zzs<wchar_t>   argenv;
	args.parent=&argfile;
	argfile.parent=&argenv;
	argenv.parent=&argini;
	if(args[L"@"].is())
		argfile = argv_file<wchar_t>(args[L"@"]);
	if(args[L"env"].def(0))
		argenv=argv_env<wchar_t>();

	argini=argv_ini<wchar_t>((wchar_t*)char_mutator<CP_UTF8>(section),L".ini");
	argsCH=args;
	return argsCH; 
}
*/
template <class CHc,class Args,class CH>
inline argv_zzs<CH>& set_args_cmdline_fei(const CHc* cmd,argv_zzs<CH>& argsCH,const CH* section=char_mutator<CP_UTF8>(L"default.options"))
{
	return set_args_fei(argv_cmdline<CHc>(cmd),argsCH,section);
}

template <int FF=0>
struct singleton_restarter_t
{
	mutex_t mutex;
	event_ref_t abort;

static	VOID CALLBACK WaitOrTimerCallback(
		PVOID lpParameter,
		BOOLEAN TimerOrWaitFired
		)
{
	//ExitProcess(0);
	TerminateProcess(GetCurrentProcess(),1);
};


	singleton_restarter_t(wchar_t* key)
	{
		mutex.hko=new_mutex(key,L"singleton_restarter.mutex");
		abort.hko=new_event(key,L"singleton_restarter.event");
		abort.signal(1);
		mutex.lock();
		HANDLE hro=0;
		abort.signal(0);
		::RegisterWaitForSingleObject(&hro,abort.hko,&WaitOrTimerCallback,this,INFINITE,WT_EXECUTEONLYONCE); 
		

	}


};