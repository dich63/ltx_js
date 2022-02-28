#pragma once 

#include <list>
#include <mutex>

#include <algorithm>
#include <vector>

#include <chrono>


namespace utils {


	template <class C>
	struct b10_t {
		C r[10];
	};


	template <class C, int N>
	struct bN_t {
		C r[N];
		inline static bN_t* ptr(void* p) {
			return (bN_t *)p;
		}
	};

	template <int N>
	struct pN_t {
		
		template <class C>
		inline static bN_t<C,N>* ptr(C* p) {
			bN_t<C, N> tmp;
			return (bN_t<C, N>*)p;
		}
		template <class C>
		inline static bN_t<C, N>** pptr(C** pp) {
			bN_t<C, N> tmp;
			return (bN_t<C, N>**)pp;
		}

		template <class C>
		inline static bN_t<C, N>& ref(C* p) {
			return *ptr(p);
		}

	};

	struct b5_t {
		double r[10];
	};


	struct p_t {

		template <class F>
		p_t(F* _p) :p((void*)_p) {}

		template <class F>
		operator F** () {
			return (F**)&p;
		}

		void* p;
	};


	template <int _V = 0>
	struct stopwatch_t {
		std::chrono::high_resolution_clock::time_point t;

		stopwatch_t() {
			if (!_V) start();
		};

		inline void start() {
			t = std::chrono::high_resolution_clock::now();
		}
		inline double sec() {

			auto tn = std::chrono::high_resolution_clock::now();
			auto time_span = tn - t;
			std::chrono::duration<double> dt = time_span;
			return dt.count();
		}
		inline double Sec() { return sec(); };
		inline void Start() { start(); };
	};


	template <class T>
	bool check_reset(T v, T& old) {

		if( v== old ) {			
			return false;
		}
		else {			
			old = v;
			return true;
		}

	};

	template <class T, class T2>
	T reattach(T& dst, T2 n)
	{
		T t = dst;
		dst = (T)n;
		return t;
	};

	template <class T>
	T reattach(T& dst)
	{
		T t = dst;
		dst = T();
		return t;
	};


	template <class T>
	T* reattach(T*& dst)
	{
		T* t = dst;
		dst = nullptr;
		return t;
	};


	inline bool reattach(bool& dst)
	{
		bool t = dst;
		dst = false;
		return t;
	}



	template <class _ITEM>
	struct c_array_t {
		typedef _ITEM item_t;

		c_array_t(size_t _m = 0) :m(_m), items(_m) {

		}
		template <class Factory>
		inline	size_t init(size_t _m, Factory f) {

			//items.resize(m = _m);
			reset(_m);
			for (auto i = 0; i < m; ++i) {
				items[i] = f(i);
			}
			return m;
		}

		inline void reset(size_t _m = 0) {

			for (auto i = 0; i < m; ++i) {
				delete reattach(items[i]);
			}


			items.resize(m = _m);
		}

		~c_array_t() {
			reset();
		}

		inline item_t& operator[](size_t n) {
			return *(items[n]);
		}
		size_t m;
		std::vector<item_t*> items;
	};




	struct fake_mutex_t {
		inline void lock() {};
		inline void unlock() {};
	};


	struct mem_pool_mng_def_t {
		inline static void* malloc(size_t n_element, size_t element_size) {
			return  ::malloc(n_element*element_size);
		}
		inline static void free(void* p) {
			::free(p);
		}
	};


	template <class Mutex = fake_mutex_t, class mem_mng_t = mem_pool_mng_def_t>
	struct mem_pool_t {
		typedef Mutex mutex_t;
		typedef typename std::lock_guard < mutex_t> locker_t;

		struct holder_t {

			holder_t() :p(nullptr) {};
			~holder_t() {
				if (p)
					mem_mng_t::free(p);
			};

			void* p;
		};

		~mem_pool_t() {
			clear();
		}

		inline void clear() {
			const locker_t lock(mutex_);
			pool_.clear();
		}

		template <class F>
		F* alloc(size_t c, F** pp = (F**)nullptr) {



			const locker_t lock(mutex_);

			pool_.push_back(holder_t());
			F* t = (F*)(pool_.back().p = mem_mng_t::malloc(c, sizeof(F)));
			if (pp)
				*pp = t;
			return t;
		}


		



		template <class F>
		std::tuple<F*, F* > alloc2(size_t c, F** pp = (F**)nullptr) {

			const locker_t lock(mutex_);

			pool_.push_back(holder_t());
			F* t = (F*)(pool_.back().p = mem_mng_t::malloc(c, sizeof(F)));
			if (pp)
				*pp = t;
			return std::make_tuple(t, t + c);
		}

		template <class F, class V>
		std::tuple<F*, F* > alloc2(size_t c, F** pp,V v) {

			const locker_t lock(mutex_);

			pool_.push_back(holder_t());
			F* t = (F*)(pool_.back().p = mem_mng_t::malloc(c, sizeof(F)));

			std::fill(t, t + c, v);

			if (pp)
				*pp = t;
			
			return std::make_tuple(t, t + c);
		}


		template <class F>
		F* alloc(size_t c, F*) {

			const locker_t lock(mutex_);

			pool_.push_back(holder_t());
			return (F*)(pool_.back().p = mem_mng_t::malloc(c, sizeof(F)));
		}

		template <class F,class V>
		F* valloc(size_t c, V v) {

			const locker_t lock(mutex_);

			pool_.push_back(holder_t());
			auto p= (F*)(pool_.back().p = mem_mng_t::malloc(c, sizeof(F)));
			std::fill(p, p + c, v);
			return p;
		}

		std::list<holder_t> pool_;
		mutex_t mutex_;
	};


	template <int N>
	struct enum_op_t {
		template <class F>
		 enum_op_t(F f) {
			 for (auto k = 0; k < N; k++) {
				 f(k);
			 }
		};
	};


	template <>
	struct enum_op_t<0> {

		template <class F>
		enum_op_t(F f) {			
		};
	};

	template <>
	struct enum_op_t<1> {
		template <class F>
		enum_op_t(F f) {
			f(0);
		};
	};

	template <>
	struct enum_op_t<2> {
		template <class F>
		enum_op_t(F f) {
			f(0);
			f(1);
		};
	};

	template <>
	struct enum_op_t<3> {
		template <class F>
		enum_op_t(F f) {
			f(0);
			f(1);
			f(3);
		};
	};

	
	template <int N>
	struct loop_op_t {


		template <class F>
		inline void operator()(F f) {
			for (auto k = 0; k < N; k++) {
				f(k);
			}
		}
	};




	
	template <>
	struct loop_op_t<0> {
		template <class F>
		inline void operator()(F f) {};
	};


	
	template <>
	struct loop_op_t<1> {
		template <class F>
		inline void operator()(F f) { 
			f(0);
		}
	};

	template <>
	struct loop_op_t<2> {
		template <class F>
		inline void operator()(F f) {
			f(0);f(1);
		}
	};

		
	template <>
	struct loop_op_t<3> {
		template <class F>
		inline void operator()(F f) {
			f(0);f(1);f(2);
		}
	};
	

	template <>
	struct loop_op_t<4> {
		template <class F>
		inline void operator()(F f) {
			f(0);f(1);
			f(2);f(3);
		}
	};



};// end namespace utils

