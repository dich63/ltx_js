// koxIOtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ipc_ports/kox.h"
#include "ipc_ports/ipc_utils.h"
#include "conio.h"
#include "wchar_parsers.h"
#include "singleton_utils.h"
#include "dummy_dispatch_aggregator.h"
#include "rc_holder.h"
#include "os_utils.h"
#include "ipc_ports/istream_pipe.h"
#include "ipc_ports/com_apartments.h"
//#include "ipc_ports/com_script_engine.h"

//#pragma comment(lib, "delayimp")
//#define  _XA64 1
#ifdef _XA64
#ifdef _DEBUG 
#pragma comment(lib ,"../../lib/x64/debug/kox.lib")
#else
#pragma comment(lib ,"../../lib/x64/release/kox.lib")
#endif
#else
#ifdef _DEBUG 
#pragma comment(lib ,"../../lib/Win32/debug/kox.lib")
#else
#pragma comment(lib ,"../../lib/Win32/release/kox.lib")
#endif

#endif
//#pragma comment(linker, "/DELAYLOAD:kox.dll" ) 

#define CComPtr ipc_utils::smart_ptr_t 
#define KB (1024)
#define MB (KB*KB)

//void  __stdcall on_connect_callback0(void* pcontext,i_kox_stream* stream,void* pconn)
char url[]="\\\\.\\pipe\\mula";
int nl=1024*2;
char* p=new char[nl];
/*
inline long refcount(IUnknown* pu)
{
	if(!pu) return -1;
	pu->AddRef();
	return pu->Release();
}
*/


#define CERROR(a) { HRESULT hr0000=a ;iferror(hr0000,#a); }
template <class T>
struct test_kox_base_t
{
	typedef T* this_T;
	char url[256];
	ipc_utils::smart_ptr_t<IUnknown> port;
	HRESULT hr0;

	static 	void   s_on_cb(void* pcontext,i_kox_stream* stream,void* pconn)
	{
		this_T(pcontext)->on_connect(stream);
	}
	test_kox_base_t(char *u)
	{   strcpy(url,u);
	//hr=io_pipe_port_connectA(url,stream.address());   
	CERROR( hr0=io_pipe_port_create_runA(url,"xPIPE_TIMEOUT=60000\0\0",&s_on_cb,this_T(this),port.address()); );
	 
	}
	HRESULT connect(ipc_utils::smart_ptr_t<i_kox_stream>& stream,char* u=0)
	{
		CERROR( hr0=io_pipe_port_connectA((u)?u:url,stream.address()) );
		return 0;
	}
	int wait_port(int ti=-1)
	{
		HRESULT hr;
		HANDLE hevent;
		if(hr=io_pipe_port_get_waitable_event(port,&hevent)) return -1;
		hr= WaitForSingleObject(hevent,ti);
		CloseHandle(hevent);
		return hr;

	}
	
	void error(HRESULT hr,char* msg=0)
	{
		wcharbuf buf;
		msg||(msg="");
		wchar_t* pp=buf.printf(L"msg:%s\r\n err:%8x\r\n %s",(wchar_t*)char_mutator<CP_UTF8>(msg),hr,(wchar_t*)error_msg(hr));
		FatalAppExitW(0,pp);
	}
	void iferror(HRESULT hr,char* msg=0)
	{
		if(FAILED(hr)) error(hr,msg);
	}
};


struct test_pipe_t:test_kox_base_t<test_pipe_t>
{
	v_buf<char> buf;
	char *pbuf;
	int iii,k;
	volatile long count;
	HANDLE hevent;

	test_pipe_t(char* u="\\\\.\\pipe\\test_pipe"):test_kox_base_t(u){

		hevent=CreateEventW(0,1,0,0);
		pbuf=buf.resize(64*MB);
	};
	void on_connect(i_kox_stream* stream)
	{
		DWORD dw,dd;
		InterlockedIncrement(&count);
		stream->Read(&dw,4,&dd);
		//
		//WaitForSingleObject(hevent,INFINITE);
		//		WaitForSingleObject(hevent,1000);
		//
		if(stream) stream->AddRef();
		
	};

	HRESULT run(int N=255)
	{
		typedef i_kox_stream* PIStream;
		PIStream* pstream=new PIStream[100*10000];
		count=0;
		DWORD dw,dd;
		InterlockedExchange(&count,count);
		for(k=0;k<N;k++)
		{
			 hr0=io_pipe_port_connectA(url,&pstream[k]);
			 iferror(hr0,charbuf().printf("k=%d count=%d",k,get_count()));
			 if(hr0) break;
			 if(!hr0) hr0=pstream[k]->Write(&dw,4,&dd);
			 iferror(hr0,charbuf().printf("k=%d count=%d",k,get_count()));
		}
		
		return hr0;
	}

	inline long get_count()
	{
		return InterlockedExchangeAdd(&count,0);
	}
};


template <int VERS=0>
struct tls_num_convert_t
{
	//	get_double
};



///*
struct test0_t:test_kox_base_t<test0_t>
{
	v_buf<char> buf;
	char *pbuf;
	int iii;

	test0_t(char* u="\\\\.\\pipe\\test0"):test_kox_base_t(u){
		pbuf=buf.resize(4*64*MB);
	};
	void on_connect(i_kox_stream* stream)
	{

		HRESULT hr;

		HANDLE hf;

/*		hr=kox_get_fd(stream,&hf);
		
       ULONG pidc=-1,pids=-1;
		GetNamedPipeClientProcessId(hf,&pidc);
		GetNamedPipeServerProcessId(hf,&pids);
		//cprintf("oncb: pids=%d pidc=%d\n ",pids,pidc);
		*/


		ipc_utils::COMInit_t ci(0);





		 kox_unmarshal_helper_t umd(stream);
		 //hr=umd.reset(stream) ;


		 iferror(umd.hr,"unmarshal error");

	  SOCKET s=umd[0];

	  HANDLE hport=umd[1];



	};

	void run(char* pname=0)
	{
		HRESULT hr;
		CComPtr<i_kox_obj> koxm,port;
		CComPtr<i_kox_stream> stream;
		;

		SOCKET sos=::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		HANDLE hiop=CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);

		//ipc_region_base_t rgs[]={{0,sizeof(sos),(char*)&sos},{IPCM_SOCKET,sizeof(sos),(char*)&sos},     {IPCM_HANDLE,sizeof(hiop),&hiop},{0,1024,pbuf}};

       //ipc_region_base_t rgs[]={{&sos,sizeof(sos),IPCM_SOCKET},{&hiop,sizeof(hiop),IPCM_HANDLE},{pbuf,4*1920*1024}};
		ipc_region_base_t rgs[]={{pbuf,2*64*1024*1024}};
	   int sii=sizeof(ipc_region_base_t);
	   int siig=sizeof(GUID );
		hr=connect(stream,pname);


		HANDLE hf;
		/*
		hr=kox_get_fd(stream,&hf);
		
		ULONG pidc=-1,pids=-1;
		GetNamedPipeClientProcessId(hf,&pidc);
		GetNamedPipeServerProcessId(hf,&pids);
		*/
		//cprintf("pids=%d pidc=%d\n ",pids,pidc);



		int cr=sizeof(rgs)/sizeof(*rgs);
		hr=kox_marshal_array(stream,cr,rgs) ;
		CERROR(hr);
		//cprintf("marshal ok!");
		//getchar();

		

		

		CloseHandle(hiop);
		closesocket(sos);

		
	}
};


//*/




struct  __declspec(uuid("{59486696-7358-466c-8886-F06500973E92}")) IStreamer:IUnknown
{
   typedef void ( *OnIO_t) (void* context,ISequentialStream* BackEndStream,int flag);
   typedef void ( *OnExit_t) (void* context);
virtual   HRESULT __stdcall GetFrontEndStream(ISequentialStream** ppStream)=0;
virtual   HRESULT __stdcall SetOnBackEnd(void* context,OnIO_t IO_callback,OnExit_t release_callback)=0;
};





/*
struct IStreamer_impl_t:base_aggregator_t<IStreamer_impl_t,IStreamer>
{
	struct Stream_impl_t:base_component_t<Stream_impl_t,ISequentialStream>
	{
		inline IStreamer_impl_t* owner(){ return (IStreamer_impl_t*)pOuter;}

		Stream_impl_t(IStreamer_impl_t* powner,bool _fmaster):base_component_t<Stream_impl_t,ISequentialStream>(powner),fmaster(_fmaster),dual(0){};

		virtual HRESULT STDMETHODCALLTYPE Read(void *pv,ULONG cb, ULONG *pcbRead)
		{
			HRESULT hr;
                  hr=dual->Write(pv,cb,pcbRead);  


			if(fmaster&&(FAILED(hr=owner()->onIO(1) ) ) )  
				  return hr;

			
			return hr;
		};

		virtual  HRESULT STDMETHODCALLTYPE Write( const void *pv,ULONG cb,ULONG *pcbWritten) 
		{
			HRESULT hr;
			if(fmaster&&(FAILED(hr=owner()->onIO(2) ) ) )  
				return hr;

			return hr;
		};

            bool fmaster;
			Stream_impl_t* dual;
			asyn_su::thread_queue queue;
	};

	HRESULT onIO(int mode)
	{
		HRESULT hr=0;
      return hr;
	}


};
*/

//ipc_utils::dup_handle()


/*
inline HRESULT CreateStreamPair(IUnknown* psite,ISequentialStream** pps1,ISequentialStream** pps2)
{
	const int PIPESIZE=4*1024;
	typedef ISequentialStreamPipe_Impl<PIPESIZE> ISequentialStreamPipe_t;
	HRESULT hr;
	

	if(!(pps1&&pps2)) return E_POINTER;

  ipc_utils::smart_ptr_t<IUnknown> unk;
  
  if(!psite)
  {
  	   if(FAILED(hr=create_dummy_unk(&unk.p))) return hr;
	   psite=unk;
  }
  

  ISequentialStreamPipe_t::handle_t hr1,hw1,hr2,hw2;
  bool f;
  if(!(f=CreatePipe(&hr1.h,&hw1.h,0,PIPESIZE)))
	    return HRESULT_FROM_WIN32(GetLastError());
  if(!(f=CreatePipe(&hr2.h,&hw2.h,0,PIPESIZE)))
	  return HRESULT_FROM_WIN32(GetLastError());

   
  *pps1=new  ISequentialStreamPipe_t(psite,hr1,hw2);
  *pps2=new  ISequentialStreamPipe_t(psite,hr2,hw1);
  
  return S_OK;


};
*/



inline HRESULT CreateStreamPair0(IUnknown* psite,ISequentialStream** pps1,ISequentialStream** pps2)
{
	int rc;
	const int PIPESIZE=4*1024;
	typedef ISequentialStreamPipe_Impl<PIPESIZE> ISequentialStreamPipe_t;
	HRESULT hr;


	if(!(pps1&&pps2)) return E_POINTER;

	ipc_utils::smart_ptr_t<iaggregator_container> ac1,ac2;
	
	//create_iaggregator_container(iaggregator_container** ppout,void* pcntx=0,void (*close_proc)(void*)=0,void (*exit_proc)(void*)=0)



	ISequentialStreamPipe_t::handle_t hr1,hw1,hr2,hw2;
	bool f;
	if(!(f=CreatePipe(&hr1.h,&hw1.h,0,PIPESIZE)))
		return HRESULT_FROM_WIN32(GetLastError());
	if(!(f=CreatePipe(&hr2.h,&hw2.h,0,PIPESIZE)))
		return HRESULT_FROM_WIN32(GetLastError());

	if(FAILED(hr=create_iaggregator_container(&ac1.p))) return hr;
	if(FAILED(hr=create_iaggregator_container(&ac2.p))) return hr;

    
	struct FStream_t:aggregator_helper_base_t<FStream_t>
	{
	
		inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ){

			return stream(Outer,riid,ppObj,hr);
		}


		FStream_t(IUnknown* punk,HANDLE h_r,HANDLE h_w):stream(h_r,h_w),dwUnk(0){
			HRESULT hr;
            if(punk) 
				hr=ipc_utils::GIT_t::get_instance().wrap(punk,&dwUnk);

		};

		//IGITDispatch_t<IUnknown> disp;
		//IGT disp;
		ISequentialStreamPipe_t stream;
		DWORD dwUnk;
		//ipc_utils::smart_ptr_t<IUnknown> unk;

		~FStream_t()
		{
           (dwUnk)&&(ipc_utils::GIT_t::get_instance().unwrap(dwUnk));
		}

	};




    //ISequentialStreamPipe_t* sp1=new  ISequentialStreamPipe_t(psite,hr1,hw2),*sp2=new  ISequentialStreamPipe_t(psite,hr2,hw1);
	ipc_utils::smart_ptr_t<FStream_t> sp1(new  FStream_t(psite,hr1,hw2),0),sp2(new  FStream_t(psite,hr2,hw1),0);
     /// rc=refcount(sp1);
           
      ipc_utils::smart_ptr_t<ISequentialStream> s1,s2;   
     ipc_utils::smart_ptr_t<IUnknown> fmtm1,fmtm2;


	 f&&SUCCEEDED(hr=ac1->SetAggregator(__uuidof(ISequentialStream),sp1) );
	 f&&SUCCEEDED(hr=ac2->SetAggregator(__uuidof(ISequentialStream),sp2) );

	f&&SUCCEEDED(hr=create_FTM_aggregator_helper(&fmtm1.p));
	f&&SUCCEEDED(hr=create_FTM_aggregator_helper(&fmtm2.p));

    f&&SUCCEEDED(hr=ac1->SetAggregator(__uuidof(IMarshal),fmtm1) ) ;   
	f&&SUCCEEDED(hr=ac2->SetAggregator(__uuidof(IMarshal),fmtm2) ) ;   

	
    

	f&&SUCCEEDED(hr=ac1.QueryInterface(&s1.p));
	f&&SUCCEEDED(hr=ac2.QueryInterface(&s2.p));

    if(!f) return hr;
//	rc=refcount(s1);

	*pps1=s1.detach();
	*pps2=s2.detach();
 //    rc=refcount(*pps1);

	return S_OK;


};


//void test_pair(ISequentialStream* pst)
struct test_pair
{
	ipc_utils::smart_ptr_t<i_kox_stream> st;
	test_pair(i_kox_stream *p):st(p){};
	char buf[1024];
	HRESULT hr;
   void operator()()
   {
	   DWORD dw;
      //hr=st->Read(buf,10,&dw);  
	   kox_unmarshal_helper_t umd(st);
	   //hr=umd.reset(stream) ;

	   //iferror(umd.hr,"unmarshal error");

	   SOCKET s=umd[0];

	   HANDLE hport=umd[1];
	   kox_unmarshal_helper_t::region_t pp=umd[2];
	   hr=11;

   }

};

struct test_pair_map
{
	ipc_utils::smart_ptr_t<i_kox_stream> st;
	test_pair_map(i_kox_stream *p):st(p){};
	char buf[1024];
	HRESULT hr;
	void operator()()
	{
		HRESULT hr;
         ipc_utils::COMInit_t ciii(0);


		CComPtr<IUnknown> koxm,como;
		char** ppargs;
		int cb;
		ipc_region_ptr_t r,rs,rh,rcom;
		

		hr=kox_map_unmarshal(koxm.address(),st);

		st->Revert();

		hr=kox_map_info(koxm,&ppargs,&cb);

		hr=kox_map_get(koxm,"io",&rh);
		HANDLE hio=make_detach(*(rh->phandle));
		hr=kox_map_get(koxm,"sock",&rs);	
		hr=kox_map_get(koxm,"data",&r);	
		hr=kox_map_get(koxm,"com",&rcom);
		hr=kox_get_unmarshal_com_object(rcom,como.ppQI());



		char * pdata=kox_get_region_ptr(r);
		koxm.detach()->Release();
		cb=99;

	}

};

struct test_pair_map_back
{
	ipc_utils::smart_ptr_t<i_kox_stream> st;
	test_pair_map_back(i_kox_stream *p):st(p){};
	char buf[1024];
	HRESULT hr;
	void operator()()
	{
	}
/*
     void run()
		HRESULT hr;
		ipc_utils::COMInit_t ciii(0);


		CComPtr<IUnknown> koxm,como;
		char** ppargs;
		int cb;
		ipc_region_ptr_t r,rs,rh,rcom;


		asyn_su::thread_queue tq;
		tq.init();
		test_pair_map tp(st2);
		asyn_call(&tp,1,0);



		hr=kox_map_unmarshal(koxm.address(),st);

		st->Revert();

		hr=kox_map_info(koxm,&ppargs,&cb);

		hr=kox_map_get(koxm,"io",&rh);
		hr=kox_map_get(koxm,"sock",&rs);	
		hr=kox_map_get(koxm,"data",&r);	
		hr=kox_map_get(koxm,"com",&rcom);
		hr=kox_get_unmarshal_com_object(rcom,como.ppQI());


		char * pdata=kox_get_region_ptr(r);
		koxm.detach()->Release();
		cb=99;

	}
	*/

};

double testGIT()
{
	HRESULT hr;
	double t;
	ipc_utils::COMInit_t(0);
	ipc_utils::stopwatch_t cs;
	
	for(int k=0;k<1000;k++){
		ipc_utils::smart_ptr_t<IGlobalInterfaceTable> GIT;
		hr=GIT.CoCreateInstance(CLSID_StdGlobalInterfaceTable);	 
		if(FAILED(hr)) throw 1;
	}
	 t=cs.Sec();
	return t;
}


int _tmain(int argc, _TCHAR* argv[])
{
	//testGIT();
	int rc;
	HRESULT hr;


	 ipc_utils::CHGlobal_t<char> hg("AAAAAAAAAhhhhhhhhhhhAAAAAA",10);


	bool fha=GetHandleInformation(HANDLE(hg.handle()),(DWORD*)&rc);

	 class_initializer_T<ipc_utils::WSA_initerfiniter>().get() ;
	ipc_utils::COMInit_t ciii(0);

	ipc_utils::COMInit_t ciii2(COINIT_APARTMENTTHREADED);
	//kox_unmarshal(0,0);
ipc_utils::smart_ptr_t<i_kox_stream> st1,st2;

char* pbuf=new char[4*1920*1024];
strcpy(pbuf,"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA!!");

ipc_utils::smart_ptr_t<IUnknown> unk;

hr=create_dummy_unk(&unk.p);
DWORD dwUnk,dwUnk2;
/*
rc=refcount(unk);
hr=ipc_utils::GIT_t::get_instance().wrap(unk.p,&dwUnk);
rc=refcount(unk);
hr=ipc_utils::GIT_t::get_instance().wrap(unk.p,&dwUnk2);
rc=refcount(unk);
*/
hr= CreateStreamPair(unk,&st1.p,&st2.p);
/*
unk->Release();
rc=refcount(st1);
rc=refcount(unk);
st1.Release();
rc=refcount(unk);
st2.Release();
unk.p=0;
return 0;
rc=refcount(unk);
*/
asyn_su::thread_queue tq;
tq.init();
test_pair_map tp(st2);
//asyn_call(&tp,1,0);
DWORD dw;
SOCKET sos=::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
HANDLE hiop=CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);

ipc_region_base_t rgs[]={{&sos,sizeof(sos),IPCM_SOCKET},{&hiop,sizeof(hiop),IPCM_HANDLE},{pbuf,4*1920*1024}};
//hr=st1->Write("AAAAAAAAAAA\0",10,&dw);  
int cr=sizeof(rgs)/sizeof(*rgs);
//hr=kox_marshal_array(st1,cr,rgs) ;

CComPtr<i_kox_obj> koxm;
hr=kox_map_unmarshal(koxm.address());



hr=kox_map_set_data(koxm,"sock",&sos,sizeof(sos),IPCM_SOCKET);
hr=kox_map_set_data(koxm,"io",&hiop,sizeof(hiop),IPCM_HANDLE);
hr=kox_map_set_data(koxm,"data",pbuf,1024);
hr=kox_map_set_data(koxm,"com",&st1.p,sizeof(void*),IPCM_COM);

//hr=kox_map_marshal(koxm,st1,GetCurrentProcessId());

ipc_utils::smart_ptr_t<i_kox_stream> stt,sst;
//getchar();
hr=CreateStreamOnHGlobal(0,1,stt.address());

HGLOBAL hstub;
//sst=stt;
//
hr=IStreamAggregator_Impl<>::CreateInstance(HGSTREAM,koxm,sst.address());
//hr=IStreamAggrenator_Impl<>::CreateInstance(stt,koxm,sst.address());
//hr=ipc_utils::CoMarshalInterfaceToHGlobal(stt,&hstub);
//hr=kox_map_set_data(koxm,"com",&hstub,sizeof(void*),IPCM_COMSTUB);
hr=kox_map_marshal(koxm,sst,GetCurrentProcessId());
koxm.reset();
hr=sst->Seek(LARGE_INTEGER(),STREAM_SEEK_SET,&ULARGE_INTEGER());
test_pair_map tpm(sst);
tpm();

getchar();

       test_pipe_t test_pipe;
	   
	   if(0)
	   {
	   hr=test_pipe.run(2048);
	
        cprintf("test_pipe.count=%d\n",test_pipe.get_count());
		cprintf("event signal..");
	   getchar();
	   
	   SetEvent(test_pipe.hevent);
	   cprintf("ok\n");
	   getchar();
	   return 0;
	   }
        test0_t test_pipe0;
	   test_pipe0.run();

	return 0;
}






