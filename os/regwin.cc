#include "common.h"
#include <vector>
#include <cstring>

using namespace std;

int f(vector<long>& timebufs, long *t, int idx, int depth)
{
    long t2 = rdtsc();

    timebufs[idx] = t2 - *t;

    if (depth == 0) {
        *t = rdtsc();
        return idx + 1;
    } else {
        long t3 = rdtsc();

        int idx2 = f(timebufs, &t3, idx + 1, depth - 1);
       
        long t4 = rdtsc();
        timebufs[idx2] = t4 - t3;

        *t = rdtsc();
        return idx2 + 1;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s depth\n", argv[0]);
        return 0;
    }

    int depth = atoi(argv[1]);

    vector<long> timebufs(depth * 2);
    memset(timebufs.data(), 0, sizeof(long) * depth * 2);

    long t = rdtsc();
    f(timebufs, &t, 0, depth);

    for (auto i = 0; i < depth * 2; i++) {
        auto t = timebufs[i];

        printf("id = %d, t = %ld\n", i, t);
    }

    return 0;
}

