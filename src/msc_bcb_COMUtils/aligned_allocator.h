#pragma once

#include <malloc.h>
#include <stdio.h>
#include <crtdefs.h>

	template <class T,int ALIGN_DIG=4>
	class aligned_allocator_t { 
	public:
		enum
		{
			memalign=1<<ALIGN_DIG
		};
		
		
		typedef typename T value_type;
		typedef value_type  *pointer;
		typedef value_type& reference;
		typedef const value_type *const_pointer;
		typedef const value_type & const_reference;

		typedef size_t size_type;
		typedef ptrdiff_t difference_type;


		
		template<class _Other>
		struct rebind
		{	typedef aligned_allocator_t<_Other,ALIGN_DIG> other;};


		aligned_allocator_t() throw (){

		}

		aligned_allocator_t(const aligned_allocator_t<T,ALIGN_DIG>&) throw ()	{}

		template<class _Other>
		aligned_allocator_t(const aligned_allocator_t<_Other,ALIGN_DIG>& t) throw (){}

		template<class _Other>
		aligned_allocator_t<T,ALIGN_DIG>& operator=(const aligned_allocator_t<_Other,ALIGN_DIG>&)
		{	
			return (*this);
		}

		pointer address(reference x)
		{ return &x; };
		const_pointer const_address(const_reference x)
		{ return &x; };
		pointer allocate(size_type n)
		{
            T* t=(T*) _aligned_malloc(sizeof(T)*n,memalign);
			return t;
		};
		void deallocate(pointer p,size_t )
		{
			_aligned_free(p);
		};

		void construct(pointer _Ptr, const T& _Val)
		{	
			void  *_Vptr = _Ptr;
			::new (_Vptr) T(_Val);
		};

		void destroy(pointer _Ptr)
		{	// destroy object at _Ptr
			(_Ptr)->~T();
		}
		
		size_type max_size() const
		{
			size_type _Count = (size_type)(-1) / sizeof (T);
			return (0 < _Count ? _Count : 1);
		};
	};

	template<int ALIGN_DIG> class  aligned_allocator_t<void,ALIGN_DIG>
	{	// generic allocator for type void
	public:
		typedef void T;
		typedef T  *pointer;
		typedef const T  *const_pointer;
		typedef T value_type;

		aligned_allocator_t() throw (){}

		aligned_allocator_t(const aligned_allocator_t<T,ALIGN_DIG>&) throw ()	{}

		template<class _Other>
		aligned_allocator_t(const aligned_allocator_t<_Other,ALIGN_DIG>& t) throw (){}

		template<class _Other>
		aligned_allocator_t<T,ALIGN_DIG>& operator=(const aligned_allocator_t<_Other,ALIGN_DIG>&)
		{	
			return (*this);
		}
	};


	template<class _Ty,	class _Other,int ALIGN_DIG> 
	inline	bool operator == (const aligned_allocator_t<_Ty,ALIGN_DIG>&, const aligned_allocator_t<_Other,ALIGN_DIG>&)
	{	// test for allocator equality (always true)
		return (true);
	}

	template<class _Ty,	class _Other,int ALIGN_DIG> 
	inline	bool operator !=(const aligned_allocator_t<_Ty,ALIGN_DIG>&, const aligned_allocator_t<_Other,ALIGN_DIG>&)
	{	// test for allocator equality (always true)
		return (false);
	}

