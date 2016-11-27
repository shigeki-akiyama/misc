#include <cstdio>
#include <initializer_list>

int main() {
    for (auto i : { 0, 1, 2, 3 }) {
        printf("i = %d\n", i);
    }
    return 0;
}
