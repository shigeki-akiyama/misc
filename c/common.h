
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include <sys/time.h>

typedef uint64_t tsc_t;

static inline tsc_t rdtsc(void)
{
    uint32_t hi,lo;
    asm volatile("lfence\nrdtsc\n"
                :"=a"(lo),"=d"(hi));
    return ((uint64_t)hi)<<32 | lo;
}

static inline double now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec*1e-6;
}

static inline void die(const char *s)
{
    fprintf(stderr, "die: %s\n", s);
    exit(1);
}

static int random_int(int n)
{
    if (n == 0)
        return 0;

    size_t rand_max =
        ((size_t)RAND_MAX + 1) - ((size_t)RAND_MAX + 1) % (size_t)n;
    int r;
    do {
        r = rand();
    } while ((size_t)r >= rand_max);

    return (int)((double)n * (double)r / (double)rand_max);
}


