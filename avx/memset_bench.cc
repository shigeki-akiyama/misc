
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdint>

#include <immintrin.h>

static size_t rdtsc()
{
#if defined(_MSC_VER)
    return __rdtsc();
#else
    uint32_t hi, lo;
    //asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    asm volatile("lfence\nrdtsc" : "=a"(lo), "=d"(hi));
    return uint64_t(hi) << 32 | lo;
#endif
}

#ifdef __AVX__
static void memset_avx_naive(void * dst, int c, size_t size)
{
    auto p = static_cast<__m256i *>(dst);
    auto p_end = p + size / sizeof(__m256i);

    auto vvalue = _mm256_set1_epi8(c);
    while (p < p_end) {
        _mm256_store_si256(p, vvalue);
        p += 1;
    }
}

static void memset_avx_unroll2(void * dst, int c, size_t size)
{
    auto p = static_cast<__m256i *>(dst);
    auto p_end = p + size / sizeof(__m256i);

    auto vvalue = _mm256_set1_epi8(c);
    while (p < p_end) {
        _mm256_store_si256(p + 0, vvalue);
        _mm256_store_si256(p + 1, vvalue);
        p += 2;
    }
}

static void memset_avx_unroll4(void * dst, int c, size_t size)
{
    auto p = static_cast<__m256i *>(dst);
    auto p_end = p + size / sizeof(__m256i);

    auto vvalue = _mm256_set1_epi8(c);
    while (p < p_end) {
        _mm256_store_si256(p + 0, vvalue);
        _mm256_store_si256(p + 1, vvalue);
        _mm256_store_si256(p + 2, vvalue);
        _mm256_store_si256(p + 3, vvalue);
        p += 4;
    }
}

static void memset_avx_unroll8(void * dst, int c, size_t size)
{
    auto p = static_cast<__m256i *>(dst);
    auto p_end = p + size / sizeof(__m256i);

    auto vvalue = _mm256_set1_epi8(c);
    while (p < p_end) {
        _mm256_store_si256(p + 0, vvalue);
        _mm256_store_si256(p + 1, vvalue);
        _mm256_store_si256(p + 2, vvalue);
        _mm256_store_si256(p + 3, vvalue);
        _mm256_store_si256(p + 4, vvalue);
        _mm256_store_si256(p + 5, vvalue);
        _mm256_store_si256(p + 6, vvalue);
        _mm256_store_si256(p + 7, vvalue);
        p += 8;
    }
}

static void memset_avx_stream(void * dst, int c, size_t size)
{
    auto p = static_cast<__m256i *>(dst);
    auto p_end = p + size / sizeof(__m256i);

    auto vvalue = _mm256_set1_epi8(c);
    while (p < p_end) {
        _mm256_stream_si256(p + 0, vvalue);
        _mm256_stream_si256(p + 1, vvalue);
        p += 2;
    }
}

static void memset_avx_prefetch(void * dst, int c, size_t size)
{
    auto p = static_cast<__m256i *>(dst);
    auto p_end = p + size / sizeof(__m256i);

    auto vvalue = _mm256_set1_epi8(c);
    while (p < p_end) {
        _mm_prefetch(static_cast<void *>(p + 32), _MM_HINT_T0);
        _mm256_store_si256(p + 0, vvalue);
        _mm256_store_si256(p + 1, vvalue);
        p += 2;
    }
}
#endif

constexpr size_t max_size = 64 * 1024 * 1024;

template <class F>
static void benchmark(const char * name, void * dst, size_t size, F f)
{
    size_t n_times = 1000;
    size_t cache_size = 16 * 1024 * 1024;

    auto tmp = rdtsc();
    auto tsc_overhead = 0;//rdtsc() - tmp;

    {
        size_t sum = 0;
        size_t min = SIZE_MAX;
        size_t max = 0;

        for (size_t i = 0; i < n_times; i++) {

            // flush all cache lines
            std::memset(dst, 0, cache_size);

            auto t0 = rdtsc();
            f(dst, 0, size);
            auto t1 = rdtsc();

            auto t = t1 - t0 - tsc_overhead;
            sum += t;
            min = std::min(min, t);
            max = std::max(max, t);
        }

        auto avg = sum / n_times;

        std::printf("%-20s: size = %9zu, avg = %9zu, min = %9zu, max = %9zu\n",
                    name, size, avg, min, max);
    }
}

int main()
{
    void * dst = _mm_malloc(max_size, 32);

#define BENCH(f) benchmark(#f, dst, size, f)

    for (size_t size = 32; size <= max_size; size *= 2) {
        BENCH(std::memset);
#ifdef __AVX__
        BENCH(memset_avx_naive);
        BENCH(memset_avx_unroll2);
        BENCH(memset_avx_unroll4);
        BENCH(memset_avx_unroll8);
        BENCH(memset_avx_stream);
        BENCH(memset_avx_prefetch);
#endif
    }

#undef BENCH

    return 0;
}

