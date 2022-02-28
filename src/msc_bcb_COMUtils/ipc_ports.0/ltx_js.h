#pragma once 
//#include "ipc_ports/ltx_js.h"


#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>
#include <ole2.h>

struct external_callbacks_t
{
	const char* name;
	void* proc;
};

struct class_object_pair_t
{
	typedef HRESULT( __stdcall *GetClassObject_t)(REFCLSID,REFIID,void** ppv);
	CLSID clsid;					   
	GetClassObject_t GetClassObject;
	
};
struct class_triplet_t
{
	CLSID clsid;					   
	class_object_pair_t::GetClassObject_t GetClassObject;
	const wchar_t* alias;
};

typedef class_object_pair_t* pclass_object_pair_t; 

//extern "C"  external_callbacks_t ltx_callback_list[];
extern "C" HRESULT ltx_register_class(CLSID clsid,class_object_pair_t::GetClassObject_t pClassObject,const wchar_t* alias=0,IUnknown** ppUnk=0,class_object_pair_t** pp_pair=0);
extern "C" HRESULT ltx_get_class(const wchar_t* moniker_str,IUnknown** ppUnk,class_object_pair_t** pp_pair=0);
extern "C" HRESULT ltxGetObject(const wchar_t* moniker_str,REFIID riid,void** ppObj);
extern "C" HRESULT bindObject(const wchar_t* moniker_str,REFIID riid,void** ppObj);
extern "C" int  ltx_stop_all_apartment(DWORD timeout);
extern "C"  void* ltx_tss_onexit(void* proc,void *ptr=0);