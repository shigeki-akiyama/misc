#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
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


#include <mpi.h>


int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int me, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    char host[1024];
    gethostname(host, 1024);

    int i;
    for (i = 0; i < n_procs; i++) {
        if (i == me) {
            printf("%d: %s\n", me, host);
            fflush(stdout);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}





