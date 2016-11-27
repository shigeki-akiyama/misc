
#include <stdio.h>

int main(void) {
    int i, j;
    printf("&i - &j = %zd\n", &i - &j);

    void *p = &i;
    void *q = &j;
    printf("p - q = %zd\n", p - q);

    return 0;
}
