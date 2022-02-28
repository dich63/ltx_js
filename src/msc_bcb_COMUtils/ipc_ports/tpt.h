#pragma once

	struct job_packet_t;

	typedef long long integer_t;

	typedef	struct job_base_t {
		void* weak_ref_handle;
	};

	typedef	struct job_packets_t:job_base_t {
		typedef int (*run_pool_proc_t)(job_packets_t* this_packets);
		typedef long(*set_affinity_proc_t)(job_packets_t* pt, int l, int* nn, KAFFINITY Mask, WORD Group);
		

		run_pool_proc_t run_pool;
		set_affinity_proc_t set_affinity;
		void* pool_context;
		integer_t thread_max;
		integer_t count;		
		job_packet_t** packets;

	} job_packets_t, pjob_packets_t;

	
	

	typedef	struct job_packet_t :job_base_t {

		//int start_job_packets_s(const void* p, const void* pack, int np)
		//typedef int (run_child_packet_proc_t)(job_packets_t* packets);

		typedef int(*packet_proc_t)(void* context, void* params, job_packet_t* packet);
		typedef int(*run_pool_packets_proc_t)( job_packet_t* packet);

		//void* weak_ref_handle;

		packet_proc_t proc;
		run_pool_packets_proc_t child_run;

		//on_destroy_t  on_destroy;		
		void *context;
		void* params;	
		integer_t  n, ngroup;
		job_packets_t*  pchild_packets;
        job_packets_t child_packets;		
		void* inner_ptr; 
	} job_packet_t, *pjob_packet_t;

#ifdef __cplusplus
#ifdef _WIN32
	#include <windows.h>
    #include <wchar.h>
    #include <vector>

#define CHECK_VOID(f) { if(!(f)) { hr=HRESULT_FROM_WIN32(GetLastError());return; } }
#define SET_PROC_ADDRESS_NAME(h,a,n)  tpt_helper_t::set_dll_proc(h,(void**)&a,n)
#define SET_PROC_ADDRESS(h,a)  SET_PROC_ADDRESS_NAME(h,a,#a)
#define CHECK_VOID_PROC_ADDRESS(h,a)  CHECK_VOID(SET_PROC_ADDRESS(h,a))


	struct tpt_helper_t {

		HRESULT(*create_pool_tree_handle)(int* pnjobtree, int deep, void** pp_pool_handle);
		long(*addref_handle)(void* handle);
		long(*release_handle)(void* handle);

		HRESULT(*release_context)(void* pcontext);

		HRESULT(*link_handle)(void* handle, void* child_handle);
		HRESULT(*clear_links)(void* handle);


		HRESULT(*get_handle_context)(void* handle, void** pcontext);
		HRESULT(*create_handle)(void* context, void(*ondestroy)(void* context), void** phandle);
		
		HRESULT hr;

		

		inline 	static void* set_dll_proc(HMODULE hlib, void** fp, char* name) {
			return (hlib) ? (*fp = (void*)GetProcAddress(hlib, name)) : NULL;
		}

		inline operator HRESULT() {
			return hr;
		}
		
		inline tpt_helper_t* operator->() {
			return this;
		}
		tpt_helper_t(const wchar_t* lib = NULL)
			:hr(S_OK) {
			HRESULT(*get_context_holder)(void** pp_ch);
			HMODULE hlib;
			std::vector<wchar_t> buf;
			void* tt;
			wchar_t* n = (wchar_t*)lib;
			if (!n)
			{
				int sz;
				if (sz = GetEnvironmentVariableW(L"tpt_lib", n, 0)){
					buf.resize(sz);
					n = &buf[0];
				}
				//n = _wgetenv(L"tpt_lib");
				GetEnvironmentVariableW(L"tpt_lib", n, sz);
			}
			CHECK_VOID(hlib = LoadLibraryW(n));
			CHECK_VOID_PROC_ADDRESS(hlib, create_pool_tree_handle);
			CHECK_VOID_PROC_ADDRESS(hlib, addref_handle);
			CHECK_VOID_PROC_ADDRESS(hlib, release_handle);
			CHECK_VOID_PROC_ADDRESS(hlib, release_context);			
			CHECK_VOID_PROC_ADDRESS(hlib, link_handle);
			CHECK_VOID_PROC_ADDRESS(hlib, clear_links);
			CHECK_VOID_PROC_ADDRESS(hlib, get_handle_context);
			CHECK_VOID_PROC_ADDRESS(hlib, create_handle);
			CHECK_VOID_PROC_ADDRESS(hlib, get_context_holder);
			hr = get_context_holder(&(tt = 0));
			
		}

	};

template <class T,class Params>
struct packet_T:job_packet_t {

	typedef Params params_t;
	typedef job_packet_t packet_t;
	int make(const params_t* params) {
		return 0;
	}

	static int packet_proc_s(void* context, void* params, packet_t* packet) {

		T* t = static_cast<T*>(packet->context);
		return t->make((params_t*)packet->params);
	};

	static int on_destroy_s(void*p) {
		delete static_cast<T*>(p);
	}

	inline int exec(const params_t & p) {
		params = (void*)&p;
		return packet_proc_s(context, params, this);
	}

	//
	/*
	static packet_T* createInstance(tpt_helper_t& tpt) :
	{
		HRESULT hr;
		T* p = new T();
		if (hr = tpt->create_handle((void*)p, &on_destroy_s,&p->weak_ref_handle))
		{
			delete p;
			return NULL;
		}

	}

	long release(tpt_helper_t& tpt) {

		return tpt->release_context(this);
	}
	//*/

   protected:
	   packet_T() :job_packet_t() {
		context = static_cast<T*>(this);
		proc = &packet_proc_s;
		//on_destroy = &on_destroy_s;		
	};

};


#endif
#endif