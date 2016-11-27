#include <vector>
#include <tuple>
#include <iostream>
using namespace std;

int main()
{
#if 0
    vector<auto> v0 {
        make_tuple("hoge", 0),
        make_tuple("fuga", 1),
    };
#endif
    auto v1 = {
        make_tuple("hoge", 2),
        make_tuple("fuga", 3),
    };

    for (auto x : v1) {
        cout << get<0>(x) << std::endl;
        cout << get<1>(x) << std::endl;
    }

    return 0;
}

