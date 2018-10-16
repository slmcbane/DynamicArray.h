#include "../src/DynamicArray.h"

#include <assert.h>
#include <stdio.h>

const char* alphabet = "abcdefghijklmnopqrstuvwxyz";
const char* ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int main()
{
    dyn_arr(char) x = dyn_arr_init(char);
    for (int i = 0; i <= 26; ++i)
        x = dyn_arr_push(x, &alphabet[i]);
    assert(*(((char *) dyn_arr_end(x)) - 1) == '\0');
    assert(*(((char *) dyn_arr_end(x)) - 2) == 'z');
    assert(dyn_arr_size(x) == 27);

    printf("x = \"%s\"; dyn_arr_capacity(x) = %ld\n", x, dyn_arr_capacity(x));

    for (int i = 0; i < 26; ++i)
        x = dyn_arr_insert(x, &ALPHABET[i], dyn_arr_size(x) - 1);
    assert(*(((char *) dyn_arr_end(x)) - 1) == '\0');
    assert(*(((char *) dyn_arr_end(x)) - 2) == 'Z');

    printf("x = \"%s\"; dyn_arr_capacity(x) = %ld\n", x, dyn_arr_capacity(x));
    x = dyn_arr_shrink(x);
    printf("After shrinking, dyn_arr_capacity(x) = %ld\n", dyn_arr_capacity(x));
    dyn_arr_free(x);

    return 0;
}
