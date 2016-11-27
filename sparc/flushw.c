#include <stdio.h>
#include "../c/common.h"

__attribute__((noinline))
void f(long depth) {
    void *sp__;
    asm volatile (
        "mov %%sp, %0\n\t"
        "flushw\n\t"
    :: "r"(sp__) : );

    if (depth >= 1)
        f(depth - 1);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s n depth\n", argv[0]);
        exit(1);
    }
    long n = atol(argv[1]);
    long depth = atol(argv[2]);

    double t0 = now();

    int i;
    for (i = 0; i < n; i++) {
#if 0
        asm volatile (
            "flushw\n\t"
        );
#else
        f(depth);
#endif
    }

    double t1 = now();

    printf("n      = %12ld\n", n);
    printf("depth  = %12ld\n", depth);
    printf("time   = %12.9f\n", (t1 - t0) / (n * depth));

    return 0;
}
