#include <stddef.h>
#include <pthread.h>
#include "common.h"
#include <stdio.h>
#include <limits.h>
#include <inttypes.h>
#include <assert.h>

long t0, t1, t2, t3;

void *do_nothing(void *p)
{
    t1 = rdtsc();
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

    long switch_child = 0;
    long continue_parent = 0;
    long join_from_parent = 0;
    long join_from_child = 0;

    for (auto i = 0 ; i < times; i++) {
        t0 = rdtsc();
        pthread_create(&th, NULL, do_nothing, NULL);
        t2 = rdtsc();
        pthread_join(th, NULL);
        t3 = rdtsc();

        switch_child += t1 - t0;
        continue_parent += t2 - t0;
        join_from_parent += t3 - t2;
        join_from_child += t3 - t1;
    }

    printf("switch_child     = %9ld\n", switch_child / times);
    printf("continue_parent  = %9ld\n", continue_parent / times);
    printf("join_from_parent = %9ld\n", join_from_parent / times);
    printf("join_from_child  = %9ld\n", join_from_child / times);

    return 0;
}

int main(int argc, char **argv)
{
    measure();
    return 0;
}

