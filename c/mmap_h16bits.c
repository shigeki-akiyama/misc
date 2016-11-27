#include <sys/mman.h>

int main(void) {
    int value;
    long long ptr_value = &value;
    int *p = ptr_value | 0x1000000000000000;
    mmap(p,4096,
         PROT_READ|PROT_WRITE,
         MAP_FIXED|MAP_PRIVATE|MAP_ANON,-1,0);
    *p = 1;
    munmap(p, 4096);
    return 0;
}
