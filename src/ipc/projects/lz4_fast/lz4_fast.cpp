// lz4_fast.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "lz4.h"
#include "singleton_utils.h"
 #include <sys/stat.h>


#include "ipc_ports/lz4_fast.h"
#include "ipc_ports/lz4_mm_region.h"



extern "C" long  lz4_compressBound(int isize)
{
  return LZ4_compressBound(isize);
}
extern "C" long  lz4_compress(const void* psrc,void* pdest, long inputSize, long maxOutputSize)
{
   return LZ4_compress_limitedOutput ((const char*) psrc,( char*)pdest,  inputSize,  maxOutputSize);
}

extern "C" long  lz4_decompress(const void* psrc,void* pdest, long inputSize, long maxOutputSize)
{
	return LZ4_decompress_safe((const char*) psrc,( char*)pdest,  inputSize,  maxOutputSize);
}

class stopwatch_t
{
public:
	stopwatch_t() {
		QueryPerformanceFrequency(&m_liPerfFreq), Start(); 
	}

	inline	 void Start() {
		QueryPerformanceCounter(&m_liPerfStart); 
	}

	inline long double Sec()
	{
		LARGE_INTEGER liPerfNow;
		QueryPerformanceCounter(&liPerfNow);
		return (long double)( liPerfNow.QuadPart - m_liPerfStart.QuadPart ) / (long double)(m_liPerfFreq.QuadPart);
	}
	inline LONGLONG Tic()
	{
		LARGE_INTEGER liPerfNow;
		QueryPerformanceCounter(&liPerfNow);
		return (LONGLONG)( liPerfNow.QuadPart - m_liPerfStart.QuadPart );
	}
	inline LONGLONG Frec()
	{
		QueryPerformanceFrequency(&m_liPerfFreq);
		return  m_liPerfStart.QuadPart;
	}
private:
	LARGE_INTEGER m_liPerfFreq;
	LARGE_INTEGER m_liPerfStart;
};



extern "C" void __stdcall  startW
(HWND hwnd, HINSTANCE hinst,LPWSTR lpszCmdLine,int nCmdShow)
{

	argv_zzs<wchar_t> args;
	int fcons=startup_console_attr(L"console.GUI",L"services.console.",1);

	 console_shared_data_t<wchar_t,0> csd;
	set_args_fei(argv_cmdline<wchar_t>(lpszCmdLine),args);
	wchar_t* fin=args[L"in"],* fout=args[L"out"];
	FILE *hin;
	FILE *hout;
	v_buf<char> bufs,bufd;
	if(!(fin&&(hin=_wfopen(fin,L"rb"))))
	{
		csd.printf(M_CC,L" in file is invalid\n");
		ExitProcess(1);
		
	}
	if(!(fout&&(hout=_wfopen(fout,L"wb"))))
	{
		csd.printf(M_CC,L" out file is invalid\n");
        ExitProcess(2);   
	}

	const wchar_t* op=args[L"op"].def(L"c");

	 //fseek(hin,0,SEEK_END);
	struct _stat st;
	 _wstat(fin,&st);
	long s_in=st.st_size,s_out,cb,cbc;
    //fseek(hin,0,SEEK_SET);
	 stopwatch_t cs;
	
	if((op[0]==L'c'))
	{
       bufs.resize(s_in);
	   bufd.resize(s_out=lz4_compressBound(s_in));
	   char* ps=bufs;
	   char* pd=bufd;
	   cb=fread(ps,1,s_in,hin);
	   cs.Start();
	   cbc=lz4_compress(ps,pd,cb,s_out);
	   double t=cs.Sec();
        fwrite(pd,1,cbc,hout);
	   csd.printf(M_CC,L" %d file size=%d  compress at %g sec  ratio=%g%%\n",s_in,cb,t,double(cbc*100)/double(cb));


	   return;
	}
	else if((op[0]==L'd'))
	{
		bufs.resize(s_in);
		bufd.resize(s_out=1024*1024*64);
		char* ps=bufs;
		char* pd=bufd;
		cb=fread(ps,1,s_in,hin);
		cs.Start();
		cbc=lz4_decompress(ps,pd,cb,s_out);
		double t=cs.Sec();
		fwrite(pd,1,cbc,hout);
		csd.printf(M_CC,L" %d file size=%d  decompress at %g sec compress ratio=%g%%\n",s_in,cb,t,double(cb*100)/double(cbc));

		return;
	}

		csd.printf(M_CC,L" unknown operation (op=[c|d] )\n");
        ExitProcess(2);

      

}