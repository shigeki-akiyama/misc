#include "ibv.h"
#include <memory>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cinttypes>
#include <cstring>
#include <cassert>
#include <sys/mman.h>
#include <mpi.h>

void madi_exit(int exitcode)
{
    exit(exitcode);
}

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

void test_put(madi::ibv::prepinned_endpoint& ep,
              uint8_t *buffer, size_t buffer_size, size_t times)
{
    auto me = ep.pid();
    auto n_procs = ep.n_procs();
    auto next = (me + 1) % n_procs;
    auto prev = (me + n_procs - 1) % n_procs;

    memset(buffer, 255, buffer_size);

    ep.barrier();

    std::vector<uint8_t *> buffers(n_procs);
    ep.allgather(buffer, &buffers[0]);

    auto data_size = buffer_size / 2;
    auto data = buffer + data_size;
    memset(data, me, data_size);

    printf("buffer = %p, data = %p\n",
           buffer, data);

    ep.rdma_put(buffers[next], data, data_size, next);

    ep.barrier();

    for (auto i = 0UL; i < data_size; i++) {
        if (buffer[i] != prev) {
            printf("error: buffer[%zu] = %zu (correct: %zu)\n",
                   i, (size_t)buffer[i], prev);
            exit(1);
        }
    }
}

void test_fetch_and_add(madi::ibv::prepinned_endpoint& ep,
                        uint8_t *buffer, size_t buffer_size, size_t times)
{
    auto me = ep.pid();
    auto n_procs = ep.n_procs();
    auto target = 0UL;

    auto lock           = (uint64_t *)(buffer + 0);
    auto lock_result    = (uint64_t *)(buffer + sizeof(uint64_t));

    *lock = 0;

    auto *lock_buf = lock;
    ep.broadcast(&lock_buf, 0);

    ep.barrier();

    for (auto i = 0UL; i < times; i++)
        ep.rdma_fetch_and_add(lock_buf, lock_result, 1, target);

    ep.barrier();

    if (me == target) {
        auto result = *lock;

        if (result == times * n_procs)
            printf("centralized atomic count: OK (%" PRIu64 ")\n", result);
        else
            printf("centralized atomic count: NG (%" PRIu64 ")\n", result);
    }
}

void real_main(madi::ibv::prepinned_endpoint& ep,
               uint8_t *buffer, size_t buffer_size, size_t times)
{
    test_put(ep, buffer, buffer_size, times);
    //test_fetch_and_add(ep, buffer, buffer_size, times);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int me;
    MPI_Comm_rank(MPI_COMM_WORLD, &me);

    if (argc < 3) {
        if (me == 0)
            fprintf(stderr, "Usage: %s times reg_size\n", argv[0]);
        MPI_Finalize();
        return 0;
    }

    int times = atoi(argv[1]);
    int buffer_size = atoi(argv[2]);

    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_ANONYMOUS | MAP_PRIVATE;
    void *buffer = (uint8_t *)mmap(NULL, buffer_size, prot, flags, -1, 0);
    {
        madi::ibv::environment env;

#if 1
        madi::ibv::prepinned_endpoint ep(env, buffer, buffer_size);
        real_main(ep, (uint8_t *)buffer, buffer_size, times);
#else
        endpoint ep(env);
        real_main(ep, (uint8_t *)buffer, buffer_size, times);
#endif
    }
    munmap(buffer, buffer_size);

    MPI_Finalize();
    return 0;
}

