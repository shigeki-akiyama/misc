
#include <stdint.h>
#include <stdio.h>

struct hoge {
    uint64_t up52 : 52;
    uint64_t dw12 : 12;
};

struct fuga {
    uint64_t up52 : 52;
    uint64_t dw12 : 12;
    uint32_t u32;
};

struct hige {
    uint64_t u64;
    uint32_t u32;
};

int main(void) {
    printf("sizeof(uint64_t) = %zd\n", sizeof(uint64_t));
    printf("sizeof(uint64_t:52+uint64_t:12) = %zd\n", sizeof(struct hoge));
    printf("sizeof(uint64_t:52+uint64_t:12+uint32_t) = %zd\n", sizeof(struct fuga));
    printf("sizeof(uint64_t+uint32) = %zd\n", sizeof(struct hige));
    return 0;
}
