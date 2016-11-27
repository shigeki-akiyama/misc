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
#if (defined __i386__) || (defined __x86_64__)
    uint32_t hi,lo;
    asm volatile("lfence\nrdtsc" : "=a"(lo),"=d"(hi));
    return (tsc_t)((uint64_t)hi)<<32 | lo;
#elif (defined __sparc__) && (defined __arch64__)
    uint64_t tick;
    asm volatile("rd %%tick, %0" : "=r" (tick));
    return (tsc_t)tick;
#else
#warning "rdtsc() is not implemented."
        return 0;
#endif
}

static inline tsc_t rdtsc_nofence(void)
{
#if (defined __i386__) || (defined __x86_64__)
    uint32_t hi,lo;
    asm volatile("rdtsc\n"
                :"=a"(lo),"=d"(hi));
    return ((uint64_t)hi)<<32 | lo;
#elif (defined __sparc__) && (defined __arch64__)
        uint64_t tick;
            asm volatile("rd %%tick, %0" : "=r" (tick));
                return (tsc_t)tick;
#else
#warning "rdtsc() is not implemented."
                        return 0;
#endif
}

static inline tsc_t rdtscp(void)
{
#if (defined __i386__) || (defined __x86_64__)
    uint32_t hi,lo;
    asm volatile("rdtscp\n"
                :"=a"(lo),"=d"(hi));
    return ((uint64_t)hi)<<32 | lo;
#elif (defined __sparc__) && (defined __arch64__)
        uint64_t tick;
            asm volatile("rd %%tick, %0" : "=r" (tick));
                return (tsc_t)tick;
#else
#warning "rdtsc() is not implemented."
                        return 0;
#endif
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


#include <mpi.h>


int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    
    if (argc != 4) {
        fprintf(stderr, "Usage: %s [none | lfence | rdtscp] [seq | par] count\n", argv[0]);
        exit(1);
    }
    char *lfence = argv[1];
    char *type = argv[2];
    long count = atol(argv[3]);

    assert(strcmp(lfence, "none") == 0 || 
           strcmp(lfence, "lfence") == 0 ||
           strcmp(lfence, "rdtscp") == 0);
    assert(strcmp(type, "seq") == 0 || strcmp(type, "par") == 0);

    int me, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = now();

    long sum = 0;
    if ((strcmp(type, "seq") == 0 && me == 0) ||
        (strcmp(type, "par") == 0)) {            

        long i;

        if (strcmp(lfence, "none") == 0) {
            for (i = 0; i < count; i++) {
                sum += rdtsc();
            }
        } else if (strcmp(lfence, "lfence") == 0) {
            for (i = 0; i < count; i++) {
                sum += rdtsc_nofence();
            }
        } else if (strcmp(lfence, "rdtscp") == 0) {
            for (i = 0; i < count; i++) {
                sum += rdtscp();
            }
        }
    }
    if (strcmp(type, "seq") == 0 && me != 0) {
        long i;
        for (i = 0; i < 1000 * 1000 * 1000; i++)
            sum += 1;
    }

    double t1 = now();

    MPI_Barrier(MPI_COMM_WORLD);

    if (me == 0) {
        printf("name       = busy_rdtsc\n");
        printf("lfence     = %12s\n", lfence);
        printf("count      = %12ld\n", count);
        printf("np         = %12d\n", n_procs);
        printf("time       = %12.6f\n", t1 - t0);
        printf("avg time   = %12.9f\n", (t1 - t0) / count);
        printf("throughput = %12.6f\n", count / (t1 - t0));
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return sum;
}





