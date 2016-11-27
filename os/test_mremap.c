
#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>

int main(void) {
    const int PAGESIZE = 4096;

    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;

    size_t size = PAGESIZE * 1024;
    void *p = mmap(NULL, size, prot, flags, -1, 0);
    assert(p != MAP_FAILED);

    int result = munmap(p, size);
    assert(result == 0);

    void *p2 = mremap(p, PAGESIZE, PAGESIZE * 2, 0);
    if (p2 == MAP_FAILED) {
        perror("mremap");
        printf("mremap failed\n");
    } else {
        printf("mremap success (%p)\n", p2);
        *(int *)p2 = 1;
    }

    return 0;
}
