#include "common.h"

#include <sys/mman.h>



int main(int argc, char **argv) {

    size_t i;
    for (i = 63; i >= 12; i--) {
        size_t size = 1ull << i;
        void *p = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                       MAP_PRIVATE | MAP_ANON, // | MAP_STACK,
                       -1, 0);
        if (p != MAP_FAILED) break;
    }

    size_t max_bit = i;
    printf("max size in mmap: %llu bytes (%zd bit)\n", 
           1ull << max_bit, max_bit);

    return 0;
}

