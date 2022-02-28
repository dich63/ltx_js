#pragma once

#include "wchar_parsers.h"

#include "md5hash.h" 
#include "hss_port.h" 

#include "sha1.h"


#pragma comment(lib ,"Ws2_32.lib")
#pragma comment(lib, "shlwapi" )

#pragma pack(push)
#pragma pack(1)


#define WBSOCKET_UUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"



struct wbkeys
{


	union{
	struct{unsigned char ib[16];};
		struct{
	unsigned long key1;
	unsigned long key2;
	char body[16];
	};
	};

charbuf skey1;
charbuf skey2;
Cryptdll md5;

inline static charbuf get_skey7(unsigned long n)
{
	unsigned long long nn=n*7;
	charbuf buf;
	char bnn[32],bnn1[32],bnn2[32];
	memset(buf,0,buf.size);
	memset(bnn,0,32);
	_ui64toa(nn,bnn,10);
	char* p=buf;
	char* pn=bnn;
	int cb;
	cb=strlen(strcat((char*)memcpy(p,pn,3),"k\x20k!\x20\x20@"));
	p+=cb;
	pn+=3;
	strcat((char*)memcpy(p,pn,2),"k\x20ZZ\x20@");
	pn+=2;
	strcat(strcat(p,pn),"k\x20ZZ\x20!\r\n");

	
	return buf;
}

inline static charbuf get_skey3(unsigned long n)
{
	unsigned long long nn=n*3;
	charbuf buf;
	char bnn[32],bnn1[32],bnn2[32];
	memset(buf,0,buf.size);
	memset(bnn,0,32);
	_ui64toa(nn,bnn,10);
	char* p=buf;
	char* pn=bnn;
	int cb;
	cb=strlen(strcat((char*)memcpy(p,pn,3),"k\x20k!@"));
	p+=cb;
	pn+=3;
	strcat((char*)memcpy(p,pn,2),"k\x20ZQ@");
	pn+=2;
	strcat(strcat(p,pn),"kww\x20!\r\n");

	return buf;
}



inline void	init(unsigned long k1,unsigned long k2,unsigned long k3)
	{
		const unsigned long msk=0x0fFFFFFF;
		k1&=msk;
		k2&=msk;
		//k3&=msk;
		key1=htonl(k1);
		key2=htonl(k2);
		sprintf(body,"%08x",-k3+key2);
		//memcpy(body,Cryptdll().update(body,8),8);
		 skey1=get_skey7(k1);
		 skey2=get_skey3(k2);
		 //
		 md5.reinit();
		 md5.update(ib,16).hash();
	}

unsigned long get_key(char* skey)
{
	char buf[128];
	char *p=skey;
	memset(buf,0,sizeof(buf));
    unsigned long nb=0,n=0;
	while((*p)&&(*p!='\n'))
	{
      if(*p=='\x20') ++nb;
	  else if(isdigit(*p)) buf[n++]=*p;
	  ++p;
	}
   unsigned long long nn=_atoi64(buf);
	   nn/=nb;
    return htonl(nn);
}

inline unsigned char* co_init(char* sk1,char*  sk2, char* sk3)
{
   memcpy(body,sk3,8);body[8]=0;
   key1=get_key(sk1);
   key2=get_key(sk2);
   md5.reinit();
   return md5.update(ib,16).hash().digest;
};
static unsigned long getsk()
{
  static unsigned long l;
  l=7*l+131;
  return l;
}

wbkeys(){}

inline void init(int k1)
{
   init(k1,k1+getsk(),k1+getsk());
}
inline wbkeys* reinit()
{
  if(this) init(GetTickCount());
   return this;
};

wbkeys(unsigned long k1)
{
	init(k1);
}

wbkeys(unsigned long k1,unsigned long k2,unsigned long k3)
{
 init(k1,k2,k3);
}

inline bool check_hash(const void* p)
{
  return memcmp(md5.mdctx.digest,p,16)==0;
}
inline bool operator ==(wbkeys& k)
{
	return check_hash(k.md5.mdctx.digest);
}

inline bool operator ==(const void* p)
{
	return check_hash(p);
}

};

template <class Stream >
Stream& set_websocket_GET(Stream& stream,char* host,char* url,char* argszz=0,wbkeys* pwbk=0,char* reconnect_port=0)
{
    
	size_t lb=0;
	std::vector< char*> vargs=args_ZZ(argszz,&lb);

	v_buf<char> buf(lb=(256+3*lb));

	char *p=url;
	
	stream<<"GET /";
	while(*p=='/') ++p;
	for(;(*p)&&(!isspace(*p));++p)
		stream<<(*p);


	//stream<<"GET /"<<p;
	//for(int n=0;n<vargs.size();n++)
	int cb;
	int narg=vargs.size();
	int n=0;
   p=buf;

	if(narg)
	{

		if(url_escape(vargs[0],-1,buf,&(cb=lb))){
			stream<<"?"<<(char*)buf;
		for(n=1;n<narg;++n)	{
			if(url_escape(vargs[n],-1,buf,&(cb=lb)))
				stream<<"&"<<(char*)buf;
		  } 
		}
	}


/*
	if(narg)
	{

		if(SUCCEEDED(::UrlEscapeA(vargs[0],buf,&(cb=8*1024),URL_ESCAPE_SEGMENT_ONLY|URL_ESCAPE_PERCENT)))
		  stream<<"?"<<(char*)buf;
		for(n=1;n<narg;++n)	{
			if(SUCCEEDED(::UrlEscapeA(vargs[n],buf,&(cb=8*1024),URL_ESCAPE_SEGMENT_ONLY|URL_ESCAPE_PERCENT)))
				stream<<"&"<<(char*)buf;
			}
	}
*/
      stream<<" HTTP/1.1\r\n";
      stream<<"Upgrade: WebSocket\r\n";
      stream<<"Connection: Upgrade\r\n";

	  if(pwbk) {
	  stream<<"Sec-WebSocket-Key2: ";
      stream<<pwbk->skey2;
	  //stream<<"\r\n";
	  } 
	  stream<<"Host: "<< host<<"\r\n";
	  if(pwbk) {
	  stream<<"Sec-WebSocket-Key1: ";
	  stream<<pwbk->skey1;
	  //stream<<"\r\n";
	  }
	  stream<<"Origin: null\r\n";
	  if(reconnect_port) stream<<"HSSH-client-port: "<< reconnect_port <<"<<\r\n";
	  stream<<"\r\n";
	  if(pwbk) {
	  	  stream<<pwbk->body;
	  }
	  return stream;
}

template <class Stream >
Stream& set_websocket_GET_8(Stream& stream,wbs_sh1_encoder_t& wbsh1,char* host,char* url,char* argszz=0,char* reconnect_port=0)
{
    
	size_t lb=0;
	std::vector< char*> vargs=args_ZZ(argszz,&lb);
	v_buf<char> buf(lb=256+3*lb);
	char *p=url;
	
	stream<<"GET /";
	while(*p=='/') ++p;
	for(;(*p)&&(!isspace(*p));++p)
		stream<<(*p);

	int cb;
	int narg=vargs.size();
	int n=0;
   p=buf;

	if(narg)
	{

		if(url_escape(vargs[0],-1,buf,&(cb=lb))){
			stream<<"?"<<(char*)buf;
		for(n=1;n<narg;++n)	{
			if(url_escape(vargs[n],-1,buf,&(cb=lb)))
				stream<<"&"<<(char*)buf;
		  } 
		}
	}


      stream<<" HTTP/1.1\r\n";
      stream<<"Upgrade: WebSocket\r\n";
      stream<<"Connection: Upgrade\r\n";
      stream<<"Host: "<< host<<"\r\n";
	  
	  stream<<"Sec-WebSocket-Key: ";
      stream<<wbsh1.base64code<<"\r\n";
	  	  
      stream<<"Sec-WebSocket-Version: 13\r\n";
	 	  
	  stream<<"Origin: null\r\n";

	  if(reconnect_port) stream<<"HSSH-client-port: "<< reconnect_port <<"\r\n";
	  stream<<"\r\n";
	  
	  return stream;
}


struct sockgetter_base
{
	SOCKET s;
	sockgetter_base(SOCKET _s):s(_s){};
	inline int operator()(void* p,int count,int flags=0) const
	{
		return ::recv(s,(char*)p,count,flags);
	}
private:
	sockgetter_base(sockgetter_base& a){};
	sockgetter_base(const sockgetter_base& a){};
};

struct sockgetter_memorized:sockgetter_base
{
	
	char* buf;
	int cb,c;
	bool fmem,fsock;
	hs_socket_utils::socket_events se;

inline	operator bool()
	{
		return INVALID_SOCKET!=s;
	}
inline	operator SOCKET()
	{
		return s;
	}

	inline SOCKET detach()
	{
		return make_detach(s,INVALID_SOCKET);
	}

	sockgetter_memorized(SOCKET _s,HANDLE h=INVALID_HANDLE_VALUE,int _cb=0,void* ptr=0,HANDLE h2=INVALID_HANDLE_VALUE):sockgetter_base(_s)
	{
		fsock=0;
		event_signaler* pes;

		if((h!=INVALID_HANDLE_VALUE)&&(pes=i_handle_cast<event_signaler>(h)))
			   h=pes->handle();
		if((h2!=INVALID_HANDLE_VALUE)&&(pes=i_handle_cast<event_signaler>(h2)))
			h2=pes->handle();
		 
		se.init_event(h);
        if(fmem=(_cb>0))
		{
			c=0;
			cb=_cb;
		  buf=(char*)(ptr);
		}

	};

	inline int operator()(void* p,int count,int flags=0) const
	{ 
          int ns;

  	   sockgetter_memorized* tnc=const_cast<sockgetter_memorized*>(this);
	   return ns=tnc->recvm(p,count,flags);

	}

inline int recvm(void* p,int count,int flags=0)
{
	  int ns;
        if((fmem)&&(c<cb))
		{ 
          int cend=c+count;
		  int cadd=cend-cb;
          if(cadd<=0)
		  {
            memcpy(p,buf+c,count);
			
			if(0==(flags&2)) c=cend;
			 return count;
		  }
		  else
		  {
			  int n=cb-c;
			  memcpy(p,buf+c,n);
			  if(0==(flags&2)) fmem=false;

			  if(cadd == 0) return n;
			  else
			  {
				    char* pp=((char*)p)+ n;
                   //
					if((ns=se.recv(s,pp,cadd,flags))>=0)
					//if((ns=::recv(s,pp,cadd,flags))>=0)
					   return n+ns;
				   else return ns;

			  }

		  }
		}
		else 	//return ns=::recv(s,(char*)p,count,flags);
		{
			fsock=1;
			return se.recv(s,(char*)p,count,flags);
		}
	}
};






struct sockgetter:sockgetter_base
{
	
	sockgetter(SOCKET _s):sockgetter_base(_s){};
	~sockgetter(){
		if(s!=INVALID_SOCKET)	::closesocket(s);
	};
	
	inline SOCKET detach()
	{
		return make_detach(s,INVALID_SOCKET);
	}
	operator bool()
	{
		return INVALID_SOCKET!=s;
	}
	operator SOCKET()
	{
		return s;
	}

} ;



inline SOCKET websocket_connect8(char* hostport,char*path ,char* argszz=0,void* hss=0)
{
	HRESULT hr;
	HANDLE habortevent=INVALID_HANDLE_VALUE;
	typedef socket_port<hss_port<> >::SOCKET_DATA_h hss_data_t;
	hss_data_t* psd=i_handle_cast<hss_data_t>(hss);

	if((psd)&&(psd->hkcount>1))
			habortevent=psd->hkObjects[0];
	  SOCKET s=tcp_connect_by_string(hostport,habortevent);
         
	  if(s==INVALID_SOCKET)  return  hr=WSAGetLastError(),s;

	  sockgetter sock_holder(s);
	  sockgetter_memorized sg(s,habortevent);
	  if(psd)
	  {
		  SOCKADDR_IN ls;
		  int nl;
		  if(SOCKET_ERROR!=getsockname(s,(SOCKADDR*)&ls,&(nl=sizeof(ls))))
		  {
			  hs_socket_utils::sockinfo si(&ls,nl);
			  psd->set_region(HSD_LOCAL_INFO,si.str,si.cb);
		  }
		  if(SOCKET_ERROR!=getpeername(s,(SOCKADDR*)&ls,&(nl=sizeof(ls))))
		  {
			  hs_socket_utils::sockinfo si(&ls,nl);
			  psd->set_region(HSD_REMOTE_INFO,si.str,si.cb);
		  }			
		  hr=WSAGetLastError();
	  }



	  int cb;
	  char* p,*pu,*ps;


	  if(sg)
	  {

		  wbs_sh1_encoder_t wbs_sh1(WBSOCKET_UUID);

		  std::strstream outs;
		  char* reconnect_port=(psd)?psd->regions[HSD_CONN_PORT].ptr:NULL;

	    set_websocket_GET_8(outs,wbs_sh1,hostport,path,argszz,reconnect_port);
		int cbs=outs.pcount(),cb=0;
		 p=pu=outs.str();
         int c=cbs;

        if(!c) return INVALID_SOCKET;

		while(c>0) 
		{
			cb=send(sg,p,c,0);
			if(cb<=0)
			{
				if(psd) psd->reply_state|=0x80000000;
				return INVALID_SOCKET;
			}
			c-=cb;
			p+=cb;
		} 
		

		//if(cb<cbs)


		if(psd)
		{
			psd->reply_state=0x0000FFFF;
			psd->set_region(HSD_REQUEST,pu,cbs+1);

		}

		


		{

			v_buf<char> buf_replay; 
			std::strstream i1;
			cb=get_recv_line(i1,sg);
			if(cb<=0)   return INVALID_SOCKET;
			i1<<'\0';
			p=i1.str();
			buf_replay.cat(p);
			if(memcmp(p,"HTTP/",5)) return INVALID_SOCKET;
			p+=5+3;
			if(atoi(p)!=101)
				return INVALID_SOCKET;




			std::strstream ins;

			cb=get_recv_lines(ins,sg);
			
			if(cb<=0)   return INVALID_SOCKET;
			ins<<'\0';
			p=ins.str();

			buf_replay.cat(p);
			hss_port_base::region_ptr rh;
			rh.sizeb=buf_replay.size_b();
			rh.ptr=buf_replay.get();


			if(psd)
			{
				psd->reply_state|=0x0003FFFF;
				psd->set_region(HSD_REPLY,rh.ptr,rh.sizeb);				
			}

			p=_GET_HEADER(Sec-WebSocket-Accept,rh,&cb);
			if(!wbs_sh1.check(p,cb))
				return INVALID_SOCKET;



		}



       //if(psd) 		   psd->fv8=true;

       return sock_holder.detach();
	  }
	  else return INVALID_SOCKET;



}

inline SOCKET websocket_connect2(char* hostport,char*path ,char* argszz=0,wbkeys* pwbk=&wbkeys(GetTickCount()),void* hss=0)
{
    HRESULT hr;
	HANDLE habortevent=INVALID_HANDLE_VALUE;
	typedef socket_port<hss_port<> >::SOCKET_DATA_h hss_data_t;
	hss_data_t* psd=i_handle_cast<hss_data_t>(hss);
	if(psd)
	{

	
	if(psd->hkcount>1)
			habortevent=psd->hkObjects[0];
	}


   int cb;
   char* p,*pu,*ps;
	//SOCKET s;
	
	//s = tcp_connect_by_string(hostport);

	//sockgetter sg(s);
       SOCKET   s=tcp_connect_by_string(hostport,habortevent);


   if(psd)
   {
	   SOCKADDR_IN ls;
	   int nl;
	   if(SOCKET_ERROR!=getsockname(s,(SOCKADDR*)&ls,&(nl=sizeof(ls))))
	   {
		   hs_socket_utils::sockinfo si(&ls,nl);
		   psd->set_region(HSD_LOCAL_INFO,si.str,si.cb);
	   }
	   if(SOCKET_ERROR!=getpeername(s,(SOCKADDR*)&ls,&(nl=sizeof(ls))))
	   {
		   hs_socket_utils::sockinfo si(&ls,nl);
		   psd->set_region(HSD_REMOTE_INFO,si.str,si.cb);
	   }			
	   hr=WSAGetLastError();
   }

       sockgetter sockholder(s);
	   sockgetter_memorized sg(s,habortevent);

	if(sg)
	{
        
		
		std::strstream outs;
		char* reconnect_port=(psd)?psd->regions[HSD_CONN_PORT].ptr:NULL;

		set_websocket_GET(outs,hostport,path,argszz,pwbk,reconnect_port);
		//set_websocket_GET(outs,hostport,path,argszz,0);
		cb=outs.pcount();
		//outs<<"\0";
		//cb0=outs.pcount();
		
		int cbs=outs.pcount();
		pu=outs.str();
		cb=send(sg,pu,cbs,0);
		if(psd)
		{
			 psd->reply_state=0x0000FFFF;
             psd->set_region(HSD_REQUEST,pu,cbs+1);
			 
		}

			

		if(cb!=cbs)
		{
         ///printf("%d==send(%d)?",cb,cbs);
			if(psd) psd->reply_state|=0x80000000;
			return INVALID_SOCKET;
		}
		//if(cb<=0)   return INVALID_SOCKET;
       //printf("repl coun=?");

		if(1)
		{

        v_buf<char> buf_replay; 
		std::strstream i1;
		cb=get_recv_line(i1,sg);
		if(cb<=0)   return INVALID_SOCKET;
		i1<<'\0';
		p=i1.str();
		buf_replay.cat(p);
		if(memcmp(p,"HTTP/",5)) return INVALID_SOCKET;
		p+=5+3;
        if(atoi(p)!=101)
			return INVALID_SOCKET;
		



		std::strstream ins;
        
		cb=get_recv_lines(ins,sg);

//		printf("repl coun=%d",cb);
		if(cb<=0)   return INVALID_SOCKET;
		ins<<'\0';
		p=ins.str();
		
		if(psd)
		{
			psd->reply_state|=0x0003FFFF;
			buf_replay.cat(p);
			psd->set_region(HSD_REPLY,buf_replay.get(),buf_replay.size_b());
			if((ps=StrStrIA(p,"Sec-WebSocket-Location"))&&(ps=StrStrIA(ps,"HSSH-client-port-confirm:")))
			{
                char* pb=ps;
				while((*pb)&&(isspace(*pb))) ++pb;
				char *pe=pb;
				while((*pe)&&(!isspace(*pe))) ++pe;
				int cb=int(pe)-int(pb);
				psd->set_region(HSD_CONN_PORT,pb,cb);
			}
				
			
		}

		
		//p[cb-1]=0;
		//		if(StrStrIA(p,"Sec-WebSocket-Location"))
		//if(1)

		if((pwbk)&&(StrStrIA(p,"Sec-WebSocket-Location")))
		{

			char buf[16];
			cb=sg(buf,16);
			if(cb<=0)  return INVALID_SOCKET;
			bool f=pwbk->check_hash(buf);
			if(!f)  return INVALID_SOCKET;
		}
		}



	}
	return sockholder.detach();
}


struct hostport_path_splitter
{
	std::vector<char> buf;
	char *hostport;
	char *path;

inline hostport_path_splitter&	init(char* url)
{

	if((url)&&(path=strchr(url,'/')))
	{
		int cb=int(path++)-int(url);
		buf.resize(cb+1);
		hostport=&buf[0];
		memcpy(hostport,url,cb);
		hostport[cb]=0;
	}
	else hostport=0;

	return *this;
};

    hostport_path_splitter():hostport(0),path(0){};
	hostport_path_splitter(char* url)
	{
      init(url);
	}
 

  inline operator bool()
  {
	  return hostport!=0;
  }

};

inline SOCKET websocket_connect(char* url,char* argszz=0,wbkeys* pwbk=&wbkeys(GetTickCount()))
{
	/*
	char* p=strchr(url,'/');
	if(!p) return INVALID_SOCKET;
	int cb=int(p)-int(url),cb0;
	v_buf<char> hostport(cb+1);
	memcpy(hostport,url,cb);

	
	char* path=p+1;
	*/

	hostport_path_splitter hps(url);
	SOCKET s;
	if(hps)
		 return  s=websocket_connect2(hps.hostport,hps.path,argszz,pwbk);
	else return INVALID_SOCKET;
	
};

template <class sockgetter>
int get_public_object_ref(sockgetter& sg,char** pps)
{

	std::strstream stream;

    int cb=get_wbs_string(stream,sg);

	//if(cb<=0) 
		 

	if((cb>0)&& (pps))
	{
     *pps=shared_heap<char>().allocate_holder(cb).detach();
      memcpy(*pps,stream.str(),cb);
	};

	return cb;

}





inline SOCKET websocket_connect_ex(char* hostport,char*path ,char* argszz=0,wbkeys* pwbk=0,char** pp_socket_ref=0,int dept=3,void* hss=0)
{     

	HANDLE habortevent=INVALID_HANDLE_VALUE;

	 
	  typedef socket_port<hss_port<> >::SOCKET_DATA_h hss_data_t;
	  hss_data_t* psd=i_handle_cast<hss_data_t>(hss);
	  if((psd)&&(psd->hkcount>1)) habortevent=psd->hkObjects[0];


	  char* pf;
	  //
	  if(pp_socket_ref) *pp_socket_ref=0;
	  
	  //	  pwbk->init(1111,22222,GetTickCount());



     SOCKET s;
      
	     if((s=websocket_connect8(hostport,path,argszz,hss))!=INVALID_SOCKET)
		 {
			 double ver=13;//8;

			 if(psd)
			 {psd->set_region(HSD_WS_VERSION,&ver,sizeof(double));
			  psd->fv8=1;}

			 return s;
		 };



          pwbk->reinit();

		 s=websocket_connect2(hostport,path,argszz,pwbk,hss);
	  sockgetter_memorized sg(s,habortevent);

	  if(!sg) return INVALID_SOCKET;

//      if(find_named_value(REF_ID_DEMAND,args_ZZ(argszz),&pf)&&atoi(pf))
	  if(atoi(named_value(REF_ID_DEMAND,args_ZZ(argszz),"0")))
	  {
		  ptr_holder<char> hsock_ref;
         int cb=get_public_object_ref(sg,hsock_ref.address());
		 char* p=hsock_ref;

		  std::vector<char*> vargs;
		  args_ZZ(p,0,vargs);
 
		 if((p)&&(*p=='^')&&(0<dept--))
		 {   
			 ++p;
            const char* new_path=named_value(":",vargs,(char*)0);
            if(!new_path) return INVALID_SOCKET;  
			const char* new_hostport=named_value("hp:",args_ZZ(p),hostport);

			return websocket_connect_ex(hostport,path,argszz,pwbk,pp_socket_ref,dept,hss);

		 }
		 if(pp_socket_ref) *pp_socket_ref=hsock_ref.detach();

	  }
	  
        //return  INVALID_SOCKET;
	  return sg.detach();
}

struct reguest_lines
{
   int fok;
  hss_port_base::region_ptr fullstr;
  hss_port_base::region_ptr url;
  hss_port_base::region_ptr httpver;
  hss_port_base::region_ptr headers;
  hss_port_base::region_ptr http_ns_prefix;
  hss_port_base::region_ptr ns_port;
  hss_port_base::region_ptr http_request;
  hss_port_base::region_ptr http_file;

  inline operator int(){return fok;}
};

struct url_data
{

	ptr_holder<char> ns_port;
    ptr_holder<char> params;
    ptr_holder<char> replay;
	ptr_holder<char> host;
	ptr_holder<char> version;
	ptr_holder<char> ws_extension;
	inline operator bool(){return (ns_port.p);};
};





template <class SocketGetter>
inline bool  make_wb_handshake(SocketGetter& sg,reguest_lines& rl,url_data& hdata)
{
	wbkeys wbk;
	charbuf skey1;
	charbuf skey2;

	hss_port_base::region_ptr& h=rl.headers;
	char* p;
	unsigned char* md5hash;
	int cb;
	 bool fmd5;
	  std::strstream stream;


	  


      p=_GET_HEADER(Upgrade,h,&cb);
     if(safe_cmpni(p,"WebSocket",cb)) return false;
       stream<<"HTTP/1.1 101 Web Socket Protocol Handshake\r\n";
	   stream<<"Upgrade: ";
	   stream.write(p,cb)<<"\r\n";




    p = _GET_HEADER(Connection,h,&cb);
	if(safe_cmpni(p,"Upgrade",cb)&&safe_cmpni(p,"keep-alive",(cb>10)?10:cb)) 
		return false;
	stream<<"Connection: ";
	//stream.write(p,cb)<<"\r\n";
	stream.write("Upgrade",7)<<"\r\n";


	p = _GET_HEADER(Sec-WebSocket-Version,h,&cb);

	double version=safe_atof(p);


//============================================
	char *pk1,*pk2;
	char body[16]={0};
	if(version>6)
	{


		if(p=hdata.ws_extension)
		{
			stream<<"Sec-WebSocket-Extensions: "<<p<<"\r\n";
		}

		fmd5=false;
		p=_GET_HEADER(Sec-WebSocket-Key,h,&cb);
        //wbs_sh1_encoder_t wsh1(p,cb,"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
		wbs_sh1_encoder_t wsh1(p,cb,WBSOCKET_UUID);// "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
		stream<<"Sec-WebSocket-Accept: "<<(char*)wsh1<<"\r\n";
		
		stream<<"\r\n";
		
		p=stream.str();
		cb=stream.pcount();
		hdata.replay=create_shared_buffer(cb,p);
        hdata.version=create_shared_buffer(sizeof(double),&version,sizeof(double));
		if(p=_GET_HEADER(Host,h,&cb))
			hdata.host=create_shared_buffer(cb,p);


				//i_handle::size_ref(hdata.replay)=cb;
		return true;
		//*/

	}
	else
	{
	  
      fmd5=(pk1=_GET_HEADER(sec-websocket-key1,h,&cb));
	  fmd5=fmd5&&(pk2=_GET_HEADER(sec-websocket-key2,h,&cb));
	  if(fmd5)
	  {
	  //if(recv(s,body,8,0)!=8) return false;
		if(sg(body,8,0)!=8) return false;
	  md5hash=wbk.co_init(pk1,pk2,body);
	  };
	}
	

     if(p=_GET_HEADER(Origin,h,&cb)) 
	 {
         if(fmd5) stream<<"Sec-";
		 stream<<"WebSocket-Origin: ";
		 stream.write(p,cb)<<"\r\n";
	 }

      if(p=_GET_HEADER(Host,h,&cb))
	  {
           if(fmd5) stream<<"Sec-";
		   stream<<"WebSocket-Location: ws://";
		   
		   stream.write(p,cb);
		   
		   
		   int c=rl.url.sizeb;

		   if(0)
			   for(int j=0;j<c;j++) 
			    if(rl.url.ptr[j]=='?') {
				   c=j;
				   break;
				}
		   stream.write(rl.url.ptr,c);
		   //stream.write(pu,c);
		   
		   
		   stream<<"\r\n";
		   hdata.host=create_shared_buffer(cb+1,p,cb);
	  }


	 if(p=_GET_HEADER(WebSocket-Protocol,h,&cb)) 
	 {
		 if(fmd5) stream<<"Sec-";
		 stream<<"WebSocket-Protocol: ";
		 stream.write(p,cb)<<"\r\n";
	 }

      stream<<"hssh-port-confirm: null\r\n";
      stream<<"\r\n";
	  
      if(fmd5)  stream.write((char*)md5hash,16);

	  p=stream.str();
	  cb=stream.pcount();
	   hdata.replay=create_shared_buffer(cb,p);
	   //i_handle::size_ref(hdata.replay)=cb;
   return true;
}

inline url_data  url_decode_parse(hss_port_base::region_ptr url)
{
	int cb;
      url_data ud;
	char* p=url.ptr;
	if(!p) return ud;

	char* pe=p+url.sizeb;
	//*pe=0;
	if(*p=='/') ++p;
    char* pb=p;
	
	
	
	while((p<pe)&&(*p!='?')&&(*p))++p;
	if(p>=pe) return ud;
	
	bool f=(*p=='?');
	//*p=0;
   	//ptr_holder<char> rget=shared_heap<char>().allocate_holder(cbrl+cbadd);
	cb=int(p)-int(pb);
	if(cb)
	{
		char *pl=p-1;
		if(*pl=='/') --cb;
		ud.ns_port=create_shared_buffer(cb+1,pb,cb);
	}
	if(!f)  return ud;

	++p;

     cb=int(pe)-int(p);
	
	ptr_holder<char> prms=create_shared_buffer(cb+2);
    
	char *pd=prms;
	int cd=0;
	while(p<pe)
	{
       char *ps=p;   
	   
	   while((!isspace(*p))&&(*p!='&')&&(*p!=';')&&(p<pe)) ++p;
	   
	   int c=int(p)-int(ps);
	   if(!url_unescape(ps,c,pd,&c)) break;
	   ++p; 
        //memcpy(pd,p,c);
		pd+=c+1;
		cd+=c+1;

	}
   	i_handle::size_ref(prms)=cd+1;

	  ud.params=prms;
      return ud;

};

template <class Stream,class Getter>
reguest_lines   get_reguest_line(Stream& stream,Getter& sget)
{
	reguest_lines rl={0};
		//reguest_lines();
	//std::strstream stream;
  int cb;
    
	rl.fullstr.sizeb=cb=get_recv_line(stream,sget);
	
	if(cb<=0) return rl;
    //stream<<'\0';
	//if(!stream.good()) return rl;

	char *p=stream.str();
	char* pend=p+cb;

	rl.fullstr.ptr=p;



	//if(memcmp(p,"GET",3)!=0) return rl;
	//p+=3;
	for(;(p<pend)&&(!isspace(*p));++p);   

    if(*p!=' ') return rl;
	rl.fok=1;
	++p;
	char* ph=rl.url.ptr=p;
	for(;(ph<pend)&&(*ph!=0x20);++ph);
    

    rl.url.sizeb = int(ph)-int(p);
    if(int(pend)-int(ph)< 9)   return rl; 

	ph++;
	//if((rl.fok=(memcmp(ph,"HTTP/",5)==0)))
	 
	for(p=pend-1;p>=ph;p--) 
		if(!isspace(*p)) 
			break;
	 int nps=int(p)-int(ph)+1;

	if(memcmp(ph,"HTTP/",5)==0)
	{
		rl.fok|=2;
	};

      rl.httpver.ptr=ph;
      rl.httpver.sizeb=nps;
	
	
    return rl;
}


//template <class stream_t >
//bool HTTP_skip_ns_port(stream_t& stream,hss_port_base::region_ptr r)



int HTTP_skip_ns_port(std::strstream& stream,reguest_lines &rl,std::string& sbuf)
{
	hss_port_base::region_ptr& r=rl.fullstr;
	char *pb=r.ptr,*pe=pb+r.sizeb;
	char *pnb=pb,*pne,*pnr,*p;
   int cb;
	int nc=r.sizeb;
	
	 while((pnb<pe)&&(*pnb!='/')) ++pnb;
	 if(*pnb!='/') return 0;
	 pnb++;

	 pne=pnb;
	 pnr=pnb;
	 
	 while((pne<pe)&&(*pne!='?')&&(!isspace(*pne))) ++pne;
	 
	 
	 if(p=find_iter("=::",3,pnb,pne))
	 {
       pnr=(p+=3);
	   rl.http_ns_prefix.ptr=pnb;
	   rl.http_ns_prefix.sizeb=int(pnr)-int(pnb);
	 }
	 else
	 {
         p=pnr=pnb;
		 /*
		 pnr=pne;
		 int cc=int(pnr)-int(pnb);
	     sbuf.insert(0,pnb,cc); 
		 sbuf.insert(cc,"=::",3); 

		 p=(char*)sbuf.c_str();
		 rl.http_ns_prefix.ptr=p;
		 p+=cc+3;
		 rl.http_ns_prefix.sizeb=cc+3;
		 */
	 }

	 int cbf=int(pne)-int(p);

     //int cbf=rl.http_ns_prefix.sizeb-cdd;       
	 char* pef=p+cbf,*pbb=p;
	 for(;(pbb<=pef)&&(!isspace(*pbb));++pbb){};
	 
	 
	 cbf=int(pbb)-int(p);
	 if((cbf>1)&&(p[cbf-1]=='?')) --cbf;
	 
	 rl.http_file.ptr=p;
	 rl.http_file.sizeb=cbf;


	    


/*
	 else
	 {
		 while((pnr<pne)&&(*pnr!=';')) ++pnr;
		 if(*pnr!=';') return false;
		 ++pnr;
	 }
*/	 
	 
	 
     

     int cboff=int(pnb)-int(pb);
     stream.write(pb,cboff);
	 if((pe>pnr)&&(pnr[0]=='/'))
		 ++pnr;
	 cb=int(pe)-int(pnr);
	 stream.write(pnr,cb);

     rl.http_request.ptr=stream.str();
	 cb=stream.pcount();

	 //rl.http_ns_prefix.ptr=rl.http_request.ptr+cboff;


     return cb;
}

	
template <class Getter>
int   get_ns_port(std::strstream& stream,Getter& sget,reguest_lines& rl)
{
            char buf[2]={0,0};
	unsigned char& c=((unsigned char*)buf)[0];
	int ib=0,ie=0;
	int cb;
	rl.fok=0;
	while((cb=sget(buf,1))>0)
	{
		stream.write(buf,1);
		++ib;
		if(isspace(c)||c==0)
			break;
	}
	
	if( (cb<=0)||(c=='\n') ) return cb;
	while((cb=sget(buf,1))>0)
	{
		stream.write(buf,1);
		++ib;
		if(c!=0x20)
			break;
	}
	if( (cb<=0)||(c=='\n') ) return cb;
	ie=ib;

	while((cb=sget(buf,1))>0)
	{
		stream.write(buf,1);
		if((c=='?')||(c==0x20)||(c=='\n')||(c==0))
			break;
		++ie;
	}
       if( (cb<=0)) return cb;
	   

	 char* p=stream.str();
	 if(p[ib]=='/') ++ib;
	 cb=stream.pcount();
	 rl.fullstr.ptr=p;
	 rl.fullstr.sizeb=cb;
	 rl.fok=1;
	 if(ie>ib)
	 {
	 rl.ns_port.ptr=p+ib;
	 rl.ns_port.sizeb=ie-ib;
	 p+=ie-1;
	 if(*p=='/')
		 --rl.ns_port.sizeb;

     rl.fok|=2; 
	 }
	return cb;
}


template <class hssport >
ptr_holder<hss_port_base::handshake_buffer> resolve_ns_port(void* ph,SOCKET s,void* pevent=0)
{

	ptr_holder<socket_port<hssport>::SOCKET_DATA_h> hss=i_handle_cast<socket_port<hssport>::SOCKET_DATA_h>(ph);	


	void* habortevent=(pevent)?pevent:INVALID_HANDLE_VALUE;


	char* pmem=0;
	int cbmem=0;


	if(hss)
	{
		pmem=hss->get_region(HSD_REQUEST,&cbmem);
		if(!hs_socket_utils::is_socket(s))
		{
			SOCKET socin=hss->reset_socket();
			s=socin;
		}
	}



	sockgetter_memorized sg(s,habortevent,cbmem,pmem);
	std::strstream stream;


	reguest_lines rl;

	if(get_ns_port(stream,sg,rl)<=0)
		return 0;


	if((rl.fok&1)&&sg.fsock)
	{
		ptr_holder<char> rget=create_shared_buffer(rl.fullstr.sizeb,rl.fullstr.ptr,rl.fullstr.sizeb);
		hss->attach_region(HSD_REQUEST,rget);
	}

	if((rl.fok&2))
	{
		ptr_holder<char> rget=create_shared_buffer(rl.ns_port.sizeb,rl.ns_port.ptr,rl.ns_port.sizeb);
		hss->attach_region(HSD_URL,rget);
	}

	return hss;

}

static inline bool _check_post(reguest_lines& rl){

	return (rl.fullstr.sizeb>5)&&(StrCmpNIA(rl.fullstr.ptr,"POST ",5)==0);

}

template <class hssport >
ptr_holder<hss_port_base::handshake_buffer> get_HTTP_request_headers(void* ph,SOCKET s,void* pevent=0,accept_address_data* paad=0)
{

     ptr_holder<socket_port<hssport>::SOCKET_DATA_h> hss=i_handle_cast<socket_port<hssport>::SOCKET_DATA_h>(ph);	
	

	void* habortevent=(pevent)?pevent:INVALID_HANDLE_VALUE;

//	if((paad)&&(paad->cb>=sizeof(*paad)))
//		habortevent=paad->shutdown;

	char* pmem=0;
	int cbmem=0;
	int fhttpcmd=0;

	
	if(hss)
	{
	   pmem=hss->get_region(HSD_REQUEST,&cbmem);
    	
	   if(!hs_socket_utils::is_socket(s))
	   {
		   SOCKET socin=hss->reset_socket();
		   s=socin;
	   }
	}

	
      
	sockgetter_memorized sg(s,habortevent,cbmem,pmem);




	   std::strstream stream;

	   reguest_lines rl;

	   rl=get_reguest_line(stream,sg);

	    hss->set_region(HSD_PROTOCOL,rl.httpver.ptr,rl.httpver.sizeb);

		url_data urld=url_decode_parse(rl.url);
		hss->attach_region(HSD_PARAMS,urld.params);


	   int cbrl = rl.fullstr.sizeb;

	   if(cbrl <= 0) return 0;

	   std::strstream stream2;

	   int cbadd=get_recv_lines(stream2<<'\n',sg);

       stream2<<'\0';

	   if(cbadd <= 0) return 0;
	   char* padd=stream2.str();

	   rl.headers.ptr=padd;
	   rl.headers.sizeb=cbadd+1;

	   char*p;
	   int cb;

	   if(p=_GET_HEADER(Host,rl.headers,&cb))
	   {
         hss->set_region(HSD_HOST,p,cb);  
	   }

	   if(_check_post(rl)&&(p=_GET_HEADER(Content-Length,rl.headers,&cb))){
		   hss->set_region(HSD_BODY_LENGTH,p,cb);

		   s_parser_t<char> parser;


		   if(p=_GET_HEADER_CR(Content-Disposition,rl.headers,&cb)){
			   char* pv=parser.get_cache(p,p+cb);
			   pv=parser._ltrim(pv);
			   if( (pv=parser._lskipi(pv,"attachment;"))&&(pv=parser._lskipi(parser._ltrim(pv),"filename=")))
			   {
				   pv=parser.undecorate(parser.trim(pv));
				   hss->set_region(HSD_UPLOAD_FILE_PATH,pv,-1);

			   }

		   }
		   else if(p=_GET_HEADER(upload-path,rl.headers,&cb)){
			   hss->set_region(HSD_UPLOAD_FILE_PATH,p,cb);  
		   }




	   }

	   /*
	   if(p=_GET_HEADER(Content-Length,rl.headers,&cb))
	   {
		   bool fpost=StrCmpNIA(rl.fullstr.ptr,"POST ",5)==0;
		   hss->set_region(HSD_BODY_LENGTH,p,cb);
		   if(fpost){
		   if(p=_GET_HEADER(upload-path,rl.headers,&cb)){

		   }else if(p=_GET_HEADER(Content-Disposition,rl.headers,&cb)){
			   if(p&&(rl.fullstr.sizeb>5)){
				   if(StrCmpNIA(rl.fullstr.ptr,"POST ",5)==0){
					   //fhttpcmd=1;
					   hss->set_region(HSD_UPLOAD_FILE_PATH,p,cb);  
				   }
			   }

		   }
		   


	   }
	   */

      if(!hss) hss=socket_port<hssport>::create_SOCKET_DATA(true);


	  if(sg.fsock)
	  {
	    ptr_holder<char> rget=create_shared_buffer(cbrl+cbadd,rl.fullstr.ptr,cbrl);
	    char *p=rget;
	    memcpy(p+cbrl,padd+1,cbadd);
    	hss->attach_region(HSD_REQUEST,rget);
	  }

		 std::strstream stream3;

		bool f=(hss->get_region(HSD_REQUEST_HTTP)==0);
		if(f)
		{
          //char* p=stream3.str(),*ph;
          //int cbh=stream3.pcount();
			std::string sbuf;
          int cbh=HTTP_skip_ns_port(stream3,rl,sbuf);
		  char* p=rl.http_request.ptr;

		  if(cbh>0)
		  {
		  	ptr_holder<char> rgeth=create_shared_buffer(cbh+cbadd,p,cbh);
			char*ph=rgeth.p;
			memcpy(ph+cbh,padd+1,cbadd);
		  	hss->attach_region(HSD_REQUEST_HTTP,rgeth);
			rgeth=create_shared_buffer(rl.http_ns_prefix.sizeb,rl.http_ns_prefix.ptr,rl.http_ns_prefix.sizeb);
			hss->attach_region(HSD_HTTP_URL_BASE,rgeth);

			//url_unescape(char* src,int cb,char* dest_buf,int* pbufsize=0)

			int cb=rl.http_file.sizeb;
			v_buf<char> buf(cb+16);
			char* pfile=buf;
			url_unescape(rl.http_file.ptr,cb,pfile,&cb);
			pfile[cb]=0;
			//char_mutator<CP_UTF8> cmm(pfile);
			//wchar_t* pwfile=cmm;
			hss->set_region(HSD_HTTP_FILE,pfile,cb);
			int c=0;
			for(;c<cb;++c) if(pfile[c]=='/') break;
            hss->set_region(HSD_HTTP_VIRTUAL_ROOT,pfile,c);
			hss->set_region(HSD_HTTP_VIRTUAL_FILE,pfile+c,cb-c);

			rgeth.release();
			hss->regions[HSD_REQUEST_POS]=hss->regions[HSD_REQUEST_HTTP];
		  }
		}
		else hss->regions[HSD_REQUEST_POS]=hss->regions[HSD_REQUEST];

       
       
	   
	   if(paad)
	   {
		   hss->set_region(HSD_LOCAL_INFO,paad->plocal_address,paad->local_length);
		   hss->set_region(HSD_REMOTE_INFO,paad->premote_address,paad->remote_length);
	   }
	   return hss;

};


template <class hssport >
ptr_holder<hss_port_base::handshake_buffer> get_ws_handshake(void* ph,SOCKET s,void* pevent=0,accept_address_data* paad=0)
{
     
    ptr_holder<socket_port<hssport>::SOCKET_DATA_h> hss=i_handle_cast<socket_port<hssport>::SOCKET_DATA_h>(ph);
		;//=ph;

		void* habortevent=(pevent)?pevent:INVALID_HANDLE_VALUE;
            
           //SOCKET socin=INVALID_SOCKET;  
	       char* pmem=0;
		   int cbmem=0;
		   url_data urld;

           //hss.attach(i_handle(ph,true);
		   if(hss)
		   {

             urld.ws_extension=hss->get_region(HSD_WS_EXTENSION);
             pmem=hss->get_region(HSD_REQUEST,&cbmem);
			 if(!hs_socket_utils::is_socket(s))
			 {
				 SOCKET socin=hss->reset_socket();
				 s=socin;
			 }

		   }

            


	//if((paad)&&(paad->cb>=sizeof(*paad)))		habortevent=paad->shutdown;
		 
	
	//hss->attach_region(HSD_REQUEST,rget);

//
/*
	{
		    void* h=paad->shutdown;
			event_signaler* pes=i_handle_cast<event_signaler>(h);
			if(pes) habortevent=pes->handle();
			else habortevent=HANDLE(h);
	}
//*/



#ifdef DEBUG
	std::vector<char*> vtest;
	//	sockgetter_base sg=s;
#endif

	//
	sockgetter_memorized sg(s,habortevent,cbmem,pmem);

	 std::strstream stream;

	 reguest_lines rl;

      rl=get_reguest_line(stream,sg);

	  int cbrl = rl.fullstr.sizeb;

	  if(cbrl <= 0) return 0;

      std::strstream stream2;

	  int cbadd=get_recv_lines(stream2<<'\n',sg);

       if(cbadd <= 0) return 0;
	   char* padd=stream2.str();
	   
	   rl.headers.ptr=padd;
	   rl.headers.sizeb=cbadd+1;


	   if(!hss) hss=socket_port<hssport>::create_SOCKET_DATA(true);


	   if(sg.fsock)
	   {
		   ptr_holder<char> rget=create_shared_buffer(cbrl+cbadd,rl.fullstr.ptr,cbrl);
		   char *p=rget;
		   memcpy(p+cbrl,padd+1,cbadd);
		   hss->attach_region(HSD_REQUEST,rget);
	   }


/*
      
      ptr_holder<char> rget=create_shared_buffer(cbrl+cbadd,rl.fullstr.ptr,cbrl);
	  char *p=rget;
	  memcpy(p+cbrl,padd+1,cbadd);
 
     if(!hss) hss=socket_port<hssport>::create_SOCKET_DATA(true);
	  
	 hss->attach_region(HSD_REQUEST,rget);
*/
	 
	 if(paad)
	 {
		 hss->set_region(HSD_LOCAL_INFO,paad->plocal_address,paad->local_length);
		 hss->set_region(HSD_REMOTE_INFO,paad->premote_address,paad->remote_length);
	 }
       
    if(rl)
	{
     
	 bool fwb=make_wb_handshake(sg,rl,urld);
     
		 //;// 	 dup_socket(s)	
	 {

	      hss->attach_region(HSD_WS_VERSION,urld.version);
          hss->attach_region(HSD_HOST,urld.host);
		  hss->set_region(HSD_PROTOCOL,rl.httpver.ptr,rl.httpver.sizeb);

		  if(fwb)  hss->attach_region(HSD_REPLY,urld.replay);

	     urld=url_decode_parse(rl.url);

	  if(urld)
	  {
		  hss->attach_region(HSD_URL,urld.ns_port);
		  hss->attach_region(HSD_PARAMS,urld.params);

//
		  argv_zz<char> test(hss->get_region(HSD_PARAMS));

                

		  //
		  hss->reset_socket(HSF_SET,s);
		  //
		  /*
		   SOCKET fd=hs_socket_utils::dup_socket(s);	
			SOCKET sold=hss->reset_socket(HSF_SET,fd);
			if(hs_socket_utils::is_socket(sold))
				::closesocket(sold);
		//*/
			
			
			
	  }
	 }

#ifdef DEBUG
         args_ZZ((char*)urld.params,0,vtest);
#endif
	}

     return hss;
	 		 
//	 hss



}



struct pool_socket_server
{
	hs_socket_utils::host_port_resolver hpr;
	hs_socket_utils::handle_holder habortevent;
	hs_socket_utils::CS mutex;
	int handlerN;
	typedef pool_socket_server* this_t;
	typedef hs_socket_utils::host_port_resolver::accept_data  accept_data;
	typedef accept_data*  paccept_data;
    typedef hss_port<> hssport;
	typedef socket_port<hssport>::SOCKET_DATA_h SOCKET_DATA;
	HRESULT hr;
	void* on_accept_proc,*pweak_context;
	ptr_holder<char> proc_holder;
    ptr_holder<event_signaler> abort_proc;

	inline ptr_holder<event_signaler> get_event_signaler()
	{
		return abort_proc;
	}
   inline void* get_io_signaler()
   {
	   void* p=proc_holder.p;
	   return p=i_handle_cast<completion_port_signaler>(p);
   }
	pool_socket_server(char* addressport,void* proc,void* proch=HSD_SAME_PTR,int nh=0)
		:hpr(addressport),habortevent(::CreateEvent(0,1,0,0))
	{
		handlerN=nh;
		if(!proc) {hr=E_POINTER;return;};

		HRESULT err=hpr.hr;

         proc_holder=make_callback_proc(proc,proch,on_accept_proc);
		 pweak_context=(proch!=HSD_SAME_PTR)?proch:0;

/*
         if(proc==HSD_IO_PORT_PTR)
                proc_holder.pv=on_accept_proc=hss_create_completion_port_closure(0,ULONG_PTR(proch));
		 else
		 {
			 void* p=hss_get_io_signaler(proc);
			 //void* p=i_handle::verify_ptr(proc);
			 //p=0;
			 if(p)
			 {
				 proc_holder.pv=on_accept_proc=hss_create_completion_port_closure(p,ULONG_PTR(proch));
				 
			 }
			 else
			 {
              on_accept_proc=proc;
		      proc_holder=(char*)((proch==HSD_SAME_PTR)?proc:proch);
			 }
		 }

*/
				   abort_proc=make_event_signaler(habortevent);
         hpr.hr=err;

           //(*abort_proc.p)();
			//sh_new_execute(closure_stub)(&::SetEvent,habortevent.get());
		   hr=0;
		   
		if(hpr)
		{
			
			hpr.ovl.hkObjects[0]=habortevent;
			hpr.ovl.hkcount=2;
			
			
		}
		else hr=hpr.hr;
		{
		//	hs_socket_utils::locker_t<hs_socket_utils::CS> lock(mutex);
		}
       //hr=hpr.hr;
       QueueUserWorkItem((LPTHREAD_START_ROUTINE)&s_proc,(void*)this,WT_EXECUTELONGFUNCTION); 		

	}
	~pool_socket_server()
	{
		SetEvent(habortevent);
		hs_socket_utils::locker_t<hs_socket_utils::CS> lock(mutex);
		hs_socket_utils::LastErrorSaver(hr);
   	}
///*


    int handler1()
	{

		HRESULT err;	
		accept_data* pad=new accept_data;
		pad->abortproc=abort_proc;
		pad->pcallback=on_accept_proc;

		if((err=hpr.accept(pad))==E_ABORT) 
		{
			delete pad;
			return 1;
		}
		//if(hpr.hr==E_ABORT) return ;
		if(err==0)
		{

			if((on_accept_proc)&&QueueUserWorkItem(&a_proc,(void*)pad,WT_EXECUTELONGFUNCTION));
			else {

				CancelIo(HANDLE(pad->s));
				::closesocket(pad->s);
				delete pad;
			};
		}

        return 0;

	};


	int handler2()
	{
    

        HRESULT err;	
		accept_data ad;
		//pad->abortproc=abort_proc;
		//pad->pcallback=on_accept_proc;

		if((err=hpr.accept(&ad))==E_ABORT) 
		{
			    closure_stub::safe_call(on_accept_proc,0);
				return 1;
		}
		//if(hpr.hr==E_ABORT) return ;

		  ptr_holder<SOCKET_DATA> hss=socket_port<hssport>::create_SOCKET_DATA(true,ad.s);

		  hss->abortproc=abort_proc;
		  hss->pcallback=on_accept_proc;
		  hss->user_data=(ULONG_PTR)pweak_context;
		  {
			  hs_socket_utils::sockinfo si(ad.pls,ad.cbls);
			  hss->set_region(HSD_LOCAL_INFO,si.str,si.cb);
		  }
		  {
             hs_socket_utils::sockinfo si(ad.prs,ad.cbrs);
             hss->set_region(HSD_REMOTE_INFO,si.str,si.cb);
		  }
		  

		if(err==0)
		{

			if((on_accept_proc)&&QueueUserWorkItem(&a_proc2,hss.pv,WT_EXECUTELONGFUNCTION))
			{
				hss.detach();
			}
			else {

				CancelIo(HANDLE(hss->s));
				::closesocket(hss->s);
				
			};
		}

		return 0;

	};

	inline void    loop()
	{
	     
//
	
		try
		{
			
			hs_socket_utils::locker_t<hs_socket_utils::CS> lock(mutex);
			//InterlockedExchangeAdd((long volatile*)&hr,0);
			hpr.hr=hr;
			if(hpr.bind())
			{
				
             while(1)
			 {
                 if(handlerN==0)
				 {
                   if(handler1()) break;
				 }
				 else if(handler2()) break;

				 

				//((LPTHREAD_START_ROUTINE)on_accept_proc)((void*)s);

			 }

			}
			//hr=hpr.hr;       
		}
    	catch (...){

		}
			
	}



	static DWORD __stdcall a_proc2(void* p)
	{
		try
		{
			SOCKET_DATA* pad=(SOCKET_DATA*)p;

			
				closure_stub::safe_call(pad->pcallback,pad);

				
			
			i_handle::release(pad);

			//delete (SOCKET_DATA*)777777;

		} catch (...)
		{

		}
		return 0;
	}

	static DWORD __stdcall a_proc(void* p)
	{
		try
		{
		paccept_data pad=paccept_data(p);
			
			try
			{
				closure_stub::safe_call(pad->pcallback,pad->abortproc,(void*)pad->s,pad);
			}
			catch (...)	{
				CancelIo(HANDLE(pad->s));
				::closesocket(pad->s);
				}
	    
			delete pad;

		} catch (...){}
		return 0;
	}
	static unsigned int __stdcall s_proc(void* p)
	{
		
			this_t(p)->loop();
		return 0;
	}
//*/
	static void  __stdcall exit_proc(void* p)
	{
		delete this_t(p);
	}

};



//int get_recv_line(Stream& s,const Getter& g)

#pragma pack(pop)