#include <cstdio>

template <class T>
T id(T v)
{
    return v;
}

int main(int argc, char **argv)
{
    int (*f)(int) = id<int>;
    char (*g)(char) = id<char>;

    printf("id<int>  = %p\n", f);
    printf("id<char> = %p\n", g);

    return 0;
}

