#pragma once

#include "marshal_by_value_base.h"
#include "ipc_utils.h"
#include "sa_utils.h"
#include "sm_region.h"
#include "sparse_file_map.h"
#include "singleton_utils.h"
#include "sa_utils.h"
#include <map>
#include <string>
#include <sstream>

namespace ipc_utils {	

		namespace mbv_container {

			//
			// 
			
			typedef shared_socket_data_t::uint64_t uint64_t; 
			typedef LONGLONG int64_t; 			
			typedef moniker_parser_t<wchar_t> parser_t;

			





			



			


            struct  __declspec(uuid("{624B71FD-5585-4d26-A764-B158CB04705D}"))  co_mbv_container_t{};

			

			//struct  mbv_container_t: marshal_by_value_base::mbv_context_base_t<mbv_container_t,__uuidof(co_mbv_container_t),true>
			struct  mbv_container_t : marshal_by_value_base::mbv_context_base_c_t<mbv_container_t, co_mbv_container_t, true>
			{


              typedef std::map<std::wstring,com_variant_t> variant_map_t;
				
				
				struct CallbackDispatch_t:IDispatch_base_t<CallbackDispatch_t,IDispatch,RefCounterMT_t<1> >
				{

					static HRESULT make_header(variant_map_t& vm,VARIANT* res)
					{
						HRESULT hr=S_OK;
						VARIANT ve={VT_EMPTY};
						variant_t v;						
						*res=ve;
						int len=vm.size();						
						if(!len)
							return hr;
						 

						SAFEARRAY* psa=SafeArrayCreateVector(VT_VARIANT,0,len+1);

						if(!psa) return HRESULT_FROM_WIN32(GetLastError());

						v.vt=VT_ARRAY|VT_VARIANT;
						v.parray=psa;

						sa_utils::safe_array_ptr_t sa(psa);						
						hr=sa;


						int n=1;

						const wchar_t zs=L'\0';
						std::wstringstream stream;
						VARIANT* pv=sa;
						if(SUCCEEDED(hr))
						for(variant_map_t::iterator i=vm.begin();i!=vm.end();++i,n++){
							
							const std::wstring& key=i->first;
							VARIANT& vs=i->second;
							if(FAILED(hr=VariantCopyInd(pv+n,&vs)))
								return hr;							
							stream<<key<<zs;

						}

						stream<<zs;
						
						 size_t sz=stream.str().length();
						 BSTR bs=SysAllocStringLen(0,sz);
						 memcpy(bs,stream.str().c_str(),sizeof(wchar_t)*(sz+1));
						 pv[0].vt=VT_BSTR;
						 pv[0].bstrVal=bs;
						*res=v.Detach();				 					
						
                        return hr; 

					}

					static HRESULT get_names(variant_map_t& vm,VARIANT* res, wchar_t* pzs=L"\0")
					{
						
						HRESULT hr=S_OK;
						OLE_CHECK_PTR(res);
						VARIANT v={VT_BSTR};
						wchar_t zs=(pzs)?pzs[0]:L'\0';
						
						std::wstringstream stream;						
						
							for(variant_map_t::iterator i=vm.begin();i!=vm.end();++i){

								const std::wstring& key=i->first;
							
								stream<<key<<zs;

							}

							//stream<<zs;

							size_t sz=stream.str().length();
							BSTR bs=SysAllocStringLen(0,sz);
							memcpy(bs,stream.str().c_str(),sizeof(wchar_t)*(sz+1));
							v.bstrVal=bs;
							*res=v;
							return S_OK;


					}

					CallbackDispatch_t(variant_map_t& vm){

						hr0=make_header(vm,&data);

					}

					inline	HRESULT impl_Invoke(DISPID did,REFIID,LCID,WORD,DISPPARAMS*dps,VARIANT* res,EXCEPINFO*, UINT*){
						
						
						HRESULT hr=E_INVALIDARG;
						

						ISequentialStream* pInnerStream=0;

						if((did==0))
						{
							OLE_CHECK_hr(hr=hr0);
							hr=VariantCopy(res,&data);
						}

						return hr;
					}

					~CallbackDispatch_t(){
						///if(owner)				owner->koxMarshalFinally(pcontext);
						VariantClear(&data);
					}


					 static HRESULT get_data(IDispatch* disp,variant_map_t& vmap)
					 {
						 HRESULT hr;
						 DISPPARAMS dps={0};
						 com_variant_t r;
						 OLE_CHECK_hr(disp->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_METHOD,&dps,&r,0,0));
						 if(r.vt==VT_EMPTY) 
							    return S_OK;

						 sa_utils::safe_array_ptr_t sa(r.parray);
						 int len=sa.length;
						 if(len<2) 
							 return S_OK;
						 --len;
						 std::vector<wchar_t*> keys;
						 size_t ll=0;
						 VARIANT* pv=sa;
						 args_ZZ(pv->bstrVal,&ll,keys);
						 if(keys.size()!=len) return E_INVALIDARG;

						 ++pv;

						 for(int k=0;k<len;k++)
						 {
							 VARIANT &v=pv[k];
							 vmap[keys[k]].Attach(v);
							 v.vt=VT_EMPTY;
						 }

						 return hr;			 

						 //inline std::vector< CH*>& args_ZZ(const CH* pzz,size_t* plength=0,std::vector<CH*>& psv=std::vector<CH*>(),int cinc=8)

					 }



					VARIANT data;
					HRESULT hr0;
				};




				mbv_container_t(){
					
					//locker_mutex_t lock(hmutex);

				}

				


				HRESULT Init(BOOL fUnmarshal,IBindCtx* bctx,LPOLESTR pszDisplayName,DWORD& flags)
				{										
					HRESULT hr;	

					return hr=S_OK;				
				}


				HRESULT marshal_to_process(i_marshal_helper_t* helper,DWORD pid_target,IStream* pStm){

					HRESULT hr;
					
					smart_ptr_t<IDispatch> cd(new CallbackDispatch_t(vmap),false);

					OLE_CHECK_hr(helper->marshal_interface(pStm,__uuidof(IDispatch),cd));	
					

					return hr;
				}

				HRESULT unmarshal(i_marshal_helper_t* helper,IStream* pStm){
					HRESULT hr;
					
					
					//OLE_CHECK_hr(helper->unmarshal_hko(pStm,(HANDLE*)&socket_data.sock));		
					smart_ptr_t<IDispatch>  cbdisp;
					OLE_CHECK_hr(helper->unmarshal_and_release(pStm,__uuidof(IDispatch),cbdisp._ppQI()));


					hr=CallbackDispatch_t::get_data(cbdisp,vmap);
										

					return hr;
					
				}

				inline HRESULT _set_empty(VARIANT *res) {
					VARIANT v = { VT_EMPTY };
					*res = v;
					return S_OK;
				}

				template <bool fget>
				inline HRESULT get_put_item( VARIANT& vkey, VARIANT* res) {

					HRESULT hr;

					com_variant_t vname;
					OLE_CHECK_hr(VariantChangeType(&vname, &vkey, 0, VT_BSTR));
					const wchar_t* name = vname.bstrVal;

					if (!name)
						return hr = E_INVALIDARG;
					return hr = get_put_item_c<fget>( name, res);

				}
				template <bool fget>
				inline HRESULT get_put_item_c( const wchar_t* name, VARIANT* res) {

					HRESULT hr;
					OLE_CHECK_PTR(res);

						if (fget) {
							variant_map_t::iterator i = vmap.find(name);
							if(i!=vmap.end()){ 
								VARIANT& v = i->second;
								return	hr = ::VariantCopy(res, &v);
							}
							else return hr=_set_empty(res);
							
						}
						else {

							VARIANT& v = vmap[name];
							return hr = ::VariantCopy(&v, res);
						}
			
					

					
				}




				inline HRESULT get_length(VARIANT* vlen)
				{
					VARIANT v={VT_I4};
					 v.intVal=vmap.size();
					 *vlen=v;
					return S_OK;
				}

				inline  HRESULT  on_get(DISPID id,const wchar_t* name,com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper)
				{

					HRESULT hr;
										
					if(id==0)
					{
						if(arguments.length()>0)
						{
							if(arguments.length() > 1){
								int c = arguments[0].def<int>(0);
								if (c == -1) {
									vmap.clear();
									return S_OK;
								}
								else return E_INVALIDARG;
							}
							bstr_t b=arguments[0].def<bstr_t>(L"\0");
							wchar_t* p=b;

							return hr=CallbackDispatch_t::get_names(vmap,&result,p);

						}
						else return hr=get_length(&result);

					}
					/*
						s_parser_t<wchar_t>::case_t<false> cs(name);

						if(cs(L"length"))										
							return get_length(&result);
							*/

						 if(name)
						 {
							 //result=vmap[name];
						     return hr=get_put_item_c<true>(name,&result);
						 }
						
					
						return E_NOTIMPL;
				};

				inline  HRESULT  on_put(DISPID id,const wchar_t* name,com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper){

					if(id&&name)
					{				

                    /*s_parser_t<wchar_t>::case_t<false> cs(name);
                    if(!cs(L"length")){		
											
							vmap[name]=result;
							return S_OK;
					  }
					  */
						if(result.vt!=VT_EMPTY)						
							return  get_put_item_c<false>(name, &result);
						else vmap.erase(name); 

						return S_OK;
					}
					return E_NOTIMPL;
				};


				struct i_mbv_container_impl:base_aggregator_t<i_mbv_container_impl,i_mbv_container>
				{
					 i_mbv_container_impl(mbv_container_t* pcontainer,iaggregator_container* aggregator_container)
						:base_aggregator_t<i_mbv_container_impl,i_mbv_container>(aggregator_container)
						,container(pcontainer){}	
					~i_mbv_container_impl(){
						OutputDebugStringW(L"~mm_socket_context_t\n");
					}
					 

					inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ) 
					{              
						
						bool f;
						f  =(riid==__uuidof(i_mbv_container));
//												

						if(f)
						{
							//InterlockedExchangePointer((void**)&pOuter,Outer);
							*ppObj=static_cast<void*>(this);
							 AddRef();
							
							hr=S_OK;

						}


						return f;
					}


					virtual HRESULT STDMETHODCALLTYPE GetLength(long* pv){
						
						HRESULT hr;
						OLE_CHECK_PTR(pv);
						VARIANT v;
						 hr= container->get_length(&v);
						 *pv=v.intVal;
						 return hr;
						

					};

					virtual HRESULT STDMETHODCALLTYPE GetNames(VARIANT** ppv,long* plength=0)
					{
						HRESULT hr=S_OK;
						OLE_CHECK_PTR(ppv);
						long len=container->vmap.size();						
						if(plength) *plength=len;
						 vnames.resize(len);
						 variant_map_t::iterator i=container->vmap.begin();
							 for(long k=0;k<len;k++)
							 {
                                vnames[k]=variant_t((i++)->first.c_str());   
							 }

							 *ppv=(len)?&vnames[0]:0; 


						return hr;

					};

					virtual HRESULT STDMETHODCALLTYPE IsItem(BSTR key){
					     
						 bool f=key&&(container->vmap.find(key)!=container->vmap.end());						 
							 return f?S_OK:S_FALSE;					 
					  
					}

					virtual HRESULT STDMETHODCALLTYPE GetItem(VARIANT key,VARIANT* pv){

						return container->get_put_item<true>(key,pv);
					};
					virtual HRESULT STDMETHODCALLTYPE SetItem(VARIANT key,VARIANT v){

						return container->get_put_item<false>(key,&v);
					};


					

					mbv_container_t* container;
					std::vector<variant_t> vnames;

				};

				


				HRESULT set_aggregator_helpers(iaggregator_container* container,pcallback_lib_arguments_t plib)
				{

					HRESULT hr;
					 
					
					

					smart_ptr_t<iaggregator_helper> helper=
						aggregator_helper_creator_t<i_mbv_container_impl>::New(new	i_mbv_container_impl(this,container));					
                    

					OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mbv_container),helper));					
					

					return hr;
				}








				//mm_socket_context_data_impl_t socket_data;
	
					variant_map_t vmap;
					



			};







		}; //mbv_socket
	
};// ipc_utils
