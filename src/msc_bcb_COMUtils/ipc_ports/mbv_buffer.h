#pragma once

//template <class ArrayType=unsigned char>
#include "dispid_find.h"
#include "variant_resolver.h"
namespace ipc_utils {
	namespace mbv {

		struct  __declspec(uuid("{A6C8F099-C1DD-4a68-82F5-DB7786FBFC3A}")) float64_class_t
		{
			typedef  double i_type_t;
			enum{ 
				vtype=VT_R8
			};
			inline  static i_type_t& ref(VARIANT& v){ return v.dblVal;  }
		};



		struct  __declspec(uuid("{5D6EDFFB-86D5-4447-8FCB-5678D260B70F}")) float32_class_t
		{
			typedef  float i_type_t;
			enum{ 
				vtype=VT_R4
			};
			inline  static i_type_t& ref(VARIANT& v){ return v.fltVal;  }
		};

		struct  __declspec(uuid("{263A5469-77F9-4f4f-AFAB-A5ADED88DFA8}")) int32_class_t
		{
			typedef  int i_type_t;
			enum{ 
				vtype=VT_I4
			};
			inline  static i_type_t& ref(VARIANT& v){ return v.intVal;  }
		};

		struct  __declspec(uuid("{EAE7301E-EB17-42fa-9671-C28889064556}")) uint32_class_t
		{
			typedef  unsigned int i_type_t;
			enum{ 
				vtype=VT_UI4
			};
			inline  static i_type_t& ref(VARIANT& v){ return v.uintVal;  }
		};


		struct  __declspec(uuid("{5885427E-6690-4803-A914-E15E2C73BCDE}")) int16_class_t
		{
			typedef  signed short i_type_t;
			enum{ 
				vtype=VT_I2
			};
			inline  static i_type_t& ref(VARIANT& v){ return v.iVal;  }
		};


		struct  __declspec(uuid("{21CA4B50-9282-4b72-900C-8EACD16A61C6}")) uint16_class_t
		{
			typedef  unsigned short i_type_t;
			enum{ 
				vtype=VT_UI2
			};
			inline  static i_type_t& ref(VARIANT& v){ return v.uiVal;  }
		};




		struct  __declspec(uuid("{F7383622-A9EE-48da-A8FE-104EC90203E4}")) int8_class_t
		{
			typedef   char i_type_t;
			enum{ 
				vtype=VT_I1
			};
			inline  static i_type_t& ref(VARIANT& v){ return v.cVal;  }

		};


		struct  __declspec(uuid("{4BC52286-C147-497c-BFCC-A143BA259B0A}")) uint8_class_t
		{
			typedef  unsigned char i_type_t;
			enum{ 
				vtype=VT_UI1
			};
			inline  static i_type_t& ref(VARIANT& v){ return v.bVal;  }

		};

		inline LONG64	SAFEARRAY_total_length(SAFEARRAY* psa)
		{
			if(psa->cDims<=0) return 0;
			LONG32 e=1;
			for(int n=0;n<psa->cDims;n++)
				e*=psa->rgsabound[n].cElements;
			//e*=m_array.cbElements;
			return e;
		}


		inline LONG64	SAFEARRAY_total_bytelength(SAFEARRAY* psa)
		{

			return (psa->cbElements)*LONG64(SAFEARRAY_total_length(psa));
		}



		//{3C30DDC4-8347-4059-A48A-F93D3B5FC2CB}
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



#pragma pop(pack)
		template <int VERS=0>
		struct shared_mem_t
		{


			//
			typedef mutex_ko_t mutex_t;


			//typedef mutex_cs_t mutex_t;



			shared_mem_t(INT64 size=0):hmap(0),ptr(0),hr(E_FAIL),shdata(shared_mem_data_t()){
				{

				}
			}

			shared_mem_t& init (INT64 _size,INT64 vt=VT_UI1)
			{
				if(_size)
				{
					LARGE_INTEGER l;
					l.QuadPart=_size;
					HANDLE hm;
					hm=CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE|SEC_COMMIT,l.HighPart,l.LowPart,0);
					reset(hm);   
					if(hmap) {
						size()=_size;
						shdata.vt=vt;
						set_element_size();
						//
						mutex.reset(CreateMutex(0,0,0));
					}
				}
				return *this;
			}

			inline int set_element_size()
			{
				return shdata.element_size=VARTYPE_finder_t<>::element_size(shdata.vt);
			}

			void reset(HANDLE h=0)
			{
				HANDLE t;
				void* p=ipc_utils::make_detach(ptr);
				if(t=ipc_utils::make_detach(hmap,h))
				{
					UnmapViewOfFile(p);
					CloseHandle(t);			
				}
				ptr=MapViewOfFile(hmap,FILE_MAP_ALL_ACCESS,0,0,0);

			}
			~shared_mem_t()
			{
				reset();
			}

			inline INT64& size()
			{
				return shdata.sizeb;
			}

			shared_mem_data_t shdata;
			HANDLE hmap;
			mutex_t mutex;

			void* ptr;	
			HRESULT hr;
			//INT64 sizeb;

		};

		template <class _ArrayTypeClass=uint8_class_t>
		struct mbv_buffer_ptr_aggregator_t:base_aggregator_t<mbv_buffer_ptr_aggregator_t<_ArrayTypeClass>,i_mbv_buffer_ptr>
		{
			typedef shared_mem_t<> shared_mem_t;
			typedef i_mbv_buffer_ptr::int64_t int64_t;

			typedef _ArrayTypeClass array_type_class_t;
			typedef typename array_type_class_t::i_type_t value_type_t;


			enum{
				vtype=array_type_class_t::vtype
			};


			mbv_buffer_ptr_aggregator_t(shared_mem_t* _shmem):shmem(_shmem){

			};

			virtual HRESULT STDMETHODCALLTYPE GetContext(void** ppcontext)
			{		
				return E_NOTIMPL;
			}

			virtual HRESULT STDMETHODCALLTYPE GetPtr(void** pptr,int64_t* pByteLength)
			{
				HRESULT hr;
				OLE_CHECK_PTR(pptr);
				*pptr=shmem->ptr;
				pByteLength&&(*pByteLength=shmem->size());		

				return 0;
			};
			virtual HRESULT STDMETHODCALLTYPE Lock()
			{
				HRESULT hr;
				shmem->mutex.lock();

				return 0;
			}
			virtual HRESULT STDMETHODCALLTYPE Unlock()
			{
				HRESULT hr;		
				shmem->mutex.unlock();
				return 0;
			}
			virtual HRESULT STDMETHODCALLTYPE GetCount(int64_t* pcount){
				OLE_CHECK_PTR(pcount);
				*pcount=shmem->size()/sizeof(value_type_t);
				return 0;

			};
			virtual HRESULT STDMETHODCALLTYPE GetElement(int64_t index,VARIANT * pvalue){
				HRESULT hr;

				int64_t  cb=shmem->size()/sizeof(value_type_t);
				if(!((0<=index)&&(index<cb))) return hr=TYPE_E_OUTOFBOUNDS;
				if(pvalue)
				{
					VARIANT v={vtype};
					value_type_t* pf=(value_type_t*)shmem->ptr;

					array_type_class_t::ref(v)=pf[index];
					*pvalue=v;
				}
				return 0;

			};
			virtual HRESULT STDMETHODCALLTYPE SetElement(int64_t index,VARIANT newvalue){
				HRESULT hr;
				int64_t  cb=shmem->size()/sizeof(value_type_t);
				if(!((0<=index)&&(index<cb))) return hr=TYPE_E_OUTOFBOUNDS;
				OLE_CHECK_hr(VariantChangeType(&newvalue,&newvalue,VARIANT_NOVALUEPROP,vtype));
				value_type_t* pf=(value_type_t*)shmem->ptr;
				pf[index]=array_type_class_t::ref(newvalue);
				return hr;
			}
			;

			inline bool operator()(IUnknown* punkOuter,REFIID riid,void** ppObj,HRESULT& hr)
			{
				bool f;     
				f=inner_QI2(punkOuter,riid,__uuidof(i_mbv_buffer_ptr), ppObj,hr);
				f|=inner_QI2(punkOuter,riid,__uuidof(i_mbv_typed_buffer_ptr), ppObj,hr);
				return f;
			}


			shared_mem_t* shmem;
		};


		template <class _ArrayTypeClass=uint8_class_t>
		struct   mbv_buffer_t{

			// MarshalCallbackData_t
			typedef _ArrayTypeClass array_type_class_t;
			typedef typename array_type_class_t::i_type_t value_type_t;
			typedef shared_mem_t<> shared_mem_t;

			enum{
				vtype=array_type_class_t::vtype
			};




			typedef ltx_helper::arguments_t<variant_t> arguments_t;
			static void dbg_print(const wchar_t * fmt,...)
			{	
				va_list argptr;
				va_start(argptr, fmt);
				v_buf<wchar_t> buf;
				wchar_t* pstr=buf.vprintf(fmt,argptr);
				OutputDebugStringW(v_buf<wchar_t>().printf(L"[%d:%d]:%s\n",GetCurrentThreadId(),GetCurrentProcessId(),pstr) );
			}

			static 		HRESULT GetClassID(CLSID* pclsid){

				*pclsid=__uuidof(array_type_class_t);

				dbg_print(L"GetClassID");
				return S_OK;
			}

			//HRESULT (*CreateInstance)(i_marshal_helper_t* helper,DWORD flags,void* init_context,LPOLESTR pszDisplayName,REFIID riid,void ** ppvObject);
			static 		HRESULT CreateContext(BOOL fUnmarshal,IBindCtx* bctx,LPOLESTR pszDisplayName,DWORD* pflags,void ** ppcontext){

				HRESULT hr;
				//wchar_t* pp= new wchar_t[1024];
				//wcscpy(pp,pszDisplayName);
				//*ppcontext=pp;
				moniker_parser_t<wchar_t> mn(pszDisplayName,bctx);
				int fm=mn.args[L"unmarshal"];
				shared_mem_t* psm=new shared_mem_t;
				if(!fm)
				{
					SAFEARRAY* psa;
					INT64 size;


					VARIANT v=mn.bind_args(L"data");
					variant_resolver vr(&v); 
					VARTYPE vt=vr.vt();
					psa=(vt&VT_ARRAY)?vr->parray:0;
					if(psa)
					{	

						size=SAFEARRAY_total_bytelength(psa);

						psm->init(size,vt&(~VT_ARRAY));
						SafeArrayLock(psa);
						char* ptr;
						long  indx[64];			
						memset(indx,0,sizeof(indx));
						OLE_CHECK_hr(SafeArrayPtrOfIndex(psa,indx,(void**)&ptr));
						memcpy(psm->ptr,ptr,size);
						SafeArrayUnlock(psa);			

					}
					else 
					{
						size=mn.bind_args(L"size").def<double>(0);
						size*=sizeof(value_type_t);
						psm->init(size,vtype);

					}



				}
				*ppcontext=psm;
				dbg_print(L"CreateContext");
				return S_OK;
			}
			static 		void OnExit(void *context){

				delete (shared_mem_t*)context;
				dbg_print(L"OnExit");
			};




			static 		void InvokeByName(void* context,pcallback_context_arguments_t pcca,i_marshal_helper_t* helper){

				//dbg_print(L"InvokeByName:%s",(wchar_t*)context);
				typedef long long int64_t;

				wchar_t* pm_name;


				struct disp_test_t:IDispatch_base_t<disp_test_t>
				{
					disp_test_t():cc(0){}
					inline	HRESULT impl_Invoke(DISPID,REFIID,LCID,WORD,DISPPARAMS*,VARIANT* r,EXCEPINFO*, UINT*){
						r->vt=VT_I4;
						r->intVal=cc++;
						return 0;
					}
					int cc;
				};

				shared_mem_t* psm=(shared_mem_t*)context; 
				value_type_t* pf=(value_type_t*)(psm->ptr);


				//	
				locker_t<shared_mem_t::mutex_t> lock_shared_mem(psm->mutex); 



				arguments_t arguments(pcca);
				HRESULT &hr=arguments.hr;
				if(arguments.id()==0)
				{

					/*		
					arguments.result().vt=VT_DISPATCH;

					arguments.result().pdispVal=new disp_test_t();

					return;
					*/
					int i=arguments[0];
					if(arguments.is_put_flag())
						pf[i]=arguments.result();
					else arguments.result()=pf[i];

					return;
				}
				INT64 indx;

				if(arguments.check_index(&indx))
				{
					//return; 
					if(arguments.is_get_flag())
						arguments.result()=pf[indx];
					else 	pf[indx]=arguments.result();
					/*
					{
					//	arguments.result()=pf[indx];
					VARIANT& r=arguments.result();
					r.vt=VT_R8;
					r.dblVal=pf[indx];

					}
					*/


					return;
				}
				else if((arguments.is_get_flag())&&(pm_name=arguments.pm_name()))
				{
					if(moniker_parser_t<wchar_t>::_lcheckni(pm_name,L"length"))
					{
						arguments.result()=double(psm->size())/sizeof(value_type_t);
						return ;
					}
					if(moniker_parser_t<wchar_t>::_lcheckni(pm_name,L"bytelength"))
					{
						arguments.result()=double(psm->size());
						return ;
					}

					if(moniker_parser_t<wchar_t>::_lcheckni(pm_name,L"foreach"))
					{
						smart_ptr_t<IDispatch> callback;
						OLE_CHECK_VOID(hr=arguments[0].QueryInterface(callback._address()));
						int64_t cb=(psm->size())/sizeof(value_type_t);
						VARIANT v[2]={{vtype},{VT_R8}};
						smart_ptr_t<IDispatchEx> callbackjs;
						bool fjs=SUCCEEDED(callback.QueryInterface(callbackjs._address()));

						DISPPARAMS dps={v,0,2,0};
						if(fjs)

							for(int64_t k=0;k<cb;k++)
							{
								VARIANT res={};
								array_type_class_t::ref(v[0])=pf[k];
								v[1].dblVal=double(k);
								OLE_CHECK_VOID(hr=callbackjs->InvokeEx(0, LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD,&dps,&res,0,0));
								OLE_CHECK_VOID(hr=VariantChangeType(&res,&res,VARIANT_NOVALUEPROP,vtype));
								pf[k]=array_type_class_t::ref(res);

							}

						else 
							for(int64_t k=0;k<cb;k++)
							{
								VARIANT res={};
								array_type_class_t::ref(v[0])=pf[k];
								v[1].dblVal=double(k);
								OLE_CHECK_VOID(hr=callback->Invoke(0, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD,&dps,&res,0,0));
								OLE_CHECK_VOID(hr=VariantChangeType(&res,&res,VARIANT_NOVALUEPROP,vtype));
								pf[k]=array_type_class_t::ref(res);

							}


							return ;
					}

					if(moniker_parser_t<wchar_t>::_lcheckni(pm_name,L"toSAFEARRAY"))
					{
						VARTYPE vt;
						if(arguments.length())
						{
							bstr_t tname=arguments[0].def<bstr_t>(bstr_t());
							if( tname.length()&&SUCCEEDED(VARTYPE_finder_t<>::type_by_name(tname,&vt)));			                				
							else OLE_CHECK_VOID(hr=DISP_E_UNKNOWNNAME);				
						}
						else vt=vtype;

						long esize=VARTYPE_finder_t<>::element_size(vt);
						if(!esize) OLE_CHECK_VOID(hr=DISP_E_UNKNOWNNAME);	
						long cb=psm->size()/esize,l=0;
						SAFEARRAY* psa;
						SAFEARRAYBOUND sb={cb,0};
						if(!(psa=SafeArrayCreate(vt,1,&sb)) ) 
							OLE_CHECK_VOID(hr=E_OUTOFMEMORY);
						SafeArrayLock(psa);
						char* ptr;
						OLE_CHECK_VOID(hr=SafeArrayPtrOfIndex(psa,&l,(void**)&ptr));
						long cc=cb*esize;
						memcpy(ptr,psm->ptr,cc);
						SafeArrayUnlock(psa);
						VARIANT vr={VT_ARRAY|vt};
						vr.parray=psa;
						arguments.result().Attach(vr);
						return;

					}
					if(moniker_parser_t<wchar_t>::_lcheckni(pm_name,L"pid"))
					{
						arguments.result()=int(GetCurrentProcessId());
						return;
					}
					if(moniker_parser_t<wchar_t>::_lcheckni(pm_name,L"tid"))
					{
						arguments.result()=int(GetCurrentThreadId());
						return;
					}

					if(moniker_parser_t<wchar_t>::_lcheckni(pm_name,L"type"))
					{
						arguments.result()=VARTYPE_finder_t<>::jsname(psm->shdata.vt,L"ArrayBuffer");
						return;
					}

					//VARTYPE_finder_t




				}
				arguments.hr=E_INVALIDARG;
				//arguments.result()=variant_t((wchar_t*)context);

			};
			static 		HRESULT MarshalToProcess(void* context,i_marshal_helper_t* helper,DWORD pid_target,IStream* pStm){
				dbg_print(L"MarshalToProcess");
				HRESULT hr;
				ULONG l;

				//hr=pStm->Write(context,1024,&l);			

				shared_mem_t* psm=(shared_mem_t*)context; 

				OLE_CHECK_hr(helper->marshal_hko(pStm,psm->hmap));		
				OLE_CHECK_hr(helper->marshal_hko(pStm,psm->mutex.hko));
				OLE_CHECK_hr(pStm->Write(&psm->shdata,sizeof(psm->shdata),&l));		

				return hr;
			};
			static 		HRESULT Unmarshal(void* context,i_marshal_helper_t* helper,IStream* pStm){
				dbg_print(L"Unmarshal");
				HRESULT hr;
				ULONG l;
				//hr=pStm->Read(context,1024,&l);	
				//wchar_t* ppp=(wchar_t* )context;

				HANDLE hmap,hmutex;
				OLE_CHECK_hr(hr=helper->unmarshal_hko(pStm,&hmap))
					OLE_CHECK_hr(hr=helper->unmarshal_hko(pStm,&hmutex))


					shared_mem_t* psm=(shared_mem_t*)context; 
				psm->reset(hmap);

				psm->mutex.reset(hmutex);

				//OLE_CHECK_hr(pStm->Read(&psm->sizeb,sizeof(psm->sizeb),&l));		
				OLE_CHECK_hr(pStm->Read(&psm->shdata,sizeof(psm->shdata),&l));		
				return hr;

			};

			static HRESULT SetAggregatorHelpers(void *context,iaggregator_container* container,pcallback_lib_arguments_t plib)
			{
				HRESULT hr;
				shared_mem_t* psm=(shared_mem_t*)context; 
				//local_ptr_t<mbv_buffer_ptr_aggregator_t> mptr=new	mbv_buffer_ptr_aggregator_t(psm);
				smart_ptr_t<iaggregator_helper> helper=aggregator_helper_creator_t<mbv_buffer_ptr_aggregator_t<array_type_class_t> >::New(new	mbv_buffer_ptr_aggregator_t<array_type_class_t>(psm));
				OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mbv_buffer_ptr),helper));
				OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mbv_typed_buffer_ptr),helper));
				// mptr.detach();
				return hr;
			};

			static MarshalCallbackData_t getMCD()
			{
				MarshalCallbackData_t m={&GetClassID,&CreateContext,&OnExit,&InvokeByName,&MarshalToProcess,&Unmarshal,&SetAggregatorHelpers};
				return m;
			}
		};


	};
};