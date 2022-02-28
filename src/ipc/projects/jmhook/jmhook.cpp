// jmhook.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "jmhook.h"
// : Defines the entry point for the application.
//
#include "winconsole.h"
#include "ipc2.h"
#include "ipc.h"
//#include "SILK.h"
#define MAX_LOADSTRING 100

_interlock g_hprocess;
process_modal_data  g_modal_data,g_modal_data_child;
bool f,fb=1,fdie=1;
#define fBeep if(fb)Beep




struct  rp_IsDebugPresent_t {


	rp_IsDebugPresent_t(DWORD pid):hrmp(0),paddr(0),hr(0){


		HMODULE hm=::GetModuleHandleW(L"Kernel32");

		if(hm){

			paddr=::GetProcAddress(hm, "IsDebuggerPresent");

			if(paddr){
				hrmp=OpenProcess(
					PROCESS_QUERY_INFORMATION |   // Required by Alpha
					PROCESS_CREATE_THREAD     |   // For CreateRemoteThread
					PROCESS_VM_OPERATION      |   // For VirtualAllocEx/VirtualFreeEx
					PROCESS_VM_READ|
					SYNCHRONIZE |
					PROCESS_VM_WRITE,             // For WriteProcessMemory
					FALSE, pid);


			}
		}
		if(!hrmp) hr=GetLastError();
	};

	bool operator ()()
	{
		///*
		BOOL f,rs=0;

		if(!hr){

			f=::CheckRemoteDebuggerPresent(hrmp,&rs);
			if(!f){
				
				hr=GetLastError();
				rs=0;

			}
			return rs;
		}

		return rs;
		//*/
		//	
		/*
		DWORD tid,res=0;
		if(!hr){
			HANDLE hrt=::CreateRemoteThread(hrmp,0,0,LPTHREAD_START_ROUTINE(paddr),(void*)0,0,&tid);
			if(hrt)
			{
				::WaitForSingleObject(hrt,INFINITE);
				::GetExitCodeThread(hrt,&res);
				::CloseHandle(hrt);
			}
			else {
				hr=GetLastError();
			}
		}
		
		return res;
		//*/
	}


	bool isterm(){
	/*
	DWORD dw=0;
	if(!h) return false;
	return GetExitCodeProcess(h,&dw), !(STILL_ACTIVE==dw);
	*/
	return (!hrmp)||(WAIT_OBJECT_0==WaitForSingleObject(hrmp,0));

   }





	~rp_IsDebugPresent_t(){
		::CloseHandle(hrmp);
	}


	HRESULT hr;
	void* paddr;
	HANDLE hrmp;

};







BOOL WINAPI HandlerRoutine(
						   DWORD dwCtrlType
						   )
{

	f= TerminateProcess( g_hprocess,-1);
	for(int i=0;i<10;i++)	  fBeep(300*i,30);
	for(int i=10;i>0;i--)	fBeep(300*i,30);
	return 0;
};


bool isterm(HANDLE h)
{
	/*
	DWORD dw=0;
	if(!h) return false;
	return GetExitCodeProcess(h,&dw), !(STILL_ACTIVE==dw);
	*/
	return WAIT_OBJECT_0==WaitForSingleObject(h,0);

}



int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,				
					   int       nCmdShow)
{
	HRESULT hr;
	int imode=0;
	HANDLE hp=0;
	g_hprocess=0;
	wchar_t buf[4];
	if(GetEnvironmentVariableW(L"DIE_WITH_PARENT",buf,2))
	{
		DWORD pid=ParentProcessId();
		hp=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE, pid);
	}


	fb=!GetEnvironmentVariableW(L"DISABLE_BEEP",buf,2);


	STARTUPINFOW si={sizeof(si),0,0,0,0,0,0,0,0,0,0,STARTF_USESHOWWINDOW,nCmdShow}; 
	PROCESS_INFORMATION pi;
	bool f;


	f=CreateProcessW(NULL,lpCmdLine,
		NULL,	// pointer to process security attributes 
		NULL,	// pointer to thread security attributes 
		false,	// handle inheritance flag 
		CREATE_SUSPENDED,	// creation flags 
		0,	// pointer to new environment block 
		0,	// pointer to current directory name 
		&si,	// pointer to STARTUPINFO 
		&pi);
	hr=GetLastError();
	if(f)
	{
		g_hprocess=pi.hProcess;



		g_modal_data.open(INJMODAL_NAME);
		if(g_modal_data)
		{
			if(g_modal_data_child.open(INJMODAL_NAME,pi.dwProcessId,true))
			{
				g_modal_data_child->pid_dbg_delegate=GetCurrentProcessId();
				g_modal_data->hwinModal=NULL;
			}
		}





		//   silk();
		//DWORD ppid=ParentProcessId();
		//if(!AttachConsole(ATTACH_PARENT_PROCESS))
		AllocConsole();
		f= SetConsoleCtrlHandler(&HandlerRoutine,true); 
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0xF0);
		clrscr(1);
		//      
		HWND hhh=(g_modal_data)?HWND(1):0;

		set_console_size(coord(80,24));
		conprintf_h(HAC,M_BC|9,"\n process=\"%s\"\n\n PROCESS ID=%d\n\n primary thread id=%d \n modalhook=%x",lpCmdLine,pi.dwProcessId,pi.dwThreadId,hhh);
		/*	
		if(IsDebuggerPresent())
		{
		f=DebugActiveProcess(pi.dwProcessId); 
		}
		*/
		{
			rp_IsDebugPresent_t isDEBUG(pi.dwProcessId);
			//remote_block rb(pi.dwProcessId,sizeof(isDEBUG),&isDEBUG);
			DWORD tic=GetTickCount();
			DWORD cb=0;
			bool f=1;
			//	
			int trig=0;

			//
			consoleXY()=coord(25,15);						
			conprintf_h(HAC,0x0F1,"press Esc for detach...");

			//inline R remote_call(HANDLE hp,P paddress,D param)

			//int frm=remote_call<int>(rb.m_hprocess, isDEBUG.paddr,(int*)0);

			//while(!rb.call())
			//while(!remote_call<int>(rb.m_hprocess, isDEBUG.paddr,(int*)0))
			while(!isDEBUG())
			{
				//Sleep(100);

				//if(isterm(pi.hProcess))
				if(isDEBUG.isterm())
				{
					TerminateProcess(g_hprocess,-1);
					return 0;
				}
				DWORD t=GetTickCount()-tic; 
				if((cb%40)==0)
				{
					cb=0;
					consoleXY()=coord(5,7);						
					conprintf_h(HAC,M_BC|M_FC,"wait debugging ");
					if(f)
				 {
					 f=0;
					 consoleXY()=coord(30,7);						
					 for(int i=0;i<40;i++) conprintf_h(HAC,071,".");
				 }
					consoleXY()=coord(22,7);
					if(trig) conprintf_h(HAC,0x1A," %05d ",t/1000);
					else conprintf_h(HAC,0x1F," %05d ",t/1000); 
					consoleXY()=coord(30,7);
					(++trig)%=2;
					if(fb) Beep(400+200*trig,100);
				}
				if(trig) conprintf_h(HAC,0x17,"o");
				else  conprintf_h(HAC,0x71,":");

				Sleep(50);

				cb++;
				if(kbhit()&&(getch()==VK_ESCAPE)) 
				{
					//return ResumeThread(pi.hThread),0;
					imode=1;
					break;
				}

			};

			//consoleXY()=coord(15,5);

			//consoleXY()=coord(15,6);
			consoleXY()=coord(0,15);	
			conprintf_h(HAC,M_BC|0x0A,lpCmdLine);
		}




		ResumeThread(pi.hThread);
		SetConsoleCtrlHandler(&HandlerRoutine,false); 
		for(int i=30;i>0;i--)	fBeep(100*i,30);
		int  trig=0;
		int bc[2]={0x0c,0xc0};
		int fc[2]={0xe0,0x0e};
		// int bc[2]={0x0a,0xa0};
		// int fc[2]={0x20,0x02};
		HANDLE hh[2]={hp,pi.hProcess};


		int cbb=0;

		while(WAIT_TIMEOUT==WaitForSingleObject(pi.hProcess,500))
		{
			int XPos=5;
			//if(((cbb++)%4)==0)
			{
				clrscr(1);          
				conprintf_h(HAC,M_FC|M_BC,"%d",GetTickCount());
			}
			//if(0)
			if(g_modal_data)
			{
				XPos=1;
				HWND hwin=g_modal_data->hwinModal;
				DWORD tid=0,pid=0;
				consoleXY()=coord(1,XPos+4);
				//if(IsWindow(hwin))
				{
					wchar_t buf1[257],buf2[257];
					tid=GetWindowThreadProcessId(hwin,&pid); 	  

					conprintf_h(HAC,M_AC|M_FC|M_BC,"modal hwnd=%08x [pid=%d ; tid=%d ] ",hwin,pid,tid); 
					buf1[256]=buf2[256]=0;
					if(!GetClassNameW(hwin,buf1,256)) {buf1[0]=L'?'; buf1[1]=0;}
					if(!GetWindowTextW(hwin,buf2,256)) {buf2[0]=L'?'; buf2[1]=0;}
					consoleXY()=coord(1,XPos+5);
					conprintf_h(HAC,M_AC|M_FC|M_BC,"WinClass=\"%s\"",buf1); 
					consoleXY()=coord(1,XPos+6);
					conprintf_h(HAC,M_AC|M_FC|M_BC,"WinTitle=\"%s\"",buf2); 
				} 
				XPos+=8;   
				hwin=g_modal_data->hwinPP;
				consoleXY()=coord(1,XPos+4);
				//if(IsWindow(hwin))
				{
					wchar_t buf1[257],buf2[257];
					tid=GetWindowThreadProcessId(hwin,&pid); 	  

					conprintf_h(HAC,M_AC|M_FC|M_BC,"parent modal hwnd=%08x [pid=%d ; tid=%d ] ",hwin,pid,tid); 
					buf1[256]=buf2[256]=0;
					if(!GetClassNameW(hwin,buf1,256)) {buf1[0]=L'?'; buf1[1]=0;}
					if(!GetWindowTextW(hwin,buf2,256)) {buf2[0]=L'?'; buf2[1]=0;}
					consoleXY()=coord(1,XPos+5);
					conprintf_h(HAC,M_AC|M_FC|M_BC,"WinClass=\"%s\"",buf1); 
					consoleXY()=coord(1,XPos+6);
					conprintf_h(HAC,M_AC|M_FC|M_BC,"WinTitle=\"%s\"",buf2); 
				} 
				XPos=1;

			}

			consoleXY()=coord(1,XPos);

			char* pstr[]={"  << DEBUGGER Attached To Process >>  ","  << Process Started without DEBUG>>  "};

			conprintf_h(HAC,M_AC|fc[trig]|bc[trig],pstr[imode]);
			(++trig)%=2;
			consoleXY()=coord(1,XPos+2);
			conprintf_h(HAC,M_AC|fc[trig]|bc[trig],"  << [pid=%d]: %s >>  ",pi.dwProcessId,lpCmdLine);

			if(isterm(hp)) break;
		};
		for(int i=0;i<30;i++)		fBeep(100*i,30);
	}

	DWORD dw=-1;
	GetExitCodeProcess(pi.hProcess,&dw);
	return (int)dw;
}

