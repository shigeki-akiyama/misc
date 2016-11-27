#include <string>
#include <sstream>
#include <iostream>

int main()
{
    int x;
//    std::string s = std::to_string(&x);

    std::stringstream ss;
    ss << &x;
    std::string s = ss.str();

    std::cout << s << std::endl;

    return 0;
}
