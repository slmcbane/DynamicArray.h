DynamicArray.h
==============
This repository exists for the purpose of sharing the eponymous header file.
This header is a bad hack to give me a generic, dynamically-resizable array
in C that I can use for bits-types instead of writing a new dynamic array
implementation every time. I've made its behavior similar to `std::vector`
as much as it makes sense to do so.

Details
-------
The dynamic array works by reserving 3 * 64 = 192 extra bytes of space at the
beginning of an allocated block, and in these storing the current size, capacity,
and `sizeof(element_type)`. (I say 192 bytes; really it's `3 * sizeof(size_t)`).
When the user creates a dynamic array, the pointer returned is the one past end
of this metadata block. Don't call `free` on it! `dyn_arr_free` does the pointer
arithmetic and calls `free` on the correct pointer.

This structure means that the ``dynamic array'' can be indexed into just like
any other array, since it's just a naked pointer. From the outside, the metadata
is invisible.

Note that for mutating operations, the result of the operation must be assigned
back to the pointer; e.g.

```
arr = dyn_arr_insert(arr, &x, 0);
```

This is because these operations may trigger a reallocation, and I didn't want
to pass a `void**` and have to write `&arr` in every call.

Example
-------
```
dyn_arr(int) x = dyn_arr_init(int);
x = dyn_arr_reserve(x, 10);

for (int i = 0; i < 10; ++i)
    // Unfortunately, arguments have to be passed as a pointer so
    // that things work generically.
    x = dyn_arr_push(x, &i);

// x = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

dyn_arr(int) y = dyn_arr_copy(x);
int i = 10;
y = dyn_arr_insert(y, &i, dyn_arr_end(y));
// y = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

y = dyn_arr_shrink(y);
// Reallocate to use only required memory; otherwise allocations are optimistic.
```

More functionality (`pop`, `delete`, `insert_range`, `delete_range`) to follow.
