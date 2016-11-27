#include <mpi.h>

#define FJMPI
#ifdef FJMPI
#include <mpi-ext.h>
#endif

int me;
int n_procs;
volatile int *buf = NULL;
uint64_t *bufs = NULL; 
int barrier_state = 0;
int phase = 0;
int phase0_idx = 0;

void barrier_init()
{
    MPI_Comm cw = MPI_COMM_WORLD;
   
    MPI_Comm_rank(cw, &me);
    MPI_Comm_size(cw, &n_procs);

    size_t n_elems = 2 * 2 + 1;
    size_t buf_size = sizeof(int) * n_elems;

    buf = new int[n_elems];

    for (size_t i = 0; i < n_elems; i++)
        buf[i] = 0;

    int memid = 510;
    uint64_t laddr = FJMPI_Rdma_reg_mem(memid, (void *)buf, buf_size);

    MPI_Barrier(cw);

    bufs = new uint64_t[n_procs];

    for (int pid = 0; pid < n_procs; pid++) {
        uint64_t addr = FJMPI_Rdma_get_remote_addr(pid, memid);
        assert(addr != FJMPI_RDMA_ERROR);
        bufs[pid] = addr;
    }
    bufs[me] = laddr;

    MPI_Barrier(cw);
}

void barrier_finalize()
{
    MPI_Comm cw = MPI_COMM_WORLD;
    MPI_Barrier(cw);

#if 0
    int memid = 510;
    FJMPI_Rdma_dereg_mem(memid);
#endif

    delete [] bufs;
    delete buf;
}

static void barrier_put(int pid, int idx, int value)
{
    // tmp buf
    buf[4] = value;

    int tag = 14;
    uint64_t raddr = bufs[pid] + sizeof(int) * idx;
    uint64_t laddr = bufs[me] + sizeof(int) * 4;
    size_t length = sizeof(int);
    int flags = 0;
    FJMPI_Rdma_put(pid, tag, raddr, laddr, length, flags);

    // FIXME: wait completion
    // while (FJMPI_Rdma_poll_cq(FJMPI_RDMA_NIC0, NULL);
}

bool barrier_try()
{
    int parent = (me == 0) ? -1 : (me - 1) / 2;
    int parent_idx = (me == 0) ? -1 : (me - 1) % 2;

    int children[2] = {
        2 * me + 1,
        2 * me + 2,
    };

    int base = 2 * barrier_state;

    // reduce
    if (phase == 0) {
        for (int i = phase0_idx; i < 2; i++) {
            if (children[i] < n_procs) {
                if (buf[base + i] != 1) {
                    phase = 0;
                    phase0_idx = i;
                    return false;
                }
            }
        }
        if (parent != -1) {
            //        dputs("reduce %d send to %d\n", me, parent);
            barrier_put(parent, base + parent_idx, 1);
            //        dputs("reduce %d send to %d done\n", me, parent);
        }
        phase += 1;
    }

    // broadcast
    if (phase == 1) {
        if (parent != -1) {
            if (buf[base + 0] != 2) {
                phase = 1;
                return false;
            }
        }
        for (int i = 0; i < 2; i++) {
            int child = children[i];
            if (children[i] < n_procs) {
                //            dputs("brdcst %d send to %d\n", me, parent);
                barrier_put(child, base + 0, 2);
                //            dputs("brdcst %d send to %d done\n", me, parent);
            }
        }
    }

    phase = 0;
    phase0_idx = 0;

    buf[base + 0] = 0;
    buf[base + 1] = 0;

    barrier_state = !barrier_state;

    return true;
}

void barrier()
{
    while (!barrier_try())
        ;
}

