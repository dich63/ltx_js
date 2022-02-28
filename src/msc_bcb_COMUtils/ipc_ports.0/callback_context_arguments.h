#pragma once
//callback_context_arguments.h


#define DISPATCH_GETSF (DISPATCH_PROPERTYGET|DISPATCH_METHOD)
#define DISPATCH_PUTSF (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)


#define is_get(f) (f&DISPATCH_GETSF)
#define is_put(f) (f&DISPATCH_PUTSF)


#define FLAG_CBD_FTM (1<<0)
#define FLAG_CBD_APA (1<<1)
#define FLAG_CBD_APP FLAG_CBD_APA
#define FLAG_CBD_STA (1<<2)
#define FLAG_CBD_GIT (1<<3)
#define FLAG_CBD_EC  (1<<4)
#define FLAG_CBD_REVERSED  (1<<8)
#define FLAG_CBD_OLD  (1<<10)
#define FLAG_CBD_COM  (1<<11)


//#define FLAG_MASK_PUT  ((DISPATCH_PUTSF)<<16)
//#define FLAG_MASK_GET  ((DISPATCH_GETSF)<<16)

#define FLAG_MASK_PUT  ((DISPATCH_PUTSF)<<0)
#define FLAG_MASK_GET  ((DISPATCH_GETSF)<<0)


struct  __declspec(uuid("{CF188C71-CABD-4673-AC70-82F3EB184B2A}")) ICallbackDispatchFactory:IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE CreateInstance(DWORD exflags,void* pcalback,void* context,void* on_exit,REFIID riid,void** ppObj)=0;
	virtual HRESULT STDMETHODCALLTYPE CreateInstanceV(DWORD exflags,void* pcalback,void* context,void* on_exit,VARIANT* pv)=0;
};



typedef void (* raise_error_proc_t)(wchar_t* smessage,wchar_t* ssource);
typedef  HRESULT (*bind_object_t)(const wchar_t* moniker_str,REFIID riid,void** ppObj);

typedef struct callback_lib_arguments_t
{
	HRESULT (*create_callback)(DWORD exflags,void* pcalback,void* context,void* on_exit,VARIANT* pv);
	HRESULT (*bind_object)(const wchar_t* moniker_str,REFIID riid,void** ppObj);

} callback_lib_arguments_t,*pcallback_lib_arguments_t;



typedef struct callback_context_arguments_t
{
	IDispatch* parguments;
	void (* raise_error)(wchar_t* smessage,wchar_t* ssource);	
	void* pcontext;
	int argc;
	VARIANT *argv;
	union{
		VARIANT* result;
		VARIANT* put_value;
	};

	DWORD flags;
	HRESULT* phr;
	EXCEPINFO * pei;
	void** ppcontext;
	int fmaskreverse;
	pcallback_lib_arguments_t pclib;

}  callback_context_arguments_t,*pcallback_context_arguments_t;
