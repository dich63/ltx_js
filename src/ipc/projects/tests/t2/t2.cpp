// tem.cpp : Defines the entry point for the console application.
//
// http://msdn.microsoft.com/en-us/library/aa384182(v=vs.85).aspx

#include "stdafx.h"
//#include "OAIdl.idl"
#include "ipc_ports/com_script_engine.h"
#include "ipc_ports/class_factory_factory.h"
//#include "ipc_ports/com_apartments.h"
#include "os_utils.h"
#include "ipc_ports/ltx_js.h"
#include "ipc_ports/com_marshal_templ.h"
#include "ipc_ports/ltx_call_helper.h"
#include "ipc_ports/com_apartments_asyn.h"
#include "ipc_ports/safe_caller.h"
#include "ipc_ports/com_process.h" 
#include "ipc_ports/com_memory_region.h" 
#include "ipc_ports/ltx_js_console.h"
#include "buf_fm.h"
#include <wchar.h>
#include "Psapi.h"
#pragma comment(lib ,"psapi.lib")
#ifdef _XA64
#ifdef _DEBUG 
#pragma comment(lib ,"../../lib/x64/debug/ltx_js.lib")
#else
#pragma comment(lib ,"../../lib/x64/release/ltx_js.lib")
#endif
#else
#ifdef _DEBUG 
#pragma comment(lib ,"../../lib/Win32/debug/ltx_js.lib")
#else
#pragma comment(lib ,"../../lib/Win32/release/ltx_js.lib")
#endif

#endif

#pragma comment(lib ,"version.lib")


using namespace ipc_utils;
using namespace com_apartments;
using namespace script_engine;

HRESULT CreateTest(LPOLESTR pszDisplayName,IStream* stream,REFIID riid,void ** ppvObject);

//struct  __declspec("{250E13C9-BA83-4f8b-BA57-247A1FAF02AE}") i_ipc_serializer_t:IUnknown{};




/*
template <class T,const CLSID& _FactoryCLSID=__uuidof(T)>
struct creator_class_t
{
	enum{
		clsid=_FactoryCLSID;
	}

	inline static HRESULT CreateInstance(LPOLESTR pszDisplayName,REFIID riid,void ** ppvObject,DWORD mcxmask=0)
	{
		HRESULT hr;

		return (new T)->getInstance(riid,ppvObject,0,mcxmask,FMT_enable);
		return hr;

	}
   
};
*/
ILockBytes * plooo;
struct  __declspec(uuid("{BA35725E-7D19-4674-8981-3284E6699EC6}"))  CoDispatch_test_t:IDispatch_impl_t<CoDispatch_test_t>{ };

template <const CLSID& _CFactory ,int  FMT_enable=0>
struct IDispatch_test2_t
	:IDispatch_impl_t<IDispatch_test2_t<_CFactory,FMT_enable> >
	,base_callsback_marshal_t<IDispatch_test2_t<_CFactory,FMT_enable>,IDispatch,creator_class_t<_CFactory> >
{
	clsid_t clsid;
   CLSID& get_clsid()  { 
		return clsid;
	};

	inline HRESULT marshal_to_process(DWORD pid_target,IStream* pStm)
	{
		HRESULT hr;
		char buf[11]="0123456789";
		ULONG l;
		hr=pStm->Write(buf,10,&l);
		return S_OK;
	};  

	inline HRESULT unmarshal(IStream* pStm)
	{
		HRESULT hr;
		char buf[]="..........................";
		ULONG l;
		hr=pStm->Read(buf,10,&l);
		return S_OK;

	};  

	inline static HRESULT CreateInstance(LPOLESTR pszDisplayName,REFIID riid,void ** ppvObject,DWORD mcxmask=0)
	{
		HRESULT hr;

		 //return (new IDispatch_test2_t)->getInstance(riid,ppvObject,0,mcxmask,FMT_enable);
		if(FMT_enable) mcxmask=MAKELONG(mcxmask,1);
		hr=creator_t::CreateInstance(new IDispatch_test2_t,pszDisplayName,riid,ppvObject,mcxmask);
		return hr;

	}

	IDispatch_test2_t(CLSID c=_CFactory):clsid(c){};
};



 //typedef callback_class_factory_t<IDispatch_test_t,0>      CTestFactory2_t;
//
typedef callback_class_factory_t<IDispatch_test2_t<__uuidof(CoDispatch_test_t),1> ,1>      CTestFactory2_t;
typedef callback_class_factory_t<IDispatch_test2_t<__uuidof(CoDispatch_test_t),1> ,0>      CTestFactory21_t;



 


 DWORD dw,dwd,dwrem;
 HGLOBAL hg;
  typedef IUnknown* PUnk;
  typedef IDispatch* PDi;
  PUnk p1,p2, p3;
  PDi pd1,pd2, pd3,pdd,pdd2=0,pda;
  int iii=0;

struct  asytest
 {
	int operator()()
	{
	 HRESULT hr;
      ipc_utils::COMInitA_t ci;
	  
	  smart_ptr_t<IDispatch> ltxsrv;
	  
	  hr=GIT_t::get_instance().unwrap(dwrem,&ltxsrv.p,iii);

	  ipc_utils::smart_ptr_t<IUnknown> uc;

	  hr=GIT_t::get_instance().unwrap(dwd,&pdd2,0);

	  hr=GIT_t::get_instance().unwrap(dw,uc.address(),iii);
	  //InterlockedIncrement((volatile LONG*)&iii);
	  
	  p3=uc;
	  hr=uc.QueryInterface(&pd3);
//	  hr=ipc_utils::CoUnmarshalInterfaceFromHGlobal(hg,__uuidof(IUnknown),uc.ppQI());

   return dw;
 }
};

 struct creator_Tt
 {
	HRESULT hr;


	 HRESULT operator()(DWORD* pcookie) 
	 {
	
		 smart_ptr_t<IDispatch> engine;
		 create_dispatch_wrapper(0,0,0,0,&engine.p);
		  pda=engine;
          hr=ipc_utils::GIT_t::get_instance().wrap(engine.p,pcookie);
		  engine.detach();
		 return hr;
	 }


 }; 

 struct sss_t
 {
	 short cb;
	 char p[0];
 };
 int test_script(IDispatch* pengine=0)
 {
     ipc_utils::COMInitF_t ci;
	 ipc_utils::stopwatch_t cs;
	 ipc_utils::smart_ptr_t<IDispatch> disp;
	 bstr_t bn=L"ltx.script:debug=3";
	 bstr_t bns=L"ltx.bind:script:";
	 clsid_t clsid(bns);
	 HRESULT hr;
	 variant_t r;
	 hr=::CoGetObject(bn,0,__uuidof(IDispatch),disp.ppQI());
	 if(FAILED(hr=call_disp(disp,r.GetAddress(),0,L"process",-100)))		   FatalAppExit(0,L"Erscr");
	 double t;
	 
		 cs.Start();
	 for(int i=0;i<0;i++)
	 {
	 if(FAILED(hr=::CoGetObject(bn,0,__uuidof(IDispatch),disp.ppQI())))		   FatalAppExit(0,L"Errrrrrr");
	 //if(FAILED(hr=::CoCreateInstance(clsid,0,CLSCTX_ALL,__uuidof(IDispatch),disp.ppQI())))		   FatalAppExit(0,L"Errrrrrr");
	   //if(FAILED(hr=call_disp(disp,r.GetAddress(),0,L"b={a:$$[0]}",-100)))		   FatalAppExit(0,L"Erscr");
      
	 }
          t=cs.Sec();
	 cprintf("t=%g\n",t);
	 getchar();
return 0;

 }

#include <set>
 struct UU{
	 int k;
	 int h;
	 UU(int kk=0):k(kk){};
	 inline operator int(){return k;};

 };

 typedef struct {
	 WORD             wLength;
	 WORD             wValueLength;
	 WORD             wType;
	 WCHAR            szKey;
	 WORD             Padding1;
	 VS_FIXEDFILEINFO Value;
	 WORD             Padding2;
	 WORD             Children;
 } VS_VERSIONINFO;







 int testEC()
 {

	 ltx_js_console_t<>::send_console_input(L"$ ");
	 return 0;
	 HRESULT hr=GetLargePageMinimum();


   clsid_t clbind(L"ltx.bind");

   bstr_t bb=L"CLSID\\" +bstr_t(clbind) +bstr_t("\\InprocServer32\\"),blib,blib32;
   //bb="CLSID";

   blib=moniker_parser_t<>::reg_value_ex<64>(HKEY_CLASSES_ROOT,bstr_t(),bb,"???");
  blib32=moniker_parser_t<>::reg_value_ex<32>(HKEY_CLASSES_ROOT,bstr_t(),bb,"???");

	argv_zzs<wchar_t> argsE;
		argsE= argv_env<wchar_t>(); 

    ipc_utils::COMInitF_t ci;
	ipc_utils::smart_ptr_t<IDispatch> ecobj,ecobj2,binder,eng,outobj,dut;


	hr=CoGetObject(bstr_t("ltx.bind:"),0,__uuidof(IDispatch),binder.ppQI());

	 module_t<>::wbuf_t dllname=module_t<>::module_name(binder.p);
	 DWORD bt,bt32=SCS_32BIT_BINARY,fex,ns;
	 fex=GetBinaryType(dllname,&bt);
	 
         ns=GetFileVersionInfoSize(dllname,&(fex=0));
         
		 VS_VERSIONINFO vsv={sizeof(vsv)};
		 fex=GetFileVersionInfo(dllname,0,ns,&vsv);



	hr=CoGetObject(bstr_t("ltx.bind:ec_locker: global=3"),0,__uuidof(IDispatch),ecobj.ppQI());
	hr=CoGetObject(bstr_t("ltx.bind:ec_locker: global=1"),0,__uuidof(IDispatch),ecobj2.ppQI());
       hr=CoGetObject(bstr_t("ltx.bind:asyn"),0,__uuidof(IDispatch),eng.ppQI());
    
	hr=CoGetObject(bstr_t("ltx.bind:CoDispatch_test_t:uuu=11;b=12"),0,__uuidof(IDispatch),dut.ppQI());

	HGLOBAL hg,hgo;

	bstr_t mona,mona2;
		//hr=ipc_utils::CoMarshalInterfaceToHGlobal(new IDispatch_empty_t,&hg);
	//
	if(1)
	{

	hr=ipc_utils::CoMarshalInterfaceToHGlobal(dut,&hg);
	hr=ipc_utils::CoUnmarshalInterfaceFromHGlobal(hg,__uuidof(IDispatch),outobj._ppQI());

	//	hr=ipc_utils::CoMarshalInterfaceToHGlobal(binder,&hg);

	





	
	mona=moniker_parser_t<wchar_t>::base64_encode(hg,true);

	hgo=moniker_parser_t<wchar_t>::base64_decode(mona);

	int s1,s2;
	s1=GlobalSize(hg);
	s2=GlobalSize(hgo);
	ipc_utils::CHGlobal_t<char> gh1(hg);
	ipc_utils::CHGlobal_t<char> gh2(hgo);
	char* p1=gh1;
	char* p2=gh2;
	for (int k=0;k<gh1.sizeb();k++)
	{

		cprintf("%d] %d %d\n",k,int(p1[k]),int(p2[k]));
	}


    hr=ipc_utils::CoUnmarshalInterfaceFromHGlobal(hg,__uuidof(IDispatch),outobj._ppQI());
	hr=ipc_utils::CoUnmarshalInterfaceFromHGlobal(hgo,__uuidof(IDispatch),outobj.ppQI());
}
	//mona2=ObjRefFromHGlobal(hg);

	//
	//stub_holder sh(ecobj);
	 //mona=sh.objref();
	//mona=ObjRefFromHGlobal(hg);

	ClipboardText()=mona;


    //ComClipboard()=binder.p;
	variant_t rr;

    hr=call_disp(ecobj,"wait",&rr,1,int(-1));

     cprintf("waik ok\n"); 
     getchar();

	 return 0;
 }

#include "web_silk.h"
int testWeb()
{

	httpget_t xdr(L"ftp://dich.asuscomm.com/usb1T_1/shared_r/Video/divx_sat005.ini",L"dich",L"qcd333");

wchar_t* str=xdr;

HRESULT hr,hr2,hr3;
hr=SEC_E_CERT_EXPIRED;
hr2=ERROR_CTX_LICENSE_EXPIRED  ;
hr3=ERROR_ACCOUNT_EXPIRED;

	return 0;
}
int _tmain(int argc, _TCHAR* argv[])
{

	return testWeb();
	PROCESS_MEMORY_COUNTERS_EX pmcex={sizeof(PROCESS_MEMORY_COUNTERS_EX)};
	GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS*)&pmcex,sizeof(PROCESS_MEMORY_COUNTERS_EX));

	apartment_list_t<> aaa;

	aaa.push_handles(0,0);

	std::list< handle_array<2> > hlist;

	hlist.push_front(handle_array<2>());


	HRESULT hr;
	int szsow;


	HMODULE hke=GetModuleHandleW(L"kernel32.dll");
	HMODULE hkl=LoadLibraryW(L"ltx_js.dll");

 BOOL (WINAPI *SetConsoleIcon)(HICON);
 SET_PROC_ADDRESS(hke,SetConsoleIcon);
 HICON hico=LoadIconW(hkl,L"#104");

hr=SetConsoleIcon(hico);
	//SetConsoleIcon
wchar_t bufc[222];

	//_getws(bufc);

	 


	 
	 

      //return test_script();
wchar_t* pfulname;
{


moniker_parser_t<wchar_t> monp;

//hr=monp.expand_file_path(L"**/../../../12t.txt",&pfulname,0,0);
hr=monp.expand_file_path(L"%temp%/../../../12t.txt",&pfulname,0,0);
//hr=monp.expand_file_path(L"cs.exe",&pfulname,L"k:/;k:/0");




szsow=sizeof(WSAPROTOCOL_INFOW);

}
int szsoa=sizeof(WSAPROTOCOL_INFOA);

	buf_res_T<char> bres;
	std::strstream stre;

	bres>>stre<<'\0'<<'\0';
	int icount =stre.pcount(),icount2;



hr=sizeof(sss_t);



SOCKET sos=::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
bool f=ipc_utils::check_socket((HANDLE)sos);
hr=WSAGetLastError();



int rc=WM_MOUSEFIRST ;

class_object_pair_t* pclass_object_pair=0;

register_class_map_t<>& rcm=register_class_map_t<>::get_instance();
ipc_utils::smart_ptr_t<IUnknown> uncf,uncf2;

void* p02=	   &CTestFactory2_t::GetClassObject;
void* p12=	   &CTestFactory21_t::GetClassObject;
//hr=ltx_register_class(__uuidof(IDispatch_test_t),&CTestFactory2_t::GetClassObject,L"CTestFactory22",uncf.address());
//hr=ltx_register_class(CTestFactory2_t::GetCLSID(),&CTestFactory2_t::GetClassObject,L"CTestFactory22",uncf.address());



//IUnknown::* ppp=&uncf->AddRef;	
void*** ppp=(void***)uncf.p;

module_t<>::wbuf_t  mona=module_t<>::module_name(uncf,&hr);
 IDispatch* pdie=0;
  IUnknown	 *puu2=0;
  pdie=new IDispatch_empty_t;

  ipc_utils::smart_ptr_t<IBindCtx> bctx;
  //hr=CreateBindCtx(0,&bctx.p);
  //hr=bctx->RegisterObjectParam(bstr_t("AAA"),pdie);
  //hr=bctx->GetObjectParam(bstr_t("AAA"),&puu2);
variant_t ree,vpro;
   HWND hwnd0=com_apartments::apartment_t<>::get_com_hwnd();
	ipc_utils::COMInitF_t ci;
	IMarshal* pfmt=0,*pfmt2=0;
  ipc_utils::smart_ptr_t<IUnknown> ut;
  ipc_utils::smart_ptr_t<IDispatch> dut,deng;


 hr=CoGetObject(bstr_t("ltx.bind:CoDispatch_test_t:uuu=11;b=12"),0,__uuidof(IDispatch),dut.ppQI());
 dut.Release();


  hr=CoGetObject(bstr_t("ltx.bind:script:debug=3;appartment=0;process=1"),0,__uuidof(IDispatch),deng._ppQI());
  hr=CoGetObject(bstr_t("ltx.bind:CoDispatch_test_t:uuu=11;b=12"),0,__uuidof(IDispatch),dut.ppQI());
  hr=CoGetObject(bstr_t("ltx.bind:{BA35725E-7D19-4674-8981-3284E6699EC6}:"),0,__uuidof(IUnknown),ut._ppQI());

  HWND hwnd=com_apartments::apartment_t<>::get_com_hwnd();
 // script_engine_t sceng(L"JScript",3);

IDispatch* pppempl= new  IDispatch_empty_t;
//pppempl->Release();

HGLOBAL hgg;
   variant_t vdut(dut.p);
    //
  // hr=call_disp(deng,0,0,variant_t(L"debugger"),11,L"debugger;aa=$$[0]");
     VARIANT yy={VT_DISPATCH};
	 if(0)
	 {
     yy.pdispVal=new IDispatch_empty_RC_t<1>;
     hr=call_disp(deng,0,0,variant_t(L"dummy=$$[0];ff=0"),yy,L"debugger;aa=$$[0]");
	 hr=VariantClear(&yy);
	 }
	 
	 hr=call_disp(deng,&ree,0,variant_t(L"www={};globalEval"),11,L"debugger;aa=$$[0]");

     hr=call_disp(ree.pdispVal,0,0,variant_t(L"debugger;aa=$$[0]"),11,L"debugger;aa=$$[0]");
    //icount=ipc_utils::ref_count(ree.pdispVal);
	//hr=call_disp(deng,0,0,variant_t(L"dummy=0"),11,L"debugger;aa=$$[0]");
	//hr=call_disp(ree.pdispVal,0,0,variant_t(L";aa=$$[0];eee=globalEval"),11,L"debugger;aa=$$[0]");
	//hr=call_disp(ree.pdispVal,0,0,variant_t(L"debugger;aa=$$[0]"),11,L"debugger;aa=$$[0]");
	 icount=ipc_utils::ref_count(ree.pdispVal);
	 //hr=call_disp(deng,0,0,variant_t(L"dummy=0"),11,L"debugger;aa=$$[0]");
	 //hr=call_disp(deng,&vpro,0,variant_t(L"process"),11,L"debugger;aa=$$[0]");
	 
	 icount=ipc_utils::ref_count(ree.pdispVal);


	 //icount2=ipc_utils::ref_count(vpro.pdispVal);
	
	//deng.Release();
	
	//CoUninitialize();
	 
  deng.Release();
    //icount2=ipc_utils::ref_count(vpro.pdispVal);
	//hr=call_disp(ree.pdispVal,0,0,variant_t(L"debugger;aa=$$[0]"),11,L"debugger;aa=$$[0]");
    hr=call_disp(ree.pdispVal,0,0,variant_t(L"debugger;aa=$$[0]"),11,L"debugger;aa=$$[0]");
	
	wcharbuf perrr=error_msg(hr);
	icount=ipc_utils::ref_count(ree.pdispVal);
    hr=VariantClear(&ree);
	//getchar();
	
    //icount=ipc_utils::ref_count(ree.pdispVal);

	//
	//icount=ree.pdispVal->Release();
	//hr=VariantClear(&ree);
	
	//icount=ipc_utils::ref_count(ree.punkVal);
	
	
  //hr=call_disp(deng,0,0,variant_t(L"debugger;evena=$$[0]"),vdut);
  //ComClipboard()=dut.p;
  //getchar();
  //
  hr=CoMarshalInterfaceToHGlobal(deng,&hgg,MSHCTX_LOCAL,MSHLFLAGS_TABLESTRONG);

ipc_utils::smart_ptr_t<IDispatch> dhm;
hr=ipc_utils::CoUnmarshalInterfaceFromHGlobal(hgg,__uuidof(dhm),dhm._ppQI(),1);

hr=ipc_utils::CoUnmarshalInterfaceFromHGlobal(hgg,__uuidof(dhm),dhm._ppQI());



  //hr=ltx_register_class(__uuidof(CoDispatch_test_t),0,pclass_object_pair);
  //hr=ltx_get_object(L"CTestFactory:uuu=11",__uuidof(IUnknown),ut._ppQI());
  //hr=ltx_get_object(L"CTestFactory",__uuidof(IDispatch),dut._ppQI());

   clsid_t clss(L"ltx.bind");

   {
        bstr_t pppid=clss.prog_id();
   }

   //


	//hr=CoCreateFreeThreadedMarshaler(0,(IUnknown**)&pfmt);
	//pfmt&&(hr=pfmt->QueryInterface(__uuidof(IMarshal),(void**)&pfmt2));
	

	

	ipc_utils::smart_ptr_t<IDispatch> engine;   
	variant_t r;

	//ipc_utils::COMInit_t ci(0);
	hr=create_apartment_script_engine(engine.address());

	
	DWORD* pdw=&dw;
	apartment_t<>* apartment=new apartment_t<>(0);//COINIT_APARTMENTTHREADED);
      creator_Tt creator;
	hr=apartment->call_sync(creator,pdw,E_FAIL);

		ipc_utils::smart_ptr_t<IDispatch>  dit,diiii;
	   hr=ipc_utils::GIT_t::get_instance().unwrap(dw,&dit.p,0);
	   pdie=dit;
	//pdd=pdie=new IDispatch_empty_t;

	create_dispatch_wrapper(0,0,0,0,&pdie);
	pdd=pdie;
	
	hr=GIT_t::get_instance().wrap(pdd,&dwd);
	

	
	  
	
	
   //hr=CreateTest(0,0,__uuidof(IUnknown),ut.ppQI());
	//	hr=IDispatch_test_t::CreateInstance(0,__uuidof(IUnknown),ut.ppQI());	

	//hr=CTestFactory2_t::GetObjectEx(0,0,0,__uuidof(IUnknown),ut.ppQI());


   module_t<>::wbuf_t  monaut=module_t<>::module_name(ut,&hr);
   p1=ut;
   hr=ut.QueryInterface(&pd1);
   //DWORD dw;
   hr=GIT_t::get_instance().wrap(pd1,&dw);

//   hr=GIT_t::get_instance().unwrap(dw,&diiii.p);
  // hr=ipc_utils::CoMarshalInterfaceToHGlobal(ut,&hg,MSHCTX_INPROC);
    
//dwrem   
   //hr=CoCreateInstance()
   smart_ptr_t<IDispatch> ltxsrv;
   hr=ltxsrv.CoCreateInstance(L"LifeTimeXsrv.ltxmonsrv");
   hr=GIT_t::get_instance().wrap(ltxsrv.p,&dwrem);
 
int ii;
ii=asyn_call(new asytest,-2);
ii=asyn_call(new asytest,-2);



ipc_utils::smart_ptr_t<IUnknown> uc;
hr=GIT_t::get_instance().unwrap(dw,uc.address(),1);
p2=uc;
//hr=uc.QueryInterface(&pd2);
bstr_t bss;
if(1)
{


hr=GetObjectRefString(ut,bss);
stub_holder sh(dut);
bss=sh.objref();
ClipboardText()=bss;
}
ComClipboard()=dut.p;
getchar();
error_msg();
	return 0;
}

