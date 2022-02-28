#pragma once 
// #include "res_execute.h"

#include "windows.h"

/*
 // example : 
    hr=res_execute_t(IDR_RCDATA1).exec("cmdline"); // from current module
	hr=res_execute_t(IDR_RCDATA1,NULL).exec("cmdline"); // from application
	
 
*/

template <int I=0>
struct module_t
{
	static HINSTANCE get_hinstance(void* p=&get_hinstance)
	{
		HINSTANCE h=0;
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|
			GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)p,&h);
		return h;
	}
};

struct res_execute_t
{
bool f;
HRESULT hr;
void* ptr;
int count;
wchar_t* filename;
wchar_t* pathname;
wchar_t* cmdexec;


~res_execute_t()
{
 free(filename);
}

res_execute_t(wchar_t* res,HINSTANCE hinst=module_t<>::get_hinstance()):ptr(0),filename(0),count(0),hr(0)
{

	HRSRC hrc;
	HGLOBAL hres; ;
	int cb;
	

	 f=(hrc=FindResourceW(hinst,(wchar_t*)IDR_RCDATA1,RT_RCDATA))&&(hres= LoadResource(0, hrc))&&(ptr=LockResource(hres));
	 if(check_error(f)) return;
	 
      count=SizeofResource(hinst,hrc);
      filename=(wchar_t*)calloc(4,sizeof(wchar_t)*4096);
	  pathname=filename+4096;
	  cmdexec=pathname+4096;

	  if((!check_error(GetTempPathW(4096-256,pathname)))&&(!check_error(GetTempFileNameW(pathname,L"z~",0,filename))))
	  {
    	   cb=writefile(filename,ptr,count);
     	  if( cb!=count) hr=HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
	  }
 	 
}


res_execute_t(int idres,HINSTANCE hinst=argv_ini<char>::get_hinstance())
{
	new(this) res_execute_t((wchar_t*)idres,hinst);
}

inline HRESULT operator()(const wchar_t* cmdline,DWORD to=INFINITE)
{
	return exec(cmdline,to);
}

inline static int check_error(int b,HRESULT& hr)
{
     if(!(b)) hr=HRESULT_FROM_WIN32(GetLastError());
	 return b==0;
}

inline int check_error(int b)
{
  return check_error(f=b,hr);
}

inline int writefile(wchar_t* filename,void* ptr,int count)
{
	FILE* hf;
	hf=_wfopen(filename,L"wb");
	if(!hf)  return -1;

	int cb=fwrite(ptr,1,count,hf);
	fclose(hf);
     return cb;
}

inline HMODULE loadlibrary()
{
	if(FAILED(hr)) return 0;
	 return LoadLibraryW(filename);
}

inline HRESULT exec(const wchar_t* cmdline,DWORD to=INFINITE,const wchar_t* currdir=0)
{
	
    if(FAILED(hr)) return hr;
	
	
	
	/*
	if(check_error(GetTempPathW(4096-256,pathname)))  return hr;
	if(check_error(GetTempFileNameW(pathname,L"z~",0,filename)))  return hr;

    int cb=writefile(filename,ptr,count);

	if( cb!=count) return hr=HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
	*/

	wchar_t* pcdir=(currdir)?(wchar_t*)currdir:pathname;

     *cmdexec=0;  
	 wcscat(wcscat(wcscat(cmdexec,L"\""),filename),L"\"");
	if(cmdline) wcscat(wcscat(cmdexec,L" "),cmdline);
		

       
       HRESULT hr0=S_OK;
	 if(check_error(start_process(cmdexec,to,pcdir),hr0))  return hr0;
       writefile(filename,"?",0);
	   DeleteFileW(filename);
	 return hr0;
}

inline int start_process(const wchar_t* lpCmdLine,DWORD waittime=INFINITE,wchar_t* currdir=0)
{
	DWORD lerr;
	STARTUPINFOW si={sizeof(si),0,0,0,0,0,0,0,0,0,0,STARTF_USESHOWWINDOW,SW_SHOW}; 
	PROCESS_INFORMATION pi={};
	
	currdir=(currdir&&(currdir[0]==0))?0:currdir;
	bool f=
		CreateProcessW(NULL,(wchar_t*)lpCmdLine,NULL,NULL,0,0,0,currdir,&si,&pi);

	lerr=GetLastError();

	if(f)
	{
		WaitForSingleObject(pi.hProcess,waittime);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);


		return pi.dwProcessId;
	}
	SetLastError(lerr);
	return 0;

};


};
