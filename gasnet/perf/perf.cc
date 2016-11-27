#include <mpi.h>
#include <gasnet.h>
#include <pthread.h>
#include "common.h"
#include <cstring>
#include <cassert>
#include <vector>
#include <cmath>
#include <unistd.h>


int debug = 0;

void barrier()
{
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);            
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS); 
}

struct poll_thread_arg {
    bool done;
};

void * poll_thread_func(void *p)
{
    poll_thread_arg *arg = (poll_thread_arg *)p;

    while (!arg->done)
        gasnet_AMPoll();

    return nullptr;
}

void amhandle_rep(gasnet_token_t token, void *p, size_t size)
{
    printf("fuga");fflush(stdout);
    long *done = *(long **)p;
    *done = 1;
}

void amhandle_req(gasnet_token_t token, void *p, size_t size)
{
    printf("hoge");fflush(stdout);
    gasnet_AMReplyMedium0(token, 129, p, size);
}

void do_comm(int type, char *dst, char *src, size_t size, int target, int enable_thread)
{
    if (type == 0)
        gasnet_put(target, dst, src, size);
    if (type == 1)
        gasnet_get(dst, target, src, size);

    if (type == 2) {
        long done = 0;
        *(long **)src = &done;
        gasnet_AMRequestMedium0(target, 128, src, size);

        while (done == 0)
            if (!enable_thread)
                gasnet_AMPoll();
    }
}

struct timebuf {
    int type;
    int size;
    long time;
};

static void real_main(int me, int n_procs,
                      int type, int times, int max_size, int enable_thread,
                      char *segment, size_t size)
{
    if (me == 0 && n_procs != 2)
        die("n_procs must be 2");

    int target = (me + 1) % n_procs;

    pthread_t th;
    poll_thread_arg arg = { false };

    if (enable_thread)
        pthread_create(&th, nullptr, poll_thread_func, (void *)&arg);

    barrier();

    if (me == 0) {
        std::vector<timebuf> timebufs(times * 30);
        memset(timebufs.data(), 0, sizeof(timebuf) * timebufs.size());

        char *dst = segment;
        char *src = segment + max_size;

        int tbidx = 0;
        for (int size = 8; size <= max_size; size *= 2) {

            // pre-exec
            for (auto i = 0; i < 10; i++) {
                do_comm(type, dst, src, size, target, enable_thread);
            }

            for (auto i = 0; i < times; i++) {
                long t0 = rdtsc();

                do_comm(type, dst, src, size, target, enable_thread);

                long t1 = rdtsc();

                auto& tb = timebufs[tbidx++];
                tb.type = type;
                tb.size = size;
                tb.time = t1 - t0;
            }
        }

        FILE *f = fopen("perf.out", "w");

        for (auto i = 0; i < tbidx; i++) {
            auto& tb = timebufs[i];

            const char *type_str = "";
            if (tb.type == 0) type_str = "put";
            if (tb.type == 1) type_str = "get";

            fprintf(f, "type = %s, pollthread = %d, size = %7d, time = %ld\n",
                   type_str, enable_thread, tb.size, tb.time);
        }

        fclose(f);

        int idx = 0;
        for (int size = 8; size <= max_size; size *= 2) {
            long sumtime = 0;
            for (auto i = 0; i < times; i++) {
                auto& tb = timebufs[idx++];
                sumtime += tb.time;
            }
            long avgtime = sumtime / times;

            const char *type_str = "";
            if (type == 0) type_str = "put";
            if (type == 1) type_str = "get";

            printf("type = %s, pollthread = %d, size = %7d, time = %7ld\n",
                   type_str, enable_thread, size, avgtime);
        }
    }

    barrier();

    if (enable_thread) {
        arg.done = true;
        pthread_join(th, nullptr);
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
    if (argc <= 1) {
        if (me == 0)
            fprintf(stderr, "Usage: %s type times enable_thread max_size\n", argv[0]);
        gasnet_exit(0);
    }
    size_t n_args = 1;
    int type          = (argc >= n_args + 1) ? atoi(argv[n_args++]) : 0;
    int times         = (argc >= n_args + 1) ? atoi(argv[n_args++]) : 1000;
    int enable_thread = (argc >= n_args + 1) ? atoi(argv[n_args++]) : 0;
    int max_size      = (argc >= n_args + 1) ? atoi(argv[n_args++]) : 1 * 1024 * 1024;

    if (me == 0)
        printf("type = %d, times = %d, enable_thread = %d, max_size = %d\n",
               type, times, enable_thread, max_size);

    // attach segment
    gasnet_handlerentry_t entries[] = {
        { 128, (void (*)())amhandle_req },
        { 129, (void (*)())amhandle_rep },
    };
    int segsize = max_size * 2;
    int minheapoffset = 8192;
    gasnet_attach(entries, 2, segsize, minheapoffset);

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

    real_main(me, n_procs, type, times, max_size, enable_thread,
              (char *)si->addr, si->size);

    MPI_Barrier(comm);

    if (me == 0)
        printf("ALL DONE\n");

    if (!init)
        MPI_Finalize();

    gasnet_exit(0);
    return 0;
}

