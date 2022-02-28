#pragma once 
//#include "ipc_ports/ltx_js.h"


#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>
#include <ole2.h>



#define  LTX_REG_FACTORY_BEGIN_0(triplet) class_triplet_t triplet[]={
#define  LTX_REG_FACTORY(_class,_alias) {_class::GetCLSID(),&_class::GetClassObject,L###_alias},

#define  LTX_REG_FACTORY2(_c,_a1,_a2) LTX_REG_FACTORY(_c,_a1) LTX_REG_FACTORY(_c,_a2)
#define  LTX_REG_FACTORY3(_c,_a1,_a2,_a3) LTX_REG_FACTORY2(_c,_a1,_a2) LTX_REG_FACTORY(_c,_a3)
#define  LTX_REG_FACTORY4(_c,_a1,_a2,_a3,_a4) LTX_REG_FACTORY2(_c,_a1,_a2) LTX_REG_FACTORY2(_c,_a3,_a4)
#define  LTX_REG_FACTORY8(_c,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8)  LTX_REG_FACTORY4(_c,_a1,_a2,_a3,_a4) LTX_REG_FACTORY4(_c,_a5,_a6,_a7,_a8)

#define  LTX_REG_FACTORY_NULL(_class) {_class::GetCLSID(),&_class::GetClassObject,0},
#define  LTX_REG_FACTORY_MARSHAL(_class,_alias) LTX_REG_FACTORY_NULL(_class) LTX_REG_FACTORY(_class,_alias)
#define  LTX_REG_FACTORY_END_0 {GUID_NULL}};
#define  LTX_REG_FACTORY_BEGIN2(p_hr,pind){ HRESULT *phr=p_hr,_hr000;int * pindex=pind ;class_triplet_t triplet[]={
#define  LTX_REG_FACTORY_BEGIN1(p_hr) LTX_REG_FACTORY_BEGIN2(p_hr,0)
#define  LTX_REG_FACTORY_BEGIN LTX_REG_FACTORY_BEGIN2(0,0)
#define  LTX_REG_FACTORY_END LTX_REG_FACTORY_END_0;_hr000=ltx_register_classes(triplet,pindex);if(phr) *phr=_hr000;}

#define LTX_BINDCONTEXT_PARAMS L"ltx.arguments"



#define  PROC_LIST_BEGIN(_external_callbacks) extern "C" external_callbacks_t _external_callbacks[]={

#define  PROC_ITEM_NAME(NAME,_proc,attr) {#NAME,&_proc,attr},

#define  PROC_ITEM_NAME_POSTFIX_ATTR(pfix,_proc,attr) PROC_ITEM_NAME(_proc,pfix##_proc,attr)
#define  PROC_ITEM_NAME_PREFIX_ATTR(pfix,_proc,attr) PROC_ITEM_NAME(pfix##_proc,_proc,attr)

#define  PROC_ITEM_NAME_POSTFIX(pfix,_proc)  PROC_ITEM_NAME_POSTFIX_ATTR(pfix,_proc,0)
#define  PROC_ITEM_NAME_PREFIX(pfix,_proc) PROC_ITEM_NAME_PREFIX_ATTR(pfix,_proc,0)


#define  PROC_ITEM(_proc) PROC_ITEM_NAME(_proc,_proc,0)

#define  PROC_ITEM_STRUCT(_struct,_proc) {#_proc,&_struct::_proc},
#define  PROC_STRUCT_STRUCT(_struct,_name) {#_name,&_struct,external_callbacks_t::attr_struct},
#define  PROC_LIST_END {0,0}};


struct external_callbacks_t
{
	enum{
		attr_proc=0, 
		attr_struct=8 
	};
	const char* name;
	void* proc;
	int attr,reserved;
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
extern "C" HRESULT ltxGetObjectEx(const wchar_t* moniker_str,REFIID riid,void** ppObj,IBindCtx *bctx=0);
extern "C" HRESULT bindObject(const wchar_t* moniker_str,REFIID riid,void** ppObj);
extern "C" HRESULT bindObjectEx(const wchar_t* moniker_str,REFIID riid,void** ppObj,IBindCtx *bctx=0);
extern "C" HRESULT bindObjectWithArgs(const wchar_t* moniker_str,REFIID riid,void** ppObj,int argc=0,VARIANT* argv=0);
extern "C" HRESULT bindObjectWithArgsEx(const wchar_t* moniker_str,REFIID riid,void** ppObj,bool freverse=true,int argc=0,VARIANT* argv=0);
extern "C" int  ltx_stop_all_apartment(DWORD timeout);
extern "C"  void* ltx_tss_onexit(void* proc,void *ptr=0);

extern "C"  HRESULT  ltx_get_register_objref(const wchar_t* name, VARIANT* pmoniker=0);

