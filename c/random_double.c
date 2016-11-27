
#include "common.h"
#include "limits.h"

double random_double()
{
    return (double)random_int(INT_MAX) / (double)INT_MAX;
}

int main()
{
    int i;
    for (i = 0; i < 1000; i++) {
        double d = random_double();
        printf("%f\n", d);
    }
    return 0;
}
