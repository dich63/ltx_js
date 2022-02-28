//ipc_marshal
#pragma once


#include "ipc_marshal_def.h"
#pragma pack(push)
#pragma pack(1)



struct ipc_region_t:ipc_region_base_t
{
	typedef void (*region_destroy_t)(void* p,void* pctx);
	union{
		region_destroy_t destroy_proc;
		void* __proc;
		uint64_t __rproc;
	};
	union{
		struct {char _ipc_buf[8];};
		char* _ipc_ptr;
		uint64_t __rsrv;
	};

};


struct ipc_marshal_data_t;

struct ipc_marshal_data_factory_t
{
 typedef void (__cdecl *on_delete_t)(ipc_marshal_data_t* pmd); 

virtual	ipc_marshal_data_t* __cdecl allocate( int sizeb,on_delete_t dp)=0;
virtual	void __cdecl release_marshal_data(ipc_marshal_data_t* pmd)=0;

};


struct ipc_region_holder_t
{
	typedef void (__cdecl *on_weak_ref_t)(void* pcntxt,void* ptr); 

	virtual	ipc_marshal_data_t* __cdecl allocate_region_data( int sizeb,on_weak_ref_t dp)=0;
	virtual	void __cdecl release_region(ipc_region_t* pmd)=0;

};



struct ipc_marshal_data_t
{
	typedef unsigned long long uint64_t;
	typedef  long long int64_t;
	typedef unsigned long  uint32_t;
	typedef unsigned short  uint16_t;
	typedef long  int32_t;
	
//#ifndef uuid_t
	typedef  GUID	uuid_t;
//#endif




  
  typedef  ipc_region_t   region_t;

  struct header_min_t  {
	  uint32_t sizeb;
		  HRESULT hr;
  };


  struct header_t
  {
	  union{
		  header_min_t hmin;
		  struct  
		  {
			  uint32_t sizeb;
			  HRESULT hr;
		  };

	  };
    
	uuid_t port_uuid;
	uuid_t conn_uuid;
	uint32_t sender_pid;
	uint32_t server_pid;

	union{
		uint64_t rsrvd0;
		HANDLE sender_handle;
	};

	union{
		uint64_t rsrvd1;
		HANDLE server_handle;
	};
/*
	union{
		uint64_t rsrvd2;
		HANDLE server_thread_handle;
	};
*/
	
	int32_t regions_attr;
	int32_t region_count;

  } header;


	union sender_trusted_t
	{
		uint64_t rsrvd00;
		HANDLE handle;
	} sender_trusted;
   //struct region_list_t{
  
   
   ipc_region_t regions[1];
  //} region_list;

  
};

#pragma pack(pop)




