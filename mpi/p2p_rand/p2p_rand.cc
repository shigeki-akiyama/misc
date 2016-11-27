#include "common.h"
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

#if __sparc__ && __arch64__
#include "topo.h"
#else
static void topo_neighbs(int rank, int neighbs[6]) {}
static int topo_rand_neighb(int neighbs[6], int rank) { return -1; }
static int topo_node_rank(int rank) { return -1; }
#endif


#include <mpi.h>
#include <pthread.h>
#include <errno.h>

#define COMM_THREAD 0
#define MULTITHREADED 1

using namespace std;

static int debug = 0;
static int deadlocking = 0;
static int g_recvcount = 0;
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
    tsc_t wait;
    tsc_t lock;
    tsc_t unlock;
};

#define MUTEX_PTHREAD       0
#define MUTEX_SPIN          1
#define MUTEX_SPIN_BACKOFF  2
#define MUTEX MUTEX_PTHREAD

#if MUTEX == MUTEX_PTHREAD
typedef pthread_mutex_t mutex_t;
static void mutex_init(mutex_t *mutex) { pthread_mutex_init(mutex, NULL); }
static void mutex_lock(mutex_t *mutex) { pthread_mutex_lock(mutex); }
static void mutex_unlock(mutex_t *mutex) { pthread_mutex_unlock(mutex); }
#elif MUTEX == MUTEX_SPIN
typedef pthread_spinlock_t mutex_t;
static void mutex_init(mutex_t *mutex) 
    { pthread_spin_init(mutex, PTHREAD_PROCESS_PRIVATE); }
static void mutex_lock(mutex_t *mutex) { pthread_spin_lock(mutex); }
static void mutex_unlock(mutex_t *mutex) { pthread_spin_unlock(mutex); }
#elif MUTEX == MUTEX_SPIN_BACKOFF
long spin(long t);

typedef pthread_spinlock_t mutex_t;
static void mutex_init(mutex_t *mutex)
    { pthread_spin_init(mutex, PTHREAD_PROCESS_PRIVATE); }
static void mutex_lock(mutex_t *mutex) {
    long max_factor = 32;
    long factor = 1;
    for (;;) {
        int r = pthread_spin_trylock(mutex);
        if (r == 0)
            break;

        assert(r == EBUSY);

        factor = factor << 1;
        if (factor > max_factor)
            factor = 1;

        spin(1000 * factor);
    }
}
static void mutex_unlock(mutex_t *mutex) {
    pthread_spin_unlock(mutex);
}
#else
#error "MUTEX is undefined"
#endif

struct process {
    struct pool {
        int n_actives;
        vector<MPI_Request> requests;
        vector<message *> buffers;
        vector<int> sources;
        bool is_recv;

        process& p;
        int tag;

        pool(process& p, int size, const vector<int> *sources, bool is_recv) :
            n_actives(0),
            requests(size, MPI_REQUEST_NULL),
            buffers(size, NULL),
            sources(size, MPI_ANY_SOURCE),
            is_recv(is_recv),
            p(p), tag(0) {

            for (size_t i = 0; i < buffers.size(); i++) {
                buffers[i] = new message;

                if (is_recv)
                    irecv(i);
            }

            if (sources != NULL) {
                assert(is_recv);
                this->sources = *sources;
            }

            if (is_recv)
                n_actives = requests.size();
        }

        ~pool() {
            for (size_t i = 0; i < buffers.size(); i++)
                delete buffers[i];
        }

        int get_free_buf() {
            int idx = MPI_UNDEFINED;

            int count = 0;
            for (;;) {
                for (int i = 0; i < (int)requests.size(); i++) {
                    if (requests[i] == MPI_REQUEST_NULL) {
                        idx = i;
                        break;
                    }
                }

                if (idx != MPI_UNDEFINED)
                    break;

                p.poll_internal();

                count++;
            }

            n_actives += 1;
            return idx;
        }

        void irecv(int idx) {
            assert(is_recv);
            assert(buffers[idx] != NULL);
            assert(requests[idx] == MPI_REQUEST_NULL);

            MPI_Irecv(buffers[idx], sizeof(message), MPI_BYTE, 
                      sources[idx], tag, p.comm, &requests[idx]);
        }

        void isend(const message& msg, int target) {
            assert(!is_recv);

            int idx = get_free_buf();

            *buffers[idx] = msg;
            MPI_Isend(buffers[idx], sizeof(message), MPI_BYTE, target, 
                      tag, p.comm, &requests[idx]);
        }

        void poll() {
            int n_completes;

            if (!is_recv) {
                MPI_Testsome(n_actives, requests.data(), 
                             &n_completes, p.tmp_completes.data(),
                             MPI_STATUSES_IGNORE);
            } else {
                // MPI_Testsome does not work with multiple Irecvs
                // because of Fujitsu MPI (OpenMPI?)'s implementation issue
                int complete, anyidx;
                MPI_Testany(n_actives, requests.data(), &anyidx, &complete,
                            MPI_STATUS_IGNORE);
                if (!complete)
                    n_completes = 0;
                if (anyidx == MPI_UNDEFINED)
                    n_completes = MPI_UNDEFINED;
                else {
                    n_completes = 1;
                    p.tmp_completes[0] = anyidx;
                }
            }

            if (n_completes == 0 || n_completes == MPI_UNDEFINED) {
                /* do nothing */
            } else {
                int prev_actives = n_actives;

                if (is_recv) {
                    for (int i = 0; i < n_completes; i++) {
                        int idx = p.tmp_completes[i];
                        assert(0 <= idx && idx < prev_actives);
                        assert(requests[idx] == MPI_REQUEST_NULL);

                        p.process_msg(*buffers[idx]);
                        irecv(idx);
                    }
                } else {
#if 1
                    for (int i = 0; i < n_completes; i++) {
                        int idx = p.tmp_completes[i];
                        assert(0 <= idx && idx < prev_actives);
                        assert(requests[idx] == MPI_REQUEST_NULL);

                        int idx2 = -1;
                        for (int j = prev_actives - 1; j >= 0; j--) {
                            if (requests[j] != MPI_REQUEST_NULL) {
                                idx2 = j;
                                break;
                            }
                        }

                        if (idx2 > idx) {
                            message *tmp = buffers[idx];
                        
                            requests[idx] = requests[idx2];
                            buffers[idx] = buffers[idx2];

                            requests[idx2] = MPI_REQUEST_NULL;
                            buffers[idx2] = tmp;

                        }

                        n_actives -= 1;
                    }
#else
                    // sort tmp_completes
                    for (int i = 1; i < n_completes; i++) {
                        int x = p.tmp_completes[i];

                        int j;
                        for (j = i; j > 0 && p.tmp_completes[j-1] > x; j--)
                            p.tmp_completes[j] = p.tmp_completes[j-1];

                        p.tmp_completes[j] = x;
                    }

                    for (int i = n_completes - 1; i >= 0; i--) {
                        int doneidx = p.tmp_completes[i];
                        int activeidx = n_actives - 1;
                        assert(0 <= doneidx && doneidx < n_actives);
                        assert(requests[doneidx] == MPI_REQUEST_NULL);

                        if (doneidx != activeidx) {
                            message *tmp = buffers[doneidx];

                            requests[doneidx] = requests[activeidx];
                            buffers[doneidx] = buffers[activeidx];

                            requests[activeidx] = MPI_REQUEST_NULL;
                            buffers[activeidx] = tmp;
                        }
                        n_actives -= 1;
                    }
#endif
                }
            }
        }
    };

    int me, n_procs;
    MPI_Comm comm;
    int barrier_state;

    pool sendpool;
    pool recvpool;
    pool replpool;

    vector<int> tmp_completes;

    mutex_t mutex;

#if MULTITHREADED
    void lock() { mutex_lock(&mutex); }
    void unlock() { mutex_unlock(&mutex); }
#else
    void lock() {}
    void unlock() {}
#endif

    int max(int i, int j, int k) { 
        if (i > j && i > k)
            return i;
        if (j > i && j > k)
            return j;
        else
            return k;
    }

    process(int me, int n_procs, 
            int n_sendreqs, int n_recvreqs, int n_replreqs, 
            const vector<int>& sources) :
        me(me), n_procs(n_procs), 
        comm(MPI_COMM_WORLD),
        barrier_state(0),
        sendpool(*this, n_sendreqs, NULL, false), 
        recvpool(*this, n_recvreqs, &sources, true),
        replpool(*this, n_replreqs, NULL, false),
        tmp_completes(max(n_sendreqs, n_recvreqs, n_replreqs)) {

        mutex_init(&mutex);
    }

    void isend_internal(const message& msg, int target) {
        return sendpool.isend(msg, target);
    }

    void isend(const message& msg, int target) {
        lock();
        isend_internal(msg, target);
        unlock();
    }

    void ireply_internal(const message&msg, int target) {
        return replpool.isend(msg, target);
    }

    void poll_internal() {
        recvpool.poll();
        replpool.poll();
        sendpool.poll();
    }

    void poll() {
        lock();
        poll_internal();
        unlock();
    }

    void wait(MPI_Request *req) {
        for (;;) {
            int complete;
            lock();
            MPI_Test(req, &complete, MPI_STATUS_IGNORE);
            unlock();

            if (complete)
                break;

            poll_internal();
        }
    }

    void barrier() {
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
        for (i = 0; i < 2; i++)
            if (children[i] < n_procs) {
                lock();
                MPI_Irecv(&dummy, 1, MPI_INT, children[i],
                          barriertag, comm, &reqs[i]);
                unlock();
            }
        for (i = 0; i < 2; i++)
            if (children[i] < n_procs)
                wait(&reqs[i]);

        if (parent != -1) {
            lock();
            MPI_Isend(&dummy, 1, MPI_INT, parent, barriertag, comm, &req);
            unlock();
            wait(&req);
        }

        // broadcast 
        if (parent != -1) {
            lock();
            MPI_Irecv(&dummy, 1, MPI_INT, parent, barriertag, comm, &req);
            unlock();
            wait(&req);
        }

        for (i = 0; i < 2; i++)
            if (children[i] < n_procs) {
                lock();
                MPI_Isend(&dummy, 1, MPI_INT, children[i],
                          barriertag, comm, &reqs[i]);
                unlock();
            }
        for (i = 0; i < 2; i++)
            if (children[i] < n_procs)
                wait(&reqs[i]);

        barrier_state = !barrier_state;
    }
 
    void process_msg(const message& msg)
    {
        if (msg.phase == 0) {
            message msgbuf;

            msgbuf.pid = me;
            msgbuf.id = msg.id;
            msgbuf.phase = 1;
            msgbuf.done = msg.done;

            if (debug) {
                fprintf(g_fp, "%05d: reply %05d to %05d\n",
                        me, msg.id, msg.pid);
                fflush(g_fp);
            }

            ireply_internal(msgbuf, msg.pid);
        } else if (msg.phase == 1) {

            if (debug) {
                fprintf(g_fp, "%05d: recv %05d from %05d\n",
                        me, msg.id, msg.pid);
                fflush(g_fp);
            }

            *msg.done = 1;

            g_recvcount++;
        } else {
            assert(0);
        }
    }
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

static void warmup(int me, int pid, int n_procs, int tid, int n_threads, 
                   process& p, const vector<int>& targets)
{
    int n_warmups = 100;

    for (auto target : targets) {
        for (auto i = 0; i < n_warmups; i++) {
            int done = 0;
            message msg;
            msg.pid = pid;
            msg.id = i;
            msg.phase = 0;
            msg.done = &done;

            p.isend(msg, target);

            while (!done)
                p.poll();
        }
    }
}
 

static void real_main(int pid, int n_procs, int tid, int n_threads,
                      process& p, int n_msgs, int wait, int target_type,
                      const vector<int>& targets, 
                      int *neighbs, int do_warmup)
{
    int i;
    int me = pid * n_threads + tid;

    if (debug) {
        char debugname[1024];
        sprintf(debugname, "hoge.%02d.debug", me);

        g_fp = fopen(debugname, "w");
    }

    vector<timebuf> timebufs(n_msgs);

    if (debug) {
        fprintf(g_fp, "%5d: start\n", me); fflush(g_fp);
    }

    if (targets.size() > 0 && do_warmup) {
        warmup(me, pid, n_procs, tid, n_threads, p, targets);
    }

    int idx = 0;
    for (i = 0; i < n_msgs; i++) {
        int target;
        if (targets.size() <= 0) {
            target = choose_target(me, pid, n_procs, target_type, neighbs);
        } else {
            int idx = random_int(targets.size());
            target = targets[idx];
        }

        int done = 0; 
        message msg;
        msg.pid = pid;
        msg.id = i;
        msg.phase = 0;
        msg.done = &done;

        tsc_t t0 = rdtsc();

        p.lock();

        tsc_t t1 = rdtsc();

        if (debug) {
            fprintf(g_fp, "%05d: send %05d to %5d\n", me, i, target);
            fflush(g_fp);
        }

        p.isend_internal(msg, target);

        if (debug) {
            fprintf(g_fp, "%05d: send %05d to %5d done\n", me, i, target);
            fflush(g_fp);
        }

        tsc_t t2 = rdtsc();

        int count = 0;
        while (!done) {
            p.poll_internal();

            if (++count % 1000000 == 0) {
                fprintf(stdout, 
                        "%5d: wait polls 100000 times "
                        "for a message %d to %d, recvcount = %d\n", 
                        me, i, target, g_recvcount);

                deadlocking = 1;
            }
        }

        tsc_t t3 = rdtsc();

        p.unlock();

        tsc_t t4 = rdtsc();

        timebuf& tbuf = timebufs[idx++];
        tbuf.target = target;
        tbuf.comm = t2 - t1;
        tbuf.wait = t3 - t2;
        tbuf.lock = t1 - t0;
        tbuf.unlock = t4 - t3;

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

    char filename[1024];
    sprintf(filename, "p2p_rand.%05d.out", me);

    FILE *fp = fopen(filename, "w");

    for (i = 0; i < n_msgs; i++) {
        const timebuf& tbuf = timebufs[i];
        fprintf(fp, 
                "pid = %9d, id = %9d, target = %9zu, "
                "comm = %9ld, wait = %9ld, lock = %9ld, unlock = %9ld\n",
                me, i, tbuf.target, (long)tbuf.comm, (long)tbuf.wait,
                (long)tbuf.lock, (long)tbuf.unlock);
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
    int target_type;
    const vector<int> *targets;
    int *neighbs;
    int do_warmup;
};

static void *thread_start(void *p)
{
    targument arg = *reinterpret_cast<targument *>(p);

    real_main(arg.pid, arg.n_procs, arg.tid, arg.n_threads,
              *arg.proc, arg.n_msgs, arg.wait, arg.target_type, *arg.targets,
              arg.neighbs,
              arg.do_warmup);
    return NULL;
}

static void start_threads(int n_threads, int pid, int n_procs,
                          process& p, int n_msgs, int wait, 
                          int target_type, const vector<int>& targets,
                          int *neighbs,
                          int do_warmup)
{
    pthread_t th[16];
    targument args[16];
    assert(n_threads <= 16);

    for (int i = 1; i < n_threads; i++) {
        targument arg = { pid, n_procs, i, n_threads,
                          &p, n_msgs, wait, target_type, &targets, neighbs,
                          do_warmup };
        args[i] = arg;
        pthread_create(&th[i], NULL, thread_start, &args[i]);
    }
    targument arg = { pid, n_procs, 0, n_threads,
                      &p, n_msgs, wait, target_type, &targets, neighbs,
                      do_warmup };
    thread_start(&arg);

    for (int i = 1; i < n_threads; i++) {
        pthread_join(th[i], NULL);
    }

    p.barrier();

    MPI_Barrier(MPI_COMM_WORLD);
}

static void make_targets(int pid, int n_procs, int n_recvreqs, 
                         int target_type, int *neighbs, int max_targets, 
                         vector<int>& targets,
                         vector<int>& sources)
{
    assert(max_targets <= n_recvreqs);

    srand(rdtsc());
    for (auto i = 0; i < max_targets; i++) {
        auto begin = targets.begin();
        auto end = targets.end();
        int target;
        do {
            target = choose_target(pid, pid, n_procs, target_type, neighbs);
        } while (find(begin, end, target) != end);
        targets.push_back(target);
    }

    MPI_Comm comm = MPI_COMM_WORLD;

    vector<int> alltargets(max_targets * n_procs);
    MPI_Allgather(targets.data(), targets.size(), MPI_INT,
                  alltargets.data(), targets.size(), MPI_INT,
                  comm);
#if 0
    for (auto target : targets)
        printf("%05d: target = %5d\n", pid, target);
#endif

#if 1
    for (auto i = 0; i < n_procs; i++) {
        for (auto j = 0; j < max_targets; j++) {
            auto idx = i * max_targets + j;
            auto target = alltargets[idx];

            if (target == pid) {
                sources.push_back(i);
                break;
            }
        }
    }

    for (auto target : targets) {
        if (find(sources.begin(), sources.end(), target) == sources.end())
            sources.push_back(target);
    }
#else
//    for (auto i = 0; i < n_procs; i++)
//        sources.push_back(i);
//    for (auto i = 0; i < n_recvreqs; i++)
//        sources.push_back(MPI_ANY_SOURCE);
#endif
#if 0
    for (auto source : sources)
        printf("%05d: source = %5d\n", pid, source);
#endif
}

int main(int argc, char **argv)
{
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
    
    int pid, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    if (provided != MPI_THREAD_SERIALIZED && pid == 0) {
        fprintf(stderr, "MPI does not support MPI_THREAD_SERIALIZED\n");
    } 

    int min_arg_count = 3;
    if (argc < min_arg_count) {
        if (pid == 0)
            fprintf(stderr, 
                    "Usage: %s n_msgs [ rand | neighb | me ] max_targets "
                    "do_warmup n_threads "
                    "wait n_sendreqs n_recvreqs n_replreqs\n", argv[0]);
        MPI_Finalize();
        exit(0);
    }
    int idx = 1;
    int n_msgs = atoi(argv[idx++]);
    const char *type = argv[idx++];
    assert(idx == min_arg_count);

    int max_targets = (argc <= idx) ? 0 : atoi(argv[idx++]);
    int do_warmup   = (argc <= idx) ? 1 : atoi(argv[idx++]);
    int n_threads   = (argc <= idx) ? 1 : atoi(argv[idx++]);
    int wait        = (argc <= idx) ? 0 : atoi(argv[idx++]);
    int n_sendreqs  = (argc <= idx) ? 4 : atoi(argv[idx++]);
    int n_recvreqs  = (argc <= idx) ? 4 : atoi(argv[idx++]);
    int n_replreqs  = (argc <= idx) ? 4 : atoi(argv[idx++]);

    printf("n_msgs = %d, ", n_msgs);
    printf("type   = %s, ", type);
    printf("max_targets = %d, ", max_targets);
    printf("do_warmup = %d, ", do_warmup);
    printf("n_threads = %d, ", n_threads);
    printf("wait   = %d, ", wait);
    printf("n_sendreqs = %d, ", n_sendreqs);
    printf("n_recvreqs = %d, ", n_recvreqs);
    printf("n_replreqs = %d, ", n_replreqs);
    printf("\n");

    if (n_threads >= 2 && provided != MPI_THREAD_SERIALIZED) {
        fprintf(stderr, 
                "n_threads = %d is not supported in this program.\n",
                n_threads);
        exit(0);
    }

    int target_type = 0;
    if (strcmp(type, "rand") == 0)
        target_type = 0;
    else if (strcmp(type, "neighb") == 0)
        target_type = 1;
    else if (strcmp(type, "me") == 0)
        target_type = 2;

    // multithreaded version only supports `rand' configuration.
    if (n_threads >= 2)
        target_type = 0;

    {
        int neighbs[6];
        if (target_type == 1)
            topo_neighbs(pid, neighbs);

         vector<int> targets;
         vector<int> sources;
         if (1) {
             // do not use MPI_ANY_SOURCE
             make_targets(pid, n_procs, n_recvreqs, 
                          target_type, neighbs, max_targets, 
                          targets, sources);
             n_recvreqs = sources.size();
        } else {
            if (max_targets >= 1) {
                for (auto i = 0; i < max_targets; i++) {
                    auto begin = targets.begin();
                    auto end = targets.end();
                    int target;
                    do {
                        target = choose_target(pid, pid, n_procs, 
                                               target_type, neighbs);
                    } while (find(begin, end, target) != end);
                    targets.push_back(target);
                }
            }

            for (auto i = 0; i < n_recvreqs; i++) {
                sources.push_back(MPI_ANY_SOURCE);
            }
        }

        process p(pid, n_procs, n_sendreqs, n_recvreqs, n_replreqs, sources);

        srand(rdtsc());

        MPI_Barrier(MPI_COMM_WORLD);

#if MULTITHREADED
        start_threads(n_threads, pid, n_procs, p, n_msgs, wait, target_type,
                      targets, neighbs, do_warmup);
#else
        real_main(pid, n_procs, 0, 1, p, n_msgs, wait, target_type, targets,
                  neighbs, do_warmup);
#endif
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}

