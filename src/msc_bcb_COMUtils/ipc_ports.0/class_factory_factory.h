#pragma once
//class_factory_factory.h

#include "ipc_ports/imarshal_aggregator.h"
#include "ltx_js.h"
#include <string>
#include <set>
namespace ipc_utils {
	namespace com_apartments
	{
		//typedef mutex_cs_t mutex_t;





		
		struct  __declspec(uuid("{4DA3CB5F-667A-44da-B89E-0AB83F8AE061}")) i_register_class:IUnknown{};
		template<int VersionN=0 >
		struct register_class_map_t:non_copyable_t
		{
			typedef mutex_cs_t mutex_t;
			typedef std::wstring key_t; 



			struct RefCounter_t
			{
				enum {RC0=1};
				RefCounter_t():refcount(RC0),f(true){};

				template <class T>
				inline ULONG AddRef(T* o ){ 					
					locker_t<T::owner_t> lock(o->owner);
					return ++refcount; 
				}
				template <class T>
				inline ULONG Release(const T* o){
					locker_t<T::owner_t> lock(o->owner);
					long l=--refcount;
					if((l==0)&&make_detach(f,false))									
						delete o;

					return l;
				}


				LONG refcount;
				bool f;
			};

			inline void lock() { mutex.lock();   }
			inline void unlock() { mutex.unlock();}


			struct class_item_t:IUnknown_base_impl<class_item_t,i_register_class,RefCounter_t>,public class_object_pair_t
			{
				typedef  register_class_map_t owner_t;
				typedef typename smart_ptr_t<class_item_t> class_item_ptr_t;

				struct less_class_item_ptr_t	: public std::binary_function<class_item_ptr_t,class_item_ptr_t, bool>
				{	// functor for operator<
					typedef class_item_ptr_t ptr_t;
					inline  bool operator()(const ptr_t& l, const ptr_t& r) const
					{	// apply operator< to operands
						return (l.p)&&(r.p)&&(l.p->clsid < r.p->clsid);
					}
				};

				typedef std::set<class_item_ptr_t,less_class_item_ptr_t> class_set_t;
				typedef typename class_set_t::iterator it_class_t;


				class_item_t():owner(0){};

				class_item_t(CLSID c,owner_t* _owner=0):owner(_owner){
					clsid=c;

				};

				void set_refcount(int rc){  refcounter.refcount=rc;}
				int get_refcount(){return  refcounter.refcount;}

				~class_item_t(){
					if(owner)
						owner->erase(this);
				}
				inline HRESULT impl_QueryInterface(const IID& riid, void** ppvObject)
				{
					if(!ppvObject) return E_POINTER;
					if(riid==GUID_NULL) 
					{
						*ppvObject=static_cast<class_object_pair_t*>(this);
						return S_OK;
					}
					return __super::impl_QueryInterface(riid,ppvObject);
				}

				HRESULT set_ClassObjectProc(HRESULT( __stdcall *pClassObject)(REFCLSID , REFIID , LPVOID*))
				{
					if(GetClassObject=pClassObject)
						return module_t<>::DLL_AddRef(GetClassObject);

				}
				//CLSID clsid;					   
				//HRESULT( __stdcall DllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
				owner_t* owner;
				it_class_t iweak;
			};

			//  typedef smart_ptr_t<class_item_t> class_item_ptr_t;





			typedef  class_object_pair_t::GetClassObject_t GetClassObject_t;

			typedef typename class_item_t::class_item_ptr_t class_item_ptr_t;		 

			typedef typename class_item_t::class_set_t class_set_t;
			typedef typename class_item_t::it_class_t it_class_t;

			typedef typename std::multimap<key_t,class_item_ptr_t> monikers_multimap_t;


			inline   void erase(class_item_t* pi){
				// locker_t<mutex_t> lock(mutex);
				//int rc=ref_count(pi);
				//class_item_ptr_t& cc=*(pi->iweak);

				//cc.detach();
				//class_item_ptr_t cc( 
				//it_class_t i=classes.find(pi);

				//(*i).detach();
				classes.erase(pi->iweak);

			};

			register_class_map_t()
			{

			}

			inline int  count()
			{

				locker_t<register_class_map_t> lock(this);
				return classes.size();


			}

			inline HRESULT register_class( CLSID clsid,GetClassObject_t pClassObject,IUnknown** ppUnk,class_object_pair_t** pp_pair=0)
			{
				if(!ppUnk) return E_POINTER;

				HRESULT hr;
				class_item_t key_blank(clsid);
				key_blank.set_refcount(111);
				class_item_ptr_t cp(&key_blank,false);			   

				locker_t<register_class_map_t> lock(this);
				it_class_t it=classes.find(cp);
				cp.detach();
				if(it==classes.end())
				{
					//E_NOINTERFACE
					if(!pClassObject) return hr=REGDB_E_CLASSNOTREG; 
					class_item_ptr_t cpn(new class_item_t(clsid,this),false);
					cpn->set_ClassObjectProc(pClassObject);
					cpn->iweak=classes.insert(it,cpn);

					*ppUnk=static_cast<IUnknown*>(cpn.p);
					if(pp_pair)
						*pp_pair=static_cast<class_object_pair_t*>(cpn.p);


					//hr=cpn->QueryInterface(__uuidof(IUnknown),(void**)ppUnk);

				}
				else
				{
					if((pClassObject)&&(pClassObject!=(*it).p->GetClassObject))
						hr= E_ACCESSDENIED;
					else hr=(*it).p->QueryInterface(__uuidof(IUnknown),(void**)ppUnk);
					if(pp_pair)
						*pp_pair=static_cast<class_object_pair_t*>((*it).p);

					return hr;


				} 	   


				//int rc=ref_count(*ppUnk);


				return S_OK;
			}
			inline HRESULT get_class( CLSID clsid,IUnknown** ppUnk,class_object_pair_t** pp_pair=0)
			{
				return register_class(clsid,0,ppUnk,pp_pair);
			}

            

			inline HRESULT register_moniker(clsid_t clsid,GetClassObject_t pClassObject,const wchar_t* alias=0,IUnknown** ppUnk=0,class_object_pair_t** pp_pair=0)
			{

				HRESULT hr;			   
				bstr_t tmp;
				{		   
					locker_t<register_class_map_t> lock(this);
					smart_ptr_t<class_item_t> item;
					OLE_CHECK_hr(register_class(clsid, pClassObject,(IUnknown**)&item.p,pp_pair));			   
					wchar_t* palias=(alias)?(wchar_t*)alias:(wchar_t*)(tmp=clsid_t(clsid));
					if(!palias) return clsid.hr;
					//std::wstring buf=palias;
					  //palias=_wcsupr(buf.c_str());					

					monikers.insert(std::make_pair(moniker_parser_t<wchar_t>::to_upper(palias),item));					
					if(ppUnk) *ppUnk=item.detach();
				}

				return S_OK;
			}

			inline HRESULT unregister_moniker(CLSID clsid,IUnknown** ppUnk=0)
			{
				return unregister_moniker(bstr_t(clsid_t(clsid)),ppUnk);
			}
			inline HRESULT unregister_moniker(const wchar_t* moniker_name,IUnknown** ppUnk=0)
			{
                   return get_moniker(const wchar_t* moniker_name,ppUnk,true);
			}


			inline HRESULT get_moniker(const wchar_t* moniker_name,IUnknown** ppUnk=0,bool frelease=false)
			{
				if(!moniker_name) return E_POINTER;
				wchar_t*mname,*ptail;
				v_buf<wchar_t> buffer;

				/*
                 bstr_t buffer;
				 
				 mname =buffer=moniker_name,*p; 

				 
				 if(p=moniker_parser_t<wchar_t>::_lskipi(mname,L":",1))
					 p[-1]=0;
					 */

				mname=moniker_parser_t<wchar_t>::extract_moniker(moniker_name,&ptail,true,buffer);
				//if(!mname) return E_INVALIDARG; 
				if(!mname) mname=L""; 


				HRESULT hr;			   
				{		   

					locker_t<register_class_map_t> lock(this);										
					monikers_multimap_t::iterator i= monikers.find(mname);		
					if(i==monikers.end()) return hr=REGDB_E_CLASSNOTREG; 
					if(ppUnk)
					{
						//hr=(*(i->second)).QueryInterface(ppUnk);
						//hr=(*(i->second)->QueryInterface(__uuidof(IUnknown),(void**)ppUnk);
						class_item_ptr_t &cip=i->second;
						hr=cip.QueryInterface(ppUnk);

					    if(FAILED(hr)) return hr;
					}

					if(frelease) monikers.erase(i);
					
				}

				return S_OK;
			}

            static HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
			{
				HRESULT hr;
				register_class_map_t& rmap=get_instance();
				smart_ptr_t<IUnknown> unk;
				class_object_pair_t* ppair;

                OLE_CHECK_hr(rmap.get_class(rclsid,&unk.p,&ppair));
				hr=ppair->GetClassObject(rclsid,riid,ppv);
                 return hr;

			}

			static HRESULT GetClassObject(const wchar_t* pmoniker, REFIID riid, LPVOID* ppv)
			{
				HRESULT hr;
				register_class_map_t& rmap=get_instance();
				smart_ptr_t<IUnknown> unk;
				pclass_object_pair_t ppair;			

				OLE_CHECK_hr(rmap.get_moniker(pmoniker,&unk.p));
				OLE_CHECK_hr(unk->QueryInterface(GUID_NULL,(void**)&ppair));
				
				OLE_CHECK_hr(hr=ppair->GetClassObject(ppair->clsid,riid,ppv));
				

				return hr;

			}








			mutex_t mutex;
			class_set_t classes;

			monikers_multimap_t monikers;

			inline static register_class_map_t& get_instance()
			{
				return class_initializer_T<register_class_map_t>().get();
			}

		};
		// 
		struct  __declspec(uuid("{C63FE25E-EDC5-4af9-8CE1-4F2A8776D04F}")) CLSID_marshal_factory_t{};

		template <class T>
		struct class_factory_factory_t:class_factory_base_aggregator_t<T>
		{

			//template <class T,class IDispatchX=IDispatch >
			//struct IDispatch_impl_t:IDispatch_impl_base_t<T, base_aggregator_t<T,IDispatchX> >

/*
			struct dispatch_binder_t: IDispatch_impl_t<dispatch_binder_t>
			{
				typedef _com_error com_exception;

				inline	HRESULT impl_Invoke(DISPID id,REFIID riid,LCID,WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi, UINT*){

					HRESULT hr=E_UNEXPECTED;
					OLE_CHECK_PTR(res);
					if(!((id==0)&&(dp->cArgs)&&(is_get(flags)))) return hr=E_NOTIMPL;
					try
					{
						arguments_t arguments(dp);
						bstr_t ps=arguments[0];
						if(!ps.length()) return E_INVALIDARG;
						wchar_t* p=ps;
						if(p[0]!=L':') ps=bstr_t(L":")+p;
						//OLE_CHECK_hr(owner->impl_ParseDisplayName())
						
						smart_ptr_t<IParseDisplayName> pdn;
						OLE_CHECK_hr(QueryInterface(__uuidof(IParseDisplayName),pdn._ppQI()));
						smart_ptr_t<IMoniker> moniker;
						smart_ptr_t<IBindCtx> bctx;
						OLE_CHECK_hr(CreateBindCtx(0,&bctx.p));
						ULONG ec;
						OLE_CHECK_hr(pdn->ParseDisplayName(bctx,ps,&ec,&moniker.p));
						smart_ptr_t<IUnknown> unk;
						OLE_CHECK_hr(moniker->BindToObject(bctx,0,__uuidof(IUnknown),unk._ppQI()));
						hr=toVARIANT(unk.p,res);
						
						//OLE_CHECK_hr(owner->CreateInstanceEx(0,ps,0,__uuidof(IUnknown),unk._ppQI()));
					    

						
					}
					catch (com_exception& e){			
						hr=e.Error();
					}
					catch (...){		
					}
					return hr;
					
				}



				dispatch_binder_t(class_factory_factory_t* po):owner(po){}
				class_factory_factory_t* owner;
			} dispatch_binder;



			class_factory_factory_t():dispatch_binder(this){};

*/





			struct marshal_header_reader_t
			{
				enum{
					SBS=2*256  
				};
				marshal_header_reader_t(IStream *_pStm,bool freleaseonly=false)
				{
					ULONG l;
					int cb;
					pStm=_pStm;
					header=(marshal_header_base_t*)buf;
					OLE_CHECK_VOID(hr=pStm->Read(&header->size,sizeof(header->size),&l));
					cb=header->size;

					if(freleaseonly)
					{
						ULARGE_INTEGER ul;
						LARGE_INTEGER ll;
						ll.QuadPart=cb-sizeof(header->size);
						hr=pStm->Seek(ll,STREAM_SEEK_CUR,&ul);
						return;
					}

					if(cb>SBS)
					{

						vbuf.resize(cb+4);
						header=(marshal_header_base_t*)vbuf.size();
						header->size=cb;

					}
					else *((DWORD*)(buf+SBS))=0;

					hr=pStm->Read(&header->clsid,cb-sizeof(header->size),&l);		                     

				}
				inline operator HRESULT()
				{
					return hr;
				}

				inline static HRESULT co_fmt_unmarshal( IStream *pStm, REFIID riid,void **ppv){
					HRESULT hr;
					smart_ptr_t<IUnknown> unk;
					smart_ptr_t<IMarshal> fmt;
					OLE_CHECK_hr(CoCreateFreeThreadedMarshaler(0,&unk.p));
					OLE_CHECK_hr(unk.QueryInterface(&fmt.p));
					hr=fmt->UnmarshalInterface(pStm,riid,ppv);
					return hr;
				}

				inline static  HRESULT co_fmt_release_marshal_data( IStream *pStm){
					HRESULT hr;
					smart_ptr_t<IUnknown> unk;
					smart_ptr_t<IMarshal> fmt;
					OLE_CHECK_hr(CoCreateFreeThreadedMarshaler(0,&unk.p));
					OLE_CHECK_hr(unk.QueryInterface(&fmt.p));
					hr=fmt->ReleaseMarshalData(pStm);
					return hr;
				}

				HRESULT unmarshal(REFIID riid,void **ppv)
				{
					OLE_CHECK_hr(hr);
						if(header->is_std())
							return hr=CoUnmarshalInterface(pStm,riid,ppv);
						else if(header->is_free())
							return hr=co_fmt_unmarshal(pStm,riid,ppv);
						else if(header->is_bind())
						{
							//register_class_map_t<>& rmap=register_class_map_t<>::get_instance();

							wchar_t*p=header->szz,const *pend;
							if(pend=moniker_parser_t<wchar_t>::_lskipi(p,L":",-1))
							{
								HRESULT hr0;
								if(p>=pend) return hr=REGDB_E_CLASSNOTREG; 
								std::wstring ws(p,pend-1);	
								

       						 smart_ptr_t<IClassFactory> cf;
							 OLE_CHECK_hr(register_class_map_t<>::GetClassObject(ws.c_str(),__uuidof(IClassFactory),cf.ppQI()));
							 smart_ptr_t<IClassFactoryStream> cfs;
							 if(SUCCEEDED(cf.QueryInterface(&cfs.p)))
							 {
                              return hr=cfs->CreateInstanceEx(0,p,pStm,riid,ppv);
							 }
                             smart_ptr_t<IParseDisplayName> pdn;
							 if(SUCCEEDED(cf.QueryInterface(&pdn.p)))
							 {
								 bool f;
								 smart_ptr_t<IMoniker> moniker;
								 smart_ptr_t<IBindCtx> bctx;
								 OLE_CHECK_hr(CreateBindCtx(0,&bctx.p));
								 OLE_CHECK_hr(bctx->RegisterObjectParam(bstr_t(L"_marshal_stream"),pStm));
								 bstr_t pstr(SysAllocStringLen(p,safe_len_zerozero(p)),0);
								 ULONG  lp;
								 OLE_CHECK_hr(pdn->ParseDisplayName(bctx,pstr,&lp,&moniker.p));
								 f=(hr==S_OK);
								 OLE_CHECK_hr(moniker->BindToObject(bctx,0,riid,ppv));

							 }
							  return hr;


							 

							};
						}
					return hr=E_NOINTERFACE;
				}

				HRESULT release_marshal()
				{
					OLE_CHECK_hr(hr);
					/*
					if(header->is_std())
						return hr=CoReleaseMarshalData(pStm);
					else if(header->is_free())
						return hr=co_fmt_release_marshal_data(pStm);
					return hr=E_NOINTERFACE;*/
                   if(header->is_free())
					     hr=co_fmt_release_marshal_data(pStm);
				   else  hr=CoReleaseMarshalData(pStm);
				   return hr;
				}


				char buf[SBS+4];
				HRESULT hr;
				IStream* pStm;
				marshal_header_base_t* header;
				std::vector<char> vbuf;

			};



			struct marshal_t:marshal_base_aggregator_t<marshal_t>
			{

                 inline LONG64 get_stream_size( IStream *pStm)   
				 {
					 ULARGE_INTEGER ul;
					 LARGE_INTEGER l={},ls={};
					 LONG64 r=-1;
					 HRESULT hr;
					 if(FAILED(hr=pStm->Seek(l,STREAM_SEEK_CUR,&ul))) return r;
                      ls.QuadPart=ul.QuadPart;
					  if(SUCCEEDED(hr=pStm->Seek(l,STREAM_SEEK_END,&ul))) 
						  r=ul.QuadPart;
					  hr=pStm->Seek(ls,STREAM_SEEK_SET,&ul);
					  return r;

				 }

				inline HRESULT IMarshal_UnmarshalInterface( IStream *pStm, REFIID riid,void **ppv) {

					HRESULT hr;
					//CLSID clsid=__uuidof(IUnknown);
					//int sizz=get_stream_size(pStm)   ;
					

					marshal_header_reader_t hreader(pStm);
					/*
					
					

					if(SUCCEEDED(hr=hreader))
					{                       
						if(hreader.header->is_std())
							return hr=CoUnmarshalInterface(pStm,riid,ppv);
						else if(hreader.header->is_free())
							return hr=co_fmt_unmarshal(pStm,riid,ppv);

					}
					register_class_map_t<>& rmap=register_class_map_t<>::get_instance();
					*/
					hr=hreader.unmarshal(riid,ppv);

					return hr;
				};
				inline HRESULT IMarshal_ReleaseMarshalData( IStream *pStm) {
					HRESULT hr;
					marshal_header_reader_t hreader(pStm);
					/*
					if(SUCCEEDED(hr=hreader))
						if(hreader.header->is_free())
							hr=co_fmt_release_marshal_data(pStm);
						else   hr=CoReleaseMarshalData(pStm);
						*/
					   hr=hreader.release_marshal();
						return hr;
				};

				inline HRESULT IMarshal_DisconnectObject(  DWORD dwReserved) {

					return S_OK;

				};
			};

			/*
			inline HRESULT impl_CreateInstance(	IUnknown * pUnkOuter,REFIID riid,void ** ppvObject,void* params=0)	{
				HRESULT hr;
				if(pUnkOuter) return CLASS_E_NOAGGREGATION;

				return hr=this->GetUnknown()->QueryInterface(riid,ppvObject);
			}
			*/

			inline HRESULT impl_ParseDisplayNameToMoniker(IBindCtx * pbc,LPOLESTR pszDisplayName,ULONG * pchEaten,IMoniker ** ppmkOut)
			{
				HRESULT hr;

				wchar_t* p,*parse_str;

					parse_str=moniker_parser_t<wchar_t>::_lskipi(pszDisplayName,L":",1,true);

				smart_ptr_t<IParseDisplayName> pdn;
				/*
				if(!(parse_str&&parse_str[0]))
				{
					//IUnknown* pCU=this->GetUnknown();
					//smart_ptr_t<IDispatch> unk;
					 //OLE_CHECK_hr(pCU->QueryInterface(__uuidof(IDispatch),unk._ppQI()));
					 //OLE_CHECK_hr(::CreatePointerMoniker(unk,ppmkOut));
					 //
					OLE_CHECK_hr(::CreatePointerMoniker(this->GetUnknown(),ppmkOut));				

				}				else
				*/
				{
					OLE_CHECK_hr(register_class_map_t<>::GetClassObject(parse_str,__uuidof(IParseDisplayName),pdn._ppQI()));
					OLE_CHECK_hr(hr=pdn->ParseDisplayName(pbc,parse_str,pchEaten,ppmkOut));

				}


				
				if(pchEaten)
					*pchEaten=moniker_parser_t<wchar_t>::parse_str_len(pszDisplayName);
					
					


				return hr;				

			}
///*
			inline HRESULT impl_CreateInstance(	IUnknown * pUnkOuter,REFIID riid,void ** ppvObject,void* params=0)	{


				struct F:aggregator_helper_base_t<F>
				{

					inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ){

						return marshal(Outer,riid,ppObj,hr);
					}

					marshal_t marshal;
				};

				HRESULT hr;
				ipc_utils::smart_ptr_t<F> sF(new F,0);
				aggregator_container_helper container;
				if(SUCCEEDED(hr=container.set_aggregator(__uuidof(IMarshal),sF)))
					hr=container.QueryInterface(riid,ppvObject); 

				return hr;
			}
//	*/

/*

			inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ) 
			{ 
				 bool f=   static_cast<T*>(this)->inner_QI2(Outer,riid,__uuidof(IClassFactory),ppObj,hr);
					f||static_cast<T*>(this)->inner_QI2(Outer,riid,__uuidof(IClassFactoryStream),ppObj,hr);
					f||parse_display_name.inner_QI(Outer,riid,ppObj,hr);
					f||dispatch_binder.inner_QI(Outer,riid,ppObj,hr);

				   return f;
			}
*/

/*
			inline HRESULT GetFactory(REFIID riid,void** ppcf){


				if(this==(void*)0) return E_OUTOFMEMORY;

				HRESULT hr;
				if(!ppcf) return E_POINTER;
				aggregator_container_helper container;

				ipc_utils::smart_ptr_t<IUnknown> sF=aggregator_helper_creator_t<class_factory_base_aggregator_t>::New(this);

				OLE_CHECK_hr(container.set_aggregator(__uuidof(IClassFactoryStream),__uuidof(IClassFactory),__uuidof(IParseDisplayName),__uuidof(IDispatch),sF));//
				OLE_CHECK_hr(container.set_FTM());
				mta_singleton_t::add_external_connector(container);
				smart_ptr_t<IClassFactory> cf;
				hr=container.QueryInterface(__uuidof(IClassFactory),cf._ppQI());
				if(__uuidof(IClassFactory)==riid) *ppcf=(void**) cf.detach();
				else  hr=container.QueryInterface(riid,ppcf);
				return hr;
			};
*/


		};
	};//com_apartments
};//ipc_utils
