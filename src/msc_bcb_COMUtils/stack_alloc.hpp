#pragma once

/**
 * @file stack_alloc.hpp
 * @author sergey_volkov@tgtoil.com
 * @author Copyright(C) 2011 TGT Oil & Gas Services. All rights reserved. 
 * @since 2010/11/08
 */

#include <cstddef>
#include <type_traits>
#include <limits>

namespace utils { namespace alloc {

    /** Primary template forward definition */
    template <class T, std::size_t N> class stack_alloc;

    template <std::size_t N>
    class stack_alloc<void, N> {
    public:
        typedef const void*       const_pointer;
        typedef void              value_type;
    };
    /** Stack based allocator. 
     * Allocates space for N object of type T
     * in the stack. When stack space is exhausting
     * allocates requested space on heap.
     * Usage:
     * @begin code
     * std::vector<int, utils::alloc::stack_alloc<int, 100> > v;
     * v.reserve(100); // v is allocated on the stack
     * @end code
     */
    template <class T, std::size_t N>
    class stack_alloc {
    public:
        typedef std::size_t       size_type;
        typedef T                 value_type;
        typedef value_type*       pointer;
        typedef const value_type* const_pointer;
        typedef value_type&       reference;
        typedef const value_type& const_reference;
        typedef std::ptrdiff_t    difference_type;

    public:
        stack_alloc() throw() : ptr_(pointer(&buf_)) {}
        stack_alloc(stack_alloc const&) throw() : ptr_(pointer(&buf_)) {}
        template <class U> 
        stack_alloc(stack_alloc<U, N> const&) throw() : ptr_(pointer(&buf_)) {}
        template <class U> 
        struct rebind { typedef stack_alloc<U, N> other; };

    public:
        pointer allocate(size_type n, typename stack_alloc<void, N>::const_pointer = 0) {
            if (pointer(&buf_) + N - ptr_ >= n) {
                pointer r = ptr_;
                ptr_ += n;
                return r;
            }
            return static_cast<pointer>(::operator new(n * sizeof(T)));
        }
        void deallocate(pointer p, size_type n) {
            if (pointer(&buf_) <= p && p < pointer(&buf_) + N) {
                if (p + n == ptr_) ptr_ = p;
            } else
                ::operator delete(p);
        }

#ifdef		maxsss
#undef maxsss
#endif

        size_type max_size() const throw() {
           return 11111;//std::numeric_limits<size_type>::max() / sizeof(T);

        }



        void destroy(T* p) { p->~T(); }
        void construct(pointer p) { ::new((void*)p) T(); }

        template <class A0>
        void construct(pointer p, const A0& a0) {
            ::new((void*)p) T[a0];
        }
        bool operator == (stack_alloc& a) const {
            return &buf_ == &a.buf_;
        }
        bool operator != (stack_alloc& a) const {
            return &buf_ != &a.buf_;
        }
    private:
        enum { DEF_ALIGN = 16 };
        stack_alloc& operator = (const stack_alloc&);
        typename std::tr1::aligned_storage<sizeof(T) * N, DEF_ALIGN>::type buf_;
        pointer ptr_;
    };


}} //namespace
