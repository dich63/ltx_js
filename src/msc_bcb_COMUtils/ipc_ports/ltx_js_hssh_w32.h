#pragma once

#include "video/singleton_utils.h"

#include "singleton_utils.h"
#include "ltxjs.h"

#include "hssh.h"

#include "wbs_buffer.h"
#include "ipc_utils.h"
#include "moniker_parsers.h"
#include "static_constructors_0.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <mstcpip.h>
typedef ipc_utils::com_scriptor_t javascript_t;
typedef ipc_utils::dispcaller_t<javascript_t::exception_t> caller_t;
typedef javascript_t::value_t jsvalue_t;
typedef i_mbv_buffer_ptr::int64_t int64_t;
typedef ltx_helper::arguments_t<jsvalue_t>  arguments_t;



#define CHECK_FAIL_(hr,a) if(FAILED(hr=(a))) \
	return hr;
#define CHECK_FAIL_hr(a)  CHECK_FAIL_(hr,a)

#define s_m(s) L#s
#define update_arguments(zz,x) zz[s_m(x)].update(x)
#define  update_args(a) update_arguments(args,a)



template <int VERS=0>
struct session_folder_t{


	//enum{		_S_IWRITE=       0x0080  	};

static	BOOL IsDots(const TCHAR* str) {
		if(_tcscmp(str,L".") && _tcscmp(str,L"..")) return FALSE;
		return TRUE;
	}
static	BOOL DeleteDirectory(const TCHAR* sPath) {
		
	      if(!sPath) return false;
	    HANDLE hFind;  // file handle
		WIN32_FIND_DATA FindFileData;

		TCHAR DirPath[MAX_PATH];
		TCHAR FileName[MAX_PATH];

		_tcscpy(DirPath,sPath);
		_tcscat(DirPath,L"\\*");    // searching all files
		_tcscpy(FileName,sPath);
		_tcscat(FileName,L"\\");

		hFind = FindFirstFile(DirPath,&FindFileData); // find the first file
		if(hFind == INVALID_HANDLE_VALUE) return FALSE;
		_tcscpy(DirPath,FileName);

		bool bSearch = true;
		while(bSearch) { // until we finds an entry
			if(FindNextFile(hFind,&FindFileData)) {
				if(IsDots(FindFileData.cFileName)) continue;
				_tcscat(FileName,FindFileData.cFileName);
				if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

					// we have found a directory, recurse
					if(!DeleteDirectory(FileName)) { 
						FindClose(hFind); 
						return FALSE; // directory couldn't be deleted
					}
					RemoveDirectory(FileName); // remove the empty directory
					_tcscpy(FileName,DirPath);
				}
				else {
					if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
						_wchmod(FileName, _S_IWRITE); // change read-only file mode
					if(!DeleteFile(FileName)) {  // delete the file
						FindClose(hFind); 
						return FALSE; 
					}                 
					_tcscpy(FileName,DirPath);
				}
			}
			else {
				if(GetLastError() == ERROR_NO_MORE_FILES) // no more files there
					bSearch = false;
				else {
					// some error occured, close the handle and return FALSE
					FindClose(hFind); 
					return FALSE;
				}

			}

		}
		FindClose(hFind);  // closing file handle

		return RemoveDirectory(sPath); // remove the empty directory

	};




	static bstr_t ltx_sessions()
	{


		//return L"~ltx_hssh_w32~{34959C05-2EE7-4126-A62E-1D2BA084C79E}";
		return L"~ltx_hssh_w32~";

	}

	static bstr_t random_name(const wchar_t * ext=L".tmp")
	{

		bstr_t b=ipc_utils::clsid_t(uuid_generate());
		return b;
	}
	session_folder_t():hr(E_NOTIMPL){};

HRESULT init(bstr_t name=random_name(),bstr_t dir=L"%TEMP%/"){

		moniker_parser_t<wchar_t> mp;
		fn=dir+ltx_sessions()+L"/"+name+L"/";
		wchar_t* pfn;

		OLE_CHECK_hr(mp.expand_file_path(fn,&pfn,0,0));
		full_fn=file_make_dir(pfn).get();
		fnlock=full_fn+L"/~.~lock~";
		ipc_utils::bstr_c_t<2048> moniker;
		moniker.printf(L"ltx.bind:mm_buffer:length=%d;name=%s;flags.temp=1",1024*1024,(wchar_t*)(fnlock));
		hr=CoGetObject(moniker,0,__uuidof(i_mm_region_ptr),mmlock._ppQI());		

		return hr;
		

	};
	~session_folder_t(){

		
		if(full_fn.length())
		{

			mmlock.Release();

		
		/*
		int cb=full_fn.length();
		wchar_t* p=full_fn;
		if(cb) p[cb-1]=0;
		*/
		wchar_t* p=fnlock;
		for(int k=0;k<200;k++)
		{
			if(!moniker_parser_t<wchar_t>::file_exists(p))
				break;
			Sleep(200);
		}


		//file_dir_delete(full_fn);
		int l;
		bstr_t tmp=(wchar_t*) full_fn;
        wchar_t* pf=tmp;

		if(l=tmp.length())
		{
			wchar_t& lc=*(pf+l-1);
			if((lc==L'\\')||(lc==L'/'))
				lc=0;

		

		 
		    for(int k=0;k<30;k++)
			{
				
				DeleteDirectory(pf);
				if(moniker_parser_t<wchar_t>::file_exists(pf))
					Sleep(200);
				else break;		 
				  				
			}
		}


		}
	}

	HRESULT hr;
	ipc_utils::COMInitF_t ci;
	ipc_utils::smart_ptr_t<i_mm_region_ptr> mmlock;
	bstr_t fn,full_fn,fnlock;


};



template <int DIG=__DIG>
struct ltx_hss_json_rpc_t
{
	typedef ltx_hss_json_rpc_t<DIG> json_rpc_t;

	typedef void (__stdcall * raise_error_proc)(const wchar_t* smessage,const wchar_t* ssource);

	
	struct job_t{
		HANDLE hjob;
		wcharbuf jobname;
		HRESULT hr;
		job_t(){
			 hr=S_OK;
			 BOOL f;
			 jobname=wstr_uuid();
			 hjob=CreateJobObjectW(NULL,jobname);
			 JOBOBJECT_EXTENDED_LIMIT_INFORMATION j={};
			 j.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION|JOB_OBJECT_LIMIT_BREAKAWAY_OK|JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			 OLE_CHECK_VOID_hr_cond(f=SetInformationJobObject(hjob,JobObjectExtendedLimitInformation,&j,sizeof(j)),GetLastError());
		}
		~job_t()
		{
			hjob&& CloseHandle(hjob);
		}
		inline const wchar_t* name(){
			return jobname;
		}
	} ;




	static  jsvalue_t zz2jsvalue(const char* pzz,jsvalue_t& v=jsvalue_t()){

		char_mutator<CP_UTF8,true> cm(pzz);
		wchar_t* pwzz=cm;
		int cbz=safe_len_zerozero(pwzz);
		
		v.Clear();
		v.vt=VT_BSTR;
		v.bstrVal=SysAllocStringByteLen(0,sizeof(wchar_t)*(cbz));
		memcpy(v.bstrVal,pwzz,sizeof(wchar_t)*cbz);		
		return v;
	}

	static  BSTR zz2bstr(const char* pzz,jsvalue_t& v=jsvalue_t()){
		return zz2jsvalue(pzz,v).bstrVal;
	}


	enum
	{
		bufsize=1<<DIG
	};

	inline static hssh_t& hsslib()
	{
		static hssh_t h;
		return h;
	}

	inline static v_buf<wchar_t>& filelog()
	{
		static v_buf<wchar_t> s_filelog;
		return s_filelog;
	}


	struct exceptinfo_t: EXCEPINFO
	{
		exceptinfo_t(){ memset(this,0,sizeof(exceptinfo_t));}
		~exceptinfo_t()
		{
			reset();
		}

		inline 	exceptinfo_t* reset()
		{
			SysFreeString( make_detach(bstrSource));
			SysFreeString(make_detach(bstrDescription));
			SysFreeString(make_detach(bstrHelpFile));
			return this;
		}

	};


	template <class CH>
	static void logmsg(const CH * fmt,...)
	{
		static console_shared_data_t<wchar_t,0> shared_console;
		va_list argptr;
		va_start(argptr, fmt);
		v_buf<CH> buf;
		//safe_cputs(buf.vprintf(fmt,argptr).get());
		shared_console.puts(buf.vprintf(fmt,argptr).get());
		if(!filelog().empty())
			append_to_file(filelog().get(),char_mutator<CP_THREAD_ACP>(buf.get()));		
	}

	template <class CH>
	static void logmsg_attr(unsigned attr,const CH * fmt,...)
	{
		static console_shared_data_t<wchar_t,0> shared_console;
		va_list argptr;
		va_start(argptr, fmt);
		v_buf<CH> buf;
		//safe_cputs(buf.vprintf(fmt,argptr).get());
		shared_console.puts(buf.vprintf(fmt,argptr).get(),attr);
		if(!filelog().empty())
			append_to_file(filelog().get(),char_mutator<CP_THREAD_ACP>(buf.get()));		
	}












	inline static  bool check_post(HSS_INFO* hovl)
	{
		WSABUF& r=hovl->buffers[HSD_REQUEST];
		return (hovl&&(r.buf)&&(r.len>6)&&(StrCmpNIA(r.buf,"POST",4)==0));
	}

	static  void   on_clone_process_terminate(void*p,PROCESS_INFORMATION* ppi)
	{
		DWORD ec=-1;
		GetExitCodeProcess(ppi->hProcess,&ec);
		DWORD attr=(ec)?(0x4f):(0x0f);
		logmsg_attr(attr,L"clone process[%d] terminated:{%d} \n",ppi->dwProcessId,ec);

	}
	static  void  __stdcall on_hss_js_accept_clone(HSS_INFO* hovl)
	{
		hssh_t& hssh=hsslib(); 
		HRESULT hr;
		hr=hssh.to_clone_process_ex(hovl,&on_clone_process_terminate,0);
		if(hr)
		{
			logmsg_attr(0xad,L"to_clone_process error[%x]:%s\n",hr,error_msg(hr).get());
		}
		else logmsg_attr(0x0f,L"clone process[%d] starting...\n",hovl->error_code);

	}

	static HRESULT set_keep_alive(SOCKET s)
	{
		HRESULT hr;
		tcp_keepalive alive={1,5*60*1000,1000};
		argv_ini<wchar_t> argini(L"tcp",L".ini");

		update_arguments(argini,alive.onoff);
		update_arguments(argini,alive.keepalivetime);
		update_arguments(argini,alive.keepaliveinterval);




		DWORD dwRet, dwSize;

		//alive.onoff = 1;
		//alive.keepalivetime = 30*1000;
		//alive.keepaliveinterval = 1000;

		dwRet = WSAIoctl(s, SIO_KEEPALIVE_VALS, &alive, sizeof(alive),
			NULL, 0, &dwSize, NULL, NULL);
		 
		 hr=(dwRet == SOCKET_ERROR)?HRESULT_FROM_WIN32(WSAGetLastError()):S_OK;
		return hr;

	}
	static int isx64(){
		SYSTEM_INFO si;
        GetNativeSystemInfo(&si);
		return (si.wProcessorArchitecture!=PROCESSOR_ARCHITECTURE_INTEL)?1:0;

	}

	static  void  __stdcall on_hss_js_accept(HSS_INFO* hovl)
	{
		ltx_hss_json_rpc_t* _this=(ltx_hss_json_rpc_t*) hovl->user_data;

		hssh_t& hssh=hsslib(); 

		ULONG st;
		HRESULT hr;
		ULONGLONG tc=GetTickCount64();
		//ltx_hss_json_rpc_t* _this=(ltx_hss_json_rpc_t*) hovl->user_data;
		int jsdbgmode=_this->jsdbgmode;
		int  fwbsdump=_this->wbsdump;

		//st=hssh.websocket_handshake_chain(hovl); 
		{
			//hssh.to_clone_process(hovl);
			//  return ;
		}
		//char *ppex=hssh.reset_SOCKET_DATA(hovl,HSD_WS_EXTENSION,HSF_SET,"deflate-stream",-1);
		char *ppex;
		//ppex=hssh.reset_SOCKET_DATA(hovl,HSD_WS_EXTENSION,HSF_SET,"deflate-stream",-1);
		//ppex=hssh.reset_SOCKET_DATA(hovl,HSD_WS_EXTENSION,HSF_SET," ",-1);
		st=hssh.HTTP_request_headers_chain(hovl);
		if(check_post(hovl))
		{
			// rpc_connect_t::single_json_rpc_t single_json_rpc(hovl);
			return;
		}
		st=hssh.websocket_handshake_reply(hovl);
		if(!(hovl->reply_state&1)) return ;


		char* ra=hssh.reset_SOCKET_DATA(hovl,HSD_REMOTE_INFO,0,0,0);
		logmsg_attr(M_BC|0x0e,"websocket connect[%s]\n",ra);


		/*

		tcp_keepalive alive={1,5*60*1000,1000};

		argv_ini<wchar_t> argini('tcp');

		
			update_arguments(argini,alive.onoff);
			update_arguments(argini,alive.keepalivetime);
			update_arguments(argini,alive.keepaliveinterval);
			



		DWORD dwRet, dwSize;

		//alive.onoff = 1;
		//alive.keepalivetime = 30*1000;
		//alive.keepaliveinterval = 1000;

		dwRet = WSAIoctl(hovl->s, SIO_KEEPALIVE_VALS, &alive, sizeof(alive),
			NULL, 0, &dwSize, NULL, NULL);
		if (dwRet == SOCKET_ERROR)
		*/

		if(FAILED(set_keep_alive(hovl->s)))
		{
			s_parser_t<wchar_t> sp;
			 wchar_t* p=sp.printf(L"WSAIoctl fail with code %x",WSAGetLastError());
			 FatalAppExitW(0,p);
		}







		//  rpc_connect_t rpc(hovl);

		int fv=hssh.wbs_version(hovl);

		ipc_utils::COMInitF_t ci;

		ipc_utils::smart_ptr_t<i_mbv_socket_context> socket_context;
		pshared_socket_data_t psocket_data;

		if( 
			SUCCEEDED(hr=socket_context.CoGetObject(L"ltx.bind:socket: socket=??"))
			&&SUCCEEDED(hr=socket_context->GetContext((void**)&(psocket_data=0)))
			)
		{

			SOCKET s=ipc_utils::make_detach(hovl->s,INVALID_SOCKET);

			WSAEventSelect(s,psocket_data->habort_event,FD_CLOSE);

			//psocket_data->sock=ipc_utils::dup_socket(s);
			psocket_data->sock=s;
			psocket_data->flags|=shared_socket_data_t::f_srv;

			char* sreq=hssh.reset_SOCKET_DATA(hovl,HSD_REQUEST,0,0,0);
			char* srep=hssh.reset_SOCKET_DATA(hovl,HSD_REPLY,0,0,0);
			char* sparams_zz=hssh.reset_SOCKET_DATA(hovl,HSD_PARAMS,0,0,0);

			//bstr_t
			//char_mutator


            argv_zz<char> zz_p(sparams_zz);
			char* lang=zz_p["lang"].def<char*>("JScript");
			
			

			int c;

			argv_zzs<wchar_t> wargs_sock;
			wargs_sock[L"handshake"]=char_mutator<CP_UTF8>(sreq);
			wargs_sock[L"reply"]=char_mutator<CP_UTF8>(srep);

			if(fwbsdump)
			{
				logmsg_attr(0x08,"handshake:\n%s\n",sreq);
				logmsg_attr(0x08,"reply:\n%s\n",srep);
			}

			const wchar_t* pwargs=wargs_sock.flat_str(0,0,&c);
			psocket_data->header=SysAllocStringLen(NULL,c);
			memcpy(psocket_data->header,pwargs,sizeof(wchar_t)*(c+1));

			
			//if(SOCKET_ERROR==)
			//  hr=HRESULT_FROM_WIN32(WSAGetLastError());	
			//::closesocket(s);

			session_folder_t<> session_folder;

			session_folder.init();


			try{

				

                      job_t job;
					  wchar_t* xos[2]={L"32",L"64"};
				  javascript_t js(ipc_utils::bstr_c_t<>().printf(L"ltx.bind:srv:[job=%s]:%s:script: lang=%s;debug=%d;imports=[json_RPC]"
					  ,job.name()
					  ,xos[isx64()]
					  ,char_mutator<CP_UTF8>(lang).get_pwc()
					  ,jsdbgmode
					  ).get());





				//js(L"var sock=$$[0];server_lock(true);")(jsock);

				//session_folder.mmlock.Release();
				int srv_pid=js(L"var __session_server__=new RPC_session_server($$[0],$$[1],$$[2],$$[3]);process.pid")(jsvalue_t(socket_context)
					,jsvalue_t(session_folder.full_fn)
					,jsvalue_t(session_folder.mmlock)
					,zz2jsvalue(sparams_zz));


                    logmsg_attr(0x0f,"session server process id=%d\n",srv_pid);
			 
               int ihr;
			   while( !(ihr=js(L"__session_server__.loopOnce(true)")()) );

				//for( js(L"__session_server__.loopOnce(true)")() );



				//WaitForSingleObject(psocket_data->habort_event,INFINITE);
				logmsg_attr(0x02,"close socket[%s]\n",ra);
				tc=GetTickCount64();
				
			}
			catch(javascript_t::exception_t& e)
			{
				bstr_t d=e.Description();    
				bstr_t s=e.Source();
				logmsg_attr(0x4f,L"Error =%08x %s",e.Error(),e.ErrorMessage());

			}

			SetEvent(psocket_data->habort_event);







		}


        tc=GetTickCount64()-tc;
		logmsg_attr(M_BC|0x0e,"disconnect[%s]: %d Ms\n",ra,int(tc));

	}

	template <class Args>
	ltx_hss_json_rpc_t( Args& argv):hport(0),hion_port(0)
	{

		hssh_t& hssh=hsslib(); 
		if(S_OK!=hssh.hr)
			TerminateProcess(GetCurrentProcess(),hssh.hr);

		HSS_INFO* hovl=0;
		HRESULT hr;
		void* pcallback;
		wchar_t* nsport=argv[L"port"];
		bool fcp=argv[L"oppc"].def<bool>(false);
		jsdbgmode=argv[L"jsdbg"].def<int>(0);
		wbsdump=argv[L"wbsdump"].def<int>(0);


		//ltx_hss_json_rpc_t<>(nsport,).wait();;
		//debugger_point_t dbg(1);
		

		fcloned=hssh.is_clone_process();

		ns_port<<nsport;
		//ns_ion_port.printf(L"%s.ion[%d]",nsport,GetCurrentProcessId());

		if(fcp&&fcloned)
		{
			hr=hssh.from_parent_process(&hovl);
			if(hr==S_OK) 
			{
				hovl->user_data=ULONG_PTR((void*)this);

				on_hss_js_accept(hovl);

			}

		}    
		else{

			pcallback=(fcp)?on_hss_js_accept_clone:on_hss_js_accept; 	 
			ns_ion_port.printf(L"%s.ion",nsport);
			hion_port=hssh.ion_channels(ns_ion_port.get()); 
			wchar_t* pport=nsport,*pion=ns_ion_port;
			hport=hssh.create_socket_portW(ns_port.get(),HSF_SRD_AUTO,pcallback,(void*)this);
		}
		//




	}
	~ltx_hss_json_rpc_t()
	{

		if(hport) hsslib().release(hport);
		if(hion_port) hsslib().release(hion_port);


	}

	DWORD wait(DWORD tio=INFINITE)
	{
		return hsslib().wait_signaled(hport,-1);
	}


	void * hport,*hion_port;
	v_buf<wchar_t> ns_port,ns_ion_port;
	bool fcloned;
	int jsdbgmode,wbsdump;


};