#include <mpi.h>
#include <gasnet.h>

#include "common.h"
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <vector>


struct entry {
    uint64_t lock;
    long data;
};

struct timebuf {
    int target;
    int fad;
    int flush0;
    int get;
    int flush1;
    int put;
    int flush2;
    int put2;
    int flush3;
    int barrier;
};

int debug = 0;

static void real_main(int me, int n_procs, int n_msgs, void *addr, size_t size)
{
    MPI_Comm comm = MPI_COMM_WORLD;

    std::vector<timebuf> timebufs(n_msgs);
    memset(&timebufs[0], 0, sizeof(timebuf) * n_msgs);

    MPI_Barrier(comm);

    int next = (me + 1) % n_procs;
    int prev = (me + n_procs - 1) % n_procs;

    uint8_t *p = (uint8_t *)addr;

    uint64_t *lockbuf = (uint64_t *)p;
    p += sizeof(*lockbuf);

    entry *e = (entry *)p;
    p += sizeof(*e);

    entry *buf = (entry *)p;
    p += sizeof(*buf);

    if (debug) printf("INIT DONE\n");

    int idx = 0;
    for (int i = 0; i < n_msgs; i++) {

        int target = next;

        e->lock = 0;
        e->data = me;

        MPI_Barrier(comm);

        tsc_t t0 = rdtsc();

//        do {
            *lockbuf = 256;
            gasnet_fetch_and_add_u64(lockbuf, target, &e->lock, 1);

            tsc_t t1 = rdtsc();

            if (*lockbuf != 0)
                fprintf(stderr,
                        "%d: fetch_and_add error (i = %9d, lockbuf = %d)\n",
                        me, i, lockbuf);
//        } while (lockbuf != 0);

        tsc_t t2 = rdtsc();

        gasnet_get(buf, target, e, sizeof(*e));

        tsc_t t3 = rdtsc();

        tsc_t t4 = rdtsc();

        buf->data += me + i;
        gasnet_put(target, e, buf, sizeof(*e));

        tsc_t t5 = rdtsc();

        gasnet_wait_syncnbi_all();

        tsc_t t6 = rdtsc();

#if 1
        gasnet_get(lockbuf, target, &e->lock, sizeof(e->lock));
        if (*lockbuf != 1)
            fprintf(stderr,
                    "%d: fetch_and_add error (%i = %9d, e->lock = %d)\n",
                    me, i, *lockbuf);
#endif

        *lockbuf = 0;
        gasnet_put(target, &e->lock, lockbuf, sizeof(e->lock));

        tsc_t t7 = rdtsc();

//        ARMCI_Fence(target);

        tsc_t t8 = rdtsc();

        MPI_Barrier(comm);

        tsc_t t9 = rdtsc();

        if (e->data != me + prev + i || e->lock != 0) {
            fprintf(stderr, "%d: error (i = %9d, value = %ld, expected = %d, lock = %d)\n",
                    me, i, e->data, me + prev + i, e->lock);
        }

        timebuf& tbuf = timebufs[idx++];
        tbuf.target = target;
        tbuf.fad    = t1 - t0;
        tbuf.flush0 = t2 - t1;
        tbuf.get    = t3 - t2;
        tbuf.flush1 = t4 - t3;
        tbuf.put    = t5 - t4;
        tbuf.flush2 = t6 - t5;
        tbuf.put2   = t7 - t6;
        tbuf.flush3 = t8 - t7;
        tbuf.barrier= t9 - t8;
    }

    if (debug) printf("SEND DONE\n");

    MPI_Barrier(comm);

    char filename[1024];
    sprintf(filename, "one_sided.%05d.out", me);

    FILE *fp = fopen(filename, "w");

    for (int i = 0; i < n_msgs; i++) {
        const timebuf& tbuf = timebufs[i];
        fprintf(fp, 
                "pid = %9d, id = %9d, target = %9d, "
                "fad = %9ld, flush0 = %9ld, "
                "get = %9ld, flush1 = %9ld, "
                "put = %9ld, flush2 = %9ld, "
                "put2 = %9ld, flush3 = %9ld, "
                "barrier = %9ld\n",
                me, i, tbuf.target,
                (long)tbuf.fad, (long)tbuf.flush0,
                (long)tbuf.get, (long)tbuf.flush1,
                (long)tbuf.put, (long)tbuf.flush2,
                (long)tbuf.put2,(long)tbuf.flush3,
                (long)tbuf.barrier);
    }

    fclose(fp);

    if (debug) printf("WRITE DONE\n");
}

int main(int argc, char **argv)
{
//    MPI_Init(&argc, &argv);
    gasnet_init(&argc, &argv);
    
    int me = gasnet_mynode();
    int n_procs = gasnet_nodes();

    char name[MPI_MAX_PROCESSOR_NAME];
    int len;
    MPI_Get_processor_name(name, &len);

    printf("me = %d, n_procs = %d, host = %s\n", me, n_procs, name);

    printf("GASNET_ALIGNED_SEGMENTS: %d\n", GASNET_ALIGNED_SEGMENTS);
    printf("GASNET_PSHM: %d\n", GASNET_PSHM);
 
    if (argc < 2) {
        if (me == 0)
            fprintf(stderr, "Usage: %s n_msgs\n", argv[0]);
        gasnet_exit(0);
    }
    int n_msgs = atoi(argv[1]);

    int segsize = 8192;
    int minheapoffset = 8192;
    gasnet_attach(NULL, 0, segsize, minheapoffset);

    gasnet_seginfo_t *seginfo = new gasnet_seginfo_t[n_procs];
    gasnet_getSegmentInfo(seginfo, n_procs);

    gasnet_seginfo_t *si = &seginfo[0];
    for (int i = 1; i < n_procs; i++) {
        if (si->addr != seginfo[i].addr)
            fprintf(stderr, "address error (%p, %p) in %d\n",
                    si->addr, seginfo[i].addr, i);
        if (si->size != seginfo[i].size)
            fprintf(stderr, "size error in %d\n", i);
    }

    printf("me = %d, start.\n", me);

    real_main(me, n_procs, n_msgs, si->addr, si->size);

    MPI_Barrier(MPI_COMM_WORLD);

    if (me == 0)
        printf("ALL DONE\n");

    gasnet_exit(0);
//    MPI_Finalize();
    return 0;
}

