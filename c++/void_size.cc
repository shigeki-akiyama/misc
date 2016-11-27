#include <cstdio>

struct hoge {
};

int main(void) { 
    printf("sizeof(void) = %zu\n", sizeof(void)); 
    printf("sizeof(struct {}) = %zu\n", sizeof(struct hoge));

    return 0;
}
