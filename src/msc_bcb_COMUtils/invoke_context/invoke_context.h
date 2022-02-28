#pragma once
//invoke_context.h



//typedef long long int64_t;
//typedef int int32_t;

/*
#ifdef _WIN32
#include <windows.h>
#ifndef uuid_t
#define  uuid_t GUID
#endif // !uuid_t
#else
typedef struct _GUID {
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];
} uuid_t;
#endif
*/

#include "context_holder.h"


typedef unsigned char byte_t;

#pragma pack(push)
#pragma pack(8)

const uuid_t i_ref_context_uuid = { 0x7b7ed1f1, 0x5b59, 0x4a90, 0x87, 0x73, 0x31, 0x44, 0x00, 0x9f, 0x3e, 0xd0 };

struct base_context_t {
	typedef long(*invoke_t)(void* context, int64_t icmd, void* params);
	typedef long(*invoke_parallel_t)(void* context, int64_t icmd, void* params, int64_t begin, int64_t end);
	typedef long(*invoke0_t)(void* context);
	void* weak_ref_handle;
	uuid_t uuid;
	
	union {
		struct { invoke_t pinvoke; };
		struct { invoke0_t pinvoke0; };
		struct { invoke_parallel_t pinvoke_parallel; };
	};
	void* context;
};

typedef base_context_t *pbase_context_t;


struct run_context_t :base_context_t {
	void(*pdestroy)(void* context);
};

struct invoke_context_t;

struct context_list_t {

	invoke_context_t** pp;
	int64_t count, rep,once;
};

struct mem_alloc_data_t {
	void* p;
	int64_t size;
	int64_t pos;
	base_context_t* memmng;
};

typedef mem_alloc_data_t mem_manager_data_t;


struct invoke_context_t:base_context_t{	
		
	union {
		struct { byte_t ccmd[16];};
		struct { wchar_t wcmd[8]; };
		struct { int64_t icmd; };
		struct { void* pcmd; };
	};
	union {
		struct { void* params; };
		struct { int64_t iparams; };
		struct { double fvalue; };
		struct { context_list_t* pclist; };
		
	};

	int64_t status;

	void* run_context;

	union {
		struct { int64_t iparams_ex[6]; };
		struct { double dparams_ex[6]; };
		struct { double d2params_ex[3][2]; };
		struct { char cbuffer[128 - 80]; };
	};
};



/*
struct sp_maitrix_params_t {
	char type[8];
	int64_t size;
	void* data;
};
*/
#pragma pack(pop)

#include <stdlib.h>
#ifndef _WIN32
typedef long HRESULT
#endif
#ifdef __cplusplus
#ifdef _WIN32
#include <windows.h>
#include <string>

#define CHECK_VOID(f) { if(!(f)) { hr=HRESULT_FROM_WIN32(GetLastError());return; } }
#define CHECK_VOID_PROC_ADDRESS(h,a)  CHECK_VOID(SET_PROC_ADDRESS(h,a))


inline long _call_context(base_context_t* p, int64_t cmd,const void* params) {
	if (p&&p->pinvoke) {
		return p->pinvoke(p->context, cmd, (void*)params);
	}
	return -1;
}

template <class T>
long call_context_ref(base_context_t* p, int64_t cmd, const T& params) {
	return _call_context(p, cmd, &params);
}





template <class CONTEXT = base_context_t>
struct context_ptr_t {
	typedef CONTEXT context_t;

	inline static i_unknown* get_i(const base_context_t* c) {
		if (c)
			return (i_unknown*)(c->weak_ref_handle);
		else
			return nullptr;
	}

	inline static i_unknown* get_i(const void* c) {
		return (i_unknown*)c;
	}


	inline static long _call(const base_context_t* c,int64_t icmd=0,const void* params=NULL) {
		if (c&&(c->pinvoke))
			return c->pinvoke(c->context,icmd,(void*)params);
		else
			return -1;
	}

	inline static long _call(const void* c, int64_t icmd = 0, const void* params = NULL) {
		return -1;
	}

	

	inline static long _addref(context_t *n) {
		i_unknown* in = get_i(n);
		return (in) ? in->AddRef() : -1;
	}

	inline static long _release(context_t *n) {
		i_unknown* in = get_i(n);
		return (in) ? in->Release() : -1;
	}


	inline  long operator()(int64_t icmd = 0, const void* params = NULL){
		return _call(p, icmd, params);
	}

	inline  long call(int64_t icmd = 0, const void* params = NULL) {
		return _call(p, icmd, params);
	}

	context_ptr_t() :p(NULL) {};

	context_ptr_t(context_t *n ):p(n) {
		_addref(n);
	}

	context_ptr_t(const context_ptr_t& sp) :p(NULL)	{
		reset(sp.p);
	}

	~context_ptr_t() {
		_release(p);
	}
	inline context_t* attach(context_t *n) {
		if (p != n) {
			reset();
			p = n;
		}
		return p;
	}
	inline context_t* dettach() {
		context_t* t = p;		
		p = NULL;
		return t;
	};

	context_ptr_t& operator =(const context_ptr_t& sp)
	{
		return reset(sp.p);
	}


	inline long link(void* h) {
		
		if (h) {			

			i_unknown *ip = get_i(p);			

			if (ip) {

				long err;
				i_link_context* ic;

				if (err = ip->QueryInterface(i_link_context_uuid, (void**)(&(ic == 0))))
					return err;
				err = ic->link((i_unknown *)h);
				ic->Release();
				return err;
			}
		}
		return -1;
	}
	inline operator context_t* () {
		return p;
	}

	inline long link(const base_context_t* h) {

		return link((void*)get_i(h));		
	}

	inline long unlink(void* h) {

		if (h) {

			i_unknown *ip = get_i(p);

			if (ip) {

				long err;
				i_link_context* ic;

				if (err = ip->QueryInterface(i_link_context_uuid, (void**)(&(ic == 0))))
					return err;
				err = ic->unlink((i_unknown *)h);
				ic->Release();
				return err;
			}
		}
		return -1;
	}

	inline long unlink(const base_context_t* h) {

		return unlink((void*)get_i(h));
	}
	inline void clear_links() {

		i_unknown* i = get_i(p);
		i_link_context* ic;
		if (i && (!i->QueryInterface(i_link_context_uuid, (void**)(&(ic == 0))))) {
			ic->clear_links();
			ic->Release();
		}
	}

	inline context_ptr_t& reset(context_t *n=0) {
		if (n != p) {
			_addref(n);			 
			_release(inner_utils::make_detach(p,n));
		}

		return *this;
	}
	
	context_t *p;
};

struct invoke_context_holder_t {
	HRESULT(*create_context)(void* context, void(*ondestroy)(void* p), base_context_t::invoke_t pinvoke, base_context_t** pp_context);
	HRESULT(*link_context)(base_context_t* context_parent, base_context_t* context_child);
	HRESULT(*unlink_context)(base_context_t* context_parent, base_context_t* context_child);	
	HRESULT (*link_handle)(base_context_t* context_parent, void* hchild);
	HRESULT(*unlink_handle)(base_context_t* context_parent, void* hchild);
	HRESULT(*clear_context_links)(base_context_t* context);


	//HRESULT(*create_invoke_context_batch)(int count, invoke_context_t** context_list, bool asyn, int rep, invoke_context_t** pp_invoke_context);
	HRESULT(*create_invoke_context_batch)(base_context_t* parallel_context, int64_t count, invoke_context_t** context_list, int64_t asyn, int64_t rep, int64_t once, invoke_context_t** pp_invoke_context);
	HRESULT(*create_invoke_context)(base_context_t* context, invoke_context_t** pp_invoke_context);
	HRESULT(*create_invoke_context_ex)(base_context_t* context, int64_t cmd, void* params, invoke_context_t** pp_invoke_context);
	//HRESULT(*invoke_context)(invoke_context_t* context);
	//HRESULT(*invoke_context_batch)(int count, invoke_context_t** pp, bool asyn);

	HRESULT(*call_invoke_context_batch)(base_context_t* parallel_context, int64_t count, int64_t rep, invoke_context_t** ppcontexts);

	long(*release_context)(base_context_t* context);
	long(*addref_context)(base_context_t* context);



	HRESULT(*call_context)(base_context_t* context, int64_t icmd, void* params);
	HRESULT(*call_context_tictoc)(base_context_t* context, int64_t icmd, void* params,double* ptictoc);
	

	HRESULT(*create_buffer_context)(base_context_t* pmem_manager, int64_t cb, base_context_t** pp_context);

	HRESULT(*create_timer_context)(base_context_t** pp_context);
	HRESULT(*create_ref_context)(base_context_t** context, base_context_t** ref_context);
	HRESULT(*create_mem_op_context)(base_context_t** ppcontexts);
	HRESULT(*unwrap_ref_context)(base_context_t* ref_context, base_context_t** pp_context);
	HRESULT(*create_run_context)(run_context_t** ppcontext);

	HRESULT(*get_context_holder)(void** context_holder);
	context_holder_t* lowlevel;
};

struct invoke_context_helper_t:invoke_context_holder_t {

	struct dll_dir_add_t {
		DLL_DIRECTORY_COOKIE cookie;
		dll_dir_add_t(wchar_t* n):cookie(0){
			if (n) {
				std::wstring s = n;
				s += L"/../";
				::SetDefaultDllDirectories(0x00001e00);
				cookie = ::AddDllDirectory(s.c_str());
			}
		};

		~dll_dir_add_t(){
			if (cookie)
				::RemoveDllDirectory(cookie);
		};
	};

	

	//HRESULT(*tls_clear)();
	//HRESULT(*tls_buffer)(int64_t size, void** pp_buff);


	HRESULT hr;
	

	inline 	static void* set_dll_proc(HMODULE hlib, void** fp, char* name) {
		return (hlib) ? (*fp = (void*)GetProcAddress(hlib, name)) : NULL;
	}


	inline operator HRESULT() {
		return hr;
	}

	inline operator bool() {
		return hr==S_OK;
	}
	
	invoke_context_helper_t(const wchar_t* env = L"invoke_context", const wchar_t* lib = NULL)
		:invoke_context_holder_t(), hr(S_OK) {
		//
		/*
		HMODULE hlib;
		wchar_t* n = (wchar_t*)lib;
		wchar_t buf[2048];

		if (!n) {
			//n = _wgetenv(L"invoke_context_wrapper");
			size_t cb;
			buf[0] = 0;
			//cb = GetEnvironmentVariableW(L"parallel_wrapper_lib", buf, 2048);
			_wgetenv_s(&cb, buf, 2048, L"invoke_context");
			 //cb = GetEnvironmentVariableW(L"invoke_context", buf, 2048);
			n = buf;
		}
		//*/

		//	dll_dir_add_t dda(n);		

		///*
		libloader_t hlib(env, lib);

		if (hr = hlib)
			return;
		//	*/
		init(hlib);
	}

	invoke_context_helper_t(HMODULE hlib):invoke_context_holder_t(), hr(E_FAIL) {
		init(hlib);
	}

	void init(HMODULE hlib){

		CHECK_VOID_PROC_ADDRESS(hlib, create_context);		
		CHECK_VOID_PROC_ADDRESS(hlib, unlink_context);
		CHECK_VOID_PROC_ADDRESS(hlib, link_context);

		CHECK_VOID_PROC_ADDRESS(hlib, link_handle);
		CHECK_VOID_PROC_ADDRESS(hlib, unlink_handle);

		CHECK_VOID_PROC_ADDRESS(hlib, clear_context_links);

		CHECK_VOID_PROC_ADDRESS(hlib, create_invoke_context_batch);
		CHECK_VOID_PROC_ADDRESS(hlib, create_invoke_context);
		CHECK_VOID_PROC_ADDRESS(hlib, create_invoke_context_ex);
		//CHECK_VOID_PROC_ADDRESS(hlib, create_invoke_context_indirect);		
		//CHECK_VOID_PROC_ADDRESS(hlib, invoke_context);
		//CHECK_VOID_PROC_ADDRESS(hlib, invoke_context_batch);
		//CHECK_VOID_PROC_ADDRESS(hlib, create_lin_comb);
		
		CHECK_VOID_PROC_ADDRESS(hlib, addref_context);
		CHECK_VOID_PROC_ADDRESS(hlib, release_context);
		//CHECK_VOID_PROC_ADDRESS(hlib, create_parallel_group);
		CHECK_VOID_PROC_ADDRESS(hlib, call_context);
		CHECK_VOID_PROC_ADDRESS(hlib, call_invoke_context_batch);
		
		CHECK_VOID_PROC_ADDRESS(hlib, create_buffer_context);
		CHECK_VOID_PROC_ADDRESS(hlib, create_ref_context);
		CHECK_VOID_PROC_ADDRESS(hlib, create_timer_context);
		CHECK_VOID_PROC_ADDRESS(hlib, create_mem_op_context);
		CHECK_VOID_PROC_ADDRESS(hlib,create_run_context);
		CHECK_VOID_PROC_ADDRESS(hlib, get_context_holder);	
		
		hr=get_context_holder((void**)&lowlevel);
		//CHECK_VOID_PROC_ADDRESS(hlib, tls_clear);		
		//CHECK_VOID_PROC_ADDRESS(hlib, tls_buffer);			
		
	}

	template<class Params>
	inline static int fast_call_context(void* p,int64_t icmd,const Params& params) {

		base_context_t* c = (base_context_t*)p;

		if (c->pinvoke) 
			return c->pinvoke(c->context, icmd, (void*)params);
		else return -1;
		
	}
};


struct invoke_context_helper2_t  {

	invoke_context_holder_t* pich;
	HRESULT hr;
	HMODULE hlib;

	struct dll_dir_add_t {
		DLL_DIRECTORY_COOKIE cookie;
		dll_dir_add_t(wchar_t* n) :cookie(0) {
			if (n) {
				std::wstring s = n;
				s += L"/../";
				::SetDefaultDllDirectories(0x00001e00);
				cookie = ::AddDllDirectory(s.c_str());
			}
		};

		~dll_dir_add_t() {
			if (cookie)
				::RemoveDllDirectory(cookie);
		};
	};
	


	inline 	static void* set_dll_proc(HMODULE hlib, void** fp, char* name) {
		return (hlib) ? (*fp = (void*)GetProcAddress(hlib, name)) : NULL;
	}


	inline operator HRESULT() {
		return hr;
	}

	inline operator bool() {
		return hr == S_OK;
	}

	//invoke_context_holder_t* invoke_context_holder;

	inline invoke_context_holder_t* operator->()
	{
		return pich;
	}

	invoke_context_helper2_t(const wchar_t* env = L"invoke_context", const wchar_t* lib = NULL):
		pich(nullptr)
	{
		

		libloader_t ll(env, lib);

		if (hr = ll)
			return;
		hlib = ll;
		//	*/
		init();
	}

	

	void init() {

		HRESULT(*geti)(invoke_context_holder_t**);
		if (SUCCEEDED(hr)&& set_dll_proc(hlib, (void**)&geti,"get_invoke_context_holder")) {
			hr=geti(&pich);
		}
		
	}

	template<class Params>
	inline static int fast_call_context(void* p, int64_t icmd, const Params& params) {

		base_context_t* c = (base_context_t*)p;

		if (c->pinvoke)
			return c->pinvoke(c->context, icmd, (void*)params);
		else return -1;

	}
};





template <class T>
struct buffer_t {
	T *p;
	size_t size;

	buffer_t() : p(NULL), size(0) {};

	buffer_t(size_t s): p(NULL),size(s){
		p = s ? (T*)::malloc(sizeof(T)*s) : (T*)NULL;
	}
	buffer_t(size_t s, T *ps) : p(NULL), size(s) {

		p = s ? (T*)::malloc(sizeof(T)*s) : (T*)NULL;
		if (s&&ps) {
			memcpy(p, ps, sizeof(T)*s);
		}
	}

	inline void zero() {
		if (p) {
			memset(p,0, sizeof(T)*size);
		}
	}
	inline T * resize(size_t s) {
		if (p)
		{
			if(size!=s)
				p = (T*)::realloc(p, size = s);		
		}
		else 
			p = (T*)::malloc(sizeof(T)*(size = s));
		return p;
	}
	inline void free() {
		::free(p);
		p = 0;
		size = 0;
	}
	inline operator T*() {
		return p;
	}

	inline T* pack(size_t count, size_t sz, T** pp) {

		T* pc;				
		if (pc=resize(count*sz)) {

			size_t sb = sz * sizeof(T);
			if(pp)
				for (size_t k = 0; k < count; k++, pc += sb) {
					::memcpy(pc, pp[k], sb);
				}

		}
		return p;
	}

	inline T* unpack(size_t count, size_t sz, T** pp) {
		T* pc=p;
		size_t sc = count*sz;
		if (sc > size)
			return 0;
		if (pc) {
			size_t sb = sz * sizeof(T);

			for (size_t k = 0; k < count; k++, pc += sb)
				::memcpy(pp[k],pc, sb);

		}
		return p;		
	}



	~buffer_t() {
		::free(p);
	}
};


template<class T>
struct buffer_context_t {

	
	mem_alloc_data_t* mad;


	long err;
	buffer_context_t(base_context_t * _bc):bc(_bc) {		

		err=bc.call('p', &(mad=0));
	}
	buffer_context_t(size_t sz, base_context_t * _bc) :buffer_context_t(_bc) {
		if (!err)
			resize(sz);
	}

	inline T* ptr() {
		return (T*)(mad->p);
	}

	inline T* zero() {

		bc.call('z');
		return ptr();
	}

	inline T* resize(size_t sz) {

		size_t s = sz * sizeof(T);

		if(err = bc.call('r', &s))
			return nullptr;

		return ptr();
	}

	inline size_t count() {
		return mad->size / sizeof(T);
	}

	inline size_t sizeb() {
		return mad->size;
	}

	inline operator T*(){
		return ptr();
	}

	inline T* pack_unpack(int op,size_t count, size_t sz, T** pp) {
		buffer_flat_params_t bfp = { op,sz,count,(char**)pp };
		if (err = bc.call('s', &bfp))
			return nullptr;
		return ptr();
	}


	
	inline  T* load_cast(const T* ps, size_t _count) {
		T* pd = resize(_count);

		if (pd)			 
			memcpy(pd, ps, sizeb());		
		return pd;
	}

	template <class O>
	inline  T* load_cast(const O* ps,size_t _count) {
		T* pd = resize(_count);
		if (pd) 
			for(auto m=0;m<_count;m++){
				pd[m] = T(ps[m]);
			}

		return pd;
	}

	template <class Buffer>
	inline  T* load_cast(const Buffer& b) {
		return load_cast(b.ptr(),b.count());
	}

	inline T* pack( size_t count, size_t sz, T** pp) {
		return pack_unpack(0, size_t count, size_t sz, T** pp);
	}

	inline T* unpack(size_t count, size_t sz, T** pp) {
		return pack_unpack(1, size_t count, size_t sz, T** pp);
	}

	inline void free() {
		bc.reset();
	}

	context_ptr_t<base_context_t> bc;
};
//


template<class T>
struct call_context_helper_t:base_context_t{

	static void ondestroy(void*p) {

		delete ((T*)p);
	}

	static long s_invoke(void* context, int64_t icmd, void* params) {

		return ((T*)context)->invoke(icmd, params);
	}

	long invoke(int64_t icmd, void* params) {
		return -1;
	};

	call_context_helper_t() 
		:base_context_t() {
		pinvoke = &s_invoke;
		context = (T*)this;
   }

};

template<class T>
struct call_context_helper_pp_t :base_context_t {

	static void ondestroy(void*p) {

		delete ((T*)p);
	}

	static long s_invoke_pp(void* context, int64_t icmd, void* params, int64_t b, int64_t e) {

		return ((T*)context)->invoke(icmd, params,b,e);
	}

	long invoke(int64_t icmd, void* params, int64_t b, int64_t e) {
		return -1;
	};

	call_context_helper_pp_t()
		:base_context_t() {
		pinvoke_parallel = &s_invoke_pp;
		context = (T*)this;
	}

};



#include <complex>
typedef std::complex<double> complex_t;

struct buffer_flat_params_t {
	int64_t op;
	int64_t N,count;
	char** pp;
};

struct lc_base_t {
	int64_t grain;
};

struct lc_t:lc_base_t {
	char type[16];
	int64_t N,D;
	complex_t* c;
	complex_t** xx;
	union {
		struct { complex_t* cout; };
		struct { double* rout; };
	};

};

struct lc_1_form_t :lc_base_t {
	char type[16];
	int64_t N, offset, offset_inc;
	double* f;
	double* x;
	double* rout;
};

struct lc_copy_t :lc_base_t {

	int64_t byteLen;	
	void* dest;
	void* src;
};

/*
struct base_context_uuid_t :base_context_t {
	GUID uuid;
};
*/
struct base_context_size_t :base_context_t {

	int64_t index_size;

	inline static  int64_t get_index_size(void* p) {

		return (p) ? ((base_context_size_t*)p)->index_size : 0;
	}

};


template <class T>
struct context_impl_T :base_context_size_t {

	//void* weak_ref_handle;



	static void ondestroy(void*p) {

		delete ((T*)p);
	}

	static long s_invoke(void* context, int64_t icmd, void* params) {

		return ((T*)context)->invoke(icmd, params);
	}

	context_impl_T() : base_context_size_t() {
		pinvoke = &s_invoke;
		context = (T*)this;
	};

	template <class context_holder_T>
	void* create_handle(context_holder_T* pch) {
		if (pch) {
			weak_ref_handle&&pch->release(weak_ref_handle);
			pch->create_handle((T*)this, &ondestroy, &(weak_ref_handle = 0));
		}
		return weak_ref_handle;
	}

	long invoke(int64_t icmd, void* params) {
		return -1;
	}

	template <class context_holder_T,class O>
	HRESULT link(context_holder_T* pch, O* o) {
		return (pch&&o) ? pch->link(weak_ref_handle, o->weak_ref_handle) : -1;
	}
	template <class context_holder_T, class O>
	HRESULT relink(context_holder_T* pch, O* o) {
		HRESULT hr;
		if (hr = link(pch, o))
			return hr;
		if (o->weak_ref_handle)
			pch->release(o->weak_ref_handle);
		return 0;
	}

	template <class context_holder_T, class O>
	inline static O* reset_context(context_holder_T* pch, O*& o, O* on=NULL) {

		if (o != on) {


			if (pch) {
				O* t = o;
				on && (pch->addref((o = on)->weak_ref_handle));
				t && (pch->release(t->weak_ref_handle));
			}
			else {
				o = on;
			}
		}

		return o;
	}

};


#endif
#endif