#include <stdio.h>
#include <stdint.h>
#include "common.h"

int main() {
    size_t i, j;

    const size_t N_TIMES = 32 * 1024 * 1024;

    for (i = 1; i <= 32; i++) {
        double t0 = now();
        uint64_t value = UINT64_MAX;
        for (j = 0; j < N_TIMES; j++)
            value /= i;

        double t1 = now();

        uint64_t value1 = UINT64_MAX;
        for (j = 0; j < N_TIMES; j++) {
            size_t count = __builtin_popcountl(i);
            if (count == 1) {
                size_t bits = __builtin_ctzl(i);
                value1 >>= bits;
            } else {
                value1 /= i;
            }
        }

        double t2 = now();

        printf("divide by %5zu: %.9f (%zu), %.9f (%zu)\n", 
               i, (t1 - t0) / N_TIMES, value, (t2 - t1) / N_TIMES, value1);
    }

    return 0;
}

