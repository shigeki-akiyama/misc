
#include <vector>
#include <iostream>

std::vector<int> f() {
    return { 41 };
}

int main() { std::cout << f().front() << std::endl; return 0; }
