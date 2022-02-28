#pragma once
//#include "typecase.h"
#include<stdint.h>
#include<vector>


#include "dispatch_call_helpers.h"

	template <class F>
	struct typecase_t {};


	template <>
	struct typecase_t<uint64_t> {
		static const char* name() {
			return "uint64";
		}
	};

	template <>
	struct typecase_t<int64_t> {
		static const char* name() {
			return "int64";
		}
	};

	template <>
	struct typecase_t<uint32_t> {
		static const char* name() {
			return "uint32";
		}
	};

	template <>
	struct typecase_t<int32_t> {
		static const char* name() {
			return "int32";
		}
	};

	template <>
	struct typecase_t<uint16_t> {
		static const char* name() {
			return "uint16";
		}
	};

	template <>
	struct typecase_t<int16_t> {
		static const char* name() {
			return "int16";
		}
	};

	template <>
	struct typecase_t<uint8_t> {
		static const char* name() {
			return "uint8";
		}
	};

	template <>
	struct typecase_t<int8_t> {
		static const char* name() {
			return "int8";
		}
	};

	template <>
	struct typecase_t<float> {
		static const char* name() {
			return "float";
		}
	};

	template <>
	struct typecase_t<double> {
		static const char* name() {
			return "double";
		}
	};

	struct buffer_facrory_t {

		buffer_facrory_t(bstr_t sc = "ltx.bind:script:", bstr_t t = "require('tensor').createSparseBuffer")
			:jc(sc),
			createBuffer(jc(t)) {};

		template <class F>
		inline variant_t create(size_t size) {
			bstr_t type = typecase_t<F>::name();
			return createBuffer(double(size), type);
		}
		template <class F>
		inline HRESULT create(size_t size, ipc_utils::smart_ptr_t<i_mm_region_ptr>& buffer,VARIANT* pv=NULL) {
			HRESULT hr;
			variant_t vb = create<F>(size);

			if (SUCCEEDED(hr = buffer.reset(vb))&&pv) {

				*pv = vb.Detach();
			}
				


			return hr;
		}


			ipc_utils::com_dispcaller_t jc;
			ipc_utils::com_dispcaller_t createBuffer;
	};

	template <class F>
	variant_t createSparseBuffer(size_t size) {

		bstr_t type = typecase_t<F>::name();

		ipc_utils::com_dispcaller_t jc("ltx.bind:script:");

		ipc_utils::com_dispcaller_t createBuffer = jc("require('tensor').createSparseBuffer");
		return createBuffer(double(size), type);
	}


	template <class F>
	variant_t data2buffer(size_t size, F*data, buffer_facrory_t& factory) {

		HRESULT hr;
		size_t sz = 0;

		variant_t vb = factory.create<F>(size);
		ipc_utils::smart_ptr_t<i_mm_region_ptr> buffer;
		ipc_utils::com_dispcaller_t::exception_helper_t::check_error(hr = buffer.reset(vb));

		{
			i_mm_region_ptr::ptr_t<double> x(buffer, -1);
			::memcpy(x, data, sz = x.ByteLength);
		}		

		return vb;
	}


	template <class F>
	std::vector<F> buffer2vector(variant_t vb) {

		HRESULT hr;
		size_t sz = 0;

		 
		VARTYPE vt;
		long ElementSize = 0;
		int64_t count = 0;
		ipc_utils::smart_ptr_t<i_mm_region_ptr> buffer;
		ipc_utils::com_dispcaller_t::exception_helper_t::check_error(hr = buffer.reset(vb));
		ipc_utils::com_dispcaller_t::exception_helper_t::check_error(hr = buffer->GetElementInfo(&vt, &count, &ElementSize));
		if (ElementSize != sizeof(F))
			ipc_utils::com_dispcaller_t::exception_helper_t::check_error(hr = E_INVALIDARG);



		i_mm_region_ptr::ptr_t<F> x(buffer, -1);

		F* pb = x, *pe = pb + count;

		return std::vector<F>(pb, pe);
	}


	template <class F, class caller_t = ipc_utils::com_dispcaller_t>
	size_t buffer_set(caller_t& setter, bstr_t name,size_t size, F*data, buffer_facrory_t& factory) {

		HRESULT hr;
		size_t sz = 0;

		variant_t vb = factory.create<F>(size);
		ipc_utils::smart_ptr_t<i_mm_region_ptr> buffer;
		caller_t::exception_helper_t::check_error(hr = buffer.reset(vb));

		{
			i_mm_region_ptr::ptr_t<double> x(buffer, -1);
			::memcpy(x, data, sz = x.ByteLength);
		}
		setter(name, vb);

		return sz;
	}

	template <class F,class caller_t= ipc_utils::com_dispcaller_t>
	size_t vector_set(caller_t& setter, bstr_t name,std::vector<F>& v, buffer_facrory_t& factory ) {		

		return buffer_set<F, caller_t>(setter,name,v.size(), v.data(), factory);
	}

	template <class F, class caller_t = ipc_utils::com_dispcaller_t>
	std::vector<F> vector_get(caller_t& getter, bstr_t name) {

		HRESULT hr;
		size_t sz = 0;

		variant_t vb= getter(name );
		VARTYPE vt;
		long ElementSize=0;
		int64_t count=0;
		ipc_utils::smart_ptr_t<i_mm_region_ptr> buffer;
		caller_t::exception_helper_t::check_error(hr = buffer.reset(vb));
		caller_t::exception_helper_t::check_error(hr = buffer->GetElementInfo(&vt,&count,&ElementSize));
		if (ElementSize != sizeof(F))
			caller_t::exception_helper_t::check_error(hr = E_INVALIDARG);

		

		i_mm_region_ptr::ptr_t<F> x(buffer, -1);

		F* pb = x, *pe = pb + count;	

		return std::vector<F>(pb,pe);
	}



;
