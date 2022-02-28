#pragma once
#include "ltx_utils/typecase.h"


    template <class F>
	variant_t matrix2buffer(std::vector<std::vector<F>>&v , buffer_facrory_t& factory,bool fmtx=false) {

		HRESULT hr;
		
		 
		 	 
		 
		size_t cols = v.size();
		size_t sz = v.front().size();;
		size_t size=cols*sz;
		const F*data;
		
		ipc_utils::smart_ptr_t<i_mm_region_ptr> buffer;
		variant_t vb;

		hr=(fmtx)? factory.create<F>(cols, sz, buffer, &vb):factory.create<F>(size, buffer,&vb);
		ipc_utils::com_dispcaller_t::exception_helper_t::check_error(hr);
		
		//ipc_utils::com_dispcaller_t::exception_helper_t::check_error(hr = buffer.reset(vb));
		
		size_t byteoffset=0,ByteLength=sz*sizeof(F);

        for(auto k=0;k<v.size();k++)
		{
			
			i_mm_region_ptr::ptr_t<double> x(buffer, byteoffset,ByteLength);
			::memcpy(x, v[k].data(), sz = x.ByteLength);
			byteoffset+=ByteLength;
		}

		return vb;
	};
	

template <class _COMMUNICATOR>
struct ltx_communicator_t :ltx_helper::dispatch_by_name_wrapper_t<ltx_communicator_t< _COMMUNICATOR>, variant_t> {
	typedef  _COMMUNICATOR communicator_t;

	//==============Comm=======
	

	ltx_communicator_t() {};
	inline  void  on_get(DISPID id, const wchar_t* name, com_variant_t& result, arguments_t& arguments, i_marshal_helper_t* helper) {


		HRESULT &hr=arguments.hr;
		auto check_error=[&](bool f, bstr_t message){
			if(!f)
				arguments.raise_error(message,L"ltx_communicator");
		};
		
		if (!id) {
			result = bstr_t(L"MAXIM::ltx_communicator_t:\nreadScalarLogData\nreadDepthData\nwriteLogData\nreadMatrixLogData\n");
			return;
		}

		QString path = (char*)arguments[0].def<bstr_t>();

		if (wcscmp(L"readScalarLogData", name) == 0) {
			
			auto log_data = communicator_t::readScalarLogData(path);
			check_error(log_data.size(),L"readScalarLogData Error");
			
			result = data2buffer(log_data.size(), log_data.data(),factory);
		}
		else if (wcscmp(L"readDepthData", name) == 0) {
			
			auto log_data = communicator_t::readDepthData(path);
			check_error(log_data.size(),L"readDepthData Error");
			
			result = data2buffer(log_data.size(), log_data.data(), factory);
		
		}
		else if (wcscmp(L"writeLogData", name) == 0) {
			ltx::value_t vlog_depth = arguments[1];
			ltx::value_t vlog_data = arguments[2];

			vlog_depth = factory.get_mm_buffer(vlog_depth);
			vlog_data = factory.get_mm_buffer(vlog_data);

			auto log_depth = buffer2vector<double>(vlog_depth);
			check_error(log_depth.size(),L"writeMatrixLogData Error");
			
			auto log_data = buffer2vector<double>(vlog_data);
			check_error(log_data.size(),L"writeMatrixLogData Error");
			
			if (log_depth.size() == log_data.size()) {
				communicator_t::writeLogData(path, log_depth, log_data);
			}
			else {
				size_t sz = log_depth.size();
				size_t count = log_data.size() /sz ;
				std::vector<std::vector<double>> lldata(sz);
				double* pd = (double*)log_data.data();
				for (auto k = 0; k < sz; k++) {
					lldata[k] = std::vector<double>(pd, pd + count);
					pd += count;
				}
				communicator_t::writeLogData(path, log_depth, lldata);
			}

		}
		else if (wcscmp(L"readMatrixLogData", name) == 0) {
			
			bool fmxt = arguments[1].def<bool>(false);

			auto log_data = communicator_t::readMatrixLogData(path);
			check_error(log_data.size(),L"readMatrixLogData Error");
			//result = data2buffer(log_data.size(), log_data.data(), factory);
			result = matrix2buffer(log_data, factory, fmxt);

		}
		else {
			hr = E_NOTIMPL;
		}

	};
	
	
	
	
	static com_variant_t bind(){
		
		HRESULT hr;
		com_variant_t r;
		auto p=new ltx_communicator_t<communicator_t>();
		ipc_utils::com_dispcaller_t::exception_helper_t::check_error_if(p,hr =E_POINTER);
		ipc_utils::com_dispcaller_t::exception_helper_t::check_error(hr = p->wrap(&r));
		return r;
		
		
	}


	buffer_facrory_t  factory;
};
