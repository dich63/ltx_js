#pragma once

#include "marshal_by_value_base.h"
#include "ipc_utils.h"
#include "sa_utils.h"
#include "sm_region.h"
#include "sparse_file_map.h"
#include "singleton_utils.h"

namespace ipc_utils {	

		namespace mbv_kernel_object{

			//
			// 
			//struct mm_kernel_object_t:kernel_object_t{};		
			typedef kernel_object_t mm_kernel_object_t;

			typedef shared_socket_data_t::uint64_t uint64_t; 
			typedef LONGLONG int64_t; 
			
			typedef moniker_parser_t<wchar_t> parser_t;

			struct SDS_t 
			{
				SECURITY_DESCRIPTOR sd;   
				SECURITY_ATTRIBUTES _sa;
				HRESULT hr;
				SDS_t()
				{
					hr=S_OK;
					bool f=InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)
						&&SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
					if(f)
					{
						SECURITY_ATTRIBUTES t={sizeof(SECURITY_ATTRIBUTES),&sd,false}; 
						_sa=t;

					}
					else hr=GetLastError();

				}

				inline SECURITY_ATTRIBUTES* operator()(bool fremote)
				{
					return ((hr==S_OK)&&fremote)?&_sa:NULL;  
				}
			};


			





			


			struct mm_kernel_object_impl_t : mm_kernel_object_t
			{
				mm_kernel_object_impl_t():mm_kernel_object_t(){  					

				}
				~mm_kernel_object_impl_t(){ close();}

				void close()
				{

                    mm_kernel_object_t t=ipc_utils::make_detach<mm_kernel_object_t>(*this,mm_kernel_object_t());
					ipc_utils::safe_close_handle( t.hko);					
					SysFreeString(t.type);
					SysFreeString(t.name);

				}

				


			};

			
			


            struct  __declspec(uuid("{E943235D-20CF-4736-95FC-5BD160655012}"))  co_mm_kernel_object_t{};

			//struct  sh_kernel_object_t: marshal_by_value_base::mbv_context_base_t<sh_kernel_object_t,__uuidof(co_mm_kernel_object_t),true>
			struct  sh_kernel_object_t : marshal_by_value_base::mbv_context_base_c_t<sh_kernel_object_t,co_mm_kernel_object_t, true>
			{



				sh_kernel_object_t(){
					
					//locker_mutex_t lock(hmutex);

				}

				~sh_kernel_object_t(){		
					

				}

				inline HRESULT set_type_name(bstr_t& t,bstr_t& n)
				{
					kernel_object.type=t.Detach();
					kernel_object.name=n.Detach();
					return S_OK;
				}

				HRESULT CreateKO(IBindCtx* bctx,LPOLESTR pszDisplayName){

					HRESULT hr;
					parser_t parser(pszDisplayName,bctx);


					bool fsa=parser.bind_args(L"fsa").def<int>(0);

					SDS_t& sds=class_initializer_T<SDS_t>::get();




					/*
					if(!parser.args.is(L"type")) 
						           return S_OK;
								   */
					bstr_t type=parser.bind_args(L"type").def<bstr_t>();
					
					
					

					bstr_t name=parser.bind_args(L"name").def<bstr_t>();


					HANDLE hko = (HANDLE)parser.bind_args(L"handle").def<int>(-1);


					
					
					if(CHECK_HANDLE(hko))
					{
						OLE_CHECK_hr_cond(ipc_utils::check_handle(hko), GetLastError());

						bool f=parser.bind_args(L"handle.attach").def<int>(0);
						kernel_object.hko=(f)?hko:ipc_utils::dup_handle(hko);
						return set_type_name(type,name);
					}

					if (!type.length())
						return S_OK;


					int flags=parser.bind_args(L"flags").def<int>(0);
                    

					wchar_t* pname=name.length()?(wchar_t* )name:(wchar_t* )0;
                	  pname=parser.undecorateQQ(pname);

					s_parser_t<wchar_t>::case_t<false> cs=(wchar_t*) type;

					



    

					if(cs(L"job"))
					{
						HANDLE hko;
						if(flags==2)
						{
                         OLE_CHECK_hr_cond(kernel_object.hko=hko=::OpenJobObjectW(JOB_OBJECT_ALL_ACCESS,0,pname),GetLastError());
						}
						else{
						OLE_CHECK_hr_cond(kernel_object.hko=hko=::CreateJobObjectW(sds(fsa),pname),GetLastError());

						HRESULT hrl=GetLastError();

						

						if(hrl!=ERROR_ALREADY_EXISTS)
						{

							DWORD flags=parser.bind_args(L"flags").def<int>(JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION|JOB_OBJECT_LIMIT_BREAKAWAY_OK|JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE);

							BOOL f;


							JOBOBJECT_EXTENDED_LIMIT_INFORMATION j={};
							j.BasicLimitInformation.LimitFlags=flags;
							OLE_CHECK_hr_cond(f=SetInformationJobObject(hko,JobObjectExtendedLimitInformation,&j,sizeof(j)),GetLastError());
							
						}	
						else{
							OLE_CHECK_hr_cond(flags!=1,hrl);
						}
						

						}
						return set_type_name(type,name);
					}
					if(cs(L"event"))
					{
												
						bool fmr=parser.bind_args(L"manual").def<bool>(true);
						bool fis=parser.bind_args(L"signaled").def<bool>(false);
						OLE_CHECK_hr_cond(kernel_object.hko=::CreateEventW(0,fmr,fis,pname),GetLastError());						
						return set_type_name(type,name);
					}
					if (cs(L"process"))
					{

						int pid = parser.bind_args(L"pid").def<int>(0);
						
						if (pid) {


							if (pid == -1) pid = GetCurrentProcessId();
							
							DWORD dwDesiredAccess = parser.bind_args(L"DesiredAccess").def<int>(PROCESS_ALL_ACCESS);
							OLE_CHECK_hr_cond(kernel_object.hko = ::OpenProcess(dwDesiredAccess,0,pid), GetLastError());
							name = variant_t(pid);
						}
						//OLE_CHECK_hr_cond(kernel_object.hko = ::CreateMutexW(0, fis, pname), GetLastError());
						return set_type_name(type, name);
					}
					if (cs(L"mutex"))
					{
												
						bool fis = parser.bind_args(L"signaled").def<bool>(false);
						OLE_CHECK_hr_cond(kernel_object.hko = ::CreateMutexW(0,fis, pname), GetLastError());
						return set_type_name(type, name);
					}
					if(cs(L"iocp"))
					{
						OLE_CHECK_hr_cond(kernel_object.hko=::CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0),GetLastError());	
						kernel_object.type=type.Detach();
						return S_OK;

					}
					if(cs(L"file"))
					{
						HANDLE h;
						DWORD DesiredAccess=(GENERIC_READ|GENERIC_WRITE)
							,ShareMode=(FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE)
							,CreationDisposition=(OPEN_ALWAYS)
							,FlagsAndAttributes;

						if(pname)
							FlagsAndAttributes=FILE_ATTRIBUTE_NORMAL;
						else {
							pname=parser.tmp_file_name2(L"~#");
							FlagsAndAttributes=FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE;
						}			
						bind_args_update_hexbin(parser,DesiredAccess,int);
						bind_args_update_hexbin(parser,ShareMode,int);
						bind_args_update_hexbin(parser,CreationDisposition,int);
						bind_args_update_hexbin(parser,FlagsAndAttributes,int);


						//if (FlagsAndAttributes&(FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE))
							//FatalAppExitW(0, pname);

							/*

						DesiredAccess=parser.bind_args(L"DesiredAccess").def<int>(GENERIC_READ|GENERIC_WRITE);
						ShareMode=parser.bind_args(L"ShareMode").def<int>(FILE_SHARE_WRITE|FILE_SHARE_READ);
						CreationDisposition=parser.bind_args(L"CreationDisposition").def<int>(OPEN_ALWAYS);
						FlagsAndAttributes=parser.bind_args(L"FlagsAndAttributes").def<int>(FlagsAndAttributes);
						*/

						h=CreateFileW(pname,DesiredAccess,ShareMode,sds(fsa),CreationDisposition,FlagsAndAttributes,0);

						OLE_CHECK_hr_cond(h!=INVALID_HANDLE_VALUE,GetLastError());
						kernel_object.hko=h;

						return set_type_name(type,bstr_t(pname));


							/*_In_     DWORD                 dwShareMode,
							_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							_In_     DWORD                 dwCreationDisposition,
							_In_     DWORD                 dwFlagsAndAttributes,*/

					}



					return E_INVALIDARG;
				}


				HRESULT Init(BOOL fUnmarshal,IBindCtx* bctx,LPOLESTR pszDisplayName,DWORD& flags)
				{					
					flags=(1<< MSHCTX_INPROC)|0x10000;
					if(fUnmarshal)
						return S_OK;				
					else 	return CreateKO(bctx,pszDisplayName);

				}


				HRESULT marshal_to_process(i_marshal_helper_t* helper,DWORD pid_target,IStream* pStm){

					HRESULT hr;
					ULONG l;
					
					OLE_CHECK_hr(helper->marshal_hko(pStm,kernel_object.hko));						

					OLE_CHECK_hr(helper->marshal_BSTR(kernel_object.type,pStm));
					OLE_CHECK_hr(helper->marshal_BSTR(kernel_object.name,pStm));					

					

					return hr;
				}
				HRESULT unmarshal(i_marshal_helper_t* helper,IStream* pStm){
					HRESULT hr;
					ULONG l;
					
					OLE_CHECK_hr(helper->unmarshal_hko(pStm,&kernel_object.hko));											
					OLE_CHECK_hr(helper->unmarshal_BSTR(pStm,&kernel_object.type));
					OLE_CHECK_hr(helper->unmarshal_BSTR(pStm,&kernel_object.name));
					

					return hr;
					
				}

				inline  HRESULT  on_get(DISPID id,const wchar_t* name,com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper){

					if(id==DISPID_VALUE)
					{
						s_parser_t<wchar_t> p;
						result=(wchar_t*)p.printf(L"{\t\"hko\":%d,\t\"type\":\"%s\",\t\"name\":\"%s\"\t}",kernel_object.hko,kernel_object.type,kernel_object.name);
						return S_OK;
					}
					else{
					   s_parser_t<wchar_t>::case_t<false> cs(name);
					   if(cs(L"handle"))
					   {
						   result=double(INT_PTR(kernel_object.hko));
						   return S_OK;
					   }
					   if(cs(L"type"))
					   {
						   result=kernel_object.type;
						   return S_OK;
					   }
					   if(cs(L"name"))
					   {
						   result=kernel_object.name;
						   return S_OK;
					   }
					   if(cs(L"close"))
					   {
						   result=(bool)ipc_utils::safe_close_handle(ipc_utils::make_detach(kernel_object.hko));
						   return S_OK;
					   }



					}


					
						return E_NOTIMPL;
				};

				struct mbv_kernel_object_t:base_aggregator_t<mbv_kernel_object_t,i_mbv_kernel_object>
				{
					mbv_kernel_object_t(sh_kernel_object_t* pc,iaggregator_container* container)
						:base_aggregator_t<mbv_kernel_object_t,i_mbv_kernel_object>(container)
						,pcontext(pc){}	
					~mbv_kernel_object_t(){
						OutputDebugStringW(L"~mbv_kernel_object_t_t\n");
					}
					 

					inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ) 
					{              
						
						bool f;
						f  =(riid==__uuidof(i_mbv_kernel_object));
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
						*ppcontext=(void*)&pcontext->kernel_object;
						return S_OK;
					}

					

					sh_kernel_object_t* pcontext;

				};

				


				HRESULT set_aggregator_helpers(iaggregator_container* container,pcallback_lib_arguments_t plib)
				{

					HRESULT hr;
					 
					
					

					smart_ptr_t<iaggregator_helper> helper=
						aggregator_helper_creator_t< mbv_kernel_object_t>::New(new	 mbv_kernel_object_t(this,container));					
                    

					OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mbv_kernel_object),helper));					
					

					return hr;
				}






				mm_kernel_object_impl_t kernel_object;



			};







		}; //mbv_socket
	
};// ipc_utils
