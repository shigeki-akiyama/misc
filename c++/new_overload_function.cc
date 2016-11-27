#include <iostream>
#include <cstdio>
#include <functional>

void *operator new(size_t size) {
        std::cout << "new(" << size << ")" << std::endl;
            return malloc(size);
}

void operator delete(void *p) {
        std::cout << "delete(" << p << ")" << std::endl;
            return free(p);
}

int main() {
    std::function<void(int)> f = [](int i) { std::cout << i << std::endl; };
    
    f(1);

    return 0;
}
















