#include <memory>

template <class T, class U = std::default_delete<T>>
struct unique_ptr : public std::unique_ptr<T, U> {
    using std::unique_ptr<T, U>::unique_ptr;
};

struct C {
    int x;

    C() : x(1) {}
    void inc() { x += 1; }
};

int main()
{
    {
        unique_ptr<int> p(new int(1));

        printf("*p == %d\n", *p);

        *p += 1;

        printf("*p += 1\n");
        printf("*p == %d\n", *p);

        unique_ptr<int> p2(std::move(p));
        unique_ptr<int> p3;
        p3 = std::move(p);
    }

    {
        unique_ptr<C> q(new C());

        printf("q->x == %d\n", q->x);

        q->inc();

        printf("q->inc()\n");
        printf("q->x == %d\n", q->x);
    }

    return 0;
}
