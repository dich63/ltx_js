#pragma once
#include "hssh.h"
#include "socket_utils.h"
#include "shared_closures.h"
#include <list>
#include <vector>





struct hss_packets_t
{
	enum
	{
		t_clear=HTPL_CLEAR,
		t_file=HTPL_FILE,
		t_memory=HTPL_MEMORY,
		t_region=HTPL_REGION,
		t_eof=HTPL_EOF,
		t_pool=HTPL_POOL
	};

 struct ioctrl_t
 {
	 LPFN_TRANSMITPACKETS TransmitPackets; 
	 
 ioctrl_t(SOCKET s):TransmitPackets(0)
   {

	   GUID wiid=WSAID_TRANSMITPACKETS;
	   DWORD cbr,err;
	   err=WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,&wiid,sizeof(wiid),&TransmitPackets,sizeof(TransmitPackets),&cbr,0,0);
   }
  operator bool()
  {
	  return TransmitPackets!=0;
  }
 };

struct tp_t
{
  int t;
  WSABUF region;
  long long offset;
  
};


 typedef hs_socket_utils::locker_t<hs_socket_utils::CS> locker_t;
 typedef  std::list<std::pair<int,void*> > list_t;
 typedef  std::list<HANDLE> handle_list_t;

 /*
 OVERLAPPED ovl;
 HANDLE hrw;
 //ptr_holder<char> proc_h;
 HSS_INFO* hssinfo; 
 hss_proc_t proc;
 */
int ffilllock;
struct transaction_t
{
	OVERLAPPED ovl;
	HANDLE hrw;
	//ptr_holder<char> proc_h;
	HSS_INFO* hssinfo; 
	void* proc;
	hss_packets_t* ppacket;
	

	transaction_t(hss_packets_t* _ppacket,HSS_INFO* _hssinfo,void* _proc)
	{
		
		hss_acquire(hssinfo=_hssinfo);
		hss_acquire(proc=_proc);
		hss_acquire(ppacket=_ppacket);
		
		OVERLAPPED t={};
		ovl=t;
		ovl.hEvent=WSACreateEvent();
	}
	//
	~transaction_t()
	{
		hss_release(ppacket);
		hss_release(proc);
		hss_release(hssinfo);
		CloseHandle(ovl.hEvent);
	}
 static void on_free(transaction_t* p)
 {
	 p->~transaction_t();
 }

 inline void on_complete()
 {

	 DWORD dwtot=0;
	 HRESULT err;
	 BOOL f=GetOverlappedResult((HANDLE)hssinfo->s,&ovl,&dwtot,1);
	 
     err=(f)?0:GetLastError(); 
	 if(!err)
	 {
		 hss_packets_t::locker_t(ppacket->mutex);
		 ppacket->fullsize-=long(dwtot);

	 }
     hssinfo->error_code=err;
	 closure_stub::safe_call(proc,hssinfo,(void*)dwtot,(void*)err);

 }

 static void __stdcall s_on_complete(void* p,BOOLEAN TimerOrWaitFired)
 {
	 transaction_t* t=(transaction_t*)p;
	 UnregisterWaitEx(t->hrw,0);
	 t->on_complete();
	 hss_release(t);
 }

bool pool()
{
 hss_acquire(this);
 bool f=RegisterWaitForSingleObject(&hrw,ovl.hEvent,&s_on_complete,this,INFINITE,WT_EXECUTEONLYONCE); 
 if(!f)
 	 hss_release(this);

 return f;
}

static transaction_t* create_instance(hss_packets_t* ppacket,HSS_INFO* hssinfo,void* proc)
{
	
	transaction_t* t=new (hss_alloc(sizeof(transaction_t))) transaction_t(ppacket,hssinfo,proc);
	hss_weak_ref(t,&transaction_t::on_free,t);
	return t;
	
}


	
};


 //LPFN_TRANSMITPACKETS TransmitPackets; 
 //WSAID_TRANSMITPACKETS

 hs_socket_utils::CS mutex;
  list_t m_list;
  handle_list_t m_handle_list;
  std::vector<TRANSMIT_PACKETS_ELEMENT> m_tpe;
  long long fullsize;


 inline long long buffer_amount()
 {
           locker_t lock(mutex);
		   return fullsize;
 }

 inline bool check_lock(int t)
 {
	 if(t==t_eof)
	 {
		 if(ffilllock) return true;
		  
		 return ffilllock=fill_TPE();
	 }
	 return false;
 }
  

 //hs_socket_utils::locker_t
  inline bool push(int t,void* ptr)
  {
	  locker_t lock(mutex);
	  if(t==t_eof) return check_lock(t);
	  if(ffilllock) return false; 
      hss_acquire(ptr);
	  m_list.push_back(std::make_pair(t,ptr));
	  return true;
  }

  inline bool  push_copy(int t,void* ptr,int cb=-1)
  {
	  locker_t lock(mutex);

	  if(t==t_eof) return check_lock(t);
	  if(ffilllock) return false; 

	  if(t==t_clear)
	  {
		  clear();
		  return true;
	  }
	  
	  void* pp;
	  if(t==t_region)
	  {
		  hss_acquire(ptr);
		  pp=ptr;
   	  }
	  else
	  {
		  char* ppp=(char*)ptr;
       if(cb<0)
	   {
		   if(cb==-2) cb=safe_len_zerozero((char*)ptr,0);
		   else   cb=safe_len((char*)ptr,0);
	   }
	   pp=hss_alloc(cb);
	   if(ptr) memcpy(pp,ptr,cb);
	  }
	   return push(t,pp);
  
  }

  static void on_free(hss_packets_t* p)
  {
	  p->~hss_packets_t();
  }

  inline void clear()
  {
	  locker_t lock(mutex);
	  for(list_t::iterator i=m_list.begin();i!=m_list.end();++i)
		  hss_release((*i).second);
	  m_list.clear();
/*
	  for(int n=0;n<m_tpe.size();++n)
	  {
		  if(m_tpe[n].dwElFlags==TP_ELEMENT_FILE)
             		  CloseHandle(m_tpe[n].hFile);
	  }
*/
	  for(handle_list_t::iterator i=m_handle_list.begin();i!=m_handle_list.end();++i)
	  {
		    CloseHandle(*i);
	  }
	  m_handle_list.clear();
	  m_tpe.clear();
  }

  hss_packets_t():fullsize(0)
   {

	   //ovl=OVERLAPPED();
	   //ovl.hEvent=WSACreateEvent();
        
   }
   ~hss_packets_t()
   {
	   //	   SIO_GET_EXTENSION_FUNCTION_POINTER 
	   //TransmitPackets 
	   //hss_release(proc);
	   //hss_release(hssinfo);
      //CloseHandle(ovl.hEvent);
	   clear();
   }

inline bool fill_TPE()
{
	locker_t lock(mutex);
	int c=m_list.size();
	m_tpe.resize(0);
    m_tpe.reserve(c<<2);
	LARGE_INTEGER ll={};
	const LONGLONG lmaxpack=0x80000000;

	int n=0;
	for(list_t::iterator i=m_list.begin();i!=m_list.end();++i)
	{
       int t=i->first;
	   void* ptr=i->second;
	   
	   TRANSMIT_PACKETS_ELEMENT tpe={TP_ELEMENT_MEMORY};
		   //m_tpe[n++];
	   //tpe.dwElFlags=TP_ELEMENT_MEMORY;
	   if(t==t_region)
	   {
        tpe.pBuffer=((WSABUF*)ptr)->buf;
		tpe.cLength=((WSABUF*)ptr)->len;
		ll.QuadPart=tpe.cLength;
		 m_tpe.push_back(tpe);
	   }
	   else  if(t==t_memory)
	     {
             tpe.pBuffer=ptr;
			 tpe.cLength=hss_size(ptr);
			 ll.QuadPart=tpe.cLength;
			 m_tpe.push_back(tpe);
	     }
	      else if(t==t_file)
		 {


			int cb=hss_size(ptr);
			
			v_buf<char> tmp(cb+1);
			char *pfn=tmp;
			memcpy(pfn,ptr,cb);
			tpe.dwElFlags=TP_ELEMENT_FILE;
			//tpe.hFile=CreateFileW(char_mutator<CP_UTF8>(pfn),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,0,0);
			tpe.hFile=CreateFileW((wchar_t*)(pfn),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);//FILE_FLAG_SEQUENTIAL_SCAN

			if(INVALID_HANDLE_VALUE==tpe.hFile)
			{
				clear();
				return false;
			}

			  m_handle_list.push_back(tpe.hFile);
			
			if(!GetFileSizeEx(tpe.hFile,&ll))
				return false;

			LONGLONG n=ll.QuadPart/lmaxpack;
			LARGE_INTEGER lr;
			lr.QuadPart=ll.QuadPart-n*lmaxpack;
			tpe.cLength=lr.LowPart;
			if(tpe.cLength)
				   m_tpe.push_back(tpe);

             tpe.nFileOffset.QuadPart+=tpe.cLength;
			 tpe.cLength=lmaxpack;
			for(LONGLONG i=0;i<n;++i)
			{

                m_tpe.push_back(tpe);
				tpe.nFileOffset.QuadPart+=lmaxpack;
			}



			//
			/*
			if(ll.QuadPart<lmaxpack)
				m_tpe.push_back(tpe);
			else
			{
				LONGLONG n=ll.QuadPart/lmaxpack;
				LONGLONG r=ll.QuadPart-n*lmaxpack;


			}
			//*/
              //m_tpe.push_back(tpe);

	    }
		else 
		{
			clear();
			return false;
		}
	
	}

	fullsize+=(long long)ll.QuadPart;

	return true;
}


  bool send(HSS_INFO* phs,void* _proc,bool syncro=false) 
  {
	  HRESULT hr;
	  if(!phs) return false;
	  ioctrl_t ioctrl(phs->s);
	  if(!ioctrl)  return false;

	  if(!push_copy(t_eof,0,0))
		   return false;
        
	  int c=m_tpe.size();
	  TRANSMIT_PACKETS_ELEMENT* pte=&m_tpe[0];


      transaction_t* t=transaction_t::create_instance(this,phs,_proc);

      
	  //if(c>1)
	  if(0)
	  {
		  ULARGE_INTEGER ll={};
          ll.LowPart=GetFileSize(pte[1].hFile,(DWORD*)&ll.HighPart);    
		  if(ll.QuadPart>0x000000007fffffff)
			  pte[1].cLength=0x80000000;//0x7fffffff;
  	  }


	  BOOL f; 
		  f=ioctrl.TransmitPackets(phs->s,pte,c,0,&t->ovl,0);
		  //f=ioctrl.TransmitPackets(phs->s,pte,c,0,0,0);
	  
	  f=f||((hr=WSAGetLastError())== ERROR_IO_PENDING);
	  if(f)
	  {
        if(syncro)
		{
		  t->on_complete();
		}
		else f=t->pool();   

	  }

	   hss_release(t);


      

	  return f;
  }

  static hss_packets_t* create_instance()
  {

	  hss_packets_t* t=new (hss_alloc(sizeof(hss_packets_t))) hss_packets_t;
	  hss_weak_ref(t,&hss_packets_t::on_free,t);
      hss_info_ll(t)[1]=(char*)typeid(t).name();
	  return t;

  }

};

