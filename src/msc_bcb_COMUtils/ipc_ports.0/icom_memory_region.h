#pragma once
//com_memory_region.h
#include "callback_context_arguments.h"

struct callback_dispatch_marshal_base_t;

struct __declspec(uuid("{476E367A-B0D0-43ae-91E1-DDBA675E930B}")) imemory_region_t:IUnknown
{	
	virtual HRESULT io(ULONG64 offset,ULONG64 size,void* p_in_buf,void* p_out_buf)=0;
	virtual HRESULT lock(BOOL f)=0;
	virtual HRESULT tryLock(DWORD timeOut)=0;
	virtual HRESULT getSize(ULONG64  *psize)=0;
	virtual HRESULT getPtr(ULONG64 offset,ULONG64 size,void** pptr)=0;
	virtual HRESULT slice(ULONG64 offset,ULONG64 size,imemory_region_t** pp_memory_region)=0;
	virtual HRESULT getInfo(BSTR *pout)=0;
};

struct aggregator_data_t
{
	IID  iid;		
	void (*on_exit)(void* pcontext);		

};

struct imemory_region_aggregator_t:aggregator_data_t
{	 
	 HRESULT (*io)(ULONG64 offset,ULONG64 size,void* p_in_buf,void* p_out_buf);
	 HRESULT (*getPtr)(void* pcontext,ULONG64 offset,ULONG64 size,void** pptr);
	 HRESULT (*getInfoSize)(void* pcontext,ULONG64  *psize,BSTR *pout);
	 HRESULT (*tryLockUnlock)(void* pcontext,BOOL f,DWORD timeOut);
	 HRESULT (*slice)(void* pcontext,ULONG64 offset,ULONG64 size,callback_dispatch_marshal_base_t** pp);	 
};


struct imarshal_aggregator_t:aggregator_data_t
{	
	DWORD marshalmask; 
	CLSID clsid_marshal;
	char createInstance_export_name[256];
	HRESULT (*createInstance)(callback_dispatch_marshal_base_t * pdata);
	HRESULT (*marshal_to_process_callback)(void* pcontext,IStream* stream,DWORD target_pid,void* putils);
	HRESULT (*unmarshal_callback)(void* pcontext,IStream* stream,void* putils);
	
};


struct callback_dispatch_marshal_base_t
{
	
	void (*dispatch_callback)(void *presult,void* pcontext,void *pcallback_context_arguments,DWORD c,void *pvar,DWORD cn,void* pf,void* pex,void* flags);
	void (*on_exit)(void* pcontext);
	void* pcontext;
	ULONG flags;
	// for custom marshaling
	
};