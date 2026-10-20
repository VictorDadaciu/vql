#include <vtl/suite.h>

#include <vql/toa.h>

class toa_tests : public vtl::suite
{
public:
    VTL_TESTS_BEGIN(toa_tests);
    VTL_TEST(static_vals);
    VTL_TEST(constructor);
    VTL_TEST(push_back);
    VTL_TESTS_END();

    void static_vals()
    {
        VTL_EQUALS(vql::toa<int>::size, 1zu);
        VTL_EQUALS((vql::toa<int, char>::size), 2zu);
        VTL_EQUALS((vql::toa<int, char, float>::size), 3zu);

        VTL_EQUALS(sizeof(vql::iterator<int>), 8zu);
        VTL_EQUALS(sizeof(vql::const_iterator<int>), 8zu);
        VTL_EQUALS(sizeof(vql::reverse_iterator<int>), 8zu);
        VTL_EQUALS(sizeof(vql::const_reverse_iterator<int>), 8zu);

        VTL_EQUALS(sizeof(vql::iterator<char>), 8zu);
        VTL_EQUALS(sizeof(vql::const_iterator<char>), 8zu);
        VTL_EQUALS(sizeof(vql::reverse_iterator<char>), 8zu);
        VTL_EQUALS(sizeof(vql::const_reverse_iterator<char>), 8zu);

        VTL_EQUALS(sizeof(vql::iterator<float>), 8zu);
        VTL_EQUALS(sizeof(vql::const_iterator<float>), 8zu);
        VTL_EQUALS(sizeof(vql::reverse_iterator<float>), 8zu);
        VTL_EQUALS(sizeof(vql::const_reverse_iterator<float>), 8zu);
    }

    void constructor()
    {
        {
            vql::toa<int, char> t1{};
            VTL_EQUALS(t1.count(), 0zu);
            VTL_EQUALS(t1.capacity(), 1zu);
            vql::toa<int, char> t2{t1};
            VTL_EQUALS(t2.count(), 0zu);
            VTL_EQUALS(t2.capacity(), 1zu);
            vql::toa<int, char> t3{std::move(t2)};
            VTL_EQUALS(t3.count(), 0zu);
            VTL_EQUALS(t3.capacity(), 1zu);
        }
        {
            vql::toa<int, char> t1{10zu};
            VTL_EQUALS(t1.count(), 0zu);
            VTL_EQUALS(t1.capacity(), 10zu);
            vql::toa<int, char> t2{t1};
            VTL_EQUALS(t2.count(), 0zu);
            VTL_EQUALS(t2.capacity(), 10zu);
            vql::toa<int, char> t3{std::move(t2)};
            VTL_EQUALS(t3.count(), 0zu);
            VTL_EQUALS(t3.capacity(), 10zu);
        }
    }

    void push_back()
    {
        {
            vql::toa<int, char> t{};
            t.push_back();
            VTL_EQUALS(t.count(), 1zu);
            VTL_EQUALS(t.capacity(), 1zu);
            t.push_back();
            VTL_EQUALS(t.count(), 2zu);
            VTL_EQUALS(t.capacity(), 2zu);
            t.push_back();
            VTL_EQUALS(t.count(), 3zu);
            VTL_EQUALS(t.capacity(), 4zu);
        }
    }
};

VTL_REGISTER_SUITE(toa_tests);
