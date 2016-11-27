#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

static inline double now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec*1e-6;
}

long fib(long n) {
    if (n < 2) {
        return n;
    } else {
        return fib(n - 1) + fib(n - 2);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s N\n", argv[0]);
        exit(1);
    }
    long n = atol(argv[1]);

    double t0 = now();
    long result = fib(n);
    double t1 = now();

    printf("fib(%ld) = %ld\n", n, result);
    printf("time = %.6f\n", t1 - t0);

    return 0;
}

