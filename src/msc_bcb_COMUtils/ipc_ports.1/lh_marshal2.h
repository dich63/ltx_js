#pragma once
#include "stack_alloc.hpp"
#include "ipc_marshal.h"
#include "ipc_utils.h"
//#include "Ntsecpkg.h"

#define REGION_STACK_COUNT 32
#include <stdlib.h>

#include <vector>
#include <list>
#include <map>

#include <string>
#include <strstream>
#include "static_constructors_0.h"
#include "wchar_parsers.h"
#include "free_com_factory.h"

namespace kox_marshal2
{

	struct ipc_marshal_region_t
	{


	};

struct lh_marshal_t
{

	typedef  ipc_marshal_data_t::region_t region_t;
	typedef std::vector<region_t> vregions_t;
	typedef  ipc_marshal_data_t::uuid_t uuid_t;
	typedef ipc_utils::COMInit_t COMInit_t;
	typedef   ipc_utils::CHGlobal_t<char> CHGlobal_t;
	typedef ipc_utils::process_handle_t process_handle_t;
	typedef ipc_utils::weak_destructor_t weak_destructor_t;

	enum
	{
		MSHLFLAGS_XX= MSHLFLAGS_TABLESTRONG,
		MSHCTX_XX=MSHCTX_DIFFERENTMACHINE,
		max_ss=sizeof(ipc_marshal_data_t)+sizeof(region_t)*(REGION_STACK_COUNT-1)
	};


	typedef    WSAPROTOCOL_INFOW socket_data_t;
	typedef    std::list<region_t*> region_list_t;

	typedef    std::vector<ipc_region_base_t*> region_vector_t;
	//	 typedef v_buf<char,utils::alloc::stack_alloc<char,max_ss> > stack_buf_t;
	//
	typedef v_buf<char > stack_buf_t;

	//vregions_t vregions;



	struct inner_data_factory_t:ipc_marshal_data_factory_t
	{

		on_delete_t dp;

		inner_data_factory_t():dp(0){};
		ipc_marshal_data_t* __cdecl allocate( int sizeb,on_delete_t _dp)
		{

			marshal_buf.resize(sizeb);
			dp=_dp;
			return (ipc_marshal_data_t*)marshal_buf.get();
		};
		void	__cdecl release_marshal_data(ipc_marshal_data_t* pmd)
		{
			if(dp) dp(pmd);
			marshal_buf.clear();

		};

		v_buf<char> marshal_buf;
	};


	lh_marshal_t(ipc_marshal_data_factory_t* _factory=0):pmd(0),state(0)
	{
		pmd_factory=(_factory)?_factory:&inner_factory;
	};

	~lh_marshal_t(){clear();}



	class_initializer_T<ipc_utils::WSA_initerfiniter> socket_lib_init_once;





	inline	static const  uuid_t& s_port_uuid()
	{
		// {9A516FBC-C85B-4d72-8EA7-D7DAD0FF8444}
		static const uuid_t s =  { 0x9a516fbc, 0xc85b, 0x4d72, { 0x8e, 0xa7, 0xd7, 0xda, 0xd0, 0xff, 0x84, 0x44 } };
		return s;

	}


	template <template<class> class  Allocator=std::allocator >
	struct marshal_data_impl_t:ipc_marshal_data_t
	{

		typedef Allocator<char> allocator_t;
		//
		// vregions_t vregions;

	};

	static	inline void region_type_auto_detect(region_t& r)
	{
		if(r.ptr)
		{
			if(r.attr&(IPCM_KO))
			{
				int ty=ipc_utils::GetHandleType(*r.phandle);
				if(ty) r.attr|=ty;
				else r.attr|=IPCM_ERROR;
			}
		}
		else   r.attr|=IPCM_ERROR;



	}

	static	inline int region_marshal_size(region_t& r,std::list<HGLOBAL> &hstubs,HRESULT& hr)
	{


		if(r.attr&IPCM_ERROR) return 0;

		if((r.attr&(~IPCM_REGIONMAP))==0)  return r.sizeb;

		if(r.attr&IPCM_SOCKET)
			return sizeof(socket_data_t);

		if((r.attr&IPCM_HGLOBAL)&&((r.attr&IPCM_COM)==0))
			return GlobalSize(*r.pcomstub);


		if((r.attr&IPCM_COM))
		{

			if(r.attr&IPCM_HGLOBAL)	 return CHGlobal_t(*r.pcomstub).sizeb();
			//hr=ipc_utils::CoDuplicateCOMStub(*r.pcomstub,&gl.m_h,false,MSHCTX_LOCAL,MSHLFLAGS_XX);			
			else 
			{
				CHGlobal_t gl(HGLOBAL(0));

				ipc_utils::COMInit_t ci(COINIT_MULTITHREADED);  
				if(S_OK==ci)	free_com_factory::GlobalApartmentThreadId<COINIT_MULTITHREADED>();	
				hr=ipc_utils::CoMarshalInterfaceToHGlobal(*r.ppunknown,&gl.m_h,MSHCTX_XX,MSHLFLAGS_XX);

				if(FAILED(hr)) { SetLastError(hr);return -1;}
				int siz=gl.sizeb();
				hstubs.push_back(gl.detach());
				r.pcomstub=&hstubs.back();
				return siz;
			}
		}


		/*
		if(r.attr&IPCM_COM)
		{
		HRESULT hr;
		ULONG ll=0;
		if(FAILED(CoGetMarshalSizeMax(&ll,__uuidof(IUnknown),*r.ppunknown,MSHCTX_LOCAL,0,MSHLFLAGS_NORMAL)))
		{
		SetLastError(hr);
		return -1;
		};
		return ll;
		}
		*/



		return sizeof(long long);
	}

	struct Impersonate_t
	{
		HRESULT hr;
		Impersonate_t(HANDLE hp=GetCurrentThread())
		{

			//BOOL fpp=ImpersonateAnonymousToken(hp);
			BOOL fpp=ImpersonateLoggedOnUser(hp);

			hr=(fpp)?S_OK:GetLastError();

		}
		~Impersonate_t(){
			if(!hr) 
				RevertToSelf();
		}

	};



	static	inline HRESULT region_marshal(region_t& region,process_handle_t& process,ISequentialStream* stream)
	{
		HRESULT hr;
		bool f;
		DWORD dw;
		region_t &r=region;

		if(r.attr&IPCM_ERROR) 
			return S_FALSE;

		if((r.attr&(~IPCM_REGIONMAP))==0) 
		{
			if(r.sizeb)	return hr=stream->Write(r.ptr,r.sizeb,&dw);
			else return S_FALSE;
		}



		if(r.attr&(IPCM_HGLOBAL|IPCM_COM))
		{

			CHGlobal_t gl(*r.pcomstub);
			/*
			if(r.attr&IPCM_COM)
			{
			CHGlobal_t gl2(HGLOBAL(0));
			hr=ipc_utils::CoDuplicateCOMStub(gl.handle(),&gl2.m_h,false,MSHCTX_LOCAL,MSHLFLAGS_NORMAL);
			int siz=gl2.size();
			if(SUCCEEDED(hr))      hr=stream->Write((char*)gl2,siz,&dw);
			}

			else */
			hr=stream->Write((char*)gl,gl.size(),&dw);

			return hr;
		}

		if(r.attr&IPCM_SOCKET)
		{

			socket_data_t wsapi={};
			SOCKET s=*r.psocket;

			hr=ipc_utils::WSASocketMarshal(s,&wsapi);

			return hr=stream->Write(&wsapi,sizeof(wsapi),&dw);

		}

		/*
		if(r.attr&IPCM_COM)
		{

		CHGlobal_t gl;
		hr=ipc_utils::CoMarshalInterfaceToHGlobal(*r.ppunknown,&gl.m_h,MSHCTX_LOCAL,MSHLFLAGS_NORMAL);
		int siz=gl.size();
		char* p=gl;
		hr=stream->Write(p,siz,&dw);
		if(SUCCEEDED(hr)&&(dw<r.sizeb))
		{ 
		v_buf<char> tmp(r.sizeb);
		hr=stream->Write(tmp.get(),r.sizeb-dw,&dw);	

		}
		return hr;
		}
		*/
		if(r.attr&IPCM_HANDLE)
		{

			HANDLE hsrc=(r.phandle)?*r.phandle:INVALID_HANDLE_VALUE;
			HANDLE hdest[2]={hsrc,(INVALID_HANDLE_VALUE!=hsrc)?0:INVALID_HANDLE_VALUE};
			return hr=stream->Write(hdest,sizeof(long long),&dw);	   



			/*
			HANDLE hsrc=*r.phandle;

			HANDLE hdest[2]={INVALID_HANDLE_VALUE,0};
			HANDLE hsp=GetCurrentProcess();
			f=DuplicateHandle(hsp,hsrc,process,hdest,0,0,DUPLICATE_SAME_ACCESS);
			if(!f) hdest[1]=INVALID_HANDLE_VALUE;
			hr=WSAGetLastError();
			return hr=stream->Write(hdest,sizeof(long long),&dw);	   
			*/
		}

	}


	inline bool region_detach(int n,region_t* pr)
	{
		if((n>=0)&&(pmd)&&(pr))
		{
			bool f=(n<pmd->header.region_count);
			f&=(pmd->regions[n].attr&(~IPCM_ERROR));
			if(f) *pr=ipc_utils::make_detach(pmd->regions[n]);

			return f;

		}
		return false;
	}




	static	inline void region_release(region_t& r)
	{
		///*
		void* p=r._ipc_ptr;
		if(r.destroy_proc)
			r.destroy_proc(p,&r); 
		//r.destroy_proc(p,p); 
		return;
		//*/


		DWORD dw;
		if((r.attr==0)||(r.attr&IPCM_ERROR)||(r.ptr==0)) return;


		if(r.attr&IPCM_SOCKET)
		{
			if(*r.psocket!=INVALID_SOCKET)
				closesocket(*r.psocket);
		}


		else if((r.attr&IPCM_HGLOBAL)&&(GlobalFlags(*r.pcomstub)!=GMEM_INVALID_HANDLE))
		{

			if((r.attr&IPCM_COM))
			{
				//com.init();
				//ipc_utils::smart_ptr_t<IStream> stream;
				//if(SUCCEEDED(CreateStreamOnHGlobal(*r.pcomstub,0,stream.address())))
				//	CoReleaseMarshalData( stream);
				ipc_utils::CoReleaseCOMStub(*r.pcomstub);

			}
			GlobalFree(*r.pcomstub);
		}

		else if(r.attr&IPCM_HANDLE)
		{
			if(GetHandleInformation(*r.phandle,&dw))
				CloseHandle(*r.phandle);
		}




	}



	static	inline void region_unmarshal(region_t& region,char*& buf,HANDLE hprocess_sender)
	{
		HRESULT hr;
		BOOL f;
		region_t &r=region;

		if(r.attr&IPCM_ERROR) return; 

		r.ptr=0;
		r.__rsrv=0;
		r.__proc=0;
		long offset=r.sizeb;
		if((r.attr&(~IPCM_REGIONMAP))==0)
			r.ptr=buf;
		else if(r.attr&(IPCM_HGLOBAL|IPCM_COM))
		{
			CHGlobal_t gl(r.sizeb,buf);

			r.attr|=IPCM_HGLOBAL;

			if(gl)
			{ 
				r.ptr=r._ipc_buf;

				r.destroy_proc=&weak_destructor_t::destroy_hglobal;

				if(r.attr&IPCM_COM) 
				{
					//r.ptr=buf;

					//if(FAILED(hr=ipc_utils::CoDuplicateCOMStub(gl.handle(),r.pcomstub,true,MSHCTX_LOCAL,MSHLFLAGS_TABLESTRONG)))
					if(SUCCEEDED(hr=ipc_utils::CoDuplicateCOMStub(gl.handle(),r.pcomstub,false,MSHCTX_XX,MSHLFLAGS_TABLESTRONG)))
						r.destroy_proc=&weak_destructor_t::destroy_comstub;

					else r.attr|=IPCM_ERROR;

				}
				else  *r.pcomstub=gl.detach();
				r.sizeb=sizeof(HGLOBAL);
			}
			else r.attr|=IPCM_ERROR;
		}
		else if(r.attr&IPCM_SOCKET)
		{
			socket_data_t* pwsapi=(socket_data_t*)buf;
			//r.ptr=buf;
			r.ptr=r._ipc_buf;
			//SOCKET s;
			hr=ipc_utils::WSASocketUnmarshal(hprocess_sender,pwsapi,GetCurrentProcess(),r.psocket);
			if(SUCCEEDED(hr))
			{
				r.sizeb=sizeof(SOCKET);
				r.destroy_proc=&weak_destructor_t::destroy_socket;

			}
			else {
				r.attr|=IPCM_ERROR;
				r.hr=hr;
			}


		}
		else if(r.attr&IPCM_HANDLE)
		{   
			//r.ptr=buf;
			//HANDLE hsrc=*r.phandle;
			r.ptr=r._ipc_buf;
			HANDLE hsrc=*((HANDLE*)buf);
			*r.phandle=INVALID_HANDLE_VALUE;
			f=DuplicateHandle(hprocess_sender,hsrc,GetCurrentProcess(),r.phandle,0,0,DUPLICATE_SAME_ACCESS);
			if(f) 
			{
				r.sizeb=sizeof(HANDLE);
				r.destroy_proc=&weak_destructor_t::destroy_handle;
			}
			else {
				r.attr|=IPCM_ERROR;
				r.hr=GetLastError();}
			//if(*r.phandle!=INVALID_HANDLE_VALUE) r.sizeb=sizeof(HANDLE);
			//else r.attr|=IPCM_ERROR;
		}

		buf+=offset;
	}


	void clear()
	{
		//		release_ipc_marshal_data(ipc_utils::make_detach(pmd));
		//marshal_buf.clear();

		pmd_factory->release_marshal_data(ipc_utils::make_detach(pmd));
		//region_list.clear();
		region_vector.clear();
	}

	static void release_ipc_marshal_data(ipc_marshal_data_t* pmd)
	{
		if(pmd)
		{

			for(int n=0;n<pmd->header.region_count;n++)
				region_release(pmd->regions[n]);
		}
	}


	inline static HRESULT read_message(ISequentialStream* stream,void* buf,DWORD cb)
	{
		HRESULT hr;
		DWORD cbr;
		char *p=(char *) buf;
		while(SUCCEEDED(hr=stream->Read(p,cb,&cbr)))
		{
			if(cbr>=cb) break;
			p+=cbr;
			cb-=cbr;
		}
		return hr;
	};




	//HRESULT unmarshal(ISequentialStream* stream,HANDLE fd=INVALID_HANDLE_VALUE,const uuid_t& conn_uuid=s_port_uuid())
	HRESULT unmarshal(ISequentialStream* stream)
	{
		HRESULT hr;
		DWORD cb,cbr;
		bool fpp;
		ipc_marshal_data_t::header_t header;

		//Impersonate_t imp;


		if(FAILED(hr=read_message(stream,&header,sizeof(header)))) 
			return hr;



		char* p=(char*)&header; 
		cbr=sizeof(header);


		process_handle_t     process_sender(header.sender_pid);
		process_handle_t     process_server(GetCurrentProcessId());


		HANDLE hsender=process_sender;


		if(!header.server_pid)
		{


			header.server_pid=GetCurrentProcessId();	
			if(!hsender) header.server_handle=process_server;


			if(FAILED(hr=stream->Write(&header,sizeof(header),&cb))) 
				return hr;
			if(cb!=sizeof(header)) return E_FAIL;
		}

		clear();

		pmd=pmd_factory->allocate(header.sizeb,&release_ipc_marshal_data);
		pmd->header=header;


		//char* buf=(char*)pmd->regions;
		char* buf=(char*)&pmd->sender_trusted;
		int siz=header.sizeb-sizeof(header);


		if(FAILED(hr=read_message(stream,buf,siz))) 
		{
			for(int n=0;n<header.region_count;n++)    pmd->regions[n].attr=IPCM_ERROR;
			return hr;
		}

		process_handle_t process_sender2(pmd->sender_trusted.handle);


		if(hsender==0) hsender=pmd->sender_trusted.handle;





		//		if(fd!=INVALID_HANDLE_VALUE)		RevertToSelf();

		buf+=sizeof(region_t)*header.region_count+sizeof(ipc_marshal_data_t::sender_trusted_t);

		region_vector.resize(header.region_count);
		for(int n=0;n<header.region_count;n++)
		{
			region_t& r=pmd->regions[n];
			region_vector[n]=&r;
			region_unmarshal(r,buf,hsender);

			//region_list.push_back(&r);
		}


		ipc_marshal_data_t::header_min_t hret={sizeof(ipc_marshal_data_t::header_min_t),0};   
		hr=stream->Write(&hret,sizeof(ipc_marshal_data_t::header_min_t),&cb);    

		return hr;


		//buf(sizeof(marshal_data_impl_t)+sizeof(region_t)*region_count);

	}





	/*

	static	HRESULT marshal(int rcount,region_t* pregions,ISequentialStream* stream,int pid=0,const uuid_t& conn_uuid=s_port_uuid())
	{
	return marshal(pregions,pregions+rcount,stream,pid,conn_uuid);
	}

	*/







	inline static	HRESULT marshal(int region_count,ipc_region_base_t* pregions,ISequentialStream* stream,int pid=0,const uuid_t& conn_uuid=s_port_uuid())
	{
		struct pp_iterator_t
		{
			ipc_region_base_t* pregions;

			inline  ipc_region_base_t* operator *()
			{
				return pregions;
			}

			inline  pp_iterator_t& operator ++()
			{
				++pregions;
				return *this;
			}



		} pp_iterator={pregions};

		HRESULT hr;

		return hr=marshal( region_count,pp_iterator,stream,pid,conn_uuid);

	}



	template <class  region_iterator>
	static	HRESULT marshal(int region_count,region_iterator iregion,ISequentialStream* stream,int pid=0,const uuid_t& conn_uuid=s_port_uuid())
	{
		//vregions.resize(region_count);
		//std::copy(pregions,pregions+region_count,vregions.begin());


		struct COMCleaner_t
		{
			std::list<HGLOBAL> hstubs;
			~COMCleaner_t(){
				clear();
			}
			void clear()
			{

				if(hstubs.size()==0) return;

				ipc_utils::COMInit_t ci(COINIT_MULTITHREADED);

				for(std::list<HGLOBAL>::iterator i=hstubs.begin();i!=hstubs.end();++i)
					ipc_utils::CoReleaseCOMStub(*i,1);
			}			  		  


		} COMCleaner;

		BOOL f;


		if(stream==0) return E_POINTER;
		HRESULT hr;
		DWORD dw;
		//int region_count=std::distance(ibegin,iend); 

		//        int  ddsizh=sizeof(ipc_marshal_data_t),ddf=sizeof(region_t);

		int sizh=sizeof(ipc_marshal_data_t)+sizeof(region_t)*(region_count-1);
		//const int max_ss=sizeof(ipc_marshal_data_t)+sizeof(region_t)*(REGION_STACK_COUNT-1);

		//v_buf<char,utils::alloc::stack_alloc<char,max_ss> > buf(sizh);
		stack_buf_t buf(sizh);
		ipc_marshal_data_t* pmd=(ipc_marshal_data_t* )buf.get();


		//        for(int n=0;n<region_count;++n,++iregion)
		//			pmd->regions[n]=*iregion;





		pmd->header.port_uuid=s_port_uuid();
		pmd->header.region_count=region_count;
		//region_iterator iregions=ibegin;

		int siz=sizh,sr;
		//		for(int n=0;n<region_count;++n)
		for(int n=0;n<region_count;++n,++iregion)
		{
			;
			region_t& r=pmd->regions[n];
			//r=*iregion;


			r.attr=(*iregion)->attr;
			r.sizeb=(*iregion)->sizeb;
			r.ptr_v=(*iregion)->ptr_v;




			//r=*iregion;
			//r=regions[n];
			//&(IPCM_HANDLE|IPCM_SOCKET);
			region_type_auto_detect(r);

			pmd->header.regions_attr|=r.attr;

			if((sr=region_marshal_size(r,COMCleaner.hstubs,hr))<0)
				return hr;
			r.sizeb=sr;
			siz+=sr;
		}
		pmd->header.sizeb=siz;
		pmd->header.conn_uuid=conn_uuid;

		pmd->header.sender_pid=GetCurrentProcessId();
		pmd->header.server_pid=pid;


		hr=stream->Write(&pmd->header,sizeof(pmd->header),&dw);
		if(FAILED(hr)) 
			return hr;



		//if((!pid)&&(pmd->header.regions_attr&(IPCM_HANDLE|IPCM_SOCKET)))	  { }
		if(!pid)	  {

			ipc_marshal_data_t::header_t header;

			DWORD server_pid;
			int sz;
			hr=stream->Read(&header,sz=sizeof(header),&dw);
			if(FAILED(hr)) 
				return hr;
			pmd->header.server_pid= header.server_pid;
			pmd->header.server_handle= header.server_handle;
			if(header.server_handle)
			{
				HANDLE hc=GetCurrentProcess();
				process_handle_t hserver(header.server_pid);
				f=DuplicateHandle(hc,hc,hserver,&(pmd->sender_trusted.handle),0,0,DUPLICATE_SAME_ACCESS);
				hr=GetLastError();
			}
			//pmd->header.server_thread_handle=header.server_thread_handle;trusted
		}

		//hr=stream->Write(pmd->regions,sizeof(region_t)*region_count,&dw);
		int sizrh=sizeof(region_t)*region_count+sizeof(ipc_marshal_data_t::sender_trusted_t);
		hr=stream->Write(&pmd->sender_trusted,sizrh,&dw);

		if(FAILED(hr)) 
			return hr;

		process_handle_t ph(pmd->header.server_pid);
		ph.hp=make_detach(pmd->header.server_handle);

		for(int n=0;n<region_count;n++)
		{
			hr=region_marshal(pmd->regions[n],ph,stream);
			if(FAILED(hr))  break;
		}

		//if(pmd->header.server_thread_handle)			CloseHandle(make_detach(pmd->header.server_thread_handle));

		ipc_marshal_data_t::header_min_t hret={};   
		if(SUCCEEDED(hr=read_message(stream,&hret,sizeof(hret))))
			hr=hret.hr;
		return hr;
	}




	//v_buf<char> marshal_buf;

	inline ipc_region_base_t**	get_regions_ptr()
	{
		return region_vector.size()? &region_vector[0]:0;
	}

	inline 	int get_region_count()
	{
		return region_vector.size(); 
	}


	ipc_marshal_data_t* pmd;
	//region_list_t region_list;
	region_vector_t region_vector;
	int state;

	ipc_marshal_data_factory_t* pmd_factory;
	inner_data_factory_t inner_factory;



};

struct lh_marshal_map_t
{
	typedef ipc_region_base_t region_t;

	typedef    std::map<std::string,region_t*> region_map_t;
	typedef    std::list<ipc_region_base_t> region_list_t;

	typedef   region_map_t::iterator iterator_t;

	inline static const uuid_t& uuid() {
		static const uuid_t s =  { 0xd34e169e, 0xdc80, 0x46b4, { 0x90, 0xd9, 0x85, 0x6, 0x3b, 0xdb, 0x23, 0x1 } };
		return s;
	}

	lh_marshal_map_t(ipc_marshal_data_factory_t* _factory =0):lh_marshal(_factory),tid(GetCurrentThreadId()){};


	inline bool check_tid(){ return GetCurrentThreadId()==tid;}
	
	inline HRESULT marshal(ISequentialStream* stream,int pid=0)
	{
		HRESULT hr;
		int region_count=region_map.size(),k=1;
		if(!region_count) return S_FALSE;

		std::vector<region_t*> vr(region_count+1);
		std::strstream strbuf;
		const char cz='\0';

		for (iterator_t i=region_map.begin();i!=region_map.end();++i,k++)
		{
			std::string name=i->first;
			region_t* regs=i->second;
			vr[k]=regs;
			strbuf<<name<<"="<<k<<cz;		 
		}
		strbuf<<cz<<cz<<cz;		
		region_t rm;
		rm.attr=IPCM_REGIONMAP;
		rm.ptr=strbuf.str();
		rm.sizeb=strbuf.pcount();
		vr[0]=&rm;
		region_t** pp=&vr[0];
		return hr=lh_marshal.marshal(region_count+1,pp,stream,pid,uuid());
	}

	int find_map_region(region_t**	ppr,int c)
	{

		for(int k=0;k<c;k++)
			if(ppr[k]->attr&IPCM_REGIONMAP) return k;

		return -1;

	}
	inline HRESULT unmarshal(ISequentialStream* stream)
	{
		HRESULT hr;
		v_buf<char> buf(20);
		int mapindex;
		if(FAILED(hr=lh_marshal.unmarshal(stream))) return hr;

		int count=lh_marshal.get_region_count();
		region_t**	ppr=(region_t**)lh_marshal.get_regions_ptr();
		if((mapindex=find_map_region(ppr,count))<0){

			for(int k=0;k<count;k++){
			char *name=itoa(k+1,buf,10);
			 region_map[name]=ppr[k];
			}
		}
		else{

			region_t *rm=ppr[mapindex];
			 
			 argv_zz<char> args(rm->ptr);
			 int rc=args.argc;
			 for(int k=0;k<rc;k++){
				 char* name=args.names(k,buf);
				 int nr=args[name].def(-1);
				 if((0<=nr)&&(nr<count))
					 region_map[name]=ppr[nr];

			 }
			 


		}
		return hr;

	}

	inline int set_region(const char* name,const ipc_region_base_t* pregion)
	{
	     if(pregion)
		{
			region_cache.push_back(*pregion);			
			region_map[name]=(region_t*)&region_cache.back();
		}
		else region_map.erase(name);

		return region_map.size();

	} 

	inline int get_names(  char*** pppnames)
	{
		if(!pppnames) return -1;
		int cb=region_map.size();
		names.resize(cb);
		char **ppnames=names.get();
		int k=0;
		for (iterator_t i=region_map.begin();i!=region_map.end();++i,k++){
			ppnames[k]=(char*)i->first.c_str();
		}
		*pppnames=ppnames;
		return cb;region_map.size();
	}
	//marshal(region_count,pp_regions.get(),pstream,0,conn_uuid);
	//std::vector<ipc_region_base_t* > pp_regions(region_count);
	region_map_t region_map;
	lh_marshal_t lh_marshal;
	region_list_t region_cache;
	v_buf<char*> names;
	DWORD tid;
	
};
};
//