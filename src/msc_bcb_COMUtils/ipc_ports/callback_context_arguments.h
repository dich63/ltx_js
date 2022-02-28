#pragma once
//callback_context_arguments.h
#include "wbs_def.h"

#define DISPATCH_GETSF (DISPATCH_PROPERTYGET|DISPATCH_METHOD)
#define DISPATCH_PUTSF (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)


#define is_get(f) (f&DISPATCH_GETSF)
#define is_put(f) (f&DISPATCH_PUTSF)


#define FLAG_CBD_FTM (1<<0)
#define FLAG_CBD_APP (1<<1)
#define FLAG_CBD_STA (1<<2)
#define FLAG_CBD_GIT (1<<3)
#define FLAG_CBD_EC  (1<<4)
#define FLAG_CBD_DBN (1<<5)
#define FLAG_CBD_MTA (1<<6)
#define FLAG_CBD_NO_MARSHAL (1<<7)
#define FLAG_CBD_REVERSED  (1<<8)
#define FLAG_CBD_OLD  (1<<10)
#define FLAG_CBD_COM  (1<<11)
#define FLAG_CBD_INFO  (1<<12)

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
	virtual HRESULT STDMETHODCALLTYPE GetContext(void** ppcontext)=0;
};




struct  __declspec(uuid("{CF188C71-CABD-4673-AC70-82F3EB184B2A}")) ICallbackDispatchFactory:IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE CreateInstance(DWORD exflags,void* pcalback,void* context,void* on_exit,REFIID riid,void** ppObj)=0;
	virtual HRESULT STDMETHODCALLTYPE CreateInstanceV(DWORD exflags,void* pcalback,void* context,void* on_exit,VARIANT* pv)=0;
	virtual HRESULT STDMETHODCALLTYPE bind_object(const wchar_t* moniker_str,REFIID riid,void** ppObj,IBindCtx* bctx=0)=0;
	virtual HRESULT STDMETHODCALLTYPE bind_object_args(const wchar_t* moniker_str,REFIID riid,void** ppObj,int argc=0,VARIANT* argv=0)=0;
	virtual HRESULT STDMETHODCALLTYPE bind_object_args_reverse(const wchar_t* moniker_str,REFIID riid,void** ppObj,int argc=0,VARIANT* argv=0)=0;

};

typedef ICallbackDispatchFactory IBindFactory;

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
	enum{
		CONNECTED=1<<0,
		OPEN=1<<1,
		CLOSED=1<<2
		
	};

	enum{
		f_srv=1<<0		
	};


	typedef ULONGLONG uint64_t;
	SOCKET sock;
	HANDLE habort_event;
	BSTR buffer_tail;
	BSTR header;
	union {
	struct {
	wbs_frame_base_t wbs_frame_read;
	 uint64_t state;
	 uint64_t flags;
	};
	struct {
		wbs_frame_base_t wbs_frame_read;
		uint64_t state;
		uint64_t flags;
	  } meta_data;

	};
   
};
typedef shared_socket_data_t* pshared_socket_data_t; 

struct kernel_object_t
{
	HANDLE hko;
	BSTR type;
	BSTR name;
};

typedef kernel_object_t* pkernel_object_t; 

// 



struct __declspec(uuid("{D573056D-66E4-433f-A75E-01C21E670079}")) i_wbs_recv_buffer:IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE on_open(void* pinfo,long reason)=0;
	virtual HRESULT STDMETHODCALLTYPE on_close(void* pinfo,long reason)=0;
    virtual HRESULT STDMETHODCALLTYPE on_recv_buffer(char** ppbuffer,long* plength,wbs_frame_base_t* pframe)=0;
	virtual HRESULT STDMETHODCALLTYPE on_message_complete(wbs_frame_base_t* pframe,HRESULT herror)=0;
};


// {07A41DD0-CF89-48b3-A02F-7CA7D4B1AAF4}

struct __declspec(uuid("{07A41DD0-CF89-48b3-A02F-7CA7D4B1AAF4}")) i_wbs_handler:IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE open(IUnknown* recv_buffer)=0;
	virtual HRESULT STDMETHODCALLTYPE close()=0;
	virtual HRESULT STDMETHODCALLTYPE send_single_message(int opcode,void* pdata,INT64* psize,ULONG32* pmask )=0;
	virtual HRESULT STDMETHODCALLTYPE send_message_begin(ULONG32* pmask,BOOL fbin=0 )=0;
	virtual HRESULT STDMETHODCALLTYPE send_frame_begin(int fin,void* pdata,INT64* psize)=0;
	virtual HRESULT STDMETHODCALLTYPE send_frame_end(int fin,void* pdata,INT64* psize)=0;
	virtual HRESULT STDMETHODCALLTYPE send_message_end()=0;	

};


typedef shared_socket_data_t* pshared_socket_data_t; 

/*
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
*/

#pragma pop(pack)




struct __declspec(uuid("{D0EF8802-8F26-4ddb-B21F-F8EF3B1A1634}")) i_mbv_container:IUnknown
{

	virtual HRESULT STDMETHODCALLTYPE GetLength(long* plength)=0;
	virtual HRESULT STDMETHODCALLTYPE GetNames(VARIANT** ppv,long* plength=0)=0;
	virtual HRESULT STDMETHODCALLTYPE GetItem(VARIANT key,VARIANT* pv)=0;
	virtual HRESULT STDMETHODCALLTYPE SetItem(VARIANT key,VARIANT v)=0;
	virtual HRESULT STDMETHODCALLTYPE IsItem(BSTR key)=0;

};


struct __declspec(uuid("{6438F2B2-0249-41a9-993A-B8CBA9E51D42}")) i_context:IUnknown
{

	virtual HRESULT STDMETHODCALLTYPE GetContext(void** ppcontext)=0;

};


struct __declspec(uuid("{9DDBC820-7316-4988-8AEC-12436D005DDD}")) i_context_with_cache:i_context
{

	virtual HRESULT STDMETHODCALLTYPE PushCacheValue(VARIANT value)=0;
	virtual HRESULT STDMETHODCALLTYPE ClearCache()=0;

};



struct __declspec(uuid("{E3E065F0-CF4A-4cfe-8B85-7FBB841AFC00}")) i_mbv_context:i_context{};
/*
{
	
	virtual HRESULT STDMETHODCALLTYPE GetContext(void** ppcontext)=0;
	

};
*/



struct __declspec(uuid("{F58084EB-A963-450e-80D2-6017CA2DE43F}")) i_mbv_kernel_object:i_mbv_context{};

struct __declspec(uuid("{D34B7EE4-EDA0-4cbc-A4A9-A0786EDB8147}")) i_mbv_socket_context:i_mbv_context{};


struct __declspec(uuid("{CAFB0CEA-E72E-40ea-9C70-2E4218A60DE3}")) i_mbv_context_lock:i_mbv_context
{
	typedef long long int64_t;

	struct locker_t
	{
		locker_t(i_mbv_context_lock * _p=0):p(_p)
		{
			p&&(p->Lock());
		}
		~locker_t()
		{
			p&&(p->Unlock());
		}
		inline i_mbv_context_lock * attach(i_mbv_context_lock * _p)
		{
              detach();
			  (p=_p)&&(p->Lock());
			  return p;

		}
		inline i_mbv_context_lock * detach(bool funlock=true)
		{
			i_mbv_context_lock * t=p;

			  p=0;
             funlock&&t&&(t->Unlock());
			 return t;
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
	virtual HRESULT STDMETHODCALLTYPE GetElementInfo(VARTYPE* pvt,int64_t* pCount=0,long* pElementSize=0)=0;

};

 


struct  __declspec(uuid("{D3AD8B42-5DDD-445c-9D30-6B7F2CCB50E4}")) i_mbv_buffer_ptr:i_mbv_context_lock
{		
	
 	virtual HRESULT STDMETHODCALLTYPE GetPtr(void** pptr,int64_t* pByteLength=0)=0;
	virtual HRESULT STDMETHODCALLTYPE DataViewAs(BSTR type,BOOL link,int64_t ByteOffset,int64_t ByteLength,REFIID iid,void** ppObj)=0;
	//virtual HRESULT STDMETHODCALLTYPE GetRegionPtr(int64_t pByteOffset,int64_t pByteLength,void** pptr)=0;
	//virtual HRESULT STDMETHODCALLTYPE Lock()=0;
	//virtual HRESULT STDMETHODCALLTYPE Unlock()=0;
	
};






struct  __declspec(uuid("{B1132551-BFB3-4287-8452-07A639E8C9E4}")) i_mbv_region_ptr:i_mbv_context_lock
{		
	virtual HRESULT STDMETHODCALLTYPE GetRegionPtr(int64_t ByteOffset,int64_t ByteLength,void** pptr)=0;
	virtual HRESULT STDMETHODCALLTYPE GetTotalSize(int64_t* pByteLength=0)=0;
};


 


 
struct  __declspec(uuid("{C6D197F2-609B-4514-8315-BF6810CA6C1C}")) i_mm_region_ptr:i_mbv_buffer_ptr // i_mbv_context_lock //<- future 
{		
	typedef long long int64_t;

	template<class T=char>
	struct ptr_t
	{
		
		ptr_t(i_mm_region_ptr * _p,int64_t _ByteOffset=0,int64_t _ByteLength=sizeof(T))
			:p(_p),ByteLength(_ByteLength),ptr(0),hr(E_POINTER)
		{
			
			if(!p) return;

			if(_ByteOffset<0)
			{
				if(FAILED(hr=p->GetTotalSize(&ByteLength)))
					return;
				_ByteOffset=0;
			}
			
			hr= p->CommitRegionPtr(_ByteOffset,ByteLength,(void**)&ptr);

		}

		~ptr_t()
		{
			(hr)||(p->DecommitRegionPtr(ptr));
		}

		inline operator T*(){ 
			return ptr;
		}
		inline T* operator ->(){ 
			return ptr;
		}

		inline operator HRESULT(){
			return hr;
		}


		i_mm_region_ptr * p;
		int64_t ByteLength;
		int64_t capacity;
		T* ptr;
		
		HRESULT hr;
	};	


	virtual HRESULT STDMETHODCALLTYPE CommitRegionPtr(int64_t ByteOffset,int64_t ByteLength,void** pptr)=0;
	virtual HRESULT STDMETHODCALLTYPE DecommitRegionPtr(void* ptr)=0;
	virtual HRESULT STDMETHODCALLTYPE GetTotalSize(int64_t* pByteLength=0)=0;
	//virtual HRESULT STDMETHODCALLTYPE ViewAs(const wchar_t* tname, int flink,int64_t byte_offset,int64_t byte_length,REFIID iid,void** ppObj)=0;
};


struct  __declspec(uuid("{53089144-8CAD-4ccf-9969-4A54F904F078}")) i_mm_cache_buffer_ptr:i_mm_region_ptr
{

	virtual HRESULT STDMETHODCALLTYPE GetPtrOfIndex(int64_t index,void ** ppvalue)=0;

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
	IDispatch* pthis;
	disp_id_t tss_id;	
}  callback_context_arguments_t,*pcallback_context_arguments_t;


struct i_marshal_helper_t
{   
   virtual  HRESULT marshal_hko(IStream* pStm,HANDLE hko=0)=0;
   virtual  HRESULT marshal_interface(IStream* pStm,REFIID riid,IUnknown* pObj)=0;
   virtual  HRESULT unmarshal_hko(IStream* pStm,HANDLE* phko=0)=0;
   virtual  HRESULT unmarshal_and_release(IStream *pStm,REFIID riid,void** ppObj)=0;
   virtual  HRESULT marshal_BSTR(BSTR bstr,IStream* pStm)=0;
   virtual  HRESULT unmarshal_BSTR(IStream* pStm,BSTR* pbstr)=0;
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
	HRESULT (*SetAggregatorHelpers)(void *context,iaggregator_container* pcontainer,pcallback_lib_arguments_t plib ) ;
	

};

struct  __declspec(uuid("{46EC035A-B7B3-4af6-811C-F41DC346A25E}")) IMarshalByValueDispatchRegister:IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE Register(LPOLESTR alias,MarshalCallbackData_t* data)=0;

};

