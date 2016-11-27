#include <mpi.h>
#include <vector>
#include <deque>
#include <cstdint>
#include <cassert>
#include <sys/time.h>
using namespace std;

#include "barrier.h"

#define FJMPI
#ifdef FJMPI
#include <mpi-ext.h>
#endif

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

static int random_int(int n)
{
    if (n == 0)
        return 0;

    size_t rand_max =
        ((size_t)RAND_MAX + 1) - ((size_t)RAND_MAX + 1) % (size_t)n;
    int r;
    do {
       r = rand();
    } while ((size_t)r >= rand_max);

    return (int)((double)n * (double)r / (double)rand_max);
}

struct message {
    uint64_t raddr;
    uint64_t laddr;
    size_t size;
    int pid;
};

#ifdef FJMPI
class send_pool {
    int tag_;
    size_t n_sending_;
    size_t n_max_sending_;
    deque<message> msg_queue_;

    int n_nics_;
    int send_flags_[4];

public:
    size_t n_sents;
    size_t n_notices;

    send_pool(int tag, size_t n_nics, size_t n_max_sends) :
        tag_(tag),
        n_sending_(0), n_max_sending_(n_max_sends),
        msg_queue_(),
        n_nics_(n_nics),
        n_sents(0), n_notices(0)
    {
        send_flags_[0] = FJMPI_RDMA_LOCAL_NIC0 | FJMPI_RDMA_REMOTE_NIC0;
        send_flags_[1] = FJMPI_RDMA_LOCAL_NIC1 | FJMPI_RDMA_REMOTE_NIC1;
        send_flags_[2] = FJMPI_RDMA_LOCAL_NIC2 | FJMPI_RDMA_REMOTE_NIC2;
        send_flags_[3] = FJMPI_RDMA_LOCAL_NIC3 | FJMPI_RDMA_REMOTE_NIC3;
    }

    ~send_pool()
    {
    }

    size_t n_sending() { return n_sending_; }

    void do_send(uint64_t raddr, uint64_t laddr, size_t size, int pid)
    {
        int idx = pid % n_nics_;

        FJMPI_Rdma_put(pid, tag_, raddr, laddr, size, 
                       send_flags_[idx] | FJMPI_RDMA_REMOTE_NOTICE);

        n_sending_ += 1;
        n_sents += 1;
    }

    void send(uint64_t raddr, uint64_t laddr, size_t size, int pid)
    {
        if (n_sending_ <= n_max_sending_) {
            do_send(raddr, laddr, size, pid);
        } else {
            message msg = { raddr, laddr, size, pid };
            msg_queue_.push_back(msg);
        }
    }

    bool handle(int r, int tag, int pid)
    {
        if (tag != tag_)
            return false;

        if (r == FJMPI_RDMA_NOTICE) {
            n_sending_ -= 1;
            n_notices += 1;

            if (!msg_queue_.empty()) {
                message msg = msg_queue_.front();
                msg_queue_.pop_front();

                do_send(msg.raddr, msg.laddr, msg.size, msg.pid);
            }

            return true;
        } else {
            return false;
        }
    }
};

struct comm {
    int me_;
    int n_procs_;
    bool is_server_;
    void *buf;
    size_t buf_size_;
    uint64_t laddr_;
    vector<uint64_t> raddrs_;

    vector<int> nics_;
    int n_nics_;
    int nic_idx_;

    int send_tag_;
    int reply_tag_;

    send_pool spool_;

    size_t n_sending_;

    long prev_poll_time_;

    bool exited_;

    size_t n_sends;
    size_t n_notices;

    comm(int me, int n_procs, size_t n_nics,
         size_t n_max_sends, size_t n_max_replies,
         bool is_server) : 
        me_(me), n_procs_(n_procs), is_server_(is_server),
        buf(NULL), buf_size_(sizeof(int)),
        laddr_(0), raddrs_(n_procs_),
        nics_(4), n_nics_(n_nics), nic_idx_(0),
        send_tag_(12), reply_tag_(13),
        spool_(send_tag_, n_nics, n_max_sends),
        n_sending_(0),
        prev_poll_time_(0),
        exited_(false),
        n_sends(0), n_notices(0)
    {
        MPI_Comm cw = MPI_COMM_WORLD;

        buf = malloc(buf_size_);

        int memid = 0;
        laddr_ = FJMPI_Rdma_reg_mem(memid, buf, buf_size_);

        MPI_Barrier(cw);

        for (int pid = 0; pid < n_procs_; pid++) {
            uint64_t addr = FJMPI_Rdma_get_remote_addr(pid, memid);
            assert(addr != FJMPI_RDMA_ERROR);
            raddrs_[pid] = addr;
        }

        nics_[0] = FJMPI_RDMA_NIC0;
        nics_[1] = FJMPI_RDMA_NIC1;
        nics_[2] = FJMPI_RDMA_NIC2;
        nics_[3] = FJMPI_RDMA_NIC3;
    }

    ~comm()
    {
    }

    bool exited() { return exited_; }

    int poll_cq(int *tag, int *pid)
    {
        int nic_idx = nic_idx_;
        for (int i = nic_idx; i < nic_idx + n_nics_; i++) {
            int idx = i % n_nics_;

            FJMPI_Rdma_cq cq;
            int r = FJMPI_Rdma_poll_cq(nics_[idx], &cq);

            if (r == FJMPI_RDMA_HALFWAY_NOTICE ||
                r == FJMPI_RDMA_NOTICE) {
                *tag = cq.tag;
                *pid = cq.pid;
                nic_idx_ = (idx + 1) % n_nics_;
                return r;
            }
        }

        nic_idx_ = 0;
        return 0;
    }

    bool poll(long *polltime, long *proctime, long *waittime, long *reptime)
    {
        long t0 = rdtsc();
#if 0
        long poll_interval = (long)(1000.0 * random_int(100) / 100);
        if (t0 - prev_poll_time_ < poll_interval)
            return false;
#endif
        prev_poll_time_ = t0;

        int tag = -1, pid = -1;
        int r = poll_cq(&tag, &pid);

        long t1 = rdtsc();

        bool ok = spool_.handle(r, tag, pid);

        if (!ok) {
            if (r == FJMPI_RDMA_HALFWAY_NOTICE) {
                if (is_server_) {
                    if (tag == 0) {
                        exited_ = true;
                    } else {
                        // reply
                        spool_.send(raddrs_[pid], laddr_, buf_size_, pid);
                    }
                } else {
                    n_sending_ -= 1;
                }
            }
        }

        long t2 = rdtsc();

        if (polltime != NULL) {
            *polltime = t1 - t0;
            *proctime = t2 - t1;
        }

        return (r != 0);
    }

    bool poll() { return poll(NULL, NULL, NULL, NULL); }

    void send(int pid)
    {
        spool_.send(raddrs_[pid], laddr_, buf_size_, pid);
        n_sending_ += 1;
    }

    void wait()
    {
        while (n_sending_ > 0)
            poll(); 
    }

    void send_exited(int pid)
    {
        int tag = 0;
        FJMPI_Rdma_put(pid, tag, raddrs_[pid], laddr_, buf_size_,
                       FJMPI_RDMA_LOCAL_NIC0 | FJMPI_RDMA_REMOTE_NIC0 
                       | FJMPI_RDMA_REMOTE_NOTICE);
    }

    void barrier()
    {
        while (!barrier_try())
            poll();
    }
};
#endif

struct timebuf {
    size_t target;
    tsc_t comm;
    tsc_t wait;
};

struct polltimebuf {
    int process;
    long polltime;
    long proctime;
    long waittime;
    long reptime;
};

int real_main(int argc, char **argv)
{
    MPI_Comm cw = MPI_COMM_WORLD;

    int me, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    int arg_idx = 1;
    int server_mod  = (argc >= arg_idx + 1) ? atoi(argv[arg_idx++]) : 16;
    int n_msgs      = (argc >= arg_idx + 1) ? atoi(argv[arg_idx++]) : 10000;
    int n_waits     = (argc >= arg_idx + 1) ? atoi(argv[arg_idx++]) : 0;
    int n_nics      = (argc >= arg_idx + 1) ? atoi(argv[arg_idx++]) : 4;
    int n_max_sends = (argc >= arg_idx + 1) ? atoi(argv[arg_idx++]) : 16;
    int n_max_repls = (argc >= arg_idx + 1) ? atoi(argv[arg_idx++]) : 16;

    if (me == 0) {
        printf("server_mod  = %d\n", server_mod);
        printf("n_msgs      = %d\n", n_msgs);
        printf("n_waits     = %d\n", n_waits);
        printf("n_nics      = %d\n", n_nics);
        printf("n_max_sends = %d\n", n_max_sends);
        printf("n_max_repls = %d\n", n_max_repls);
        fflush(stdout);
    }

    bool is_server = (me % server_mod == 0);

    comm c(me, n_procs, n_nics, n_max_sends, n_max_repls, is_server);

    vector<timebuf> timebufs(n_msgs);
    memset(&timebufs[0], 0, sizeof(timebuf) * n_msgs);
    int idx = 0;

    size_t n_pollbufs = 5 * 1000 * 1000;
    vector<polltimebuf> polltimebufs(n_pollbufs);
    memset(&polltimebufs[0], 0, sizeof(polltimebuf) * n_pollbufs);

    if (is_server) {
        while (!c.exited()) {
            long polltime = 0, proctime = 0, waittime = 0, reptime = 0;
            bool process = c.poll(&polltime, &proctime,
                                  &waittime, &reptime);

            if (process) {
                polltimebuf tbuf = {
                    process, polltime, proctime, waittime, reptime
                };
                polltimebufs[idx++] = tbuf;
            }
        }
    } else {
        for (int i = 0; i < n_msgs; i++) {
            int target;
            do {
                target = random_int(n_procs);
            } while (target == me);

            target = target / server_mod * server_mod;

            tsc_t t0 = rdtsc();

            c.send(target);

            tsc_t t1 = rdtsc();

            c.wait();

            tsc_t t2 = rdtsc();

            long t = rdtsc();
            while (rdtsc() - t < n_waits)
                ;

            timebuf& tbuf = timebufs[idx++];
            tbuf.target = target;
            tbuf.comm = t1 - t0;
            tbuf.wait = t2 - t1;
            
            if (0) {
                if (i % 1000 == 999) {
                    printf("%04d: i = %d (n_sents = %zu, n_notices = %zu)\n",
                           me, i, c.spool_.n_sents, c.spool_.n_notices);
                    fflush(stdout);
                }
            }
        }

        if (me == 1) {
            for (int pid = 0; pid < n_procs; pid += server_mod) {
                printf("%04d: send done to %d "
                       "(n_sents = %zu, n_notices = %zu)\n",
                       me, pid, c.spool_.n_sents, c.spool_.n_notices);
                fflush(stdout);

                c.send_exited(pid);
            }
        }
    }

    if (1)
        c.barrier();
    else
        MPI_Barrier(cw);

    if (0) {
        printf("%04d: termination done\n", me);
        fflush(stdout);
    }

    char filename[1024];
    sprintf(filename, "rcas.%05d.out", me);

    FILE *fp = fopen(filename, "w");

    if (is_server) {
        for (size_t i = 0; i < idx; i++) {
            auto& tbuf = polltimebufs[i];
            fprintf(fp, 
                    "pid = %9d, id = %9zu, process = %d, "
                    "poll = %9ld, proc = %9ld "
                    "wait = %9ld, repl = %9ld\n",
                    me, i, (int)tbuf.process,
                    tbuf.polltime, tbuf.proctime,
                    tbuf.waittime, tbuf.reptime);
        }
    } else {
        for (int i = 0; i < n_msgs; i++) {
            const timebuf& tbuf = timebufs[i];
            fprintf(fp, 
                    "pid = %9d, id = %9d, target = %9zu, "
                    "comm = %9ld, wait = %9ld\n",
                    me, i, tbuf.target, (long)tbuf.comm, (long)tbuf.wait);
        }
    }

    fclose(fp);

    MPI_Barrier(cw);
    return 0;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    FJMPI_Rdma_init();
    barrier_init();

    srand((unsigned)rdtsc());
    real_main(argc, argv);

    barrier_finalize();
    FJMPI_Rdma_finalize();
    MPI_Finalize();
    return 0;
}

