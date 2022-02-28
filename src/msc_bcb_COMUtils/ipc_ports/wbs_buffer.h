#pragma once


#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>


#include <list>
#include <utility>
//#include "aligned_allocator.h" 
#include "wchar_parsers.h"
#include "wbs_def.h"
#define __DIG 12
#pragma pack(push)
#pragma pack(1)



template <class T,class H>
struct wbs_op_t
{
	typedef wbs_op_t<T,H> base_t; 

	HRESULT hr;
	H* hovl;
	DWORD fprocessmessages;

	static 	bool wait_msg_alertable(HANDLE* ph,int nh=1)
	{
		bool fo,fa;
		DWORD ws;
		MSG msg;
		for(;;){
			ws=MsgWaitForMultipleObjectsEx(nh,ph,INFINITE,QS_ALLINPUT,MWMO_INPUTAVAILABLE|MWMO_ALERTABLE);
			fo=((WAIT_OBJECT_0<=ws)&&(ws<(WAIT_OBJECT_0+nh)));
			fa=((WAIT_ABANDONED<=ws)&&(ws<(WAIT_ABANDONED+nh)));
			if(fo||fa)
				break;

			while(PeekMessage(&msg,0,0,0,PM_REMOVE))
		 {
			 if((msg.message==WM_QUIT)) return false;

			 TranslateMessage(&msg);
			 DispatchMessage(&msg);

		 }
		} ;

		return WAIT_OBJECT_0==ws;
	};

	wbs_op_t(H* _hovl,DWORD _fprocessmessages=0):hovl(_hovl),hr(E_FAIL),fprocessmessages(_fprocessmessages){};
	inline operator HRESULT(){ return HRESULT_FROM_WIN32(hr);}

	int op(void *pbuf,int cb,int flags=0)
	{
		WSABUF wb={cb,(char*)pbuf};
		T* t=static_cast<T*>(this);
		return (*t)(1,&wb);
	}

	int  check_res(int res)
	{
		DWORD cbtot;


		if((!res)||((hr=WSAGetLastError())==WSA_IO_PENDING))
		{
			/*if(fprocessmessages)
			{
			bool f= wait_msg_alertable(&hovl->hEvent,hovl->hkcount);
			if(!f) 
			{
			hr=ERROR_GRACEFUL_DISCONNECT;
			return -1;
			}
			}
			*/
			if(GetOverlappedResult((HANDLE)hovl->s,hovl,&cbtot,true))
			{
				hr=(cbtot>0)?S_OK:ERROR_GRACEFUL_DISCONNECT;
				return cbtot;					 					 
			}
			else hr=WSAGetLastError();

		}
		return -1;

	}
};

template <class HSS>
struct wbs_getter_t:wbs_op_t<wbs_getter_t<HSS>,HSS>
{
	wbs_getter_t(HSS *h,DWORD _fprocessmessages=0):wbs_op_t<wbs_getter_t<HSS>,HSS>(h,_fprocessmessages){};

	int operator()(void *pbuf,int cb,int flags=0)
	{        
		return op(pbuf,cb,flags);
	}
	int operator()(int nbuf,WSABUF *pbuf)
	{
		int cbtot=0;
		DWORD flags=0;
		int rc=WSARecv(hovl->s,pbuf,nbuf,(DWORD*)&cbtot,&flags,hovl,0);
		return cbtot=check_res(rc);
	}

};

template <class HSS>
struct wbs_setter_t:wbs_op_t<wbs_setter_t<HSS>,HSS>
{
	wbs_setter_t(HSS *h,DWORD _fprocessmessages=0):wbs_op_t<wbs_setter_t<HSS>,HSS>(h,_fprocessmessages){};

	int operator()(void *pbuf,int cb,int flags=0)
	{        
		return op(pbuf,cb,flags);
	}
	int operator()(int nbuf,WSABUF *pbuf)
	{
		int cbtot=0;
		DWORD flags=0;
		int rc=WSASend(hovl->s,pbuf,nbuf,(DWORD*)&cbtot,flags,hovl,0);
		return cbtot=check_res(rc);
	}

};


struct wbs_frame_io_t
{

	typedef unsigned char uint8_t;
	typedef unsigned int uint32_t;
	typedef unsigned short uint16_t;
	typedef unsigned long long uint64_t;
	typedef  long long int64_t;


	struct wbs_frame_t:wbs_frame_base_t
	{

		enum
		{
			trush_size=4096
		};

		inline static char* trush()
		{
			static char _trush[trush_size];
			return _trush;
		}


		void*	remask(void* bufd,void* bufs,int cb)
		{

			if(mask) 
			{


				uint8_t* pbd=(uint8_t* )bufd;
				uint8_t* pbs=(uint8_t* )bufs;

				uint8_t ups=(posb[0])&0x3;

				for(int n=0;n<cb;++n)
				{
					pbd[n]=pbs[n]^maskb[ups];
					ups=(ups+1)&0x3;

				};
			}
			else if(bufd!=bufs)
				memcpy(bufd,bufs,cb); 
			pos+=uint64_t(cb);
			return bufd;
		}

		void*	remask(void* buf,int cb)
		{

			if(mask) 
			{


				uint8_t* pb=(uint8_t* )buf;


				uint8_t ups=(posb[0])&0x3;

				for(int n=0;n<cb;++n)
				{
					pb[n]^=maskb[ups];
					ups=(ups+1)&0x3;

				};
			}
			pos+=uint64_t(cb);
			return buf;
		}

		inline int size()
		{
			return  cbsz+2;  
		}

		template <class N>
		inline int append_buf(N** p=0)
		{
			get_len_mask(false);

			if(cbsz&&p) *p=(N*)inner_buf;

			return cbsz;
		}

		wbs_frame_t & get_len_mask(bool ff=true)
		{

			cbsz=0;
			if(f01.f1.len>=126)
			{
				cbsz=(f01.f1.len==127)?8:2;
				len=0;
				if(ff) for(int n=0,nb=cbsz-1;n<cbsz;++n,--nb)
					lenb[n]=inner_buf[nb];

			}
			else len=f01.f1.len;

			if(f01.f1.mask)
			{

				uint32_t* pm=(uint32_t*)(inner_buf+cbsz);
				if(ff) mask=pm[0];
				cbsz+=4;

			}
			return *this;
		}

		wbs_frame_t & set_len_mask()
		{
			int cbl;
			cbsz=0;
			if(len<126) 
			{
				//cbh=sizeof(wbsf.f01);
				f01.f1.len=len;
			}
			else
			{

				if(len>=0x10000)
				{
					cbl=8;
					f01.f1.len=127;
				}
				else 
				{
					cbl=2;
					f01.f1.len=126;

				}

				for(int n=0,nb=cbl-1;n<cbl;++n,--nb)
					inner_buf[n]=lenb[nb];



				cbsz+=cbl;
			}


			if(f01.f1.mask)
			{

				uint32_t* pm=(uint32_t*)(inner_buf+cbsz);
				pm[0]=mask;
				cbsz+=4;

			}

			return *this;
		}

		inline int64_t tail()
		{
			return int64_t(len)-int64_t(pos);
		}

		inline bool eom()
		{
			return (f01.f0.fin)&&(tail()<=0);
		}

		inline void clear(uint8_t s=0)
		{
			memset(this,0,sizeof(wbs_frame_t));
			state=s;

		}






		template <class Getter>
		static int  recv(Getter& sget,void * buf,int cb)
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

		inline HRESULT check_graceful_disconnect(HRESULT hr,int cb)
		{
			return ((hr==S_OK)&&(cb<=0))?ERR_DCN:hr;
		}

		template <class Getter> bool recv_header(Getter& sget,HRESULT& hr,bool f_pong_auto=true)
		{
			int cc,c;

			bool f_pp=false;

			do{
				pos=0;
				len=0;
				if(sizeof(f01)!=(cc=recv(sget,&f01,cc=sizeof(f01))))
					return hr=check_graceful_disconnect(sget,cc),false;

				uint8_t opc=f01.f0.opcode;

				if(opc!=0)
				{


					if(opc==WSF_CLO) 
						return hr=ERR_DCN,false;
					opcode=opc;
				}
			}
			while(f_pp);

			char* p;   
			cc=append_buf(&p);
			if(cc)
			{  
				c=recv(sget,p,cc);

				if(cc==c) 
					get_len_mask(true);
				else
					return hr=check_graceful_disconnect(sget,c),false;
			}

			return true;


		}

		template <class Getter>
		HRESULT skip_frames(Getter& sget)
		{
			HRESULT hr;
			//if(eom())  return hr=ERR_EOM;
			//f01.f1.mask=0;
			int cc;
			char* p=(char*)trush();
			while(!eom())
			{
				if(FAILED(hr=read_frames(sget,0,trush(),trush_size,&cc)))
					return hr;
			}


			//while(S_OK==(hr=read_frames(sget,0,trush(),trush_size,&cc)));
			return hr=ERR_EOM;
		}

		template <class Getter>
		HRESULT read_frame(Getter& sget,void* buf=0,int cb=0,int *pcbread=0)
		{
			int ccc,c;
			int& cbread=(pcbread)?*pcbread:ccc;
			HRESULT hr=S_OK;
			int64_t ct=tail();
			if(ct<=0)
			{
				if(f01.f0.fin) 
					return cbread=0,ERR_EOM;

				if(!recv_header(sget,hr)) 
					return hr;
			};

			ct=tail();
			int cbr=(cb<ct)?cb:ct;

			if(cbr!=(c=recv(sget,buf,cbr)))
				return hr=check_graceful_disconnect(sget,c);
			else hr=S_OK;
			remask(buf,cbr);
			cbread=cbr;

			return hr;
		}

		template <class Getter>
		HRESULT read_frames(Getter& sget,int fnext,void* buf=0,int cb=0,int *pcbread=0)
		{
			HRESULT hr=S_OK;
			int ccc,c;
			int& cbread=(pcbread)?*pcbread:ccc;

			uint8_t st=state;


			if(st==0)
			{
				clear(1);
				if(!recv_header(sget,hr)) 
					return hr;
			}
			else
			{
				if(fnext)
				{
					if(SUCCEEDED(hr=skip_frames(sget)))
					{
						state=0;
						hr=read_frames(sget,0,buf,cb,pcbread);
					}
					return hr;					  					  
				}

				//if(eom())						  return cbread=0,ERR_EOM;


			}

			int cbr=0,cbt=0;
			char* p=(char*) buf;

			if((cb==0)&&eom()) 
				return cbread=0,ERR_EOM;



			while(cb>0)
			{
				c=0;    
				hr=read_frame(sget,p,cb,&c);
				if(FAILED(hr)) return hr;
				cb-=c;
				p+=c;
				cbr+=c;
				if(hr) break;
			}

			cbread=cbr;

			return hr;

		}




	};


	wbs_frame_io_t()
	{
		clear_r();
	}
	inline void clear_r()
	{
		err_r=0;
		memset(&wbs_frame,0,sizeof(wbs_frame)); 
	}

	inline void clear_w()
	{
		err_w=0;
		//memset(&wbs_frame_w,0,sizeof(wbs_frame_w)); 
	}


	/*
	template <class Getter>
	HRESULT read_frames(Getter& sget,int fnext,void* buf=0,int cb=0,int *pcbread=0)
	{
	HRESULT hr;
	int64_t cbtot=0,cc;
	while(read_frame(sget,fnext))


	}
	*/



	template <class Getter>
	HRESULT read_frame(Getter& sget,int fnext,void* buf=0,int cb=0,int *pcbread=0)
	{
		int cc;
		int& cbr=(pcbread)?*pcbread:cc;
		HRESULT hr;
		bool feom=wbs_frame.eom();
		bool fb=wbs_frame.frame_count==0;


		if(wbs_frame.tail()==0)
		{

			//if(wbs_frame.f01.f0.opcode) return hr=WSF_END; 

			if(wbs_frame.f01.f0.fin||fb) 
			{	

				if(fnext)
				{
					wbs_frame.clear();
					++wbs_frame.frame_count;
					if(cc!=recv(sget,&wbs_frame.f01,cc=sizeof(wbs_frame.f01)))
						return hr=sget;
					if(wbs_frame.f01.f0.opcode==WSF_CLO) return WSF_CLO;
					if(fb) wbs_frame.opcode=wbs_frame.f01.f0.opcode;

					char* p;   
					cc=wbs_frame.append_buf(&p);
					if(cc)
						if(cc==recv(sget,p,cc)) 
							wbs_frame.get_len_mask(true);
						else
							return hr=sget;


				}
				else return hr=ERROR_HANDLE_EOF;         
			}
		}
		else 
			if(fnext)
			{
				bool smask=wbs_frame.f01.f1.mask;
				wbs_frame.f01.f1.mask=0;
				while((hr=read_frame(sget,0,trush(),trush_size))==S_OK);
				wbs_frame.f01.f1.mask=smask;
				if(hr==ERROR_HANDLE_EOF) 
				{
					wbs_frame.clear();
					hr=read_frame(sget,fnext,buf,cb,pcbread);
				}
				return hr;

			}




			int64_t ct= wbs_frame.tail();

			if(ct>0)
			{
				int c=(ct>cb)?cb:ct;
				;
				if(c==recv(sget,buf,c))

					wbs_frame.remask(buf,cbr=c);		
				else return hr=sget;

			}
			else cbr=0;

			feom=wbs_frame.eom();
			return feom?ERROR_HANDLE_EOF:S_OK;

	}


	static  wbs_frame_t make_header(uint16_t fields=WSF_FIN|WSF_TXT,int64_t len=0,uint32_t mask=0)
	{
		wbs_frame_t wbsf;
		wbsf.clear();
		wbsf.len=len;
		wbsf.mask=mask;
		//memset(&wbsf,0,sizeof(wbsf)); 
		wbsf.fields=fields;	
		return wbsf.set_len_mask();
	}



	template <class Setter>
	static HRESULT write_frame(Setter& sset,void *buf,int cb,int* pcbwrite=0,uint16_t fields=WSF_FIN|WSF_TXT,uint32_t mask=0)
	{
		HRESULT hr;
		int cc;
		int& cbw=(pcbwrite)?*pcbwrite:cc;

		wbs_frame_t wf=make_header(fields,cb,mask);

		if(cbw!=send(sset,&wf,cbw=wf.size())) 
			return hr=sset;
		if(cb<=0) 
			return  cbw=0,S_OK;

		if((wf.f01.f1.mask)&&mask) 
		{
			const int bsz=256;
			char bufm[bsz], *ps=(char*)buf;
			int c;
			int nn=cb/bsz;
			cbw=0;

			for(int n=0;n<nn;++n)
		 {
			 void* p=(mask)?wf.remask(bufm,ps,bsz):ps;

			 c=send(sset,p,bsz); 
			 if(c<=0) return hr=sset;
			 ps+=c;
			 cbw+=c;
		 }
			if((c=cb-cbw)>0)
		 {
			 void* p=(mask)?wf.remask(bufm,ps,c):ps;
			 c=send(sset,p,c); 
			 if(c<=0) return hr=sset;
			 cbw+=c;
		 }


		}
		else  cbw=send(sset,buf,cb);

		return (cbw>0)? 0: hr=sset;

	};



	static int set_header( wbs_frame_t *pwbsf,int64_t cbw,int opcode=1,uint32_t mask=0)
	{
		wbs_frame_t wbsf;
		memset(&wbsf,0,sizeof(wbsf)); 
		wbsf.f01.f0.opcode=opcode;
		wbsf.f01.f0.fin=1;
		if(mask) 	wbsf.f01.f1.mask=1;

		int cbh=sizeof(wbsf.f01);
		int cbl=0;
		wbsf.len=cbw;
		wbsf.mask=mask;

		if(cbw<126) 
		{
			//cbh=sizeof(wbsf.f01);
			wbsf.f01.f1.len=cbw;
		}
		else
		{
			wbsf.f01.f1.len=126;
			cbl=2;
			if(cbw>=0x10000)
			{
				cbl=8;
				wbsf.f01.f1.len=127;
			}

			for(int n=0,nb=cbl-1;n<cbl;++n,--nb)
			{
				std::swap(wbsf.lenb[n],wbsf.inner_buf[nb]);
			}

			cbh+=cbl;

		}




		if(mask)
		{
			cbh+=4;
			uint32_t* pm=(uint32_t*)( wbsf.inner_buf+cbl);
			pm[0]=mask;
			//for(int n=0,nb=3;n<4;++n,--nb)		std::swap(pm[n],wbsf.maskb[nb]);


		}

		if(pwbsf) *pwbsf=wbsf;

		return cbh;
	}

	template <class Getter>
	bool get_header(Getter& sget)
	{
		clear_r();
		err_r=-1;
		wbs_frame_t::wbs_frame_t_01& f01= wbs_frame.f01;

		int cbt=sizeof(f01),cbr=cbt;
		while(cbr)
		{
			int cb=sget((char*)&f01,cbr,0);
			if(cb<=0) return false;
			cbr-=cb;
		}

		uint8_t lb=f01.f1.len;
		if(lb<0x7e)
		{
			uint64_t	ll=	uint64_t(lb);
			wbs_frame.len=ll;

		}
		else 
		{
			int cbl=(lb==0x7f)?8:2;
			if(cbl!=sget(((char*)&(wbs_frame.len=0))+8-cbl,cbl,0))
				return false;
			for(int n=0,nb=7;n<4;++n,--nb)
			{
				std::swap(wbs_frame.lenb[n],wbs_frame.lenb[nb]);
			}

		}

		if(f01.f1.mask)
		{
			if(4!=sget(&wbs_frame.mask,4,0))
				return false;
		}

		err_r=0;
		return true;

	}

	inline bool eom()
	{
		return wbs_frame.pos>=wbs_frame.len; 
	}

	inline int64_t tail()
	{
		return int64_t(wbs_frame.len)-int64_t(wbs_frame.pos);
	}


	template <class Getter>
	static int  recv(Getter& sget,void * buf,int cb)
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


	template <class Getter>
	int read(Getter& sget,void* buf,int cb )
	{
		if(err_r) return false; 
		int64_t cbt=tail();
		if(cb<cbt) cbt=cb;

		//int cbr=sget(buf,cbt,0);
		int cbr=recv(sget,buf,cbt);


		if((cbt)&&(cbr<=0)) err_r=-2;


		if(cbr<=0) return cbr;

		/*

		if(wbs_frame.mask) 
		{


		uint8_t* pb=(uint8_t* )buf;

		uint8_t ups=(wbs_frame.posb[0])&0x3;

		for(int n=0;n<cbr;++n)
		{
		pb[n]^=wbs_frame.maskb[ups];
		ups=(ups+1)&0x3;

		};
		}

		wbs_frame.pos+=uint64_t(cbr);
		*/

		wbs_frame.remask(buf,cbr);

		return cbr;

	}



	template <class Getter>
	bool begin_read(Getter& sget)
	{

		if((err_r==0)&&(wbs_frame.pos==0)) 
			return get_header(sget);
		return false;
	}

	enum
	{
		trush_size=4096
	};

	inline static char* trush()
	{
		static char _trush[trush_size];
		return _trush;
	}


	template <class Getter>
	bool end_read(Getter& sget)
	{


		if(err_r==0)
			//wbs_frame.mask=0;
			while(!eom()) 	
			{
				read(sget,trush(),trush_size);
			}
			wbs_frame.pos=0;
			return err_r==0;
	}



	template <class Setter>
	static int  send(Setter& sset,void * buf,int cb)
	{
		char *p=(char *)buf;
		int cbr=cb;
		while(cbr>0)
		{
			int c=sset(p,cbr);
			if(c<=0) return c;
			cbr-=c;
			p+=c;
		}
		return cb;
	}

	template <class Setter>
	//static 
	int write(Setter& sset,void * buf,int cb,int opcode=1,int mask=0)
	{
		const int bsz=256;
		char bufm[bsz], *ps=(char*)buf;
		wbs_frame_t wbf;
		int cbhw=set_header(&wbf,cb,opcode, mask);
		if(cbhw>sset(&wbf,cbhw)) return -1;
		int cbw=0,c;

		int nn=cb/bsz;
		for(int n=0;n<nn;++n)
		{
			void* p=(mask)?wbf.remask(bufm,ps,bsz):ps;

			c=send(sset,p,bsz); 
			if(c<=0) return c;
			ps+=c;
			cbw+=c;
		}
		if((c=cb-cbw)>0)
		{
			void* p=(mask)?wbf.remask(bufm,ps,c):ps;
			c=send(sset,p,c); 
			if(c<=0) return c;
			cbw+=c;
		}

		return cbw;
	}



	int err_r,err_w;
	wbs_frame_t wbs_frame;
	//wbs_frame_t wbs_frame_w,wbs_frame_wc;
	//v_buf_fix<uint8_t,bufsize> vbuf;



};





template <int DIG=__DIG,int RC=2>
struct recv_buffer_t
{

	typedef typename recv_buffer_t<DIG> self_t;

	enum
	{
		idig=DIG,
		isize=1<<idig,
		imask=isize-1,
		st_continue=2,
		st_message_end=4,
		st_error=1,
		region_capacity=RC

	};
	int flag;
	int state;
	int data_type,last_data_type;
	int actual_count;
	int pos;
	int begin_offset;
	int end_offset;
	long long totsize;
	bool fstarting;

	struct buf_item_t
	{
		buf_item_t()
		{

			memset(buf,'+',isize); buf[isize]=0;

		}

		buf_item_t(const buf_item_t& b)
		{
			memset(buf,'+',isize); c=0;
		}
		void operator=(const buf_item_t& b)
		{

		}
		char buf[isize],c;
		/*
		union
		{
		struct {char buf[isize];};
		struct {char __b[isize+1];};
		};
		*/		

	};

	//
	//typedef typename std::list<buf_item_t,aligned_allocator_t<buf_item_t,idig>> list_t;
	typedef typename std::list<buf_item_t> list_t;
	typedef typename list_t::iterator region_iterator;

	//typedef  std::list<buf_item_t> list_t;


	struct region_t
	{
		int b,e;
		region_iterator i0;
		region_iterator i1;

		region_t():b(0),e(0){};

		inline bool is_multi()
		{
			return (i0!=i1);
		}

		inline bool is_empty()
		{
			return (i0==i1)&&(b==e);
		}
		inline operator bool()
		{
			return !is_empty(); 
		}

		template <class BUF>
		inline int  prepare(BUF* pbuf)
		{
			if(is_empty()) 
				return 0;

			if(i0==i1)
			{
				pbuf[0].len=e-b;
				pbuf[0].buf=i0->buf+b;
				return 1;
			}
			else
			{
				pbuf[0].len=isize-b;
				pbuf[0].buf=i0->buf+b;
				pbuf[1].len=isize;
				pbuf[1].buf=i1->buf;
				return 2;

			}
		}


		inline int fullsize()
		{
			region_iterator i=i0;
			int sz;
			if(i==i1) sz= e-b;
			else
			{
				sz=isize-b;
				while((++i)!=i1) 
					sz+=isize;
				sz+=e;
			}
			return sz;
		}


	};

	region_t tail_region;
	region_t recv_region;
	region_t read_region;



	//list_t::iterator ip;
	//list_t::iterator ipacket_begin;
	//list_t::iterator ipacket_end;


	list_t itemlist;

	recv_buffer_t():state(st_error),fstarting(false)
	{
		//itemlist.push_back(buf_item_t());
		list_t::iterator i =itemlist.insert(itemlist.begin(),buf_item_t());
		bool ff=i==itemlist.begin();
		tail_region.i0=itemlist.end();
		tail_region.i1=itemlist.end();
		reset_recv_region();
		begin_offset=end_offset=0;
		data_type=last_data_type=0;
		totsize=0;

	}

	inline int  recv_region_start(WSABUF* pbuf=0)
	{
		WSABUF tmp[2];
		if(!make_detach(fstarting,true)) 
			reset_recv_region();
		return recv_region.prepare( (pbuf)?pbuf:tmp);

	}
	inline int  recv_region_next(WSABUF* pbuf)
	{
		fstarting=false;
		if(state==st_continue)
			return recv_region.prepare(pbuf);
		else return 0;
	}




	inline void reset_recv_region()
	{


		bool ftail=tail_region;

		if(ftail)
		{
			list_t tmp;

			region_iterator i0=tail_region.i0;   
			region_iterator i1=tail_region.i1;   
			bool ff=(i0!=i1);
			++i1;
			tmp.splice(tmp.begin(),itemlist,i0,i1);
			tmp.swap(itemlist);
			region_iterator i=itemlist.begin();
			tail_region.i0=i;			
			tail_region.i1=(ff)?++i:i;

		}
		else itemlist.clear();

		/*
		int ls=itemlist.size();	
		for(int i=ls;i<2;++i) 
		itemlist.push_back(buf_item_t());
		*/
		expand();




		if(ftail)
		{




			recv_region.b=tail_region.e;




			region_iterator i=tail_region.i1;
			recv_region.i0=i;
			++i;
			recv_region.i1=i;

			recv_region.e=isize;


			int b=tail_region.b;

			read_region.e=read_region.b=b;
			read_region.i1=read_region.i0=tail_region.i0;


		}
		else
		{
			region_iterator ib=itemlist.begin();

			recv_region.i0=recv_region.i1=ib;
			recv_region.b=0;
			recv_region.e=isize;

			int b=recv_region.b;
			read_region.e=read_region.b=b;
			read_region.i1=read_region.i0=recv_region.i0;

		}


	}


	inline	int read_data_type(region_t& region)
	{
		int dt=0;
		if(region.i0->buf[region.b]==0x00)
		{
			dt=1;
			begin_offset=1;
			end_offset=-1;
		}
		return dt;
	}

	inline	bool find_wbs_string_end0( int byteread,int& bytetail)
	{
		bool f=false;
		unsigned char *p=(unsigned char*)recv_region.i0->buf;
		int br0=isize-recv_region.b;
		bool f2=(br0<byteread);
		int e0=f2?isize:byteread;

		int n=recv_region.b;
		int n2=byteread-br0;

		while(n<e0)
		{
			if(p[n++]==0xFF)
			{
				f=true;
				break;
			}
		}
		bytetail=byteread;
		if(f)
		{

			if(n<isize)
			{
				tail_region.b=n;
				tail_region.i0=recv_region.i0; 
				tail_region.i1=recv_region.i1; 
				tail_region.e=byteread-br0;							
			}
			else
			{
				tail_region.b=0;
				tail_region.e=n2;
				tail_region.i1=tail_region.i0=recv_region.i1;
			}
			read_region.e=n;
			read_region.i1=tail_region.i0;
			bytetail=n;


		}
		else 
		{
			if(recv_region.is_multi())
			{

				p=(unsigned char*)recv_region.i1->buf;
				int m=0;
				while(m<n2)
				{
					if(p[m++]==0xFF)
					{
						f=true;
						break;
					}
				}



				if(f)
				{
					region_iterator i=recv_region.i1;
					tail_region.b=m;
					tail_region.e=n2;
					tail_region.i1=tail_region.i0=i; 

					read_region.e=m;
					read_region.i1=i;

					bytetail=br0+m;
				}

			}


			if(f)
			{

				if(n2>0)
				{
					region_iterator i=recv_region.i1;

					//recv_region.b=recv_region.e;
					recv_region.b=n2;
					recv_region.e=isize;
					recv_region.i0=i;
					//itemlist.insert(++i,1,buf_item_t());
					itemlist.push_back(buf_item_t());
					recv_region.i1=++i;

				}
				else
				{ 
					if(recv_region.i1==recv_region.i0)
					{
						itemlist.push_back(buf_item_t());
						++recv_region.i1;
					}
					if(n2<0)   
					{
						//recv_region.b=recv_region.e;
						recv_region.b+=byteread;

					}
					else
					{
						recv_region.b=0;
						recv_region.i0=recv_region.i1;
					}
					recv_region.e=isize;

				}


			}



		}

		//if(f) return true;




		return f;

	}


	int on_complete(int byteread)
	{



		if(data_type==0)
		{
			if(tail_region) 
				data_type=read_data_type(tail_region);
			else 
				data_type=read_data_type(recv_region);
			if(data_type) state=st_continue;
			else 		state=st_error;

		}


		if(state==st_continue)
		{

			recv_region=resize_region(recv_region,byteread);

			if(data_type==1)
			{
				int bytetail=0;
				bool fs=find_wbs_string_end( byteread,bytetail);
				if(fs)
				{
					last_data_type=make_detach(data_type,0);
					state=st_message_end;
				}
				totsize+= bytetail;
			}

		}

		return state;
	}


	int get_next_read_buf(char** pp)
	{
		int sz;
		region_iterator i=read_region.i0;
		int b=read_region.b;
		if(read_region.i0==read_region.i1)
		{
			sz=read_region.e-read_region.b;
			read_region.b=read_region.e;

		}
		else
		{
			sz=isize-read_region.b;
			read_region.b=0;
			++read_region.i0;
		}

		if(sz)

			*pp=i->buf+b;

		return sz;
	}


	//read_data_type(
	int string_end_pos(region_iterator i,int b,int e) 
	{
		for(int n=b;n<e;++n)
		{
			unsigned char c= ((unsigned char*)i->buf)[n];
			if(c==0xFF) return n+1;

		}
		return 0;
	}

	int check_end_and_set(region_t& region,region_t& regread,int dt)
	{
		if(region.is_empty()) 	return false;


		if(dt==1)
		{


			int e=-1;
			if(region.is_multi())
			{
				if((e=string_end_pos(region.i0,region.b,isize))>0)
				{
					regread.b=region.b;
					regread.e=e;
					regread.i0=regread.i1=region.i0;
					region.b=e;
				}
				else if((e=string_end_pos(region.i1,0,region.e))>0)
				{
					region_iterator i=region.i0;
					regread.b=region.b;
					regread.e=e;
					regread.i0=i;
					regread.i1=region.i1;

					region.i0=region.i1;
					region.b=e;			  
				}


			}
			else if((e=string_end_pos(region.i0,region.b,region.e))>0)
			{
				regread.b=region.b;
				regread.e=e;
				regread.i0=regread.i1=region.i0;
				region.b=e;
			}

			return e>0; 
		}

		return 0;

	}


	inline region_t  resize_region(region_t r,int byteread)
	{
		if(r.is_multi())
		{
			int nn=byteread-(isize-r.b);
			if(nn>0) r.e=nn;
			else
		 {
			 r.e=r.b+byteread;
			 r.i1=r.i0;
		 }

		}
		else
		{
			r.e=r.b+byteread;
		}
		return r;
	}

	inline region_iterator expand()
	{
		return expand(itemlist.begin());
	}
	inline region_iterator expand(region_iterator  i)
	{
		if(i==itemlist.end()) 
			i=itemlist.insert(i,buf_item_t());
		if((++i)!=itemlist.end()) 
			return i;
		else return itemlist.insert(i,buf_item_t());
		//itemlist.push_back(buf_item_t());
		//return itemlist.back();




		/*
		region_iterator r;
		++(r=i);
		if(r==itemlist.end())
		{
		itemlist.push_back(buf_item_t());
		++(r=i);
		}
		return r;
		*/
	}

	inline	bool find_wbs_string_end( int byteread,int& bytetail)
	{
		bool f,fm0,fm=false;
		region_t r= recv_region;

		region_iterator iend;
		bytetail=byteread;



		fm0=recv_region.is_multi();
		int e;

		if(fm0)
		{
			if((e=string_end_pos(r.i0,r.b,isize))>0)
			{
				r.i1=r.i0;
				r.e=e;
				bytetail=e-r.b;
			}
			else
			{
				if((e=string_end_pos(r.i1,0,r.e))>0)
				{
					fm=true;
					r.e=e;
					bytetail=(isize-r.b)+e;

				}
			}


		}
		else
		{
			if((e=string_end_pos(r.i0,r.b,r.e))>0)
			{
				r.e=e; 
				bytetail=e-r.b;
			}

		}

		f=(e>0);

		if(f)
		{
			region_iterator ib=itemlist.begin();
			iend=r.i1;

			read_region.e=e;
			read_region.i1=iend;

			tail_region.b=e;
			tail_region.e=recv_region.e;


			tail_region.i0=iend;
			tail_region.i1=recv_region.i1;

			/*	  
			if(fm0)
			{
			}
			*/	  


		}
		else
		{
			region_iterator i;
			r=recv_region;

			recv_region.b=r.e;
			recv_region.e=isize;

			i=r.i1;
			recv_region.i0=i;
			recv_region.i1=expand(i);

		}


		return f;

	}


	bool read_region_from_tail()
	{
		bool f;
		f=(last_data_type=read_data_type(tail_region))!=0;
		if(f)
			f= check_end_and_set(tail_region,read_region,last_data_type);
		return f;
	}

};


template <class Getter,int DIG=__DIG>
struct wbs_string_loader_t
{

	typedef recv_buffer_t<DIG> buffer_t;
	typedef Getter   getter_t;
	getter_t& sget;
	int count;
	char *ptail;
	int cbtail;
	bool feof,fstart;
	int freadmode;

	buffer_t recv_buffer;

	enum
	{
		idig=buffer_t::idig,
		isize=buffer_t::isize,
		imask=buffer_t::imask,
		st_continue=buffer_t::st_continue,
		st_message_end=buffer_t::st_message_end,
		st_error=buffer_t::st_error,
		region_capacity=buffer_t::region_capacity
	};

	inline int length()
	{
		return count-2;
	}

	wbs_string_loader_t( getter_t& _sget, int readmode=0 )
		:sget(_sget),count(0),feof(0),freadmode(readmode){};

	int recv()
	{
		feof=0;
		count=0;
		cbtail=0;
		int cb;
		int state;
		WSABUF buf[2];
		int nbuf;
		nbuf=recv_buffer.recv_region_start(buf);
		if(!nbuf) return -1;

		for(;;) {
			cb=sget(nbuf,buf);
			if(cb<=0) return cb;

			state=recv_buffer.on_complete(cb);
			if(state==st_continue)
				nbuf=recv_buffer.recv_region_next(buf);
			else if(state==st_message_end)
		 {
			 fstart=true;
			 return   count=recv_buffer.read_region.fullsize();
		 }
			else return -1;

		}  
	}

	inline  bool eof()
	{
		return recv_buffer.read_region.is_empty()&&(cbtail==0);
	}


	inline int _wberaser(int cn,char** pp)
	{

		if(freadmode&&(cn>0)){
			if(make_detach(fstart,false))
			{
				++(*pp);
				--cn;
			}
			if(recv_buffer.read_region.is_empty()) --cn;
			if(cn<0) cn=0;
		}
		return cn;
	}

	inline int read(char* buf,int cb)
	{
		if(cb<=0) return cb;
		int c=0;


		if(cbtail)
		{
			//int cm=(cb>cbtail)?cbtail:cb;
			if(cb<cbtail)
			{
				memcpy(buf,ptail,cb);
				ptail+=cb;
				cbtail-=cb;
				return cb;
			}
			else
			{
				memcpy(buf,ptail,cbtail);
				buf+=cbtail;
				cb-=cbtail;
				c+=cbtail;
				cbtail=0;	
			}

			//int c=()
			//memcpy(buf,)
		}

		char* p;

		while(cb>0)
		{

			/*
			int cn=recv_buffer.get_next_read_buf(&p);

			if(cn==0) {
			cbtail=0; break;
		 }


		 cn=_wberaser(cn,&p);



		 if(freadmode){
		 if(make_detach(fstart,false))
		 {
		 ++p;
		 --cn;
		 }
		 if(recv_buffer.read_region.is_empty()) --cn;

		 }

		 */		  
			int cn=get_next_read_buf(&p);

			if(cn>cb){
				cbtail=cn-cb;
				ptail=p+cb;
				memcpy(buf,p,cb);
				c+=cb;
				return c;
		 }
			else
		 {
			 cbtail=0;	
			 if(cn<=0) 
				 break;					 
			 memcpy(buf,p,cn);
			 c+=cn;
			 cb-=cn;
			 buf+=cn;

		 }


		}

		return c;

	}

	inline int get_next_read_buf(char** pp)
	{
		int cn=recv_buffer.get_next_read_buf(pp);
		return cn=_wberaser(cn,pp);
	}

	template<class Stream> 
	inline	Stream& operator>>(Stream& i)
	{
		int c;
		char* p;

		while((c=get_next_read_buf(&p)))
			i.write(p,c);
		return i;
	}


};

/*

template<class Stream,class Loader> 
inline	Stream& operator<<(Stream& i, Loader& loader)
{

int c;
char* p;

//while((c=loader.get_next_read_buf(&p)))
i.read(p,c);
return i;
};
*/

#pragma pack(pop)
