#pragma once 
//#include "ipc_ports/ltx_js.h"


#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>
#include <ole2.h>



#define  LTX_REG_FACTORY_BEGIN_0(triplet) class_triplet_t triplet[]={
#define  LTX_REG_FACTORY(_class,_alias) {_class::GetCLSID(),&_class::GetClassObject,L###_alias},
#define  LTX_REG_FACTORY_NULL(_class) {_class::GetCLSID(),&_class::GetClassObject,0},
#define  LTX_REG_FACTORY_MARSHAL(_class,_alias) LTX_REG_FACTORY_NULL(_class) LTX_REG_FACTORY(_class,_alias)
#define  LTX_REG_FACTORY_END_0 {GUID_NULL}};
#define  LTX_REG_FACTORY_BEGIN2(p_hr,pind){ HRESULT *phr=p_hr,_hr000;int * pindex=pind ;class_triplet_t triplet[]={
#define  LTX_REG_FACTORY_BEGIN1(p_hr) LTX_REG_FACTORY_BEGIN2(p_hr,0)
#define  LTX_REG_FACTORY_BEGIN LTX_REG_FACTORY_BEGIN2(0,0)
#define  LTX_REG_FACTORY_END LTX_REG_FACTORY_END_0;_hr000=ltx_register_classes(triplet,pindex);if(phr) *phr=_hr000;}

#define LTX_BINDCONTEXT_PARAMS L"ltx.arguments"

struct external_callbacks_t
{
	const char* name;
	void* proc;
	int attr;
};

struct class_object_pair_t
{
	typedef HRESULT( __stdcall *GetClassObject_t)(REFCLSID,REFIID,void** ppv);
	CLSID clsid;					   
	GetClassObject_t GetClassObject;
	IUnknown* pmarshal_context;
	
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
extern "C" HRESULT ltx_register_classes(const class_triplet_t* pct,int * pbadindex=0);
extern "C" HRESULT ltx_get_class(const wchar_t* moniker_str,IUnknown** ppUnk,class_object_pair_t** pp_pair=0);
extern "C" HRESULT ltxGetObject(const wchar_t* moniker_str,REFIID riid,void** ppObj);
extern "C" HRESULT bindObject(const wchar_t* moniker_str,REFIID riid,void** ppObj);
extern "C" int  ltx_stop_all_apartment(DWORD timeout);
extern "C"  void* ltx_tss_onexit(void* proc,void *ptr=0);
