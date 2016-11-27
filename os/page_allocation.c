#include "common.h"

#include <sys/mman.h>

int main(void) {

    const size_t page_size = 4096;
    size_t size = 128 * 1024 * 1024;

    int *p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON,
                   -1, 0);
    size_t n = size / page_size / sizeof(int);
    size_t i;
    for (i = 0; i < n; i++) {
        tsc_t tsc0 = rdtsc();
        p[page_size * i] = 9210;
        tsc_t tsc1 = rdtsc();

        printf("page allocation time: %lld clocks\n", tsc1 - tsc0);
    }

    for (i = 0; i < n; i++) {
        size_t idx = rand() % n;
        tsc_t tsc0 = rdtsc();
        int value = p[page_size * idx];
        tsc_t tsc1 = rdtsc();
        if (value != 9210) die("error");

        printf("TLB miss time: %lld clocks, %zd\n", tsc1 - tsc0, idx);
    }

    int result = munmap(p, size);
    if (result != 0) die("munmap");



    return 0;
}

