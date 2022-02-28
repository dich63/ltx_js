#pragma once

#include "video/singleton_utils.h"
#include "ipc_ports/hs_socket.h"

#include <time.h>
#include <stdio.h>
#include <map>
#include <string>
#include <memory>
//#include "ions.h"
//#include "http_dir_gen.h"

#define  CP_X CP_THREAD_ACP

#define s_m(s) char_mutator<CP_X>(#s)
#define _m(s) char_mutator<CP_X>(s)
#define get_m(a,s) a[s_m(s)]

//#define FILEIID  L"FD583F3CB33E4226B56558AF1F59C7C3"
template<int ver=0>
struct hss_mirrorserver_t
{ 
	typedef hss_mirrorserver_t* this_t;  
	typedef mutex_cs_t mutex_t;
	typedef  std::map<std::string,std::string>  map_t;


    struct handle_t
	{
		HANDLE h;
		handle_t(HANDLE _h):h(_h){}
		~handle_t(){ if(h&&(h!=INVALID_HANDLE_VALUE)) CloseHandle(h); }
		inline operator HANDLE(){ return h;} 
		
	};

    struct server_item_t
	{
		hss_mirrorserver_t& owner;
		map_t::iterator it;
		std::string key,port;
		void* h_item_port;


		server_item_t(hss_mirrorserver_t& owner_,map_t::iterator it_,std::string port_,std::string key_)
			:owner(owner_),it(it_),port(port_),key(key_)
		{
            
		}
		~server_item_t(){
			hss_release(h_item_port);
		}



	};



	static void   s_on_http_accept(HSS_INFO* hovl)
	{

		if(hovl&&hovl->user_data) this_t(hovl->user_data)->on_http_accept(hovl);
	}

	void on_http_accept(HSS_INFO* hovl)
	{
		ULONG st;
		//				return;
		//st=::hss_HTTP_request_headers_chain(hovl); 
		//st=::hss_websocket_handshake_chain(hovl);
		st=::hss_websocket_handshake_reply(hovl);
		if((st&(1<<HSD_REPLY))==0) return;

		char* pargs=hss_reset_SOCKET_DATA(hovl,HSD_PARAMS,0,0,0);
		argv_zz<char> args(pargs);
		std::string port;
		map_t::iterator it;

		char* pk=args["public.key"];
		if(pk)
		{

		}
		else
		{

		}




		//


	}


template <class Args>
inline hss_mirrorserver_t& init( Args& a)
{
	clear();
    args=a;
	hport=hss_create_socket_portW(args[L"port"],HSF_SRD_AUTO|HSF_SRD_CREATE_PATH,&s_on_http_accept,this);
 
	return *this;
}


 inline void clear(){  hss_release(hport);}

~hss_mirrorserver_t()	{	  clear();	}

 void wait(int tio=-1)
 {
	 hss_wait_event(hport,tio);
  }
 hss_mirrorserver_t():hport(0){ };

inline  void erase_port(map_t::iterator i)
{
	 //str_uuid()
}

inline  bool find_port(char* key,std::string& port )
 {
	  locker_t<mutex_t> lock(mutex);
	  map_t::iterator i=keymap.find(key);
	  if(i==map_t::end()) return false;
         port=i->second;
        return true;
 }

inline  bool generate_port(char* key,std::string& port,map_t::iterator& iout )
{
	locker_t<mutex_t> lock(mutex);
	map_t::iterator i=keymap.find(key);
	if(i!=map_t::end()) return false;
	port=(char*)str_uuid();
	iout=keymap.insert(i,std::make_pair(std::string(key),port));	
	return true;
}



 mutex_t mutex;
 map_t keymap;
 console_shared_data_t<wchar_t,0> shared_console;
 void * hport;
 argv_zzs<wchar_t> args;
};