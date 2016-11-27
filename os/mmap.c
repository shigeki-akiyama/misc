#include "common.h"

#include <sys/mman.h>

#ifdef USE_MPI
#include <mpi.h>
#endif

int main(int argc, char **argv)
{
#ifdef USE_MPI
    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    size_t i;

    if (argc != 2) die("argument error");

    size_t size = atoll(argv[1]);

    {
        void *p = mmap(NULL, size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANON,
                       -1, 0);
        if (p == NULL) die("mmap");
        int result = munmap(p, size);
        if (result != 0) die("munmap");
    }

    tsc_t tsc_mmap0 = rdtsc();
    uint8_t *p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON,
                   -1, 0);
    tsc_t tsc_mmap1 = rdtsc();
    if (p == NULL) die("mmap");

    tsc_t tsc_clear0 = rdtsc();
    for (i = 0; i < size; i += 4096)
        p[i] = 0;
    tsc_t tsc_clear1 = rdtsc();

    tsc_t tsc_munmap0 = rdtsc();
    int result = munmap(p, size);
    tsc_t tsc_munmap1 = rdtsc();
    if (result != 0) die("munmap");

    printf("mmap(size = %zd): %lld clocks\n",
           size, tsc_mmap1 - tsc_mmap0);
    printf("clear(size = %zd): %lld clocks\n",
           size, tsc_clear1 - tsc_clear0);
    printf("munmap(size = %zd): %lld clocks\n",
           size, tsc_munmap1 - tsc_munmap0);

#ifdef USE_MPI
    MPI_Finalize();
#endif
    return 0;
}

