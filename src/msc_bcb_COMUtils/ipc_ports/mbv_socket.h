#pragma once

#include "marshal_by_value_base.h"
#include "ipc_utils.h"
#include "sa_utils.h"
#include "sm_region.h"
#include "sparse_file_map.h"
#include "singleton_utils.h"
#include "iocp_utils.h"
#include <io.h>

namespace ipc_utils {	

		namespace mbv_socket{

			//
			// 
			struct mm_socket_context_data_t:shared_socket_data_t{};		
			typedef shared_socket_data_t::uint64_t uint64_t; 
			typedef LONGLONG int64_t; 

			typedef sm_region::sm_region_base_t region_base_t;
			typedef sm_region::sm_region_t region_t;
			typedef moniker_parser_t<wchar_t> parser_t;

			typedef sm_region::region_cache_t<2>  region_cache_t;





			


			struct mm_socket_context_data_impl_t : mm_socket_context_data_t
			{
				mm_socket_context_data_impl_t()
				{  
					shared_socket_data_t t={INVALID_SOCKET};
					*pshared_socket_data_t(this)=t;

				}
				~mm_socket_context_data_impl_t(){ clear();}

				void clear()
				{

					shared_socket_data_t t0={INVALID_SOCKET};
					shared_socket_data_t t=ipc_utils::make_detach<shared_socket_data_t>(*this,t0);
					(t.sock==INVALID_SOCKET)||::closesocket(t.sock);
					(t.habort_event==0)||CloseHandle(t.habort_event);
					SysFreeString(t.buffer_tail);
					SysFreeString(t.header);

				}

				void close()
				{
					SOCKET t;
					if(INVALID_SOCKET!=(t=ipc_utils::make_detach(sock,INVALID_SOCKET)))
					{
						::closesocket(t);
						SetEvent(habort_event);
						
					}
					
				}


			};

			


            struct  __declspec(uuid("{EB4E1EE5-4702-4844-AF04-52CDC4554AC0}"))  co_mm_socket_context_t{};

			//struct  sh_socket_context_t: marshal_by_value_base::mbv_context_base_t<sh_socket_context_t,__uuidof(co_mm_socket_context_t),true>
			struct  sh_socket_context_t : marshal_by_value_base::mbv_context_base_c_t<sh_socket_context_t,co_mm_socket_context_t, true>
			{



				sh_socket_context_t(){
					
					//locker_mutex_t lock(hmutex);

				}

				~sh_socket_context_t(){		
					

				}


				HRESULT Init(BOOL fUnmarshal,IBindCtx* bctx,LPOLESTR pszDisplayName,DWORD& flags)
				{

					//
					flags=(1<< MSHCTX_INPROC)|0x10000; 
					if(fUnmarshal) return S_OK;

					HRESULT hr;					

					parser_t parser(pszDisplayName,bctx);

					
					bool feventdup=parser.bind_args(L"socket.dup").def<int>(0);
					if(parser.args.is(L"socket"))
					{
						bool f;
						f=parser.bind_args(L"socket.dup").def<int>(0);
						INT_PTR isoc = parser.bind_args(L"socket").def<INT_PTR>(-1);
						if (parser.bind_args(L"socket.posix").def<int>(0)) {
						   f=HANDLE(isoc = ::_get_osfhandle(isoc)) != (INVALID_HANDLE_VALUE);						   
						}

						SOCKET s=(SOCKET)isoc;
						socket_data.sock=(f)?ipc_utils::dup_socket(s):s;
						socket_data.flags=0;
						if(f=parser.bind_args(L"socket.srv").def<int>(0))
							socket_data.flags|=socket_data.f_srv;

					
					}
					else if (parser.args.is(L"socket.shared")) {
						
						WSAPROTOCOL_INFOW wsa_info;
						SOCKET s;

						bstr_t sdata = parser.bind_args(L"socket.shared").def<bstr_t>();

						parser.base64decode_to(wsa_info, (wchar_t*)sdata);

						s=WSASocketW(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, &wsa_info, 0, FROM_PROTOCOL_INFO);
						if (INVALID_SOCKET == s) {
							return HRESULT_FROM_WIN32(WSAGetLastError());
						}
						socket_data.sock = s;
					}
					else
					{
						SOCKET s=WSASocketW(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
						
						
							int flag=1;
							if(SOCKET_ERROR==setsockopt( s, IPPROTO_TCP,TCP_NODELAY,(char*)&flag,sizeof(BOOL)))
								return HRESULT_FROM_WIN32(WSAGetLastError());
								socket_data.sock=s;

						
					}


					if(parser.args.is(L"abortEvent"))
					{
						bool feventdup=parser.bind_args(L"socket.dup").def<int>(0);
						HANDLE hevent=(HANDLE)parser.bind_args(L"socket").def<INT_PTR>(-1);
						socket_data.habort_event=(feventdup)?ipc_utils::dup_handle(hevent):hevent;

					}
					else socket_data.habort_event=CreateEvent(0,true,false,0);
					
					
					

					return hr=S_OK;				





				}


				HRESULT marshal_to_process(i_marshal_helper_t* helper,DWORD pid_target,IStream* pStm){

					HRESULT hr,hr0;
					ULONG l;
					
					OLE_CHECK_hr(helper->marshal_hko(pStm,(HANDLE)socket_data.sock));		
					OLE_CHECK_hr(helper->marshal_hko(pStm,socket_data.habort_event));		

					//OLE_CHECK_hr(pStm->Write(&socket_data.state,sizeof(socket_data.state),&l));
					//OLE_CHECK_hr(pStm->Write(&socket_data.wbs_frame_read,sizeof(socket_data.wbs_frame_read),&l));

					OLE_CHECK_hr(pStm->Write(&socket_data.meta_data,sizeof(socket_data.meta_data),&l));

					OLE_CHECK_hr(helper->marshal_BSTR(socket_data.buffer_tail,pStm));
					OLE_CHECK_hr(helper->marshal_BSTR(socket_data.header,pStm));
					

					//context_data.close();
					   SOCKET t;

					    OLE_CHECK_if_hr(CancelIoEx((HANDLE)socket_data.sock,NULL),hr=GetLastError()); 

					    t=ipc_utils::make_detach(socket_data.sock,INVALID_SOCKET);
						
					//	
						hr0=iocp_utils::iocp_t<>::get_instance().clearIOCP(t);
						::closesocket(t);

					return hr;
				}
				HRESULT unmarshal(i_marshal_helper_t* helper,IStream* pStm){
					HRESULT hr;
					ULONG l;
					
					OLE_CHECK_hr(helper->unmarshal_hko(pStm,(HANDLE*)&socket_data.sock));		
					OLE_CHECK_hr(helper->unmarshal_hko(pStm,&socket_data.habort_event));		
					
					//OLE_CHECK_hr(pStm->Read(&socket_data.state,sizeof(socket_data.state),&l));					
					//OLE_CHECK_hr(pStm->Read(&socket_data.wbs_frame_read,sizeof(socket_data.wbs_frame_read),&l));
					OLE_CHECK_hr(pStm->Read(&socket_data.meta_data,sizeof(socket_data.meta_data),&l));

					OLE_CHECK_hr(helper->unmarshal_BSTR(pStm,&socket_data.buffer_tail));
					OLE_CHECK_hr(helper->unmarshal_BSTR(pStm,&socket_data.header));

					

					return hr;
					
				}

				inline  HRESULT  on_get(DISPID id,const wchar_t* name,com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper){

					

					if(id==DISPID_VALUE)
					{
						VARIANT v={VT_BSTR};
						v.bstrVal=socket_data.header;
						result=v;
						return S_OK;
					}
					else
					{
						s_parser_t<wchar_t>::case_t<false> cs(name);

						if(arguments.is_method()&&cs(5,L"close"))
						{
							socket_data.close();
							return S_OK;
						}
						if(cs(2,L"fd"))
						{
							if (arguments.length() == 0) {
								result = int(socket_data.sock);
								return S_OK;
							}
							else {
								HRESULT hr;
								bstr_t psx = arguments[0].def<bstr_t>();
								int flags= arguments[1].def<int>(0);
								bool f = 0 == moniker_parser_t<>::str_cmp_ni((wchar_t*)psx, L"posix", 5);
								OLE_CHECK_hr_cond(f, E_INVALIDARG);//0x00787370
								//OLE_CHECK_hr_cond( psx == *((int*)"psx"), E_INVALIDARG);//0x00787370
								SOCKET s = dup_socket(socket_data.sock);
								OLE_CHECK_hr_cond(INVALID_SOCKET != s,WSAGetLastError());
								int fd = _open_osfhandle(INT_PTR(socket_data.sock), flags);
								
								if (fd == -1) {
									closesocket(s);
									return E_INVALIDARG;
								}

								else result = fd;
								return S_OK;
							}
							
						}

						
					}
						return E_NOTIMPL;
				};

				struct mbv_socket_context_t:base_aggregator_t<mbv_socket_context_t,i_mbv_socket_context>
				{
					mbv_socket_context_t(sh_socket_context_t* pc,iaggregator_container* container)
						:base_aggregator_t<mbv_socket_context_t,i_mbv_socket_context>(container)
						,pcontext(pc){}	
					~mbv_socket_context_t(){
						OutputDebugStringW(L"~mm_socket_context_t\n");
					}
					 

					inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ) 
					{              
						
						bool f;
						f  =(riid==__uuidof(i_mbv_socket_context));
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

					virtual HRESULT STDMETHODCALLTYPE GetContext(void** ppcontext){							
						OLE_CHECK_PTR(ppcontext);
						*ppcontext=(void*)&(pcontext->socket_data);
						return S_OK;
					}

					

					sh_socket_context_t* pcontext;

				};

				


				HRESULT set_aggregator_helpers(iaggregator_container* container,pcallback_lib_arguments_t plib)
				{

					HRESULT hr;
					 
					
					

					smart_ptr_t<iaggregator_helper> helper=
						aggregator_helper_creator_t<mbv_socket_context_t>::New(new	mbv_socket_context_t(this,container));					
                    

					OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mbv_socket_context),helper));					
					

					return hr;
				}






				mm_socket_context_data_impl_t socket_data;



			};







		}; //mbv_socket
	
};// ipc_utils
