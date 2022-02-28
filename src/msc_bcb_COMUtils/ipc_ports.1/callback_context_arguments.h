#pragma once
//callback_context_arguments.h


#define DISPATCH_GETSF (DISPATCH_PROPERTYGET|DISPATCH_METHOD)
#define DISPATCH_PUTSF (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)


#define is_get(f) (f&DISPATCH_GETSF)
#define is_put(f) (f&DISPATCH_PUTSF)


#define FLAG_CBD_FTM (1<<0)
#define FLAG_CBD_APP (1<<1)
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


struct __declspec(uuid("{08BFFEBA-F4B3-4024-B102-FDC7094E551D}")) IFakeUnknown_t{};
struct  __declspec(uuid("{06B97168-EBD8-499d-906A-DCF447A8D3EF}")) IFreeThreadMarshal:IMarshal{};

struct  __declspec(uuid("{9E1C4FBC-74D2-4a4c-886C-25A5A9562A8C}")) iaggregator_helper:IUnknown
{
	virtual BOOL STDMETHODCALLTYPE innerQueryInterface(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT* phr )=0; 
	virtual HRESULT STDMETHODCALLTYPE setContext(void* pContext)=0; 
};

struct  __declspec(uuid("{F1CDE93B-86B3-44c0-B9DF-0110791D450A}")) iaggregator_container:IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE SetAggregator(GUID iid,IUnknown* punk=0)=0;	
};



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
	struct disp_id_t
	{
		enum{
			TSSid=0xBABAEB
		};
		  DISPID id;
          BSTR tssName;
		  
	};
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
	disp_id_t tss_id;
}  callback_context_arguments_t,*pcallback_context_arguments_t;


struct i_marshal_helper_t
{   
   virtual  HRESULT marshal_hko(IStream* pStm,HANDLE hko=0)=0;
   virtual  HRESULT marshal_interface(IStream* pStm,REFIID riid,IUnknown* pObj)=0;
   virtual  HRESULT unmarshal_hko(IStream* pStm,HANDLE* phko=0)=0;
   virtual  HRESULT unmarshal_and_release(IStream *pStm,REFIID riid,void** ppObj)=0;
};


struct aggregator_callback_list_t{

	HRESULT (*GetClassID)(CLSID* pclsid);	
	HRESULT (*CreateContext)(iaggregator_container* pcontainer,LPOLESTR pszDisplayName,int  unmarshal_flag,void ** ppcontext);
	void (*OnExit)(void *context);
	//HRESULT (*InitContext)(void* context,i_marshal_helper_t* helper,IStream* pStm);
	//HRESULT (*MarshalToProcess)(void* context,i_marshal_helper_t* helper,DWORD pid_target,IStream* pStm);	

};

struct MarshalCallbackData_t
{  
    HRESULT (*GetClassID)(CLSID* pclsid);
	//HRESULT (*CreateInstance)(i_marshal_helper_t* helper,DWORD flags,void* init_context,LPOLESTR pszDisplayName,REFIID riid,void ** ppvObject);
	HRESULT (*CreateContext)(LPOLESTR pszDisplayName,DWORD* pflags,void ** ppcontext);
	void (*OnExit)(void *context);
    void (*InvokeByName)(void* context,pcallback_context_arguments_t pcca,i_marshal_helper_t* helper);
	HRESULT (*MarshalToProcess)(void* context,i_marshal_helper_t* helper,DWORD pid_target,IStream* pStm);
	HRESULT (*Unmarshal)(void* context,i_marshal_helper_t* helper,IStream* pStm);

};

struct  __declspec(uuid("{46EC035A-B7B3-4af6-811C-F41DC346A25E}")) IMarshalByValueDispatchRegister:IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE Register(LPOLESTR alias,MarshalCallbackData_t* data)=0;

};

