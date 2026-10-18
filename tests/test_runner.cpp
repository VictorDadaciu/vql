#include <iostream>

#include <vtl/suite.h>

#include <vql/tuple.h>

class tuple_tests : public vtl::suite
{
public:
    VTL_TESTS_BEGIN(tuple_tests);
    VTL_TEST(constructor);
    VTL_TEST(comparison);
    VTL_TESTS_END();

    void constructor()
    {
        int x = 5;
        vql::tuple<int, float, float> t(x, 10.5f, -4.f);
        vql::tuple<int, float, float> t1(x, 10.5f, -4.f);
        vql::tuple<int, float, float> w(t);
        vql::tuple<int, float, float> y(std::move(t));
        w = std::move(y);
    }

    void comparison()
    {
    }

    void set_up() override
    {
        std::cout << "\t\t\tSetting up...\n";
    }

    void tear_down() override
    {
        std::cout << "\t\t\tTearing down...\n";
    }
};

VTL_REGISTER_SUITE(tuple_tests);

int main()
{
    vtl::run();

    return 0;
}