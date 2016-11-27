#include <stdio.h>

int main()
{
#if defined(__APPLE__) && defined(__MACH__)
    printf("This OS is Apple Darwin.\n");
#else
    printf("This OS is not Apple Darwin.\n");
#endif
    return 0;
}

