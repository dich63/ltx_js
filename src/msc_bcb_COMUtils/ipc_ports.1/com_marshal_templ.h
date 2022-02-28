#pragma once
//com_marshal_templ.h
#include "ipc_ports/class_factory_factory.h"
#include "ipc_ports/istream_pipe.h"

namespace ipc_utils {
	namespace com_apartments
	{



		template <class T>
		struct IPersistStream_base_aggregator_t:base_aggregator_t<T,IPersistStream>
		{


			inline  HRESULT  impl_GetClassID( CLSID *pClassID){  return E_NOTIMPL; 	}
			inline  HRESULT  impl_IsDirty(){ 	return S_OK;	};

			inline  HRESULT  impl_Load(  IStream *pStm) { return E_NOTIMPL; };

			inline  HRESULT   impl_Save(  IStream *pStm, BOOL fClearDirty)  { return E_NOTIMPL; };

			inline  HRESULT   impl_GetSizeMax( ULARGE_INTEGER *pcbSize) { 
				pcbSize->QuadPart=0;
				return S_OK;
			};

			// =====
			virtual HRESULT STDMETHODCALLTYPE GetClassID( CLSID *pClassID) { 	 return (pClassID)?static_cast<T*>(this)->impl_GetClassID(pClassID):E_POINTER; };

			virtual HRESULT STDMETHODCALLTYPE IsDirty( void){ 	 return static_cast<T*>(this)->impl_IsDirty(); };

			virtual HRESULT STDMETHODCALLTYPE Load(IStream *pStm) {      return static_cast<T*>(this)->impl_Load(pStm); 	};

			virtual HRESULT STDMETHODCALLTYPE Save(IStream *pStm, BOOL fClearDirty) {  return static_cast<T*>(this)->impl_Save(pStm,fClearDirty); 	};

			virtual HRESULT STDMETHODCALLTYPE GetSizeMax(ULARGE_INTEGER *pcbSize) {  return (pcbSize)?static_cast<T*>(this)->impl_GetSizeMax(pcbSize):E_POINTER;};

		};




		template <class T>
		struct persist_stream_aggregator_t:IPersistStream_base_aggregator_t<persist_stream_aggregator_t<T> >
		{
			inline  HRESULT  impl_GetClassID( CLSID *pClassID){
				//*pClassID=owner->get_clsid();
				//return S_OK;
				return owner->GetClassID(pClassID);

			}

			static HRESULT rewind( IStream *pStm)
			{
				LARGE_INTEGER l0={};
				return pStm->Seek(l0,STREAM_SEEK_SET,(ULARGE_INTEGER*)&l0);
			}

			inline  HRESULT  impl_Load(  IStream *pStm) {
				HRESULT hr;
				hr=owner->unmarshal(pStm);  
				return hr; 
			};

			inline  HRESULT   impl_Save(  IStream *pStm, BOOL fClearDirty)  {
				HRESULT hr;
				DWORD pid_target,l;

				OLE_CHECK_hr(rewind(pStm));
				OLE_CHECK_hr(pStm->Read(&pid_target,sizeof(DWORD),&l));
				OLE_CHECK_hr(rewind(pStm));
				OLE_CHECK_hr(owner->marshal_to_process(pid_target,pStm));  
				OLE_CHECK_hr(rewind(pStm));

				return hr; 
			};

			//persist_stream_t(T* _owner=0):owner( _owner){}

			T* owner;

		};

     	template <class C,class T=C>
       struct  callback_class_factory_base_t:class_factory_base_aggregator_t< C >
	   {
		   inline static HRESULT  invoke_callback(IDispatch* pdisp,IStream** ppStream,DWORD pidTarget=GetCurrentProcessId())
		   {
			   HRESULT hr;
			   VARIANT v={VT_I4},r={};
			   v.intVal=pidTarget;
			   DISPPARAMS dps={&v,0,1,0};
			   OLE_CHECK_hr( pdisp->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_METHOD,&dps,&r,0,0));
			   if(r.vt!=VT_UNKNOWN) return E_NOTIMPL;
			   hr=r.punkVal->QueryInterface(__uuidof(IStream),(void**)ppStream);
			   VariantClear(&r);
			   return hr;
		   };



	   };

		template <class T,DWORD mcxmask=0 >
		//
		struct  callback_class_factory_t:callback_class_factory_base_t<callback_class_factory_t<T,mcxmask> ,T>
	//	struct  callback_class_factory_t:class_factory_base_aggregator_t<callback_class_factory_t<T,mcxmask> >
		{

			/*
			inline static HRESULT  invoke_callback(IDispatch* pdisp,IStream** ppStream,DWORD pidTarget=GetCurrentProcessId())
			{
				HRESULT hr;
				VARIANT v={VT_I4},r={};
				v.intVal=pidTarget;
				DISPPARAMS dps={&v,0,1,0};
				OLE_CHECK_hr( pdisp->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_METHOD,&dps,&r,0,0));
				if(r.vt!=VT_UNKNOWN) return E_NOTIMPL;
				hr=r.punkVal->QueryInterface(__uuidof(IStream),(void**)ppStream);
				VariantClear(&r);
				return hr;
			};
			*/


			inline HRESULT impl_CreateInstanceEx(	IUnknown * pUnkOuter,LPOLESTR pszDisplayName,IStream* stream,REFIID riid,void ** ppvObject,IBindCtx * pbc)	
			{
				HRESULT hr;
				//moniker_parser_t<wchar_t> mp(pszDisplayName);
				//hr=CreateTest(pszDisplayName,stream,riid, ppvObject);	

				if(!stream) return hr=T::CreateInstance(pszDisplayName,riid,ppvObject,mcxmask);
				smart_ptr_t<IDispatch> callback;
				smart_ptr_t<IStream> callback_stream;

				LARGE_INTEGER pos,pos2;
				//OLE_CHECK_hr(hr=stream->Seek(LARGE_INTEGER(),STREAM_SEEK_CUR,PULARGE_INTEGER(&pos)));

				OLE_CHECK_hr(CoUnmarshalInterface(stream,__uuidof(IDispatch),callback._ppQI()));
				//hr=stream->Seek(LARGE_INTEGER(),STREAM_SEEK_CUR,PULARGE_INTEGER(&pos2));
				//OLE_CHECK_hr(hr=stream->Seek(pos,STREAM_SEEK_SET,&ULARGE_INTEGER()));
			//	hr=CoReleaseMarshalData(stream);

				OLE_CHECK_hr(invoke_callback(callback,&callback_stream.p));

				smart_ptr_t<IPersistStream> ps;
				
				OLE_CHECK_hr(T::CreateInstance(pszDisplayName,__uuidof(IPersistStream),ps._ppQI(),mcxmask));
				OLE_CHECK_hr(ps->Load(callback_stream));
				hr=ps->QueryInterface(riid,ppvObject);		
				return hr;

			}
		   
			inline HRESULT GetClassID(CLSID* clsid)
			{
				 return T::creator_t::GetClassID(&clsid);
			}

			
			inline static CLSID GetCLSID(){
				//return T::creator_t::get_clsid();
				  CLSID clsid={};
                    T::creator_t::GetClassID(&clsid);
					return clsid;
			};	
			
		};


		template<DWORD _contextmask=0>
		struct marshal_callback_t:marshal_base_aggregator_t<marshal_callback_t<_contextmask> >
		{
			//typedef register_class_map_t<>::RefCounter_t fRefCounter_t ;

			template <class T,class _RefCounter>
			struct CallbackDispatch_t:IDispatch_base_t<CallbackDispatch_t<T,_RefCounter>,IDispatch,_RefCounter>
			{
				CallbackDispatch_t(T* powner=0,DWORD _dwDestContext=0):owner(powner),dwDestContext(_dwDestContext),pcontext(0){}
				inline	HRESULT impl_Invoke(DISPID did,REFIID,LCID,WORD,DISPPARAMS*dps,VARIANT* res,EXCEPINFO*, UINT*){
					HRESULT hr;

					ISequentialStream* pInnerStream=0;

					if((did==0)&&(dps->cArgs==1))
					{
						OLE_CHECK_hr(owner->callback_marshal(dps->rgvarg->uintVal,&pInnerStream,&pcontext));
						V_VT(res)=VT_UNKNOWN;
						res->punkVal=pInnerStream;
						return hr;
					}

					return E_INVALIDARG;
				}

				~CallbackDispatch_t(){
					///if(owner)				owner->koxMarshalFinally(pcontext);
				}

				void * pcontext;   
				ipc_utils::smart_ptr_t<T> owner;
				DWORD dwDestContext;
			};


			inline  HRESULT callback_marshal(DWORD pidTarget,ISequentialStream** ppkoxStream,void** pcontext)  { 

				HRESULT hr;
				ULONG l;

				smart_ptr_t<IPersistStream> ps;

				OLE_CHECK_hr(GetUnknown()->QueryInterface(__uuidof(IPersistStream),ps._ppQI()));
				smart_ptr_t<IStream> stream;
				OLE_CHECK_hr(IStreamAggregator_Impl<1>::CreateInstanceForMarshal(HGSTREAM,GetUnknown(),&stream.p));
				OLE_CHECK_hr(stream->Write(&pidTarget,sizeof(pidTarget),&l));
				OLE_CHECK_hr(ps->Save(stream,true));
				*ppkoxStream=stream.detach();
				return hr=S_OK; 
			}



			inline HRESULT IMarshal_GetMarshalSizeMax(  REFIID riid, void *pv,DWORD dwDestContext, void *pvDestContext, DWORD mshlflags,DWORD *pSize){
                     
				   OLE_CHECK_PTR(pSize);
				   /*
				    CallbackDispatch_t<marshal_callback_t,RefCounterFake_t<777> > blank_disp;
					get_custom_marshaller_size(false,riid,&blank_disp,dwDestContext,pvDestContext,mshlflags,pSize);
					*pSize+=1024;
					*/
				   *pSize=1024;
					return S_OK;

			};




			inline  HRESULT IMarshal_GetUnmarshalClass( REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags, CLSID *pCid){		
				*pCid=__uuidof(CLSID_marshal_factory_t);
				return S_OK;

			};

			inline bool check_context( DWORD dwDestContext)
			{
				DWORD l=1<<dwDestContext;
				return l&(ctx_mask&0x0000FFFF);
			}

			inline HRESULT IMarshal_MarshalInterface( IStream *pStm, REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags) {

				ULONG l;
				HRESULT hr;			   
				//if(dwDestContext==MSHCTX_DIFFERENTMACHINE) return E_NOTIMPL;
				//+++++++++++++++++++++++++            
				//return 0;
				smart_ptr_t<IUnknown> unk1,unk2;
				hr=QueryInterface(__uuidof(IUnknown),unk1._ppQI());
				if(pv)
					hr=((IUnknown*)pv)->QueryInterface(__uuidof(IUnknown),unk2._ppQI());


				v_buf_fix<char,2*512> buf;
				marshal_header_base_t* pmh=(marshal_header_base_t*)buf.get();


				if(check_context(dwDestContext))
				{
					smart_ptr_t<IMarshal> m;
					//(REFIID riid,void *pv,DWORD dwDestCtx, void *pvDestCtx,DWORD mshlflags,IMarshal** ppm,marshal_header_base_t* pmh=0)
					OLE_CHECK_hr(get_custom_marshaller(ctx_mask&0xFFFF0000,riid,GetUnknown(),dwDestContext,pvDestContext,mshlflags,&m.p,pmh));
					OLE_CHECK_hr(pStm->Write(pmh,pmh->size,&l))
						return hr = m->MarshalInterface(pStm,riid,pv,dwDestContext,pvDestContext,mshlflags);
				}

				//+++++++++++++++++++++++++            
				//				return 0;

				//
				new (pmh) marshal_header_t<CLSID_bind_marshal_t>();
				//				pmh=new  marshal_header_t<CLSID_bind_marshal_t>();
				pmh->flags=ctx_mask;
				
				smart_ptr_t<IPersistStream> ps;
				OLE_CHECK_hr(QueryInterface(__uuidof(IPersistStream),ps._ppQI()));

				clsid_t clsid;
				OLE_CHECK_hr(ps->GetClassID(&clsid.clsid));

                if(0)
				{

				
				bstr_t bcl=clsid.parse_string(L"__unmarshal__=-111");			 			   
				safe_copy(pmh->szz,(wchar_t*)bcl,bcl.length());
				pmh->size+=bcl.length()*sizeof(wchar_t);		   
				}
				else
				{
					///*
					v_buf<wchar_t> buf;
					buf<<(wchar_t*)bstr_t(clsid)<<L":__unmarshal__=-111";
					int cb=buf.size_b();
					wchar_t* p=buf;
					safe_copy(pmh->szz,p,cb);
					pmh->size+=cb;
					//*/

					//pmh->size+=350;

				}

				OLE_CHECK_hr(pStm->Write(pmh,pmh->size,&l));  


				//++++++
				//return 0;


				//
				smart_ptr_t<IDispatch> cd(new CallbackDispatch_t<marshal_callback_t,RefCounterMT_t<1> >(this,dwDestContext),false);
				//smart_ptr_t<IDispatch> cd(new IDispatch_empty_t,false);

				//
				//++++++
				//hr=0;
			//				hr=CoMarshalInterface(pStm,riid,cd.p,dwDestContext,0,mshlflags);
				
				//
				hr=CoMarshalInterface(pStm,riid,cd.p,MSHCTX_LOCAL ,0,MSHLFLAGS_TABLESTRONG);

				return hr;


			};

			inline HRESULT IMarshal_DisconnectObject(  DWORD) {	
				return S_OK;
			};

			marshal_callback_t(DWORD _ctx_mask=_contextmask):ctx_mask(_ctx_mask){

			};
			DWORD ctx_mask;

		};





		struct creator_class_base_t
		{
			

            template <class T>
			inline static HRESULT CreateInstance(T* p,LPOLESTR pszDisplayName,REFIID riid,void ** ppvObject,DWORD mcxmask=0){
				
				HRESULT hr;
				return hr=p->getInstance(riid,ppvObject,0,mcxmask);
				
			}

		};



		template <const CLSID& _FactoryCLSID>
		struct creator_class_t:creator_class_base_t
		{
			
/*
			inline static const CLSID& get_clsid()  {
				return _FactoryCLSID;
			}
			*/

			inline static HRESULT GetClassID(CLSID* pclsid)  {
				 *pclsid=_FactoryCLSID;
				return S_OK;
			}


		};


       template<class T,class Intf,class _Creator=creator_class_t<__uuidof(T)> >
		struct  base_callsback_marshal_t
		{


            #pragma pack(push)
			#pragma pack(1)
			struct HANDLE64_t
			{
				union{
					struct{ HANDLE h_target_ko;};
					struct{ ULONG64 __dummy__1;};
				};

				union{
					struct{ HANDLE h_ko;};
					struct{ ULONG64 __dummy__2;};
				};
				 DWORD pid_source;
				 DWORD size;
				 DWORD attrib;
				
			};
			#pragma pack(pop)

			inline static HRESULT marshal_hko_to_process(HANDLE hko,DWORD pid_target,IStream* pStm)
			{
				HRESULT hr,err;
				bool fsock;
				WSAPROTOCOL_INFOA wsainfo;
				ULONG l;
				process_handle_t target=pid_target;
				HANDLE ht=target;
				//OLE_CHECK_hr(target.hr);
				HANDLE64_t hsource_to_target={};
				hsource_to_target.h_target_ko=dup_handle(hko,ht);
				hr=GetLastError();
				hsource_to_target.h_ko=hko;
				hsource_to_target.pid_source=GetCurrentProcessId();				 
				hsource_to_target.size=sizeof(hsource_to_target);
				if(fsock=check_socket(hko))
				{
                 OLE_CHECK_hr( HRESULT_FROM_WIN32(WA_socket_utils::get_info(SOCKET(hko),&wsainfo)) );
				 hsource_to_target.attrib=1;
				 hsource_to_target.size+=sizeof(wsainfo);
				}
				
				
				OLE_CHECK_hr(pStm->Write(&hsource_to_target,sizeof(hsource_to_target),&l));
				if(fsock)
						hr=pStm->Write(&wsainfo,sizeof(wsainfo),&l);				
				
				return hr;

			}

			inline static bool is_valid_handle_value(HANDLE h)
			{
				return (h)&&(h!=INVALID_HANDLE_VALUE);
			}

			inline static HRESULT unmarshal_hko(IStream* pStm,HANDLE* phko=0)
			{
				WSAPROTOCOL_INFOA wsainfo;
				HRESULT hr=S_FALSE;
				ULONG l;
				HANDLE h_um=0;
				HANDLE64_t hsource_to_target={};

				OLE_CHECK_hr(hr=pStm->Read(&hsource_to_target,sizeof(hsource_to_target),&l));
				if(hsource_to_target.h_target_ko)
					h_um=hsource_to_target.h_target_ko;
				else if(is_valid_handle_value(hsource_to_target.h_ko))
				{
					process_handle_t source=hsource_to_target.pid_source;
					HANDLE hs=source;
					OLE_CHECK_hr(source.hr);
					h_um=dup_handle(hsource_to_target.h_ko,hs);
					if(!h_um) return HRESULT_FROM_WIN32(GetLastError());

				}
				if(hsource_to_target.h_ko)
				{
					DWORD l;
					bool f=GetHandleInformation(h_um,&l);
					if(!f) return HRESULT_FROM_WIN32(GetLastError());

				}

				if((hsource_to_target.attrib==1)&&(hsource_to_target.size>sizeof(hsource_to_target)))
				{
                  if(SUCCEEDED(hr=pStm->Read(&wsainfo,sizeof(wsainfo),&l)))
				  {
					  wsainfo.dwProviderReserved=(DWORD_PTR)h_um;
					  SOCKET sock;
					  if(SUCCEEDED(hr=WA_socket_utils::socket(wsainfo,&sock)))
					  {
						  if(phko) *phko=HANDLE(sock);
						  else ::closesocket(sock);
						  return hr;
					  }
				  };

				  if(h_um) CloseHandle(h_um);
				  return hr;

				}

				if(phko) *phko=h_um;
				else if(h_um) CloseHandle(h_um);

				return hr;

			}







        	typedef  typename _Creator creator_t;

			persist_stream_aggregator_t<T> persist_stream;

			//clsid_t clsid;

			//
			inline  CLSID get_clsid()  {

				CLSID c;
				 creator_t::GetClassID(&c);			
				 return c;
			};


			static HRESULT unmarshal_and_release(IStream *pStm,REFIID riid,void** ppObj)
			{
				OLE_CHECK_PTR(ppObj);	
				HRESULT hr;
				LARGE_INTEGER pos;
                				
				if(SUCCEEDED(hr=pStm->Seek(LARGE_INTEGER(),STREAM_SEEK_CUR,PULARGE_INTEGER(&pos))))
					if(SUCCEEDED( hr=CoUnmarshalInterface(pStm,__uuidof(IDispatch),ppObj)))
						if(SUCCEEDED(hr=pStm->Seek(pos,STREAM_SEEK_SET,&ULARGE_INTEGER())))
							SUCCEEDED(hr=CoReleaseMarshalData(pStm));
				return hr;

			}

			template <class I>  
			static HRESULT unmarshal_and_release(IStream *pStm,I** ppObj)
			{
				return unmarshal_and_release(pStm,__uuidof(I),(void**)ppObj);
			}


			inline HRESULT marshal_to_process(DWORD pid_target,IStream* pStm)
			{				
				return S_OK;
			};  

			inline HRESULT unmarshal(IStream* pStm)
			{			
				return S_OK;
			};  	

			base_callsback_marshal_t()
			{
                persist_stream.owner=static_cast<T*>(this);
			}


			template<class I>
			inline HRESULT getInstance(I ** ppI,DWORD mcxmask=0)
			{
				return getInstance(__uuidof(I),(void**)ppI,mcxmask);
			}

			inline HRESULT getInstance(REFIID riid,void ** ppvObject,IUnknown* p_aggregator_container=0,DWORD mcxmask=0)
			{
				struct M:aggregator_helper_base_t<M>
				{

					inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ){

						return marshal(Outer,riid,ppObj,hr);
					}
					M(DWORD m):marshal(m){}

					marshal_callback_t<> marshal;
				};

				struct O:aggregator_helper_base_t<O>
				{

					inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ){

						return ((*o)(Outer,riid,ppObj,hr))||(o->persist_stream(Outer,riid,ppObj,hr));
					}
					O(T* _o):o(_o){}

					~O() {
						delete o;
					}

					T* o;
					
				};

				struct P:aggregator_helper_base_t<P>
				{

					inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ){

						return (o->persist_stream(Outer,riid,ppObj,hr));
					}
					P(T* _o):o(_o){}

					T* o;
				};



				HRESULT hr;
				bool freemarshal=mcxmask&0xFFFF0000;

				ipc_utils::smart_ptr_t<M> sM(new M(mcxmask),0);
				ipc_utils::smart_ptr_t<O> sO(new O(static_cast<T*>(this)),0);
				//ipc_utils::smart_ptr_t<P> sP(new P(static_cast<T*>(this)),0);


				aggregator_container_helper container(p_aggregator_container,true);

				if(freemarshal)
					container.add_FMT(true);
                   hr=container.add(__uuidof(IMarshal),sM).add(__uuidof(Intf),sO).add(__uuidof(IPersistStream),sO).QueryInterface(riid,ppvObject);        
				
				return hr;

			}

			inline HRESULT GetClassID(CLSID* pclsid)
			{
				return T::creator_t::GetClassID(pclsid);
			}

			
		};



	};//com_apartments
};//ipc_utils

