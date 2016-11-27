#include <mpi.h>
#include <mpi-ext.h>
#include <assert.h>
#include <sys/mman.h>

#include "common.h"

typedef uint64_t u64;

int main(int argc, char **argv)
{
    int i;

    MPI_Init(&argc, &argv);
    FJMPI_Rdma_init();

    int me, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    int src = 0;
    int dst = 1;

    int buf_len = 1 * 1024 * 1024; //8192;
    int buf_size = buf_len * sizeof(int);

#define USE_MMAP 1
#if USE_MMAP
    int *buf = (int *)mmap((void *)0x40000000000, buf_size,
                           PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
                           -1, 0);
    assert(buf != MAP_FAILED);
    assert(buf == (int *)0x40000000000);
#else
    int *buf = new int[buf_len];
#endif
    memset(buf, 0, buf_size);

    u64 t_reg_0 = rdtsc();

    int mid = 0;
    u64 laddr = FJMPI_Rdma_reg_mem(mid, buf, buf_size);

    u64 t_reg_1 = rdtsc();

    u64 t_remote_reg_0 = rdtsc();

    u64 raddr;
    do {
        raddr = FJMPI_Rdma_get_remote_addr(dst, mid);
    } while (raddr == FJMPI_RDMA_ERROR);

    printf("# me = %d, dst = %d, mid = %d, buf = %p, "
           "laddr = 0x%lx, raddr = 0x%lx\n", 
           me, dst, mid, buf, laddr, raddr);

    u64 t_remote_reg_1 = rdtsc();

    MPI_Barrier(MPI_COMM_WORLD);

    int data_size = 128;

    for (i = 0; i < 20; i++) {
        if (me == src) {
            int flags = FJMPI_RDMA_LOCAL_NIC0
                | FJMPI_RDMA_REMOTE_NIC0
                | FJMPI_RDMA_REMOTE_NOTICE;

            u64 t_put_0 = rdtsc();

            FJMPI_Rdma_put(dst, 0, raddr, laddr, data_size, flags);

            u64 t_put_1 = rdtsc();

            u64 t_wait_0 = rdtsc();

            // wait local completion
            struct FJMPI_Rdma_cq cq;
            for (;;) {
                int result = FJMPI_Rdma_poll_cq(FJMPI_RDMA_NIC0, &cq);

                if (result == FJMPI_RDMA_NOTICE)
                    break;
            }

            u64 t_wait_1 = rdtsc();

            MPI_Barrier(MPI_COMM_WORLD);

            printf("reg = %10d, remote_reg = %10d, put = %10d, "
                   "local_wait = %10d\n",
                   t_reg_1 - t_reg_0,
                   t_remote_reg_1 - t_remote_reg_0,
                   t_put_1 - t_put_0,
                   t_wait_1 - t_wait_0);

            MPI_Barrier(MPI_COMM_WORLD);
        }
        if (me == dst) {
            u64 t_wait_0 = rdtsc();

            // wait remote completion
            struct FJMPI_Rdma_cq cq;
            for (;;) {
                int result = FJMPI_Rdma_poll_cq(FJMPI_RDMA_NIC0, &cq);

                if (result == FJMPI_RDMA_HALFWAY_NOTICE)
                    break;
            }

            u64 t_wait_1 = rdtsc();

            MPI_Barrier(MPI_COMM_WORLD);

            printf("remote wait = %10d\n", t_wait_1 - t_wait_0);

            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

//    FJMPI_Rdma_dereg_mem(mid);

    FJMPI_Rdma_finalize();
    MPI_Finalize();
    return 0;
}
