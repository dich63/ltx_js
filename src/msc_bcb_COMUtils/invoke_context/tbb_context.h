#pragma once
#include "context_holder.h"
#include "invoke_context.h"


struct tbb_context_holder_t {
	HRESULT(*wrap_parallel_context)(base_context_t* _pcontext, int64_t _count, int64_t _grain, base_context_t** pp_context);
	HRESULT(*call_context_parallel)(base_context_t* _pcontext, int64_t icmd, void* params, int64_t _count, int64_t _grain);
	HRESULT(*create_parallel_batch_context)(base_context_t** pp_context);
	HRESULT(*create_allocator_context)(base_context_t** pp_context);
	HRESULT(*create_allocator_context_ex)(int64_t ftype, base_context_t** pp_context);
	HRESULT(*create_parallel_group_ex)(int64_t mode, base_context_t** pp_context);
	HRESULT(*create_parallel_group)(base_context_t** pp_context);	

};
