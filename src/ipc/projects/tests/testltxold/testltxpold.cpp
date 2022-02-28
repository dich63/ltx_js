// tem.cpp : Defines the entry point for the console application.
//
// http://msdn.microsoft.com/en-us/library/aa384182(v=vs.85).aspx

#include "stdafx.h"
#include <conio.h>
#include "ipc_ports/dispatch_call_helpers.h"
#include "resource.h"
#include <vector>
#include <math.h>
#include "wchar_parsers.h"
//#include "OAIdl.idl"
using namespace ipc_utils;


typedef ltx_helper::arguments_t<variant_t>  arguments_t;
extern "C"  void __cdecl  print_console
( variant_t* presult, void* pctx,pcallback_context_arguments_t pcca,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
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




#undef GetObject
int _tmain(int argc, _TCHAR* argv[])
{

	argv_zzs<wchar_t> a1,a2 ,aa;

	a1[L"AA"]=L"A1";
	a2[L"AA"]=L"A2";
	a1.parent=&a2;
	aa=a1;


	COMInitF_t ci;
	HRESULT hr,hrEx,hrCC;
	GetProcAddress(0,"procAAA");
	hr=GetLastError();

	smart_ptr_t<IDispatch> externalLTXOld,ec_locker;
	variant_t callback_print,extV;

     hr=ec_locker.CoGetObject("ltx.bind:ec_locker:  global=3 ;");

	hrEx=hr=::CoGetObject(bstr_t("ltx.bind:old_style.external: debug=0"),0,__uuidof(IDispatch),externalLTXOld.ppQI());
	cprintf("ltx.bind:old_style.external: status=0x%x \"%s\" \n",hr,(char*)exceptinfo_t::error_msg(hr));

	if(SUCCEEDED(hr)) extV=externalLTXOld;


	/*
	smart_ptr_t<ICallbackDispatchFactory> callbackFactory;

	hr=::CoGetObject(bstr_t("ltx.bind:callback.factory"),0,__uuidof(ICallbackDispatchFactory),callbackFactory.ppQI());
	cprintf("ltx.bind:callback.factory: status=0x%x \"%s\" \n",hr,(char*)exceptinfo_t::error_msg(hr));
	if(SUCCEEDED(hr))
	{
	hrCC=hr=callbackFactory->CreateInstanceV(0,&print_console,0,0,callback_print.GetAddress());
	cprintf("ltx.bind:callback.CreateInstanceV: status=0x%x \"%s\" \n",hr,(char*)exceptinfo_t::error_msg(hr));

	}
	*/

	// test 1;

	hr=(new print_obj_t)->wrap(callback_print.GetAddress(),MAKELONG(FLAG_MASK_PUT,FLAG_CBD_EC));


	cprintf("wrap: status=0x%x \"%s\" \n",hr,(char*)exceptinfo_t::error_msg(hr));


	scriptcaller_t scriptcaller(L"ltx.bind:script:debug=3 ;");


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

	hr=(new binary_array_t<double>(NA))->wrap(array_disp.GetAddress(),1*(FLAG_CBD_FTM<<16));
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
	vwin=scriptcaller(L"show_html(arguments[0],arguments[1],$$[2])")(vax,vay,purl);

	getchar();
	return 0;
}

