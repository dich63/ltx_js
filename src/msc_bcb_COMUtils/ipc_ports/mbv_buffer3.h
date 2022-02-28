#pragma once

//template <class ArrayType=unsigned char>
#include "ipc_utils_base.h"
#include "dispid_find.h"
#include "variant_resolver.h"
namespace ipc_utils {
	namespace mbv3 {

		typedef  INT64 int64_t;

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

		struct safe_array_ptr_t
		{
			HRESULT hr,hrl;
			char* p;
			SAFEARRAY* psa;
			LONG64 length;
			LONG64 sizeb;
			long  element_size;
			safe_array_ptr_t(SAFEARRAY* _psa):p(0),psa(_psa)
			{
				long ll[64];
				memset(ll,0,sizeof(ll));
				OLE_CHECK_VOID(hr=hrl=SafeArrayLock(psa));
				sizeb=(length=SAFEARRAY_total_length(psa))*(element_size=psa->cbElements);
				OLE_CHECK_VOID(hr=SafeArrayPtrOfIndex(psa,ll,(void**)&p));	   				   
				
			}
			~safe_array_ptr_t(){  if(SUCCEEDED(hrl)) SafeArrayUnlock(psa); }
			template<class N>
			inline operator N*(){ return (N*)p;}
			inline operator char*(){ return p;}
			inline operator HRESULT(){ return hr;};

		};



		struct page_region
		{
			int64_t beg_offset;
			int64_t end_offset;

			inline bool into(page_region const& ps)
			{
				return (ps.end_offset>=end_offset)&&(ps.beg_offset<=beg_offset);
			}
		};

		struct sm_region_base_t
		{
            int64_t size;
			int64_t offset;

		};





		//{3C30DDC4-8347-4059-A48A-F93D3B5FC2CB}
		template <int VERS=0>
		struct shared_mem_t
		{


			//
			typedef mutex_ko_t mutex_t;


			//typedef mutex_cs_t mutex_t;



			shared_mem_t():hmap(0),ptr(0),hr(E_FAIL),shdata(shared_mem_data_t()){				
			}

			shared_mem_t& init (INT64 _size,INT64 vt)
			{
				hr=E_FAIL;
				smc.psmd=&shdata;
				smc.pobject=(void*)this;

				long element_size=VARTYPE_finder_t<>::element_size(vt);
				INT64 sizeb;
				if(_size)
				{
					sizeb=element_size*_size;
					LARGE_INTEGER l;
					l.QuadPart=sizeb+7;
					HANDLE hm;
					hm=CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE|SEC_COMMIT,l.HighPart,l.LowPart,0);
					if(!hm)
								hr=HRESULT_FROM_WIN32(GetLastError());					
					
					
					reset(hm);   
					if(hmap) {
						size()=sizeb;
						shdata.vt=vt;
						shdata.element_size=element_size;
						//set_element_size();
						//
						mutex.reset(CreateMutex(0,0,0));
						hr=S_OK;
					}
				}
				return *this;
			}
			/*
			inline int set_element_size()
			{
			return shdata.element_size=VARTYPE_finder_t<>::element_size(shdata.vt);
			}
			*/

			inline int element_size()
			{
				return shdata.element_size;
			}

			inline INT64 count()
			{
				return shdata.sizeb/shdata.element_size;
			}

			inline int vt()
			{
				return shdata.vt;
			}

			void reset(HANDLE h=0)
			{
				HANDLE t;
				void* p=ipc_utils::make_detach(ptr);
				if(t=ipc_utils::make_detach(hmap,h))
				{
					UnmapViewOfFile(p);
					if(t!=h) CloseHandle(t);			
				}
				if(hmap)
				{
					ptr=(char*)MapViewOfFile(hmap,FILE_MAP_ALL_ACCESS,0,0,0);
				  if(!ptr) 		hr=HRESULT_FROM_WIN32(GetLastError());					
				}

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
			shared_mem_context_t smc;
			HANDLE hmap;
			mutex_t mutex;

			char* ptr;	
			HRESULT hr;
			//INT64 sizeb;
			inline HRESULT check_bound(INT64 ib)
			{
				return ((0<=ib)&&(ib<shdata.sizeb))?0:DISP_E_BADINDEX;
			}
			inline static HRESULT check_bound(shared_mem_data_t* smd, INT64 ib)
			{
				return ((0<=ib)&&(ib<smd->sizeb))?0:DISP_E_BADINDEX;
			}
			inline HRESULT set_element(INT64 i,VARIANT v)
			{
				HRESULT hr;
				long es=shdata.element_size;
				OLE_CHECK_hr(check_bound(i*=es));
				if(shdata.vt!=v.vt)
					OLE_CHECK_hr(VariantChangeType(&v,&v,0,shdata.vt));


				memcpy(((char*)ptr)+i,&v.intVal,es);			   
				return 0;
			}

			inline  HRESULT get_element(INT64 i,VARIANT *pv)
			{
				HRESULT hr;
				long es=shdata.element_size;
				OLE_CHECK_hr(check_bound(i*=es));

				pv->vt=shdata.vt;								
				char* p=((char*)ptr)+i;
				memcpy(&pv->intVal,p,es);
				return 0;
			}


		};

		template <class _ArrayTypeClass=uint8_class_t>
		struct mbv_buffer_ptr_aggregator_t:base_aggregator_t<mbv_buffer_ptr_aggregator_t<_ArrayTypeClass>,i_mbv_buffer_ptr>
		{
			typedef typename shared_mem_t<> sh_mem_t;
			typedef i_mbv_buffer_ptr::int64_t int64_t;

			typedef _ArrayTypeClass array_type_class_t;
			typedef typename array_type_class_t::i_type_t value_type_t;


			enum{
				vtype=array_type_class_t::vtype
			};


			mbv_buffer_ptr_aggregator_t(sh_mem_t* _shmem):shmem(_shmem){

			};

			virtual HRESULT STDMETHODCALLTYPE GetContext(void** ppcontext)
			{
				HRESULT hr;
				OLE_CHECK_PTR(ppcontext);
				//*((shared_mem_data_t**)ppcontext)=&shmem->shdata;
				*((shared_mem_context_t**)ppcontext)=&shmem->smc;
				return 0;
			}

			virtual HRESULT STDMETHODCALLTYPE GetPtr(void** pptr,int64_t* pByteLength)
			{
				HRESULT hr;
				OLE_CHECK_PTR(pptr);
				*pptr=shmem->ptr;
				pByteLength&&(*pByteLength=shmem->size());		

				return 0;
			};
			virtual HRESULT STDMETHODCALLTYPE GetTotalSize(int64_t* pByteLength=0)
			{
				HRESULT hr;
				OLE_CHECK_PTR(pByteLength);
					*pByteLength=shmem->size();		
				return 0;

			}
			virtual HRESULT STDMETHODCALLTYPE GetRegionPtr(int64_t ByteOffset,int64_t ByteLength,void** pptr)
			{
				HRESULT hr;
				OLE_CHECK_PTR(pptr);
				int64_t cb=shmem->size(),cr=ByteOffset+ByteLength;
				if(!((0<=cr)&&(cr<=cb))) return hr=TYPE_E_OUTOFBOUNDS;
				char* p=shmem->ptr+ByteOffset;
				*pptr=p;
				return hr=S_OK;
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


			sh_mem_t* shmem;
		};


		template <class _ArrayTypeClass=uint8_class_t>
		struct   mbv_buffer_t{

			// MarshalCallbackData_t
			typedef _ArrayTypeClass array_type_class_t;
			typedef typename array_type_class_t::i_type_t value_type_t;
			typedef shared_mem_t<> sh_mem_t;

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
				int fm=fUnmarshal;
					//mn.args[L"__unmarshal__"];
				sh_mem_t* psm=new sh_mem_t;
				if(!fm)
				{
					SAFEARRAY* psa;
					INT64 size;
 

					VARIANT v=mn.bind_args(L"data");
					variant_resolver vr(&v,true); 
					VARTYPE vt=vr.vt();
					psa=(vt&VT_ARRAY)?vr->parray:0;

					if(psa)
					{	
						safe_array_ptr_t sa_ptr(psa);
						OLE_CHECK_hr(sa_ptr);

						size=sa_ptr.length;					

						OLE_CHECK_hr(psm->init(size,vt&(~VT_ARRAY)).hr);
						memcpy(psm->ptr,sa_ptr,sa_ptr.sizeb);

						/*
						SafeArrayLock(psa);
						char* ptr;
						long  indx[64];			
						memset(indx,0,sizeof(indx));
						OLE_CHECK_hr(SafeArrayPtrOfIndex(psa,indx,(void**)&ptr));
						memcpy(psm->ptr,ptr,size);
						SafeArrayUnlock(psa);	
						*/

					}
					else

						if ((vt==VT_DISPATCH)||(vt==VT_UNKNOWN))
						{
							OLE_CHECK_PTR(vr->punkVal);

							smart_ptr_t<i_mbv_buffer_ptr> buffer;
							OLE_CHECK_hr(vr->punkVal->QueryInterface(__uuidof(i_mbv_buffer_ptr),buffer._ppQI()));
							
							pshared_mem_context_t psmc;
							
							OLE_CHECK_hr(buffer->GetContext((void**)&psmc));

							pshared_mem_data_t  pshd=psmc->psmd;
							
							
							size=mn.bind_args(L"length").def<double>(0);
							INT64 offset=mn.bind_args(L"offset").def<double>(0);
							VARTYPE vt=pshd->vt;
							bstr_t tn=mn.bind_args(L"type").def<bstr_t>(bstr_t());
							if(tn.length())
								OLE_CHECK_hr(VARTYPE_finder_t<>::type_by_name(tn,&vt));

							long element_size=VARTYPE_finder_t<>::element_size(vt);
							long offsetb=offset*pshd->element_size;
							long sizeb=size*pshd->element_size;

							int fclone=mn.bind_args(L"clone").def<double>(1);

							OLE_CHECK_hr(sh_mem_t::check_bound(pshd,offsetb+sizeb));
                            
							if(fclone)
							{	

							 char* ps;
							 OLE_CHECK_hr(buffer->GetPtr((void**)&ps));
							 OLE_CHECK_hr(psm->init(size,vt).hr);
							 memcpy(psm->ptr,ps+offsetb,sizeb);

							}
							else
							{
                                sh_mem_t* psm_src=(sh_mem_t*)psmc->pobject;

							}


							


						}

						else
						{
							VARTYPE vt=vtype;

							size=mn.bind_args(L"length").def<double>(0);
							//size*=sizeof(value_type_t);
							bstr_t tn=mn.bind_args(L"type").def<bstr_t>(bstr_t());
							if(tn.length())
								OLE_CHECK_hr(VARTYPE_finder_t<>::type_by_name(tn,&vt));

							OLE_CHECK_hr(psm->init(size,vt).hr);

						}



				}

				*ppcontext=psm;
				dbg_print(L"CreateContext");
				return S_OK;
			}
			static 		void OnExit(void *context){

				delete (sh_mem_t*)context;
				dbg_print(L"OnExit");
			};







			static 		void InvokeByName(void* context,pcallback_context_arguments_t pcca,i_marshal_helper_t* helper){

				//dbg_print(L"InvokeByName:%s",(wchar_t*)context);
				typedef long long int64_t;

				wchar_t* pm_name;




				sh_mem_t* psm=(sh_mem_t*)context; 
				value_type_t* pf=(value_type_t*)(psm->ptr);


				//	
				locker_t<sh_mem_t::mutex_t> lock_shared_mem(psm->mutex); 



				arguments_t arguments(pcca);
				HRESULT &hr=arguments.hr;
				bool fput=arguments.is_put_flag();
				if(arguments.id()==0)
				{

					
					 if((!fput)&(arguments.argc<=0))
					 {

						 const wchar_t* s=VARTYPE_finder_t<>::jsname(psm->shdata.vt,L"ArrayBuffer");

						 variant_t v=(wchar_t*)v_buf<wchar_t>().printf(L"%s[%I64d]",s,psm->count());
						 arguments.result()=v;
						 return;

					 }

					/*		
					arguments.result().vt=VT_DISPATCH;

					arguments.result().pdispVal=new disp_test_t();

					return;
					*/
					int i=arguments[0];
					if(fput)
						hr=psm->set_element(i,arguments.result());
					//pf[i]=arguments.result();
					else  hr=psm->get_element(i,&arguments.result());

					//arguments.result()=pf[i];

					return;
				}
				INT64 indx;

				if(arguments.check_index(&indx))
				{
					//return; 
					if(fput)
						hr=psm->set_element(indx,arguments.result());
					//pf[i]=arguments.result();
					else  hr=psm->get_element(indx,&arguments.result());

					/*
					if(arguments.is_get_flag())
					arguments.result()=pf[indx];
					else 	pf[indx]=arguments.result();

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
						arguments.result()=double(psm->count());
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
						int64_t cb=psm->count();//(psm->size())/sizeof(value_type_t);
						VARIANT v[2]={{vtype},{VT_R8}};
						smart_ptr_t<IDispatchEx> callbackjs;
						bool fjs=SUCCEEDED(callback.QueryInterface(callbackjs._address()));

						DISPPARAMS dps={v,0,2,0};
						if(fjs)

							for(int64_t k=0;k<cb;k++)
							{
								variant_t res;
								//array_type_class_t::ref(v[0])=pf[k];
								OLE_CHECK_VOID(hr=psm->get_element(k,v));								
								v[1].dblVal=double(k);
								OLE_CHECK_VOID(hr=callbackjs->InvokeEx(0, LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD,&dps,&res,0,0));
								if(res.vt!=VT_EMPTY)
								{							
								  OLE_CHECK_VOID(hr=VariantChangeType(&res,&res,VARIANT_NOVALUEPROP,psm->vt()));
                                  OLE_CHECK_VOID(hr=psm->set_element(k,res));								
								}
								//pf[k]=array_type_class_t::ref(res);

							}

						else 
							for(int64_t k=0;k<cb;k++)
							{
								
								variant_t res;
								//array_type_class_t::ref(v[0])=pf[k];
								OLE_CHECK_VOID(hr=psm->get_element(k,v));								
								v[1].dblVal=double(k);
								OLE_CHECK_VOID(hr=callback->Invoke(0, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD,&dps,&res,0,0));
								if(res.vt!=VT_EMPTY)
								{							
									OLE_CHECK_VOID(hr=VariantChangeType(&res,&res,VARIANT_NOVALUEPROP,psm->vt()));
									OLE_CHECK_VOID(hr=psm->set_element(k,res));								
								}

							}


							return ;
					}

					if(moniker_parser_t<wchar_t>::_lcheckni(pm_name,L"raw_load"))
					{

						VARIANT v=arguments[0];
						variant_resolver vr(&v,true);
						VARTYPE vt=vr.vt();
						if(!((vt&VT_ARRAY)&&((vt&(~VT_ARRAY))!=VT_VARIANT)))
							 arguments.raise_error(L"raw SAFEARRAY type mismatch");
						safe_array_ptr_t sap(vr->parray);			
						if(SUCCEEDED(arguments.hr=sap.hr))
						{						
						  INT64 cb=psm->size();
						  if(cb!=sap.sizeb)
							 arguments.raise_error(L"raw SAFEARRAY length mismatch");
						  memcpy(psm->ptr,sap.p,cb);

						}
						return ;
					}

					if(moniker_parser_t<wchar_t>::_lcheckni(pm_name,L"toSAFEARRAY"))
					{
						VARTYPE vt;
						long esize;
						if(arguments.length())
						{
							bstr_t tname=arguments[0].def<bstr_t>(bstr_t());
							if( tname.length()&&SUCCEEDED(VARTYPE_finder_t<>::type_by_name(tname,&vt)));			                				
							else OLE_CHECK_VOID(hr=DISP_E_UNKNOWNNAME);				
							esize=VARTYPE_finder_t<>::element_size(vt);
						}
						else {
							vt=psm->vt();
							esize=psm->element_size();
						}



						if(!esize) OLE_CHECK_VOID(hr=DISP_E_UNKNOWNNAME);	
						long cb=psm->size()/esize,l=0;
						SAFEARRAY* psa;
						SAFEARRAYBOUND sb={cb,0};
						safe_array_ptr_t sa_ptr(SafeArrayCreate(vt,1,&sb));
						OLE_CHECK_VOID(hr=sa_ptr);
						memcpy(sa_ptr,psm->ptr,sa_ptr.sizeb);
						/*
						SafeArrayLock(psa);
						char* ptr;
						OLE_CHECK_VOID(hr=SafeArrayPtrOfIndex(psa,&l,(void**)&ptr));

						long cc=cb*esize;
						memcpy(ptr,psm->ptr,cc);
						SafeArrayUnlock(psa);
						*/
						VARIANT vr={VT_ARRAY|vt};
						vr.parray=sa_ptr.psa;
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

					if(moniker_parser_t<wchar_t>::_lcheckni(pm_name,L"KillMeAfter"))
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

				sh_mem_t* psm=(sh_mem_t*)context; 

                OLE_CHECK_hr(pStm->Write(&psm->shdata,sizeof(psm->shdata),&l));
				OLE_CHECK_hr(helper->marshal_hko(pStm,psm->hmap));		
				OLE_CHECK_hr(helper->marshal_hko(pStm,psm->mutex.hko));
						

				return hr;
			};
			static 		HRESULT Unmarshal(void* context,i_marshal_helper_t* helper,IStream* pStm){
				dbg_print(L"Unmarshal");
				HRESULT hr;
				ULONG l;
				//hr=pStm->Read(context,1024,&l);	
				//wchar_t* ppp=(wchar_t* )context;
				
				HANDLE hmap,hmutex;

				
				

				sh_mem_t* psm=(sh_mem_t*)context; 

				OLE_CHECK_hr(pStm->Read(&psm->shdata,sizeof(psm->shdata),&l));		
				
				OLE_CHECK_hr(hr=helper->unmarshal_hko(pStm,&hmap));
				if(FAILED(hr=helper->unmarshal_hko(pStm,&hmutex)))
				{
					 CloseHandle(hmap);
					 return hr;
				};



					
				psm->hr=hr;
				psm->reset(hmap);

				psm->mutex.reset(hmutex);
				



				//OLE_CHECK_hr(pStm->Read(&psm->sizeb,sizeof(psm->sizeb),&l));		
				
				return hr=psm->hr;

			};

			static HRESULT SetAggregatorHelpers(void *context,iaggregator_container* container)
			{
				HRESULT hr;
				sh_mem_t* psm=(sh_mem_t*)context; 
				//local_ptr_t<mbv_buffer_ptr_aggregator_t> mptr=new	mbv_buffer_ptr_aggregator_t(psm);
				smart_ptr_t<iaggregator_helper> helper=aggregator_helper_creator_t<mbv_buffer_ptr_aggregator_t<array_type_class_t> >::New(new	mbv_buffer_ptr_aggregator_t<array_type_class_t>(psm));

				OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mbv_context),helper));
				OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mbv_context_lock),helper));
				OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mbv_buffer_ptr),helper));
				OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mbv_region_ptr),helper));
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