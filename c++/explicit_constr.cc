

struct box0 {
    int x_;

    box0(int x) : x_(x) {}
};

struct box1 {
    int x_;

    explicit box1(int x) : x_(x) {}
};

void f(box0 b)
{
}

void g(box1 b)
{
}

int main() {
    box0 v0 = 0;
    box0 v1(1);
    f(2);

    box1 v3 = 3;
    box1 v4(4);
    g(5);

    return 0;
}
