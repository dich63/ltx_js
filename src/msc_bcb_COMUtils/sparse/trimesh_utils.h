#pragma once
//#include "sparse/trimesh_utils.h"
#include "invoke_context/invoke_context.h"

namespace trimesh_utils {
#pragma pack(push)
#pragma pack(8)

	typedef long long int64_t;
	typedef int int32_t;
	
	struct tri_assembly_data_t :lc_base_t {

		int64_t index_base, index_size;
		int64_t D, Nc;
		int64_t nvxs, ntrs;

		void* trs;
		void* vxs;
		void* vxs_mask;
		int64_t  Nmx;
		void** mx_data;

		void* user_data;
		int32_t(*user_proc)(void* user_data, int64_t ntrs_begin, int64_t ntrs_end, tri_assembly_data_t* data);

		void** mx_out;
		int64_t work;

	};
#pragma pack(pop)
	struct tri_assembly_impl_t :tri_assembly_data_t {

		struct pp_error_t {
			int32_t err;
			pp_error_t(int32_t _err) :err(_err) {}
		};

		inline void make(int64_t ntrs_begin, int64_t ntrs_end) {
			pp_error_t err = {0};
			if (err.err = user_proc(user_data, ntrs_begin, ntrs_end, this))
				throw err;
		}
	};

}; //end namespase