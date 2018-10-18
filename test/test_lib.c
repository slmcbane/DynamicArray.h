#include "../src/DynamicArray.h"

#include <assert.h>
#include <stdio.h>

int main()
{
    dyn_arr(int) x = dyn_arr_init(int);
    dyn_arr(int) y = dyn_arr_init(int);

    for (int i = 0; i < 5; ++i) x = dyn_arr_push(x, &i);
    assert(dyn_arr_size(x) == 5);
    for (int i = 0; i < 5; ++i) assert(x[i] == i);
    
    for (int i = 5; i < 10; ++i) y = dyn_arr_push(y, &i);
    assert(dyn_arr_size(y) == 5);
    for (int i = 0; i < 5; ++i) assert(y[i] == i+5);

    x = dyn_arr_insert_range(x, y, 5, 0);
    assert(dyn_arr_size(x) == 10);
    for (int i = 0; i < 5; ++i) assert(x[i] == i + 5 && x[i+5] == i);

    x = dyn_arr_delete_range(x, 5, 10);
    assert(dyn_arr_size(x) == 5);
    for (int i = 0; i < 5; ++i) assert(x[i] == y[i]);

    x = dyn_arr_shrink(x);
    for (int i = 4; i >= 0; --i) x = dyn_arr_insert(x, &i, 0);
    assert(dyn_arr_size(x) == 10);
    for (int i = 0; i < 5; ++i) assert(x[i] == i);

    x = dyn_arr_delete(x, 5);
    assert(x[5] == 6);
    int tmp = 5;
    x = dyn_arr_insert(x, &tmp, 5);
    assert(x[5] == 5);

    x = dyn_arr_shrink(dyn_arr_resize(x, 0));
    x = dyn_arr_insert_range(x, y, dyn_arr_size(y), dyn_arr_size(x));
    for (unsigned i = 0; i < dyn_arr_size(y); ++i) assert(x[i] == y[i]);
    x = dyn_arr_insert_range(x, y, dyn_arr_size(y), dyn_arr_size(x));
    for (unsigned i = 0; i < dyn_arr_size(y); ++i) assert(x[i+5] == y[i]);
    x = dyn_arr_insert_range(x, y, 5, 5);
    for (unsigned i = 0; i < dyn_arr_size(y); ++i) assert(x[i+5] == y[i] && x[i+10] == y[i]);
    
    dyn_arr_free(x);
    dyn_arr_free(y);

    return 0;
}
