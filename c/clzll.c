
#include <stdio.h>
#include <stdint.h>

static size_t
ceil_with_pow2(size_t size)
{
    return 1ul << (64 - __builtin_clzl(size - 1));
}


int main(void) {
    uint64_t i;
    for (i = 1 ; i <= 32; i++) {
       printf("__builtin_clzll(%lld) = %d\n", i, __builtin_clzll(i));
       printf("__builtin_ctzll(%lld) = %d\n", i, __builtin_ctzll(i));
       printf("ceil_with_pow2(%lld)  = %d\n", i, ceil_with_pow2(i));
    }

    return 0;
}
