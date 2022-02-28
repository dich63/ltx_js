// ----->>>> logger.c --------------


#include <windows.h>
#include <stdio.h>


typedef struct _tag_PROCESS_LOGGER
{
	char* pbuf;
	HANDLE hmutex;

} PROCESS_LOGGER,*PPROCESS_LOGGER;


PROCESS_LOGGER g_process_logger;

int open_process_logger(PPROCESS_LOGGER ppl)
{
	char buf[128];
	HANDLE hmap;
	int pid;
	pid=GetCurrentProcessId();
	ppl->pbuf=0;
	ppl->hmutex=0;
	snprintf(buf,128,"fmm_F9C00F052F0A41fbAFFF1BB5553DB53C_%x",pid);
	hmap=OpenFileMappingA(FILE_MAP_ALL_ACCESS,0,buf);
	if(hmap==0) return GetLastError();

	ppl->pbuf=(char*)MapViewOfFile(hmap,FILE_MAP_ALL_ACCESS,0,0,0x10000);
	CloseHandle(hmap);
	if(ppl->pbuf==0) return GetLastError();

	snprintf(buf,128,"mtx_F9C00F052F0A41fbAFFF1BB5553DB53C_%x",pid);
	ppl->hmutex=CreateMutexA(0,0,buf);
	return 0;

};

int process_logger_printf(const char* fmt,...)
{
	PPROCESS_LOGGER ppl;
    va_list argptr;
    int n;

	ppl=&g_process_logger;
	
	if(ppl->pbuf==0)
		return 0;
	WaitForSingleObject(ppl->hmutex,INFINITE);
	va_start(argptr, fmt);
	n=vsnprintf(ppl->pbuf,0x10000,fmt,argptr); 
	ReleaseMutex(ppl->hmutex);
	return n;
}


int process_logger_open()
{
  return open_process_logger(&g_process_logger);
}


