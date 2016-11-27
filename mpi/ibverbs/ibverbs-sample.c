/*
 *  $ gcc -g -Wall -libverbs -o ibverbs-sample1 ibverbs-sample1.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <infiniband/verbs.h>
#include <mpi.h>

enum {
    PORT_NUM = 1,
};

char buffer[1024 * 1024];

static void test_run(struct ibv_context *context, uint16_t lid);
static void modify_qp(struct ibv_qp *qp, uint32_t src_psn, uint16_t dest_lid, uint32_t dest_pqn, uint32_t dest_psn);
static void post_recv(struct ibv_qp *qp, struct ibv_mr *mr, void *addr, uint32_t length);
static void post_send(struct ibv_qp *qp, struct ibv_mr *mr,
                      void *addr, uint32_t length);
static void post_fetch_and_add(struct ibv_qp *qp, struct ibv_mr *mr,
                               struct ibv_mr *target_mr,
                               void *p, void *result_ptr,
                               unsigned long value);

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int ret;

    ret = ibv_fork_init();
    if (ret) {
        fprintf(stderr, "Failure: ibv_fork_init (errno=%d)\n", ret);
        exit(EXIT_FAILURE);
    }

    struct ibv_device **dev_list;
    dev_list = ibv_get_device_list(NULL);

    if (!dev_list) {
        int errsave = errno;
        fprintf(stderr, "Failure: ibv_get_device_list (errno=%d)\n", errsave);
        exit(EXIT_FAILURE);        
    }

    if (dev_list[0]) {
        struct ibv_device *device = dev_list[0];

        printf("IB device: %s\n", ibv_get_device_name(device));

        struct ibv_context *context;
        context = ibv_open_device(device);
        assert(context);

        struct ibv_port_attr port_attr;
        ret = ibv_query_port(context, PORT_NUM, &port_attr);
        assert(ret == 0);
        assert(port_attr.lid != 0);

        test_run(context, port_attr.lid);

        ibv_close_device(context);
    }

    ibv_free_device_list (dev_list);

    MPI_Finalize();
    return 0;
}

int exchange_int(int *sendbuf, int target)
{
    int recvbuf;
    MPI_Sendrecv(sendbuf, 1, MPI_INT, target, 0,
                 &recvbuf, 1, MPI_INT, target, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return recvbuf;
}

void exchange_bytes(void *sendbuf, void *recvbuf, int size, int target)
{
    MPI_Sendrecv(sendbuf, size, MPI_BYTE, target, 0,
                 recvbuf, size, MPI_BYTE, target, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

static void
test_run(struct ibv_context *context, uint16_t lid)
{
    int me, n_procs;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &me);
    MPI_Comm_size(comm, &n_procs);

    assert(n_procs == 2);

    int i, j, ret;
    struct ibv_pd *pd;
    struct ibv_mr *mr;
    struct ibv_cq *cq;
    struct ibv_qp *qp;

    pd = ibv_alloc_pd(context);
    assert(pd);

    int access =
          IBV_ACCESS_LOCAL_WRITE
        | IBV_ACCESS_REMOTE_WRITE
        | IBV_ACCESS_REMOTE_READ
        | IBV_ACCESS_REMOTE_ATOMIC;
    mr = ibv_reg_mr(pd, buffer, sizeof(buffer), access);
    assert(mr);

    cq = ibv_create_cq(context, 64, NULL, NULL, 0);
    assert(cq);

    struct ibv_qp_init_attr qp_init_attr = {
        .qp_type    = IBV_QPT_RC,
        .send_cq    = cq,
        .recv_cq    = cq,
        .cap        = {
            .max_send_wr  = 32,
            .max_recv_wr  = 32,
            .max_send_sge =  1,
            .max_recv_sge =  1,
        },
        .sq_sig_all = 1, 
    };
    qp = ibv_create_qp(pd, &qp_init_attr);
    assert(qp);

    int target = (me + 1) % n_procs;
    int qp_num = qp->qp_num;
    int psn = rand() & 0xFFFFFF;

    int target_qp_num   = exchange_int(&qp_num, target);
    int target_psn      = exchange_int(&psn, target);

    int16_t target_lid;
    exchange_bytes(&lid, &target_lid, sizeof(target_lid), target);

    struct ibv_mr target_mr;
    exchange_bytes(mr, &target_mr, sizeof(target_mr), target);

    char *target_buffer = target_mr.addr;
//    char *buffer_ptr = buffer;
//    exchange_bytes(&buffer_ptr, &target_buffer, sizeof(target_buffer), target);

    printf("%d: lid             = %d\n", me, lid);
    printf("%d: target_lid      = %d\n", me, target_lid);
    printf("%d: qp_num          = %d\n", me, qp_num);
    printf("%d: psn             = %d\n", me, psn);
    printf("%d: target_qp_num   = %d\n", me, target_qp_num);
    printf("%d: target_psn      = %d\n", me, target_psn);
    printf("%d: mr.addr         = %p\n", me, mr->addr);
    printf("%d: target_mr.addr  = %p\n", me, target_mr.addr);

    // transit qp state
    modify_qp(qp, psn, target_lid, target_qp_num, target_psn);
  
    printf("%d: qp init done\n", me);

    int num_wr = 0;

    if (me == 1) {
        // post receive work request
        post_recv(qp, mr, buffer, 4096);
        num_wr += 1;
    }

    MPI_Barrier(comm);

    unsigned long fad_init_value = 188;
    unsigned long fad_add_value = 100;
    unsigned long *buf = (unsigned long *)(buffer + 8192);
    unsigned long *target_buf = (unsigned long *)(target_buffer + 8192);
    *buf = fad_init_value;

    if (me == 0) {
        // post send work request
        for (i=0 ; i<1000 ; i++)
            buffer[4096 + i] = rand();

        post_send(qp, mr, buffer + 4096, 1000);
        num_wr += 1;

        // post send fetch-and-add request
        post_fetch_and_add(qp, mr, &target_mr, target_buf, buf + 1,
                           fad_add_value);
        num_wr += 1;
    }

    // poll cq
    struct ibv_wc wc;
    while (num_wr > 0) {
        ret = ibv_poll_cq(cq, 1, &wc);

        if (ret == 0)
            continue; /* polling */

        if (ret < 0) {
            fprintf(stderr, "Failure: ibv_poll_cq\n");
            exit(EXIT_FAILURE);
        }
        
        if (wc.status != IBV_WC_SUCCESS) {
            fprintf(stderr, "Completion error: %s (%d)\n",
                    ibv_wc_status_str(wc.status), wc.status);
            exit(EXIT_FAILURE);
        }
    
        switch (wc.opcode) {
        case IBV_WC_SEND:
            printf("poll send wc: wr_id=0x%016" PRIx64 "\n", wc.wr_id);
            break;

        case IBV_WC_RECV:
            {
                int data_sum = 0;
                int i;

                for (i = 0; i < 4096 / sizeof(int); i++) {
                    int *buf = (int *)buffer;
                    data_sum += buf[i];
                }

                printf("poll recv wc: wr_id=0x%016" PRIx64 " byte_len=%u, "
                       "imm_data=0x%x, data_sum=%d\n",
                       wc.wr_id, wc.byte_len, wc.imm_data, data_sum);
            }
            break;

        case IBV_WC_FETCH_ADD:
            printf("poll fetch-and-add wc: wr_id=0x%016" PRIx64 "\n",
                   wc.wr_id);

            break;

        default:
            exit(EXIT_FAILURE);
        }

        num_wr--;        
    }

    fflush(stdout);
    MPI_Barrier(comm);

    // assertion for fetch-and-add
    if (me == 1) {
        printf("%d: *buf = %lu\n", me, *buf);
        assert(*buf == fad_init_value + fad_add_value);
    }
    if (me == 0) {
        printf("%d: *(buf + 1) = %lu\n", me, *(buf + 1));
        assert(*(buf + 1) == fad_init_value);
    }

    ibv_destroy_qp(qp);
//    ibv_destroy_qp(qp2);
//    ibv_destroy_qp(qp1);
    ibv_destroy_cq(cq);
    ibv_dereg_mr(mr);
    ibv_dealloc_pd(pd);

    printf("OK\n");
}

static void
modify_qp(struct ibv_qp *qp, uint32_t src_psn, uint16_t dest_lid, uint32_t dest_pqn, uint32_t dest_psn)
{
    int ret;

    int access =
          IBV_ACCESS_LOCAL_WRITE
        | IBV_ACCESS_REMOTE_WRITE
        | IBV_ACCESS_REMOTE_READ
        | IBV_ACCESS_REMOTE_ATOMIC;

    struct ibv_qp_attr init_attr = {
        .qp_state        = IBV_QPS_INIT,
        .port_num        = PORT_NUM,
        .qp_access_flags = access,
    };

    ret = ibv_modify_qp(qp, &init_attr,
                        IBV_QP_STATE|IBV_QP_PKEY_INDEX|IBV_QP_PORT|IBV_QP_ACCESS_FLAGS);
    assert(ret == 0);

    struct ibv_qp_attr rtr_attr = {
        .qp_state               = IBV_QPS_RTR,
        .path_mtu               = IBV_MTU_4096,
        .dest_qp_num            = dest_pqn,
        .rq_psn                 = dest_psn,
        .max_rd_atomic          = 16,
        .max_dest_rd_atomic     = 16,
        .min_rnr_timer          = 0,
        .ah_attr                = {
            .is_global          = 0,
            .dlid               = dest_lid,
            .sl                 = 0,
            .src_path_bits      = 0,
            .port_num           = PORT_NUM,
        },
    };

    ret = ibv_modify_qp(qp, &rtr_attr,
                        IBV_QP_STATE|IBV_QP_AV|IBV_QP_PATH_MTU|IBV_QP_DEST_QPN|
                        IBV_QP_RQ_PSN|IBV_QP_MAX_DEST_RD_ATOMIC|IBV_QP_MIN_RNR_TIMER);
    assert(ret == 0);

    struct ibv_qp_attr rts_attr = {
        .qp_state           = IBV_QPS_RTS,
        .timeout            = 0,
        .retry_cnt          = 7,
        .rnr_retry          = 7,
        .sq_psn             = src_psn,
        .max_rd_atomic      = 0,
    };

    ret = ibv_modify_qp(qp, &rts_attr,
                        IBV_QP_STATE|IBV_QP_TIMEOUT|IBV_QP_RETRY_CNT|IBV_QP_RNR_RETRY|IBV_QP_SQ_PSN|IBV_QP_MAX_QP_RD_ATOMIC);
    assert(ret == 0);
}

static void
post_recv(struct ibv_qp *qp, struct ibv_mr *mr, void *addr, uint32_t length)
{
    int ret;

    struct ibv_sge sge = {
        .addr   = (uint64_t)(uintptr_t)addr,
        .length = length,
        .lkey   = mr->lkey,
    };

    struct ibv_recv_wr recv_wr = {
        .wr_id   = (uint64_t)(uintptr_t)addr,
        .sg_list = &sge,
        .num_sge = 1,
    };

    struct ibv_recv_wr *bad_wr;
    ret = ibv_post_recv(qp, &recv_wr, &bad_wr);
    assert(ret == 0);

    printf("post recv wr\n");
}

static void
post_send(struct ibv_qp *qp, struct ibv_mr *mr,
          void *addr, uint32_t length)
{
    int i;
    int data_sum;
    int ret;

    struct ibv_sge sge = {
        .addr   = (uint64_t)(uintptr_t)addr,
        .length = length,
        .lkey   = mr->lkey,
    };

    struct ibv_send_wr send_wr = {
        .wr_id      = (uint64_t)(uintptr_t)addr,
        .sg_list    = &sge,
        .num_sge    = 1,
        .opcode     = IBV_WR_SEND_WITH_IMM,
        .imm_data   = rand(),
    };

    struct ibv_send_wr *bad_wr;
    ret = ibv_post_send(qp, &send_wr, &bad_wr);
    assert(ret == 0);

    data_sum = 0;
    for (i = 0; i < length / sizeof(int); i++) {
        int *buf = addr;
        data_sum += buf[i];
    }

    printf("post send wr: imm_data=0x%08x, byte_len=%u,"
           "data_sum=%d\n", send_wr.imm_data, length, data_sum);
}

static void
post_fetch_and_add(struct ibv_qp *qp, struct ibv_mr *mr,
                   struct ibv_mr *target_mr,
                   void *p, void *result_ptr, unsigned long value)
{
    int ret;
    struct ibv_sge sge = {
        .addr   = (uint64_t)(uintptr_t)result_ptr,
        .length = sizeof(unsigned long),
        .lkey   = mr->lkey,
    };

    struct ibv_send_wr send_wr = {
        .wr_id          = (uint64_t)(uintptr_t)p,
        .next           = NULL,
        .sg_list        = &sge,
        .num_sge        = 1,
        .opcode         = IBV_WR_ATOMIC_FETCH_AND_ADD,
        .send_flags     = IBV_SEND_FENCE,
        .wr             = {
            .atomic = {
                .remote_addr    = (uint64_t)(uintptr_t)p,
                .compare_add    = value,
                .rkey           = target_mr->rkey,
            }
        },
    };

    struct ibv_send_wr *bad_wr;
    ret = ibv_post_send(qp, &send_wr, &bad_wr);

    printf("post fetch-and-add wr: p=%p, value=%lu\n",
           (void *)send_wr.wr.atomic.remote_addr,
           value);

    if (ret != 0)
        printf("ibv_post_send: error '%s'\n", strerror(ret));
}

