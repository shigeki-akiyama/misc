#include <iostream>

int main() {
    int i = 0;
    auto& r = i;
    std::cout << "&r = " << &r << std::endl
              << "&i = " << &i << std::endl;
    return 0;
}
