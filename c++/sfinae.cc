#include <type_traits>
#include <iostream>
#include <cstdint>

using U = int64_t;

template <class T, class = void> T hoge();

#if 1
template <>
long hoge() { return 0; }
#else
template <class T>
typename std::enable_if<std::is_same<T, long>::value, T>::type
hoge() { return 0; }
#endif

#if 1
template <class T, typename std::enable_if<std::is_same<T, U>::value
                                     && !std::is_same<T, long>::value>::type>
T hoge() { return 1; }
#else
template <class T>
typename std::enable_if<std::is_same<T, U>::value
                        && !std::is_same<long, U>::value, T>::type
hoge() { return 1; }
#endif

int main() {
    std::cout << hoge<long>() << std::endl;
    std::cout << hoge<int64_t>() << std::endl;
    return 0;
}
