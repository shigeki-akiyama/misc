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


#include <mpi.h>


int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [seq | par] count\n", argv[0]);
        exit(1);
    }
    char *type = argv[1];
    long count = atol(argv[2]);

    assert(strcmp(type, "seq") == 0 || strcmp(type, "par") == 0);

    int me, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    size_t next = (me + 1) % n_procs;

    int buf;
    MPI_Request req;
    MPI_Irecv(&buf, 1, MPI_INT, next, 0, MPI_COMM_WORLD, &req);

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = now();

    if ((strcmp(type, "seq") == 0 && me == 0) ||
        (strcmp(type, "par") == 0)) {            

        long i;
        for (i = 0; i < count; i++) {
            int flag;
            MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t1 = now();

    if (me == 0) {
        printf("name       = para_mpi_test\n");
        printf("count      = %12ld\n", count);
        printf("np         = %12d\n", n_procs);
        printf("time       = %12.6f\n", t1 - t0);
        printf("throughput = %12.6f\n", count / (t1 - t0));
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}





