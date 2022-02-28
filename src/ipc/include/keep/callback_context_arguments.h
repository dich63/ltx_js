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



struct  __declspec(uuid("{B88FF4D5-7296-42fc-A310-54729E2C1FDC}")) i_stub_holder:IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE wrap(IUnknown* punk)=0;
	virtual HRESULT STDMETHODCALLTYPE unwrap(REFIID riid,void** ppObj)=0;
	virtual HRESULT STDMETHODCALLTYPE objref(BSTR* pref)=0;

};


#pragma push(pack)
#pragma pack(1)
struct shared_mem_data_t
{
	INT64 sizeb;
	INT64 offset;
	int vt;
	int element_size;
	INT64 flags;	
	

};
typedef shared_mem_data_t* pshared_mem_data_t; 

struct shared_socket_data_t
{
   SOCKET s;
   char*  header_data;
   int header_data_length;
};

typedef shared_socket_data_t* pshared_socket_data_t; 

struct shared_mem_context_t
{
	union{
      pshared_mem_data_t psmd;
	  pshared_socket_data_t pssd;
	  void* pvoid;
	};
   void* pobject;
};
typedef shared_mem_context_t* pshared_mem_context_t; 

#pragma pop(pack)




struct __declspec(uuid("{E3E065F0-CF4A-4cfe-8B85-7FBB841AFC00}")) i_mbv_context:IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE GetContext(void** ppcontext)=0;

};


struct __declspec(uuid("{CAFB0CEA-E72E-40ea-9C70-2E4218A60DE3}")) i_mbv_context_lock:i_mbv_context
{
	typedef long long int64_t;

	struct locker_t
	{
		locker_t(i_mbv_context_lock * _p):p(_p)
		{
			p&&(p->Lock());
		}
		~locker_t()
		{
			p&&(p->Unlock());
		}

		i_mbv_context_lock * p;
	};	

	struct unlocker_t
	{
		unlocker_t(i_mbv_context_lock * _p):p(_p)
		{
			p&&(p->Unlock());
		}
		~unlocker_t()
		{
			p&&(p->Lock());
		}

		i_mbv_context_lock * p;
	};	

	virtual HRESULT STDMETHODCALLTYPE Lock()=0;
	virtual HRESULT STDMETHODCALLTYPE Unlock()=0;

};

 


struct  __declspec(uuid("{D3AD8B42-5DDD-445c-9D30-6B7F2CCB50E4}")) i_mbv_buffer_ptr:i_mbv_context_lock
{		
	
 	virtual HRESULT STDMETHODCALLTYPE GetPtr(void** pptr,int64_t* pByteLength=0)=0;
	//virtual HRESULT STDMETHODCALLTYPE GetRegionPtr(int64_t pByteOffset,int64_t pByteLength,void** pptr)=0;
	//virtual HRESULT STDMETHODCALLTYPE Lock()=0;
	//virtual HRESULT STDMETHODCALLTYPE Unlock()=0;
	
};




struct  __declspec(uuid("{B1132551-BFB3-4287-8452-07A639E8C9E4}")) i_mbv_region_ptr:i_mbv_context_lock
{		
	virtual HRESULT STDMETHODCALLTYPE GetRegionPtr(int64_t ByteOffset,int64_t ByteLength,void** pptr)=0;
	virtual HRESULT STDMETHODCALLTYPE GetTotalSize(int64_t* pByteLength=0)=0;
};


struct  __declspec(uuid("{2252071F-A955-4e7e-8094-8CA2A8AFB14E}")) i_mbv_typed_buffer_ptr:i_mbv_buffer_ptr
{
	virtual HRESULT STDMETHODCALLTYPE GetCount(int64_t* pcount)=0;
	virtual HRESULT STDMETHODCALLTYPE GetElement(int64_t index,VARIANT * pvalue)=0;
	virtual HRESULT STDMETHODCALLTYPE SetElement(int64_t index,VARIANT newvalue)=0;

};


typedef void (* raise_error_proc_t)(wchar_t* smessage,wchar_t* ssource);
typedef  HRESULT (*bind_object_t)(const wchar_t* moniker_str,REFIID riid,void** ppObj);

typedef struct callback_lib_arguments_t
{
  HRESULT (*create_callback)(DWORD exflags,void* pcalback,void* context,void* on_exit,VARIANT* pv);
  HRESULT (*bind_object)(const wchar_t* moniker_str,REFIID riid,void** ppObj);
  HRESULT (*bind_object_args)(const wchar_t* moniker_str,REFIID riid,void** ppObj,int argc,VARIANT* argv);

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
	HRESULT (*CreateContext)(BOOL fUnmarshal,IBindCtx* bctx,LPOLESTR pszDisplayName,DWORD* pflags,void ** ppcontext);
	void (*OnExit)(void *context);
    void (*InvokeByName)(void* context,pcallback_context_arguments_t pcca,i_marshal_helper_t* helper);
	HRESULT (*MarshalToProcess)(void* context,i_marshal_helper_t* helper,DWORD pid_target,IStream* pStm);
	HRESULT (*Unmarshal)(void* context,i_marshal_helper_t* helper,IStream* pStm);
	//HRESULT (*local_QueryInterface)(void *context,REFIID riid,void** ppv);
	HRESULT (*SetAggregatorHelpers)(void *context,iaggregator_container* pcontainer);
	

};

struct  __declspec(uuid("{46EC035A-B7B3-4af6-811C-F41DC346A25E}")) IMarshalByValueDispatchRegister:IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE Register(LPOLESTR alias,MarshalCallbackData_t* data)=0;

};

