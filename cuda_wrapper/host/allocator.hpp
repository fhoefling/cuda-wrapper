/* Allocator that wraps cuMemHostAlloc -*- C++ -*-
 *
 * Copyright (C) 2016 Felix Höfling
 * Copyright (C) 2008 Peter Colberg
 * Copyright (C) 2020 Jaslo Ziska
 *
 * This file is part of cuda-wrapper.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
 */

#ifndef CUDA_HOST_ALLOCATOR_HPP
#define CUDA_HOST_ALLOCATOR_HPP

#include <limits>

#include <cuda.h>

#include <cuda_wrapper/error.hpp>

namespace cuda {
namespace host {

using std::size_t;
using std::ptrdiff_t;

/*
 * The implementation of a custom allocator class for the STL is described
 * here and there:
 * http://www.codeproject.com/Articles/4795/C-Standard-Allocator-An-Introduction-and-Implement
 * http://stackoverflow.com/a/11417774
 *
 * The same pattern is used in ext/malloc_allocator.h of the GNU Standard C++
 * Library, which wraps "C" malloc.
 */

template <typename T>
class allocator {
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    template <typename U> struct rebind { typedef allocator<U> other; };

    allocator(unsigned int flags = 0) throw() : _flags(flags) {}
    allocator(const allocator& alloc) throw() : _flags(alloc._flags) {}

    template<typename U>
    allocator(const allocator<U>& alloc) throw() : _flags(alloc._flags) {}

    ~allocator() throw() {}

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }

    pointer allocate(size_type s, void const* = 0)
    {
        void* p;

        if (__builtin_expect(s > this->max_size(), false))
        {
            throw std::bad_alloc();
        }

        CU_CALL(cuMemHostAlloc(&p, s * sizeof(T), _flags));

        return reinterpret_cast<pointer>(p);
    }

    void deallocate(pointer p, size_type) throw() // no-throw guarantee
    {
        cuMemFreeHost(reinterpret_cast<void *>(p));
    }

    size_type max_size() const throw()
    {
        return std::numeric_limits<size_t>::max() / sizeof(T);
    }

    void construct(pointer p, const T& val)
    {
        ::new((void *)p) T(val);
    }

    void destroy(pointer p)
    {
        p->~T();
    }

private:
    unsigned int _flags;
};

template<typename T>
inline bool operator==(const allocator<T>&, const allocator<T>&)
{
    return true;
}

template<typename T>
inline bool operator!=(const allocator<T>&, const allocator<T>&)
{
    return false;
}

} // namespace host
} // namespace cuda

#endif
