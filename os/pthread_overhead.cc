#include <stddef.h>
#include <pthread.h>
#include "common.h"
#include <stdio.h>
#include <limits.h>
#include <inttypes.h>
#include <assert.h>

void *do_nothing(void *p)
{
    return p;
}

int measure()
{
    size_t times = 1000 * 100;

    // warmup
    pthread_t th;
    for (auto i = 0 ; i < times; i++) {
        pthread_create(&th, NULL, do_nothing, NULL);
        pthread_join(th, NULL);
    }

    long t0 = rdtsc();
    for (auto i = 0 ; i < times; i++) {
        pthread_create(&th, NULL, do_nothing, NULL);
        pthread_join(th, NULL);
    }
    long t1 = rdtsc();

    printf("tasking overhead = %ld\n", (t1 - t0) / times);

    return 0;
}

int main(int argc, char **argv)
{
    measure();
    return 0;
}

