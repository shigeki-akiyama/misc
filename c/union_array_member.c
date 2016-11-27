
#include <stdio.h>

typedef union {
    int hoge;
    int fuga[2];
} hoge_t;

int main(void) {
    hoge_t h;
    h.hoge = 1;
    printf("fuga[0] = %d\n", h.fuga[0]);
    printf("fuga[1] = %d\n", h.fuga[1]);

    h.fuga[0] = 2;
    printf("hoge = %d\n", h.hoge);
    printf("fuga[1] = %d\n", h.fuga[1]);

    h.fuga[1] = 3;
    printf("hoge = %d\n", h.hoge);
    printf("fuga[0] = %d\n", h.fuga[0]);

    return 0;
}
