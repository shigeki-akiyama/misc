#include <stdio.h>
#include <pthread.h>

int main(void)
{
    printf("sizeof(pthread_mutex_t) = %zd\n", sizeof(pthread_mutex_t));
    return 0;
}
