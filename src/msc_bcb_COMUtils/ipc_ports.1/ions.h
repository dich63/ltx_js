#pragma once
#include "hs_socket.h"
#include "video\singleton_utils.h"
#include "ipc_utils.h"
#include "algorithms\cycle.h"
#include <set>
#include <string>

#define BUFDEFSZ (2*4096)
//(2*4096)

template <int BufSize=BUFDEFSZ,class Mutex=mutex_cs_t>
struct ions_t:_non_copyable_
{
   typedef Mutex mutex_t;
    typedef  console_shared_data_t<wchar_t,0> console_t;
    static console_t& console()
	{
         static  console_t sc;
			 return sc;
	}
//template <class Mutex=mutex_cs_t>
   struct iunknow_fakeimpl_t:IUnknown
   {
	   LONG refcount;
	   virtual HRESULT __stdcall QueryInterface(REFIID riid,void** pp){	return E_NOINTERFACE;};
	   virtual ULONG STDMETHODCALLTYPE AddRef()
	   { 
		   return InterlockedIncrement(&refcount);
	   };
	   virtual ULONG STDMETHODCALLTYPE Release()
	   {
		   LONG l=InterlockedDecrement(&refcount);
		   if(l==0) 
			   delete this;
		   return l;
	   };
	   iunknow_fakeimpl_t():refcount(1){};
	   virtual ~iunknow_fakeimpl_t(){};
   };




   template <class T, int BufSize=2*4096>
   struct io_overlapped2_t
   {


	   enum
	   {
		   bufsize=BufSize
	   };

	   char buf[bufsize];

	   struct io_overlapped_t:OVERLAPPED
	   {
		   io_overlapped_t(T* p,bool t)
		   {
			   memset(this,0,sizeof(io_overlapped_t));
			   powner=p;
			   fr=t;
		   }
		   T* powner;
		   union
		   {
			   HANDLE hf;
			   SOCKET s;
		   };
		   HRESULT hr;
		   bool fr;


	   };
	   io_overlapped_t ovl_r,ovl_s;
	   int rp,ro,so,rc,sc,sp;

	   io_overlapped2_t():ovl_r(static_cast<T*>(this),1),ovl_s(static_cast<T*>(this),0)
	   { 
		   rp=ro=so=rc=sc=sp=0;
		   memset(buf,0x11,bufsize); buf[bufsize-1]=0;
	   };

   };


struct annihilator_t
{

    v_buf<char> portname;	
	 
	struct socket_pair_t;
	struct less_socket_pair_ptr_t;


	
	typedef typename ipc_utils::smart_ptr_t<socket_pair_t> socket_pair_ptr_t;
	typedef std::multiset<socket_pair_ptr_t,less_socket_pair_ptr_t> connection_set_t;
	typedef typename connection_set_t::iterator iconnection;



	struct socket_pair_t:iunknow_fakeimpl_t {
		  
       inline bool less(const socket_pair_t* p)
	   {
           int q=charge-p->charge;
		   if(q==0)
		   {
			   return name<p->name;
		   }
		   return q<0;


	   }
/*
	   inline bool operator < (const socket_pair_t* i2)
	   {
		   return less(i2);
	   }
*/
	   socket_pair_t(annihilator_t* o,char* channel,int q,SOCKET s,int _vws):vws(_vws),owner(o),name(channel),charge(q),s0(s),s1(INVALID_SOCKET),hr(0),fcl(0),hro(0),hevent(0)
	   {
		   
		   i_self=o->connections.end();
	   };

	   ~socket_pair_t()
	   {
		   //safe_cputs("~socket_pair_t\n");
		   console().log("~socket_pair_t\n",0xA0);
		   closesocket(s0);
		   closesocket(s1);
	   }

	   void onclose()
	   {
		    UnregisterWait(hro);
		   if(detach())    owner->unregister_socket(i_self);

	   }

       bool detach()
	   {
		   if(InterlockedCompareExchange((LONG volatile*)&fcl,1,0)) return false; 
		   
		   WSAEventSelect(s0,0,0);
		   if(hevent)
		   {
			   SetEvent(hevent);
			   CloseHandle(hevent);
		   }
		   return true;
	   }

       HRESULT attach(DWORD tio=INFINITE)
	   {
	     hr=0;
        hevent=WSACreateEvent();
	    if(SOCKET_ERROR==WSAEventSelect(s0,hevent,FD_CLOSE))
		   hr=WSAGetLastError();
	     socket_pair_ptr_t sp(this);
	     if(!hr)
			 if(RegisterWaitForSingleObject(&hro,hevent,WAITORTIMERCALLBACK(&s_onclose),this,tio,WT_EXECUTEONLYONCE))
			   sp.detach();
			   else hr=WSAGetLastError();
		 return hr;
	   }
	   
	   static   void __stdcall s_onclose(socket_pair_t* p,BOOLEAN TimerOrWaitFired)
	   {

           socket_pair_ptr_t sp(p,false);
            sp->onclose();
		   
	   };

	   std::string name;
	   int charge;
	   SOCKET s0,s1;
	   HRESULT hr;
	   LONG fcl;
	   HANDLE hevent;
	   HANDLE hro;
	   iconnection i_self;
	   int vws;

	   annihilator_t* owner;


	};


	struct less_socket_pair_ptr_t: std::binary_function<socket_pair_ptr_t,socket_pair_ptr_t, bool>
	{	// functor for operator<
		bool operator()(const socket_pair_ptr_t& _Left, const socket_pair_ptr_t& _Right) const
		{	// apply operator< to operands
			return _Left.p->less(_Right.p);
		}
	};



	   
	   struct io_c2_t:iunknow_fakeimpl_t,io_overlapped2_t<io_c2_t,BufSize>
	   {
               
		   struct io_c_t:io_overlapped_t
		   {
			   io_c_t(io_c2_t* p,bool fr):io_overlapped_t(p,fr),io(p){};

			   ipc_utils::smart_ptr_t<io_c2_t> io;
		   };

		   io_c2_t(SOCKET s1,SOCKET s2):cycle(BufSize)
		   {
			   ovl_r.s=s1;
			   ovl_s.s=s2;

		   }    

		   io_c2_t(bool f,socket_pair_t* psockets):cycle(BufSize)
		   {
               sockets=psockets;
			   ovl_r.s=(f)?psockets->s0:psockets->s1;
			   ovl_s.s=(f)?psockets->s1:psockets->s0;
			   start();
		   }    



		   ~io_c2_t()
		   {
			   
			   
			   //safe_cputs(v_buf<wchar_t>().printf(L"~io_c2_t:\nrecv_err=[%08x]%s \nsend_err=[%08x]%s\n",ovl_r.hr,(wchar_t*)error_msg(ovl_r.hr),ovl_s.hr,(wchar_t*)error_msg(ovl_s.hr)));
			   console().log(v_buf<wchar_t>().printf(L"~io_c2_t:\nrecv_err=[%08x]%s \nsend_err=[%08x]%s\n",ovl_r.hr,(wchar_t*)error_msg(ovl_r.hr),ovl_s.hr,(wchar_t*)error_msg(ovl_s.hr)).get(),M_BC|0x0f);
			   //closesocket(ovl_r.s);
			   //closesocket(ovl_s.s);
			     shutdown(ovl_r.s,SD_BOTH );
				 shutdown(ovl_s.s,SD_BOTH );
		   }

	     cycle_buffer_t<mutex_cs_t> cycle;


        void  dump(int n,WSABUF* buf)
		 {
		    v_buf<char> bb;
			for(int i=0;i<n;i++)
			{
				bb.resize(buf[i].len+1);
				for(int k=0;k<buf[i].len;k++)
				{
					bb[k]=buf[i].buf[k];
					if (bb[k]==0)
						bb[k]='<';
					if (bb[k]==char(-1)) 
						bb[k]='>';

				}
					
					bb[buf[i].len]=0;

				safe_cputs(bb.get());
			}
		 }


		inline static HRESULT check_graceful_disconnect(HRESULT& hr,int cb)
		{
			return ((hr==S_OK)&&(cb<=0))?ERROR_GRACEFUL_DISCONNECT:hr;
		}


       void  io_completition(HRESULT hr,ULONG_PTR cb_op,io_overlapped_t* povl)
	   {
		   int cbr;

		   if(povl->fr)
		   {
			   ++rc;
		   			   cbr=recv_complete(cb_op);
					   send_op();
					   //  recv_op();
					   
		   }
		   else 
		   {
			   ++sc;
			   

			   cbr=send_complete(cb_op);

			   if(!recv_op())
				      send_op();
			   //if (cycle.cocursor.distance()==0)    recv_op();
			   //else	     send_op();
		   }

		//   SetConsoleTitleA(v_buf<char>().printf(" rp=%d ro=%d rc=%d ;sp=%d so=%d sc=%d ",rp,ro,rc,sp,so,sc));


            
			

	   }
         
		static void  __stdcall s_io_completition(HRESULT hr,ULONG_PTR cb_op,io_overlapped_t* povl)
		{
			//if(hr==0) 
			//io_completition(hr, bytetransfer,povl);

			
			if(povl)
			{

		          ipc_utils::smart_ptr_t<io_c2_t> io(povl->powner,0);
				  //if(check_graceful_disconnect(hr,cb_op))	  {         povl->hr=hr;
				    //  //return;
			        
                   if(!povl->hr) povl->hr=check_graceful_disconnect(hr,cb_op);
					   
				  io->io_completition(hr,cb_op,povl);

/*
              if(povl->fr)
			  {
                   cbr=recv_complete(cb_op);
			  }
*/

              //delete povl;
			}
		}



/*
		   HRESULT recv_op()
		   {
			   HRESULT hh;
			   if(ovl_r.hr==0)
			   {
  		   
    		   WSABUF wbufs[2];
               int nbuf=cycle.get_recv_buf(buf,wbufs);
			   DWORD flag=0,cbtot=0;
			   ovl_r.InternalHigh=0;
			   int res=WSARecv(ovl_r.s,wbufs,nbuf,(DWORD*)&cbtot,&flag,&ovl_r,0);


			     if(res!=0) {

				   hh=GetLastError();
				   if(hh!=WSA_IO_PENDING)
					   ovl_r.hr=hh;
				   return hh;
			   }
			   
			   }
			   return ovl_r.hr;
  		   }

		   //if(ovl_r.hr==WSA_IO_PENDING)
		   
           HRESULT  recv_complete()
		   {
			   WSABUF wbufs[2];
			   HRESULT hh;
			   int nbuf;

			   DWORD cbtot=ovl_r.InternalHigh;
     
			   if(ovl_s.hr==0) 
			   {
				
				   nbuf=cycle.get_recv_buf(buf,wbufs,cbtot);
				   //safe_cputs( v_buf<char>().printf("[%d]",cbtot));
				   //dump(nbuf,wbufs);
 			      cycle.recv_complete(cbtot);
				  //
                  

				  nbuf=cycle.get_send_buf(buf,wbufs);
				  dump(nbuf,wbufs);
				  ovl_s.InternalHigh=0;
                 int res=WSASend(ovl_s.s,wbufs,nbuf,(DWORD*)&cbtot,0,&ovl_s,0);
				 //dump(nbuf,wbufs);
                     
				 if(res==0) ;//cycle.send_complete(cbtot);
				 else
				  {
   
				   	   hh=GetLastError();
					   if(hh!=WSA_IO_PENDING)
						   ovl_s.hr=hh;
                       return hh;
				 }
				   
			   }
     		   return ovl_s.hr;  
		   }

		   HRESULT  send_complete()
		   {
			   DWORD cbtot=ovl_s.InternalHigh;
               cycle.send_complete(cbtot);
			   return  recv_op();
		   
		   }

   
   static void  __stdcall io_completition(HRESULT hr,ULONG_PTR bytetransfer,io_overlapped_t* povl)
		{

			int kljhjh;
			bool repeat;
	if(povl)
	{
		ipc_utils::smart_ptr_t<io_c2_t> io(povl->powner,0);

		do{
           repeat=0;
           
             if(povl->fr)
			 {
				 if(hr==0) {

                	hr=io->recv_complete(); 

					if(hr==0) 
					{
						//repeat=1;
						//povl=&(io->ovl_s);
						io.detach();
					} 
					else if(hr==WSA_IO_PENDING) io.detach();

				 }

			 }
			 else
			 {
				 if(hr==0)
				 {
					 hr=io->send_complete(); 
					 //if(hr==0) hr=io->recv_op();
					 

					 if(hr==0)
					 {
						 //repeat=1;
						 //povl=&(io->ovl_r);
						 io.detach();
					 }
					 else if(hr==WSA_IO_PENDING) io.detach();

				 }

			 }

		   

    		 }while(repeat); 

	   }

		}
		*/

    inline int check_op(int res ,io_overlapped_t*ovl)
	{
		HRESULT hh;
        if(res)
		{
			if((hh=GetLastError())!=WSA_IO_PENDING)
			{
				ovl->hr=hh;
				return 0;
			}
			else return WSA_IO_PENDING;

		}
		return 1;
	}

	int   recv_complete(int cbtot)
	{
		int cb,d,ds;
		if(ovl_r.hr==0)    
		{
			/*
			WSABUF wbufs[2];
			int nbuf=cycle.get_recv_buf(buf,wbufs,cbtot);
			safe_cputs( v_buf<char>().printf("[%d]",cbtot));
			*/
			//dump(nbuf,wbufs);

			cb= cycle.recv_complete(cbtot);
			d=cycle.cursor.distance();
			ds=cycle.cocursor.distance();
			return cb;
		}
		else return 0;
	}

	int   send_complete(int cbtot)
	{
		int cb,ds,d,dss;
		if(ovl_s.hr==0)  
		{
			//safe_cputs( v_buf<char>().printf("{%d}",cbtot));
			/*
			WSABUF wbufs[2];
			int nbuf=cycle.get_send_buf(buf,wbufs,cbtot);
			
			dump(nbuf,wbufs);
			*/

			dss=cycle.cocursor.distance();
			cb=cycle.send_complete(cbtot);
			d=cycle.cursor.distance();
			ds=cycle.cocursor.distance();

			return cb;
		}
		else return 0;
	}

	bool send_op()
	{
		WSABUF wbufs[2];
		int  nbuf,d;
		DWORD cbtot=0;

		if(ovl_s.hr==0)
		{
		
			 d=cycle.cocursor.distance();
			 if(d==0) return false;
			 nbuf=cycle.get_send_buf(buf,wbufs);
			 if(nbuf<=0) return false;
			 //d=cycle.cocursor.distance();
              //ovl_s.InternalHigh=0;
			 //io_c_t * povl_s=new io_c_t(this,0);
             //int res=WSASend(ovl_s.s,wbufs,nbuf,&cbtot,0,povl_s,0);
			 int res=WSASend(ovl_s.s,wbufs,nbuf,&cbtot,0,&ovl_s,0);

			 bool f=check_op(res,&ovl_s);

			 if(f)
			 {
				 so++;
				 if(WSA_IO_PENDING==f)
					 ++sp;
				 AddRef();
			 }
			 //else delete povl_s;

			 return f;
		}
		return false;


	}



	bool recv_op()
	{
		//HRESULT hh;
		WSABUF wbufs[2];
		int  nbuf,d;

		if(ovl_r.hr==0)
		{


			d=cycle.cursor.distance();
			if(d==0) return false;
			nbuf=cycle.get_recv_buf(buf,wbufs);
			d=cycle.cursor.distance();



			DWORD flag=0,cbtot=0;
			//ovl_r.InternalHigh=0;


			//io_c_t * povl_r=new io_c_t(this,1);

			int res=WSARecv(ovl_r.s,wbufs,nbuf,(DWORD*)&cbtot,&flag,&ovl_r,0);

			int f=check_op(res,&ovl_r);

			if(f)
			{
				++ro;
				if(WSA_IO_PENDING==f)
					++rp;
				AddRef();
			}
			//else delete povl_r;

			return f;
		}
		return false;

	}





	    void start()
		{
			/*
			HRESULT hh=recv_op();
			if(hh==WSA_IO_PENDING) AddRef();
			else    
				if(hh==0) AddRef();
					//io_completition(0,&ovl_r);
					*/
			recv_op() ;
				
		}

               
         socket_pair_ptr_t sockets;

	   };

	   
       
	   struct asyn_channel_t
	   {
		   socket_pair_ptr_t sp;
		   SOCKET s0,s1;
		   asyn_channel_t(bool f,socket_pair_t* psp):sp(psp)
		   {
			   s0=(f)?sp->s0:sp->s1;
			   s1=(f)?sp->s1:sp->s0;
		   }
		   HRESULT operator()()
		   {
			 HRESULT hr=hss_wbs_frame_from_to((void*)s0,(void*)s1,BufSize);
			 //shutdown(s1,SD_RECEIVE);
			 //shutdown(s2,SD_SEND);
			  shutdown(s0,SD_BOTH);
			  shutdown(s1,SD_BOTH);
			    console().log(v_buf<wchar_t>().printf(L"asyn_channel:\n exit_code=[%08x]%s\n",hr,(wchar_t*)error_msg(hr)).get(),M_BC|0x0f);
			 return hr;
		   }

	   };


 bool register_channel(char* channel,int q,SOCKET s,int vws,int tio=INFINITE)
 {
	 if(!channel) return 0;
	 int vws0;
	 socket_pair_ptr_t sp(new socket_pair_t(this,channel,-q,s,vws),0);

	 {

	 
	 locker_t<mutex_t> lock(mutex);
	  iconnection i=connections.find(sp);

	  if(i==connections.end())
	  {
          sp->charge=q;

		  register_socket(sp,tio);    
		  console().printf(M_BC|0x0f,"[%s] register channel=%s valence=%d\n",portname.get(),channel,q);
         return true;
  	  }
	  //(*i)->s1=make_detach(sp->s0,INVALID_SOCKET);
	  (*i)->detach();
	  vws0=(*i)->vws;
	  sp->s1=make_detach((*i)->s0,INVALID_SOCKET);
	  
	  connections.erase(i);
	 // if(vws0!=vws) return false;
	 }

	 if(vws>12)
	 {
        asyn_call(new asyn_channel_t(0,sp));
		asyn_call(new asyn_channel_t(1,sp));
		return true;
	 }

BOOL ff1,ff2,ff3,ff4;
	 ff1=BindIoCompletionCallback(HANDLE(sp->s0),LPOVERLAPPED_COMPLETION_ROUTINE(&io_c2_t::s_io_completition),0);   
	 ff2=BindIoCompletionCallback(HANDLE(sp->s1),LPOVERLAPPED_COMPLETION_ROUTINE(&io_c2_t::s_io_completition),0);   
/*
	 HRESULT hrr;
     ff3=SetFileCompletionNotificationModes(HANDLE(sp->s0),0x1);
		 hrr=GetLastError();
     ff4=SetFileCompletionNotificationModes(HANDLE(sp->s0),0x0);
		 hrr=GetLastError();

       ff1=BindIoCompletionCallback(HANDLE(sp->s0),LPOVERLAPPED_COMPLETION_ROUTINE(&io_c2_t::s_io_completition),0);   
	   hrr=GetLastError();
	   */

	 ipc_utils::smart_ptr_t<io_c2_t> io01( new io_c2_t(0,sp),0);
     ipc_utils::smart_ptr_t<io_c2_t> io10( new io_c2_t(1,sp),0);
	 console().printf(M_BC|0x0f,"[%s] annihilate channels=%s valence=%d\n",portname.get(),channel,q);
	 return true; 
	 
 }
  


  bool register_socket(socket_pair_t* p,int tio=-1)
	  {
		  locker_t<mutex_t> lock(mutex);
		  
		   p->i_self=connections.insert(p);
		   bool f=p->attach(tio);

		  return f;	
	  }

  bool unregister_socket(iconnection i)
	  {
		  locker_t<mutex_t> lock(mutex);
		  return connections.end()!=connections.erase(i);
	  };

 
	  
	  
 socket_pair_t* ppp;


 mutex_t mutex;
connection_set_t connections;

	//std::multiset<>


} annihilator;


void* hport;

void  on_accept(HSS_INFO* ovl)
{
	UINT st=hss_websocket_handshake_reply(ovl);
	//bool fwbs=st&(1<<HSD_REPLY);
	int vws=hss_wbs_version(ovl);
	char* paramszz=hss_reset_SOCKET_DATA(ovl,HSD_PARAMS);
	argv_zz<char> args(paramszz);
	int q=args["valence"];
	char* pchannel=args["channel"];
	int tio=args["tio"].def<int>(-1);

	SOCKET s=ovl->s;
	bool ff=annihilator.register_channel(pchannel,q,s,vws,tio);
	if(ff) make_detach(ovl->s,INVALID_SOCKET);    
}

static void  s_on_accept(HSS_INFO* ovl)
{
    ((ions_t*)ovl->user_data)->on_accept(ovl);
}
  
inline operator bool(){ return hport;}

inline ions_t* open(wchar_t *nsport)
{
   annihilator.portname.clear()<< char_mutator<CP_UTF8>(nsport);
   hport=hss_create_socket_portW(nsport,1,&s_on_accept,(void*)&annihilator);
   DWORD attr=hport?0xf1:0xcf; 
   console().log(v_buf<wchar_t>().printf(L"register ions pair port=%s\n",nsport).get(),attr);
  return this;
}

/*
template <class Args>
  ions_t(Args& args)
  {
     wchar_t *nsport=args[L"ion_port"];

	 open(nsport);
  }
*/
~ions_t()
{
	hss_release(hport);
}
inline int  wait()
{
	return hss_wait_event(hport);
}


};