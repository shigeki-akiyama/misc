#include "common.h"
#include "topo.h"
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <vector>

#include <mpi.h>
#include <pthread.h>


using namespace std;

static int debug = 0;
static FILE *g_fp = NULL;

struct message {
    int pid;
    int id;
    int phase;
    int *done;
};

struct timebuf {
    size_t target;
    tsc_t comm;
    tsc_t fence;
    tsc_t lock;
    tsc_t unlock;
};

static int choose_target(int me, int pid, int n_procs, int target_type, 
                         int neighbs[]) {
    int target;

    if (target_type == 0) {
        do {
            target = random_int(n_procs);
        } while (target == pid);
    } else if (target_type == 1) {
        target = topo_rand_neighb(neighbs, me);
    } else if (target_type == 2) {
        int node = topo_node_rank(me);
        do {
            target = node + random_int(n_procs <= 16 ? n_procs : 16);
        } while (target == me);
    }

    return target;
}

static void warmup(int me, int pid, int n_procs,
                   vector<int>& targets, 
                   MPI_Win win, void *data, void *buf, 
                   size_t data_size, int comm_type)
{
    int n_warmups = 100;

    for (auto target : targets) {
        for (auto i = 0; i < n_warmups; i++) {
            MPI_Win_lock(MPI_LOCK_SHARED, target, 0, win);

            if (comm_type == 0)
                MPI_Put(data, 0, MPI_BYTE, target, 0, data_size, MPI_BYTE, 
                        win);
            else if (comm_type == 1)
                MPI_Get(buf,  0, MPI_BYTE, target, 0, data_size, MPI_BYTE, 
                        win);
            else if (comm_type == 2)
                MPI_Accumulate(data, 0, MPI_BYTE, 
                               target, 0, data_size, MPI_BYTE, MPI_REPLACE, 
                               win);

            MPI_Win_unlock(target, win);
        }
    }
}

static void real_main(int me, int n_procs, 
                      int n_msgs, int wait, 
                      int comm_type, int target_type, int max_targets,
                      int do_warmup)
{
    int i;
    MPI_Comm comm = MPI_COMM_WORLD;

    if (debug) {
        char debugname[1024];
        sprintf(debugname, "hoge.%02d.debug", me);

        g_fp = fopen(debugname, "w");
    }

    srand(rdtsc());

    vector<timebuf> timebufs(n_msgs);

    MPI_Barrier(comm);

    if (debug) {
        fprintf(g_fp, "%5d: start\n", me); fflush(g_fp);
    }

    size_t data_size = 8;
    char data[8], buf[8];
    memset(data, 0, data_size);
    memset(buf, 0, data_size);

    MPI_Win win;
    MPI_Win_create(data, data_size, 1, MPI_INFO_NULL, comm, &win);

    int neighbs[6];
    if (target_type == 1)
        topo_neighbs(me, neighbs);

    vector<int> targets(max_targets);
    for (auto& t : targets) {
        t = choose_target(me, me, n_procs, target_type, neighbs);
    }

    if (max_targets > 0 && do_warmup) {
        warmup(me, me, n_procs, targets, win, data, buf, data_size, 
               comm_type);
    }

    int idx = 0;
    for (i = 0; i < n_msgs; i++) {
        int target;
        if (max_targets <= 0) {
            target = choose_target(me, me, n_procs, target_type, neighbs);
        } else {
            int idx = random_int(max_targets);
            target = targets[idx];
        }

        if (debug) {
            fprintf(g_fp, "%05d: send %05d to %5d\n", me, i, target);
            fflush(g_fp);
        }

        tsc_t t0 = rdtsc();

        MPI_Win_lock(MPI_LOCK_SHARED, target, 0, win);

        tsc_t t1 = rdtsc();

        if (comm_type == 0)
            MPI_Put(data, 0, MPI_BYTE, target, 0, data_size, MPI_BYTE, win);
        else if (comm_type == 1)
            MPI_Get(buf,  0, MPI_BYTE, target, 0, data_size, MPI_BYTE, win);
        else if (comm_type == 2)
            MPI_Accumulate(data, 0, MPI_BYTE, 
                           target, 0, data_size, MPI_BYTE, MPI_SUM, win);

        tsc_t t2 = rdtsc();

        MPI_Win_unlock(target, win);

        tsc_t t3 = rdtsc();

        if (debug) {
            fprintf(g_fp, "%05d: send %05d to %5d done\n", me, i, target);
            fflush(g_fp);
        }

        tsc_t t4 = t3;
//        MPI_Win_fence(0, win);

//        tsc_t t4 = rdtsc();

        timebuf& tbuf = timebufs[idx++];
        tbuf.target = target;
        tbuf.comm = t2 - t1;
        tbuf.fence = t4 - t3;
        tbuf.lock = t1 - t0;
        tbuf.unlock = t3 - t2;

        if (debug && i % 100 == 0) {
            fprintf(stdout, "%5d: sent %dth message\n", me, i);
        }

        if (debug) {
            fprintf(g_fp, "----------------\n");
            fflush(g_fp);
        }
    }

    if (debug) {
        fprintf(g_fp, "%5d: send done\n", me);fflush(g_fp);
    }

    MPI_Win_free(&win);

    MPI_Barrier(comm);

    if (debug) {
        fprintf(g_fp, "%5d: barrier done\n", me);fflush(g_fp);
    }

    char filename[1024];
    sprintf(filename, "one_sided.%05d.out", me);

    FILE *fp = fopen(filename, "w");

    for (i = 0; i < n_msgs; i++) {
        const timebuf& tbuf = timebufs[i];
        fprintf(fp, 
                "pid = %9d, id = %9d, target = %9zu, "
                "comm = %9ld, fence = %9ld, "
                "lock = %9ld, unlock = %9ld\n",
                me, i, tbuf.target, (long)tbuf.comm, (long)tbuf.fence,
                (long)tbuf.lock, (long)tbuf.unlock);
    }

    fclose(fp);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    
    int me, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

   if (argc != 7) {
        if (me == 0)
            fprintf(stderr, 
                    "Usage: %s n_msgs wait"
                    "[ put | get | acc ] [ rand | neighb | me ]"
                    "max_targets do_warmup\n", argv[0]);
        MPI_Finalize();
        exit(0);
    }
    int n_msgs = atoi(argv[1]);
    int wait = atoi(argv[2]);
    const char *ctype = argv[3];
    const char *ttype = argv[4];
    int max_targets = atoi(argv[5]);
    int do_warmup = atoi(argv[6]);

    int comm_type = 0;
    if (strcmp(ctype, "put") == 0)
        comm_type = 0;
    else if (strcmp(ctype, "get") == 0)
        comm_type = 1;
    else if (strcmp(ctype, "acc") == 0)
        comm_type = 2;

    int target_type = 0;
    if (strcmp(ttype, "rand") == 0)
        target_type = 0;
    else if (strcmp(ttype, "neighb") == 0)
        target_type = 1;
    else if (strcmp(ttype, "me") == 0)
        target_type = 2;

    MPI_Barrier(MPI_COMM_WORLD);

    real_main(me, n_procs, n_msgs, wait, comm_type, target_type,
              max_targets, do_warmup);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}

