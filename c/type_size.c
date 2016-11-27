#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <inttypes.h>

int main(void) {
    printf("sizeof(void) = %zu\n", sizeof(void));
    printf("sizeof(unsigned long) = %zu\n", sizeof(unsigned long));
    printf("sizeof(uint64_t) = %zu\n", sizeof(uint64_t));

    printf("ULONG_MAX  = %lu\n", ULONG_MAX);
    printf("UINT64_MAX = %" PRIu64 "\n", UINT64_MAX);

#if ULONG_MAX == UINT64_MAX
    printf("ULONG_MAX == UINT64_MAX\n");
#else
    printf("ULONG_MAX != UINT64_MAX\n");
#endif
    return 0;
}
