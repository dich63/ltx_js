#pragma once
#include <windows.h>

/*
template <int Ver=0>
struct pipe_id_t
{
	typedef 	BOOL WINAPI (*pid_proc_t)(HANDLE Pipe,PULONG ProcessId);
	
	pid_proc_t pGetNamedPipeServerProcessId,pGetNamedPipeClientProcessId;
	
	pipe_id_t()
	{
		HMODULE hk=GetModuleHandleA("Kernel32.dll");
		pGetNamedPipeServerProcessId=(pid_proc_t)GetProcAddress(hk,"GetNamedPipeServerProcessId");
		pGetNamedPipeClientProcessId=(pid_proc_t)GetProcAddress(hk,"GetNamedPipeClientProcessId");
	}
	int get_remote_pid(HANDLE hpipe)
	{
		DWORD flags;
		ULONG pid=0;
		if(GetNamedPipeInfo(hpipe,&flags,0,0,0))
		{
			if(flags&PIPE_SERVER_END)
				(pGetNamedPipeClientProcessId)&&pGetNamedPipeClientProcessId(hpipe,&pid));
			else (pGetNamedPipeServerProcessId)&&pGetNamedPipeServerProcessId(hpipe,&pid);

		}

		return pid;
	}
	
	int pid_resolve(HANDLE hpipe,int& pid)
	{
		if(!pid)
			pid=get_remote_pid(hpipe);
			return pid;
	}
};
*/
