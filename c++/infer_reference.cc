#include <cstdio>

int& reference(int *p)
{
  return *p;
}

int main()
{
    int value = 10;
    std::printf("&value = %p\n", &value);
    
    auto& ref0 = reference(&value);
    std::printf("&ref0  = %p\n", &ref0);

    auto ref1 = reference(&value);
    std::printf("&ref1  = %p\n", &ref1);

    return 0;
}
