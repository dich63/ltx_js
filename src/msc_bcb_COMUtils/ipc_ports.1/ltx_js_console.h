#pragma once
//#include "ltx_js_console.h"
#define LTX_SCRIPT_OBJREF L"ltx.{61F8A798-2A81-4d98-B3C5-543E558B1468}"
#include "ipc_ports/com_process.h" 

#undef GetObject

namespace ipc_utils {
	namespace com_apartments
	{
		namespace script_engine
		{



            const wchar_t* 	get_LTX_SCRIPT_OBJREF()
			{
				return (sizeof(void*)==4)? LTX_SCRIPT_OBJREF L".x86":LTX_SCRIPT_OBJREF L".x64";
			}




			template <int _VERS=0>
			struct ltx_js_server_t
			{


				static	HRESULT console_holder(const wchar_t* lpszCmdLine)
				{

					//				template <class T ,class IDispatchX=IDispatch,class RefCounter=RefCounterMT_t<1> >
					//				struct IDispatch_base_t:IDispatch_impl_base_t<T,IUnknown_base_impl<T,IDispatchX,RefCounter > >{};
					/*struct hpair_t
					{
						hpair_t()
						{
							h[0]=hevent.hko;
							h[1]=hevent2.hko;
							
						}
						event_local_t hevent;
						event_local_t hevent2;
						HANDLE h[2];
					};*/
					struct event_disp_t:IDispatch_base_t<event_disp_t,IDispatch,RefCounterFake_t<1> >
					{

						inline	HRESULT impl_Invoke(DISPID id,REFIID,LCID,WORD,DISPPARAMS* pdp ,VARIANT*,EXCEPINFO*, UINT*)
					 {
						 if(id==0) {

							 arguments_t<variant_t> args(pdp);

							 bool f=args[0];
							 if(f) hevent2.signal();
							 else hevent.signal();

							 //hevent.signal();
							 return S_OK;
						 } 
						 return E_NOTIMPL;}


						//hpair_t hp;
						event_local_t hevent,hevent2;
						//inline operator HANDLE(){ return hevent.hko;}
						//inline operator PHANDLE(){ return hp.h;}
						inline DWORD wait(DWORD tio){
							HANDLE h[2]={hevent.hko,hevent2.hko};
                            DWORD ws=WaitForMultipleObjects(2,h,false,tio);
							return ws;
						}

					} event_disp;




					HRESULT hr;
					argv_cmdline<wchar_t> argscmd( lpszCmdLine);

					LockSetForegroundWindow(LSFW_UNLOCK);
					AllowSetForegroundWindow(GetCurrentProcessId());
					unsigned int ui_hwnd=argscmd[L"console.hwnd"].def<int>(0);
					moniker_parser_t<wchar_t> mp;
					wchar_t* engine_opts=argscmd[L"engine.opts"];
					HWND hwinc=HWND(ui_hwnd);
					 process_com_t::get_instance().console_hwin(1,hwinc);

						 




					if(!(ui_hwnd&&IsWindow(hwinc))) return E_INVALIDARG;
					const wchar_t* prfx=argscmd[L"console.prefix"];
					if(!prfx)  return E_INVALIDARG;
					console_shared_data_t<wchar_t> csd((wchar_t*)prfx);






					COMInitF_t ci;

					smart_ptr_t<IDispatch> external_obj;
					OLE_CHECK_hr(hr=super_factory_t::CoGetObject(bstr_t(L"ltx.bind:external"),__uuidof(IDispatch),external_obj._ppQI()));

					smart_ptr_t<IDispatch> ec_locker,engine;	



					OLE_CHECK_hr(super_factory_t::CoGetObject(bstr_t(L"ltx.bind:ec_locker:  global=3 ;"),__uuidof(IDispatch),ec_locker._ppQI()));
					OLE_CHECK_hr(super_factory_t::CoGetObject(bstr_t(L"ltx.bind:script:")+bstr_t(engine_opts),__uuidof(IDispatch),engine._ppQI()));
					// var ec_locker=GetObject("ltx.bind:ec_locker:  global=1 ;");
					wchar_t* pscript =js_text(
                        hwndcons=  bindObject('dll: lib=**; proc=ltx_callback_list::win_console')(4);    
						quit=(function(o){ 
							var __signalQuit__=o;
							return function Quit(fsafe)
							{                         
								__signalQuit__(fsafe);
							}
					})($$[0]);
                      bindObject("ltx.bind:external")[0]; 
					);
                      //bindObject("ltx.bind:external").def(0,''); 
                    variant_t vexternal_moniker;

					//OLE_CHECK_hr(call_disp(engine,&vexternal_moniker,DISPID(0),pscript,variant_t((IDispatch*)&event_disp)));
					OLE_CHECK_hr(call_disp(engine,&vexternal_moniker,DISPID(0),pscript,variant_t((IDispatch*)&event_disp)));


					DWORD ws=WAIT_FAILED;



					{
						VARIANT vh={};

						//OLE_CHECK_hr(call_disp(ec_locker,L"handle",&vh,variant_t(int(GetCurrentProcessId()))));
						//HANDLE hev=HANDLE(vh.ullVal);
						//HANDLE* phev=event_disp;

						stub_holder_t stub(engine.p);
						bstr_t objref=stub.objref();
						{					
							locker_t<console_shared_data_t<wchar_t> > lock(csd);
							argv_zzs<wchar_t> argsc=csd;
							argsc[get_LTX_SCRIPT_OBJREF()]=(wchar_t*)objref;
							argsc[L"console.server.pid"]=GetCurrentProcessId();

							csd=argsc;							 		 				
						}

						variant_cast_t<VT_BSTR> vmon(vexternal_moniker);
						wchar_t* enval= (vmon)?vmon->bstrVal:0;
						SetEnvironmentVariableW(LTX_PROCESS_OBJREF,enval);

						OLE_CHECK_hr(call_disp(external_obj,L"result",0,(wchar_t*)objref));

						external_obj.Release();	
						
						/*
						if(sizeof(void*)>4) 
							SetWindowTextW(hwinc,v_buf<wchar_t>().printf(L"ltx.js.console[x64] pid=%d",GetCurrentProcessId()).get());
						else SetWindowTextW(hwinc,v_buf<wchar_t>().printf(L"ltx.js.console[win32] pid=%d",GetCurrentProcessId()).get());
						*/



						while(IsWindow(hwinc)&&(WAIT_TIMEOUT==(ws=event_disp.wait(250)))){};

					}
					//


					if(WAIT_OBJECT_0!=ws)
						OLE_CHECK_hr(call_disp(ec_locker,L"wait",0,int(-1)));   

					//mta_singleton_t::stop_and_wait(5000);
					int cfull=apartment_list_t<>::get_instance().count();
					IsWindow(hwinc)&&SetWindowTextW(hwinc,v_buf<wchar_t>().printf(L"ltx.js.server terminating... [%d active apartment]",cfull));
					
					int cbad=apartment_list_t<>::get_instance().stop_all(5000);
					IsWindow(hwinc)&&SetWindowTextW(hwinc,v_buf<wchar_t>().printf(L"ltx.js.server died! [%d [at %d] zombie apparments]",cbad,cfull));


					return hr;
				}
			};




			template <int _VERS=0>
			struct ltx_js_console_t
			{
				HRESULT hr;
				wchar_t consname[256];
				console_shared_data_t<wchar_t> csd0;
				static void set_icon()
				{
					HMODULE hke=GetModuleHandleW(L"kernel32.dll");
					HMODULE hkl=module_t<>::__getHinstance();
					HICON hico;

					BOOL (WINAPI *SetConsoleIcon)(HICON);
					if(SET_PROC_ADDRESS(hke,SetConsoleIcon))
					{

						if(hico=LoadIconW(hkl,(sizeof(void*)==4)?L"#108":L"#105"))
							SetConsoleIcon(hico);
					}	  


				}

				void print(const wchar_t * fmt,...)
				{

					va_list argptr;
					va_start(argptr, fmt);
					v_buf<wchar_t> buf;

					csd0.puts(buf.vprintf(fmt,argptr).get());		

				}

				void print_attr(DWORD attr,const wchar_t * fmt,...)
				{
					//		return;
					va_list argptr;
					va_start(argptr, fmt);
					v_buf<wchar_t> buf;
					csd0.puts(buf.vprintf(fmt,argptr).get(),attr);
				}




				static size_t send_console_input(const wchar_t* pwc)
				{

					DWORD cb=safe_len(pwc);
					if(!cb) return 0;

					HANDLE hin=GetStdHandle(STD_INPUT_HANDLE);
					if(INVALID_HANDLE_VALUE==hin) return 0;
					INPUT_RECORD* pinp= (INPUT_RECORD*)::calloc(2*cb,sizeof(INPUT_RECORD));

					for(int n=0;n<cb;n++)
					{
						INPUT_RECORD& inr_d=pinp[2*n];
						inr_d.EventType	=KEY_EVENT;
						KEY_EVENT_RECORD& ker=inr_d.Event.KeyEvent;
						ker.bKeyDown=true;
						ker.wRepeatCount=1;
						ker.uChar.UnicodeChar=pwc[n];
						(pinp[2*n+1]=inr_d).Event.KeyEvent.bKeyDown=false;

					}

					bool f;

					if(!WriteConsoleInput(hin,pinp,2*cb,&cb))
						cb=0;
					::free(pinp);
					return cb;
				};

				static HRESULT dispcall_1(IDispatch* p, variant_t param,VARIANT* pvarRet=NULL,EXCEPINFO* pei=NULL)
				{
					HRESULT hr;
					DISPPARAMS dispparams = { &param, NULL, 1, 0};
					variant_t vr;
					if (!pvarRet) pvarRet=&vr;
					return hr=p->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD, &dispparams, pvarRet, pei, NULL);

				}

				static HRESULT dispcall_2(IDispatch* p, variant_t param, variant_t param2,VARIANT* pvarRet=NULL,EXCEPINFO* pei=NULL)
				{
					VARIANT vv[2]={param2,param};
					DISPPARAMS dispparams = { vv, NULL, 2, 0};
					variant_t vr;
					if (!pvarRet) pvarRet=&vr;
					return p->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD, &dispparams, pvarRet, pei, NULL);

				}



				static inline wchar_t* read_line(HANDLE hin,v_buf<wchar_t>& buf)
				{
					wchar_t c;
					DWORD cbout;
					std::wstringstream stream;

					bool f;
					while(f=ReadConsoleW(hin,&c,1,&(cbout=0),0))
					{

						if(c==L'\\') break;
						stream<<c;

					}

					buf.cat(stream.str().c_str());
					return buf.get();
				}


				HRESULT init(const wchar_t* cmdline)
				{

					DWORD pid;
					HWND hwndc;
					moniker_parser_t<wchar_t> mn;
					wchar_t* cmdl=mn.trim(cmdline);

					wchar_t inputchar[5]=L"#\0";
					GetEnvironmentVariableW(L"__ltx_cmd__.noline",inputchar,5);

					if(!GetEnvironmentVariableW(L"__ltx_cmd__",consname,254))
						return hr=HRESULT_FROM_WIN32(GetLastError());
					//wcscat(consname,L" ");
					if(!AttachConsole(ATTACH_PARENT_PROCESS))
					{
						AllocConsole();				   
						int f=start_process("cmd.exe /K prompt :$G&cls ",0);
						if(!f)
							return hr=HRESULT_FROM_WIN32(GetLastError());


					}
					if(!(hwndc=GetConsoleWindow()))
						return hr=HRESULT_FROM_WIN32(GetLastError());
                    
                    set_icon();
					console_info_t coni;
					coni.set_alpha(85);

					console_shared_data_t<wchar_t> csd;

					/*
					DWORD tid;
					if(!(tid=GetWindowThreadProcessId(hwndc,&pid))) 
					return hr=HRESULT_FROM_WIN32(GetLastError());
					*/

					//wchar_t wcn[256];
					//GetClassNameW(hwndc,wcn,256);


					//
					COMInitF_t ci;
					//COMInitA_t ci;
					smart_ptr_t<IDispatch> js_server;
					DWORD server_pid;


					{				   

						locker_t<console_shared_data_t<wchar_t> > lock(csd);
						argv_zzs<wchar_t> argsc=csd;
						const wchar_t* pobjref=argsc[get_LTX_SCRIPT_OBJREF()];	
						server_pid=argsc[L"console.server.pid"];


// set mode
						int fscript_mode;
						bool freset;   
						wchar_t*    pscript_mode=_wgetenv(L"ltx.script.mode");
						fscript_mode=get_double(pscript_mode);


						wchar_t* ptail,*p;
						if(freset=mn._lcheckni(cmdl,L":reset:",-1,&ptail))
						{
							if(p=mn._lskipi(ptail,L":"))
							{
								fscript_mode=safe_atof(ptail);								
								cmdl=mn.get_cache(p);
							} 
							else cmdl=mn.get_cache(ptail);
						}




						
						//if(pobjref)
						//print(L"? %s=%s\n",LTX_SCRIPT_OBJREF,pobjref);
						//if(pobjref){ OLE_CHECK_hr(::CoGetObject(pobjref,0,__uuidof(IDispatch),js_server._ppQI()));}
						if(pobjref&&SUCCEEDED(hr=::CoGetObject(pobjref,0,__uuidof(IDispatch),js_server._ppQI()))&&freset)
						{
							hr=dispcall_1(js_server,L"quit()");
                            hr=E_FAIL;
						};

						if(FAILED(hr))
						{

           
							bstr_t bindstr=v_buf<wchar_t>().printf(
								L"ltx.bind:process: file=rundll32.exe;cmd=\"%s\",js_console_holder console.hwnd=%d console.prefix=%s \"engine.opts=debug=%d;\""
								,(wchar_t*)module_t<>::module_name(),(int)hwndc,(wchar_t*)csd._prefix(),fscript_mode);

							smart_ptr_t<IDispatch> process;
							//OLE_CHECK_hr(CoGetObject(bindstr,0,__uuidof(IDispatch),process._ppQI()));
							OLE_CHECK_hr(super_factory_t::CoGetObject(bindstr,__uuidof(IDispatch),process._ppQI()));
							variant_t res;

							{	
								v_buf_fix<wchar_t> buf;
							    GetEnvironmentVariableW(LTX_PROCESS_OBJREF,buf,buf.count());
								unlocker_t<console_shared_data_t<wchar_t> > unlock(csd);
								OLE_CHECK_hr(dispcall_1(process,(wchar_t*)buf,&res));
							}
							variant_cast_t<VT_BSTR> vc(res);
							OLE_CHECK_hr(vc.hr);
							pobjref=vc->bstrVal;
							argv_zzs<wchar_t> argsc2=csd;
							server_pid=argsc2[L"console.server.pid"];

							OLE_CHECK_hr(::CoGetObject(pobjref,0,__uuidof(IDispatch),js_server._ppQI()));


							smart_ptr_t<IDispatch> js_server2;


							hr=::CoGetObject(pobjref,0,__uuidof(IDispatch),js_server2._ppQI());
							//argsc[LTX_SCRIPT_OBJREF]=pobjref;
							//csd=argsc;
						}

						if(sizeof(void*)>4) 
							SetConsoleTitleW(v_buf<wchar_t>().printf(L"ltx.js.console[x64] pid=%d",server_pid).get());
						else SetConsoleTitleW(v_buf<wchar_t>().printf(L"ltx.js.console[win32] pid=%d",server_pid).get());


						variant_t res;
						ipc_utils::exceptinfo_t exinfo;

						

						int c;
						c=safe_len(cmdl);					   
						wchar_t lastchar=(c)?cmdl[c-1]:0;
						if((c)&&(lastchar==inputchar[0])) cmdl[c-1]=0;
						///*
						if(lastchar==L'\\')
						{
							HANDLE hin=GetStdHandle(STD_INPUT_HANDLE);
							DWORD dwm=0;
							if(GetConsoleMode(hin,&dwm))
								SetConsoleMode(hin,dwm|ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT);

							v_buf_fix<wchar_t> bufF;
							v_buf<wchar_t>& bufcmd= mn.get_cache();
							if(c) cmdl[c-1]=0;
							wchar_t* pread=bufF;
							bufcmd.cat(cmdl);
							DWORD cdw;

							cmdl=read_line(hin,bufcmd);

						}
						// */


						if(SUCCEEDED(hr=dispcall_1(js_server,cmdl,&res,&exinfo)))
						{
							if(lastchar!=L';')
							{


								variant_cast_t<VT_BSTR> vc(res);
								if(vc) print_attr(M_BC|0x0b,L"%s\n",(wchar_t*)vc->bstrVal);	 
							}
						}
						else 
						{
							print_attr(M_BC|0x0c,L"error=%s[%08x]\n",(wchar_t*)error_msg(exinfo.scode),exinfo.scode);						   
							print_attr(M_BC|0x0c,L"Source: %s\n",(wchar_t*)exinfo.bstrSource);
							print_attr(M_BC|0x0c,L"Description: %s\n",(wchar_t*)exinfo.bstrDescription);				   
							hr=S_FALSE;
						}

						if(lastchar!=inputchar[0]) send_console_input(wcscat(consname,L" "));

					}





				}


				ltx_js_console_t(const wchar_t* cmdline):hr(E_POINTER)
				{

					

					init(cmdline);
					if(FAILED(hr))
						print_attr(0x0F|0x40,L"error=%s[%08x]\n",(wchar_t*)error_msg(hr),hr);


				}



			};

		}; // script_engine


	}; // com_apartments
}; //ipc_utils

