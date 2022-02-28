#pragma once

#include "static_constructors_0.h"
#include "invoke_context/invoke_context.h"
#include <list>
#include <mutex>

//std::lock_guard<std::mutex> lock(mutex);
namespace tls_utils {

	struct tls_buffer_t;
	typedef  std::list<tls_buffer_t> buffer_list_t;

	struct tls_buffer_t {
		buffer_t<char> buffer;
		size_t stack;
		buffer_list_t::iterator it;

	};


	template <int _Vers = 0>
	struct tls_buffer_mng_t {





		inline tls_buffer_t*& tls_buffer() {
			static thread_local tls_buffer_t* p_tls_buffer = nullptr;
			return p_tls_buffer;
		}


		tls_buffer_t* get_tls_buffer() {
			if (!tls_buffer()) {
				std::lock_guard<std::mutex> lock(mutex);
				buffer_list.push_front(tls_buffer_t());
				auto it = buffer_list.begin();
				tls_buffer_t &b = *it;
				b.it = it;
				b.stack = 0;
				tls_buffer() = &b;
			}
			return tls_buffer();
		}

		char* _get_buffer(size_t sz) {
			buffer_t<char>& b = get_tls_buffer()->buffer;
			if (b.size < sz)
				b.resize(sz);
			return b.p;
		}

		void _on_thread_detach() {
			if (tls_buffer())
			{
				std::lock_guard<std::mutex> lock(mutex);
				buffer_list.erase(tls_buffer()->it);
			}
		}
		int64_t _clear() {
			std::lock_guard<std::mutex> lock(mutex);
			int64_t sz = 0;// buffer_list.size();
			for (auto i = buffer_list.begin(); i != buffer_list.end(); ++i) {
				sz += i->buffer.size;
				i->stack = 0;
				i->buffer.free();
			}
			return sz;
		}

		buffer_list_t buffer_list;
		std::mutex mutex;

		inline	static tls_buffer_mng_t<_Vers> & getInstance() {

			return class_initializer_T<tls_buffer_mng_t<_Vers> >::get();
		}

		static void on_thread_detach() {
			getInstance()._on_thread_detach();
		}
		static int64_t clear() {
			return getInstance()._clear();
		}

		static char* get_buffer(int64_t sz) {
			return getInstance()._get_buffer(sz);
		}

	};

};// tls_utils;