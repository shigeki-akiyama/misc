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


#include <gasnet.h>

/* Macro to check return codes and terminate with useful message. */
#define GASNET_SAFE(fncall) do {                                     \
    int _retval;                                                     \
    if ((_retval = fncall) != GASNET_OK) {                           \
      fprintf(stderr, "ERROR calling: %s\n"                          \
                      " at: %s:%i\n"                                 \
                      " error: %s (%s)\n",                           \
              #fncall, __FILE__, __LINE__,                           \
              gasnet_ErrorName(_retval), gasnet_ErrorDesc(_retval)); \
      fflush(stderr);                                                \
      abort();                                                       \
      /*gasnet_exit(_retval);*/                                      \
    }                                                                \
  } while(0)

#define MAKEWORD(hi,lo)                                             \
    ((((uint64_t)(hi)) << 32) | (((uint64_t)(lo)) & 0xFFFFFFFF))
#define HIWORD(arg)     ((uint32_t)(((uint64_t)(arg)) >> 32))
#define LOWORD(arg)     ((uint32_t)((uint64_t)(arg)))


void comm_barrier(void)
{
    // sometimes gasnet_barrier_notify does not progress
    // in PSHM configuration (?).

    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    for (;;) {
        int result = gasnet_barrier_try(0, GASNET_BARRIERFLAG_ANONYMOUS);
        if (result == GASNET_OK)
            break;
        else if (result == GASNET_ERR_NOT_READY)
            ;
        else
            assert(0);
    }
}   

int main(int argc, char **argv)
{
    {
        static gasnet_handlerentry_t *handlers = NULL;

        GASNET_SAFE(gasnet_init(&argc, &argv));

        uintptr_t max_segsize = gasnet_getMaxLocalSegmentSize();
        uintptr_t minheapoffset = GASNET_PAGESIZE;

        uintptr_t segsize = max_segsize / GASNET_PAGESIZE * GASNET_PAGESIZE;

        GASNET_SAFE(gasnet_attach(handlers, 0,
                                  segsize, minheapoffset));
    }

    if (argc != 3) {
        fprintf(stderr, "Usage: %s [seq | par] count\n", argv[0]);
        exit(1);
    }
    char *type = argv[1];
    long count = atol(argv[2]);

    assert(strcmp(type, "seq") == 0 || strcmp(type, "par") == 0);
    
    size_t me = gasnet_mynode();
    size_t n_procs = gasnet_nodes();

    comm_barrier();
    double t0 = now();
    {
        if ((strcmp(type, "seq") == 0 && me == 0) ||
            (strcmp(type, "par") == 0)) {            
            long i;
            for (i = 0; i < count; i++)
                gasnet_AMPoll();
        }
    }
    comm_barrier();
    double t1 = now();

    if (me == 0) {
        printf("count      = %12ld\n", count);
        printf("np         = %12zu\n", n_procs);
        printf("time       = %12.6f\n", t1 - t0);
        printf("throughput = %12.6f\n", count / (t1 - t0));
    }

    comm_barrier();
    gasnet_exit(0);
    return 0;
}
