// tem.cpp : Defines the entry point for the console application.
//
// http://msdn.microsoft.com/en-us/library/aa384182(v=vs.85).aspx

#include "stdafx.h"
#include <conio.h>
#include "ipc_ports/dispatch_call_helpers.h"
#include "ipc_ports/ipc_utils.h"
#include "tls_dll.h"
#include "resource.h"
#include <vector>
#include <math.h>
#include "wchar_parsers.h"
#include "singleton_utils.h"
#include "os_utils.h"

#include "dbghelp.h"
//
#include "winconsole.h"
#pragma comment(lib ,"dbghelp.lib")
//#include "OAIdl.idl"
using namespace ipc_utils;
//IFileOperation* ppp=0;
//ImageNtHeader
typedef ltx_helper::arguments_t<variant_t>  arguments_t;
//typedef ipc_utils::scriptcaller_t<> scriptcaller_t;


extern "C"  void __cdecl  print_console
( variant_t& result, void* pctx,pcallback_context_arguments_t pcca)
{
	arguments_t arguments(pcca);

	_cwprintf(L"%s\n",(wchar_t*) arguments[0].def<bstr_t>(L"??") );
}

struct print_obj_t: ltx_helper::dispatch_wrapper_t<print_obj_t,variant_t>
{

	inline  void  oncallback(com_variant_t& result,arguments_t& arguments){

		_cwprintf(L"%s\n",(wchar_t*) arguments[0].def<bstr_t>(L"??") );
	};

};


template< class NumType >
struct binary_array_t: ltx_helper::dispatch_wrapper_t<binary_array_t<NumType>,variant_t>
{
	typedef NumType number_t;


	inline bool check_bound(int i,HRESULT& hr)
	{
		if((0<=i)&&(i<vbuf.size())) return true;
		hr=HRESULT_FROM_WIN32(RPC_S_INVALID_BOUND);
		return false ;

	}

	binary_array_t(int N):vbuf(N){}

	inline  void  on_get(com_variant_t& result,arguments_t& arguments){
		try{

			if(!arguments.length())
				result=int(vbuf.size());			
			else{				
				int i=arguments[0];

				if(check_bound(i,arguments.hr))				
					result=vbuf[i];		
			}


		}
		catch(com_exception& e)
		{
			arguments.hr=e.Error();
		}

	};

	inline  void  on_put(com_variant_t& new_value,arguments_t& arguments){
		try{

			if(!arguments.length())
				arguments.hr=E_INVALIDARG;
			else{				
				int i=arguments[0];
				if(check_bound(i,arguments.hr))				
					vbuf[i]=number_t(new_value);		
			}


		}
		catch(com_exception& e)
		{
			arguments.hr=e.Error();
		}

	};
	/*
	inline  void  oncallback(com_variant_t& result,arguments_t& arguments){
	try{

	if(!arguments.length())
	{
	if(arguments.is_put_flag()) { arguments.hr=E_INVALIDARG; return; }
	result=int(vbuf.size());

	}
	else{

	//if(arguments.is_put_flag());				  
	//else result=77.9;


	int i=arguments[0];
	if(check_bound(i,arguments.hr)){

	if(arguments.is_put_flag())			       
	vbuf[i]=number_t(result);
	else result=vbuf[i];
	}

	}


	}
	catch(com_exception& e)
	{
	arguments.hr=e.Error();
	}

	};
	*/
	inline NumType& operator[](int n)
	{
		return vbuf[n];
	}
	inline int size(){
		return vbuf.size();
	}
	std::vector<number_t> vbuf; 
};

struct testTLS_T {};
INT_PTR g_rr=0;
int testTSS()
{
 HANDLE hm=CreateMutex(0,0,0);
 mutex_cs_t mu;
 INT_PTR r,N=1024*1024;
double t;

bstr_t bb=L"012345678";
wchar_t* pp=bb;
INT32* np=(INT32*)pp;
np--;

 stopwatch_t cs;


   cs.Start();
   for(int k=0;k<N;k++)
   {
	   r=tls_var<testTLS_T>();
	   r+=k;
        tls_var<testTLS_T>()=r;
   }
   t=cs.Sec();
   cprintf("testtls: time %g [ms] repeat %d sum=%g \n",1000*t,N,r);
   r=0;
   cs.Start();
   for(int k=0;k<N;k++)
   {
	   
	   r=g_rr;
	   r+=k;
	   g_rr=r;
   }
   t=cs.Sec();
   cprintf("test0: time %g [ms] repeat %d sum=%g \n",1000*t,N,r);
     r=0;
   cs.Start();
   for(int k=0;k<N;k++)
   {
	   WaitForSingleObject(hm,-1);
	   r=g_rr;
	   r+=k;
	   g_rr=r;
   }
   t=cs.Sec();
   cprintf("testmu: time %g [ms] repeat %d sum=%g \n",1000*t,N,r);

   cs.Start();
   for(int k=0;k<N;k++)
   {
      locker_t<mutex_cs_t> lock(mu);
	   r=g_rr;
	   r+=k;
	   g_rr=r;
   }
   t=cs.Sec();
   cprintf("testmu: time %g [ms] repeat %d sum=%g \n",1000*t,N,r);



	return 0;
}


#undef GetObject


struct asy_t
{
	DWORD dwcook;
	asy_t(DWORD dw):dwcook(dw){}
	void operator()()
	{
		HRESULT hr;
		COMInitA_t ci;


		smart_ptr_t<IDispatch> mmbv;
hr=::CoGetObject(bstr_t(L"ltx.bind:CoDispatch_mbv: uzuzuzu=ebtacc"),0,__uuidof(IDispatch),mmbv.ppQI());
		smart_GIT_ptr_t<IDispatch> ddf(mmbv.p);
		Sleep(-1);

/*    	smart_ptr_t<IDispatch> dd;
		smart_GIT_ptr_t<IDispatch> disp;
		disp.detach( dwcook);
		hr=disp.unwrap(__uuidof(IDispatch),dd._ppQI());
		*/

	}
};

#define MB (1024*1024)
#define GB (1024*MB)
void testFM()
{ BOOL f;
	HRESULT hr;
	LARGE_INTEGER l;
	l.QuadPart=8*INT64(GB);
	HANDLE	hm=CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE|SEC_COMMIT,l.HighPart,l.LowPart,0);
	

	hr=GetLastError();
	l.QuadPart=28*INT64(GB);
	char* pp=(char*) MapViewOfFile(hm,FILE_MAP_ALL_ACCESS,l.HighPart,l.LowPart,GB/2);
	void* ptr;
	ptr=VirtualAlloc(pp,GB/2,MEM_COMMIT,PAGE_READWRITE);
	strcpy(pp,"GGGGGGGGA");
	 //f=VirtualFree(pp,GB,MEM_DECOMMIT);
	//ptr=VirtualAlloc(pp,GB,MEM_RESERVE,PAGE_NOACCESS);
    hr=GetLastError();
	f=UnmapViewOfFile(pp);
	hr=GetLastError();
    pp=(char*) MapViewOfFile(hm,FILE_MAP_ALL_ACCESS,l.HighPart,l.LowPart,GB/2);
	ptr=VirtualAlloc(pp,GB,MEM_COMMIT,PAGE_READWRITE);
	hr=GetLastError();
	getchar();


}
void* testdll()
{
	HMODULE hl=GetModuleHandleA("kernel32.dll");
    void* ptr=GetProcAddress(hl,"AddDllDirectory");
	return ptr;
}
/*
struct module_helper_t
{
	HRESULT hr;
	HANDLE hmap;
	PIMAGE_NT_HEADERS pnth;
	void *pbase;

	module_helper_t(const wchar_t* modulename=0):pbase(0),hmap(0),hr(S_OK)
	{
        
		void* p;

		if(modulename)
		{

		
		HANDLE hm,hf=CreateFileW(modulename,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
		if(hf==INVALID_HANDLE_VALUE)
			OLE_CHECK_VOID(hr=HRESULT_FROM_WIN32(GetLastError()));
		hm = CreateFileMapping(hf,NULL,PAGE_READONLY,0,0,NULL );
		
		if(!hm) hr=HRESULT_FROM_WIN32(GetLastError());
		OLE_CHECK_VOID(hr);
         
		hmap=hm;
		
		p=MapViewOfFileEx(hm,FILE_MAP_READ,0,0,0,NULL );        
		if(!p)
             OLE_CHECK_VOID(hr=HRESULT_FROM_WIN32(GetLastError()));
		pbase=p;
		}
		else p=GetModuleHandleW(0);

		pnth=ImageNtHeader(p);
		if(!pnth)
			OLE_CHECK_VOID(hr=HRESULT_FROM_WIN32(GetLastError()));

			 


	};
	~module_helper_t(){
		if(pbase) UnmapViewOfFile(pbase);
		if(hmap) CloseHandle(hmap);
	}
	inline bool is_win32()
	{
		return SUCCEEDED(hr)&&(pnth->FileHeader.Machine==IMAGE_FILE_MACHINE_I386);
	}

	inline bool is_dll()
	{
		return SUCCEEDED(hr)&&(pnth->FileHeader.Characteristics&IMAGE_FILE_DLL);
	}

};
*/

PIMAGE_NT_HEADERS testimload(const wchar_t* modulename=0)
{
      module_helper_t mh(modulename);
	  module_helper_t mh0;
	  bool fdll=mh.is_dll();
	  bool fx=mh.is_win32();
	  bool fx0=mh0.is_win32();



	IMAGE_NT_HEADERS ih={};
	void* pp=argv_ini<wchar_t>::get_hinstance();
	pp=GetModuleHandleW(0);
    return ImageNtHeader(pp);

	////
}

#include<Mshtml.h>
#include "ipc_ports/moniker_parsers.h"
int _tmain(int argc, _TCHAR* argv[])
{
	HRESULT hr, hrEx, hrCC;
	BSTR psi=0;
	ipc_utils::clsid_t clsid(L"{16D51579-A30B-4C8B-A276-0FF4DC41E755}");
	hr = ProgIDFromCLSID((CLSID)clsid, &psi);


	bool fl,fh,fr;
	  
	fl=1;
	fh=0;
	fr=fh^fl;
	fl=0;
	fh=1;
	fr=fh^fl;
	fl=1;
	fh=1;
	fr=fh^fl;
	fl=0;
	fh=0;
	fr=fh^fl;



	//PIMAGE_NT_HEADERS pih=testimload();
	testimload(L"r:\\ipc\\bin_d\\win32\\ltx_js.dll");
	testimload(L"r:\\ipc\\bin_d\\x64\\test3.exe");
//testFM();
 //testTSS()	;
	IHTMLWindow2* pppW;
	v_buf<wchar_t> vbk(2000);
	wchar_t *pbk=vbk,*ppp;
	DWORD ct,cb;
	HKEY hk;
	moniker_parser_t<wchar_t> mnp;

    ppp=mnp.assoc_module(L"R:/ipc/scripts/canvas.hta");

	HRESULT hrr=RegGetValue(HKEY_CLASSES_ROOT,L".js",L"",RRF_RT_REG_SZ,&ct,pbk,&(cb=vbk.size_b())); 

	hrr=RegOpenKeyExW(HKEY_CLASSES_ROOT,L"avsfile",0,KEY_QUERY_VALUE,&hk);

	hrr=RegGetValue(HKEY_CLASSES_ROOT,L"JSfile\\shell\\open\\command",L"",RRF_RT_REG_SZ,&ct,pbk,&(cb=vbk.size_b())); 


	void * pppp=(void*)-1;
int rc;
	
	COMInitF_t ci;

	SYSTEM_INFO* si=new SYSTEM_INFO,*si2;
	GetSystemInfo(si);

	variant_t vsi;
		hr=ltx_helper::wrapObject(si,&vsi);
         hr=ltx_helper::unwrapObject(vsi,&si2);
		 vsi.Clear();




	
	smart_ptr_t<IDispatch> externalLTXOld,ec_locker,testmarsh,mmbv;
	smart_ptr_t<IUnknown> tmmm;
	variant_t callback_print,extV;

     hr=ec_locker.CoGetObject("ltx.bind:ec_locker:  global=3 ;");
	 int index;

	

	hrEx=hr=::CoGetObject(bstr_t("ltx.bind:old_style.external: debug=0"),0,__uuidof(IDispatch),externalLTXOld.ppQI());
	cprintf("ltx.bind:old_style.external: status=0x%x \"%s\" \n",hr,(char*)exceptinfo_t::error_msg(hr));

   smart_ptr_t<IMarshalByValueDispatchRegister> mbvr;
   hr=mbvr.CoGetObject("ltx.bind:mbv.register");


   scriptcaller_t<> scriptcaller(L"ltx.bind:script:debug=3 ;");
   scriptcaller_t<> scriptcaller2(L"ltx.bind:script:");


  // 
   scriptcaller(L"zzb=require()")();
   variant_t r11=scriptcaller(L"zzb=bindObject('ltx.bind:CoDispatch_mbv: jjjjj=jsjsjsjs');aa=zzb()")();
  // 
   variant_t rrr11=scriptcaller(L"zzb")();


   hr=::CoGetObject(bstr_t(L"ltx.bind:CoDispatch_mbv: uzuzuzu=ebtacc"),0,__uuidof(IDispatch),mmbv.ppQI());
	//tmmm.Release();


	rc=ipc_utils::ref_count(mmbv.p);


    hr=::CoGetObject(bstr_t("ltx.bind:testmarshal"),0,__uuidof(IDispatch),testmarsh.ppQI());

	if(SUCCEEDED(hr)) extV=externalLTXOld;

	
	hr=(new print_obj_t)->wrap(callback_print.GetAddress(),FLAG_MASK_PUT|FLAG_CBD_EC);


	cprintf("wrap: status=0x%x \"%s\" \n",hr,(char*)exceptinfo_t::error_msg(hr));


	{

		variant_t rr=mmbv.toVARIANT();

	}

	//ComClipboard()=mmbv.p;
//	ComClipboard()=variant_t(898);

if(0)	{
       variant_t gha;
		//stub_holder sho(mmbv.p,MSHCTX_LOCAL);
		stub_holder sho(testmarsh.p,MSHCTX_LOCAL);
		bstr_t b1=sho.objref();

		stub_holder sho2(ec_locker.p,MSHCTX_LOCAL);

		CHGlobal<BYTE> ch(sho.m_hmem);
		CHGlobal<BYTE> ch2(sho2.m_hmem);
        bstr_t b3=sho2.objref();
		hr=sho.unwrap(gha);

	

	}

//
clrscr();

  //smart_GIT_ptr_t<IDispatch> ddf(testmarsh.p);
  

if(0)
{

	//    asyn_call(new asy_t(ddf.dwcookie));
	    asyn_call(new asy_t(0));


	getchar();
}
   //variant_t rdd221=scriptcaller(L"zz=$$[0]")(mmbv.toVARIANT());
   variant_t rdd11=scriptcaller(L"zz=$$[0]")(mmbv.toVARIANT());

   
   
	   variant_t rdd2211=scriptcaller2(L"zz=$$[0]")(rdd11);
	   variant_t r1122=scriptcaller2(L"zz()")();
	   ComClipboard()=mmbv.p;
	   getchar();
   
	/*	
	ComClipboard()=testmarsh.p;
	variant_t vmm=ComClipboard();
	variant_t v1mm=ComClipboard();
	*/

//	variant_t raa1=scriptcaller(L"aa=$$[0];00")(mmbv.toVARIANT());
	variant_t raa1=scriptcaller(L"aa=$$[0];00")(testmarsh.toVARIANT());

	 raa1=scriptcaller(L"aa")();
     scriptcaller(L"aa.zzazaz")();

	variant_t r122=scriptcaller(L"zz=$$[0];zz('aaaa')")(callback_print);

     

	 variant_t r12=scriptcaller2(L"zz=$$[0];zz.a222")(mmbv.toVARIANT());

	//
	scriptcaller(L"aa=$$[0]")(testmarsh.toVARIANT());


	variant_t  res=scriptcaller.arguments(callback_print,extV)<<js_text(

		var cbprint=$$[0],external=$$[1];
	function printf() {
		cbprint(vsprintf(arguments));
	};
	if(!external) { 
		printf("Error : ProcessExternalData not Found!");	
	}
	else {
		printf("DUMP ProcessExternalData!");
		var len=external.arg.len;
		var extern2=GetObject("ltx.bind:old_style.external: debug=3");

		var fequ=(extern2==external)? "extern2==external":"extern2!=external";
		printf(fequ);
		printf("source args len=%d",len);        
		if(len)	
		{
			s=dump(external.arg[0],1);
			printf(s);
			var win=webform();
			win.document.write(s);

		}

	}

	);

	hr=scriptcaller.hr;
	cprintf("scriptcaller: status=0x%x \"%s\" \n",hr,(char*)exceptinfo_t::error_msg(hr));
	// test 2;





	stopwatch_t cs;
	double t,sum;

	res_str_t resjs(MAKEINTRESOURCEW(IDR_HTML1));
	scriptcaller(resjs);

	int NA=1024*1024;


	variant_t array_disp;
	binary_array_t<double>* pba0,* pba1;

	hr=(pba0=new binary_array_t<double>(NA))->wrap(array_disp.GetAddress(),1*(FLAG_CBD_FTM<<16));

	hr=array_disp.punkVal->QueryInterface(GUID_NULL,(void**)&pba1);


	

	int N=NA;



	cs.Start();
	sum=scriptcaller(L"debugger;\ntest_write_array(arguments[0],arguments[1])")(array_disp,N);
	t=cs.Sec();
	hr=scriptcaller.hr;
	cprintf("scriptcaller: status=0x%x \"%s\" \n",hr,(char*)exceptinfo_t::error_msg(hr));
	cprintf("test_write_array: time %g [ms] repeat %d sum=%g \n",1000*t,N,sum);
	cs.Start();
	sum=scriptcaller(L"test_read_array(arguments[0],arguments[1])")(array_disp,N);
	t=cs.Sec();
	hr=scriptcaller.hr;
	cprintf("scriptcaller: status=0x%x \"%s\" \n",hr,(char*)exceptinfo_t::error_msg(hr));
	cprintf("test_read_array: time %g [ms] repeat %d sum=%g \n",1000*t,N,sum);


	//test 3 

	variant_t vax,vay,vwin;
	int NG=40*1024;

	binary_array_t<double>* pax=new binary_array_t<double>(NG);
	binary_array_t<double>* pay=new binary_array_t<double>(NG);


	hr=pax->wrap(vax.GetAddress(),(FLAG_CBD_FTM<<16));
	hr=pay->wrap(vay.GetAddress(),(FLAG_CBD_FTM<<16));

	for(int x=0;x<NG;x++)
	{
		double r= rand()/double(RAND_MAX) ;
		double A=1+r/2;
		(*pax)[x]=A*cos(1000*double(x)/double(NG))*exp(-4*double(x)/double(NG));
		(*pay)[x]=A*sin(1000*A*double(x)/double(NG))*exp(-4*double(x)/double(NG));
	}
	wchar_t* purl=L"S:\\highcharts\\Highstock-1.2.2\\examples\\data-grouping\\shex.htm";
	purl=L"**\\..\\..\\Plugins\\highcharts\\Highstock-1.2.2\\examples\\data-grouping\\shex.htm";
	//purl=L"S:\\flot\\examples\\zooming.hta";
	//purl=L"S:\\flot\\examples\\image.hta";

	vwin=scriptcaller(L"show_html(arguments[0],arguments[1],$$[2])")(vax,vay,purl,int(GetConsoleWindow()));

	getchar();
	return 0;
}

