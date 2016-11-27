#include <stdio.h>
#include "common.h"

long fib(long n) {
    if (n < 2) {
        return n;
    } else {
        return fib(n-1) + fib(n-2);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s n\n", argv[0]);
        exit(1);
    }
    long n = atol(argv[1]);

    double t0 = now();

    long result = fib(n);

    double t1 = now();

    printf("n      = %12ld\n", n);
    printf("result = %12ld\n", result);
    printf("time   = %12f\n", t1 - t0);

    return 0;
}
