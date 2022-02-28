#pragma once
//#define uuid_null (uuid_t())

#ifndef _WIN32
//typedef long HRESULT;
#define E_NOINTERFACE           HRESULT(0x80004002L)
#define S_OK (0)
typedef struct _uuid_t {
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];
} uuid_t;

struct i_unknown {

	virtual HRESULT  QueryInterface(uuid_t& riid, void **ppvObject) = 0;
	virtual long  AddRef(void) = 0;
	virtual long  Release(void) = 0;


};
#else
#include <windows.h>
#include <objbase.h>
//#include <Inspectable.h>
typedef   IUnknown i_unknown;

//#define i_unknown_uuid  (__uuidof(i_unknown))

#endif

//#define i_unknown_uuid  (__uuidof(i_unknown)


typedef void(*on_destroy_proc_t)(void*);
const uuid_t uuid_null = {};
const uuid_t i_unknown_uuid={ 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 };
const uuid_t i_link_context_uuid={ 0x7b7ed1f1, 0x5b59, 0x4a90, 0x87, 0x73, 0x31, 0x44, 0x00, 0x9f, 0x3e, 0xd0 };

struct link_context_data_t {
	void* ptr;
	on_destroy_proc_t destroy_proc;
	uuid_t uuid_check;
};


struct i_link_context :i_unknown {	
	virtual HRESULT  link(i_unknown* punk) = 0;
	virtual HRESULT  unlink(i_unknown* punk) = 0;
	virtual HRESULT  clear_links(void) = 0;
	//virtual HRESULT  get_iid(uuid_t* piid) = 0;
};


