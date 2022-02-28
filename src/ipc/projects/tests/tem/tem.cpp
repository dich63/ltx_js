// tem.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "OAIdl.idl"
#include "ipc_ports/com_script_engine.h"
#include "ipc_ports/class_factory_factory.h"
//#include "ipc_ports/com_apartments.h"
#include "os_utils.h"
#include "ipc_ports/ltx_js.h"
#include "ipc_ports/com_marshal_templ.h"
#include "buf_fm.h"


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


int dieex = DISP_E_EXCEPTION;
IDispatchEx* pdieex  ;

using namespace ipc_utils;
using namespace com_apartments;
using namespace script_engine;

HRESULT CreateTest(LPOLESTR pszDisplayName,IStream* stream,REFIID riid,void ** ppvObject);

//struct  __declspec("{250E13C9-BA83-4f8b-BA57-247A1FAF02AE}") i_ipc_serializer_t:IUnknown{};











struct  __declspec(uuid("{B035725E-7D19-4674-8981-3284E6699EC6}")) CTestFactory_t:class_factory_base_aggregator_t<CTestFactory_t>
{
	inline HRESULT impl_CreateInstanceEx(	IUnknown * pUnkOuter,LPOLESTR pszDisplayName,IStream* stream,REFIID riid,void ** ppvObject)	
	{
		HRESULT hr;
		moniker_parser_t<wchar_t> mp(pszDisplayName);
		hr=CreateTest(pszDisplayName,stream,riid, ppvObject);
		return hr;

	}

	static HRESULT __stdcall GetClassObject2(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
	{
		return GetClassObject(rclsid,riid,  ppv);
	}

};


struct  __declspec(uuid("{BA35725E-7D19-4674-8981-3284E6699EC6}"))  IDispatch_test_t:IDispatch_impl_t<IDispatch_test_t>
{
       
	persist_stream_aggregator_t<IDispatch_test_t> persist_stream;
	clsid_t clsid;

	inline CLSID& get_clsid(){ 
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
    
	bool inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr)
	{
		return persist_stream.inner_QI(Outer,riid,ppObj,hr)||__super::inner_QI(Outer,riid,ppObj,hr);
	}
	IDispatch_test_t(clsid_t _clsid=__uuidof(IDispatch_test_t)):clsid(_clsid)
	{
		persist_stream.owner=this;
	}

	inline static HRESULT CreateInstance(LPOLESTR pszDisplayName,REFIID riid,void ** ppvObject,DWORD mcxmask=0,bool freemarshal=0)
	{
		struct F:aggregator_helper_base_t<F>
		{

			inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ){

				return marshal(Outer,riid,ppObj,hr);
			}
			F(DWORD m):marshal(m){}

			marshal_callback_t<> marshal;
		};

		HRESULT hr;
		ipc_utils::smart_ptr_t<F> sF(new F(mcxmask),0);

		ipc_utils::smart_ptr_t<iaggregator_helper> sD=aggregator_helper_creator_t<IDispatch_test_t>::New(new  IDispatch_test_t);
		int ic=ipc_utils::ref_count(sD.p);
		aggregator_container_helper container;
		if(freemarshal)
			container.add_FMT(true);
		hr=container.add(__uuidof(IMarshal),sF).add(__uuidof(IDispatch),sD).add(__uuidof(IPersistStream),sD).QueryInterface(riid,ppvObject);        
		return hr;

	}
};


struct interface_data_t
{
  
   void* pcontext;
   CLSID clsid;

   HRESULT (*addAggregateInterfaces)(interface_data_t* p,IUnknown* pcontainer,IUnknown* paggregator);
   HRESULT (*marshalToProcess)(interface_data_t* p,DWORD pid_target,IStream* pStm);
   HRESULT (*unmarshal)(interface_data_t*p,DWORD pid_target,IStream* pStm);
  // HRESULT (createInstance)(interface_data_t*p,const wchar_t* pszDisplayName,REFIID riid,void ** ppvObject);

};
/*
template <const CLSID& _CFactory ,int  FMT_enable=0>
struct IDispatch_test2_t
	:IDispatch_impl_t<IDispatch_test2_t<_CFactory,FMT_enable> >
	,base_callsback_marshal_t<IDispatch_test2_t<_CFactory,FMT_enable> ,IDispatch>
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

		 return (new IDispatch_test2_t)->getInstance(riid,ppvObject,0,mcxmask,FMT_enable);
		return hr;

	}

	IDispatch_test2_t(CLSID c=_CFactory):clsid(c){};
};
*/


 //typedef callback_class_factory_t<IDispatch_test_t,0>      CTestFactory2_t;
//
typedef callback_class_factory_t<IDispatch_test2_t<__uuidof(IDispatch_test_t),0> ,0>      CTestFactory2_t;
typedef callback_class_factory_t<IDispatch_test2_t<__uuidof(IDispatch_test_t),1> ,0>      CTestFactory21_t;

struct marshal_test_t:marshal_base_aggregator_t<marshal_test_t>
{
	inline  HRESULT IMarshal_GetUnmarshalClass( REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags, CLSID *pCid){

		//	return S_OK;
		//ipc_utils::clsid_t clsid(L"JScript");
		IUnknown* ppUU=GetUnknown();
		*pCid=__uuidof(CLSID_marshal_factory_t);
		return S_OK;

	};


	inline HRESULT IMarshal_GetMarshalSizeMax(  REFIID riid, void *pv, DWORD dwDestCtx, void *pvDestCtx,DWORD mshlflags, DWORD *pSize){

			//*pSize=1024;
		 	   HRESULT hr=S_OK;
                *pSize=0;
			   return hr;

			   
		  
			   hr=get_custom_marshaller_size(1,riid,pv, dwDestCtx,pvDestCtx,mshlflags,pSize);
			   if(SUCCEEDED(hr))
				   //*pSize+=sizeof(marshal_header_t<CLSID_std_marshal_t>);
				   *pSize+=1024;

		//   hr=CoGetMarshalSizeMax(pSize,riid,(IUnknown*) pv,dwDestCtx,pvDestCtx,mshlflags);
			   /*
			   ipc_utils::smart_ptr_t<IMarshal> stdm;
			    hr=get_custom_marshaller(riid,pv, dwDestCtx,pvDestCtx,mshlflags,&stdm.p);
			   //hr = CoGetStandardMarshal (riid,(IUnknown*) pv, dwDestCtx, pvDestCtx, mshlflags, &stdm.p); 
			   if (SUCCEEDED(hr))  
			   {

				   if (SUCCEEDED( hr = stdm->GetMarshalSizeMax(riid,pv, dwDestCtx, pvDestCtx, mshlflags, pSize)))
					   *pSize+=sizeof(marshal_header_t<CLSID_std_marshal_t>);
			   }
			   */


		  
		    
			return hr;

	};
	inline HRESULT IMarshal_MarshalInterface( IStream *pStm, REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags) {

		       ULONG l;
			   HRESULT hr=S_OK;
			   ipc_utils::smart_ptr_t<IMarshal> stdm;

			   if(1)
			   {

			   
			   
			   v_buf_fix<char,1024> buf;

			   marshal_header_base_t* pmh=new (buf.get()) marshal_header_t<CLSID_bind_marshal_t>();
			   bstr_t bcl=clsid_t(__uuidof(CTestFactory_t)).parse_string(L"unmarshal=1;ee=iii");
			 //  bcl+=bstr_t(L":");
			   
			   safe_copy(pmh->szz,(wchar_t*)bcl,bcl.length());
			   pmh->size+=bcl.length()*sizeof(wchar_t);
			   
        
               hr=pStm->Write(pmh,pmh->size,&l);  
			   return hr;
			   }



			   //hr = CoGetStandardMarshal (riid,(IUnknown*) pv, dwDestContext, pvDestContext, mshlflags, &stdm.p); 
			   marshal_header_base_t mh;
			   hr=get_custom_marshaller(1,riid,pv,dwDestContext,pvDestContext,mshlflags,&stdm.p,&mh);
			   //marshal_header_t<CLSID_std_marshal_t> mh;
			   
				  if(SUCCEEDED(hr)&&SUCCEEDED(hr=pStm->Write(&mh,mh.size,&l))) 
					  hr = stdm->MarshalInterface(pStm,riid,pv,dwDestContext,pvDestContext,mshlflags);

		   //hr= pStm->Write("HHHHHHHHHHHHHvvvvvvvvvvH",10,&l);
			   //aggregator_container_helper container;
			//   hr=CoMarshalInterface(pStm,riid,container.container,dwDestContext,pvDestContext,mshlflags);
			return hr;

	};
	inline HRESULT IMarshal_UnmarshalInterface( IStream *pStm, REFIID riid,void **ppv) {

		return E_NOINTERFACE;
	};
	inline HRESULT IMarshal_ReleaseMarshalData( IStream *pStm) {
		return S_OK;
	};
	inline HRESULT IMarshal_DisconnectObject(  DWORD dwReserved) {

		return S_OK;// CoDisconnectObject(this_T(),dwReserved);

	};

};



 HRESULT CreateTest(LPOLESTR pszDisplayName,IStream* stream,REFIID riid,void ** ppvObject)	{


	 if(stream)
	 {
        


	 }


	struct F:aggregator_helper_base_t<F>
	{

		inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ){

			return marshal(Outer,riid,ppObj,hr);
		}

		marshal_callback_t<> marshal;
	};

	HRESULT hr;
	ipc_utils::smart_ptr_t<F> sF(new F,0);

	ipc_utils::smart_ptr_t<iaggregator_helper> sD=aggregator_helper_creator_t<IDispatch_test_t>::New(new  IDispatch_test_t(__uuidof(CTestFactory_t)));
       int ic=ipc_utils::ref_count(sD.p);
	aggregator_container_helper container;
	hr=container.add(__uuidof(IMarshal),sF).add(__uuidof(IDispatch),sD).add(__uuidof(IPersistStream),sD).QueryInterface(riid,ppvObject);

	smart_ptr_t<IPersistStream> ps;
	hr=container.QueryInterface(&ps.p);
	//
	/*
	if(SUCCEEDED(hr=container.set_aggregator(__uuidof(IMarshal),sF)))
		 if(SUCCEEDED(hr=container.set_aggregator(__uuidof(IDispatch),sD)))
		 	if(SUCCEEDED(hr=container.set_ExternalConnect()))
				if(SUCCEEDED(hr=container.set_FTM(true)))
					hr=container.QueryInterface(riid,ppvObject); 
					*/

	return hr;
}

 HRESULT CreateTest2(LPOLESTR pszDisplayName,IStream* stream,REFIID riid,void ** ppvObject)	{


	 struct F:aggregator_helper_base_t<F>
	 {

		 inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ){

			 return marshal(Outer,riid,ppObj,hr);
		 }

		 marshal_test_t marshal;
	 };

	 HRESULT hr;
	 ipc_utils::smart_ptr_t<F> sF(new F,0);

	 ipc_utils::smart_ptr_t<iaggregator_helper> sD=aggregator_helper_creator_t<IDispatch_empty_aggregator_t>::New(new  IDispatch_empty_aggregator_t);
	 int ic=ipc_utils::ref_count(sD.p);
	 aggregator_container_helper container;
	 //
	 if(SUCCEEDED(hr=container.set_aggregator(__uuidof(IMarshal),sF)))
		 if(SUCCEEDED(hr=container.set_aggregator(__uuidof(IDispatch),sD)))
			 if(SUCCEEDED(hr=container.set_ExternalConnect()))
				 if(SUCCEEDED(hr=container.set_FTM(true)))
					 hr=container.QueryInterface(riid,ppvObject); 

	 return hr;
 }


 


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
 void test_script(IDispatch* pengine=0)
 {
     ipc_utils::COMInitF_t ci;
	 ipc_utils::stopwatch_t cs;
	 ipc_utils::smart_ptr_t<IDispatch> disp;
	 bstr_t bn=L"ltx.script:debug=3";
	 bstr_t bns=L"ltx.script";
	 clsid_t clsid(bns);
	 HRESULT hr;
	 hr=::CoGetObject(bn,0,__uuidof(IDispatch),disp.ppQI());
	 double t;
	 variant_t r;
		 cs.Start();
	 for(int i=0;i<1000;i++)
	 {
	 if(FAILED(hr=::CoGetObject(bn,0,__uuidof(IDispatch),disp.ppQI())))		   FatalAppExit(0,L"Errrrrrr");
	 //if(FAILED(hr=::CoCreateInstance(clsid,0,CLSCTX_ALL,__uuidof(IDispatch),disp.ppQI())))		   FatalAppExit(0,L"Errrrrrr");
	   //if(FAILED(hr=call_disp(disp,r.GetAddress(),0,L"b={a:$$[0]}",-100)))		   FatalAppExit(0,L"Erscr");
      
	 }
          t=cs.Sec();
	 cprintf("t=%g\n",t);
	 getchar();


 }

#include <set>
 struct UU{
	 int k;
	 int h;
	 UU(int kk=0):k(kk){};
	 inline operator int(){return k;};

 };

int _tmain(int argc, _TCHAR* argv[])
{






HRESULT hr=sizeof(sss_t);
int rc;

class_object_pair_t* pclass_object_pair;

register_class_map_t<>& rcm=register_class_map_t<>::get_instance();
ipc_utils::smart_ptr_t<IUnknown> uncf,uncf2;
hr=rcm.register_class(__uuidof(CTestFactory_t),&CTestFactory_t::GetClassObject,uncf.address());
rc=ref_count(uncf);
hr=rcm.register_class(__uuidof(CTestFactory_t),&CTestFactory_t::GetClassObject2,uncf2.address(),&pclass_object_pair);
rc=ref_count(uncf);

uncf.Release();
uncf2.Release();

	HMODULE hm=LoadLibraryW(L"R:\\ipc\\bin_d\\x64\\ltx_js.dll");
       hr=GetLastError();
	//
	//std::set<UU> seu;
	//seu.insert(10);

	 //test_script();
	 //return 0;


//hr=ltx_register_class(__uuidof(CTestFactory_t),&CTestFactory_t::GetClassObject,L"CTestFactory",uncf.address());
//hr=ltx_register_class(__uuidof(CTestFactory_t),0);
//hr=ltx_register_class(CTestFactory2_t::GetCLSID(),&CTestFactory2_t::GetClassObject,L"CTestFactory22",uncf.address());
void* p02=	   &CTestFactory2_t::GetClassObject;
void* p12=	   &CTestFactory21_t::GetClassObject;
hr=ltx_register_class(__uuidof(IDispatch_test_t),&CTestFactory2_t::GetClassObject,L"CTestFactory22",uncf.address());
hr=ltx_register_class(__uuidof(IDispatch_test_t),0);

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


	ipc_utils::COMInitF_t ci;
	IMarshal* pfmt=0,*pfmt2=0;


   clsid_t clss(L"ltx.bind");

   {
        bstr_t pppid=clss.prog_id();
   }

   //


	//hr=CoCreateFreeThreadedMarshaler(0,(IUnknown**)&pfmt);
	//pfmt&&(hr=pfmt->QueryInterface(__uuidof(IMarshal),(void**)&pfmt2));
	

	ipc_utils::smart_ptr_t<IUnknown> ut;

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
	

	
	  get_double(0);
	  //_atodbl
	
	
   //hr=CreateTest(0,0,__uuidof(IUnknown),ut.ppQI());
	//	hr=IDispatch_test_t::CreateInstance(0,__uuidof(IUnknown),ut.ppQI());	

	hr=CTestFactory2_t::GetObjectEx(0,0,0,__uuidof(IUnknown),ut.ppQI());

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
//ComClipboard()=ut.p;
getchar();

	return 0;
}

