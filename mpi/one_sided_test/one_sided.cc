#include "common.h"
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <vector>

#include <mpi.h>
#include <pthread.h>

struct entry {
    int lock;
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

static void real_main(int me, int n_procs, int n_msgs)
{
    MPI_Comm comm = MPI_COMM_WORLD;

    std::vector<timebuf> timebufs(n_msgs);
    memset(&timebufs[0], 0, sizeof(timebuf) * n_msgs);

    MPI_Barrier(comm);

    int next = (me + 1) % n_procs;
    int prev = (me + n_procs - 1) % n_procs;

    entry *e = new entry;
    entry *buf = new entry;

    MPI_Win win;
    MPI_Win_create(e, sizeof(entry), 1, MPI_INFO_NULL, comm, &win);
    MPI_Win_lock_all(0, win);

    if (debug) printf("INIT DONE\n");

    int idx = 0;
    for (int i = 0; i < n_msgs; i++) {

        int target = next;

        e->lock = 0;
        e->data = me;

        MPI_Win_flush(me, win);

        MPI_Barrier(MPI_COMM_WORLD);

        tsc_t t0 = rdtsc();

        int lockbuf = 256;
        int value = 1;
//        do {
            int r = MPI_Fetch_and_op(&value, &lockbuf, MPI_INT, target, 0,
                                     MPI_SUM, win);
            if (r != MPI_SUCCESS)
                fprintf(stderr, "%d: fech_and_add failed (i = %9d)\n",
                        me, i);

            tsc_t t1 = rdtsc();

            MPI_Win_flush(target, win);

            if (lockbuf != 0)
                fprintf(stderr,
                        "%d: fetch_and_add error (i = %9d, lockbuf = %d)\n",
                        me, i, lockbuf);
//        } while (lockbuf != 0);
        
        tsc_t t2 = rdtsc();

        MPI_Get(buf, sizeof(entry), MPI_BYTE,
                target, 0, sizeof(entry), MPI_BYTE, win);

        tsc_t t3 = rdtsc();

        MPI_Win_flush(target, win);

        tsc_t t4 = rdtsc();

        buf->data += me + i;
        MPI_Put(buf, sizeof(entry), MPI_BYTE,
                target, 0, sizeof(entry), MPI_BYTE, win);

        tsc_t t5 = rdtsc();

        MPI_Win_flush(target, win);

        tsc_t t6 = rdtsc();

        lockbuf = 0;
        MPI_Put(&lockbuf, 1, MPI_INT, target, 0, 1, MPI_INT, win);

        tsc_t t7 = rdtsc();

        MPI_Win_flush(target, win);

        tsc_t t8 = rdtsc();

        MPI_Barrier(comm);

        tsc_t t9 = rdtsc();

        if (e->data != me + prev + i || e->lock != 0) {
            fprintf(stderr, "%d: error (i = %9d, value = %ld, expected = %d)\n",
                    me, i, e->data, me + prev + i);
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

    MPI_Win_unlock_all(win);
    MPI_Win_free(&win);

    if (debug) printf("FREE DONE\n");

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
    MPI_Init(&argc, &argv);
    
    int me, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    printf("me = %d, n_procs = %d\n", me, n_procs);

    if (argc < 2) {
        if (me == 0)
            fprintf(stderr, "Usage: %s n_msgs\n", argv[0]);
        MPI_Finalize();
        exit(0);
    }
    int n_msgs = atoi(argv[1]);

    MPI_Barrier(MPI_COMM_WORLD);

    real_main(me, n_procs, n_msgs);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}

