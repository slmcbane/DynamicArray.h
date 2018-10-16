#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * This is a basic facility to have a type-agnostic dynamic array supporting
 * some of the functionality of std::vector in C++. The user interacts with
 * arrays as pointer-to-type as usual, so that indexing in the normal way is
 * supported, but must manipulate the arrays using the functions provided here.
 * The metadata for the array is contained in the 24 bytes before the pointer
 * provided to the user. The start of the array is guaranteed to be 8-byte
 * aligned; beyond that, the management of alignment, only storing objects of
 * the correct type, etc. is up to the user.
 * 
 * The layout is as follows:
 * <-- 8 bytes  -->
 * |  array size  | array capacity | sizeof(dtype) | values...
 * ^                                               ^
 * block start                                     pointer returned to user
 */

static inline void* _dyn_arr_init_impl(size_t type_size)
{
    size_t* block_start = (size_t*) malloc(sizeof(size_t) * 3);
    if (!block_start) {
        fprintf(stderr, "Error in _dyn_arr_init_impl - failed to allocate"
                        " for block metadata.\n");
        exit(-1);
    }
    block_start[0] = 0;
    block_start[1] = 0;
    block_start[2] = type_size;
    return (void*)(block_start + 3);
}

/* 
 * Initialize a dynamic array (pointer returned has 0 capacity).
 * The parameter 'ty' should be the name of the type that this array will hold.
 */
#define dyn_arr_init(ty) _dyn_arr_init_impl(sizeof(ty))
/*
 * Retrieve the size (number of stored items) of a dynamic array
 */
#define dyn_arr_size(ptr) (*((const size_t*)(ptr) - 3))
/*
 * Retrieve the capacity (number of elements that may be stored before
 * reallocation) of a dynamic array.
 */
#define dyn_arr_capacity(ptr) (*((const size_t*)(ptr) - 2))
/*
 * Get the size of the stored element type (in bytes).
 */
#define dyn_arr_elsize(ptr) (*((const size_t*)(ptr) - 1))
/*
 * Get the one-past-the-end pointer from the dynamic array.
 * Can be used for C++ style iteration.
 */
#define dyn_arr_end(ptr) ((const void*) ((const char*)(ptr) + \
                          dyn_arr_elsize(ptr)*dyn_arr_size(ptr)))
/*
 * Release memory allocated by dyn_arr_init
 */
#define dyn_arr_free(ptr) free((size_t*)ptr - 3)
/*
 * Trivial substitution so it's clear where a pointer is allocated
 * using this facility.
 */
#define dyn_arr(ty) ty*

/*
 * Reserve space to grow the array, without increasing the array's size.
 * This allows push to work without any extra allocations.
 */
static inline void* dyn_arr_reserve(void* ptr, size_t size)
{
    if (dyn_arr_capacity(ptr) >= size) return ptr;

    void* block_begin = (size_t*)ptr - 3;
    block_begin = realloc(block_begin, 3*sizeof(size_t) +
                                       size * dyn_arr_elsize(ptr));

    if (!block_begin) {
        fprintf(stderr, "Error in _dyn_arr_reserve_impl - "
                        "failed to reallocate\n");
        exit(-1);
    }

    *((size_t*)block_begin + 1) = size;
    return (void*)((size_t*)block_begin + 3);
}

/*
 * Resize the array to size = `size`. The new values are initialized to 
 * all-bits zero, so for integer or floating point values it should be 
 * equivalent to zero-initialization.
 */
static inline void* dyn_arr_resize(void* ptr, size_t size)
{  
    if (size <= dyn_arr_size(ptr)) {
        *((size_t*)ptr - 3) = size;
        return ptr;
    }

    ptr = dyn_arr_reserve(ptr, size);
    memset((char*)ptr + dyn_arr_elsize(ptr) * dyn_arr_size(ptr), 
           0, dyn_arr_elsize(ptr)*size - dyn_arr_elsize(ptr) * dyn_arr_size(ptr));
    *((size_t*)ptr - 3) = size;
    return ptr;
}

/*
 * Pushes the value stored in `val` on the array. Works by a pure bit-for-bit
 * copy. Resizes the the array if necessary; therefore returns a pointer to
 * the reallocated buffer.
 */
static inline void* dyn_arr_push(void* ptr, const void* val)
{
    if (dyn_arr_capacity(ptr) < dyn_arr_size(ptr) + 1)
        ptr = dyn_arr_reserve(ptr, dyn_arr_size(ptr) * 3 / 2 + 1);
    memcpy((char*)ptr + dyn_arr_elsize(ptr) * dyn_arr_size(ptr), val, 
           dyn_arr_elsize(ptr));
    *((size_t*)ptr - 3) = dyn_arr_size(ptr) + 1;
    return ptr;
}

/*
 * Insert the element pointed at `val` before the element stored at index
 * `pos` in the array. If `pos == dyn_arr_size(ptr)`, same as dyn_arr_push.
 */
static inline void* dyn_arr_insert(void* ptr, const void* val, size_t pos)
{
    assert(pos <= dyn_arr_size(ptr));
    if (dyn_arr_capacity(ptr) < dyn_arr_size(ptr) + 1)
        ptr = dyn_arr_reserve(ptr, dyn_arr_size(ptr) * 3 / 2 + 1);
    memmove((char*)ptr + dyn_arr_elsize(ptr)*(pos + 1), 
            (char*)ptr + dyn_arr_elsize(ptr)*pos, 
            dyn_arr_elsize(ptr)*(dyn_arr_size(ptr) - pos));
    memcpy((char*)ptr + pos * dyn_arr_elsize(ptr), val, dyn_arr_elsize(ptr));
    *((size_t*)ptr - 3) = dyn_arr_size(ptr) + 1;
    return ptr;
}

/*
 * Shrink the array to take up just enough space for the data it holds,
 * don't keep extra space reserved. The implementation otherwise keeps other
 * space around, assuming that the array may grow again.
 */
static inline void* dyn_arr_shrink(void* ptr)
{
    if (!(dyn_arr_size(ptr) < dyn_arr_capacity(ptr)))
        return ptr;
    void* block_begin = (size_t*) ptr - 3;
    block_begin = realloc(block_begin, sizeof(size_t)*3 + dyn_arr_elsize(ptr) * 
                          (dyn_arr_size(ptr)+1));
    if (!block_begin) {
        fprintf(stderr, "Error reallocating in dyn_arr_shrink\n");
        exit(-1);
    }
    ptr = (size_t*)block_begin + 3;
    *((size_t*)ptr - 2) = dyn_arr_size(ptr);
    return ptr;
}

/*
 * Return a new array initialized by dyn_arr_init and holding the same
 * data as the input. The new array will not have the reserved space that the
 * copied array has.
 */
static inline void* dyn_arr_copy(const void* ptr)
{
    void* new_ptr = dyn_arr_resize(_dyn_arr_init_impl(dyn_arr_elsize(ptr)), 
                                   dyn_arr_size(ptr));
    memcpy(new_ptr, ptr, dyn_arr_elsize(ptr) * dyn_arr_size(ptr));
    return new_ptr;
}

#endif
