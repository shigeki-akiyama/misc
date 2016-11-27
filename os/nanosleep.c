#include "../common/common.h"

int main(int argc, char **argv) {

    if (argc != 2) { fprintf(stderr, "argument error\n"); exit(1); }

    int ns = atoi(argv[1]);

    const struct timespec rqtp = { 0, ns };
    struct timespec rmtp = { 0, 0 };
    tsc_t tsc0 = rdtsc();
    int result = nanosleep(&rqtp, &rmtp);
    tsc_t tsc1 = rdtsc();
    if (result != 0) { fprintf(stderr, "signal trapped\n"), exit(1); }

    printf("nanosleep time:\n");
    printf("rqtp = { %llu, %ld }\n", (uint64_t)rqtp.tv_sec, rqtp.tv_nsec);
    printf("rmtp = { %llu, %ld }\n", (uint64_t)rmtp.tv_sec, rmtp.tv_nsec);
    printf("rdtsc = %lld\n", tsc1 - tsc0);

    return 0;
}
