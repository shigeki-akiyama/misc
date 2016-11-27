#include <stdlib.h>
#include <stdio.h>

int uniform_int(int n)
{
    size_t adjusted_max = ((size_t)RAND_MAX + 1) - ((size_t)RAND_MAX + 1) % n;
    int r;
    do {
        r = rand();
    } while ((size_t)r >= adjusted_max);

     return (int)((double)n * (double)r / (double)adjusted_max);
}


int main() {
    int i;
    for (i = 0; i < 128; i++) {
        int n = uniform_int(128);
        printf("%d\n", n);
    }
    return 0;
}
