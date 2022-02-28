#pragma once

#include "ipc_ports/hssh.h"
#include "video/screencapture.h"
#include "video/lasercalibrate.h"
#include <video/webcam_control.h> 
#include "wchar_parsers.h"
#include "conio.h"
#include <string>

#include "video/ishared_image_impl.h"


struct hssh_desktop_base
{
	typedef ULONG( *cx_convert_t)(const char*sext,int scb,void* sbuf,const char*dext,int* pdcb,void* dbuf,void * popt);
	typedef _non_copyable::CStopwatch  stopwatch_t;



static    int send_ex(SOCKET s,void *p,int cb)
	 {
		 WSABUF wsa={cb,(char*)p};
		 DWORD cbt;
         if(WSASend(s,&wsa,1,&cbt,0,0,0))
			 return -1;
		 return cbt;
	 }

	struct bin_image_t
	{
		long crc32;
		char ext[8];
		POINT cursor;
		long  kfmask;
		long  rsrv0,rsrv1;
		long size;
	};

	struct sockgetter
	{
		SOCKET s;
		sockgetter(SOCKET _s=INVALID_SOCKET):s(_s){};
		inline int operator()(void* p,int count,int flags=0) const
		{
			return ::recv(s,(char*)p,count,flags);
		}
	private:
		sockgetter(sockgetter& a){};
		sockgetter(const sockgetter& a){};
	};

	struct socksetter
	{
		SOCKET s;
		socksetter(SOCKET _s=INVALID_SOCKET):s(_s){};
		inline int operator()(void* p,int count,int flags=0) const
		{
			int cbtot=0,c;
			char*pc=(char*)p;
			while(count>0)
			{
             c=::send(s,pc,count,flags);
			 if(c<=0) return c;
			 pc+=c;
			 count-=c;
			 cbtot+=c;
			};
			

			return cbtot;
			//return send_ex(s,p,count);

		}
	private:
		socksetter(socksetter& a){};
		socksetter(const socksetter& a){};
	};

	hssh_t hssh;
	cx_convert_t cx_convert;

static	int wb_sent(SOCKET s,std::strstream& stream_out )
	{
		int cb;
		char cz=0,cfin=0xFF;
        socksetter sset(s);
		stream_out<<cz<<cz;
		char* pout=stream_out.str();
		int cbout=stream_out.pcount();
		// test ..
		cb=sset(&cz,1);
		if(cb<=0) return -1;
		if(cbout) cb=sset(pout,cbout);
		if(cb<=0) return -1;
		cb=sset(&cfin,1);
		if(cb<=0) return -1;
		return cbout;
	}

  
HRESULT load_converter()
{
    wchar_t* libn= _wgetenv(L"cximg_converter");

	HMODULE hlib=LoadLibraryW((libn)?(libn):L"cximg_converter.dll");
	if(!hlib) 
   { 
	    v_buf<wchar_t> buf;
		if(!(hlib=LoadLibraryW(argv_ini<wchar_t>().get_module_path(buf).cat(L"cximg_converter.dll"))))
		return HRESULT_FROM_WIN32(GetLastError());
	}
	cx_convert=(cx_convert_t)GetProcAddress(hlib,"cx_convert");
	return S_OK;

}

};

struct hssh_desktop_client:hssh_desktop_base
{
	SOCKET s;
	sockgetter sget;
	socksetter sset;
	RECT rw,rscr; 
	int fhttpget;
	ISharedImage* pshared_image;

	hssh_desktop_client():s(INVALID_SOCKET)
	{
		pshared_image=0;
		RECT r={};
		rw=r;
		rscr=r;
	}

	hssh_desktop_client(const wchar_t* host_port,const wchar_t* argszz=0,const wchar_t* ns_path=_wgetenv(L"laser.hook.hssh_port")):s(INVALID_SOCKET)
	{
		pshared_image=0;
		RECT r={};
		rw=r;
		rscr=r;
		reconnect(host_port,argszz,ns_path);
	}
hssh_desktop_client& reconnect(const wchar_t* host_port,const wchar_t* argszz=0,const wchar_t* ns_path=_wgetenv(L"laser.hook.hssh_port"))
	{

		argv_zz<wchar_t> test(argszz);
		fhttpget=0;
       close();
		hssh.lock_forever();
		s=hssh.websocket_connect_exW((wchar_t*)host_port,(wchar_t*)ns_path,(wchar_t*)argszz,0);
		if(s==INVALID_SOCKET)
		{
			s=hssh.tcp_connectW(host_port,0);
			fhttpget=1;
		}
		sget.s=s;
		sset.s=s;
		return *this;
	}
     
void     close(){
	         if(s!=INVALID_SOCKET)
		     ::closesocket(make_detach(s,INVALID_SOCKET));	

			 if(pshared_image) make_detach(pshared_image)->Release();

			}

	~hssh_desktop_client()
	{
		close();
	}

//send_stream()argv_zz<char> args_in(stream_in.str());
bool	get_monitor_rect(RECT& r)
	{
		std::strstream stream_in,stream_out;  
		stream_out<<"mode=1"<<char(0);
		stream_out<<"get.rect=1";
		int cb=wb_sent(s,stream_out);
		if(cb>0)
		{
           cb=get_wbs_string(stream_in,sget);
		   if(cb>0)
		   {
             argv_zz<char> args_in(stream_in.str());
			 std::vector<double> vr=args_in["rect"];
			 if(vr.size()<4) return -7;
			 r.left  = vr[0];			 
			 r.top   = vr[1];
			 r.right = vr[2];
			 r.bottom = vr[3];
		   }

		}
    	return cb>0;
	}
operator bool()
{
	return s!=INVALID_SOCKET;
}

inline bool  reset_monitor_rect()
{
	
	if(get_monitor_rect(rscr))
	{
		RECT t={0,0,rscr.right-rscr.left,rscr.bottom-rscr.top};
		rw=t;
		return true;
	}
	return false;
}


};

struct hssh_webcam_control_client:hssh_desktop_client
{
	char* host_port,*ns_path;
	argv_zzs<char> args;
	bool fnoreplay;

	hssh_webcam_control_client(){};
	template <class ARGS>
	hssh_webcam_control_client(ARGS& args)
	{
		init(args);
	}
	template <class ARGS>
	hssh_webcam_control_client& init(ARGS& _args)
	{
		
		{
			argv_zzs<ARGS::char_type> a,f,e,i;
				a=_args;
			argv_file<ARGS::char_type> af(a[char_mutator<>("@")].def<ARGS::char_type*>());
			f=af;
			a.parent=&f;
			f.parent=&e;
			e.parent=&i;
			if(a[char_mutator<>("env")].def(0))
			{
				e=argv_env<wchar_t>();
			}
            i=argv_ini<wchar_t>(L"webcamcontrol",L".ini"); 

			args=a;

		}

		    fnoreplay=args["noreplay"].def(0);

		   host_port=args["ip"].def((char*)0);


		   if(!host_port) return *this;

		argv_env<char> env;
		char* portname=env["laser.hook.hssh_port"];
			
        ns_path=args["hsport"].def(portname);

		reconnect(char_mutator<>(host_port),L"webcamcontrol=1\0\0",char_mutator<>(ns_path));	
		//open_shared_image(args[L"key"],args[L"pathname"]);
		return *this;
	}

	std::string	reset(char *delim="\n")
	{
		if(host_port==0)
			return CamControl_reset(args,delim);
		else 
		{
			if((*this))
			{
              int cb;
			  const char* p=args.flat_str();
			  std::strstream stream_out((char*)p,strlen(p)),stream_in;
			  cb=wb_sent(s,stream_out);
			  if(cb<=0) return "";
			  if(fnoreplay) return "";
			  cb=get_wbs_string(stream_in,sget);
			  if(cb<=0) return "";
			  argv_zz<char> argout(stream_in.str());
			  return argout.flat_str(0,delim);
		  	}
			else return "";
		}
	}

};

struct hssh_desktop_client_image:hssh_desktop_client
{
	
	

	enum{
		BUFFERSIZE= (16*1024*1024) 
	};

	enum    
	{
		alpha_denom_dig = 12 ,
		alpha_denom =1<<alpha_denom_dig

	};

inline	void blend_delta_pt(const BYTE* pline_src,BYTE* pline_dest,DWORD a,DWORD b,DWORD* pdwdest)
	{
		DWORD v;
		v= (b*(*pdwdest))>>alpha_denom_dig;
		v= v+a*(*pline_src);
		*pdwdest=v; 
		*pline_dest=v>>alpha_denom_dig;
	}


	//cx_convert_t cx_convert;

    BMPMap bmpmap;
	long crc32;
	v_buf<char> buf;
    v_buf<char> buf_kf;
	v_buf<char> tmp_buf,tmp_buf2;
	v_buf<char> getv;
	v_buf<DWORD> buf_blend;
	BITMAPFILEHEADER* img_ptr;
	double fps,gamma;
    long itemsize;
	long kf_period,kf;
	int cb;
	
	asyn_su::thread_queue image_queue;
		stopwatch_t swt;

	int nmon;
	v_buf<wchar_t> vhost_port,vns_path,vkey,vpath;
	v_buf<char>	vfmt,vfmtout;

	int fbin;

	hssh_desktop_client_image():gamma(0)
	{
		buf.resize(BUFFERSIZE);
	};

	hssh_desktop_client_image
		(const wchar_t* host_port,int _nmon=0,const wchar_t* ns_path=_wgetenv(L"laser.hook.hssh_port"),bool _fbin=true)
		:gamma(0)
	{
		buf.resize(BUFFERSIZE);
		init(host_port,_nmon,ns_path);
	}

template <class ARGS>
	hssh_desktop_client_image(ARGS& args):gamma(0)
	{
		buf.resize(BUFFERSIZE);
		init(args);
	};

	hssh_desktop_client_image& init(const wchar_t* host_port,int _nmon=0,const wchar_t* ns_path=_wgetenv(L"laser.hook.hssh_port"),bool _fbin=true)
	{
		wchar_t *fmt=L"jpg";
		fbin=_fbin;
		wchar_t  wcz=0;
		std::wstringstream wstr;
		wstr<<L"binary="<<fbin<<wcz;
		wstr<<L"nmon="<<nmon<<wcz;
        wstr<<L"pack.fmt="<<fmt<<wcz<<wcz;
		vfmt.cat(char_mutator<>(fmt));
		std::wstring sf=wstr.str();
		reconnect(host_port,sf.c_str(),ns_path);	
		
		return *this;
	}
 

	template <class ARGS>
	hssh_desktop_client_image& init(ARGS& args)
  {
	    char* pgv;
	  argv_env<wchar_t> env;
	  wchar_t* portname=env[L"laser.hook.hssh_port"];

		fbin=args[L"binary"].def(0);
		nmon=args[L"nmon"].def(0);
		vfmt.cat(char_mutator<>(args[L"pack.fmt"].def(L"png")));
		vfmtout.cat(char_mutator<>(args[L"unpack.fmt"].def(L"yuy2")));
		getv.printf("GET /%s HTTP/1.1\r\nConnection: keep-alive\r\n\r\n",(char*)char_mutator<>(args[L"getv"].def(L"")));
		pgv=getv;

		vhost_port.cat(args[L"ip"].def(L"localhost:7777"));
		
		//vns_path.cat(args[L"monitor.namespace"].def(portname));
		vns_path.cat(args[L"hsport"].def(portname));
		vkey.cat(args[L"key"]);
		vpath.cat(args[L"path"]);
		fps=args[L"fps"].def(5);
	    itemsize=args[L"itemsize"].def(4*1024);
		kf=0;
		if(args[L"keyframe.mode"].def(0))
		  kf_period=args[L"keyframe.period"].def(1);
		  else kf_period=1;

		if(kf_period<1) kf_period=1;

		if(kf_period>1) buf_kf.resize(BUFFERSIZE);

		if((gamma=args[L"gamma"].def<double>(0))>0)
		{
           buf_blend.resize(BUFFERSIZE);   
		};

		 
		//
		reconnect(vhost_port,args.flat_str(true),vns_path);	
		//reconnect(vhost_port,0,vns_path);	


       //open_shared_image(args[L"key"],args[L"pathname"]);

		return *this;
	}

 

	template <class Getter>
static int  read(Getter& sget,void * buf,int cb)
	{
		char *p=(char *)buf;
		int cbr=cb;
		while(cbr>0)
		{
			int c=sget(p,cbr);
			if(c<=0) return c;
			cbr-=c;
			p+=c;
		}
		return cb;
	}



bool load_image_httpget2(long *pcrc32)
{

	struct region_t
	{
		char* ptr;
		int sizeb;
	};
	HRESULT hr,hrp=WSA_IO_PENDING,hrrecv;
	if(tmp_buf.size_b()<BUFFERSIZE) tmp_buf.resize(BUFFERSIZE);
	if(tmp_buf2.size_b()<BUFFERSIZE) tmp_buf2.resize(BUFFERSIZE);

    WSABUF wsa[2]={{tmp_buf.size_b(),(char*)tmp_buf},{tmp_buf2.size_b(),(char*)tmp_buf2}};
	wsa[0].buf=tmp_buf.get();
	wsa[1].buf=tmp_buf2.get();

	int cb_buf=BUFFERSIZE;
	bool fyuy2=safe_cmpni(vfmtout,"yuy2",5)==0,ffill=0;

	int cb,cbr=0,c, err,cbrfull=0;

	SOCKET s=sget.s;

		//char  pgetstr[]="GET /image.jpg?1 HTTP/1.1\n\n";
	//	char  pgetstr[]="GET /liveimg.cgi?9999 HTTP/1.1\n\n";
	
	
	//cb=sset(pgetstr,sizeof(pgetstr)-1);
	
	char * pget=getv;
	char buuu[1024+11];
	WSABUF www={1024,buuu};
	www.len=1024;
	www.buf=buuu;
	DWORD flags;
	
    for(;;)
	{

	
	cb=sset(pget,getv.size_b()-1);
	if(cb<0) return false;
    OVERLAPPED ovl={};
	SetLastError(0);
	
    err=WSARecv(s,wsa,1,(DWORD*)&cb,&(flags=0),&ovl,NULL);
	hrrecv=WSAGetLastError();
	//
	//err=WSARecv(s,&www,1,&cdw,0,&ovl,NULL);
	//err=recv(s,www.buf,www.len,0);

	
	if(ffill)
	{
		char* pfull=(char* )wsa[1].buf;
		char *pimage=find_http_end(pfull,cbrfull);
		if(!pimage) return false;
		int cbn=int(pimage)-int(pfull);
		region_t r={pfull,cbn};
		cb=cbrfull-cbn;

		char* pcl=_GET_HEADER(Content-Length,r,&c);
		if((!pcl)||((cbr=atoi(pcl))<=0)) return false;
		int cbadd=cbr-cb;
		if(cbadd>0)
		{
          OVERLAPPED ovl={};
		  WSABUF wadd={cbadd,pimage+cb};
		  err=WSARecv(s,&wadd,1,(DWORD*)&cb,&(flags=0),&ovl,NULL);
		  hrrecv=WSAGetLastError();
		  if((err!=0)&&(hrrecv!=WSA_IO_PENDING)) 	return false;
		  if(!GetOverlappedResult(HANDLE(s),&ovl,(DWORD*)&cb,1))
			  return false;

		}

		int cbout=BUFFERSIZE;
		char* pbuf0=buf;
		char* pbuf=pbuf0;
		if(!cx_convert("jpg",cbr,pimage,"bmp",&cbout,pbuf0,0))
			return false;
		if(fyuy2)
		{
			pbuf=tmp_buf.get();
			cbr=make_detach(cbout,BUFFERSIZE);
			if(!cx_convert("bmp",cbr,pbuf0,"yuy2",&cbout,pbuf,0))
				return false;

		}

		
		if(!pshared_image)
		{
			ISharedImage_impl<>::CreateInstance(&pshared_image);  
			hr=pshared_image->Init(vkey,pbuf);
			if(hr) return false;

		}
		hr=pshared_image->PushFrame(pbuf);

		if(hr)	return false;
	}
	if((err!=0)&&(hrrecv!=WSA_IO_PENDING)) 	return false;
	else
	{
		
		
		if(!GetOverlappedResult(HANDLE(s),&ovl,(DWORD*)&cbrfull,1))
			return false;
		wsa[0]=make_detach(wsa[1],wsa[0]); 
		ffill=1;
		;
	}
	
    
	}


	return false;
}

//*/

bool make_shared_image(char* p,int cbr,bool fyuy2)
{
	int cbout=BUFFERSIZE;
	char* pbuf0=buf;
	char* pbuf=pbuf0;
	if(!cx_convert("jpg",cbr,p,"bmp",&cbout,pbuf0,0))
		return false;
	if(fyuy2)
	{
		pbuf=tmp_buf2.get();
		cbr=make_detach(cbout,BUFFERSIZE);
		if(!cx_convert("bmp",cbr,pbuf0,"yuy2",&cbout,pbuf,0))
			return false;

	}

	HRESULT hr;
	if(!pshared_image)
	{
		ISharedImage_impl<>::CreateInstance(&pshared_image);  
		hr=pshared_image->Init(vkey,pbuf);
		if(hr) return false;

	}
	hr=pshared_image->PushFrame(pbuf);
   return !hr;
}


struct item_get_t
{
	char* p;
	int cb;
	bool fyuy2;
	hssh_desktop_client_image* pdci;

	item_get_t(hssh_desktop_client_image* _pdci,char* ps,int cbs,bool _fyuy2):pdci(_pdci),cb(cbs),fyuy2(_fyuy2)
	{
		p=(char*)::malloc(cbs);
		memcpy(p,ps,cbs);
	};
	~item_get_t(){free(p);}
	inline   void operator()()
	{
		pdci->make_shared_image(p,cb,fyuy2);
	}


};


bool load_image_httpget(long *pcrc32)
{
	struct region_t
	{
		char* ptr;
		int sizeb;
	};
     if(pcrc32) *pcrc32+=2;
	 if(tmp_buf.size_b()<BUFFERSIZE) tmp_buf.resize(BUFFERSIZE);
	 if(tmp_buf2.size_b()<BUFFERSIZE) tmp_buf2.resize(BUFFERSIZE);
	 
	 //char  pgetstr[]="GET /image.jpg?1 HTTP/1.1\n\n";
	 //
	 char  pgetstr[]="GET /liveimg.cgi?9999 HTTP/1.1\n\n";
	 int cb,cbr,c;
	 int cb_buf=tmp_buf.size_b();
         //cb=sset(pgetstr,sizeof(pgetstr)-1);
     bool fyuy2=safe_cmpni(vfmtout,"yuy2",5)==0;
	 
	  char * pget=getv;
      cb=sset(pget,getv.size_b()-1);
	 if(cb>0)
	 {
		 
		 
		 
		 char* p=tmp_buf.get();
		 
		 std::strstream stream(p,cb_buf);
		 cbr=get_recv_lines(stream,sget);
		 region_t r={p,cbr};
		  if(cbr<=0) return false;
     	  char* pcl=_GET_HEADER(Content-Length,r,&c);
		   //c=0;
		   if((!pcl)||((cbr=atoi(pcl))<=0)) return false;
		   
		   if((c=read(sget,p,cbr))!=cbr) 
			     return false;

		   	     //
if(1)
{
		   item_get_t * pitem=new item_get_t(this,p,cbr,fyuy2);

		   //
		   image_queue.asyn_call(pitem);
		   //(*pitem)();		   delete pitem;

		   return true;
}		   
		   int cbout=BUFFERSIZE;
		   char* pbuf0=buf;
		   char* pbuf=pbuf0;
		   if(!cx_convert("jpg",cbr,p,"bmp",&cbout,pbuf0,0))
		     return false;
		   if(fyuy2)
		   {
			   pbuf=tmp_buf.get();
			   cbr=make_detach(cbout,BUFFERSIZE);
			   if(!cx_convert("bmp",cbr,pbuf0,"yuy2",&cbout,pbuf,0))
			       return false;
			   			   
		   }

		   HRESULT hr;
		   if(!pshared_image)
		   {
             ISharedImage_impl<>::CreateInstance(&pshared_image);  
			  hr=pshared_image->Init(vkey,pbuf);
			  if(hr) return false;

		   }
            hr=pshared_image->PushFrame(pbuf);
			
			return !(hr);
		   
		   //locker_t<BMPMap>  lock(bmpmap);
           //nt indx=bmpmap.get_last_index();    
           //		   memcpy(img_ptr,pbuf,cbout);
         
          
	 }



	return false;
}

bool load_image(long *pcrc32)
{

	//if(fhttpget) return load_image_httpget2(pcrc32);
	if(fhttpget) return load_image_httpget(pcrc32);
	
	std::strstream stream_out;
	int rc0,rc1,kfmm;
      
    double tn,tc;
      //crc32=0;
     stream_out<<crc32<<char(0);
	 
	 stream_out<<make_detach(kf,(kf+1)%kf_period)<<char(0);
	 	 

     cb=wb_sent(s,stream_out);
	 if(cb<=0) return false;

     stopwatch_t sw;
	 if(fbin)
	 {
		 sw.Start(); 
		 cb=0;
		 int cbi;
        bin_image_t bi;
	    cbi=sget(&bi,sizeof(bi));
		if(cbi<sizeof(bi))
			   return false;
		cb+=cbi;
		
		POINT pt=bi.cursor;

		tn=sw.Sec();
		bool fupdate=bi.crc32!=*pcrc32;
		*pcrc32=bi.crc32;
		if(fupdate)
		{
			char* p=buf.get();
             
           char *pc=p;
		   cb=0;
          do 
          {
			  cbi=sget(pc,bi.size-cb);
			  if(cbi<=0) return false;
			  cb+=cbi;
			  pc+=cbi;
          } 
		  while (cb<bi.size);
/*			
		  if(bi.size!=cb)
		  {
			  HRESULT hr=WSAGetLastError();
			  return false;
		  }
*/		  
         int cbs=BUFFERSIZE;
		 bool f;
		 
		 {
			 sw.Start(); 
			 locker_t<BMPMap>  lock(bmpmap);
			 char* pext;
			 if((strcmpi(bi.ext,"yuy2gz")==0)||(strcmpi(bi.ext,"bmpgz")==0)) pext="gzip";
			 else pext=bi.ext;

			 void* punz=(bi.kfmask)? (void*)buf_kf.get():(void*)img_ptr;

			 

			 f=cx_convert(pext,bi.size,p,"bmp",&cbs,punz,NULL);

			 BITMAPINFOHEADER& bih= bmpmap.header<BITMAPINFOHEADER>();
			 if(bi.kfmask)
			 {
				 int cb=0;
				 BYTE *pimz=pBMP_bits(img_ptr,&cb);
                 BYTE *pdelta=((BYTE *)punz)+int(pimz)-int(img_ptr);
				 
				 /*
				 cb=cbs;
				 pimz=(BYTE*)img_ptr;
				 pdelta=(BYTE*)punz;
				 */
				 

				 for(int n=0;n<cb;n++)
				 {
					 pimz[n]+=pdelta[n];
				 }
				 

			 }
			 //POINT& cursor= *((POINT*)bmpmap.pAddData());
             //cursor=bi.cursor;   
			 

			 BMPMap::remote_control_data_t* rcd= bmpmap.remote_control_data(); 
             rcd->crc32=bi.crc32;
			 rcd->cursor=bi.cursor;

			 if(0)
			 {
				 rc0=bi.crc32;
				 rc1= bmpmap.get_crc32();
			kfmm=bi.kfmask;	 
			 }
              


		 }
		   tc=sw.Sec();
		  double cbk=bi.size/1024.0, cbuk=cbs/1024.0;

		  //_cprintf("kf=%d d_cr32=%x\n",kfmm,rc0-rc1);
		  _cprintf("[%d] xy:(%d,%d) pack[%s] size=%g KB unpack size=%g KB  net=%g sec convert=%g sec res=%d \n",bi.kfmask,pt.x,pt.y,bi.ext,cbk,cbuk,tn,tc,f);
		 return f;
		}
		else
		{
			if(0)
			{
				rc0=bi.crc32;
				rc1= bmpmap.get_crc32();
				kfmm=bi.kfmask;
			}

			locker_t<BMPMap>  lock(bmpmap);
			BMPMap::remote_control_data_t* rcd= bmpmap.remote_control_data(); 
			rcd->crc32=bi.crc32;
			rcd->cursor=bi.cursor;
			//POINT& cursor= *((POINT*)bmpmap.pAddData());
			//cursor=bi.cursor;   
		//	_cprintf("kf=%d d_cr32=%x\n",kfmm,rc0-rc1);
			_cprintf("[%d]xy:(%d,%d) net=%g sec %d bytes recv ... \n",bi.kfmask,pt.x,pt.y,tn,cb);
			return true;
		}
 	 }
  

	 std::strstream stream_in(buf.get(),BUFFERSIZE);
     sw.Start(); 
	 cb=get_wbs_string(stream_in,sget);
	 tn=sw.Sec();
	 

	if(cb>0)
	{
    	char* p=buf.get();
		long crc32N=atoi(p);
		int off=strlen(p);
		p+=off+1;
		cb-=off+1;
		int cbs=BUFFERSIZE;
       locker_t<BMPMap>  lock(bmpmap);

	    bool fupdate=crc32N!=*pcrc32;
	   
        *pcrc32=crc32N;
		if(!fupdate)
		{
        _cprintf(" net=%g sec ... \n",tn);
		  return true;
		}
	   sw.Start();
       bool f=cx_convert("base64",cb,p,"bmp",&cbs,img_ptr,NULL);
	   tc=sw.Sec();
	   double cbk=cb/1024.0, cbuk=cbs/1024.0;
	   _cprintf("pack size=%g KB unpack size=%g KB  net=%g sec convert=%g sec\n",cbk,cbuk,tn,tc);
	   return true;
	}
	
	return false;
}

bool 	open_shared_image()
{
  return open_shared_image(vkey,vpath);

}

bool 	open_shared_image(const wchar_t* key,const wchar_t* pathname=0)
{
       crc32=0;

	   //HMODULE hlib=LoadLibraryW(L"cximg_converter.dll");
	   //if(!hlib) return false;


	   if(load_converter()) return false;
	   
 
  
	   if(!fhttpget)
	   {

	   
	   bmpmap.open_key(key,pathname);
	  img_ptr=(BITMAPFILEHEADER*) bmpmap.reset(BUFFERSIZE);

	  if(!img_ptr) 
		   return false;
	   }
	  long newcrc32=-1;

	  if(!(*this)) 
		  return false;

	  if(load_image(&newcrc32))
	  {
		  if(newcrc32==crc32)
		  {
			  crc32=~crc32;
			  if(!load_image(&newcrc32))
				  return false;
		  }
         crc32= newcrc32;
	  }

	  if(0){

	  
	  if(!bmpmap.open_key(key,pathname))
		  return false;

	  img_ptr=(BITMAPFILEHEADER*)bmpmap.ptr;
	  }

       return img_ptr;
  //if()
				
}

bool  update()
{
	return load_image(&crc32);
}


bool loop_update()
{
	double tf=1./fps;
    double t;
	swt.Start();
	double fcbev=0,alpha=tf;
	double efps=0,_fps=fps;
    while(update())
	{
	 double dt=swt.Sec(),fcb=1000000000000,tt=max(dt,tf);
         
	 if(tt>0) {
		 fcb=  double(8*cb)/(tt*1024);
        _fps=1/tt;
	 }
	  fcbev=(1-alpha)*fcbev+alpha*(fcb);
	  efps=(1-alpha)*efps+alpha*(_fps);


	  //SetConsoleTitleA(v_buf<char>().printf("fps::kbits/sec: <%g>::<%g> %g::%g ",efps,fcbev,_fps,fcb));
	  SetConsoleTitleA(v_buf<char>().printf("fps: <%g> %g ",efps,_fps));
	 t=tf-dt;
	 swt.Start();
	 if(t>0) Sleep(t*1000);
	 
	}
	return 0;
}


};

struct hssh_desktop_client_control:hssh_desktop_client
{

	v_buf<wchar_t> vhost_port,vns_path;

     int nmon;
     int reconnect_mode;
	hssh_desktop_client_control(){reconnect_mode=0;};

	hssh_desktop_client_control
		(const wchar_t* host_port,int nmon,int _reconnect_mode=0,const wchar_t* ns_path=_wgetenv(L"laser.hook.hssh_port"))
	{
		reconnect_mode=0;
		init(host_port,nmon,ns_path);
	}

hssh_desktop_client_control& init(const wchar_t* host_port,int _nmon,const wchar_t* ns_path=_wgetenv(L"laser.hook.hssh_port"))
{
	vhost_port.clear().cat(host_port);
	vns_path.clear().cat(ns_path);
	nmon=_nmon;
	
	return _init(host_port,ns_path);
}

hssh_desktop_client_control& _init(const wchar_t* host_port,const wchar_t* ns_path=_wgetenv(L"laser.hook.hssh_port"))
	{
		
		wchar_t  wcz=0;
		std::wstringstream wstr;
		wstr<<L"control=1"<<wcz;
		//wstr<<L"winrect.bkcolor=0x0000ff"<<wcz;
		wstr<<L"nmon="<<nmon<<wcz<<wcz;
		
		reconnect(host_port,wstr.str().c_str(),ns_path);	
		if(!reset_monitor_rect())     close();
		return *this;
	}

inline operator bool()
{
	 return (reconnect_mode)||(s!=INVALID_SOCKET);
}
    

	inline int draw_detection(int  f,POINT& xy,POINT &pi,bool fcursor,bool fwinrect)
	{
        if((reconnect_mode)&&(s==INVALID_SOCKET))
		{
			_init(vhost_port.get(),vns_path.get());
		}

		 int cb;
         std::strstream stream;
		 stream<<"mode=2"<<char(0);

         stream<<"set.cursor.x="<<int(xy.x)<<char(0);
         stream<<"set.cursor.y="<<int(xy.y)<<char(0);

		 if(fwinrect)
		 {
             stream<<"set.cursor.rect=1"<<char(0);
			 stream<<"set.cursor.rect.show="<<int(f)<<char(0);
		 }
		 if(fcursor)
		 {
			 stream<<"set.cursor.cursor=1"<<char(0);
	 	 }

		 cb=wb_sent(s,stream);


		 if((reconnect_mode)&&(cb<=0))
			 close();
		 if(0)
		 if(cb>0)
		 {
          std::strstream stream_rep;
		  cb=get_wbs_string(stream_rep,sget);

		 }
		 return cb;
	}

};



struct hssh_desktop_client_calibrate:hssh_desktop_client
{
	
  hssh_desktop_client_calibrate(){};
  hssh_desktop_client_calibrate
	  (const wchar_t* host_port,int nmon,
	  int diskcolor=RGB(255,255,255),int bkcolor=0,
	  const wchar_t* ns_path=_wgetenv(L"laser.hook.hssh_port"))
  {
	  init(host_port,nmon,diskcolor,bkcolor,ns_path);
  }

hssh_desktop_client_calibrate& init
	  (const wchar_t* host_port,int nmon,
	  int diskcolor=RGB(255,255,255),int bkcolor=0,
	  const wchar_t* ns_path=_wgetenv(L"laser.hook.hssh_port"))
  {
	  wchar_t  wcz=0;
	  std::wstringstream wstr;
	  wstr<<L"lm_calibrate=1"<<wcz;
	  wstr<<L"diskColor="<<diskcolor<<wcz;
	  wstr<<L"bkColor="<<bkcolor<<wcz;
	  wstr<<L"nmon="<<nmon<<wcz<<wcz;
	  reconnect(host_port,wstr.str().c_str(),ns_path);	
	 if(!reset_monitor_rect())     close();
	 return *this;
	  
  }
  

  inline bool show(bool fshow=true)
  {
	  int cb;
	  char cz=0;
	  if(!(*this)) return false;
	  std::strstream stream_in,stream_out;  
	  stream_out<<"set.show="<<int(fshow)<<char(0);
	  cb=wb_sent(s,stream_out);
	  if(cb>0)
	  {
		  cb=get_wbs_string(stream_in,sget);
	  }
	  return cb>0;
 
  }

  inline bool hide()
  {
	  return show(false);
  };


  inline  bool drawrect(int l,int t,int r,int b,bool fclear=true,bool finvert=false)
  {

	  int cb;
	  char cz=0;
	  if(!(*this)) return false;
	  std::strstream stream_in,stream_out;  
	  char buf[128];
	  stream_out<<"fclear="<<int(fclear)<<char(0);
	  stream_out<<"finvert="<<int(finvert)<<char(0);
	  stream_out<<"set.drawrect=1"<<char(0);
	  sprintf(buf,"[%d,%d,%d,%d]",l,t,r,b);

	  cb=wb_sent(s,stream_out);
	  if(cb>0)
	  {
		  cb=get_wbs_string(stream_in,sget);
	  }

	  return cb>0;
  }

  inline  bool drawcycle(int x,int y,double rc,bool fclear=false,bool finvert=false)
  {

	  int cb;
	  char cz=0;
	  if(!(*this)) return false;
	  std::strstream stream_in,stream_out;  
	  //stream_out<<"set.show=1"<<char(0);
	  stream_out<<"point.x="<<x<<char(0);
	  stream_out<<"point.y="<<y<<char(0);
	  stream_out<<"rc="<<rc<<char(0);
	  stream_out<<"finvert="<<int(finvert)<<char(0);
	  stream_out<<"fclear="<<int(fclear)<<char(0);
	  stream_out<<"set.drawcycle=1"<<char(0);
	  
	  cb=wb_sent(s,stream_out);
	  if(cb>0)
	  {
		  cb=get_wbs_string(stream_in,sget);
	  }

	  return cb>0;
  }


  template<class float_type>
 bool  draw_target(std::complex<float_type> z,float_type rc,int tic=4)
  {
	  int cb;
	  char cz=0;
	  if(!(*this)) return false;
	  double x=z.real();
	  double y=z.imag();
	  std::strstream stream_in,stream_out;  
	 // stream_out<<"set.show=1"<<char(0);
	  stream_out<<"point.x="<<x<<char(0);
	  stream_out<<"point.y="<<y<<char(0);
	  stream_out<<"rc="<<rc<<char(0);
	  stream_out<<"tic="<<tic<<char(0);
	  stream_out<<"set.draw_target=1"<<char(0);
	   cb=wb_sent(s,stream_out);
	  if(cb>0)
	  {
		  cb=get_wbs_string(stream_in,sget);
	  }

	  return cb>0;
  }

 template<class float_type>
 inline  std::pair<std::complex<float_type>,float_type> unitcycle(std::complex<float_type> z,float_type rc,bool fclear=true,bool finvert=false)
 {
	 wchar_t buf[256]; 
	 //   SelectObject(dc,hpen);

	 int xc=rw.right/2;
	 int yc=rw.bottom/2;
	 float_type rs=min(xc,yc);
	 float_type ix=xc+rs*z.real();
	 float_type iy=yc+rs*z.imag();
	 float_type ir=rs*rc;
	 drawcycle(ix,iy,ir,fclear,finvert);
	 return std::make_pair(std::complex<float_type>(ix,iy),ir);
 }


};





struct hssh_desktop_service:hssh_desktop_base
{

	 struct  counter_t
	 {
		 volatile long* pcount;
		 
		 counter_t(volatile long* _pcount):pcount(_pcount)
		 {
            long l=InterlockedIncrement(pcount);
			SetConsoleTitleA(v_buf<char>().printf("RDS connection count = %d",l));

		 }
		 ~counter_t()
		 {
            long l=InterlockedDecrement(pcount);
			SetConsoleTitleA(v_buf<char>().printf("RDS connection count = %d",l));
		 }

		inline  operator long()
		{
			return InterlockedExchangeAdd(pcount,0);
		}
            
	 };


	struct strsetter_base
	{
		std::strstream stream;
		strsetter_base(){};

		inline int operator()(void* p,int count,int flags=0) const
		{
			//return ::send(s,(char*)p,count,flags);
			strsetter_base * t=const_cast<strsetter_base *>(this);
			t->stream.write((char*)p,count);
			int cb=t->stream.rdbuf()->pcount();
			return count;

		}
	};

	//typedef ULONG( *cx_convert_t)(char*sext,int scb,void* sbuf,char*dext,int* pdcb,void* dbuf,void * popt);

	

	


	monitors_t monitors;

     POINT laser_points[16];	

typedef	Capture_base::CS mutex_t;
        mutex_t mutex;
         volatile long count;     



//	HGDI_holder hbrush;
//	HWND hwT;

struct win_rect_t
{
	HGDI_holder hbrush;
	HGDI_holder hbrushE;
	HWND hwT;
	DC_holder dc;

  
     win_rect_t(DWORD clr=RGB(0,255,0),DWORD clrE=RGB(255,0,0))
	 {
		 hbrush=CreateSolidBrush(clr);
		 hbrushE=CreateSolidBrush(clrE);
		 //
		 hwT=Capture_base::wnd_alloc("wrffAxq",0,0,WS_POPUPWINDOW,(HBRUSH)hbrush.hobj);
		 //		 hwT=Capture_base::wnd_alloc("wrffAxq",0,0,0,(HBRUSH)hbrush.hobj);
		 dc.attach(hwT);
		 SelectObject(dc,hbrushE);
	 }
  ~win_rect_t()
  {
	  DestroyWindow(hwT);
  }
	inline    void  hide_winrect()
	{
		ShowWindow(hwT,SW_HIDE);
	}
	inline    void  draw_winrect(int x,int y,bool fshow=true)
	{
		if(fshow)
		{
			//SetWindowPos(hwT,HWND_TOPMOST,x-8,y-8,16,16,SWP_SHOWWINDOW);
			SetWindowPos(hwT,HWND_TOPMOST,x-8,y-8,16,16,SWP_NOACTIVATE);
			ShowWindow(hwT,SW_SHOWNOACTIVATE);

		      
		      Ellipse(dc,0,0,16,16); 
		}
		else hide_winrect();
	}

};

struct winrect_msg_t
{

	static DWORD __stdcall s_thread_proc(void *p)
	{
		win_rect_t win_rect=DWORD(p);   
		MSG msg;
		while(GetMessage(&msg,0,0,0))
		{
			if(msg.message==WM_USER)
			{
				int x=LOWORD( msg.lParam);
				int y=HIWORD( msg.lParam);
				bool f=msg.wParam;
				win_rect.draw_winrect(x,y,f);

			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
        return 0;
	}

	DWORD tid;




inline    void  draw_winrect(int x,int y,bool fshow=true)
{
   PostThreadMessage(tid,WM_USER,fshow,MAKELONG(x,y));
}
   winrect_msg_t(DWORD clr=RGB(0,255,0))
   {
	   tid=0;
      CloseHandle(CreateThread(0,0,&s_thread_proc,(void*)clr,0,&tid));


   }

 ~winrect_msg_t()
 {
	 PostThreadMessage(tid,WM_QUIT,0,0);
 }

};



	void laser_monitor_calibrate(HSS_INFO* hovl,argv_def<char>& args_connect)
	{
		
		int nmon=args_connect["nmon"].def(0);

		int mon_count=monitors.count();
		if(nmon>=mon_count) return;


		sockgetter sget(hovl->s);
		socksetter sset(hovl->s);

		char cz=0,cfin=0xFF;
		int cb;

		char buf[256];


		//ScreenCapture monitor(nmon);
		ScreenCapture& monitor=monitors[nmon];
		DWORD diskColor=args_connect["diskColor"].def(RGB(255,255,255));
		DWORD bkcol=args_connect["bkColor"].def(RGB(0,0,0));
      	Calibrate cal(monitor.monitor_rect(),diskColor,bkcol);
		

		for(;;)
		{
			std::strstream stream_in,stream_out;  
			if((cb=get_wbs_string(stream_in,sget))<=0)
				return;



			argv_zz<char> args_in(stream_in.str());

			if(!!args_in["get.rect"])
			{
				RECT r=monitor.monitor_rect();
				sprintf(buf,"rect=[%d,%d,%d,%d]",r.left,r.top,r.right,r.bottom);
				stream_out<<buf<<cz;

			}
			if(!!args_in["get.monitors_count"])
			{
				stream_out<<"monitors_count="<<mon_count<<cz;
			}
			
			
			bool fshow=args_in["set.show"].def(0);
			if(!!args_in)	cal.show(fshow);

			int x=args_in["point.x"].def(0);
			int y=args_in["point.y"].def(0);
			double rc=args_in["rc"].def(0);
			bool fclear=args_in["fclear"].def(0);
            bool finvert=args_in["finvert"].def(0);
			int tic=args_in["tic"].def(4);
			
			if(!!args_in["set.drawcycle"])
			{
			  cal.drawcycle(x,y,rc,fclear,finvert);	
			}

			if(!!args_in["set.draw_target"])
			{
				cal.draw_target(std::complex<double>(x,y),rc,tic);	
			}
			if(!!args_in["set.rect"])
			{
				std::vector<int> vr=args_in["rect"];
				cal.drawrect(vr,fclear,finvert);	
			}

			//draw_target(std::complex<float_type> z,float_type rc,int tic=4)


			stream_out<<cz<<cz;
			char* pout=stream_out.str();
			int cbout=stream_out.pcount();
			// test ..

			cb=sset(&cz,1);
			cb=sset(pout,cbout);
			cb=sset(&cfin,1);

		}

	}


   
	void control(HSS_INFO* hovl,argv_def<char>& args_connect)
	{
		
         

		int cb;
		int nmon=args_connect["nmon"].def(0);
		ScreenCapture& monitor=monitors[nmon];
		int mon_count=monitors.count();

        RECT mon_rect=monitor.monitor_rect();

		//win_rect_t
			winrect_msg_t winrect(args_connect["winrect.bkcolor"].def(RGB(255,0,0)));
       
		if(nmon>=mon_count) return;

		char cz=0,cfin=0xFF;
		sockgetter sget(hovl->s);
		socksetter sset(hovl->s);


		while(1)
		{

			std::strstream stream_in,stream_out;

			if((cb=get_wbs_string(stream_in,sget))<=0)
				return;

			argv_zz<char> args_in(stream_in.str());

			unsigned mode=args_in["mode"].def(0);

			char buf[256];

			if(mode&2)
			{
				int x=args_in["set.cursor.x"].def(0);
				int y=args_in["set.cursor.y"].def(0);
				  
				if(args_in["dump"].def(0)) _cprintf("set.cursor=[%d,%d]\n",x,y);

				bool frect=args_in["set.cursor.rect"].def(0);

				if(frect)
				{
			
					bool f=args_in["set.cursor.rect.show"].def(0);
					winrect.draw_winrect(x,y,f);
				}
                
				if(args_in["set.cursor.cursor"].def(0))
				{
						SetCursorPos(x,y);
						
						//POINT p={x-mon_rect.left,y-mon_rect.top};
						POINT p={x,y};
						locker_t<mutex_t> lock(mutex);
						laser_points[nmon]=p;

				}

                //_cprintf("set.cursor=[%d,%d]\n",x,y);

				if(args_in["set.mouse"].def(0))
				{
					//int x=args_in["set.cursor.x"].def(0);
					//int y=args_in["set.cursor.y"].def(0);
					std::vector<int> vevents=args_in["set.mouse.events"];
					int nev;
					
					if(nev=vevents.size())
					{
						std::vector<INPUT> vinpts(nev);  
                       
                         INPUT inpt={INPUT_MOUSE};
					   for(int n=0;n<nev;++n)
					   {
						    inpt.mi.dwFlags=vevents[n];
							vinpts[n]=inpt;
					   }

                       SendInput(nev,&vinpts[0],sizeof(INPUT));
					   

					}
					else
					{
						ULONG evnt=args_in["set.mouse.event"].def(0);
						if(!!args_in)
						{
						    INPUT inpt={INPUT_MOUSE};	
							inpt.mi.dwFlags=evnt;
							SendInput(1,&inpt,sizeof(INPUT));

						}

					}

					



				}




			}


			if(mode&1)
			{

				if(!!args_in["get.rect"])
				{
					RECT r=monitor.monitor_rect();
					sprintf(buf,"rect=[%d,%d,%d,%d]",r.left,r.top,r.right,r.bottom);
					stream_out<<buf<<cz;

				}

				bool lp=args_in["get.laser.point"].def(0);
				bool lpr=args_in["get.laser.point.rel"].def(0);
				if(lp||lpr)
				{

					//POINT p={x-mon_rect.left,y-mon_rect.top};
					POINT p;
					{
						locker_t<mutex_t> lock(mutex);
						p=laser_points[nmon];
					}

					if(lp) {
						sprintf(buf,"laser.point=[%d,%d]",p.x,p.y);
						stream_out<<buf<<cz;
					}

					if(lpr)
				 {
					 p.x-=mon_rect.left;
					 p.y-=mon_rect.top;
					 sprintf(buf,"laser.point.rel=[%d,%d]",p.x,p.y);
					 stream_out<<buf<<cz;
				 }



				}

				if(!!args_in["get.monitors_count"])
				{
					stream_out<<"monitors_count="<<mon_count<<cz;
				}

/*
			}

        if(mode&1)	  {
*/
			stream_out<<cz<<cz;
			char* pout=stream_out.str();
			int cbout=stream_out.pcount();
			// test ..

			cb=sset(&cz,1);
			cb=sset(pout,cbout);
			cb=sset(&cfin,1);
	     }


		}

	}

	template <class Getter>
static	bool get_wbs_crc32(Getter &sget,ULONG32& crc32,int *pkfm=0)
	{
		std::strstream stream;
		int cb;
		cb=get_wbs_string(stream,sget);
		if(cb<=0)
		{				
			return false;
		}
        int c= stream.pcount(),cn;
		char* pask=stream.str(); 
		

		crc32=atoi(pask); 
		if(pkfm&&(c>(cn=strlen(pask))))
		{
           *pkfm=atoi(pask+cn+1);  
		}
		return true;

	}

    
     struct image_options
	 {
       long itemsize;
	   long bufsize;
	   int fbin;
	   void* pbuf[3];
	   int  np;
	   void* pfinal;
	   int kfm;
	    POINT cursor;
		bool fwebsocket;
		
	 };


	 template <class Setter,class Updater> 
	int send_image_base64_2(Setter& sset,Updater& updater,ULONG crc32,BMPMap& bmpmap,const char* ext,image_options* popt )
	{

		
		ULONG32 crc32new;
		int fcv,cb=0,fcnv=0,cbfull=0,cbi;
		int pack_size=0,unpack_size=0;
		bool fbin=popt->fbin;
		long itemsize=popt->itemsize;
		bool fupdate;
		
		if(!fbin) return 0;
		int bufsize=popt->bufsize;
         int& np=popt->np; 

		BYTE* pkeyframe=(BYTE*)popt->pbuf[np];
		BYTE* pframe=pkeyframe;
		BYTE* ppackframe=(BYTE*)popt->pbuf[2];
          int vvcrc32new;

		
		stopwatch_t swt;
		{

			//BMPMap::
				locker_t<BMPMap> lock(bmpmap);
			swt.Start();
			updater();
			crc32new=bmpmap.get_crc32();
			
			fupdate=crc32new!=crc32;

			bin_image_t bi={crc32new};
			bi.cursor=popt->cursor;

			if(fupdate)
			{

				//_cprintf("pack size=%g KB unpack size=%g KB  net=%g sec convert=%g sec\n",cbk,cbuk,tn,tc);

				if(strcmpi(ext,"yuy2gz")==0)
				{
					/*
					fcnv=cx_convert("bmp",bmpmap.size,bmpmap.ptr,"yuy2",&(unpack_size= bufsize),pkeyframe,0);
					if(fcnv)
					{
						if(popt->kfm)
						{
							//np=(np+1)%2;
							pframe=(BYTE*)popt->pbuf[np+1];
							for(int n=0;n<unpack_size;++n)
							{
                               pframe[n]=pkeyframe[n]-pframe[n]; 
							}
						}

					}
					*/


					void* pbb=(popt->kfm==0)?pkeyframe:popt->pbuf[1];
					fcnv=cx_convert("bmp",bmpmap.size,bmpmap.ptr,"yuy2",&(unpack_size= bufsize),pbb,0);
					if(fcnv)
					{
						if(popt->kfm)
						{
							//np=(np+1)%2;
							pframe=(BYTE*)popt->pbuf[1];
							for(int n=0;n<unpack_size;++n)
							{ 
								BYTE c=pframe[n];  
								pframe[n]-=pkeyframe[n]; 
								pkeyframe[n]=c;

							}
						}

					}
					
				}
				else if(strcmpi(ext,"bmpgz")==0)
				{
                    //
					fcnv=1;
					unpack_size=bmpmap.size;
					if(popt->kfm)
					{
						//
						pframe=(BYTE*)popt->pbuf[np+1];
						//np=(np+1)%2;
						for(int n=0;n<unpack_size;++n)
						{
							BYTE c=((BYTE*)bmpmap.ptr)[n];
							//pframe[n]=c-pframe[n]; 
							pframe[n]=c-pkeyframe[n]; 
							pkeyframe[n]=c;

						}
					}
					else memcpy(pkeyframe,bmpmap.ptr,unpack_size=bmpmap.size);
					
				}
				
		   }
		}

             fupdate=fupdate&&fcnv;
		    if(fupdate)
			{
			   fcnv=cx_convert("bmp",unpack_size,pframe,"gzip",&(pack_size= bufsize),ppackframe,0);	

			   if(0){
				   BMPMap bmmm;
				   v_buf<BYTE> bbb(bufsize);
				   bmmm.ptr=bbb.get();

				   fcnv=cx_convert("gzip",pack_size,ppackframe,"bmp",(int*)&(bmmm.size= bufsize),bmmm.ptr,0);
				   vvcrc32new=bmmm.get_crc32();
				  
			   }

			}
            fupdate=fupdate&&fcnv;

			bin_image_t bi={crc32new};
			bi.cursor=popt->cursor;
			bi.kfmask=popt->kfm;
			strcpy(bi.ext,ext);
			bi.size=pack_size;


			cb=sset(&bi,sizeof(bi));
			if(cb<=0) return cb;
			cbfull+=cb;

			if(fupdate)
			{
				int nn=pack_size/itemsize;
				int nr=pack_size-nn*itemsize;
				char* p=(char*)ppackframe;

				for(int k=0;k<nn;k++)
				{
					cbi= sset(p,itemsize);
					if(cbi<=0) return cbi;
					cb+=cbi;
					p+=cbi;
				}
				cbi= sset(p,nr);

				if(cbi<=0) return cbi;

				cbfull+=cb;

			}





			double t= swt.Sec();
			double szk=bmpmap.size/(1024.0*1024.0),szpk=pack_size/1024.0;
			double perc=100.*double(pack_size)/double(bmpmap.size);
			_cprintf("[%d] image size=%g MB  pack[%s]=%2.3f%% KB grab\&pack=%g sec ",popt->kfm,szk,ext,perc,t);




		if(cbfull>1024)
			_cprintf("send packet size=%g KB\n",double(cbfull)/(1024));
		else _cprintf("send packet size=%g Bytes\n",double(cbfull));
		return cbfull;


	}
	template <class Setter,class Updater> 
	int send_image_base64(Setter& sset,Updater& updater,ULONG crc32,BMPMap& bmpmap,const char* ext ,int bufsize,void *picbuf,void *picbuf64,image_options* popt )
		//
	{
		char buf[64];
		ULONG32 crc32new;
		int fcv,cb=0,fcnv,cbfull=0,cbi;
		int fs=0,fs64;
		bool fbin=popt->fbin;
		long itemsize=popt->itemsize;
		
		
		bool fupdate;
		stopwatch_t swt;
		{


			BMPMap::locker_t<BMPMap> lock(bmpmap);
			swt.Start();
			updater();
			crc32new=bmpmap.get_crc32();
			fupdate=(crc32new!=crc32)||(!popt->fwebsocket);
			if(fupdate)
			{
				 
				//_cprintf("pack size=%g KB unpack size=%g KB  net=%g sec convert=%g sec\n",cbk,cbuk,tn,tc);
				
                if(strcmpi(ext,"yuy2gz")==0)
				{

                   fcnv=cx_convert("bmp",bmpmap.size,bmpmap.ptr,"yuy2",&(fs64= bufsize),picbuf64,0);
				   if(fcnv)
				      fcnv=cx_convert("bmp",fs64,picbuf64,"gzip",&(fs= bufsize),picbuf,0);
				   
				}
				else 	fcnv=cx_convert("bmp",bmpmap.size,bmpmap.ptr,(char*)ext,&(fs= bufsize),picbuf,0);
				double t= swt.Sec();
				double szk=bmpmap.size/(1024.0),szpk=fs/1024.0;
				_cprintf("image size=%g  pack[%s]=%g KB grabpack=%g sec ",szk,ext,szpk,t);
				

			}

		}

		if(!popt->fwebsocket)
		{
			int cb=1;
			char header_fmt[]="HTTP/1.1 200 OK\nContent-Type: image/jpeg\nContent-Length: %d\n\n";
			//char header_fmt[]="HTTP/1.1 200 OK\nContent-Length: %d\n\n";
			//char header_fmt[]="HTTP/1.1 200 OK\nContent-Length: %d\n\n";

			v_buf<char> header;
			//header.printf(header_fmt,bmpmap.size);
			header.printf(header_fmt,fs);
			cb=sset(header.get(),header.size_b()-1);

			if(cb<=0) return cb;
			//
			cb=sset(picbuf,fs);

			// cb=sset(bmpmap.ptr,bmpmap.size);
			if(cb<=0) return cb;
			//return -1;
			char buf[1024];
			cb=recv(sset.s,buf,1024,0);
			return cb; 

		}



  if(fbin)
  {
  
	  bin_image_t bi={crc32new};
	  bi.cursor=popt->cursor;
	  
      if(fupdate)
	  { 
		   strcpy(bi.ext,ext);
		   bi.size=fs;
		   cb=sset(&bi,sizeof(bi));
		   if(cb<=0) return cb;
		   cbfull+=cb;

		   int nn=fs/itemsize;
		   int nr=fs-nn*itemsize;
		   char* p=(char*)picbuf;

		   for(int k=0;k<nn;k++)
		   {
			   cbi= sset(p,itemsize);
			   if(cbi<=0) return cbi;
			   cb+=cbi;
			   p+=cbi;
		   }
		    cbi= sset(p,nr);
		   if(cbi<=0) return cbi;

		   //cb=sset(picbuf,fs);
		  // if(cb<=0) return cb;
		   cbfull+=cb;
	  }
	  else 
	  {
         cb=sset(&bi,sizeof(bi));
         cbfull+=cb;
	  }



  }
  else
  {
		buf[0]=0; 
		char *pcrs32=itoa(crc32new,buf+1,10);
		int len=strlen(pcrs32);
		cb= sset(buf,len+2);
		if(cb<=0) return cb;
		cbfull+=cb;
             
		if(fupdate)
		{
            swt.Start();     
			fcnv=cx_convert(ext,fs,picbuf,"base64",&(fs64= bufsize),picbuf64,0);
			double t= swt.Sec();
			
             swt.Start();     
			 char* p=(char*)picbuf64;
			 int cb=0,cbi;
			 int nn=fs64/1024;
			 int nr=fs64-nn*1024;
     
			 for(int k=0;k<nn;k++)
			 {
               cbi= sset(p,1024);
			   cb+=cbi;
			   p+=cbi;
   			 }
			 cbi= sset(p,nr);
			 cb+=cbi;
            
			//cbi= sset(picbuf64,fs64);
			double ts= swt.Sec();
			_cprintf("convert_base64=%g sec send= %g sec ",t,ts);
			if(cb<=0) return cb;
			cbfull+=cb;
			//_cprintf("packet size=%g KB\n",double(fs64)/(1024));
		}
		cb=sset("\xFF",1);
		if(cb<=0) return cb;
   
		cbfull+=cb;
  }
		//_cprintf("pack size=%g KB unpack size=%g KB  net=%g sec convert=%g sec\n",cbk,cbuk,tn,tc);
		if(cbfull>1024)
			_cprintf("send packet size=%g KB\n",double(cbfull)/(1024));
		else _cprintf("send packet size=%g Bytes\n",double(cbfull));
		return cbfull;

}

  bool is_websocket_client(HSS_INFO* hovl)
  {
	  	  return hovl->buffers[HSD_REPLY].buf;
  }
  template <class ARGS >
	void monitor_n(HSS_INFO* hovl,ARGS& args_connect)
	{
		sockgetter sget(hovl->s);
		socksetter sset(hovl->s);
		int cb;

		bool fwbs=is_websocket_client(hovl);

		int nmon=args_connect["nmon"].def(0);
		bool fbin=args_connect["binary"].def(0);
		bool keyframe_mode=args_connect["keyframe.mode"].def(0);
		const char* pack_fmt=args_connect["pack.fmt"].def((fwbs)?"png":"jpg");
		long itemsize=args_connect["itemsize"].def(4*1024);
        keyframe_mode&=((!strcmpi(pack_fmt,"yuy2gz"))|(!strcmpi(pack_fmt,"bmpgz")));
		bool frop=args_connect["hiq"].def(0);

		


		if(nmon>=monitors.count()) return;

		//ScreenCapture monitor(nmon);

		ScreenCapture& monitor=monitors[nmon];
		RECT monrect=monitor.monitor_rect();

		int bufsize=(monitor.bip.sizeb()*3)/2;

		v_buf<unsigned char> vbuf1(bufsize); 
		v_buf<char> vbuf2(bufsize),vbuf3(bufsize);

		unsigned	char* picbuf =  vbuf1;
		char* picbuf64 = vbuf2;


		image_options opt={itemsize,bufsize,fbin,{vbuf1.get(),vbuf2.get(),vbuf3.get()},0,vbuf3.get()};

		struct 	updater_t
		{
			ScreenCapture* psc;
			bool frop;
			inline void operator()()
			{
				psc->update(frop);
			}

		} updater={&monitor,frop} ;


		while(1)
		{
			ULONG32 crc32=0;
            int kfm=0;   
			if(fwbs)
			{
				bool f=get_wbs_crc32(sget,crc32,&kfm);
				if(!f) return;
			}
			
			
			//,fbin,itemsize);
             
			
			
			//image_options opt={itemsize,fbin};
			
			GetCursorPos(&opt.cursor);
			opt.cursor.x-=monrect.left;
			opt.cursor.y-=monrect.top;
			if(keyframe_mode)
			{
				opt.kfm=kfm;
               cb=send_image_base64_2(sset,updater,crc32,monitor.bmpmap,pack_fmt,&opt);
			}			
			else  cb=send_image_base64(sset,updater,crc32,monitor.bmpmap,pack_fmt,bufsize,picbuf,picbuf64,&opt);
		}


	}


void webcamcontrol(HSS_INFO* hovl,argv_def<char>& args_connect)
	{
		sockgetter sget(hovl->s);
		socksetter sset(hovl->s);
		int cb;

		//std::string CamControl_reset(argv_zzs<char>& argv=argv_zzs<char>(),char* delim=0)


		std::strstream stream_in;
		

		if((cb=get_wbs_string(stream_in,sget))<=0)
			return;
		   argv_zz<char> args_in(stream_in.str());

         int noreplay=args_in["noreplay"].def(0);  
         std::string strout;
         argv_zzs<char> argrw;

		 if(!noreplay)
		 {
			    argrw= args_in;

				std::string strout=CamControl_reset(argrw);
				
				char* pout=(char*)strout.c_str();
				int cbout=strout.length();
				// test ..
                char cz=0x00,cfin=0xFF;  
				cb=sset(&cz,1);
				cb=sset(pout,cbout);
				cb=sset(&cfin,1);

		 }
		 else CamControl().update_argv(args_in);
		


	}


	void webcamera(HSS_INFO* hovl,argv_def<char>& args_connect)
	{
		sockgetter sget(hovl->s);
		socksetter sset(hovl->s);
		int cb;

		WC_filemap_only wbcam;


		int bufsize=((8*1024*1024)*2)/3;


		v_buf<unsigned char> vbuf1(bufsize); 
		v_buf<char> vbuf2(bufsize);

		unsigned	char* picbuf =  vbuf1;
		char* picbuf64 = vbuf2;

		struct 	updater_t
		{

			inline void operator()()
			{

			}

		} updater ;


		bool fwbs=is_websocket_client(hovl);
        cb=1;
		while(cb>0)
		{
   			ULONG32 crc32=0;
			image_options opt={4096};
          if(opt.fwebsocket=fwbs)
		  {
			bool f=get_wbs_crc32(sget,crc32);
			if(!f) return;
			
		  }
		  else opt.fbin=true;

		  
		  ;
			cb=send_image_base64(sset,updater,crc32,wbcam.get_bmpmap(),"jpg",bufsize,picbuf,picbuf64,&opt);
		}


	}




	static void __stdcall s_on_accept(hssh_desktop_service* p, HSS_INFO* hovl)
	{
		p->on_accept(hovl);
	}
	inline void  on_accept(HSS_INFO* hovl)
	{

		 counter_t counter(&count);

		if(!hovl) return;

		char* sn= hssh.reset_SOCKET_DATA(hovl,HSD_LOCAL_INFO,HSF_GET,0,0);
		char* pn= hssh.reset_SOCKET_DATA(hovl,HSD_REMOTE_INFO,HSF_GET,0,0);
		_cprintf("on_accept:: sockname=(%s)  peername=(%s)\n",sn,pn);

		SOCKET s=hssh.reset_handshake_socket(hovl);
		if(s==INVALID_SOCKET) return;

		PROC abort=(PROC)hssh.reset_SOCKET_DATA(hovl,HSD_SIGNALER,HSF_GET,0,0);
		char* prms= hssh.reset_SOCKET_DATA(hovl,HSD_PARAMS,HSF_GET,0,0);

		argv_zz<char> args(prms);

		

		if(args["stop"].def(0)) 
		{
			abort();

			return;
		}

		if(args["control"].def(0))
		{
			_cprintf("control connected sockname=(%s)  peername=(%s)\n",sn,pn);
			control(hovl,args);
			_cprintf("control disconnect sockname=(%s)  peername=(%s)\n",sn,pn);
			return;
		}

		if(args["webcamera"].def(0))
		{
			_cprintf("webcamera connected sockname=(%s)  peername=(%s)\n",sn,pn);
			webcamera(hovl,args);
			_cprintf("webcamera disconnect sockname=(%s)  peername=(%s)\n",sn,pn);
			return;
		}

		if(args["webcamcontrol"].def(0))
		{
			_cprintf("webcamcontrol connected sockname=(%s)  peername=(%s)\n",sn,pn);
			webcamcontrol(hovl,args);
			_cprintf("webcamcontrol disconnect sockname=(%s)  peername=(%s)\n",sn,pn);
			return;
		}
		if(args["lm_calibrate"].def(0))
		{
			_cprintf("laser_monitor_calibrate connected sockname=(%s)  peername=(%s)\n",sn,pn);
			laser_monitor_calibrate(hovl,args);
			_cprintf("laser_monitor_calibrate disconnect sockname=(%s)  peername=(%s)\n",sn,pn);
			return;
		}

		{
             _cprintf("monitor connected sockname=(%s)  peername=(%s)\n",sn,pn);
			monitor_n(hovl,args);
			_cprintf("monitor disconnect sockname=(%s)  peername=(%s)\n",sn,pn);
			return;
		}

	}


	
	HRESULT run(wchar_t* portname)
	{

		HRESULT hr;
		if((hr=load_converter())) return hr;

		void* pclosure=hssh.create_stdcall_closure(&s_on_accept,this);

		void* hport=hssh.create_socket_portW(portname,HSF_SRD_AUTO,pclosure,HSD_SAME_PTR);
		if(!hport) return hssh.error();
		hssh.release(pclosure);

		hssh.wait_signaled(hport,INFINITE);
		hssh.release(hport);

		//FreeLibrary(hlib);

		return S_OK;
	}

/*
	inline    void  hide_winrect()
	{
		 ShowWindow(hwT,SW_HIDE);
	}
	inline    void  draw_winrect(int x,int y,bool fshow=true)
	{
      if(fshow)
     	SetWindowPos(hwT,HWND_TOPMOST,x-15,y-15,15,15,SWP_SHOWWINDOW);
	  else hide_winrect();
	}
*/
	hssh_desktop_service():count(0)
{
	memset(laser_points,0,sizeof(laser_points));
}
~hssh_desktop_service()
{

}
};


template <class Args>
inline int start_shared_desktop_client(Args& a)
{
	argv_zzs<wchar_t> argsfull;
	{

	
	argv_zzs<wchar_t> args;
	args=a;
	argv_zzs<wchar_t>  argfile;
	argv_zzs<wchar_t>   argini;
	args.parent=&argfile;
	argfile.parent=&argini;
	if(args[L"@"].is())
		argfile = argv_file<wchar_t>(args[L"@"]);
	argini=argv_ini<wchar_t>(L"remote_desktop_client_options",L".ini");

	argsfull=args;
	}
      
	
	int cm=argsfull[L"console.mode"].def(0);
	if(cm)
	{
		if(cm&2) AttachConsole(ATTACH_PARENT_PROCESS);
		AllocConsole();
	}    

	singleton_restarter_t<> sr(argsfull[L"key"]);

	

	while(1)
	{

		hssh_desktop_client_image dci(argsfull);
		dci.image_queue.init();
		dci.open_shared_image();
		dci.loop_update();
		dci.close();
		_cprintf("disconnect wait 2 sec... \r");
		Sleep(2000);
		_cprintf("reconnecting ....\n");
		
		

	}


	return 0;

}
