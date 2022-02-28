#pragma once
//#include "hook_coregister.h"

#include "ipc_ports/ipc_utils.h"

namespace ipc_utils
{
	struct hook_ltx_corerister_t
	{
		inline static bool check_win_class(HWND hwin,const char* wc="OleMainThreadWndClass")
		{
			char buf[256];
			DWORD pid;
           if(GetWindowThreadProcessId(hwin,&pid)&&(pid==GetCurrentProcessId())&&GetClassNameA(hwin,buf,256))		     
			   return (StrCmpNIA(buf,"OleMainThreadWndClass",256)==0);		   
		   
		   return false;
		}

		static LRESULT CALLBACK s_hook(int code, WPARAM wParam, LPARAM lParam){
			DWORD apt;

			

			if((wParam)&&(code==HC_ACTION)){

				CWPRETSTRUCT* pcw=(CWPRETSTRUCT*)lParam; 
				//if((pcw->message==WM_CREATE)&&check_win_class(pcw->hwnd))
				if(pcw->message==WM_CREATE)
				{
					if(check_win_class(pcw->hwnd))
						apt=COMInit_t::apartment_type();
				}

			} 
			
			return CallNextHookEx(0,code,wParam,lParam);
		};

		hook_ltx_corerister_t(){
			//hr=(hproc=SetWindowsHookEx(WH_CALLWNDPROCRET,&s_hook,HINSTANCE_IN_CODE,0))?S_OK:GetLastError();
			hr=(hproc=SetWindowsHookEx(WH_CALLWNDPROCRET,&s_hook,0,0))?S_OK:GetLastError();
		}

		~hook_ltx_corerister_t(){  if(hproc) UnhookWindowsHookEx(hproc);  }

		HRESULT hr;
		HHOOK hproc;
		//SetWindowsHookEx 

	};

};// ipc_utils