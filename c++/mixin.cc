
#include <iostream>
using namespace std;

class member_A {
public:
    member_A() { cout << __PRETTY_FUNCTION__ << endl; }
    ~member_A() { cout << __PRETTY_FUNCTION__ << endl; }
};

class member_B {
public:
    member_B() { cout << __PRETTY_FUNCTION__ << endl; }
    ~member_B() { cout << __PRETTY_FUNCTION__ << endl; }
};

class member_C {
public:
    member_C() { cout << __PRETTY_FUNCTION__ << endl; }
    ~member_C() { cout << __PRETTY_FUNCTION__ << endl; }
};

class mixin_A {
    member_A a;
public:
    mixin_A() { cout << __PRETTY_FUNCTION__ << endl; }
    ~mixin_A() { cout << __PRETTY_FUNCTION__ << endl; }

    void hoge() { cout << "hoge" << endl; }
};

class mixin_B {
    member_B b;
public:
    mixin_B() { cout << __PRETTY_FUNCTION__ << endl; }
    ~mixin_B() { cout << __PRETTY_FUNCTION__ << endl; }

    void fuga() { cout << "fuga" << endl; }
};


class cls_A : public mixin_A, public mixin_B
{
    member_C c;
public:
    cls_A() { cout << __PRETTY_FUNCTION__ << endl; }
    ~cls_A() = default;
};

class cls_B : public mixin_A, public mixin_B
{
    member_C c;
public:
    cls_B() : c(), mixin_B(), mixin_A() { cout << __PRETTY_FUNCTION__ << endl; }
    ~cls_B() = default;
};

int main() {
    { cls_A c; }
    cout << "----------------------" << endl;
    { cls_B cb; }
    return 0;
}
