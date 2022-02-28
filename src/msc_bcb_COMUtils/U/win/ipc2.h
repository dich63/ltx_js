#pragma once
#include <Windows.h>
#include "shlwapi.h"
#include <Tlhelp32.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <conio.h>
#include <memory>
#include <comdef.h>

#pragma pack(push)
#pragma pack(1)
#define PUSH_CODE 0x68
#define RET_CODE  0xC3
#define PUSHEAX_CODE  0x50
#define POPEAX_CODE   0x58
#define ADDESP_CODE   0xC481
#define NOP_CODE   0x90

#define KB (1024)
#define ENV_SIZ (64*KB)


#define MAXPCOUNT (4*1024)




template <class R,class P,class D>
inline R remote_call(HANDLE hp,P paddress,D param)
{
	DWORD tt,res=0;
	HANDLE hrt=CreateRemoteThread(hp,0,0,LPTHREAD_START_ROUTINE(paddress),(void*)(param),0,&tt);
	if(hrt)
	{
		WaitForSingleObject(hrt,INFINITE);
		GetExitCodeThread(hrt,&res);
		CloseHandle(hrt);
	}
	return R(res);
}






typedef struct _push_param
{
	byte cpush;byte* p;
} PUSH_PARAM,*PPUSH_PARAM;

template < int N ,int RN=0>
struct  _proc_stub_N
{

	byte cpopeax;                       // POP EAX <-- RET ADDRESS
	WORD caddesp;void* dpc; 
	PUSH_PARAM push_p[N+1];            // PUSH this
	byte cpusheax;                      // PUSH EAX -->RET ADDRESS
	byte cpush2;void* paddr;byte cret;  //  jmp  T:: method

	//=========================

	inline _proc_stub_N(): cpopeax(POPEAX_CODE)
		,cpusheax(PUSHEAX_CODE)
		,cpush2(PUSH_CODE),paddr(NULL)
		,cret(RET_CODE),caddesp(ADDESP_CODE)

	{
		dpc=(void*)(RN*sizeof(void*));
		for(int n=0;n<N;n++) {push_p[n].p=NULL;push_p[n].cpush=PUSH_CODE;}
        memset(push_p+N,NOP_CODE,sizeof(PUSH_PARAM));
	}
	inline _proc_stub_N* param_shift(int nshift)
	{
		dpc=(void*)(nshift*sizeof(void*));
		return this;
	}
	inline _proc_stub_N* init(void *_proc,...)
	{
		//va_list ptr;
		//va_start(ptr,fmt);
		void** params=&_proc;
		paddr=_proc;    
		for(int n=N-1;n>=0;n--) push_p[n].p=(byte*)*(++params);  
		return this;
	}
	inline long call()
	{
		void* p=this;

		return  FARPROC(p)();  
	}
	inline long call_1()
	{
		void* p=this;
		return LPTHREAD_START_ROUTINE(pp)(NULL);  
	}

	inline  operator LPTHREAD_START_ROUTINE()
	{
		return LPTHREAD_START_ROUTINE((void*)this);
	}


};

struct remote_block 
{
	DWORD m_pid;
	HANDLE m_hprocess;
	void* m_rp;
	MEMORY_BASIC_INFORMATION m_mbi;
	DWORD errcode;

	remote_block(DWORD pid,size_t siz=0,void* p=NULL):m_pid(pid),m_rp(NULL)
	{

		if(! (m_hprocess=OpenProcess(
			PROCESS_QUERY_INFORMATION |   // Required by Alpha
			PROCESS_CREATE_THREAD     |   // For CreateRemoteThread
			PROCESS_VM_OPERATION      |   // For VirtualAllocEx/VirtualFreeEx
			PROCESS_VM_READ|
			PROCESS_VM_WRITE,             // For WriteProcessMemory
			FALSE, pid) ) )  
			return ;
		if(siz)
		{

			m_rp=::VirtualAllocEx(m_hprocess, NULL, siz, MEM_COMMIT,PAGE_EXECUTE_READWRITE);
			if(p) load_from(siz,p);
		}
	}
	remote_block(HANDLE hprocess,size_t siz=0,void* p=NULL):m_pid(0),m_rp(NULL),m_hprocess(hprocess)
	{
		
		if(siz)
		{
			m_rp=::VirtualAllocEx(m_hprocess, NULL, siz, MEM_COMMIT,PAGE_EXECUTE_READWRITE);
			if(p) load_from(siz,p);
		}
	}

	inline size_t load_from(size_t siz,void* p=NULL)
	{
		DWORD nbw=0;
		if(p) ::WriteProcessMemory(m_hprocess,m_rp,p,siz,&nbw); 
		return nbw;
	}

	inline size_t save_to(size_t siz,void* p)
	{
		/*
		DWORD nbr=0;
		if(p) ReadProcessMemory(m_hprocess,m_rp,p,siz,&nbr); 
		return nbr;
		*/
		return save_from_address_to(m_rp,siz,p);
	}

	inline size_t save_from_address_to(void* paddress,size_t siz,void* p)
	{
		DWORD nbr=0;
         
		siz=min(remote_max_size(paddress),siz);
		if(errcode) return 0;
		bool f=::ReadProcessMemory(m_hprocess,paddress,p,siz,&nbr); 
		if(!f) 
			errcode=GetLastError();
		return nbr;
	}
inline     void* remote_ptr()
{
	return  m_rp;
}
inline     HANDLE remote_handle()
{
	return  m_hprocess;
}
inline MEMORY_BASIC_INFORMATION& remote_info(void* r_ptr)
{
	m_mbi=MEMORY_BASIC_INFORMATION();
   errcode=0;
   if(!VirtualQueryEx(m_hprocess,r_ptr,&m_mbi,sizeof(m_mbi)))
  	errcode=GetLastError();
    return m_mbi;
}
inline LONG remote_max_size(void* r_ptr)
{
        MEMORY_BASIC_INFORMATION& m=remote_info(r_ptr);
      if(errcode) return 0;   
   LONG dec=LONG(m.BaseAddress)-LONG(r_ptr);
   return LONG(m.RegionSize)+dec;
   
}


	~remote_block()
	{
		if(!m_hprocess) return;
		if(m_rp) VirtualFreeEx(m_hprocess,m_rp,0,MEM_RELEASE);
		if(m_pid) CloseHandle(m_hprocess);
	}
	inline LONG_PTR call()
	{
		DWORD dw;
		HANDLE hp=CreateRemoteThread(m_hprocess,NULL,0,LPTHREAD_START_ROUTINE(m_rp),0,0,&dw);
		WaitForSingleObject(hp,INFINITE);
		GetExitCodeThread(hp,&(dw=-1));
		CloseHandle(hp);
		return dw;
	}

};
#pragma pack(pop)


class  rp_SetEnvironmentVariable :public _proc_stub_N<2,1>
{
	//  public:
	friend  void  test();
	friend  int RemoteSetEnvironmentVariable(wchar_t* name,wchar_t* value,DWORD pid);
//
protected:
//	 public:
	wchar_t m_varname[1024];
	wchar_t m_value[ENV_SIZ];
	rp_SetEnvironmentVariable(wchar_t* name,wchar_t* value)
	{
		wcscpy_s(m_varname,1024,name);
		wcscpy_s(m_value,ENV_SIZ,value);
		void* pproc=GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "SetEnvironmentVariableW");
		init(pproc,m_varname,m_value);
	}


	inline void* reshift(void* paddress)
	{
		LONG_PTR shift= LONG_PTR(paddress)-LONG_PTR(this);
		// 
		push_p[0].p+=shift;
		push_p[1].p+=shift;
		return this;
	}
	/*
	inline LPTHREAD_START_ROUTINE tp()
	{
	return LPTHREAD_START_ROUTINE((void*)this);
	}
	*/
};



class  rp_GetEnvironmentVariable :public _proc_stub_N<3,1>
{
	//  public:
	//friend  int testR(DWORD pid);
    friend 	BSTR RemoteGetEnvironmentVariable(wchar_t* name,DWORD pid);
	friend  int RemoteGetEnvironmentVariable(wchar_t* name,wchar_t* value,size_t cb,DWORD pid);
	//
protected:
//public:
	wchar_t m_varname[ENV_SIZ];
	rp_GetEnvironmentVariable(wchar_t* name)
	{
		wcscpy_s(m_varname,ENV_SIZ,name);
		void* pproc=GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "GetEnvironmentVariableW");
		init(pproc,m_varname,m_varname,ENV_SIZ);
	}


	inline void* reshift(void* paddress)
	{
		LONG_PTR shift= LONG_PTR(paddress)-LONG_PTR(this);
		// 
    //  push_p[0].p //NO MODIFY 
		push_p[1].p+=shift;
		push_p[2].p+=shift;
		return this;
	}
	/*
	inline LPTHREAD_START_ROUTINE tp()
	{
	return LPTHREAD_START_ROUTINE((void*)this);
	}
	*/
};


class  rp_GetCommandLine :public _proc_stub_N<0,1>
{

friend	BSTR RemoteGetCommandLine(DWORD pid);
protected:
	//public:
	rp_GetCommandLine()
	{
        init(GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "GetCommandLineW"));   
	}
  
};

class  rp_IsDebugPresent :public _proc_stub_N<0,1>
{

   
	public:
	rp_IsDebugPresent()
	{
		init(GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "IsDebuggerPresent"));   
	}

};




class  rp_GetEnvironmentStrings :public _proc_stub_N<0,1>
{

	friend	BSTR RemoteEnvironmentStrings(DWORD pid,long* pcount);
protected:
	//public:
	 rp_GetEnvironmentStrings()
	{
		init(GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "GetEnvironmentStringsW"));   
	}

};

class  rp_FreeEnvironmentStrings :public _proc_stub_N<1,1>
{

	friend	BSTR RemoteEnvironmentStrings(DWORD pid,long* pcount);
protected:
	//public:
	rp_FreeEnvironmentStrings(void* p)
	{
		init(GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "FreeEnvironmentStringsW"),p);   
	}

};

class  rp_SetWindowLong:public _proc_stub_N<3,1>
{
	//  public:
	
	friend 	long RemoteSetWindowLong(HWND h,int nIndex,LONG dwNewLong,DWORD pid);
	
	
protected:
		
	rp_SetWindowLong(HWND h,int nIndex,LONG dwNewLong)
	{
		
		void* pproc=GetProcAddress(GetModuleHandle(TEXT("user32")), "SetWindowLongA");
		init(pproc,h,nIndex,dwNewLong);
	}
};





struct BSTR_BUF
{
	long m_cb;
	BSTR m_bstr;
    BSTR_BUF(long cb)
	{
      if(m_bstr=SysAllocStringLen(NULL,cb+2)) m_cb=cb;
	}
	~BSTR_BUF(){if(m_bstr) SysFreeString(m_bstr);}
 operator BSTR()
 {
	 return m_bstr;
 }
 inline void* ptr()
 {
	 return m_bstr;
 }
};

inline BSTR BSTR_DOUBLE_NULL(BSTR bin,long maxcount,long* pcount=NULL)
{
	wchar_t* pend=bin+maxcount;
	int count=0;
	for(wchar_t* pi=bin;(pi!=pend)&&( (*PULONG32(pi))!=0 );pi++)
		count++;
	BSTR bout=::SysAllocStringLen(NULL,count+8);
	if(bout)
	{

	for(int n=0;n<count;n++) bout[n]=bin[n];
	 bout[count++]=0;bout[count++]=0;
     if( pcount) *pcount=count;
   	}
	return bout;
}

inline BSTR RemoteEnvironmentStrings(DWORD pid=GetCurrentProcessId(),long* pcount=NULL)
{
   void *premote_result;
    long lbuf=64*KB;
    LONG siz=lbuf*sizeof(wchar_t);  
	BSTR_BUF buf(lbuf);

   rp_GetEnvironmentStrings rges;
   
   remote_block rb(pid,sizeof(rges),&rges);
        premote_result=(void *)rb.call();
     if(!premote_result) return NULL;
	 
     siz=rb.save_from_address_to(premote_result,siz,buf);
   
    rp_FreeEnvironmentStrings rfes(premote_result); 
    remote_block(pid,sizeof(rfes),&rfes).call();
	
    return BSTR_DOUBLE_NULL(buf,lbuf,pcount);
};

inline BSTR RemoteGetCommandLine(DWORD pid=GetCurrentProcessId())
{
    rp_GetCommandLine rcl;
    long lbuf;
    BSTR_BUF buf(4096);
    remote_block rb(pid,sizeof(rcl),&rcl);
	void *pres=(void *)rb.call();
	if(!pres) return NULL;
   
	int siz=rb.save_from_address_to(pres,sizeof(wchar_t)*1024,buf);
	if(siz<=0) return NULL;
	return ::SysAllocString(buf);
};


//*/

inline DWORD __stdcall ParentProcessId(DWORD cid=GetCurrentProcessId())
{
	// if(cid==DEFCID) cid=GetCurrentProcessId();
	HANDLE hp= CreateToolhelp32Snapshot(TH32CS_SNAPALL,NULL);
	PROCESSENTRY32 pe;
	pe.dwSize=sizeof(PROCESSENTRY32);
	DWORD pid=0;
	bool fnp=Process32First(hp,&pe);
	while(fnp)
	{
		if (cid==pe.th32ProcessID)
		{
			pid=pe.th32ParentProcessID;
			break;
		}
		fnp=Process32Next(hp,&pe);
	};
	CloseHandle(hp);
	return pid;
};

inline DWORD __stdcall ProcessIdFromThreadId(DWORD tid=GetCurrentThreadId())
{
	// if(cid==DEFCID) cid=GetCurrentProcessId();
	HANDLE hp= CreateToolhelp32Snapshot(TH32CS_SNAPALL,NULL);
	THREADENTRY32 te;
	te.dwSize=sizeof(THREADENTRY32);
	DWORD pid=0;
	bool fnp=Thread32First(hp,&te);
	while(fnp)
	{
		if (tid==te.th32ThreadID)
		{
			pid=te.th32OwnerProcessID;
			break;
		}
		fnp=Thread32Next(hp,&te);
	};
	CloseHandle(hp);
	return pid;
};



inline  BSTR  RemoteGetEnvironmentVariable(wchar_t* name,DWORD pid=GetCurrentProcessId())
{
	BSTR_BUF buf(sizeof(rp_GetEnvironmentVariable));
	rp_GetEnvironmentVariable& gev=*( ::new(buf.ptr())rp_GetEnvironmentVariable(name));
	remote_block rb(pid,sizeof(gev));
	gev.reshift(rb.m_rp);
	rb.load_from(sizeof(gev),&gev);
	int res=rb.call();
	if(res<=0) return  NULL;
	gev.m_varname[0]=0;
	if(!rb.save_to(sizeof(gev),&gev)) return  NULL;
	return ::SysAllocString(gev.m_varname);
};




inline  int RemoteGetEnvironmentVariable(wchar_t* name,wchar_t* pvalue,size_t cb,DWORD pid=GetCurrentProcessId())
{
	rp_GetEnvironmentVariable gev(name);
	remote_block rb(pid,sizeof(gev));
	gev.reshift(rb.m_rp);
	rb.load_from(sizeof(gev),&gev);
	 int res=rb.call();
	 if(res<=0) return 0;
     if(!rb.save_to(sizeof(gev),&gev)) return 0;
	 StrCpyNW(pvalue,gev.m_varname,cb);
	  return res;
};

inline int RemoteSetEnvironmentVariable(wchar_t* name,wchar_t* value,DWORD pid=GetCurrentProcessId())
{
	BSTR_BUF buf(sizeof(rp_SetEnvironmentVariable));
	rp_SetEnvironmentVariable& sev=*( ::new(buf.ptr())rp_SetEnvironmentVariable(name,value));	
	//rp_SetEnvironmentVariable sev(name,value);
	remote_block rb(pid,sizeof(sev));
	sev.reshift(rb.m_rp);
	rb.load_from(sizeof(sev),&sev);
	return rb.call();
}

inline long RemoteSetWindowLong(HWND h,int nIndex,LONG dwNewLong,DWORD pid=GetCurrentProcessId())
{
	rp_SetWindowLong swl(h,nIndex,dwNewLong);
	remote_block rb(pid,sizeof(swl),&swl);
	return rb.call();
};



inline  DWORD ConsoleProcessId()
{
	DWORD pid;
    HWND hwnd;GetConsoleWindow();
   if((hwnd=GetConsoleWindow())&&(GetWindowThreadProcessId(hwnd,&pid)))
	   return pid;
   return 0;
}
inline  int ConsoleGetEnvironmentVariable(wchar_t* name,wchar_t* pvalue,size_t cb)
{
	return RemoteGetEnvironmentVariable(name,pvalue,cb,ConsoleProcessId());
}
inline  int ConsoleSetEnvironmentVariable(wchar_t* name,wchar_t* pvalue,bool fall=false)
{
	if(fall) 
	{
         DWORD pl[MAXPCOUNT],cp=GetConsoleProcessList(pl,MAXPCOUNT);
		 for(int n=0;n<cp;n++)
               RemoteSetEnvironmentVariable(name,pvalue,pl[n]);  
			 return cp;
	}
    else   return RemoteSetEnvironmentVariable(name,pvalue,ConsoleProcessId());
}