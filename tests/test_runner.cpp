#include <iostream>

#include <vtl/suite.h>

#include <vql/tuple.h>

int main()
{
    std::cout << "vql tests begin...\n";

    int x = 5;
    vql::tuple<int, float, float> t(x, 10.5f, -4.f);
    vql::tuple<int, float, float> t1(x, 10.5f, -4.f);
    vql::tuple<int, float, float> w(t);
    vql::tuple<int, float, float> y(std::move(t));
    w = std::move(y);
    std::cout << w.get<0>() << "\n";
    std::cout << w.get<float, 1>() << "\n";
    std::cout << (w == t1) << "\n";
    std::cout << (w != t1) << "\n";

    return 0;
}