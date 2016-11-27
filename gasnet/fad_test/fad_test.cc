#include <mpi.h>
#include <gasnet.h>

#include "common.h"
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <vector>


int debug = 0;

void barrier()
{
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);            
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS); 
}

void wait()
{
    long t = rdtsc();
    while (rdtsc() - t < 100 * 1000 * 1000)
        gasnet_AMPoll();
}

static void real_main(int me, int n_procs, int times, int add_value,
                      char *segment, size_t size)
{
    // centralized atomic count
    {
        int target = 0;

        uint64_t *lock      = (uint64_t *)(segment + 0);
        uint64_t *lockbuf   = (uint64_t *)(segment + sizeof(uint64_t));

        *lock = 0;

        printf("%d: lock    = %p\n", me, lock);
        printf("%d: lockbuf = %p\n", me, lockbuf);

        __sync_synchronize();
        gasnet_wait_syncnbi_all();
        barrier();
        MPI_Barrier(MPI_COMM_WORLD);
        wait();

#define SERIALIZE 0
#if SERIALIZE
        for (int proc = 0; proc < n_procs; proc++) {
            if (proc == me) {
#endif
                for (int i = 0; i < times; i++) {
                    gasnet_fetch_and_add_u64(lockbuf, target, lock,
                                             add_value);

                    long t = rdtsc();
                    while (rdtsc() - t < 1000 * 1000)
                        gasnet_AMPoll();
                }
#if SERIALIZE
            }
            barrier();
        }
#endif
        __sync_synchronize();
        gasnet_wait_syncnbi_all();
        barrier();
        MPI_Barrier(MPI_COMM_WORLD);
        wait();

        if (me == 0) {
            if (*lock == n_procs * times * add_value) {
                printf("centralized atomic count: OK (%lu)\n", *lock);
            } else {
                printf("centralized atomic count: NG (%lu)\n", *lock);
            }
        } else {
            assert(*lock == 0);
        }
    }
}

int main(int argc, char **argv)
{
    gasnet_init(&argc, &argv);
    
    int init;
    MPI_Initialized(&init);

    if (!init)
        MPI_Init(&argc, &argv);

    MPI_Comm comm = MPI_COMM_WORLD;
    int me = gasnet_mynode();
    int n_procs = gasnet_nodes();

    // print basic info
    {
        char name[MPI_MAX_PROCESSOR_NAME];
        int len;
        MPI_Get_processor_name(name, &len);

        printf("me = %d, n_procs = %d, host = %s\n", me, n_procs, name);

        printf("GASNET_ALIGNED_SEGMENTS: %d\n", GASNET_ALIGNED_SEGMENTS);
        printf("GASNET_PSHM: %d\n", GASNET_PSHM);
    }

    // parse arguments
    if (argc < 3) {
        if (me == 0)
            fprintf(stderr, "Usage: %s n_msgs\n", argv[0]);
        gasnet_exit(0);
    }
    size_t n_args = 1;
    int times       = atoi(argv[n_args++]);
    int add_value   = atoi(argv[n_args++]);

    if (me == 0)
        printf("times = %d, add_value = %d\n", times, add_value);

    // attach segment
    int segsize = 8192;
    int minheapoffset = 8192;
    gasnet_attach(NULL, 0, segsize, minheapoffset);

    // verify segment
    gasnet_seginfo_t *seginfo = new gasnet_seginfo_t[n_procs];
    gasnet_getSegmentInfo(seginfo, n_procs);
    gasnet_seginfo_t *si = &seginfo[0];
    {
        for (int i = 1; i < n_procs; i++) {
            if (si->addr != seginfo[i].addr)
                fprintf(stderr, "address error (%p, %p) in %d\n",
                        si->addr, seginfo[i].addr, i);
            if (si->size != seginfo[i].size)
                fprintf(stderr, "size error in %d\n", i);
        }
    }

    printf("me = %d, start.\n", me);

    MPI_Barrier(comm);

    real_main(me, n_procs, times, add_value, (char *)si->addr, si->size);

    MPI_Barrier(comm);

    if (me == 0)
        printf("ALL DONE\n");

    if (!init)
        MPI_Finalize();

    gasnet_exit(0);
    return 0;
}

