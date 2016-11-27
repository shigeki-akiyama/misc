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


static void real_main(int me, int n_procs, 
                      int n_msgs, int n_sendreqs, int n_recvreqs)
{
    int i, j;

    MPI_Comm comm = MPI_COMM_WORLD;
    int tag = 0;
    int recvtarget = (me + n_procs - 1) % n_procs;
    int sendtarget = (me + n_procs) % n_procs;

    int n_timebuf = 4096;
    tsc_t *timebuf = malloc(sizeof(tsc_t) * n_timebuf);
    long *sizebuf = malloc(sizeof(long) * n_timebuf);

    int data_size = 4096;
    void *data = malloc(data_size);
    memset(data, 0, data_size);

    int buf_size = data_size;
    void **recvbufs = malloc(sizeof(void *) * n_recvreqs);

    MPI_Request *sendreqs = malloc(sizeof(MPI_Request) * n_sendreqs);
    MPI_Request *recvreqs = malloc(sizeof(MPI_Request) * n_recvreqs);
   
    for (i = 0; i < n_recvreqs; i++) {
        recvbufs[i] = malloc(buf_size);
        memset(recvbufs[i], 0, buf_size);

        MPI_Irecv(recvbufs[i], buf_size, MPI_BYTE, recvtarget, tag, comm, 
                  &recvreqs[i]);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    tsc_t t0 = rdtsc();

    int count = 0;
    int timebuf_count = 0;
    long accum_data_size = 0;

    for (i = 0; i < n_sendreqs; i++) {
        tsc_t t0 = rdtsc();
        MPI_Isend(data, data_size, MPI_BYTE, sendtarget, tag, comm, 
                  &sendreqs[i]);
        tsc_t t1 = rdtsc();

        accum_data_size += data_size;

        timebuf[timebuf_count] = t1 - t0;
        sizebuf[timebuf_count] = accum_data_size;
        timebuf_count++;
    }

    for (;;) {
        int complete, idx;

        MPI_Testany(n_recvreqs, recvreqs, &idx, &complete, MPI_STATUS_IGNORE);
        if (complete) {
            MPI_Wait(&recvreqs[idx], MPI_STATUS_IGNORE);

            MPI_Irecv(recvbufs[idx], buf_size, MPI_BYTE, recvtarget, tag, comm,
                      &recvreqs[idx]);
        }

        MPI_Testany(n_sendreqs, sendreqs, &idx, &complete, MPI_STATUS_IGNORE);
        if (complete) {
            MPI_Wait(&sendreqs[idx], MPI_STATUS_IGNORE);

            if (++count >= n_msgs)
                break;

            tsc_t t0 = rdtsc();
            MPI_Isend(data, data_size, MPI_BYTE, sendtarget, tag, comm,
                      &sendreqs[idx]);
            tsc_t t1 = rdtsc();

            accum_data_size += data_size;

            timebuf[timebuf_count] = t1 - t0;
            sizebuf[timebuf_count] = accum_data_size;
            timebuf_count++;

            if (timebuf_count >= n_timebuf) {
                for (i = 0; i < timebuf_count; i++) {
                    fprintf(stdout, "%3d: t = %9llu (%9ld bytes)\n",
                            me, timebuf[i], sizebuf[i]);
                }

                timebuf_count = 0;
            }
        }
    }

    for (i = 0; i < timebuf_count; i++) {
        fprintf(stdout, "%3d: t = %9llu (%9ld bytes)\n",
                me, timebuf[i], sizebuf[i]);
    }
    fflush(stdout);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    
    if (argc != 4) {
        fprintf(stderr, "Usage: %s n_msgs n_sendreqs n_recvreqs\n", argv[0]);
        exit(1);
    }
    int n_msgs = atoi(argv[1]);
    int n_sendreqs = atoi(argv[2]);
    int n_recvreqs = atoi(argv[3]);

    int me, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    MPI_Barrier(MPI_COMM_WORLD);

    real_main(me, n_procs, n_msgs, n_sendreqs, n_recvreqs);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}





