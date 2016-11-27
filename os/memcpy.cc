#include "common.h"
#include <vector>
#include <cstdio>
#include <cstring>

struct timebuf {
    size_t size;
    long cache_hit;
    long cache_miss;
};

int main()
{
    size_t n_times = 1000;

    timebuf tb = { 0, 0 };
    std::vector<timebuf> timebufs(n_times, tb);

    size_t buf_size = 64 * 1024 * 1024;

    std::vector<char> dst(buf_size, 0);
    std::vector<char> src(buf_size, 0);
//     char *dst = new char[buf_size];
//     char *src = new char[buf_size];
//     memset(dst, 1, buf_size);
//     memset(src, 2, buf_size);

    std::vector<char> cachespill(buf_size, 3);

    size_t idx = 0;
    for (size_t size = 1; size <= buf_size; size *= 2) {

        // load data to cache
        memcpy(&dst[0], &src[0], size);

        long t0 = rdtsc();

        memcpy(&dst[0], &src[0], size);

        long t1 = rdtsc();

        // evict data from cache
        memset(&cachespill[0], 0, buf_size);

        long t2 = rdtsc();

        memcpy(&dst[0], &src[0], size);

        long t3 = rdtsc();

        timebuf& t = timebufs[idx++];
        t.size = size;
        t.cache_hit = t1 - t0;
        t.cache_miss = t3 - t2;
    }

    for (size_t i = 0; i < idx; i++) {
        timebuf& t = timebufs[i];
        printf("size = %9ld, cache_hit_copy = %9ld, cache_miss_copy = %9ld\n",
               t.size, t.cache_hit, t.cache_miss);
    }
   
    return 0;
}
