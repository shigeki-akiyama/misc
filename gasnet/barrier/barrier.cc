#include "common.h"
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <vector>
#include <tuple>

#include <gasnet.h>
#include <mpi.h>
#include <pthread.h>
#include <errno.h>

using namespace std;

static int debug = 0;
static FILE *g_fp = NULL;

static void dprintf(const char *format, ...) {
    va_list arg;

    if (debug) {
        va_start(arg, format);
        vfprintf(g_fp, format, arg);
        va_end(arg);
    }
}

struct timebuf {
    int type;
    tsc_t time;
    tsc_t comm;
    tsc_t wait;
};

struct process {
    int me, n_procs;
    MPI_Comm comm;
    int barrier_state;

    process(int me, int n_procs) :
        me(me), n_procs(n_procs), 
        comm(MPI_COMM_WORLD),
        barrier_state(0) {
    }

    void poll_internal() {
    }

    void wait(MPI_Request *req) {
        for (;;) {
            int complete;
            MPI_Test(req, &complete, MPI_STATUS_IGNORE);

            if (complete)
                break;

            poll_internal();
        }
    }

    void tree_barrier(tsc_t *t_comm, tsc_t *t_wait) {
        int i;
        int dummy;
        MPI_Request req, reqs[2];

        int barriertag = 100 + barrier_state;
        MPI_Comm comm = MPI_COMM_WORLD;

        int parent = (me == 0) ? -1 : (me - 1) / 2;

        int children[2] = {
            2 * me + 1,
            2 * me + 2,
        };

        // reduce
        tsc_t t0 = rdtsc();
        for (i = 0; i < 2; i++)
            if (children[i] < n_procs)
                MPI_Irecv(&dummy, 1, MPI_INT, children[i],
                          barriertag, comm, &reqs[i]);
        tsc_t t1 = rdtsc();
        for (i = 0; i < 2; i++)
            if (children[i] < n_procs)
                wait(&reqs[i]);
        tsc_t t2 = rdtsc();

        tsc_t t3 = rdtsc();
        tsc_t t4 = t3;
        tsc_t t5 = t3;
        if (parent != -1) {
            MPI_Isend(&dummy, 1, MPI_INT, parent, barriertag, comm, &req);
            t4 = rdtsc();
            wait(&req);
            t5 = rdtsc();
        }

        // broadcast 
        tsc_t t6 = t5;
        tsc_t t7 = t5;
        if (parent != -1) {
            MPI_Irecv(&dummy, 1, MPI_INT, parent, barriertag, comm, &req);
            t6 = rdtsc();
            wait(&req);
            t7 = rdtsc();
        }

        for (i = 0; i < 2; i++)
            if (children[i] < n_procs)
                MPI_Isend(&dummy, 1, MPI_INT, children[i],
                          barriertag, comm, &reqs[i]);
        tsc_t t8 = rdtsc();
        for (i = 0; i < 2; i++)
            if (children[i] < n_procs)
                wait(&reqs[i]);
        tsc_t t9 = rdtsc();

        barrier_state = !barrier_state;

        *t_comm = (t1 - t0) + (t4 - t3) + (t6 - t5) + (t8 - t7);
        *t_wait = (t2 - t1) + (t5 - t4) + (t7 - t6) + (t9 - t8);
    }
    void tree_barrier() {
        tree_barrier(NULL, NULL);
    }

    void mpi_barrier() {
        MPI_Barrier(comm);
    }
};

void comm_barrier(tsc_t *t_try, tsc_t *t_wait)
{
    // sometimes gasnet_barrier_notify does not progress
    // in PSHM configuration (?).

    tsc_t t0 = rdtsc();
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    tsc_t t1 = rdtsc();
    for (;;) {
        int result = gasnet_barrier_try(0, GASNET_BARRIERFLAG_ANONYMOUS);
        if (result == GASNET_OK)
            break;
        else if (result == GASNET_ERR_NOT_READY)
            ;
        else
            assert(0);
    }
    tsc_t t2 = rdtsc();

    if (t_try != NULL)
        *t_try = t1 - t0;
    if (t_wait != NULL)
        *t_wait = t2 - t1;
}   

static void real_main(int pid, int n_procs, int tid, int n_threads,
                      process& p, int count, int type, int do_warmup)
{
    int i;
    int me = pid * n_threads + tid;

    if (debug) {
        char debugname[1024];
        sprintf(debugname, "hoge.%02d.debug", me);

        g_fp = fopen(debugname, "w");
    }

    vector<timebuf> timebufs(count);

    if (debug) {
        fprintf(g_fp, "%5d: start\n", me); fflush(g_fp);
    }

    if (do_warmup) {
        for (i = 0; i < 100; i++) {
            switch (type) {
                case 0: p.mpi_barrier(); break;
                case 1: comm_barrier(NULL, NULL); break;
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    int idx = 0;
    for (i = 0; i < count; i++) {
        tsc_t t_comm = 0;
        tsc_t t_wait = 0;

        tsc_t t0 = rdtsc();

        switch (type) {
            case 0: p.mpi_barrier(); break;
            case 1: comm_barrier(&t_comm, &t_wait); break;
        }

        tsc_t t1 = rdtsc();

        timebuf& tbuf = timebufs[i];
        tbuf.type = type;
        tbuf.time = t1 - t0;
        tbuf.comm = t_comm;
        tbuf.wait = t_wait;

        if (i % 100 == 0)
            dprintf("%5d: sent %dth message\n", me, i);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    char filename[1024];
    sprintf(filename, "barrier.%05d.out", me);

    FILE *fp = fopen(filename, "w");

    for (i = 0; i < count; i++) {
        const timebuf& tbuf = timebufs[i];
        fprintf(fp, 
                "pid = %9d, id = %9d, type = %d, "
                "time = %9ld, comm = %9ld, wait = %9ld\n",
                me, i, tbuf.type, (long)tbuf.time, 
                (long)tbuf.comm, (long)tbuf.wait);
    }

    fclose(fp);
}

struct targument {
    int pid;
    int n_procs;
    int tid;
    int n_threads;
    process *proc;
    int n_msgs;
    int wait;
    int type;
};

#if 0
static void *thread_start(void *p)
{
    targument arg = *reinterpret_cast<targument *>(p);

    real_main(arg.pid, arg.n_procs, arg.tid, arg.n_threads,
              *arg.proc, arg.n_msgs, arg.wait, arg.type);
    return NULL;
}

static void start_threads(int n_threads, int pid, int n_procs,
                          process& p, int n_msgs, int wait, 
                          int type)
{
    pthread_t th[16];
    targument args[16];
    assert(n_threads <= 16);

    printf("me = _, pid = %d, n_procs = %d, tid = _, n_threads = %d\n",
           pid, n_procs, n_threads);

    for (int i = 1; i < n_threads; i++) {
        targument arg = { pid, n_procs, i, n_threads,
                          &p, n_msgs, wait, type };
        args[i] = arg;
        pthread_create(&th[i], NULL, thread_start, &args[i]);
    }
    targument arg = { pid, n_procs, 0, n_threads,
                      &p, n_msgs, wait, type };
    thread_start(&arg);

    for (int i = 1; i < n_threads; i++) {
        pthread_join(th[i], NULL);
    }

    p.barrier();

    MPI_Barrier(MPI_COMM_WORLD);
}
#endif

int main(int argc, char **argv)
{
    {
        static gasnet_handlerentry_t handlers[] = {
        };

        gasnet_init(&argc, &argv);

        uintptr_t max_segsize = gasnet_getMaxLocalSegmentSize();
        uintptr_t minheapoffset = GASNET_PAGESIZE;

        uintptr_t segsize = max_segsize / GASNET_PAGESIZE * GASNET_PAGESIZE;

        gasnet_attach(handlers, 0,
                                  segsize, minheapoffset);
    }

    int pid, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

   if (argc != 4) {
        if (pid == 0)
            fprintf(stderr, 
                    "Usage: %s count [ mpi | gasnet ]\n",
                    argv[0]);
        MPI_Finalize();
        exit(0);
    }
    int count = atoi(argv[1]);
    const char *type_str = argv[2];
    int do_warmup = atoi(argv[3]);

    auto type_map = { 
        make_tuple("mpi", 0),
        make_tuple("gasnet", 1),
    };

    int type = 0;
    for (auto t : type_map) {
        if (strcmp(type_str, get<0>(t)) == 0) {
            type = get<1>(t);
            break;
        }
    }

    {
        process p(pid, n_procs);

        srand(rdtsc());

        MPI_Barrier(MPI_COMM_WORLD);

        real_main(pid, n_procs, 0, 1, p, count, type, do_warmup);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    gasnet_exit(0);
    return 0;
}

