#pragma once

#include "os_utils.h"

#define ECM_SET 0x100
#define ECM_CLS 0x200

 HWND g_hwnd=0;

template <class _Module, typename _Module& _module,long flags=0>
struct external_connection_monitor
{
	HRESULT m_hr;
	HINSTANCE m_hlib;

   HWND m_hwnd;

	typedef  LONG (  __cdecl *PEXTERNAL_CONTROLLER_PROC)(bool fconnect,long strong_lock_count); 
	void* ( __stdcall *ltx_SetHardExternalControllerMonitor)(PEXTERNAL_CONTROLLER_PROC pp,LONG flags);
	ULONG ( __stdcall *ltx_StrongLockObjectsCount)();
	HWND* ltx_hwnd_srv;

	external_connection_monitor(long f=flags):m_hr(E_NOTIMPL)
  {

     if(!(m_hlib=LoadLibraryX(L"LifeTimeX.dll",L"LifeTimeX.AsynWait"))) return ;
	 // if(!(m_hlib=LoadLibraryX(L"LifeTimeX.dll",L"ipc-ltx"))) return ;
     if(!SET_PROC_ADDRESS(m_hlib,ltx_SetHardExternalControllerMonitor)) return;
	 if(!SET_PROC_ADDRESS(m_hlib,ltx_StrongLockObjectsCount)) return;
	 if(!SET_PROC_ADDRESS(m_hlib,ltx_hwnd_srv)) return;
      ltx_SetHardExternalControllerMonitor(&ec_proc,f); 
	   g_hwnd=m_hwnd=wnd_lite("external_connection_monitor");
	   void* ppp=InterlockedExchangePointer((void**)ltx_hwnd_srv,g_hwnd);
	   SetWindowLong(m_hwnd,GWL_USERDATA,LONG(this));
	   SetWindowLong(m_hwnd,GWL_WNDPROC,LONG(&s_wndproc));
     ULONG ll=ltx_StrongLockObjectsCount();
      m_hr=S_OK;
  }

~external_connection_monitor()
{
 if(ltx_SetHardExternalControllerMonitor)
  ltx_SetHardExternalControllerMonitor(NULL,0);
 if(m_hlib) FreeLibrary(m_hlib);
 
}

inline  void AttempQuit(bool force=true)
{
   if(force||(ltx_StrongLockObjectsCount()==0))
	   ::PostThreadMessage(_module.m_dwMainThreadID,WM_QUIT,0,0);
}
static LRESULT __stdcall s_wndproc(  HWND hWnd, UINT Msg, WPARAM wParam,LPARAM lParam)
{
  if(Msg==WM_USER+4)
  {
 
  external_connection_monitor* _this =  (external_connection_monitor*) GetWindowLong(hWnd,GWL_USERDATA);
  if(_this) _this->AttempQuit(wParam);
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
};

static  long   __cdecl ec_proc(bool fconnect,long strong_lock_count)
{

	if(fconnect)
	{
      _module.Lock(); 
	}
	else
	{
    	 _module.Unlock();
		 ::PostMessage(g_hwnd,WM_USER+4,0,0);
       //::PostThreadMessage(_module.m_dwMainThreadID,WM_QUIT,0,0);
	}
	return 0;
}; 
};

//inline HINSTANCE LoadLifeTimeX
     //COMINIT




