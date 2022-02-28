#pragma once
// mbv_mm.h
#include "marshal_by_value_base.h"
#include "ipc_utils.h"
#include "sa_utils.h"
#include "sm_region.h"
#include "sparse_file_map.h"
#include "singleton_utils.h"
#include "variant_resolver.h"
#include "dispfast.h"
namespace ipc_utils {	

//#define f_attrib_ronly uint64_t(0x8000000000000000)

	namespace mbv_mm{

		//
		// 
		
		typedef ULONGLONG uint64_t; 
		typedef LONGLONG int64_t; 

		//enum{			f_attrib_ronly=0x8000000000000000		};//(uint64_t(1)<<31)		};



		typedef sm_region::sm_region_base_t region_base_t;
		typedef sm_region::sm_region_t region_t;
		typedef moniker_parser_t<wchar_t> parser_t;

		typedef sm_region::region_cache_t<2>  region_cache_t;

		const uint64_t f_attrib_ronly =0x8000000000000000;

		static void dbg_print(const wchar_t * fmt,...)
		{	
			va_list argptr;
			va_start(argptr, fmt);
			v_buf<wchar_t> buf;
			wchar_t* pstr=buf.vprintf(fmt,argptr);
			OutputDebugStringW(v_buf<wchar_t>().printf(L"[%d:%d]:%s\n",GetCurrentThreadId(),GetCurrentProcessId(),pstr) );
		}




		struct mm_context_data_t{
			HANDLE hfile;
			HANDLE hmap;
			HANDLE hmutex;
			uint64_t attrib;
			region_base_t region;
			BSTR infobuffer;


			inline bool is_ro(){

				return !!(f_attrib_ronly&attrib);
			}

			inline DWORD map_access()
			{
				return is_ro()?FILE_MAP_READ:FILE_MAP_ALL_ACCESS;
			}

			static inline HANDLE dup_handle(HANDLE h)				
			{
				if(h!=INVALID_HANDLE_VALUE)
					h=ipc_utils::dup_handle(h);
				return h;

			}

			inline DWORD try_lock(DWORD tio=0){					 
				return is_ro()||(!hmutex)||(WAIT_TIMEOUT!=WaitForSingleObject(hmutex,tio));
			}

			inline DWORD lock(){
				return try_lock(INFINITE);
			}

			inline BOOL unlock(){
				return is_ro()||(!hmutex )|| ReleaseMutex(hmutex);
			}


			inline void clear(bool fsize_truncate=false )
			{
				
				CloseHandle(ipc_utils::make_detach(hmap));

				if(fsize_truncate)
				{						
					spfm::sparse_file_map<>::set_file_size(hfile,region.end.QuadPart);
				}
				CloseHandle(ipc_utils::make_detach(hfile));
				SysFreeString(ipc_utils::make_detach(infobuffer));
				CloseHandle(ipc_utils::make_detach(hmutex));

			}

			inline void detach_to(mm_context_data_t& cd)
			{
				/*
				cd.hmutex=ipc_utils::make_detach(hmutex);
				cd.hmap=ipc_utils::make_detach(hmap);
				cd.hfile=ipc_utils::make_detach(hfile);
				cd.infobuffer=ipc_utils::make_detach(infobuffer);
				//cd.attrib=ipc_utils::make_detach(attrib);
				//cd.region=ipc_utils::make_detach(region);
				cd.attrib=attrib;
				cd.region=region;
				*/

				cd=make_detach(*this,mm_context_data_t());
			}

			inline HRESULT detach_to(mm_context_data_t* pcd)
			{
				if(!pcd) return E_POINTER;
				detach_to(*pcd);
				return S_OK;
			}


			mm_context_data_t& copy_from(mm_context_data_t& cd)
			{
				clear();
				hfile=dup_handle(cd.hfile);
				hmap=dup_handle(cd.hmap);
				hmutex=dup_handle(cd.hmutex);
				region=cd.region;
				attrib=cd.attrib;
				long len=SysStringByteLen(cd.infobuffer);
				if(len)
				{
					infobuffer=SysAllocStringByteLen(0,len);
					memcpy(infobuffer,cd.infobuffer,len);
				}

				return *this;
			}


		};		


		struct mm_context_data_2_t:mm_context_data_t
		{
			mm_context_data_2_t():mm_context_data_t(){  hfile=INVALID_HANDLE_VALUE; }
			~mm_context_data_2_t(){ clear();}

		};




		struct  __declspec(uuid("{612F4B92-75FE-4976-882D-F5D351CFF5C0}"))  co_mm_context_t{};

		//struct  mm_context_t: marshal_by_value_base::mbv_context_base_t<mm_context_t,__uuidof(co_mm_context_t),true>
		struct  mm_context_t : marshal_by_value_base::mbv_context_base_c_t<mm_context_t,co_mm_context_t, true>
		{



			mm_context_t():hfile(INVALID_HANDLE_VALUE),hmap(0),hmutex(0),attrib(0),region(),infobuffer(0){

				//locker_mutex_t lock(hmutex);

			}
			~mm_context_t(){

				context_data.clear();

			}


			struct loader_t
			{





				loader_t(parser_t& m):mn(m),pcontext(0),flags(0),byteLen(0),hr(0){

					if (!mn.args.is(L"data"))
						return;
					VARIANT v=mn.bind_args(L"data");						
					variant_resolver vr(&v,true);
					vt=vr.vt();
					IUnknown* punk;

					if(vt&VT_ARRAY)
					{							 
						vt&=(~VT_ARRAY);
						OLE_CHECK_VOID(hr=VARTYPE_finder_t<>::element_size(vt,&element_size));
						saptr.init(vr->parray);
						byteLen=saptr.sizeb;
						flags=1;
					}
					else if(((vt==VT_DISPATCH)||(vt==VT_UNKNOWN))&&(punk=vr->punkVal)){



						OLE_CHECK_VOID(hr=mbv_context.reset(v));
						if(1)
						{
							//OLE_CHECK_VOID(hr=mbv_context.reset(VARIANT(vr)));



							OLE_CHECK_VOID(hr=mbv_context->GetContext((void**)&pcontext));
							vt=0x00000FFFF&(pcontext->attrib);
							OLE_CHECK_VOID(hr=VARTYPE_finder_t<>::element_size(vt,&element_size));							 
							byteLen=pcontext->region.size();


							flags=2;
						};
					}

					// ????



				}
				inline operator HRESULT(){
					return hr;
				}

				HRESULT hr;

				int flags; 

				int64_t byteLen;
				long element_size;
				VARTYPE vt;
				mm_context_data_t* pcontext;




				sa_utils::safe_array_ptr_t saptr;
				ipc_utils::smart_ptr_t<i_mbv_context_lock> mbv_context;


				parser_t& mn;


			};






			static bool mapfile_open_check(parser_t& mn,spfm::sparse_file_map<>::pair_mmn_t& pairmn,int64_t funsafe, VARTYPE vt,int64_t size,HRESULT& hr,mm_context_data_t* pcontext_data){

				
				HANDLE hfile,hfile2=INVALID_HANDLE_VALUE;

				//FILE_MAP_READ
				hr=S_OK;
				if(!pairmn.mapname)
					return false;
				
				//HANDLE hmap=mn.push_handle(::OpenFileMappingW(FILE_MAP_READ,FALSE,pairmn.mapname));
				mm_context_data_2_t cd;
				HANDLE hmap;
				if(!(hmap=::OpenFileMappingW(FILE_MAP_READ|FILE_MAP_WRITE,FALSE,pairmn.mapname)))
					hmap=::OpenFileMappingW(FILE_MAP_READ,FALSE,pairmn.mapname);


				cd.hmap=hmap;


				bool ff, f=!!hmap;
				if(f){
										


					const DWORD daccess=FILE_READ_ATTRIBUTES|FILE_READ_EA,
						        sharemode=FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
								crds=OPEN_EXISTING,
								fattr=FILE_ATTRIBUTE_NORMAL;

					wchar_t* fn;
					//=mn.get_cache(1024);
					//ff=::GetMappedFileNameW(GetCurrentProcess(),p,fn,1024);
					fn=mn.get_filemap_name_by_handle(hmap,&hr);

					ff=hr==S_OK;

					cd.hfile=hfile=mn.open_info_handle(fn);
					ff=(INVALID_HANDLE_VALUE!=hfile);
					if(ff){

						cd.hfile=hfile=mn.open_info_handle(fn);
						//if(!(INVALID_HANDLE_VALUE!=hfile))
						//	return hr=HRESULT_FROM_WIN32(GetLastError()),true;

						if(pairmn){
							hr=HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
							if(!mn.cmp_file_id(hfile,pairmn.filename,&hr))
								return true;
						}




					

						      int64_t sizeh;


							 if(size<=0)
							 {
								 LARGE_INTEGER ll;
								 
								 //ll.HighPart=hfi.nFileSizeHigh;
								 //ll.LowPart=hfi.nFileIndexLow;
								 //sizeh=ll.QuadPart;
								 ff=GetFileSizeEx(hfile,&ll);
								 size=ll.QuadPart;

							 }

					}

							  
							 sm_region::region_cache_t<1,FILE_MAP_READ> rc(hmap);
							 char*p= rc.recommit(1,size-1);
							 if(!p)
                                return hr=HRESULT_FROM_WIN32(GetLastError()),true;

							 long element_size=1;						 
							 hr=VARTYPE_finder_t<>::element_size(vt,&element_size);
							 if(FAILED(hr))
								 return true;

							 if(!funsafe)
								 if(!(cd.hmutex=CreateMutex(0,0,0)))
									 return hr=HRESULT_FROM_WIN32(GetLastError()),true;

							 cd.region=region_t(size);
							 cd.attrib=vt|(WORD(element_size)<<16);


							 wchar_t* pinfo=  mn.printf(L"%s[%llu] pid=%d;( allocate: %s) file=%s; name=%s [hf=%0x]",VARTYPE_finder_t<>::jsname(vt,L"?"),size/element_size,GetCurrentProcessId(),GMKB(size),fn,pairmn.mapname,hfile);
							 cd.infobuffer=bstr_t(pinfo).Detach();
							 hr=cd.detach_to(pcontext_data);
							 return true;


												
						

					
					//hr=HRESULT_FROM_WIN32(GetLastError());
					//pairmn.filename=L"";

				}
				else hr=HRESULT_FROM_WIN32(GetLastError());





				return f;
			}




			static HRESULT mapfile_create(parser_t& mn,VARTYPE vt,int64_t size,mm_context_data_t* pcontext_data)
			{
				HRESULT hr;

				wchar_t* filemapname=mn.undecorateQQ((wchar_t*)mn.bind_args(L"name").def(bstr_t()));
				wchar_t* filename=0;		
				int funsafe=mn.bind_args(L"flags.unsafe").def<int>(0);
				spfm::sparse_file_map<>::pair_mmn_t pairmn(filemapname);

				HANDLE hfile=INVALID_HANDLE_VALUE;

				 bool ffnExist=mapfile_open_check(mn,pairmn,funsafe,vt,size,hr,pcontext_data);
				 if(ffnExist)
					 return hr;





				long element_size;

				mm_context_data_2_t cd;



				OLE_CHECK_hr(VARTYPE_finder_t<>::element_size(vt,&element_size));



				int mode=O_RDWR;
				bool fopen_existing;
				const DWORD O_TMP=O_TEMPORARY|_O_SHORT_LIVED;

				mode=mn.bind_args(L"flags").def<int>(mode); 

				if(mn.bind_args(L"flags.temp").def<int>(0))
					mode|=O_TMP;


				if(mn.bind_args(L"flags.open_always").def<int>(1))
					mode|=O_CREAT;

				if(fopen_existing=mn.bind_args(L"flags.open_existing").def<int>(0))
					mode|=_O_EXCL;

				if(mn.bind_args(L"flags.create_always").def<int>(0))
					mode|=O_CREAT|O_TRUNC;

				


				bool fsparse=mn.bind_args(L"flags.sparse").def<int>(0);
				int  compress=mn.bind_args(L"flags.compress").def<int>(0);

				



				




				if(pairmn) filename=pairmn.filename;


				if(fsparse||compress||filename)
				{


					if(!filename) 
					{

						filename=mn.tmp_file_name2(L"~mm~");
						mode|=O_TMP;
					}
					else
					{
						OLE_CHECK_hr(mn.expand_file_path(filename,&filename,0,fopen_existing));
						//HRESULT expand_file_path(const wchar_t* fnc,wchar_t** pp_fullpathname,const wchar_t* search_pathc=0,int fcheck_exists=1,const wchar_t* ext_def=0)

					}
					HANDLE hf;
					if(INVALID_HANDLE_VALUE==(hf=cd.hfile=spfm::sparse_file_map<>::create_file(filename,mode,SH_DENYNO,0x0080,true)))
						return hr=HRESULT_FROM_WIN32(GetLastError());



					if (fsparse) 
						OLE_CHECK_hr(spfm::sparse_file_map<>::set_sparse(hf));
					if (compress) 
						OLE_CHECK_hr(spfm::sparse_file_map<>::set_compression(hf,compress));


					if(size>0)
					{
						if(size!=spfm::sparse_file_map<>::set_file_size2(hf,size))
							return hr=HRESULT_FROM_WIN32(GetLastError());
					}
					else 
						size=spfm::sparse_file_map<>::file_size(hf);


				}







				if(size<0) return E_INVALIDARG;// can be ==0 future...

				cd.region=region_t(size);
				cd.attrib=vt|(WORD(element_size)<<16);











				ULARGE_INTEGER ll;
				ll.QuadPart=size;
				ll=sm_region::size_align(ll);						
				if(ll.QuadPart)
				{

					if(!(cd.hmap=CreateFileMappingW(cd.hfile,0,PAGE_READWRITE,ll.HighPart,ll.LowPart,pairmn.mapname)))
						return hr=HRESULT_FROM_WIN32(GetLastError());

				}

				if(!funsafe)
					if(!(cd.hmutex=CreateMutex(0,0,0)))
						return hr=HRESULT_FROM_WIN32(GetLastError());

				//const double gb=1024*1024*1024;
				wchar_t* pinfo=  mn.printf(L"%s[%llu] pid=%d;( allocate: %s) file=%s; name=%s",VARTYPE_finder_t<>::jsname(vt,L"?"),size/element_size,GetCurrentProcessId(),GMKB(ll.QuadPart),filename,pairmn.mapname);
				cd.infobuffer=bstr_t(pinfo).Detach();
				hr=cd.detach_to(pcontext_data);				 

				return hr;

			}



			HRESULT create_mapfile(parser_t& mn,loader_t &loader)
			{
				HRESULT hr;
				mm_context_data_2_t cd;//={INVALID_HANDLE_VALUE};






				VARTYPE vt;
				long element_size;

				bool ftypedef=mn.args[L"type"].is();


				wchar_t*  ptype=mn.trim((wchar_t* )mn.bind_args(L"type").def<bstr_t>(L"byte"));
				OLE_CHECK_hr(VARTYPE_finder_t<>::type_by_name(ptype,&vt));
				OLE_CHECK_hr(VARTYPE_finder_t<>::element_size(vt,&element_size));

				int64_t byte_offset=0;



				if(loader.flags&1)
				{

					if(!ftypedef)
					{
						int64_t sz=loader.byteLen,sz2;
						char* p=loader.saptr;
						OLE_CHECK_hr(mapfile_create(mn,loader.vt,loader.byteLen,&cd));
						sz2= sm_region::mp_copy<20>(cd.hmap,p,sz);
						if(sz2!=sz) return HRESULT_FROM_WIN32(GetLastError());

						return hr=cd.detach_to(&context_data);

					}
					else return hr=E_NOTIMPL;


				}



				int64_t unit=mn.bind_args(L"unit").def<double>(1);
				int64_t length=mn.bind_args(L"length").def<double>(mn.bind_args(L"len").def<double>(-1));
				int64_t byte_length=mn.bind_args(L"bytelength").def<double>(mn.bind_args(L"bytelen").def<double>(element_size*length));
				int64_t capacity;
				capacity=mn.bind_args(L"capacity").def<double>(byte_length);

				length*=unit;
				byte_length*=unit;
				capacity*=unit;


				if(loader.flags==0)					
					return hr=mapfile_create(mn,vt,capacity,&context_data);	


				//int64_t byte_offset=mn.bind_args(L"byteoffset").def<double>(0);

				if(loader.flags&2){

					int64_t llen=loader.byteLen;

					if(!ftypedef)
					{
						vt=loader.vt;
						element_size=loader.element_size;
						if (mn.args.is(L"len"))
						{
							length = mn.bind_args(L"len").def<double>(-1);
							byte_length = element_size * length;

						}
						if(mn.args.is(L"length"))
						{
							length=mn.bind_args(L"length").def<double>(-1);
							byte_length=element_size*length;

						}
						if (mn.args.is(L"bytelen"))
							byte_length = mn.bind_args(L"bytelen").def<double>(-1);
						if(mn.args.is(L"bytelength"))
							byte_length=mn.bind_args(L"bytelength").def<double>(-1);


					}

					bool ftypeequil=(vt==loader.vt);

					if(ftypeequil&&mn.args.is(L"offset"))
					{
						byte_offset=mn.bind_args(L"offset").def<double>(byte_offset)*element_size;
					}

					if(mn.args.is(L"byteoffset"))
						byte_offset=mn.bind_args(L"byteoffset").def<double>(byte_offset);


					if(byte_length<=0)
						byte_length=llen-byte_offset;			  



					if((byte_length+byte_offset)>llen)
						return E_INVALIDARG;

					OLE_CHECK_hr(mapfile_create(mn,vt,byte_length,&cd));

					mm_context_data_t& cds=*loader.pcontext;


					HANDLE hs=cds.hmap,hd=cd.hmap;
					region_base_t rs=cds.region,rd=cd.region;

					int64_t sz2;
					int64_t off_src=rs.begin.QuadPart+byte_offset;


					{
						locker_t<mm_context_data_t> lock_dest(cd);
						i_mbv_context_lock::locker_t lock_src(loader.mbv_context);
						sz2= sm_region::mm_copy<20>(hd,hs,byte_length,rd.begin.QuadPart,off_src);
					}
					if(sz2!=byte_length) return HRESULT_FROM_WIN32(GetLastError());
					else hr=cd.detach_to(&context_data);			  



				}

				return hr;
			}




			HRESULT bind_mapfile(parser_t& mn,loader_t &loader)
			{
				HRESULT hr=E_NOTIMPL;

				VARTYPE vt;
				long element_size;


				mm_context_data_2_t cd;

				mm_context_data_t& cds=*loader.pcontext;

				bool ftypedef=mn.args[L"type"].is();

				wchar_t*  ptype=mn.trim((wchar_t* )mn.bind_args(L"type").def<bstr_t>(L"byte"));
				OLE_CHECK_hr(VARTYPE_finder_t<>::type_by_name(ptype,&vt));
				OLE_CHECK_hr(VARTYPE_finder_t<>::element_size(vt,&element_size));



				int64_t length=mn.bind_args(L"length").def<double>(-1);
				int64_t byte_length=mn.bind_args(L"bytelength").def<double>(element_size*length);
				int64_t capacity;
				int64_t byte_offset=0;		











				int64_t llen=loader.byteLen;

				if(!ftypedef)
				{
					vt=loader.vt;
					element_size=loader.element_size;
					if(mn.args.is(L"length"))
					{
						length=mn.bind_args(L"length").def<double>(-1);
						byte_length=element_size*length;

					}
					if(mn.args.is(L"bytelength"))
						byte_length=mn.bind_args(L"bytelength").def<double>(-1);


				}

				if(byte_length<0)
					byte_length=llen-byte_offset;			  



				if((byte_length+byte_offset)>llen)
					return E_INVALIDARG;


				bool ftypeequil=(vt==loader.vt);				

				if(ftypeequil&&mn.args.is(L"offset"))
				{
					byte_offset=mn.bind_args(L"offset").def<double>(byte_offset)*element_size;
				}

				if(mn.args.is(L"byteoffset"))
					byte_offset=mn.bind_args(L"byteoffset").def<double>(byte_offset);




				if((byte_length+byte_offset)>llen)
					return E_INVALIDARG;		   

				cd.copy_from(cds);
				cd.region=region_t(byte_length,byte_offset);
				cd.attrib=vt;
				cd.attrib=vt|(WORD(element_size)<<16);

				hr=cd.detach_to(&context_data);		

				return hr;
			}



			HRESULT Init(BOOL fUnmarshal,IBindCtx* bctx,LPOLESTR pszDisplayName,DWORD& flags){

				//flags=(1<< MSHCTX_INPROC)|0x10000; 
				if(fUnmarshal) return S_OK;

				HRESULT hr;
				//

				parser_t mn(pszDisplayName,bctx);
				loader_t loader(mn);
				OLE_CHECK_hr(loader);

				bool f_link=mn.bind_args(L"link").def<int>(0);

				bool f=(loader.flags&2);
				if(f_link&&f) hr=bind_mapfile(mn,loader);
				else 
					hr=create_mapfile(mn,loader);

				region_cache=region_cache_t(hmap,region.begin.QuadPart);
				return hr;				





			}



			HRESULT marshal_to_process(i_marshal_helper_t* helper,DWORD pid_target,IStream* pStm){

				HRESULT hr;
				ULONG l;

				OLE_CHECK_hr(helper->marshal_hko(pStm,hfile));		
				OLE_CHECK_hr(helper->marshal_hko(pStm,hmap));		
				OLE_CHECK_hr(helper->marshal_hko(pStm,hmutex));
				OLE_CHECK_hr(pStm->Write(&attrib,sizeof(attrib),&l));
				OLE_CHECK_hr(pStm->Write(&region,sizeof(region_base_t),&l));
				OLE_CHECK_hr(helper->marshal_BSTR(infobuffer,pStm));


				dbg_print(L"mbv_mm:marshal_to_process\n");


				return hr;
			}
			HRESULT unmarshal(i_marshal_helper_t* helper,IStream* pStm){
				HRESULT hr;
				ULONG l;

				OLE_CHECK_hr(helper->unmarshal_hko(pStm,&hfile));		
				OLE_CHECK_hr(helper->unmarshal_hko(pStm,&hmap));		
				OLE_CHECK_hr(helper->unmarshal_hko(pStm,&hmutex));
				OLE_CHECK_hr(pStm->Read(&attrib,sizeof(attrib),&l));
				OLE_CHECK_hr(pStm->Read(&region,sizeof(region_base_t),&l));
				OLE_CHECK_hr(helper->unmarshal_BSTR(pStm,&infobuffer));

				region_cache=region_cache_t(hmap,region.begin.QuadPart);

				dbg_print(L"mbv_mm:unmarshal\n");

				return hr;

			}

			inline long get_element_size()
			{
				return 0x0FFFF&(attrib>>16);
			}
			inline long get_vt()
			{
				return 0x0FFFF&(attrib>>0);
			}

			inline int64_t get_count()
			{

				return region.size()/get_element_size();

			}


			inline HRESULT check_bound(int64_t ib)
			{
				return ((0<=ib)&&(ib<region.size()))?0:DISP_E_BADINDEX;
			}

			inline HRESULT set_element(int64_t i,VARIANT& ve)
			{
				HRESULT hr;
				OLE_CHECK_hr_cond(!is_ro(),E_ACCESSDENIED);
				long es=get_element_size();
				OLE_CHECK_hr(check_bound(i*=es));
				VARTYPE vt;
				VARIANT v={};
				if((vt=get_vt())!=v.vt)
					OLE_CHECK_hr(VariantChangeType(&v,&ve,0,vt));

				char* ptr=region_cache.recommit(es,i);
				if(!ptr) return hr=HRESULT_FROM_WIN32(GetLastError());
				memcpy(ptr,&v.intVal,es);

				return S_OK;
			}

			inline HRESULT _set_elements(int count,VARIANT* pv,VARIANT* presult)
			{
				HRESULT hr;
				int c=count-1;
				VARIANT t={};
				OLE_CHECK_hr(VariantChangeType(&t,pv+c,0,VT_I8));
				int64_t i=t.llVal;
				t.llVal+=c;
				while(c>0)
				{
					OLE_CHECK_hr(set_element(i++,pv[--c]));
				}

				*presult=t;
				return hr;

			}

			inline  HRESULT get_element(int64_t i,VARIANT *pv)
			{
				HRESULT hr;
				long es=get_element_size();
				OLE_CHECK_hr(check_bound(i*=es));
				VARTYPE vt=get_vt();

				pv->vt=vt;								
				char* ptr=region_cache.recommit(es,i);
				if(!ptr) return hr=HRESULT_FROM_WIN32(GetLastError());
				memcpy(&pv->intVal,ptr,es);
				if((vt==VT_I8)||(vt==VT_UI8))
				{
					OLE_CHECK_hr(VariantChangeType(pv,pv,0,VT_R8));
				}



				return S_OK;
			}

			inline  HRESULT get_element(VARIANT vi,VARIANT *pv)
			{
				HRESULT hr;					
				OLE_CHECK_hr(VariantChangeType(&vi,&vi,0,VT_I8));
				hr=get_element(vi.llVal,pv);
				return hr;

			}



			inline HRESULT for_each(IDispatch* callback)
			{

				HRESULT hr=0;
				OLE_CHECK_PTR(callback);
				int64_t cb=get_count();
				VARIANT v[2]={{get_vt()},{VT_R8}};

				smart_ptr_t<IDispatchEx> callbackjs;
				bool fjs=SUCCEEDED(callback->QueryInterface(__uuidof(IDispatchEx),callbackjs._ppQI()));
				DISPPARAMS dps={v,0,2,0};
				locker_t<mm_context_data_t> lock(context_data);
				VARTYPE vt=get_vt();

				if(fjs)

					for(int64_t k=0;k<cb;k++)
					{
						com_variant_t res;
						//array_type_class_t::ref(v[0])=pf[k];

						OLE_CHECK_hr(get_element(k,v));								
						v[1].dblVal=double(k);
						OLE_CHECK_hr(callbackjs->InvokeEx(0, LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD,&dps,&res,0,0));
						if(res.vt!=VT_EMPTY)
						{							
							if(res.vt!=vt) OLE_CHECK_hr(VariantChangeType(&res,&res,VARIANT_NOVALUEPROP,vt));
							OLE_CHECK_hr(set_element(k,res));								
						}
						//pf[k]=array_type_class_t::ref(res);

					}

				else 

					for(int64_t k=0;k<cb;k++)
					{
						com_variant_t res;
						//array_type_class_t::ref(v[0])=pf[k];
						OLE_CHECK_hr(get_element(k,v));								
						v[1].dblVal=double(k);
						OLE_CHECK_hr(callback->Invoke(0, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD,&dps,&res,0,0));
						if(res.vt!=VT_EMPTY)
						{							
							if(res.vt!=vt) OLE_CHECK_hr(VariantChangeType(&res,&res,VARIANT_NOVALUEPROP,vt));
							OLE_CHECK_hr(set_element(k,res));								
						}
						//pf[k]=array_type_class_t::ref(res);

					}


					return hr;
			}





			//BEGIN MATLAB VB utils..

			HRESULT toVBARRAY(int64_t boff,int64_t eoff,VARIANT& result)
			{
				HRESULT hr;
				VARTYPE vt=get_vt();
				long esize=VARTYPE_finder_t<>::element_size(vt);
				ULONG l=eoff-boff; 
				SAFEARRAYBOUND sb={l,0};
				com_variant_t vsa;
				if(vsa.parray=SafeArrayCreate(VT_VARIANT,1,&sb))
					vsa.vt=VT_ARRAY|VT_VARIANT;
				else return E_OUTOFMEMORY;
				sa_utils::safe_array_ptr_t sa_ptr(vsa.parray);
				OLE_CHECK_hr(sa_ptr);
				VARIANT* pv=sa_ptr;
				{
					locker_t<mm_context_data_t> lock(context_data);	
					//l=sm_region::pm_copy<20>((char*)sa_ptr,hmap,sizeb,region.begin.QuadPart);
					for(ULONG i=0;i<l;i++ )
					{
						char* ptr=region_cache.recommit(esize,boff);
						boff+=esize;
						memcpy(&pv[i].intVal,ptr,esize);
						pv[i].vt=vt;
					}
				}

				result=vsa.Detach();

				return S_OK;

			}

			HRESULT toSAFEARRAY(VARTYPE vt,VARIANT vindx,VARIANT& result)
			{
				HRESULT hr;
				ipc_utils::smart_ptr_t<i_mm_region_ptr> rg;
				OLE_CHECK_hr(rg.reset(vindx));
				i_mm_region_ptr::ptr_t<SAFEARRAYBOUND> indx(rg,-1);
				OLE_CHECK_hr(indx);

				uint64_t l,sizeb=region.size(),count=get_count();

				SAFEARRAYBOUND* psb=indx;
				int nd=indx.ByteLength/sizeof(SAFEARRAYBOUND);
				if(nd<1)
					return hr=E_INVALIDARG;

				l=sa_utils::SAFEARRAY_total_length(nd,psb);
				if(l!=count)
					return hr=E_INVALIDARG;


				//if(vsa.parray=SafeArrayCreate(vt,nd,sb))
				com_variant_t vsa;

				if(vsa.parray=SafeArrayCreate(vt,nd,psb))
					vsa.vt=VT_ARRAY|vt;
				else return hr=E_OUTOFMEMORY;

				sa_utils::safe_array_ptr_t sa_ptr(vsa.parray);
				OLE_CHECK_hr(sa_ptr);




				{
					locker_t<mm_context_data_t> lock(context_data);	
					l=sm_region::pm_copy<20>((char*)sa_ptr,hmap,sizeb,region.begin.QuadPart);

				}

				if(l!=sizeb)
					return  hr=HRESULT_FROM_WIN32(GetLastError());

				result=vsa.Detach();



				return hr;
			}

			HRESULT toSAFEARRAY(VARTYPE vt,VARIANT& result){
				HRESULT hr;
				long esize=VARTYPE_finder_t<>::element_size(vt);
				if(!esize) OLE_CHECK_hr(DISP_E_UNKNOWNNAME);

				uint64_t sizeb=region.size(),count=sizeb/uint64_t(esize),l,cc;
				const uint64_t maxbound=uint64_t(1)<<32;

				int nd=1;
				LARGE_INTEGER ll;

				long hidim=1;

				if(count>maxbound)
				{
					nd=2;
					while(count>maxbound)
					{
						hidim=hidim<<1;
						count=count>>1;
					}
				}




				SAFEARRAYBOUND sb[2]={{count},{hidim}};


				com_variant_t vsa;

				if(vsa.parray=SafeArrayCreate(vt,nd,sb))
					vsa.vt=VT_ARRAY|vt;
				else return E_OUTOFMEMORY;

				sa_utils::safe_array_ptr_t sa_ptr(vsa.parray);
				OLE_CHECK_hr(sa_ptr);
				{
					locker_t<mm_context_data_t> lock(context_data);	
					l=sm_region::pm_copy<20>((char*)sa_ptr,hmap,sizeb,region.begin.QuadPart);

				}

				if(l!=sizeb)
					return  hr=HRESULT_FROM_WIN32(GetLastError());

				result=vsa.Detach();
				return S_OK;



			}

			HRESULT raw_load(VARIANT v,arguments_t& arguments)
			{

				
				HRESULT hr;
				OLE_CHECK_hr_cond(!context_data.is_ro(),E_ACCESSDENIED);
				
				variant_resolver vr(&v,true);
				VARTYPE vt=vr.vt();
				if(!((vt&VT_ARRAY)&&((vt&(~VT_ARRAY))!=VT_VARIANT)))
					arguments.raise_error(L"raw SAFEARRAY type mismatch");
				sa_utils::safe_array_ptr_t sa_ptr(vr->parray);			
				OLE_CHECK_hr(sa_ptr);

				int64_t sizeb=region.size(),l;

				if(sizeb!=sa_ptr.sizeb)
					arguments.raise_error(L"raw SAFEARRAY length mismatch");
				{
					locker_t<mm_context_data_t> lock(context_data);	
					l=sm_region::mp_copy<20>(hmap,(char*)sa_ptr,sizeb,region.begin.QuadPart);

				}

				hr=(l==sizeb)?S_OK:HRESULT_FROM_WIN32(GetLastError());


				return hr;
			}

			// prop[..] checks
			HRESULT  setset(arguments_t& arguments){

				HRESULT hr;

				
				ipc_utils::smart_ptr_t<i_mbv_context_lock> context_s;
				int64_t i;

				VARTYPE vts;
				int64_t count,byteOffset,byteOffset_src,sizeb;
				long ElementSize;
				//bool f_src_count,f_dest_offset;




				if(SUCCEEDED(hr=arguments[0].QueryInterface(context_s._address())))
				{



					mm_context_data_t* pcontext_data_s;

					OLE_CHECK_hr(context_s->GetContext((void**)&pcontext_data_s));
					OLE_CHECK_hr(context_s->GetElementInfo(&vts,&count,&ElementSize));





					i=arguments[1].def<double>(0);

					count=arguments[2].def<double>(count);


					byteOffset_src=arguments[3].def<double>(0);
					byteOffset_src*=ElementSize;




					byteOffset=i*get_element_size();
					sizeb=count*ElementSize;
					if((sizeb+byteOffset)>region.size())
						return hr=DISP_E_BADINDEX;					  						  

					HANDLE hs=pcontext_data_s->hmap,hd=hmap;
					{
						locker_t<mm_context_data_t> lock(context_data);
						i_mbv_context_lock::locker_t lock_src(context_s);

						uint64_t offs_dest=region.begin.QuadPart+byteOffset;
						uint64_t offs_src=pcontext_data_s->region.begin.QuadPart+byteOffset_src;

						i=sm_region::mm_copy<20>(hd,hs,sizeb,offs_dest,offs_src);
					}

					if(i!=sizeb)
						hr=HRESULT_FROM_WIN32(GetLastError());




				}	
				else if((i=arguments[0].def<double>(-1))>=0)
				{
					locker_t<mm_context_data_t> lock(context_data);

					hr=set_element(i,arguments.result());
				}
				else
				{
					VARIANT a=arguments[0],v;
					variant_resolver vr(&a);
					v=vr;
					if(v.vt&VT_ARRAY)
					{
						vts=v.vt&(~VT_ARRAY);

						OLE_CHECK_hr(VARTYPE_finder_t<>::element_size(vts,&ElementSize));
						sa_utils::safe_array_ptr_t sa_ptr(v.parray);
						OLE_CHECK_hr(sa_ptr);
						if(sa_ptr.element_size!=ElementSize) 
							return DISP_E_TYPEMISMATCH;
						count=sa_ptr.length;


						i=arguments[1].def<double>(0);

						count=arguments[2].def<double>(count);


						byteOffset_src=arguments[3].def<double>(0);
						byteOffset_src*=ElementSize;




						byteOffset=i*get_element_size();
						sizeb=count*ElementSize;
						if((sizeb+byteOffset)>region.size())
							return hr=DISP_E_BADINDEX;					  						  

						HANDLE hd=hmap;
						char* p=sa_ptr;

						{
							locker_t<mm_context_data_t> lock(context_data);							  

							uint64_t offs_dest=region.begin.QuadPart+byteOffset;
							uint64_t offs_src=byteOffset_src;

							i=sm_region::mp_copy<20>(hd,p+offs_src,sizeb,offs_dest);
						}

						if(i!=sizeb)
							hr=HRESULT_FROM_WIN32(GetLastError());





					}
					else hr=E_INVALIDARG;			   



				}



				return hr;

			}

			inline HRESULT view_as(arguments_t& arguments,VARIANT& result)
			{
				HRESULT hr;

				const wchar_t* tn=VARTYPE_finder_t<>::jsname(get_vt(),L"ArrayBuffer");
				bstr_t typen=arguments[0].def<bstr_t>(tn);
				IDispatch* pdisp=arguments.self();//pcallback_context->pthis;


				VARIANT r={VT_DISPATCH};
				//double sizeb=
				double offsetb=arguments[1].def<double>(0);
				double sizeb=arguments[2].def<double>(region.size()-offsetb);					

				OLE_CHECK_hr(view_as_row(pdisp,typen,1,offsetb,sizeb,__uuidof(IDispatch),&r.byref));					

				result=r;

				return hr;

			}


			static inline const wchar_t* reparse_vt(const wchar_t* tname)
			{
				if(tname)
				{
					UINT_PTR itn=UINT_PTR(tname);
					if((0<itn)&&(itn<UINT_PTR(VT_TYPEMASK)))
						return VARTYPE_finder_t<>::name(itn,0);
				}
				return tname;

			}



			static inline HRESULT view_as_row(IUnknown* pOuter,const wchar_t* _tname, int flink,double byte_offset,double byte_length,REFIID iid,void** ppObj)
			{
				HRESULT hr;
				OLE_CHECK_PTR(ppObj);
				wchar_t* fmt;
				const wchar_t *tname=reparse_vt(_tname);


				if(tname){

					fmt=L"ltx.bind:mm_buffer:link=#1;data=#2; byteOffset=#3;byteLength=#4;type=#5";
				}
				else fmt=L"ltx.bind:mm_buffer:link=#1;data=#2; byteOffset=#3;byteLength=#4";					
				hr=parser_t::bindObject(fmt,iid,ppObj)(flink,pOuter,byte_offset,byte_length,tname);
				return hr;

			}

			inline HRESULT slice_subarray(int flink,arguments_t& arguments,VARIANT& result)
			{
				HRESULT hr;

				//const wchar_t* tn=VARTYPE_finder_t<>::jsname(get_vt(),L"ArrayBuffer");
				//bstr_t typen=tn;
				IDispatch* pdisp=arguments.pcallback_context->pthis;
				double c=get_count();
				double b=arguments[0].def<double>(0);
				double e=arguments[1].def<double>(c);
				if(e>c) e=c;
				if(b>c) b=c;
				double es=get_element_size();
				double offsetb=es*b,sizeb=es*(e-b);

				VARIANT r={VT_DISPATCH};
				/*
				parser_t::bindObject_t   bind(L"ltx.bind:mm_buffer:link=#1;data=#2;length=#3;offset=#4",__uuidof(IDispatch),&r.byref);
				hr=bind(flink,pdisp,count,offset);
				if(SUCCEEDED(hr)) */
				OLE_CHECK_hr(view_as_row(pdisp,0,flink,offsetb,sizeb,__uuidof(IDispatch),&r.byref));				

				result=r;

				return hr;

			}

			inline HRESULT memfill(VARIANT vs,int64_t step=1,int64_t offs=0)
			{
				HRESULT hr;
				com_variant_t v;
				OLE_CHECK_hr(VariantChangeType(&v,&vs,0,get_vt()));
				long es=get_element_size();
				int64_t count=get_count(),i,sz=region.size();
				count-=offs;
				int64_t offsb=es*offs;



				{					
					locker_t<mm_context_data_t> lock(context_data);
					i=sm_region::mp_set<20>(hmap,&v.intVal,es,region.begin.QuadPart+offsb,count,step);
				}

				hr=(i==sz)?S_OK:HRESULT_FROM_WIN32(GetLastError());
				return hr;					

			}


			inline HRESULT setgroup(arguments_t& arguments)
			{
				HRESULT hr=S_OK;
				int c=arguments.argc;
				VARIANT voff={};
				VARIANT *pv=arguments.argv;
				if(!c) return hr;
				pv+=c-1;
				OLE_CHECK_hr(VariantChangeType(&voff,pv,0,VT_I4));
				for(int k=0;k<c-1;k++)
					OLE_CHECK_hr(set_element(voff.intVal+k,*(--pv)));						
				return hr;


			}






			// END MATLAB VB utils . .

			inline  HRESULT  on_get(DISPID id,const wchar_t* name,com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper){

				HRESULT hr;

				s_parser_t<wchar_t>::case_t<false> cn(name);
				int len=arguments.length();


				if(cn(4,L"item")&&(len==1))
				{
					locker_t<mm_context_data_t> lock(context_data);
					VARIANT& v=(VARIANT)arguments[0];
					return hr=get_element(v,&result);

				}

				if(cn(8,L"setgroup"))
				{
					locker_t<mm_context_data_t> lock(context_data);
					return hr=setgroup(arguments);

				}

				if(cn(7,L"fastbuf")||cn(7,L"lockbuf"))
				{

					result.vt=VT_DISPATCH;
					result.pdispVal=new IDispatch_fast_t<mm_context_t>(this,pcontainer_weakref);
					return hr=S_OK;

				}


				if(cn(L"refcount"))
				{
					result=int(ipc_utils::ref_count(arguments.pcallback_context->pthis));
					return S_OK;
				}
				//else if(parser_t::_lcheckni(name,L"length"))
				else if(cn(3,L"len"))
				{

					result=double(get_count());
					return S_OK;
				}
				else if(cn(7,L"bytelen"))
				{
					result=double(region.size());
					return S_OK;

				}
				else if(cn(L"byteoffset"))
				{
					result=double(region.begin.QuadPart);
					return S_OK;

				}
				else if(cn(L"BYTES_PER_ELEMENT"))
				{
					result=double(get_element_size());
					return S_OK;

				}					
				else if(cn(4,L"type")||cn(4,L"name"))
				{
					if(arguments[0].def<bool>(false))
						result=int(get_vt());
					else	result=VARTYPE_finder_t<>::jsname(get_vt(),L"ArrayBuffer");
					return S_OK;
				}
				else if(cn(L"set"))
				{
					OLE_CHECK_hr_cond(!is_ro(),E_ACCESSDENIED);
					hr=setset(arguments);
					return hr;
				}
				else if(cn(L"subarray"))
				{

					hr=slice_subarray(1,arguments,result);
					return hr;
				}
				else if(cn(L"slice"))
				{

					hr=slice_subarray(0,arguments,result);
					return hr;
				}
				else if(cn(4,L"viewas"))
				{

					hr=view_as(arguments,result);
					return hr;
				}
				else if(cn(L"foreach"))
				{
					smart_ptr_t<IDispatch> callback;
					OLE_CHECK_hr(arguments[0].QueryInterface(callback._address()));
					hr=for_each(callback);						
					return hr;
				}
				else if(cn(L"toSAFEARRAY"))
				{
					VARTYPE vt=get_vt();

					/*

					if(arguments.length())
					{						
					bstr_t tname=arguments[0].def<bstr_t>(bstr_t());
					//OLE_CHECK_hr(arguments[0].QueryInterface(callback._address()));
					if(!( tname.length()&&SUCCEEDED(VARTYPE_finder_t<>::type_by_name(tname,&vt))))			                				
					OLE_CHECK_hr(DISP_E_UNKNOWNNAME);
					}
					else vt=get_vt();
					*/

					//long hidim=arguments[0].def<int>(0);
					if(arguments.length()) hr=toSAFEARRAY(vt,arguments[0],result);
					else hr=toSAFEARRAY(vt,result);
					return hr;
				}
				else if(cn(L"toVBARRAY"))
				{
					VARTYPE vt;


					int64_t b=arguments[0].def<double>(0);
					int64_t size=arguments[1].def(double(get_count()));
					if((size>>32)) return E_INVALIDARG;
					//long hidim=arguments[0].def<int>(0);
					hr=toVBARRAY(b,b+size,result);
					return hr;
				}
				else if(cn(L"raw_load"))
				{
					OLE_CHECK_hr_cond(!is_ro(),E_ACCESSDENIED);
					hr=raw_load(arguments[0],arguments); 
					return hr;
				}
				else if(cn(4,L"fill"))
				{
                    OLE_CHECK_hr_cond(!is_ro(),E_ACCESSDENIED);
					int step=arguments[1].def<int>(1);
					int offs=arguments[2].def<int>(0);
					hr=memfill(arguments[0],step,offs); 
					return hr;
				}
				else if(cn(L"pid"))
				{
					arguments.result()=int(GetCurrentProcessId());
					return S_OK;
				}
				if(cn(L"tid"))
				{
					arguments.result()=int(GetCurrentThreadId());
					return S_OK;
				}
				if(cn(L"trylock",L"try_lock"))
				{
					//arguments.result()=int(GetCurrentThreadId());
					arguments.result()=!!try_lock(arguments[0].def<int>(0));

					return S_OK;
				}
				if(cn(L"lock"))
				{
					//arguments.result()=int(GetCurrentThreadId());
					arguments.result()=!!lock();

					return S_OK;
				}
				if(cn(L"unlock"))
				{
					//arguments.result()=int(GetCurrentThreadId());
					arguments.result()=unlock();

					return S_OK;
				}
				if(cn(L"close"))
				{
					//arguments.result()=int(GetCurrentThreadId());
					context_data.clear();
					//arguments.result()=unlock();

					return S_OK;
				}
				if(cn(L"discard"))
				{
					//arguments.result()=int(GetCurrentThreadId());
					bool fst;				

					fst=arguments[0].def<bool>(false);
					context_data.clear(fst);
					region_cache.decommit();
					//arguments.result()=unlock();

					return S_OK;
				}
				if(cn(L"decommit"))
				{
					region_cache.decommit();
					return S_OK;
				}
				if(cn(L"set_readonly"))
				{ 
					
					/*bool f=!is_ro();
					if(f)
					{
                     context_data.attrib|=f_attrib_ronly; 
                     region_cache.reset_access(map_access());
					}
					*/
					bool f=arguments[0].def<bool>(true);
					if(f!=is_ro()) 
					{
                        if(f) 
							context_data.attrib|=f_attrib_ronly; 
						else 
							context_data.attrib&=~f_attrib_ronly; 

                        region_cache.reset_access(map_access());

						result=true;
					}
					else
						result=false;

					return S_OK;
				}

				if (cn.eqlen(L"__raw_handle_map__",5)){
					VARIANT v = { VT_R8 };
					v.dblVal = double((int64_t)region_cache.hmap);
					arguments.result() = v;// variant_t(double((void*)region_cache.hmap));
					return S_OK;
				}
				if (cn(L"__pid__")) {
					VARIANT v = { VT_R8 };
					v.dblVal = double((int64_t)GetCurrentProcessId());
					arguments.result() = v;// variant_t(double((void*)region_cache.hmap));
					return S_OK;
				}

				if(cn(L"commit_test"))
				{
					//arguments.result()=int(GetCurrentThreadId());
					int64_t sz;						
					sz=arguments[0].def<double>(0);
					char* p=region_cache.recommit(sz,0);
					if(arguments.length()>1)
					{
						int v=arguments[0].def<int>(0);
						char* pend=p+sz;

						for(;p<pend;p++) *p=char(v);						 

					}

					arguments.result()=VARIANT_BOOL(p!=0);

					return S_OK;
				}

				;


				return E_NOTIMPL;
			};
			inline   HRESULT  on_put(DISPID id,const wchar_t* name,com_variant_t& value,arguments_t& arguments,i_marshal_helper_t* helper){
				return E_NOTIMPL;
			};

			inline  void  oncallback( com_variant_t& result,arguments_t& arguments,i_marshal_helper_t* helper){

				wchar_t* name=arguments.pm_name();
				DISPID id=arguments.id();
				HRESULT& hr=arguments.hr;
				int64_t indx;
				bool f_is_get=arguments.is_get_flag();

				int len=arguments.length();

				if(id==DISPID_VALUE)
				{




					if(f_is_get)
					{
						if(len)
						{
							if(len==1)
								hr=get_element(arguments.argv[0],&result);
							else hr=_set_elements(len,arguments.argv,&result);



						}
						else{
							parser_t mn;

							const wchar_t* ptype=VARTYPE_finder_t<>::jsname(get_vt(),L"?");
							//bstr_t b=ptype;
							//b+=bstr_t(L" :DataView from: ")+ infobuffer;

							result=(wchar_t*)mn.printf(L"%s[%llu] {byteOffset:%llu ; from:%s}",ptype,get_count(),region.begin.QuadPart,infobuffer); 
							hr=S_OK;
						}



					}
					else {

						if(len==1)
							hr=set_element(arguments.argv[0],result);
						else hr=E_NOTIMPL;


					}

					return;

				}






				if(arguments.check_index(&indx))
				{				
					//
					locker_t<mm_context_data_t> lock(context_data);
					if(f_is_get)
						hr=get_element(indx,&result);
					else hr=set_element(indx,result);
					return;
				}


				if(f_is_get)
					hr=	on_get(id,name,result, arguments,helper);
				else hr=on_put(id,name,result, arguments,helper);
			};




			struct mm_region_ptr_t:base_aggregator_t<mm_region_ptr_t,i_mm_region_ptr>
			{
				mm_region_ptr_t(mm_context_t* pc,iaggregator_container* container)
					:base_aggregator_t<mm_region_ptr_t,i_mm_region_ptr>(container)
					,pcontext(pc){}	
				~mm_region_ptr_t(){

					OutputDebugStringW(L"~mm_region_ptr_t\n");

				}


				inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ) 
				{              
					//return  inner_QI(Outer,riid,__uuidof(IntfA),ppObj,hr);
					bool f;
					f  =(riid==__uuidof(i_mm_region_ptr));
					f=f||(riid==__uuidof(i_mbv_context_lock));
					f=f||(riid==__uuidof(i_mbv_context));
					f=f||(riid==__uuidof(i_mm_cache_buffer_ptr));
					f=f||(riid==__uuidof(i_mbv_buffer_ptr));



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
					*ppcontext=(void*)&(pcontext->context_data);
					return S_OK;
				}

				virtual HRESULT STDMETHODCALLTYPE Lock(){

					pcontext->lock();
					return S_OK;
				};
				virtual HRESULT STDMETHODCALLTYPE Unlock(){

					pcontext->unlock();
					return S_OK;
				};
				virtual HRESULT STDMETHODCALLTYPE GetElementInfo(VARTYPE* pvt,int64_t* pCount,long* pElementSize)
				{
					pvt&&(*pvt=pcontext->get_vt());
					pCount&&(*pCount=pcontext->get_count());
					pElementSize&&(*pElementSize=pcontext->get_element_size());
					return S_OK;

				}

				virtual HRESULT STDMETHODCALLTYPE GetPtr(void** pptr,int64_t* pByteLength){


					OLE_CHECK_PTR(pptr);
					HRESULT hr=S_OK;
					int64_t szb=pcontext->region.size();
					if(!szb)
					{
						*pptr=0;
						pByteLength&&(*pByteLength=szb);  
						return S_OK;
					}

					region_cache_t& rc=pcontext->region_cache;
					char* p=rc.recommit(szb,0);
					if(p) {

						*pptr=p;
						pByteLength&&(*pByteLength=szb);  


					}
					else hr=HRESULT_FROM_WIN32(GetLastError());

					return hr;



				};

				virtual HRESULT STDMETHODCALLTYPE DataViewAs(BSTR type,BOOL link,int64_t ByteOffset,int64_t ByteLength,REFIID iid,void** ppObj)
				{
					HRESULT hr;
					hr=mm_context_t::view_as_row(pOuter,type,link,ByteOffset,ByteLength,iid,ppObj);

					return hr;

				}


				virtual HRESULT STDMETHODCALLTYPE GetTotalSize(int64_t* pByteLength=0){
					OLE_CHECK_PTR(pByteLength);
					*pByteLength=pcontext->region.size();  
					return S_OK;
				}




				virtual HRESULT STDMETHODCALLTYPE CommitRegionPtr(int64_t ByteOffset,int64_t ByteLength,void** pptr){

					OLE_CHECK_PTR(pptr);
					HRESULT hr;
					region_base_t rg=pcontext->region;

					region_t r(ByteLength,rg.begin.QuadPart+ByteOffset);
					if( r.in_region(rg))
					{
						*pptr=sm_region::map_region(pcontext->hmap,r,0,pcontext->map_access());

						pcontext->lock();
						hr=S_OK;

					}
					else hr=DISP_E_BADINDEX;


					return hr;
				}
				virtual HRESULT STDMETHODCALLTYPE DecommitRegionPtr(void* ptr){
					HRESULT hr;
					pcontext->unlock();
					sm_region::unmap_ptr(ptr);
					return S_OK;
				}

				virtual HRESULT STDMETHODCALLTYPE GetPtrOfIndex(int64_t index,void ** ppvalue){

					OLE_CHECK_PTR(ppvalue);
					HRESULT hr=S_OK;

					long els=pcontext->get_element_size();

					region_cache_t& rc=pcontext->region_cache;
					char* p=rc.recommit(els,els*index);
					if(p) 
						*ppvalue=p;							
					else hr=HRESULT_FROM_WIN32(GetLastError());
					return hr;

				};


				mm_context_t* pcontext;

			};




			HRESULT set_aggregator_helpers(iaggregator_container* container,pcallback_lib_arguments_t plib)
			{



				HRESULT hr;
				pcontainer_weakref=container;

				smart_ptr_t<iaggregator_helper> helper=
					aggregator_helper_creator_t<mm_region_ptr_t>::New(new	mm_region_ptr_t(this,container));					


				OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mm_region_ptr),helper));
				OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mbv_context_lock),helper));
				OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mbv_context),helper));
				OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mm_cache_buffer_ptr),helper));
				OLE_CHECK_hr(container->SetAggregator(__uuidof(i_mbv_buffer_ptr),helper));

				return hr;
			}


			inline DWORD lock(){
				return context_data.lock();
			}

			inline DWORD try_lock(DWORD tio=0){
				return context_data.try_lock(tio);
			}
			inline BOOL unlock(){
				return context_data.unlock();
			}

			inline DWORD map_access()
			{
				return context_data.map_access();
			}
			inline bool is_ro()
			{
				return context_data.is_ro();
			}


			union{
				struct{
					HANDLE hfile;
					HANDLE hmap;
					HANDLE hmutex;
					uint64_t attrib;
					region_base_t region;
					BSTR infobuffer;
				};		
				mm_context_data_t context_data;
			};


			IUnknown* pcontainer_weakref;

			region_cache_t region_cache;



		};







	}; //mbv_mm

};// ipc_utils
