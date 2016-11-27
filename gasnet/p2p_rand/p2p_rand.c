#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include <sys/time.h>

#include "topo.h"

enum {
    HANDLER_REQ = 128,
    HANDLER_REP,
};


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


#include <gasnet.h>

/* Macro to check return codes and terminate with useful message. */
#define GASNET_SAFE(fncall) do {                                     \
    int _retval;                                                     \
    if ((_retval = fncall) != GASNET_OK) {                           \
      fprintf(stderr, "ERROR calling: %s\n"                          \
                      " at: %s:%i\n"                                 \
                      " error: %s (%s)\n",                           \
              #fncall, __FILE__, __LINE__,                           \
              gasnet_ErrorName(_retval), gasnet_ErrorDesc(_retval)); \
      fflush(stderr);                                                \
      abort();                                                       \
      /*gasnet_exit(_retval);*/                                      \
    }                                                                \
  } while(0)

#define MAKEWORD(hi,lo)                                             \
    ((((uint64_t)(hi)) << 32) | (((uint64_t)(lo)) & 0xFFFFFFFF))
#define HIWORD(arg)     ((uint32_t)(((uint64_t)(arg)) >> 32))
#define LOWORD(arg)     ((uint32_t)((uint64_t)(arg)))


void comm_barrier(void)
{
    // sometimes gasnet_barrier_notify does not progress
    // in PSHM configuration (?).

    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    for (;;) {
        int result = gasnet_barrier_try(0, GASNET_BARRIERFLAG_ANONYMOUS);
        if (result == GASNET_OK)
            break;
        else if (result == GASNET_ERR_NOT_READY)
            ;
        else
            assert(0);
    }
}   


typedef struct msg {
    int *done;
} msg_t;

static void handler_req(gasnet_token_t token, void *buf, size_t nbytes)
{
    gasnet_hold_interrupts();
    gasnet_resume_interrupts();

    gasnet_AMReplyMedium0(token, HANDLER_REP, buf, nbytes);
}

static void handler_rep(gasnet_token_t token, void *buf, size_t nbytes)
{
    gasnet_hold_interrupts();
    gasnet_resume_interrupts();

    msg_t *msg = buf;
    *msg->done = 1;
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

typedef struct {
    size_t target;
    tsc_t comm;
    tsc_t wait;
} timebuf_t;

static void real_main(size_t me, size_t n_procs, 
                      size_t n_msgs, size_t wait, size_t data_size, 
                      const char *type)
{
    size_t i, j;

    srand((unsigned)rdtsc());

    int target_type = 0;
    if (strcmp(type, "rand") == 0)
        target_type = 0;
    else if (strcmp(type, "neighb") == 0)
        target_type = 1;
    else if (strcmp(type, "me") == 0)
        target_type = 2;

    timebuf_t *timebuf = calloc(sizeof(*timebuf), n_msgs);

    msg_t *msg = malloc(sizeof(msg_t) + data_size);
    memset(msg, 0, sizeof(msg_t) + data_size);

    int neighbs[6];
    if (target_type == 1)
        topo_neighbs(me, neighbs);

    for (i = 0; i < n_msgs; i++) {
        int target;
        if (target_type == 0) {
            do {
                target = random_int(n_procs);
            } while (target == me);
        } else if (target_type == 1) {
            target = topo_rand_neighb(neighbs, me);
        } else if (target_type == 2) {
            int node = topo_node_rank(me);
            do {
                target = node + random_int(n_procs <= 16 ? n_procs : 16);
            } while (target == me);
        }

        int done = 0;
        msg->done = &done;

        tsc_t t0 = rdtsc();

        gasnet_AMRequestMedium0(target, HANDLER_REQ, 
                                msg, sizeof(msg_t) + data_size);

        tsc_t t1 = rdtsc();
        
        while (!done)
            gasnet_AMPoll();

        tsc_t t2 = rdtsc();

        timebuf[i].target = target;
        timebuf[i].comm = t1 - t0;
        timebuf[i].wait = t2 - t1;

        for (j = 0; j < wait; j++)
            gasnet_AMPoll();
    }

    comm_barrier();

    char filename[PATH_MAX];
    sprintf(filename, "p2p_rand.%05d.out", me);

    FILE *fp = fopen(filename, "w");

    for (i = 0; i < n_msgs; i++) {
        fprintf(fp, 
                "pid = %6zu, id = %9zu, target = %9zu, "
                "comm = %9llu, wait = %9llu\n",
                me, i, timebuf[i].target, timebuf[i].comm, timebuf[i].wait);
    }

    fclose(fp);
}

int main(int argc, char **argv)
{
    {
        static gasnet_handlerentry_t handlers[] = {
            { HANDLER_REQ, handler_req },
            { HANDLER_REP, handler_rep },
        };

        GASNET_SAFE(gasnet_init(&argc, &argv));

        uintptr_t max_segsize = gasnet_getMaxLocalSegmentSize();
        uintptr_t minheapoffset = GASNET_PAGESIZE;

        uintptr_t segsize = max_segsize / GASNET_PAGESIZE * GASNET_PAGESIZE;

        GASNET_SAFE(gasnet_attach(handlers, 2,
                                  segsize, minheapoffset));
    }

    if (argc != 5) {
        fprintf(stderr, "Usage: %s n_msgs wait data_size [ rand | neighb | me ]\n", argv[0]);
        exit(1);
    }
    long n_msgs = atol(argv[1]);
    long wait = atol(argv[2]);
    long data_size = atol(argv[3]);
    const char *type = argv[4];

    size_t me = gasnet_mynode();
    size_t n_procs = gasnet_nodes();

    comm_barrier();

    real_main(me, n_procs, (size_t)n_msgs, wait, data_size, type);

    comm_barrier();

    gasnet_exit(0);
    return 0;
}
