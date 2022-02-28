#pragma once

#include "video/singleton_utils.h"
#include "hssh.h"
#include "os_utils.h"
#include "buf_fm.h"
#include "wbs_buffer.h"
#include "COMErrorHelper.h"
#include <strstream>
#include "ipc_ports/hssh.h"
#include "free_com_factory.h"
#include "ltx_proc_table.h"
#include "MBVLocalHost.h"
#include "frame_array_buf.h"


#define CHECK_FAIL_(hr,a) if(FAILED(hr=(a))) \
	return hr;
#define CHECK_FAIL_hr(a)  CHECK_FAIL_(hr,a)


template <int DIG=__DIG>
struct ltx_hss_json_rpc_t
{
	typedef ltx_hss_json_rpc_t<DIG> json_rpc_t;

	typedef void (__stdcall * raise_error_proc)(const wchar_t* smessage,const wchar_t* ssource);

	typedef ltx_proc_table_helper::arguments_t<variant_t> arguments_t;


	enum
	{
		bufsize=1<<DIG
	};

inline static hssh_t& hsslib()
 {
    static hssh_t h;
	return h;
 }

inline static v_buf<wchar_t>& filelog()
{
    static v_buf<wchar_t> s_filelog;
	return s_filelog;
}


struct exceptinfo_t: EXCEPINFO
{
	exceptinfo_t(){ memset(this,0,sizeof(exceptinfo_t));}
	~exceptinfo_t()
	{
		reset();
	}

inline 	exceptinfo_t* reset()
	{
		SysFreeString( make_detach(bstrSource));
		SysFreeString(make_detach(bstrDescription));
		SysFreeString(make_detach(bstrHelpFile));
		return this;
	}

};



struct wbs_getter_t
{
	SOCKET s;
	//bool fv;
	wbs_getter_t(HSS_INFO* hovl)
	{
		s=hovl->s;
		//fv=hovl->buffers[HSD_WS_VERSION].buf!=0;
	}

	wbs_getter_t():s(INVALID_SOCKET){};

	int operator()(int nbuf,WSABUF *pbuf)
	{
		DWORD Flags=0,cbtot=-1;
		int err=WSARecv(s,pbuf,nbuf,&cbtot,&Flags,NULL,NULL);
		return (err)?-1:cbtot; 
	}

};

struct fmo_stream_t 
{
	HRESULT hr;
	MBVLocalHost::com_ptr<isa_frame_buf > frames;
	variant_t frames_disp;
	LONG64 pos_g;
	fmo_stream_t(pltx_proc_table ltx):pos_g(0) 
	{  
        hssh_t& hssh=json_rpc_t::hsslib();

		if((SUCCEEDED(hr=hssh.hr))&&(SUCCEEDED(hr=ltx->ltx_parse_bind(L"mmap:frames('',[1],'byte',8)",&frames_disp))))
		   hr=frames_disp.pdispVal->QueryInterface(isa_frame_buf::iid(),frames.ppQI()); 
	}


	fmo_stream_t(VARIANT v):pos_g(0) 
	{  
		hssh_t& hssh=json_rpc_t::hsslib();
 

		if(SUCCEEDED(hr=hssh.hr)&&SUCCEEDED(hr=VariantChangeType(&frames_disp,&v,0,VT_DISPATCH)))
		{

			hr=frames_disp.pdispVal->QueryInterface(isa_frame_buf::iid(),frames.ppQI()); 

		}
	}


inline HRESULT put_unicode(VARIANT v)
{
	variant_t tmp;
  if(FAILED(hr)) return hr;
   HRESULT hr0;
   BSTR p;
   if(v.vt==VT_BSTR) p=v.bstrVal;
   else
   {
     CHECK_FAIL_(hr0,VariantChangeType(&tmp,&v,0,VT_BSTR));
	 p=tmp.bstrVal;
   }

   return hr0=frames->put_unicode_string(0,0,CP_UTF8,p);
}

inline HRESULT get_unicode(VARIANT* pv)
{
	if(!pv) return E_POINTER;
	if(FAILED(hr)) return hr;
	HRESULT hr0;
	VARIANT r={VT_BSTR};
	hr0=frames->get_unicode_string(0,-4,CP_UTF8,&r.bstrVal);
	if(SUCCEEDED(hr0)) *pv=r;
    return hr0;  
}



inline  fmo_stream_t& rewind_read(){ 
	
	pos_g=0;
	return *this;
}

inline fmo_stream_t& rewind_write(){

	frames->SetCursor(0);
	return *this;
}
inline fmo_stream_t& reset(){
  rewind_write();
  return rewind_read();
}

inline LONG64 size()
{
 return frames->GetCursor();
}
inline bool eof()
{
	return frames->GetCursor()<=pos_g;
}
inline int write(void* ps,int cb)
{ 
	if(FAILED(hr)) return -1;
   LONG64 ibegin=frames->GetCursor();
   LONG64 iend=ibegin+LONG64(cb); 
   char* pd=frames->getFramePtrEx(0,ibegin,iend);
   if(!pd) return -1;
   memcpy(pd,ps,cb);
   frames->SetCursor(iend);
   return cb;
}
inline int read(void* pd,int cb)
{ 
	if(FAILED(hr)) return -1;
	LONG64 ic=frames->GetCursor();
	
	LONG64 ibegin=pos_g;
	LONG64 iend=ibegin+LONG64(cb); 
	if(ic<iend)	{
		cb=ic-ibegin;
		iend=ic;
	}
	if(cb>0){
	char* ps=frames->getFramePtrEx(0,ibegin,iend);
	if(!ps) return -1;
	memcpy(pd,ps,cb);
	pos_g+=cb;
	}
	
	return cb;
}

 inline LONG64 get_size()
 {
     return  frames->GetCursor();
 }

 inline int get_next_frame(int cb,char** pp)
 {
         if(FAILED(hr)) return -1;
		 LONG64 ic=frames->GetCursor();
		 LONG64 ibegin=pos_g;
		 LONG64 iend=ibegin+LONG64(cb); 
		 if(ic<iend)	{
			 cb=ic-ibegin;
			 iend=ic;
		 }
		 if(cb>0){
			 char* ps=frames->getPtr(ibegin,iend);
			 if(!ps) return -1;
			 *pp=ps;
			 pos_g+=cb;
		 }
         
		 return cb;

 }

 inline INT64 send_frames(hssh_t& hssh,HSS_INFO* hovl,HRESULT& hr)
 {

	 //	 const int bufsize_r=bufsize;
	 //
	 const int bufsize_w=1<<16;

     INT64 cbtot=0;
	 int cbout;
	 char* p;

    if(FAILED(hr=hssh.write_wbs_frame(hovl,0,0,WSF_TXT,0))) 
		   return -1;
	while( (cbout=get_next_frame(bufsize_w,&p))>0 )
	{
		//cbout=hssh.send(hovl,p,cbout,0); 			
       if(FAILED(hr=hssh.write_wbs_frame(hovl,p,&cbout,WSF_CNT,0))) 
		          return -1;
		cbtot+=cbout;

	}
	if(FAILED(hr=hssh.write_wbs_frame(hovl,0,0,WSF_FIN,0))) 
		return -1;

	 return cbtot;
      
 }

 
inline  INT64 recv_frames(HSS_INFO* hovl,HRESULT& hr,mutex_cs_t* mutex=0)
 {
	 //	 const int bufsize_r=bufsize;
	 //
	 const int bufsize_r=1<<16;
	 
	 int cbin;
	 char* p;
	 hssh_t& hssh=hsslib();
	 wbs_frame_base_t wbs_frame;

	 {
	  unlocker_t<mutex_cs_t> unlock(mutex);
	 if(FAILED(hr=hssh.read_wbs_frame(hovl,1,0,0,&wbs_frame))) 
		 return -1;
	 }
	 frames->SetCursor(0);
	 INT64 cbtot=0;
  	 do
	 {
		 
	     char* ps=frames->getPtr(cbtot,cbtot+bufsize_r);	 
		 if(FAILED(hr=hssh.read_wbs_frame(hovl,0,ps,&(cbin=bufsize_r),&wbs_frame))) 
			 		 return -1;

		 
		 cbtot+=cbin;
		//		 logmsg(L"recv[%d]:%d\r",cbin,int(cbtot));
        
	 }
	 while(hr==0);

	 frames->SetCursor(cbtot);
	 
	 return cbtot;
 }

 inline INT64 recv_http_body(HSS_INFO* hovl,HRESULT& hr)
 {
	 //	 const int bufsize_r=bufsize;
	 //
	 const int bufsize_r=1<<16;

	 int cbin;
	 char* p;
	 hssh_t& hssh=hsslib();

	 

	 frames->SetCursor(0);
	 
	 char* pbcb=hssh.reset_SOCKET_DATA(hovl,HSD_BODY_LENGTH,0,0,0);
	 if(!pbcb) {hr=E_FAIL;return -1;}
	 INT64 cbtot=0,cbboby=atof(pbcb);
      
	 while(cbboby>0)
	 {

		 char* ps=frames->getPtr(cbtot,cbtot+bufsize_r);	 
		 
		 cbin=hssh.recv(hovl,ps,(cbboby>bufsize_r)?bufsize_r:cbboby,0x010000);
		 if(cbin<0)	 {hr=E_FAIL;return -1;}
		 if(cbin==0)	break;
		 cbtot+=cbin;
		 cbboby-=cbin;
		 //		 logmsg(L"recv[%d]:%d\r",cbin,int(cbtot));

	 }
	 

	 frames->SetCursor(cbtot);

	 return cbtot;
 }

inline INT64 send_http_body(HSS_INFO* hovl,HRESULT& hr)
{
	     const int bufsize_w=1<<16;

	    hssh_t& hssh=hsslib();
		

        INT64 cbtot=frames->GetCursor();
		int cbout;
		const char fmt[]="HTTP/1.x 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %g\r\n\r\n"; 
		charbuf replyheader;
		replyheader.printf(fmt,double(cbtot));
		int cbh=hssh.send(hovl,replyheader.get(),strlen(replyheader),0);
		if(cbh<0) { hr=E_FAIL;return cbh;}
		if(cbh==0) return cbh;
		char* p;
		INT64 cbsend=0;
		while( (cbout=get_next_frame(bufsize_w,&p))>0 )
		{
			cbout=hssh.send(hovl,p,cbout,0);
			if(cbout<0) { hr=E_FAIL;return cbout;}
			if(cbout==0) break;
			
			cbsend+=cbout;
		}

       return cbsend;
		//frames->SetCursor(0);

}

 inline INT64 send(HSS_INFO* hovl,HRESULT& hr)
 {
	 hssh_t& hssh=hsslib();
	 int fv=hssh.wbs_version(hovl);
	 if(fv<0) return -1;
	 if(fv) return send_frames(hssh,hovl,hr);

	 int cbout;
	 char* p;
	 unsigned char wh[2]={0,0xff};

	 INT64 cbtot=0;
	 cbout=hssh.send(hovl,wh,1,0); 			
	 if( cbout<=0) return -1;
	 cbtot+=cbout;
	 while( (cbout=get_next_frame(bufsize,&p))>0 )
	 {
		 cbout=hssh.send(hovl,p,cbout,0); 			
		 if(cbout<=0) 
			 return -1;
		 cbtot+=cbout;

	 }
	 cbout=hssh.send(hovl,wh+1,1,0); 			
	 if( cbout<=0) return -1;
	 cbtot+=cbout;
	 return cbtot;
 }

   
};




struct client_connect_holder_t:disp_wrap_base_t<client_connect_holder_t>
{
	typedef variant_t com_variant_t;

	

	HSS_INFO* hovl;
	HRESULT hr;
	wbs_getter_t wbs_getter;//(hovl);
	wbs_string_loader_t<wbs_getter_t,DIG> wbsloader;//(wbs_getter,1);
	fmo_stream_t  fmo_buf;
	int fv;

	client_connect_holder_t(pltx_proc_table ltx, arguments_t& args):hovl(0),wbsloader(wbs_getter,1),fmo_buf(ltx)
	{
		hssh_t& hssh=json_rpc_t::hsslib();
		hr=hssh.hr;
		if(FAILED(hr))
		{
            if(FAILED(hr=(new(&hssh) hssh_t())->hr))
				  return;
		}

		bstr_t host_port=args[0].def<bstr_t>();
		bstr_t ns_path=args[1].def<bstr_t>();				  
	//	bstr_t bargs=args[2].def<bstr_t>();
		variant_t vparamszz=args[2];
		HRESULT hrcv;
		v_buf<wchar_t> bufzz;
		wchar_t* pzz=0;
		UINT cbzz;

		if(SUCCEEDED(hrcv=VariantChangeType(&vparamszz,&vparamszz,0,VT_BSTR)))
		{
           cbzz=SysStringLen(vparamszz.bstrVal);
		   pzz=bufzz.resize(cbzz+4).get();
		   memcpy(pzz,vparamszz.bstrVal,cbzz*sizeof(wchar_t));
	   
		}
	//	bargs+=bstr_t(L"\0");
		hovl= hssh.websocket_connect2W(host_port,ns_path,pzz);
		if((fv=hssh.wbs_version(hovl))<0)  hr=hssh.error();
		else wbs_getter=wbs_getter_t(hovl);

	}
	~client_connect_holder_t()
	{
		
		if(hovl)		json_rpc_t::hsslib().release(hovl);
				
	}

	void  disconnect()
	{
		SOCKET s=make_detach(hovl->s,INVALID_SOCKET);
		shutdown(s,SD_BOTH);
          closesocket(s);
	}


	inline  void  operator()
		( com_variant_t* presult,raise_error_proc error,int argc,com_variant_t* argv,int arglast,HRESULT& hrout,void* pei,unsigned flags)
	{

		bool fmta=free_com_factory::is_MTA();

        hssh_t& hssh=json_rpc_t::hsslib();

		arguments_t varg(argc,argv);


		int masksr=varg[1].def<int>(0);
		fmo_stream_t fmo_stream_io(varg[0]);
		bool ffmo=SUCCEEDED(fmo_stream_io.hr);

		fmo_stream_t& fmo_stream=(ffmo)?fmo_stream_io:fmo_buf;
		INT64 cbtot=1;

		if(!ffmo) 
		{
			fmo_buf.reset();
			
			if( ((masksr&1)==0)&&(FAILED(hrout=fmo_buf.put_unicode(varg[0]))))						
				 return;
			
		}

         if((masksr&1)==0) 
		 {
			 cbtot=fmo_stream.send(hovl,hrout);
			 if(FAILED(hrout)) 
			 { 
				 disconnect();
				 return;
			 }
			 else hrout=0;
		 }

		 if(((masksr&2)==0)&&(cbtot>=0))
		 {
			 bool fok=false;
             fmo_stream.reset();
			 if(fv>0)
			 {
               fmo_stream.recv_frames(hovl,hrout);

              if(fok=SUCCEEDED(hrout)) hrout=0;
			  else 
			  {
				  disconnect();
				  return;
				   
			  }
				   
			 }
			 else			 
			 if((cbtot=wbsloader.recv())>0)
			 {
				 wbsloader>>fmo_stream;
				 fok=true;
				 //return;
			 }


             if(!fok) return;  

			 if(ffmo)
				 *presult=fmo_stream.frames_disp.Detach();
			 else hrout=fmo_buf.get_unicode(presult);


		 }

			  


   }

   
};



struct rpc_connect_t
{

	HRESULT hr;
	HSS_INFO* hovl;
	void *hion_port,*hsynchro_port;
		 mutex_cs_t mutex;
	
	wbs_getter_t wbs_getter;//(hovl);
	wbs_string_loader_t<wbs_getter_t,DIG> wbsloader;//(wbs_getter,1);



 variant_t script_holder;
 variant_t  rpc_function;
 free_com_factory::COMInit cominit;
 ltx_proc_table_helper ltx;
 exceptinfo_t ei;
 v_buf<char> err_report_buf;
 fmo_stream_t fmo_in,fmo_out;
 int cbrecv;
 json_rpc_t* owner;
 stub_holder stub;

 argv_zz<char> args;

 v_buf<char>   xdrchannels;
 
/*
 static  console_shared_data_t<wchar_t,0>& log()
 {

 }
*/

 struct single_json_rpc_t
 {
	 HSS_INFO* hovl;
	 
	 HRESULT hr;
	 free_com_factory::COMInit cominit;
	 ltx_proc_table_helper ltx;


	 exceptinfo_t ei;

	 single_json_rpc_t(HSS_INFO* _hovl)
		 :hovl(_hovl)
		 ,cominit(COINIT_MULTITHREADED)
	 {
		 hssh_t& hssh=hsslib(); 
		 if(SUCCEEDED(hr=cominit))
		 {
			 variant_t params,result;
			 BSTR pw=0; 
			 int c,cb;
			 char* pe,*p;
            int siz=get_double(hssh.reset_SOCKET_DATA(hovl,HSD_BODY_LENGTH,0,0,0));
			logmsg_attr(M_BC|0x0A,L"POST RPC Body-Lenght=%sBytes\n",GMKB(siz));
			if(siz>0)
			{
                 v_buf<char> buf(siz+8),bufd(siz+8);
				  pe=buf;
				  p=bufd;
				  //hovl->buffers[HSD_REQUEST_POS].len=0;
				 c=hssh.recv(hovl,pe,siz,0x10000);
				 if(c<=0) return;
				 url_unescape(pe,c,p,&cb);
				 c=cb;
				//p[c]=0;p[c+1]=0;
				 cb=MultiByteToWideChar(CP_UTF8,0,p,c+2,0,0);
       			if(cb)
				{
					
                    
					if(pw=SysAllocStringLen(0,cb))
					{
						cb=MultiByteToWideChar(CP_UTF8,0,p,c+2,pw,cb);
						params.bstrVal=pw;
						params.vt=VT_BSTR;

					}
					else {hr= E_OUTOFMEMORY;return;};
				}
				if(!cb) { hr=HRESULT_FROM_WIN32(GetLastError());return;};

			} else return;

			if(FAILED(hr=ltx.script(&result,L"json_rpc_server($0)",params)))
					  return;
			params.Clear();
			char_mutator<CP_UTF8> cm(result.bstrVal);

			cb= cm.char_count();
			 p=cm;
			 hssh.send(hovl,p,cb,0);
			 logmsg_attr(M_BC|0x0A,L"POST RPC Reply=%sBytes\n",GMKB(cb));
			 

		 };
			 
        
	 }

	 inline HRESULT init()
	 {
		 
         //CHECK_FAIL_hr(ltx->ltx_parse_bind(L"sta:script",script_holder.GetAddress()));
		 variant_t res;

		 //hr=ltx.script(&res)

	 }

 };


 static HRESULT invoke_dflt(VARIANT& vdisp, int argc,VARIANT* argv,VARIANT* pres=0,EXCEPINFO* pei=0)
 {
	 HRESULT hr;
	 DISPPARAMS dp={argv,0,argc,0};
     variant_t tmp;
	 pres=(pres)?pres:&tmp;
	 hr=vdisp.pdispVal->Invoke(DISPID_VALUE,IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_METHOD,&dp,pres,pei, NULL);
	 return hr;
 }

 static HRESULT invoke_dflt2(variant_t& vdisp,variant_t* res,EXCEPINFO* pei,variant_t v1,variant_t v2=variant_t())
{

	 return invoke_dflt(vdisp,2,&v1,res,pei);
}
 
 inline HRESULT init()
 {
	 hssh_t& hssh=hsslib(); 

	 v_buf<wchar_t> buf,buf2;
	 v_buf<wchar_t> cbchannels,ns_ion_port;
	 v_buf<char> xdrhold;

	 CHECK_FAIL_hr(cominit);

	 char* paramszz=hssh.reset_SOCKET_DATA(hovl,HSD_PARAMS,0,0,0);	 

	 args.init(paramszz);

	 const char* pfunn= args["rpc_proc"].def("fmo_json_rpc_server");
	  char* pinit= args["rpc_init"].def<char*>(0);


			
     

	 

	 CHECK_FAIL_hr(ltx->ltx_parse_bind(L"sta:script",script_holder.GetAddress()));

	 {
		// variant_t vv[3]={script_holder,variant_t(L"__script__= $0; 10")},r;
        //  CHECK_FAIL_hr(invoke_dflt(script_holder,2,vv,&r,ei.reset()));

	 }

	 if(pinit)
	 {
		 variant_t tmp;
		 CHECK_FAIL_hr(invoke_dflt(script_holder,1,&variant_t((wchar_t*)char_mutator<CP_UTF8>(pinit)),tmp.GetAddress(),ei.reset()));
	 }


	 wchar_t* pions;
	 
	   json_rpc_t* owner=(json_rpc_t*)hovl->user_data;


	   if(!owner->fcloned) pions=owner->ns_ion_port.get();
	   else
	   {
		   pions=ns_ion_port.printf(L"%s.ion[%d]",owner->ns_port.get(),GetCurrentProcessId());
		   hion_port=hion_port=hssh.ion_channels(pions); 
	   }

	 
	 

	 



	 char_mutator<CP_UTF8> cmhost(hssh.reset_SOCKET_DATA(hovl,HSD_HOST,0,0,0));
	 wchar_t* phost=cmhost; 
	 char_mutator<CP_UTF8> cmlocalhost(hssh.reset_SOCKET_DATA(hovl,HSD_LOCAL_INFO,0,0,0));
	 wchar_t* plh=cmlocalhost; 
	 cbchannels.printf(L"{url:['%s','%s'],lurl:['%s','%s']}",phost,pions,plh,pions);


	 buf.printf(L"__callback_channel_url__=callback_channel=%s",(wchar_t*) char_mutator<CP_UTF8>(cbchannels.get()));
	 HRESULT hr0=invoke_dflt(script_holder,1,&variant_t(buf.get()));


	 wchar_t* wfun=buf.printf(L"[%s][0]",(wchar_t*)char_mutator<CP_UTF8>(pfunn));   
	 CHECK_FAIL_hr(invoke_dflt(script_holder,1,&variant_t(wfun),rpc_function.GetAddress(),ei.reset()));

      int openflag;
     if(openflag=args["__on_open__"].def<int>(0))
	 {
           argv_zzs<char> repl_ref;
           stub=stub_holder(script_holder.punkVal);
		   repl_ref["script_objref"]=(char*)stub.objref();
		   repl_ref["callback_channel_url"]=(char*)char_mutator<CP_UTF8>(cbchannels.get());

		   if(openflag&2)
		   {
                char* pport=xdrchannels.printf("::%s",str_uuid().get());

				hsynchro_port=hssh.create_socket_port(pport,HSF_SRD_AUTO,&s_on_xdr_accept,(void*)this);



				
				
               xdrhold.printf("{url:['%s','%s'],lurl:['%s','%s']}",(char*)cmhost,pport,(char*)cmlocalhost,pport);  
			   repl_ref["xdr_channel_url"]=xdrhold.get();
		   }

		   int cb;
		    char* p=(char*)repl_ref.flat_str(0,0,&cb);
	       hr=hssh.write_wbs_frame(hovl,p,&cb,WSF_FIN|WSF_TXT,0); 

	 }
	 


 }

  void on_xdr_accept(HSS_INFO* hovl)
  {
	  HRESULT hrerr;
	  INT64 cbtot;
	  bool f;

	  hssh_t& hssh=hsslib(); 
	  if(FAILED(hr)) return ;

	  

	  ULONG st;
	  HRESULT hr;
	  DWORD tic,tic0;


	  st=hssh.HTTP_request_headers_chain(hovl);

	  if(check_post(hovl))
	  {
            locker_t<mutex_cs_t> lock(mutex);		
			fmo_out.reset();
			fmo_in.reset();

			if((cbtot=fmo_in.recv_http_body(hovl,hrerr))>=0)
		 {
			 tic=GetTickCount();
			 logmsg_attr(0x0f,L"recv_POST=%s\n",GMKB(cbtot));
			 VARIANT vparams[2]={fmo_out.frames_disp,fmo_in.frames_disp};
			 variant_t res;
			 tic0=GetTickCount();
			 hrerr=invoke_dflt(rpc_function,2,vparams,&res,&ei);
             tic0=GetTickCount()-tic0;
			 cbtot=fmo_out.send_http_body(hovl,hr);
			 tic=GetTickCount()-tic;
			 logmsg_attr(0x0f,L"send_POST=%s [%dms %dms]\n",GMKB(cbtot),tic0,tic);
		 }		



	  }
  

  }
 static  void  __stdcall s_on_xdr_accept(HSS_INFO* hovl)
 {
    if(hovl&&hovl->user_data)
		((rpc_connect_t*)hovl->user_data)->on_xdr_accept(hovl);

 }
 
 rpc_connect_t(HSS_INFO* _hovl)
	 :hovl(_hovl)
	 ,wbs_getter(hovl)
	 ,wbsloader(wbs_getter,1)
	 ,cominit(COINIT_MULTITHREADED)
	 ,fmo_in(ltx),fmo_out(ltx)
	 ,owner((_hovl)?(json_rpc_t*)_hovl->user_data:0)
	 ,hion_port(0),hsynchro_port(0)
 {
	 if(SUCCEEDED(hr=cominit)&&SUCCEEDED(hr=fmo_in.hr)&&SUCCEEDED(hr=fmo_out.hr))
		    init(); 

 }

~rpc_connect_t()
{
	if(hion_port)
		hsslib().release(hion_port);
	if(hsynchro_port)
		hsslib().release(hsynchro_port);
	
}
inline  bool loop_once()
 {
	     locker_t<mutex_cs_t> lock(mutex);

         bool f;
		 hssh_t& hssh=hsslib(); 
		 if(FAILED(hr)) return false;
		 
		 if((cbrecv=wbsloader.recv())<=0) return false;
		 fmo_out.reset();
		 fmo_in.reset();
		 wbsloader>>fmo_in;
		 VARIANT vparams[2]={fmo_out.frames_disp,fmo_in.frames_disp};
		 variant_t res;
		 HRESULT hrerr;
		  hrerr=invoke_dflt(rpc_function,2,vparams,&res,&ei);

		  INT64 cbtot=fmo_out.send(hovl,hr);

		  return cbtot>0;



		  /*
		  int cbout;
		  char* p;
		  unsigned char wh[2]={0,0xff};

		  INT64 cbtot=0;
		  hssh.send(hovl,wh,1,0); 			
		  while( (cbout=fmo_out.get_next_frame(bufsize,&p))>0 )
		  {
			  cbout=hssh.send(hovl,p,cbout,0); 			
			  if(cbout<=0) 
				  return false;
			  cbtot+=cbout;

		  }
		  hssh.send(hovl,wh+1,1,0); 			
		  return cbtot>0;
		  */

 }

inline  bool loop_once_new()
 {
	     HRESULT hrerr;
		 INT64 cbtot;
         bool f;
		 hssh_t& hssh=hsslib(); 
		 if(FAILED(hr)) return false;

		 locker_t<mutex_cs_t> lock(mutex);
		 
		 
		 fmo_out.reset();
		 fmo_in.reset();
		 //if((cbrecv=wbsloader.recv())<=0) return false;
		 //wbsloader>>fmo_in;
         
		 if((cbtot=fmo_in.recv_frames(hovl,hrerr,&mutex))>=0)
		 {
			 
			 logmsg_attr(0x0f,L"recv_frames=%s\n",GMKB(cbtot));
			 DWORD tic=GetTickCount(),tic0=GetTickCount();
		   VARIANT vparams[2]={fmo_out.frames_disp,fmo_in.frames_disp};
		   variant_t res;
		    hrerr=invoke_dflt(rpc_function,2,vparams,&res,&ei);
          tic0=GetTickCount()-tic0;
		  cbtot=fmo_out.send(hovl,hr);
		  tic=GetTickCount()-tic;
		  logmsg_attr(0x0f,L"send_frames=%s [%dms %dms]\n",GMKB(cbtot),tic0,tic);
		 }

		  return cbtot>=0;



 }


};


template <class CH>
static void logmsg(const CH * fmt,...)
{
	static console_shared_data_t<wchar_t,0> shared_console;
	va_list argptr;
	va_start(argptr, fmt);
	v_buf<CH> buf;
	//safe_cputs(buf.vprintf(fmt,argptr).get());
	shared_console.puts(buf.vprintf(fmt,argptr).get());
	if(!filelog().empty())
		append_to_file(filelog().get(),char_mutator<CP_THREAD_ACP>(buf.get()));		
}

template <class CH>
static void logmsg_attr(unsigned attr,const CH * fmt,...)
{
	static console_shared_data_t<wchar_t,0> shared_console;
	va_list argptr;
	va_start(argptr, fmt);
	v_buf<CH> buf;
	//safe_cputs(buf.vprintf(fmt,argptr).get());
	shared_console.puts(buf.vprintf(fmt,argptr).get(),attr);
	if(!filelog().empty())
		append_to_file(filelog().get(),char_mutator<CP_THREAD_ACP>(buf.get()));		
}


static  void   on_clone_process_terminate(void*p,PROCESS_INFORMATION* ppi)
{
  DWORD ec=-1;
  GetExitCodeProcess(ppi->hProcess,&ec);
  DWORD attr=(ec)?(0xdf):(0x0c);
  logmsg_attr(attr,L"clone process[%d] terminated:{%d} \n",ppi->dwProcessId,ec);

}
static  void  __stdcall on_hss_js_accept_clone(HSS_INFO* hovl)
{
	hssh_t& hssh=hsslib(); 
    HRESULT hr;
	argv_zzs<wchar_t> argpi;

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if(GetConsoleScreenBufferInfo(HAC,&csbi))
	{
		SMALL_RECT & w=csbi.srWindow;
		argpi[L"console.winrect"]=v_buf<wchar_t>().printf(L"[%d,%d,%d,%d]",int(w.Left),int(w.Top),int(w.Right),int(w.Bottom)).get();
		WSABUF& wsa=hovl->buffers[HSD_PROCESS_INFO];
		wsa.buf=(char*)argpi.flat_str(0,0,(int*)&wsa.len);
		wsa.len*=sizeof(wchar_t);

	}

	hr=hssh.to_clone_process_ex(hovl,&on_clone_process_terminate,0);
	if(hr)
	{
		logmsg_attr(0xad,L"to_clone_process error[%x]:%s\n",hr,error_msg(hr).get());
	}
	else logmsg_attr(0x0f,L"clone process[%d] starting...\n",hovl->error_code);

}

inline static  bool check_post(HSS_INFO* hovl)
{
	WSABUF& r=hovl->buffers[HSD_REQUEST];
	return (hovl&&(r.buf)&&(r.len>6)&&(StrCmpNIA(r.buf,"POST",4)==0));
}

  
 static  void  __stdcall on_hss_js_accept(HSS_INFO* hovl)
 {
     hssh_t& hssh=hsslib(); 

	 ULONG st;
	 HRESULT hr;

	 //st=hssh.websocket_handshake_chain(hovl); 
	 {
	  //hssh.to_clone_process(hovl);
		//  return ;
	 }
	 //char *ppex=hssh.reset_SOCKET_DATA(hovl,HSD_WS_EXTENSION,HSF_SET,"deflate-stream",-1);
	 char *ppex;
	 //ppex=hssh.reset_SOCKET_DATA(hovl,HSD_WS_EXTENSION,HSF_SET,"deflate-stream",-1);
	 //ppex=hssh.reset_SOCKET_DATA(hovl,HSD_WS_EXTENSION,HSF_SET," ",-1);
	 st=hssh.HTTP_request_headers_chain(hovl);
	 if(check_post(hovl))
	 {
		 rpc_connect_t::single_json_rpc_t single_json_rpc(hovl);
	   return;
	 }
	 st=hssh.websocket_handshake_reply(hovl);
	 if(!(hovl->reply_state&1)) return ;


	 char* ra=hssh.reset_SOCKET_DATA(hovl,HSD_REMOTE_INFO,0,0,0);
	 logmsg_attr(M_BC|0x0e,"connect[%s]\n",ra);

     rpc_connect_t rpc(hovl);

	 int fv=hssh.wbs_version(hovl);

      

	 if(fv<=0) 
		 while(rpc.loop_once());
	 else while(rpc.loop_once_new());

	 logmsg_attr(M_BC|0x0e,"disconnect[%s]\n",ra);

 }

 ltx_hss_json_rpc_t(wchar_t* nsport,bool fcp=false):hport(0),hion_port(0)
 {
	   HSS_INFO* hovl=0;
	   HRESULT hr;
	   void* pcallback;
	     //debugger_point_t dbg(1);
	       hssh_t& hssh=hsslib(); 

		   fcloned=hssh.is_clone_process();

		   ns_port<<nsport;
		   //ns_ion_port.printf(L"%s.ion[%d]",nsport,GetCurrentProcessId());

		 
   
	    if(fcp&&fcloned)
		{
			hr=hssh.from_parent_process(&hovl);
			
			if(hr==S_OK) 
			{
                if(is_console())
				{

					wchar_t* pw;
					argv_zz<wchar_t> argpi(pw=(wchar_t*)hssh.reset_SOCKET_DATA(hovl,HSD_PROCESS_INFO,0,0,0));

					std::vector<SHORT> vr=argpi[L"console.winrect"];
					//logmsg_attr(M_BC|0x1F,L"console.winrect_is =%s \n",pw);
					if(vr.size()==4)
					{
						bool ff=SetConsoleWindowInfo(HAC,1,(SMALL_RECT*)&vr[0]);
						//logmsg_attr(0xF1,"console.winrect=[%d,%d,%d,%d]:%d\n",vr[0],vr[1],vr[2],vr[3],ff);
					}
					
				
				}
				
				//argpi[L"console.winrect"]

				hovl->user_data=ULONG_PTR((void*)this);
				on_hss_js_accept(hovl);
			}
			
		}
		else
		{
	     pcallback=(fcp)?on_hss_js_accept_clone:on_hss_js_accept; 	 

		 ns_ion_port.printf(L"%s.ion",nsport);

         hion_port=hssh.ion_channels(ns_ion_port.get()); 

		 hport=hssh.create_socket_portW(ns_port.get(),HSF_SRD_AUTO,pcallback,(void*)this);
		 //


		}
		   
 }
 ~ltx_hss_json_rpc_t()
 {
   
   if(hport) hsslib().release(hport);
   if(hion_port) hsslib().release(hion_port);
   
   
 }
 
 DWORD wait(DWORD tio=INFINITE)
 {
    return hsslib().wait_signaled(hport,-1);
 }
   void * hport,*hion_port;
   v_buf<wchar_t> ns_port,ns_ion_port;
   bool fcloned;

   
};