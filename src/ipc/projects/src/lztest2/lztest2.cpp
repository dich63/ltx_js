// lztest2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ipc_ports/com_apartments.h"

#include "ipc_ports/com_script_engine.h"
#include "os_utils.h"
#include "ltx_proc_table.h"
#include "ipc_ports/reg_scripts.h"

//#include "ipc_ports/class_factory_factory.h"

#include "export_tools.h"

 #include "call_functor_reverse.h"
console_shared_data_t<wchar_t,0>  g_csd;
using namespace ipc_utils;
using namespace com_apartments;
using namespace script_engine;

PIMAGE_NT_HEADERS ppopop;

void testGIT(DWORD dw)
{
	HRESULT hr;
   double t;
   CStopwatch cs;
   cs.Start();
   for(int i=0;i<1000;i++)
   {
	   variant_t vdisp,r;
	   if(FAILED(hr=GIT_t::get_instance().unwrap(dw,&vdisp,0)))
		   break	    ;
   }
   t=cs.Sec(); 
   cprintf("t=%g\n",t);
 
}


void test_appart(DWORD dw=0)
{
	HRESULT hr;
	double t;
	CStopwatch cs;
	cs.Start();
	double N=100;
	for(int i=0;i<N;i++)
	{
		apartment_t<>* ap=new apartment_t<>(dw);
	}
	t=cs.Sec(); 
	cprintf("t=%g t0=%g Ms  \n",t,1000.0*t/N);

	getchar();

}







int testdisp(IUnknown* endisp)
{
	HRESULT hr;
	script_engine_t*pengine=new script_engine_t;
	smart_ptr_t<IDispatch> disp;

	create_dispatch_wrapper(&script_engine_t::global_eval,pengine,0,0,&disp.p);

	script_engine_t& engine=*pengine;


	


	variant_t vdisp,r;
	//hr=GIT_t::get_instance().unwrap(dw,&vdisp);
	hr=toVARIANT(endisp,&vdisp);

	hr=engine.call(L"QQE=111;script=$$[0]",0,1,&vdisp);
	hr=engine.call(L"script($$[0])",&r,1,&variant_t(L"zzz=11"));
	DWORD dwr;
	hr=GIT_t::get_instance().wrap(disp.p,&dwr);
	return dwr;

}

int testap(DWORD dw)
{
	HRESULT hr;
	script_engine_t*pengine=new script_engine_t;
	smart_ptr_t<IDispatch> disp;

	create_dispatch_wrapper(&script_engine_t::global_eval,pengine,0,0,&disp.p);

	script_engine_t& engine=*pengine;


    testGIT(dw);


	variant_t vdisp,r;
	hr=GIT_t::get_instance().unwrap(dw,&vdisp);

	hr=engine.call(L"QQE=111;script=$$[0]",0,1,&vdisp);
	hr=engine.call(L"script($$[0])",&r,1,&variant_t(L"zzz=11"));
	DWORD dwr;
	hr=GIT_t::get_instance().wrap(disp.p,&dwr);
 return dwr;

}
__declspec(dllexport)
struct AA
{
	static void uuu(){};
	double v;
	

	int 	operator()(int a){  return a; }
};

struct BB:AA
{
	BB(double r=0){v=r;};
   ~BB()
   {

   }
};

typedef _com_error com_exception;

struct dispcaller_t:invoker_base_t<dispcaller_t,variant_t,variant_t,VARIANT>
{
	HRESULT hr;
	dispcaller_t(IDispatch* pdisp,DISPID id=DISPID_VALUE):disp(pdisp),dispid(id){};
inline 	res_t invoke_proc(int argc,argh_t* argv)
{
	DISPPARAMS dps={argv,0,argc,0};
	WORD wFlags=(argc)? DISPATCH_METHOD:DISPATCH_METHOD|DISPATCH_PROPERTYGET;
	res_t r;
	hr=disp->Invoke(dispid,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,&r,0,0);
	return r;

} 
  
   smart_ptr_t<IDispatch> disp;
   DISPID dispid;
};

struct dispcaller_plus_t:invoker_base_t<dispcaller_plus_t,variant_t,variant_t,VARIANT>
{
	
	dispcaller_plus_t(IDispatch* pdisp,BSTR txt,HRESULT* _phr):disp(pdisp),stxt(txt),fcomplete(false),phr(_phr){};
	inline 	res_t invoke_proc(int argc,argh_t* argv)
	{
		fcomplete=true;
        argv[argc].vt=VT_BSTR;
        argv[argc].bstrVal=stxt;
		DISPPARAMS dps={argv,0,argc+1,0};
		WORD wFlags=(argc)? DISPATCH_METHOD:DISPATCH_METHOD|DISPATCH_PROPERTYGET;
		res_t r;
		*phr=disp->Invoke(DISPID_VALUE,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,&r,0,0);
		return r;

	} 

	inline dispcaller_plus_t& operator<<(bstr_t txt) 
	{
		if(!buf.length()) buf=stxt;
		buf+=L"\n"+txt;
		stxt=buf;
		return *this;
	}


	inline variant_t invoke_proc()
	{
		VARIANT tmp={}; 
		return  invoke_proc(0,(argh_t*)&tmp);
	}

	inline operator variant_t(){ 
		return invoke_proc();
	}
	template <class N>
	inline operator N(){ 
		return (N)invoke_proc();
	}

	dispcaller_plus_t(dispcaller_plus_t& d)
	{
		disp=d.disp;
		stxt=d.stxt;
		fcomplete=d.fcomplete;
		phr=d.phr;
		buf=d.buf;
		d.fcomplete=true;
	}
    ~dispcaller_plus_t()
	{
		
		if(!fcomplete) invoke_proc();
	}

	IDispatch* disp;
	BSTR stxt;
	bool fcomplete;
	HRESULT* phr;
	bstr_t buf;
	
};


struct delay_t
{ 



 inline variant_t operator()(bstr_t stxt) 
 {
	 argv[argc].vt=VT_BSTR;
	 argv[argc].bstrVal=stxt;
	 DISPPARAMS dps={argv,0,argc+1,0};
	 WORD wFlags=(argc)? DISPATCH_METHOD:DISPATCH_METHOD|DISPATCH_PROPERTYGET;
	 variant_t r;
	 *phr=disp->Invoke(DISPID_VALUE,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,&r,0,0);
	 return r;

 }

inline variant_t operator<<(bstr_t stxt) 
{
	return this->operator()(stxt);
}

 delay_t(){};
 delay_t(int c,VARIANT* pv,IDispatch* p,HRESULT* _phr):argc(c),disp(p),phr(_phr)
 {
          for(int k=0;k<argc;k++) argv[k]=pv[k];
 };
 int argc;
 VARIANT argv[32];
 IDispatch* disp;
 HRESULT* phr;
};

struct dispcaller_delay_t:invoker_base_t<dispcaller_delay_t,variant_t,delay_t>
{

	dispcaller_delay_t(IDispatch* pdisp,HRESULT* _phr):disp(pdisp),phr(_phr){};
	inline 	delay_t invoke_proc(int argc,argh_t* argv)
	{
		//VARIANT vargv[32];
		//for(int k=0;k<argc;k++) vargv[k]=argv[k];
		//delay_t d={argc,vargv,disp,phr};
		return delay_t(argc,argv,disp,phr);
	} 

	
	
	IDispatch* disp;	
	HRESULT* phr;

};



#define __Lpn(a) L###a
#define _Lpn(...) __Lpn(__VA_ARGS__)

#define js_text(...) __Lpn(__VA_ARGS__)
#define js_script(...) operator()(__Lpn(__VA_ARGS__))

template <class _Variant>
struct result_t
{
	typedef _Variant var_t;
	HRESULT hri;  
	const var_t& v;  
	result_t(const  var_t& _v,HRESULT hr):v(_v),hri(hr){};

	operator  const  var_t&() const 
	{
		return v;
	}
	template <class N>
	operator N() 
	{
		return v;
	}

	inline 	bool is_empty()
	{
		VARTYPE t=v.vt;
		return (t==VT_NULL)||(t==VT_EMPTY)||(t==VT_ERROR);
	}


	template <class N>
	N def(N dv=N()) {
		try     {
			return (!is_empty())?v:dv;
		}
		catch(...){
		}
		return dv;
	}

	template<class Intf>
	HRESULT QueryInterface(Intf** ppObj)
	{
		HRESULT hr;
		return hr=QueryInterface(__uuidof(Intf),(void**)ppObj); 
	}

	HRESULT QueryInterface(REFIID riid, void** ppObj)
	{
		HRESULT hr;
		var_t tmp;
		if(SUCCEEDED(hr=VariantChangeType(&tmp,&v,0,VT_UNKNOWN))) 
		{ 
			hr=tmp.punkVal->QueryInterface(riid,ppObj);
		}
		return hr; 
	}

};

/*

struct dispcaller_plus2_t:invoker_base_t<dispcaller_plus2_t,result_t<variant_t>,variant_t,VARIANT>
{

	dispcaller_plus2_t(IDispatch* pdisp,BSTR txt,HRESULT* _phr):disp(pdisp),stxt(txt),fcomplete(false),phr(_phr){};
	inline 	res_t invoke_proc(int argc,argh_t* argv)
	{
		fcomplete=true;
		argv[argc].vt=VT_BSTR;
		argv[argc].bstrVal=stxt;
		DISPPARAMS dps={argv,0,argc+1,0};
		WORD wFlags=(argc)? DISPATCH_METHOD:DISPATCH_METHOD|DISPATCH_PROPERTYGET;
		
		*phr=disp->Invoke(DISPID_VALUE,IID_NULL, LOCALE_USER_DEFAULT,wFlags,&dps,&result,0,0);
		return res_t(result,*phr);

	} 

	inline dispcaller_plus2_t& operator<<(bstr_t txt) 
	{
		if(!buf.length()) buf=stxt;
		buf+=L"\n"+txt;
		stxt=buf;
		return *this;
	}


	inline res_t invoke_proc()
	{
		VARIANT tmp={}; 
		return  invoke_proc(0,(argh_t*)&tmp);
	}

	inline operator res_t(){ 
		return invoke_proc();
	}
	dispcaller_plus2_t(dispcaller_plus2_t& d)
	{
		disp=d.disp;
		stxt=d.stxt;
		fcomplete=d.fcomplete;
		phr=d.phr;
		buf=d.buf;
		d.fcomplete=true;
	}
	~dispcaller_plus2_t()
	{

		if(!fcomplete) invoke_proc();
	}

	IDispatch* disp;
	BSTR stxt;
	bool fcomplete;
	HRESULT* phr;
	bstr_t buf;
	arg_t result;

};



struct scriptcaller2_t
{
	HRESULT hr;
	scriptcaller2_t(IDispatch* pdisp):disp(pdisp){};

	inline dispcaller_plus2_t operator()(bstr_t parse_script)
	{

		return dispcaller_plus2_t(disp.p,parse_script,&hr);

	}

	inline dispcaller_plus2_t operator<<(bstr_t parse_script) 
	{
		return dispcaller_plus2_t(disp.p,parse_script,&hr);
	}


//	dispcaller_delay_t set_args;
	smart_ptr_t<IDispatch> disp;
};

*/
struct scriptcaller_t
{
   HRESULT hr;
   scriptcaller_t(IDispatch* pdisp):disp(pdisp),set_args(pdisp,0){
      set_args.phr=&hr;
   };

   inline dispcaller_plus_t operator()(bstr_t parse_script)
   {

	   return dispcaller_plus_t(disp.p,parse_script,&hr);

   }

   inline dispcaller_plus_t operator<<(bstr_t parse_script) 
   {
	   return dispcaller_plus_t(disp.p,parse_script,&hr);
   }


  dispcaller_delay_t set_args;
  smart_ptr_t<IDispatch> disp;
};


extern "C"  void __cdecl  mtaproc
( variant_t* presult, void* pltx,void* error,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
{
	*presult=argc;
}

struct creatormta_t
{
	DWORD dw;
HRESULT	operator()(void* p)
{
	HRESULT hr;
	VARIANT v=create_dispatch_wrapper(&mtaproc,(void*)0,0,0);
	hr=GIT_t::get_instance().wrap(v.pdispVal,&dw,1);
	return hr;
}

};


void testmta()
{
	creatormta_t creator;
		HRESULT hr;
global_com_t::get_instance().MTA();
apartment_t<> app(0);
void* p;
 COMInit_t ci(COINIT_APARTMENTTHREADED);
 smart_ptr_t<IDispatch> disp;
 variant_t r;
 hr=app.call_sync(creator,p,E_FAIL);

hr=GIT_t::get_instance().unwrap(creator.dw,disp.address(),1);
hr=call_disp(disp,r.GetAddress(),0);
app.close();
hr=call_disp(disp,r.GetAddress(),0);


}

inline HRESULT create_dummy_ObjectWithSite(IUnknown** ppunk)
{
	struct F:IUnknown_impl<F,IObjectWithSite>{
		~F()
		{

		};
		virtual HRESULT STDMETHODCALLTYPE SetSite( 
			/* [in] */ __RPC__in_opt IUnknown *pUnkSite)
		{
			return  E_NOTIMPL;
		};

		virtual HRESULT STDMETHODCALLTYPE GetSite( 
			/* [in] */ __RPC__in REFIID riid,
			/* [iid_is][out] */ __RPC__deref_out_opt void **ppvSite) 
		{
			return  E_NOTIMPL;
		}

	};

	if(!ppunk) return E_POINTER;
	*ppunk=new F;
	return S_OK;
}


extern "C"  void __cdecl  external_connect
( variant_t* presult, void* pltx,void* error,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
{

	*presult=argc;
}


extern "C"  void __cdecl  external_connect2
( variant_t* presult, void* pltx,void* error,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
{

	*presult=argc;
}


extern "C"  void __cdecl  on_event
( variant_t* presult, void* pltx,void* error,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
{
		g_csd.printf(M_CC,L"on_event!!!=%s\n",argv[0].bstrVal);
		return;
	smart_ptr_t<IObjectWithSite> site;
    hr=argv->punkVal->QueryInterface(__uuidof(IObjectWithSite),(void**)&site.p);

	*presult=argc;
}
extern "C"  void __cdecl  asyntest
( variant_t& result, void* pltx,void* error,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
{
	SleepEx(500,1);
	result=L"JJJJJJJJJJAAA!";
	return;
	smart_ptr_t<IUnknown> site;
	create_dummy_ObjectWithSite(&site.p);
	hr=toVARIANT(site.p,&result);
}




HRESULT testasyn()
{   
	HRESULT hr;
int rc;
	ipc_utils::stopwatch_t cs;
	//hr=asyn_object_t::CreateInstance(disp,evnt,&asyn.p);
	cs.Start();

	smart_ptr_t<IDispatch> disp,evnt,asyn;
	__int64 lp=__int64(&cs);
   create_dispatch_wrapper(&asyntest,0,0,0,&disp.p);
   create_dispatch_wrapper(&on_event,0,0,0,&evnt.p);
   
   double t;
   



   hr=asyn_object_t::CreateInstance(disp,evnt,&asyn.p);
   

   disp.Release();
   evnt.Release();

   hr=call_disp(asyn,0,0,lp,22,33.444);
   rc=refcount(asyn);

   smart_ptr_t<IObjectWithSite> os;
   hr=asyn.QueryInterface(os.address());
   //variant_t dispevent(create_dispatch_wrapper(&external_connect2,(void*)0,0,0,0,3,1),false);
   VARIANT dispevent=(create_dispatch_wrapper(&external_connect2,(void*)0,0,0,0,3,1));
   if(SUCCEEDED(hr)) hr=os->SetSite(dispevent.punkVal);

   ComClipboard()=asyn.p;

   ComClipboard()=dispevent;

   rc=refcount(asyn);

   //asyn.detach();
   IUnknown * pau=asyn;
   os.Release();
   asyn.Release();
   rc=refcount(pau);

   //cprintf(":");
   t=cs.Sec();
   //
   g_csd.printf(M_CC,L"t=%g\n",t);
   //cprintf("t=%g\n",t);
 //loop();
  return hr;
}




struct diditest_t:invoker_base_t<diditest_t,BB,int,AA>
{
inline 	int invoke_proc(int argc,argh_t* argv){
	return argc;
} 
};
IObjectWithSite* ioioiooi;
IPersistStreamInit* ipiui;
IPersistStream* ipii;


int testCoInit()
{
	HRESULT hr;
	{
		ipc_utils::smart_ptr_t<IDispatch> engine;   
		variant_t r;
		
		//ipc_utils::COMInit_t ci(0);
	    hr=create_apartment_script_engine(engine.address());

		hr=call_disp(engine,&r,0,variant_t(L"hh=111+$$[0]"),99.777);

		scriptcaller_t scriptcaller(engine);


		scriptcaller("zzz=7111");

		double yyy,rr;
		yyy=scriptcaller("zzz");

		rr=scriptcaller("ee=$$[0]*$$[1]")(3,111);
		rr=scriptcaller("ee=-ee")();
		rr=scriptcaller("ee=100*ee")();
	
	}




	return 0;
}

//using namespace ipc_utils;
int inner_inv(int n,int*)
{
	return n;
}

//_MCALL_1_n(3,int atestAAa ,inner_inv,int ,int);
//_MCALL_s_1(int atestAAa ,inner_inv,int ,int);

//_MCALL_s_3
//:invoker_base_t<diditest_t,BB,int,AA>
#define OPERATOR int operator()
struct didAtest_t
{
	inline 	int invoke_proc(int argc,int* argv){
		return argc;
	} 

	MACRO_CALL_s_n(5,OPERATOR ,invoke_proc,int ,int);
};


MACRO_CALL_s_n(5,int atestAAa ,inner_inv,int ,int);

//#define @jztext jztext

bstr_t bbq;
//#include "js\zz_parsers.js"
;
int _tmain(int argc, _TCHAR* argv[])
{

   argv_zz<char> bazz("aa=11\0hh.bb=22\0g=kkkk\0\0");
   argv_zz<char> aazz("aa=11;hh.bb=22;g=kkkk",";");


	HRESULT hr;
    int rew;
	
hr=com_register_t<>::dll_class_register(bstr_t("{61F8A798-2A81-4d98-B3C5-543E558B1468}"),L"both",L"ltx.script");
	diditest_t didAtest;
	//int yyr=didAtest(BB(1),BB(2),BB(3));
	rew=didAtest(1,2,3);
	rew=atestAAa();
	rew=atestAAa(1);
	rew=atestAAa(1,2);
	rew=atestAAa(1,2,3);
	return testCoInit();

void*	procc=&GetProcAddress;
//procc=&AA::uuu;

	const char* ppp=proc_name(procc);
	chbuf chb=lib_name(procc);
	WORD ord=proc_ordinal(procc);




	
GUID ggg= GUID_NULL;


//hr=com_register_t<>().dll_class_register(__uuidof(CLSID_kox_marshal_t),L"both");

//return 0;

  //variant_t vvG(ggg);
    hr=script_execute("debugger;a=$$.def(1,-777);b=$$.def(0,-777);zz=$$.len",0,L"AAAAAAAAAA");   

	variant_t  verr(long(0),VT_ERROR),reda;
	{
       COMInit_t cddi(COINIT_APARTMENTTHREADED);
	  //hr=script_execute(" (new ActiveXObject('wscript.shell')).Popup('aaaaaaaaaaaaaa',-1,'assdd');");   
	}


	hr=script_execute("ss='sssssssssssssssA'",&reda);   


	clsid_t ccc=__uuidof(CLSID_bind_marshal_t);//CLSID_StdGlobalInterfaceTable;
	bstr_t bcc=ccc;

	clsid_t cccc=bcc;



	//int ieerr=verr;

	diditest_t diditest;
	int yyr=diditest(BB(1),BB(2),BB(3));
	yyr=MAX_PATH;

		COMInit_t cddi(0*COINIT_APARTMENTTHREADED);
     testasyn();
	 testasyn();
	 testasyn();
	 //getchar();
	 //
	 while(!loop()){};

	smart_ptr_t<IDispatch> ddd;
	std::queue< smart_ptr_t<IDispatch> > dilist;

 dilist.push((IDispatch*)0);
 dilist.push(ddd);

 dilist.push(smart_ptr_t<IDispatch>());

// test_appart(2);
exceptinfo_t eeei;

 eeei=EXCEPINFO();
testmta();

	BB bb;
	int uuweu=bb(777);

	GUID gg=__uuidof(IDispatch),gg2=__uuidof(iaggregator_helper);

	bool rf=gg==gg2;

	std::map<int,double> mmma;


	std::map<int,double>::iterator ii;

	std::pair<std::map<int,double>::iterator,bool> pp=mmma.insert(std::make_pair(77,999.22));
	ii=pp.first;

	double ew=ii->first;
	double qqw=ii->second;





	std::list<int>  lii;
	std::list<int>::iterator ili;

	lii.push_front(11);
     ili=lii.begin();

 	



	COMInit_t ci(COINIT_APARTMENTTHREADED);
	

     smart_ptr_t<IDispatch> dengine;
	 VARIANT ath,dispevent;
	 variant_t rr;
	dispevent=create_dispatch_wrapper(&external_connect,(void*)0,0,0,0,3,1);
//create_apartment_script_engine(IDispatch** ppdisp ,IDispatch* exc_event=0,wchar_t* lang=L"JScript",int fdebug=3)
	hr=create_apartment_script_engine( &dengine.p,dispevent.pdispVal);

	HGLOBAL hmm;
	hr=CoMarshalInterfaceToHGlobal(dengine.p,&hmm);
	hr=CoReleaseCOMStub(hmm,1);

     //dengine.reset();
	//getchar();

     //hr=call_disp(dengine,&rr,0,variant_t(L"aa=11"));


		apartment_t<> ap(2),ap00(0);

	rr=dispcaller_t(dengine)(L"aa=11");


        scriptcaller_t scriptcaller(dengine);


		scriptcaller("zzz=7111");

		int yyy=scriptcaller("zzz");

		rr=scriptcaller("ee=$$[0]*$$[1]")(3,111);
		rr=scriptcaller("ee=-ee")();
		rr=scriptcaller("ee=100*ee")();

		//rr=(scriptcaller<<js_text(ee=$$[0]*$$[1]) )(3,111);


scriptcaller<< js_text(

		sss="AAAAAAAAAAAAAAAA"
			)
<<js_text(  ee=7777;  )
<<js_text(  ese=-107777;  );

VARIANT vvee={VT_NULL};
//rr=scriptcaller.compile(3,111).operator()("aaa=11"); 
rr=scriptcaller.set_args(3,111,vvee)<< js_text(

      debugger;
    oo={};\n
     oo.a=$$[2];\n

	  ww=$$[0]*$$[1];\n

	  var aa=[];
	  for(var kk=0;kk<10;kk++) aa[kk]=kk;

       aa =11+ww
);



	      //ComClipboard()=dengine.p;

	   HGLOBAL hm;
	   hr=CoMarshalInterfaceToHGlobal(dengine.p,&hm);//,MSHCTX_INPROC);
	   smart_ptr_t<IUnknown> pup;
	   hr=ipc_utils::CoUnmarshalInterfaceFromHGlobal(hm,__uuidof(IUnknown),(void**)&pup.p);
       hr=CoReleaseCOMStub(hm,1);
	   pup.reset();
       dengine.reset();

    getchar();

      script_engine_t engine;

     smart_ptr_t<iaggregator_container> aggc;
	 smart_ptr_t<IUnknown> uuu,ftm0;
      smart_ptr_t<IMarshal> mama;
	hr= create_iaggregator_container(&aggc.p);
	hr=create_FTM_aggregator_helper(&ftm0.p);
    hr=aggc->SetAggregator(__uuidof(IMarshal),ftm0);

	hr=aggc.QueryInterface(&mama.p);

	hr=engine.call("o={};o.a=$$[0];debugger",0,1,&variant_t());
	smart_ptr_t<IDispatch> disp;


	smart_ptr_t<IUnknown> ftm,dispgit,extconn;
	

	//hr=create_FTM_aggregator_helper(&ftm.p);
	//	hr=create_ExternalConnection_aggregator_helper(dispevent.pdispVal,&ftm.p);
	//hr=create_FMTEC_aggregator_helper(dispevent.pdispVal,&ftm.p);
	
//        create_dispatch_wrapper_aggr(ftm,&apptype,(void*)0,(void*)0,(void*)0);
	//create_dispatch_wrapper_aggr(ftm,&script_engine_t::global_eval,&engine,0,0,&disp.p);

	create_dispatch_wrapper(&script_engine_t::global_eval,&engine,0,0,&disp.p);

	hr=create_GITDispatch_aggregator_helper(disp,&dispgit.p);
	hr=create_ExternalConnection_aggregator_helper(dispevent.pdispVal,&extconn.p);
	hr=aggc->SetAggregator(__uuidof(IDispatch),dispgit);
	hr=aggc->SetAggregator(__uuidof(IExternalConnection),extconn);

	//hr=aggc->SetAggregator(__uuidof(IDispatch),ftm0);
   variant_t vdisp2,r;

	hr=engine.call(L"ee=Error('aasas');",r.GetAddress());

        smart_ptr_t<IErrorInfo> ei;

		hr=r.pdispVal->QueryInterface(__uuidof(IErrorInfo),ei.ppQI());

   
	DWORD dw,dwe;
    
	hr=GIT_t::get_instance().wrap(disp.p,&dw);


	




   // loop();  
   //dwe=ap.call_sync(testap,dw,int(-1),1);
	dwe=ap.call_sync(testdisp,aggc.p,int(-1),1);

     ComClipboard()=aggc.p;

	// ComClipboard()=r;

	 while(!loop()){};
   hr=GIT_t::get_instance().unwrap(dwe,&vdisp2);
   hr=engine.call(L"script=$$[0]",0,1,&vdisp2);
   hr=engine.call(L"script($$[0])",r.GetAddress(),1,&variant_t(L"debugger;zzz=11"));

   

   
   




	thread_handle_t th(GetCurrentThread());

      void* peep(argv);

	return 0;
}

