#pragma once

#include <io.h>

#include "video/singleton_utils.h"
#include "ipc_ports/hs_socket.h"

#include <time.h>
#include <stdio.h>
#include <map>
#include <string>
#include <memory>
#include "ions.h"
#include "http_dir_gen.h"

#define  CP_X CP_THREAD_ACP

#define s_m(s) char_mutator<CP_X>(#s)
#define _m(s) char_mutator<CP_X>(s)
#define get_m(a,s) a[s_m(s)]

//#define FILEIID  L"FD583F3CB33E4226B56558AF1F59C7C3"

struct hss_servises_t
{ 
	typedef ions_t<>  ions_channel_t;

	typedef hss_servises_t* this_t;

	typedef std::map<std::string,std::wstring> root_map_t;
	argv_zzs<wchar_t> mimes;

	struct handle_t
	{
		HANDLE h;
		handle_t(HANDLE _h):h(_h){}
		~handle_t(){ if(h&&(h!=INVALID_HANDLE_VALUE)) CloseHandle(h); }
		inline operator HANDLE(){ return h;} 

	};


	static   const  wchar_t* fileid()
	{
		static  const wchar_t fid[]=L"FD583F3CB33E4226B56558AF1F59C7C3";
		return fid;
	}



	struct transmit_packets_t
	{

		LPFN_TRANSMITPACKETS TransmitPackets; 
		HRESULT herr;
		transmit_packets_t(SOCKET s):TransmitPackets(0),herr(0)
		{

			GUID wiid=WSAID_TRANSMITPACKETS;
			DWORD cbr,err;
			err=WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,&wiid,sizeof(wiid),&TransmitPackets,sizeof(TransmitPackets),&cbr,0,0);
			if(err) herr=WSAGetLastError();
		}
		operator bool()
		{
			return TransmitPackets!=0;
		}
		HRESULT http_replay(HSS_INFO* hovl,wchar_t* filename)
		{
			const char* reply_header[2]={"HTTP/1.0 200 OK\n\n","HTTP/1.0 404 Not Found\n"};

			TRANSMIT_PACKETS_ELEMENT tpe[2]={{TP_ELEMENT_MEMORY},{TP_ELEMENT_FILE}};
			if(herr) return herr;

			struct file_holder_t
			{
				HANDLE hf;
				HRESULT hr;
				file_holder_t(wchar_t* filename)
				{
					hf=CreateFileW(filename,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
					hr=(hf==INVALID_HANDLE_VALUE)? GetLastError():0;
				}

				~file_holder_t()
				{
					if(hf!=INVALID_HANDLE_VALUE) CloseHandle(hf);
				}


			} file_holder(filename);

			BOOL f;
			HRESULT hr=file_holder.hr,hr0;
			DWORD cbt,flags=0;
			OVERLAPPED ovl={};
			//ovl.hEvent=WSACreateEvent();//hovl->hEvent;
			if(!hr)
			{

				//TRANSMIT_FILE_BUFFERS tfb={(void*)reply_header[0],strlen(reply_header[0]),0,0};

				v_buf<char> rh;
				rh<<reply_header[0];

				memset(tpe,0,sizeof(tpe));

				tpe[0].dwElFlags=TP_ELEMENT_MEMORY;
				tpe[0].cLength=rh.size_b();
				tpe[0].pBuffer=rh.get();


				tpe[1].dwElFlags=TP_ELEMENT_FILE;
				tpe[1].hFile=file_holder.hf;

				//v_buf<TRANSMIT_PACKETS_ELEMENT> tbb(2);
				//tbb[0]=tpe[0];
				//tbb[1]=tpe[1];



				f=TransmitPackets(hovl->s,tpe,2,0,0,0);
				//f=TransmitFile(s,file_holder.hf,0,0,&ovl,&tfb,0);
				f=f||((hr=WSAGetLastError())== ERROR_IO_PENDING);

				if(f)
				{ 
					if(!(f=WSAGetOverlappedResult(hovl->s,&ovl,&cbt,1,&flags)))
						hr=WSAGetLastError();
				}

			}
			else send(hovl->s,reply_header[1],strlen(reply_header[1]),0);

			//CloseHandle(ovl.hEvent);
			return hr;
		}

	};



	struct transmit_file_t
	{
		LPFN_TRANSMITFILE TransmitFile; 
		HRESULT herr;
		transmit_file_t(SOCKET s):TransmitFile(0),herr(0)
		{

			GUID wiid=WSAID_TRANSMITFILE;
			DWORD cbr,err;
			err=WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,&wiid,sizeof(wiid),&TransmitFile,sizeof(TransmitFile),&cbr,0,0);
			if(err) herr=WSAGetLastError();
		}

		transmit_file_t():TransmitFile(0),herr(0)
		{

			SOCKET s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			GUID wiid=WSAID_TRANSMITFILE;
			DWORD cbr,err;
			err=WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,&wiid,sizeof(wiid),&TransmitFile,sizeof(TransmitFile),&cbr,0,0);
			if(err) herr=WSAGetLastError();
			closesocket(s);
		}
		operator bool()
		{
			return TransmitFile!=0;
		}


		inline 	HRESULT transmit(SOCKET s,HANDLE hf,LONGLONG offset,LONG cb,LONG* pcbw,TRANSMIT_FILE_BUFFERS* ptfb=0)
		{
			HRESULT hr;
			BOOL f;
			OVERLAPPED ovl={};
			LARGE_INTEGER& l=*PLARGE_INTEGER(&offset);
			ovl.Offset=l.LowPart;
			ovl.OffsetHigh=l.HighPart;
			f=TransmitFile(s,hf,cb,0,&ovl,ptfb,0);
			DWORD flags=0;

			f=f||((hr=WSAGetLastError())== ERROR_IO_PENDING);

			if(f)
			{ 
				f=WSAGetOverlappedResult(s,&ovl,(DWORD*)pcbw,1,&flags);
				hr=(f)?S_OK:WSAGetLastError();
			}

			return hr;
		}

		HRESULT http_replay(SOCKET s,wchar_t* filename)
		{
			const char* reply_header[2]={"HTTP/1.0 200 OK\n\n","HTTP/1.0 404 Not Found\n"};
			const LONGLONG lmaxpack=0x040000000;
			if(herr) return herr;

			struct file_holder_t
			{
				HANDLE hf;
				HRESULT hr;
				LARGE_INTEGER size;

				file_holder_t(wchar_t* filename)
				{

					hf=CreateFileW(filename,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
					hr=(hf==INVALID_HANDLE_VALUE)? GetLastError():0;
					if(!hr)
					{
						BY_HANDLE_FILE_INFORMATION fi;
						if(GetFileInformationByHandle(hf,&fi))
						{
							size.LowPart=fi.nFileSizeLow; 
							size.HighPart=fi.nFileSizeHigh;
						}
						else hr=GetLastError();
					}

				}

				~file_holder_t()
				{
					if(hf!=INVALID_HANDLE_VALUE) CloseHandle(hf);
				}


			} file_holder(filename);

			BOOL f;
			HRESULT hr=file_holder.hr,hr0;
			DWORD cbt,flags=0;
			if(!hr)
			{
				OVERLAPPED ovl={};
				char buf[256];

				LONGLONG fsize=file_holder.size.QuadPart;

				//char *_i64toa(fsize,buf,10)

				v_buf<char> vheader;

				//vheader.printf("HTTP/1.0 200 OK\nContent-Length: %s\n\n",_i64toa(fsize,buf,10));

				//TRANSMIT_FILE_BUFFERS tfb={vheader.get(),vheader.size_b(),0,0},*ptfb=&tfb;



				//
				TRANSMIT_FILE_BUFFERS tfb={(void*)reply_header[0],strlen(reply_header[0]),0,0},*ptfb=&tfb;


				;

				LONGLONG n=fsize/lmaxpack;
				LONGLONG offset=0;
				LONG lr;
				lr=fsize-n*lmaxpack;

				LONG cbw=0;
				if(lr)
				{
					hr=transmit(s,file_holder.hf,offset,lr,&cbw,make_detach(ptfb));
					if(hr) return hr;
					if(lr!=(cbw-tfb.HeadLength)) return E_FAIL;
					offset=+lr;
				}

				for(int k=0;k<int(n);k++)
				{
					hr=transmit(s,file_holder.hf,offset,LONG(lmaxpack),&cbw,make_detach(ptfb));
					if(hr) return hr;
					if(LONG(lmaxpack)!=cbw) return E_FAIL;
					offset=+cbw;
				}



			}
			else send(s,reply_header[1],strlen(reply_header[1]),0);
			return hr;
		}

	};




	struct http_srv_t
	{
		typedef http_srv_t* this_t;
		std::wstring root,storage_root;
		hss_servises_t* owner;
		int fstorage;

		http_srv_t(hss_servises_t* _owner,std::wstring _root):owner(_owner),root(_root),fstorage(0){};
		http_srv_t(hss_servises_t* _owner):owner(_owner),fstorage(0){};


		inline		bool is_dir(wchar_t* fn,LONG64& sizeF)
		{
			//sizeF=0;

			LARGE_INTEGER& ll=*(PLARGE_INTEGER(&sizeF));
			WIN32_FILE_ATTRIBUTE_DATA fad;
			if(!GetFileAttributesExW(fn,GetFileExInfoStandard,&fad) ) return false;
			DWORD dw=FILE_ATTRIBUTE_DIRECTORY&fad.dwFileAttributes;
			ll.LowPart=fad.nFileSizeLow;
			ll.HighPart=fad.nFileSizeHigh;


			return dw;
		}




		bool check_ftype(wchar_t* fn)
		{
			DWORD ct,cb;
			wchar_t buf[512];
			if(ERROR_SUCCESS!=RegQueryValueExW(HKEY_CLASSES_ROOT,fn,NULL,&ct,(LPBYTE)buf,&(cb=512)))
			{

			}
			return false;
		}



		inline   HRESULT http_reply_file(wchar_t* filename)
		{
			HRESULT hr;

			//TF_USE_DEFAULT_WORKER 


			return hr;

		}



		static void   s_on_http_accept(HSS_INFO* hovl)
		{

			if(hovl&&hovl->user_data) this_t(hovl->user_data)->on_http_accept(hovl);
		}

		inline int http_send_end(HSS_INFO* hovl,const char* code){
			return hss_send(hovl,(void*)code,strlen(code));
		}


		bool http_upload(HSS_INFO* hovl){

            
			INT64 bytelen;
			char *ufp,*sbytelen;
			

			if(!(fstorage&&(ufp=hss_reset_SOCKET_DATA(hovl,HSD_UPLOAD_FILE_PATH))))
				return false;

			sbytelen=hss_reset_SOCKET_DATA(hovl,HSD_BODY_LENGTH);
			

			bytelen=safe_atof(sbytelen);

			if(bytelen<=0)
			{
				http_send_end(hovl,"HTTP/1.0 411 Length Required\r\n\r\n");
				return true;
			}


			
			std::wstring filename=storage_root+std::wstring(L"\\")+(wchar_t*)char_mutator<CP_UTF8>(ufp);
			const wchar_t* pfilename=filename.c_str();


			file_make_dir_check(pfilename);			

			handle_t hfile=::CreateFileW(pfilename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
			owner->logmsg_attr(M_BC|0x0e,L"uploading %I64d bytes to file: \"%s\" ... \n",(bytelen),pfilename);


			if(INVALID_HANDLE_VALUE==hfile){
				http_send_end(hovl,"HTTP/1.0 500 Internal Server Error\r\n\r\n");
				owner->logmsg_attr(M_BC|0x0c,L" open file: \"%s\" error \n",pfilename);
				return true;
			}


			int buf_size=(bytelen>0x10000)?0x10000:bytelen,cb,cbr,cbw,c;
			v_buf<char> buf(buf_size);
			char* pbuf=buf;
			BOOL f;
			HRESULT hr;

			wbs_getter_t<HSS_INFO> sgetter(hovl);



			while(bytelen>0){
              cbr=(bytelen>0x10000)?0x10000:bytelen;
              char *p=pbuf; 
			  cb=cbr;
			  do{
				   c=sgetter(p,cb);
				   cb-=c;
				   if(hr=sgetter.hr)
					   break;
				   
				   p+=c;
				   //ERROR_GRACEFUL_DISCONNECT
			  }
			  while(cb>0);

				  if((cb>0)&&hr)
					  break;

			  /*
			  cb= hss_recv(hovl,pbuf,cbr,0x010000);
			  if(!(cb==cbr))
				  break;			   
				  */
			   cb=cbr;
               f=WriteFile(hfile,pbuf,cb,(DWORD*)&(cbw=0),0);
			   if(!(f&&(cb==cbw)))
				   break;
			   bytelen-=cb;	  

			}			
           
            bool ferr=bytelen>0;

			if(ferr) {
                owner->logmsg_attr(M_BC|0x0c,L"error saving %I64d bytes to file: \"%s\" ... \n",(bytelen),pfilename);
				http_send_end(hovl,"HTTP/1.0 500 Internal Server Error\r\n\r\n");
			}
			else{
				http_send_end(hovl,"HTTP/1.0 200 OK\r\nAccess-Control-Allow-Origin: *\r\n\r\n");
				owner->logmsg_attr(M_BC|0x0a,L"file: \"%s\" uploaded Ok \n",pfilename);
			}
			return true;

		}  

		void  __stdcall on_http_accept(HSS_INFO* hovl)
		{
			ULONG st;
			//				return;
			st=::hss_HTTP_request_headers_chain(hovl); 

			if(http_upload(hovl))
				return;


			char* ra=hss_reset_SOCKET_DATA(hovl,HSD_REMOTE_INFO);
			char* phost=hss_reset_SOCKET_DATA(hovl,HSD_HOST);

			if(1) if(check_post(hovl))
		 {
			 owner->logmsg(L"POST++\n");  
		 }

			char * fn=hss_reset_SOCKET_DATA(hovl,HSD_HTTP_FILE);
			char* params=hss_reset_SOCKET_DATA(hovl,HSD_PARAMS);
			char_mutator<CP_UTF8> cm(params,0,1);
			v_buf<wchar_t> buf,buffn;
			//bstr_t bb=params;
			wchar_t* wparams=cm;
			argv_zz<wchar_t> args(wparams);

			{

				owner->logmsg("host: %s\n",phost);
				if(args.argc)

					for(int n=0;n<args.argc;++n)
						owner->logmsg_attr(M_BC|0x0e,L"args[%d]: %s\n",n,args.args(n));

			}

			wchar_t* file_path=args[hss_servises_t::fileid()];

			if(file_path)
			{  
				int cb;
				v_buf<wchar_t> vfpath(cb=safe_len(file_path,1));
				wchar_t* fpath=vfpath;
				wchar_t* pname=0;
				wchar_t* pwfile=0;
				http_magic_root_t http_magic_root;


				{
					//  http_magic_root_t http_magic_r(file_path,hss_servises_t::fileid());
				}

				//http_magic_root.init(file_path,hss_servises_t::fileid());

				//			  if(GetFullPathNameW(file_path,cb,fpath,&pname))
				//
				if(http_magic_root.init(file_path,hss_servises_t::fileid()))
				{
					//  if(pname) 					   *(pname-1)=0;
					// pwfile=pname;
					pname=http_magic_root.pfilename;
					fpath=http_magic_root.path;
					pwfile=http_magic_root.pfile;

					std::string fid;
					if(owner->set_root(fpath,&fid))
					{
						v_buf<char> urlbuf,reply;

						v_buf<char> vfilename;

						char* pfile="";

						//if(pname&&pname[0]&&(pname[0]!=L'.'))
						{ 
							//if(pname[0]!='.') {
							//char_mutator<CP_UTF8> cm(pname);
							char_mutator<CP_UTF8> cm(pwfile);
							char* p=cm.get_pc();
							//int cb=cm.buf.count()-1;
							int cb=cm.char_count();
							int cbo=3*cb+16;
							vfilename.resize(cbo);
							for(int m=0;m<cb;m++)
							{
								if(p[m]=='\\')  p[m]='/';
							}
							//pfile=url_escape(cm,cm.buf.count()-1,vfilename,&cb,0);
							pfile=url_escape2(p,cb,vfilename,&cbo);
							//}		 else pfile="/";
						}


						char* pbase=hss_reset_SOCKET_DATA(hovl,HSD_HTTP_URL_BASE);
						const char* proot=fid.c_str();

						urlbuf.printf("http://%s/%s%s/%s",phost,pbase,proot,pfile);
						char* purl=urlbuf;

						//reply.printf("HTTP/1.x 303 See Other\nLocation: %s\n\n<html><body><a href=\"%s\">Click here</a></body></html>",purl,purl);
						reply.printf("HTTP/1.x 303 See Other\r\nAccess-Control-Allow-Origin: *\r\nLocation: %s\r\n\r\n<html><body><a href=\"%s\">Click here</a></body></html>",purl,purl);
						char* pp=reply;
						hss_send(hovl,reply.get(),reply.count(),0);


					}

					return;


				}

			}

			//		  if((!file_path)&&(fn))		             
			//			 file_path=buf.cat(root.c_str()).cat(L"/").cat((wchar_t*)char_mutator<CP_UTF8>(fn));

			char* pvirtual_root_id=hss_reset_SOCKET_DATA(hovl,HSD_HTTP_VIRTUAL_ROOT);
			std::wstring virtual_root;
			if(owner->get_root(pvirtual_root_id,&virtual_root))
			{
				fn=hss_reset_SOCKET_DATA(hovl,HSD_HTTP_VIRTUAL_FILE);   
				file_path=buf.cat(virtual_root.c_str()).cat((wchar_t*)char_mutator<CP_UTF8>(fn));
			}
			else file_path=buf.cat(root.c_str()).cat(L"/").cat((wchar_t*)char_mutator<CP_UTF8>(fn));


			if(file_path)
			{

				argv_def<wchar_t,std::vector<wchar_t*>  > lmimes(owner->mimes.ppvect);
				wchar_t * wmimetype;
				wchar_t* ext=wcsrchr(file_path,L'.');
				wmimetype=(wchar_t*)lmimes[ext];
				char_mutator<CP_UTF8> cm(wmimetype);
				char* mimetype=cm;






				//std::wstring pfn=root;
				//pfn=pfn.append(L"/");
				//pfn=pfn.append((wchar_t*)char_mutator<CP_UTF8>(fn));

				//hss_servises_t::logmsg(L"sending file=%s to %s\n",buf.get(),(wchar_t*)char_mutator<CP_UTF8>(ra)); 

				v_buf<wchar_t> bb;

				bb.undecorate(file_path);
				wchar_t* fp=bb;
				//bool fdir=(fp)&&(GetFileAttributesW(fp)&FILE_ATTRIBUTE_DIRECTORY);
				INT64 sizeF=-1;
				if(is_dir(fp,sizeF))
				{
					char* pbase=hss_reset_SOCKET_DATA(hovl,HSD_HTTP_URL_BASE);
					char* http_file=hss_reset_SOCKET_DATA(hovl,HSD_HTTP_FILE);  

					http_dir_gen_t dir_gen(fp,v_buf<char>()<<pbase<<http_file);
					//http_dir_gen_t dir_gen(fp,http_file);
					hss_send(hovl,(char*)dir_gen,dir_gen.size(),0);
					//dir_gen.save_to_file(L"k://tmp.htm_hssh");
					owner->logmsg(L"sending folder=%s to %s\n",file_path,(wchar_t*)char_mutator<CP_UTF8>(ra)); 


					///reply.printf("HTTP/1.x 303 See Other\nLocation: %s\n\n<html><body><a href=\"%s\">Click here</a></body></html>",purl,purl);

				}
				else
					if(sizeF>=0)
					{

						owner->logmsg(L"sending file=%s to %s ..\n",file_path,(wchar_t*)char_mutator<CP_UTF8>(ra)); 
						//
						//						return ;
						owner->http_send_file(hovl,fp,mimetype);

						//					transmit_packets_t transmit_file(hovl->s);
						//					hovl->error_code=transmit_file.http_replay(hovl,fp);

						double smb=double(sizeF)/double(1024*1024);

						owner->logmsg(L"%s transfered of file=%s to %s (%s)\n",GMKB(sizeF),file_path,(wchar_t*)char_mutator<CP_UTF8>(ra),(wchar_t*)error_msg(hovl->error_code)); 

					} 
					else
						owner->logmsg_attr(M_BC|0x0c,L"file %s not FOUND!!\n",file_path); 

			}
		}
	};

	std::vector<void*> vh;
	std::vector<void*> vstub;
	std::vector<http_srv_t> vhttp;
	std::list<std::auto_ptr<ions_channel_t> > vions;

	v_buf<wchar_t> filelog;

	root_map_t virtual_roots;

	std::string idnull;

	mutex_cs_t mutex;
	inline	bool get_root(const char* id  ,std::wstring* prootout=0)
	{
		if(id&&id[0])
		{

			locker_t<mutex_cs_t> lock(mutex);
			root_map_t::iterator i=virtual_roots.find(id);
			if(i==virtual_roots.end()) return false;
			if(prootout)
			{
				*prootout=i->second;
			}
			return true;
		}
		else return false;

	}

	inline bool   is_dir(GUID& fid)
	{
		DWORD *ii=(DWORD *)&fid;
		return ii[3]&FILE_ATTRIBUTE_DIRECTORY;
	}
	inline	bool set_root(wchar_t* root,std::string* proot_id)
	{
		GUID fid=uuid_file(root,true);
		if(!is_dir(fid)) return false;
		locker_t<mutex_cs_t> lock(mutex);  
		std::string& root_id=* proot_id;
		//
		root_id= (char*) str_uuid(fid);
		//root_id= (char*) v_buf<char>().cat(str_uuid(fid)).cat("/zzz");
		virtual_roots[root_id]=root;
		return true;

	}




	//void (* logcallback)(wchar_t* msg);




	template <class CH>
	void logmsg(const CH * fmt,...)
	{
		//		return;
		va_list argptr;
		va_start(argptr, fmt);
		v_buf<CH> buf;
		shared_console.puts(buf.vprintf(fmt,argptr).get());
		//safe_cputs(buf.vprintf(fmt,argptr).get());
		if(!filelog.empty())
			append_to_file(filelog.get(),char_mutator<CP_THREAD_ACP>(buf.get()));		
	}

	template <class CH>
	void logmsg_attr(DWORD attr,const CH * fmt,...)
	{
		//		return;
		va_list argptr;
		va_start(argptr, fmt);
		v_buf<CH> buf;
		shared_console.puts(buf.vprintf(fmt,argptr).get(),attr);
		//safe_cputs(buf.vprintf(fmt,argptr).get());
		if(!filelog.empty())
			append_to_file(filelog.get(),char_mutator<CP_THREAD_ACP>(buf.get()));		
	}

	HRESULT send_file_syncro(HSS_INFO* hovl,wchar_t* fn,char* mimetype=0)
	{
		char buf[4096];
		v_buf<char> bufv;
		int cb=4026,cbr,c;
		//	 const char* reply_header[2]={"HTTP/1.0 200 OK\nAccess-Control-Allow-Origin: *\n\n","HTTP/1.0 404 Not Found\n"};
		const char* reply_header[2]={"HTTP/1.0 200 OK\r\nAccess-Control-Allow-Origin: *\r\n\r\n","HTTP/1.0 404 Not Found\r\n"};
		//const char* reply_header[2]={"HTTP/1.0 200 OK\n\n","HTTP/1.0 404 Not Found\n"};
		int f;
		handle_t hf=CreateFileW(fn,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);

		f=(hf!=INVALID_HANDLE_VALUE)?0:1;
		char* preply;

		if(1&&(!f)&&mimetype)
		{
			INT64 sz=0;
			GetFileSizeEx(hf,PLARGE_INTEGER(&sz));
			char tmp[32];
			//buf.printf("HTTP/1.0 200 OK\r\nContent-Type: %s\r\nContent-Length: %s\r\n\r\n",mimetype,_i64toa(sz,tmp,10));
			bufv.printf("HTTP/1.0 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: %s\r\n\r\n",mimetype);
			preply=bufv.get();
		}
		else preply=(char*)reply_header[f];

		{
			c=strlen(preply);
			if(c!=send(hovl->s,preply,c,0))
				return (c<0)?WSAGetLastError():ERROR_GRACEFUL_DISCONNECT;
		}


		if(f==0)
			while(f=ReadFile(hf,buf,cb,(DWORD*)&cbr,0))
			{
				if(cbr==0) return S_OK;
				char* p=buf;
				while(cbr>0)
				{
					c=send(hovl->s,p,cbr,0);
					if(c<0) return WSAGetLastError();
					else if(c==0) return ERROR_GRACEFUL_DISCONNECT;
					p+=c;
					cbr-=c;
				}
			}
			return GetLastError();
	}

	HRESULT http_send_file(HSS_INFO* hovl,wchar_t* fn,char* mimetype=0)
	{
		return hovl->error_code=send_file_syncro(hovl,fn,mimetype);

		if(1)
		{
			transmit_file_t transmit_file(hovl->s);
			return hovl->error_code=transmit_file.http_replay(hovl->s,fn);

		}
		else 
		{
			hss_http_sendfileW(hovl,fn);
			return hovl->error_code;
		}

	}


	inline static  bool check_options(HSS_INFO* hovl)
	{
		WSABUF& r=hovl->buffers[HSD_REQUEST];
		ULONG st;
		bool f=(hovl&&(r.buf)&&(r.len>5)&&(StrCmpNIA(r.buf,"OPTIONS",7)==0));
		if(f)
		{
			//const char reply[]="HTTP/1.1 200 OK\r\nAllow: OPTIONS,POST,GET\r\n\r\nPublic: OPTIONS,POST,GET\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 0\r\n\r\n";
			//const char reply[]="HTTP/1.1 200 OK\r\nAllow: OPTIONS POST GET\r\n\r\nPublic: OPTIONS POST GET\r\n\r\n";
			
			//				return;
			st=::hss_HTTP_request_headers_chain(hovl); 
			const char reply[]=\
				"HTTP/1.1 200 OK\r\n"\
				"Content-Type: text/plain\r\n"\
				"Access-Control-Allow-Origin:*\r\n"\
				"Access-Control-Allow-Methods: HEAD, OPTIONS, GET, POST\r\n"\
				"Access-Control-Allow-Headers: Upload-Path, Upload-Unzip\r\n"\
				"\r\n";

			hss_send(hovl,(void*)reply,sizeof(reply),0);
		}
		return f;
	}

	inline static  bool check_get(HSS_INFO* hovl)
	{
		WSABUF& r=hovl->buffers[HSD_REQUEST];
		return (hovl&&(r.buf)&&(r.len>5)&&(StrCmpNIA(r.buf,"GET",3)==0));
	}

	inline static  bool check_post(HSS_INFO* hovl)
	{
		WSABUF& r=hovl->buffers[HSD_REQUEST];
		return (hovl&&(r.buf)&&(r.len>6)&&(StrCmpNIA(r.buf,"POST",4)==0));
	}

	inline static  bool check_delay(HSS_INFO* hovl,int& it)
	{
		WSABUF& r=hovl->buffers[HSD_URL];

		const char cc[]="\n\n";
		if((hovl&&(r.buf)&&(r.len>=4))&&(safe_cmpni(r.buf,"::::",4)==0))
		{
			double t=safe_atof(r.buf+4);
			t= (t<0)?0:t;
			t= (t>5000)?5000:t;
			;
			Sleep(it=t);
			send(hovl->s,cc,sizeof(cc),0);
			closesocket(make_detach(hovl->s,INVALID_SOCKET));

			return true;

		}
		return false;
		//&&(StrCmpNIA(r.buf,"POST",4)==0));
		//safe_atof()

	}


	void  static s_on_tcp_accept(HSS_INFO* hovl)
	{
		//hovl->user_data 
		if(hovl&&hovl->user_data) this_t(hovl->user_data)->on_tcp_accept(hovl);
	}



	void  __stdcall on_tcp_accept(HSS_INFO* hovl)
	{

		HRESULT hr=0;
		ULONG st=::hss_ns_ipc_chain(hovl);

		//int fd=_open_osfhandle(intptr_t(hovl->s),0);
		//long sdu=_get_osfhandle(fd); 

		//return;
		hr=hss_error();
		if(!check_get(hovl))
		{
			if(check_options(hovl)){return ;}
			else
				if(check_post(hovl))
				{
					int it=-1;
					if(check_delay(hovl,it))
			  {

				  logmsg_attr(M_BC|0x0c,L":sleep:%d ",it);

				  return ;
			  }
					logmsg_attr(M_BC|0x0c,L"\n++warn++ HSD_REQUEST POST \n");
				}
				else
				{
					logmsg_attr(M_BC|0x0c,L"\n++ERROR++ HSD_REQUEST errcode=%x {%s} \n",hr,error_msg(hr).get());
					return ;
				}

		}


		char* rh=hss_reset_SOCKET_DATA(hovl,HSD_REMOTE_INFO);
		rh=(rh)?rh:"???";
		char* rl=hss_reset_SOCKET_DATA(hovl,HSD_LOCAL_INFO);
		rl=(rl)?rl:"???";
		char* purl=hss_reset_SOCKET_DATA(hovl,HSD_URL);  
		const char favico[]="favicon.ico";	
		if(safe_cmpni(purl,favico,sizeof(favico))==0)
		{
			::hss_HTTP_request_headers_chain(hovl); 
			v_buf<wchar_t> fn;
			fn.cat((wchar_t*)argv_ini<wchar_t>().get_module_path()).cat(char_mutator<CP_THREAD_ACP>(favico));
			wchar_t* fp=fn;
			http_send_file(hovl,fp,"image/x-icon");

		}
		else if(SUCCEEDED(hr))	
			hr=::hss_send_SOCKET_DATA(hovl);

		__time64_t ltime;
		_time64( &ltime );
		wchar_t *stim=_wctime64( &ltime ),*pp;

		if((stim)&&(*stim))
		{
			pp=stim+wcslen(stim)-1;
			if(*pp==L'\n') *pp=0;

		}
		int ch=hss_heap_count();    
		DWORD attr=(hr)? (M_BC|0x0c):(M_BC|0x0b);
		logmsg_attr(attr,L"<%d>[%s] URL:=%s\n(%S)->(%S) ns_ipc_chain: errcode=%x {%s} \n",ch,stim,(wchar_t*)char_mutator<CP_UTF8>(purl),rh,rl,hr,error_msg(hr).get());

	}

	hss_servises_t(){};
	template <class Args>
	hss_servises_t(const Args& args)
	{
		init(args);
	};

	template <class Args>
	inline void set_mimes( Args& args)
	{
		argv_zzs<wchar_t> tmp;
		tmp=args;
		//tmp.set_prefix_name("mime.");
		v_buf<wchar_t> buf  ;
		for(int n=0;n<tmp.argc;n++)
		{
			wchar_t *name=tmp.names(n,buf);
			wchar_t *val=tmp[name];
			wchar_t *pn=0;
			if(name)
			{
				pn=wcsstr(name,L"mime.");
				if(pn) pn+=4;
			}

			if(pn&&val)
			{
				mimes[pn]=val;
			}
		}


		//pflatmimes=mimes.flat_str();

	}

	inline std::wstring  remq(const wchar_t* p)
	{
		if(!p) return L"";
		v_buf<wchar_t> buf1,buf2;
		buf1.undecorate(p,L'\"',L'\"');
		return buf2.undecorate(buf1.get(),L'\'',L'\'').get();
	};

	template <class Args>
	inline hss_servises_t& init( Args& args)
	{
		clear();
		std::vector<std::wstring> vipp,vhttpns,vhttpdir,vionsns,vstorage_root;
		GUID gg={};
		idnull=str_uuid(gg);  

		vipp=args[L"ipp"];
		vhttpns=args[L"http"];
		vhttpdir=args[L"http.root"];
		vstorage_root=args[L"http.storage.root"];
		vionsns=args[L"ions"];

		filelog.cat(args[L"log"]);

		set_mimes(args);



		void *pcltcp,*pclhttp,*p;
		/*
		pcltcp=hss_create_stdcall_closure(to_ptr(&hss_servises_t::on_tcp_accept),this);
		vstub.push_back(pcltcp);
		*/
		//	pclhttp=hss_create_stdcall_closure(to_ptr(&hss_servises_t::on_http_accept),this);
		//	vstub.push_back(pclhttp);
		int cbipp=vipp.size();

		for(int n=0;n<cbipp;n++)
		{
			wchar_t* sp=(wchar_t*) vipp[n].c_str();
			//p=hss_socket_server_pool_chain(char_mutator<CP_UTF8>(sp),pcltcp,0);
			p=hss_socket_server_pool_chain(char_mutator<CP_UTF8>(sp),&hss_servises_t::s_on_tcp_accept,this);
			vh.push_back(p);
		}
		int cbhttp=vhttpns.size(),cbr=vhttpdir.size(),cb_storage=vstorage_root.size();
		for(int n=0;n<cbhttp;n++)
		{
			wchar_t* fn=(wchar_t*) vhttpns[n].c_str();
                 
             int nr= n%cbr;
			std::wstring sroot=remq(expand_env(vhttpdir[nr].c_str()).get());


			if (n<cbr) vhttp.push_back(http_srv_t(this,sroot));
			else vhttp.push_back(http_srv_t(this));
			http_srv_t* pctx=&vhttp.back();
			if(cb_storage==cbr){
                 std::wstring s=remq(expand_env(vstorage_root[nr].c_str()).get());
				 pctx->fstorage=s.length();
				 pctx->storage_root=s;
			}


			//	pclhttp=hss_create_stdcall_closure(to_ptr(&http_srv_t::on_http_accept),pctx);
			//	vstub.push_back(pclhttp);
			// p=hss_create_socket_portW(fn,HSF_SRD_AUTO|HSF_SRD_CREATE_PATH,pclhttp,0);
			p=hss_create_socket_portW(fn,HSF_SRD_AUTO|HSF_SRD_CREATE_PATH,&http_srv_t::s_on_http_accept,pctx);
			vh.push_back(p);
		}


		int cbions=vionsns.size();
		//	  vions.resize(cbions);
		for(int n=0;n<cbions;n++)
		{
			wchar_t* ion=(wchar_t*) vionsns[n].c_str();
			void* p= hss_ion_channels(ion);
			vh.push_back(p);
			/*
			//std::auto_ptr_ref<ions_channel_t> 
			ions_channel_t* ic=new ions_channel_t;
			ic->open(ion);
			//std::auto_ptr_ref<ions_channel_t> icr(ic);
			vh.push_back(ic->hport);
			//std::auto_ptr<ions_channel_t> icc(ic);
			//vions.push_back(icc);
			*/
		}

		return *this;
	}

	inline static void clearv(std::vector<void*>& vh)
	{
		int cb=vh.size();
		for(int n=0;n<cb;++n)
			hss_release(vh[n]);
		if(cb) vh.clear();

	}

	inline void clear()
	{
		filelog.clear();
		clearv(vh);	
		clearv(vstub);	
	}

	~hss_servises_t()
	{
		clear();
	}

	void wait_all()
	{
		int cb;
		if(cb=vh.size())
			hss_wait_group_signaled_0(-1,true,cb,&vh[0]);

	}
	transmit_file_t transmit_file;
	console_shared_data_t<wchar_t,0> shared_console;
	const char* pflatmimes;

};