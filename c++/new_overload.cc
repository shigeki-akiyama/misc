#include <iostream>
#include <vector>
#include <functional>
#include <cstdlib>

void *operator new(size_t size) throw(std::bad_alloc) {
    std::cout << "new(" << size << ")" << std::endl;
    return malloc(size);
}

void operator delete(void *p) throw() {
    std::cout << "delete" << std::endl;
    free(p);
}

int main() {
    std::vector<int> v(4);
    v[0] = 1;
    v[1] = 4;
    v[2] = 3;
    v[3] = 9;

    std::cout << "sizeof(std::vector<int>) = " 
              << sizeof(std::vector<int>) << std::endl;

    for (int i = 0; i < v.size(); i++) {
        std::cout << i << std::endl;
    }
/*
    int n = 15;
    auto f = [=]() { std::cout << n << std::endl; };
    f();
    std::function<void()> g = [&]() { std::cout << v[0] << std::endl; };
    g();
*/
    return 0;
}
