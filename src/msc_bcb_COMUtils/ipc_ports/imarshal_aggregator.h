#pragma once
// imarshal_aggregator.h

#include "dummy_dispatch_aggregator.h"
//#include "ipc_ports/com_script_engine.h"

//struct ISequentialStreamPipe_Impl:base_component_t<ISequentialStreamPipe_Impl<PIPESIZE>,ISequentialStream>

namespace ipc_utils {
	namespace com_apartments
	{

		// 
		// 

		struct  __declspec(uuid("{5582D639-39B2-4015-85AD-632D3976B04C}")) IClassFactoryStream:IClassFactory{

			virtual HRESULT __stdcall CreateInstanceEx(	IUnknown * pUnkOuter,LPOLESTR parse_string,IStream* stream ,REFIID riid,void ** ppvObject,IBindCtx * pbc)=0;	
			virtual HRESULT __stdcall SetMarshalContext(IUnknown * pContext)=0;

		};

		struct  __declspec(uuid("{E6B5F53D-3677-44f5-82AB-B561DFC6DD28}")) CLSID_std_marshal_t{};
		struct  __declspec(uuid("{AE7EC621-8169-4d7d-9EDE-B1CEE0ED8F8E}")) CLSID_free_marshal_t{};
		struct  __declspec(uuid("{9346008F-021A-4284-86D0-541C09D8C26E}")) CLSID_bind_marshal_t{};

#pragma pack(push)
#pragma pack(1)

		struct marshal_header_base_t
		{
			short size;
			CLSID clsid;
			DWORD flags;
			wchar_t szz[2];
			inline bool is_std()
			{
				return clsid==__uuidof(CLSID_std_marshal_t);
			}
			inline bool is_free()
			{
				return clsid==__uuidof(CLSID_free_marshal_t);
			}
			inline bool is_bind()
			{
                return clsid==__uuidof(CLSID_bind_marshal_t);
			}

		};
#pragma pack(pop)
		template<class MClass>
		struct marshal_header_t:marshal_header_base_t
		{
			
			marshal_header_t(const CLSID& c=__uuidof(MClass)){
				size=sizeof(*this);
				clsid=c;
				flags=0;
				szz[0]=0;
				szz[1]=0;
			}
			marshal_header_t(int _size,const CLSID& c=__uuidof(MClass)){
				size=_size;
				clsid=c;				
			}
			
		};
		






		template <class T>
		struct class_factory_base_aggregator_t:base_aggregator_t<T,IClassFactoryStream>
		{


			struct parse_display_name_t:base_aggregator_t< parse_display_name_t ,IParseDisplayName>
			{
				virtual HRESULT __stdcall ParseDisplayName(
					IBindCtx * pbc,
					LPOLESTR pszDisplayName, 
					ULONG * pchEaten,
					IMoniker ** ppmkOut
					){
						
						HRESULT hr;
						/*
						ipc_utils::smart_ptr_t<IUnknown> unk;
						if(SUCCEEDED(hr=static_cast<T*>(owner)->impl_ParseDisplayName(pbc,pszDisplayName,pchEaten,&unk.p)))
							if(SUCCEEDED(hr=::CreatePointerMoniker(unk,ppmkOut))&&pchEaten)
								*pchEaten=SysStringLen(pszDisplayName);
								*/
						hr=static_cast<T*>(owner)->impl_ParseDisplayNameToMoniker( pbc, pszDisplayName, pchEaten,ppmkOut);
						if(SUCCEEDED(hr)&&pchEaten)
							*pchEaten=moniker_parser_t<wchar_t>::parse_str_len(pszDisplayName);

						return hr;
				};

				class_factory_base_aggregator_t* owner;

			} parse_display_name;


			inline HRESULT impl_ParseDisplayNameToMoniker(IBindCtx * pbc,LPOLESTR pszDisplayName,ULONG * pchEaten,IMoniker ** ppmkOut)	{

   			     HRESULT hr;
 				 ipc_utils::smart_ptr_t<IUnknown> unk;
				 if(SUCCEEDED(hr=static_cast<T*>(this)->impl_ParseDisplayName(pbc,pszDisplayName,pchEaten,&unk.p)))
					if(SUCCEEDED(hr=::CreatePointerMoniker(unk,ppmkOut))&&pchEaten)
						*pchEaten=moniker_parser_t<wchar_t>::parse_str_len(pszDisplayName);
				 return hr;

			}

			class_factory_base_aggregator_t()
			{
				parse_display_name.owner=this;

			}

			virtual HRESULT __stdcall CreateInstanceEx(	IUnknown * pUnkOuter,LPOLESTR parse_string,IStream* stream ,REFIID riid,void ** ppvObject,IBindCtx * pbc)
			{
                      return static_cast<T*>(this)->impl_CreateInstanceEx(pUnkOuter,parse_string,stream,riid,ppvObject,pbc);
			}

			virtual HRESULT __stdcall SetMarshalContext(IUnknown * pContext){
				return static_cast<T*>(this)->impl_SetMarshalContext(pContext);
			};

			inline HRESULT impl_SetMarshalContext(IUnknown * pContext){
                 return E_NOTIMPL;
			}


			inline HRESULT impl_ParseDisplayName(IBindCtx * pbc,LPOLESTR pszDisplayName,ULONG * pchEaten,IUnknown ** ppUnk)	{

				long rct;
				HRESULT hr;
				hr=CreateInstanceEx(0,pszDisplayName,0,__uuidof(IUnknown),(void**)ppUnk,pbc);
				if(0&&SUCCEEDED(hr))
				{
					rct=ref_count(*ppUnk);
					OutputDebugStringW(v_buf<wchar_t>().printf(L"RefCount=%d\n",rct).get());
				}

				return hr;
					//					static_cast<T*>(this)->impl_CreateInstance(0,__uuidof(IUnknown),(void**)ppUnk);
			}

			inline HRESULT impl_CreateInstance(	IUnknown * pUnkOuter,REFIID riid,void ** ppvObject,void* params=0)	{
				return CreateInstanceEx(pUnkOuter,0,0,riid,ppvObject,0); 
			}
			inline HRESULT impl_CreateInstanceEx(	IUnknown * pUnkOuter,LPOLESTR parse_string,IStream* stream,REFIID riid,void ** ppvObject,IBindCtx * pbc=0)	{return E_NOINTERFACE ;}
			inline HRESULT impl_LockServer(BOOL fLock)	{return S_OK ;}



			inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ) 
			{ 
				return   static_cast<T*>(this)->inner_QI2(Outer,riid,__uuidof(IClassFactory),ppObj,hr)
					   ||static_cast<T*>(this)->inner_QI2(Outer,riid,__uuidof(IClassFactoryStream),ppObj,hr)
					   ||parse_display_name.inner_QI(Outer,riid,ppObj,hr);
			}

			virtual HRESULT __stdcall CreateInstance(	IUnknown * pUnkOuter,REFIID riid,void ** ppvObject)
			{
				return static_cast<T*>(this)->impl_CreateInstance(pUnkOuter,riid,ppvObject);
			};
			virtual	HRESULT __stdcall LockServer( BOOL fLock  ){
				return static_cast<T*>(this)->impl_LockServer(fLock);
			};



			inline HRESULT GetFactory(REFIID riid,void** ppcf){


				if(this==(void*)0) return E_OUTOFMEMORY;

				HRESULT hr;
				if(!ppcf) return E_POINTER;
				aggregator_container_helper container;

			   ipc_utils::smart_ptr_t<IUnknown> sF=aggregator_helper_creator_t<class_factory_base_aggregator_t>::New(this);

			   /*
				OLE_CHECK_hr(container.set_aggregator(__uuidof(IClassFactoryStream),__uuidof(IClassFactory),__uuidof(IParseDisplayName),sF));
				OLE_CHECK_hr(container.set_FTM());				
               hr=container.QueryInterface(riid,ppcf);     
				*/					
				
				return hr=container.add(__uuidof(IClassFactoryStream),__uuidof(IClassFactory),__uuidof(IParseDisplayName),sF).add_FMT().QueryInterface(riid,ppcf);
			};

			template<class I>
			inline HRESULT GetFactory(I** ppcf)
			{
				return static_cast<T*>(this)->GetFactory(__uuidof(I),(void**)ppcf);
			};

			static HRESULT __stdcall GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
			{
				return (new T())->GetFactory(riid,ppv);
			}

			static HRESULT __stdcall GetObjectEx(IUnknown* pUnkOuter,LPOLESTR parse_string,IStream* pStrm ,REFIID riid, LPVOID* ppv)
			{
				HRESULT hr;
				smart_ptr_t<IClassFactoryStream> cfs;
				OLE_CHECK_hr(GetClassObject(__uuidof(IClassFactoryStream),__uuidof(IClassFactoryStream),cfs._ppQI()));
				hr=cfs->CreateInstanceEx(pUnkOuter,parse_string,pStrm,riid,ppv);
				return hr;
			}

			inline static CLSID GetCLSID(){ return __uuidof(T) ;};	

		};

		struct  __declspec(uuid("{46677618-35AF-4af1-99E5-88DFB892A30E}")) IMarshalStreamList:IMarshal
		{
			virtual HRESULT STDMETHODCALLTYPE PushStream(IStream* stream)=0;
		};


		template <class T>
		struct marshal_base_aggregator_t:base_aggregator_t<T,IMarshal>
		{
			 struct stream_pos_t
			 {
				 LARGE_INTEGER pos;
				 smart_ptr_t<IStream> stream;
			 };
			//IClassFactory
			inline T* this_T(){ return static_cast<T*>(this);};
			//inline HRESULT IMarshal_GetUnmarshalClass()

			inline  HRESULT get_custom_marshaller(bool free_enabled,REFIID riid,void *pv,DWORD dwDestCtx, void *pvDestCtx,DWORD mshlflags,IMarshal** ppm,marshal_header_base_t* pmh=0)
			{
				HRESULT hr;
				if(free_enabled&&(MSHCTX_INPROC==dwDestCtx)&&SUCCEEDED(hr=this->QueryInterface(__uuidof(IFreeThreadMarshal),(void**)ppm)))
				{
					if(pmh) new(pmh) marshal_header_t<CLSID_free_marshal_t>;

				}
				else if(SUCCEEDED(hr=CoGetStandardMarshal (riid,(IUnknown*) pv, dwDestCtx, pvDestCtx, mshlflags, ppm)))
				{
					if(pmh) new(pmh) marshal_header_t<CLSID_std_marshal_t>;
				}					  

				return hr;

			};
			inline  HRESULT get_custom_marshaller_size(bool free_enabled,REFIID riid,void *pv,DWORD dwDestCtx, void *pvDestCtx,DWORD mshlflags,ULONG* pSize)
			{
				HRESULT hr;
				smart_ptr_t<IMarshal> m;
				OLE_CHECK_hr(get_custom_marshaller( free_enabled,riid,pv,dwDestCtx,pvDestCtx,mshlflags,&m.p));
				hr = m->GetMarshalSizeMax(riid,pv, dwDestCtx, pvDestCtx, mshlflags, pSize);
				return hr;
			}


			inline  HRESULT IMarshal_GetUnmarshalClass( 
				/* [in] */ REFIID riid,
				/* [unique][in] */ void *pv,
				/* [in] */ DWORD dwDestContext,
				/* [unique][in] */ void *pvDestContext,
				/* [in] */ DWORD mshlflags,
				/* [out] */ CLSID *pCid){

					return S_FALSE;
			};

			inline HRESULT IMarshal_GetMarshalSizeMax( 
				/* [in] */ REFIID riid,
				/* [unique][in] */ void *pv,
				/* [in] */ DWORD dwDestContext,
				/* [unique][in] */ void *pvDestContext,
				/* [in] */ DWORD mshlflags,
				/* [out] */ DWORD *pSize){

					*pSize=0;
					return S_OK;

			};
			inline HRESULT IMarshal_MarshalInterface( 
				/* [unique][in] */ IStream *pStm,
				/* [in] */ REFIID riid,
				/* [unique][in] */ void *pv,
				/* [in] */ DWORD dwDestContext,
				/* [unique][in] */ void *pvDestContext,
				/* [in] */ DWORD mshlflags) {


					return E_NOINTERFACE ;

			};
			inline HRESULT IMarshal_UnmarshalInterface( 
				/* [unique][in] */ IStream *pStm,
				/* [in] */ REFIID riid,
				/* [out] */ void **ppv) {

					return E_NOINTERFACE;
			};
			inline HRESULT IMarshal_ReleaseMarshalData( IStream *pStm) {

					return S_OK;

			};
			inline HRESULT IMarshal_DisconnectObject(  DWORD dwReserved) {

				return CoDisconnectObject(this_T(),dwReserved);

			};
			inline HRESULT  IMarshalStreamList_PushStream(IStream *pStm) {
				return S_OK;
			};

			//=================================
			virtual HRESULT STDMETHODCALLTYPE GetUnmarshalClass( 
				/* [in] */ REFIID riid,
				/* [unique][in] */ void *pv,
				/* [in] */ DWORD dwDestContext,
				/* [unique][in] */ void *pvDestContext,
				/* [in] */ DWORD mshlflags,
				/* [out] */ CLSID *pCid) {
					return static_cast<T*>(this)->IMarshal_GetUnmarshalClass(riid,pv,dwDestContext,pvDestContext,mshlflags,pCid);
			};
			virtual HRESULT STDMETHODCALLTYPE GetMarshalSizeMax( 
				/* [in] */ REFIID riid,
				/* [unique][in] */ void *pv,
				/* [in] */ DWORD dwDestContext,
				/* [unique][in] */ void *pvDestContext,
				/* [in] */ DWORD mshlflags,
				/* [out] */ DWORD *pSize){
                   HRESULT hr;
					 hr=static_cast<T*>(this)->IMarshal_GetMarshalSizeMax(riid,pv,dwDestContext,pvDestContext,mshlflags,pSize);
					return hr;
			};
			virtual HRESULT STDMETHODCALLTYPE MarshalInterface( 
				/* [unique][in] */ IStream *pStm,
				/* [in] */ REFIID riid,
				/* [unique][in] */ void *pv,
				/* [in] */ DWORD dwDestContext,
				/* [unique][in] */ void *pvDestContext,
				/* [in] */ DWORD mshlflags){

					return static_cast<T*>(this)->IMarshal_MarshalInterface(pStm,riid,pv,dwDestContext,pvDestContext,mshlflags);
			};
			virtual HRESULT STDMETHODCALLTYPE UnmarshalInterface( 
				/* [unique][in] */ IStream *pStm,
				/* [in] */ REFIID riid,
				/* [out] */ void **ppv) {

					HRESULT hr;
                     //OLE_CHECK_hr(static_cast<T*>(this)->IMarshal_UnmarshalInterface(pStm,riid,ppv));
					 //((IUnknown*)(*ppv))->AddRef();

					return hr=static_cast<T*>(this)->IMarshal_UnmarshalInterface(pStm,riid,ppv);
			};
			virtual HRESULT STDMETHODCALLTYPE ReleaseMarshalData( 
				/* [unique][in] */ IStream *pStm) {
					return static_cast<T*>(this)->IMarshal_ReleaseMarshalData(pStm);
			};
			virtual HRESULT STDMETHODCALLTYPE DisconnectObject( 
				/* [in] */ DWORD dwReserved) {
					return static_cast<T*>(this)->IMarshal_DisconnectObject(dwReserved);
			};
			virtual HRESULT STDMETHODCALLTYPE PushStream(IStream* pStm)
			{
				return static_cast<T*>(this)->IMarshalStreamList_PushStream(pStm);
			};

			//std::list<stream_pos_t> stream_pos_list;
			//mutex_cs_t mutex;


		};


		//
		/*

		struct  __declspec(uuid("{F6C19774-7905-42df-88B4-2A6CE02E2C40}")) i_kox_marshaler_t:IUnknown{
			virtual	    HRESULT __stdcall MarshalToTarget(DWORD pidTarget,ISequentialStream** ppkoxStream)=0;
			virtual		HRESULT __stdcall Unmarshal(ISequentialStream* pkoxStream, REFIID riid,IUnknown* punk)=0;
		};

		// 





		template <class T>
		struct marshal_kox_aggregator_t:marshal_base_aggregator_t<T> {


			struct CallbackDispatch_t:IDispatch_base_t<CallbackDispatch_t>
			{
				CallbackDispatch_t(T* powner=0,DWORD _dwDestContext=0):owner(powner),dwDestContext(_dwDestContext),pcontext(0){}
				inline	HRESULT impl_Invoke(DISPID did,REFIID,LCID,WORD,DISPPARAMS*dps,VARIANT* res,EXCEPINFO*, UINT*){
					HRESULT hr;

					ISequentialStream* pkoxStream=0;

					if((did==0)&&(dps->cArgs==1))
			  {
				  OLE_CHECK_hr(owner->koxMarshal(dps->rgvarg->uintVal,&pkoxStream,&pcontext));
				  V_VT(res)=VT_UNKNOWN;
				  res->punkVal=pkoxStream;
				  return hr;
			  }

					return E_INVALIDARG;
				}

				~CallbackDispatch_t(){
					if(owner)
						owner->koxMarshalFinally(pcontext);
				}

				void * pcontext;   
				ipc_utils::smart_ptr_t<T> owner;
				DWORD dwDestContext;
			};

			inline void koxMarshalFinally(void *pcontext) {};
			inline  HRESULT koxMarshal(DWORD pidTarget,ISequentialStream** ppkoxStream,void** pcontext)  { 

				return E_NOTIMPL; 
			}

			inline  HRESULT koxUnmarshal(ISequentialStream* pkoxStream, REFIID riid,IUnknown* punk){  return E_NOTIMPL;  }



			inline IDispatch* getCallbackDispatch(){   return NULL; }

			inline bool check_context(DWORD dwDestContext) {
				return (dwDestContext&&MSHCTX_DIFFERENTMACHINE);
			}

			inline  HRESULT IMarshal_GetUnmarshalClass(REFIID riid,void *pv,DWORD dwDestContext,void *pvDestContext,DWORD mshlflags,CLSID *pCid){

				if(!check_context(dwDestContext))   return S_FALSE;
				*pCid=__uuidof(CLSID_bind_marshal_t);
				return S_OK;
			};
			inline HRESULT IMarshal_GetMarshalSizeMax( REFIID riid,void *pv,DWORD dwDestContext,void *pvDestContext,DWORD mshlflags,DWORD *pSize){

				HRESULT hr;
				CallbackDispatch_t blank;
				if(!check_context(dwDestContext))   return  E_NOINTERFACE;			   
				hr = CoGetMarshalSizeMax(pSize,riid,&blank,dwDestContext,pvDestContext,mshlflags);			
				return hr;


			};
			inline HRESULT IMarshal_MarshalInterface(IStream *pStm,REFIID riid,void *pv,DWORD dwDestContext,void *pvDestContext,DWORD mshlflags) {

				HRESULT hr;
				//IDispatch* pdisp;
				if(!check_context(dwDestContext))   return  E_NOINTERFACE;

				ipc_utils::smart_ptr_t<CallbackDispatch_t> disp(new CallbackDispatch_t(this_T()),false); 		  
				hr=CoMarshalInterface(pStm,riid,disp,dwDestContext,pvDestContext,mshlflags) ;

				return hr;



			};

			inline HRESULT  invoke_callback(IDispatch* pdisp,DWORD pidSrc,ISequentialStream** ppkoxStream)
			{
				HRESULT hr;
				VARIANT v={VT_I4},r={};
				v.intVal=pidSrc;
				DISPPARAMS dps={&v,0,1,0};
				OLE_CHECK_hr( pdisp->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_METHOD,&dps,&r,0,0));
				if(r.vt!=VT_UNKNOWN) return E_NOTIMPL;
				hr=r.punkVal->QueryInterface(__uuidof(ISequentialStream),(void**)ppkoxStream);
				VariantClear(&r);
				return hr;
			};

			inline HRESULT IMarshal_UnmarshalInterface(  IStream *pStm, REFIID riid, void **ppv) {
				HRESULT hr;
				ipc_utils::smart_ptr_t<IDispatch> disp;
				OLE_CHECK_hr(CoUnmarshalInterface(pStm,__uuidof(IDispatch),disp.ppQI()));
				ipc_utils::smart_ptr_t<ISequentialStream> kox_stream;
				OLE_CHECK_hr(invoke_callback(disp,GetCurrentProcessId(),&kox_stream.p));		  			  
				OLE_CHECK_hr(koxUnmarshal(kox_stream,riid,(IUnknown**)ppv));
				return hr;
			};
			inline HRESULT IMarshal_ReleaseMarshalData( IStream *pStm) {

				HRESULT hr;				 
				OLE_CHECK_hr(pStm->Seek(LARGE_INTEGER(),STREAM_SEEK_SET,&ULARGE_INTEGER()));
				OLE_CHECK_hr(CoReleaseMarshalData(pStm));
				return hr;
			};

			
		};
		// */

	};//com_apartments
};//ipc_utils
// */