#pragma once

#include "invoke_context/invoke_context.h"


struct matrix_op_t{
	uint64_t  op;
	void* pin;
	void* pout;
	void** ppout;
	base_context_t * pbuffer_context;
};