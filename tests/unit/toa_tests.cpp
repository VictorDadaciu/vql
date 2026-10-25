#include "suite_helpers.h"

class toa_tests : public vtl::suite
{
public:
    VTL_TESTS_BEGIN(toa_tests);
    VTL_TEST(static_vals);
    VTL_TEST(constructor);
    VTL_TEST(push_back);
    VTL_TEST(append);
    VTL_TEST(resize);
    VTL_TEST(clear);
    VTL_TEST(reserve);
    VTL_TEST(shrink_to_fit);
    VTL_TEST(iterate);
    VTL_TEST(multi_iterate);
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
        vql::toa<size_t, char> t{};
        size_t c = 1zu;
        for (size_t i = 0zu; i < 10zu; ++i, ++c)
        {
            VTL_EQUALS(t.push_back(), i);
            VTL_EQUALS(t.count(), c);
            VTL_EQUALS(t.capacity(), vql::next_power_of_2(c));

            VTL_EQUALS(t.get<0>()[i], 0zu);
            VTL_EQUALS(t.get<1>()[i], '\0');
        }
        for (size_t i = 10zu; i < 20zu; ++i, ++c)
        {
            auto tuple = vql::make_tuple(i, static_cast<char>(i));
            VTL_EQUALS(t.push_back(tuple), i);
            VTL_EQUALS(t.count(), c);
            VTL_EQUALS(t.capacity(), vql::next_power_of_2(c));
            
            VTL_EQUALS(t.get<0>()[i], i);
            VTL_EQUALS(t.get<1>()[i], static_cast<char>(i));
        }
        for (size_t i = 20zu; i < 30zu; ++i, ++c)
        {
            VTL_EQUALS((t.push_back(std::move(vql::make_tuple(i, static_cast<char>(i))))), i);
            VTL_EQUALS(t.count(), c);
            VTL_EQUALS(t.capacity(), vql::next_power_of_2(c));
            
            VTL_EQUALS(t.get<0>()[i], i);
            VTL_EQUALS(t.get<1>()[i], static_cast<char>(i));
        }
        for (size_t i = 30zu; i < 40zu; ++i, ++c)
        {
            VTL_EQUALS((t.push_back(i, static_cast<char>(i))), i);
            VTL_EQUALS(t.count(), c);
            VTL_EQUALS(t.capacity(), vql::next_power_of_2(c));
            
            VTL_EQUALS(t.get<0>()[i], i);
            VTL_EQUALS(t.get<1>()[i], static_cast<char>(i));
        }
        for (size_t i = 40zu; i < 50zu; ++i, ++c)
        {
            VTL_EQUALS((t.push_back(42zu, 'c')), i);
            VTL_EQUALS(t.count(), c);
            VTL_EQUALS(t.capacity(), vql::next_power_of_2(c));
            
            VTL_EQUALS(t.get<0>()[i], 42zu);
            VTL_EQUALS(t.get<1>()[i], 'c');
        }
    }

    void append()
    {
        vql::toa<size_t, char> t{};
        size_t c = 1zu;
        size_t from;
        {
            from = t.append(10zu);
            VTL_EQUALS(from, 0zu);
            VTL_EQUALS(t.count(), from + 10zu);
            VTL_EQUALS(t.capacity(), vql::next_power_of_2(t.count()));
            for (size_t i = from; i < t.count(); ++i)
            {
                VTL_EQUALS(t.get<0>()[i], 0zu);
                VTL_EQUALS(t.get<1>()[i], '\0');
            }
        }
        {
            auto tuple = vql::make_tuple(67zu, 'c');
            from = t.append(10zu, tuple);
            VTL_EQUALS(from, 10zu);
            VTL_EQUALS(t.count(), from + 10zu);
            VTL_EQUALS(t.capacity(), vql::next_power_of_2(t.count()));
            for (size_t i = from; i < t.count(); ++i)
            {
                VTL_EQUALS(t.get<0>()[i], 67zu);
                VTL_EQUALS(t.get<1>()[i], 'c');
            }
        }
        {
            from = t.append(10zu, 42zu, 't');
            VTL_EQUALS(from, 20zu);
            VTL_EQUALS(t.count(), from + 10zu);
            VTL_EQUALS(t.capacity(), vql::next_power_of_2(t.count()));
            for (size_t i = from; i < t.count(); ++i)
            {
                VTL_EQUALS(t.get<0>()[i], 42zu);
                VTL_EQUALS(t.get<1>()[i], 't');
            }
        }
        {
            {
                vql::toa<size_t, char> t2(10zu, 432zu, 'o');
                from = t.append(t2);
            }
            VTL_EQUALS(from, 30zu);
            VTL_EQUALS(t.count(), from + 10zu);
            VTL_EQUALS(t.capacity(), vql::next_power_of_2(t.count()));
            for (size_t i = from; i < t.count(); ++i)
            {
                VTL_EQUALS(t.get<0>()[i], 432zu);
                VTL_EQUALS(t.get<1>()[i], 'o');
            }
        }
        {
            {
                from = t.append(std::move(vql::toa<size_t, char>(10zu, 9000zu, 'z')));
            }
            VTL_EQUALS(from, 40zu);
            VTL_EQUALS(t.count(), from + 10zu);
            VTL_EQUALS(t.capacity(), vql::next_power_of_2(t.count()));
            for (size_t i = from; i < t.count(); ++i)
            {
                VTL_EQUALS(t.get<0>()[i], 9000zu);
                VTL_EQUALS(t.get<1>()[i], 'z');
            }
        }
    }

    void resize()
    {
        vql::toa<size_t, char> t(20zu, 5zu, 'c');
        VTL_EQUALS(t.count(), 20zu);
        VTL_EQUALS(t.capacity(), 32zu);
        {
            t.resize(30zu);
            VTL_EQUALS(t.count(), 30zu);
            VTL_EQUALS(t.capacity(), 32zu);
            t.resize(30zu);
            VTL_EQUALS(t.count(), 30zu);
            VTL_EQUALS(t.capacity(), 32zu);
            t.resize(20zu);
            VTL_EQUALS(t.count(), 20zu);
            VTL_EQUALS(t.capacity(), 32zu);
        }
        {
            auto tuple = vql::make_tuple(10zu, 't');
            t.resize(30zu, tuple);
            VTL_EQUALS(t.count(), 30zu);
            VTL_EQUALS(t.capacity(), 32zu);
            t.resize(30zu, tuple);
            VTL_EQUALS(t.count(), 30zu);
            VTL_EQUALS(t.capacity(), 32zu);
            t.resize(20zu, tuple);
            VTL_EQUALS(t.count(), 20zu);
            VTL_EQUALS(t.capacity(), 32zu);
        }
        {
            t.resize(30zu, 20zu, 'w');
            VTL_EQUALS(t.count(), 30zu);
            VTL_EQUALS(t.capacity(), 32zu);
            t.resize(30zu, 20zu, 'w');
            VTL_EQUALS(t.count(), 30zu);
            VTL_EQUALS(t.capacity(), 32zu);
            t.resize(20zu, 20zu, 'w');
            VTL_EQUALS(t.count(), 20zu);
            VTL_EQUALS(t.capacity(), 32zu);
        }
    }

    void clear()
    {
        {
            vql::toa<size_t, char> t(20zu);
            VTL_EQUALS(t.count(), 0zu);
            VTL_EQUALS(t.capacity(), 20zu);
            t.clear();
            VTL_EQUALS(t.count(), 0zu);
            VTL_EQUALS(t.capacity(), 20zu);
        }
        {
            vql::toa<size_t, char> t(20zu, 5zu, 'c');
            VTL_EQUALS(t.count(), 20zu);
            VTL_EQUALS(t.capacity(), 32zu);
            t.clear();
            VTL_EQUALS(t.count(), 0zu);
            VTL_EQUALS(t.capacity(), 32zu);
        }
    }

    void reserve()
    {
        vql::toa<size_t, char> t(20zu);
        VTL_EQUALS(t.count(), 0zu);
        VTL_EQUALS(t.capacity(), 20zu);
        t.reserve(1zu);
        VTL_EQUALS(t.count(), 0zu);
        VTL_EQUALS(t.capacity(), 20zu);
        t.reserve(20zu);
        VTL_EQUALS(t.count(), 0zu);
        VTL_EQUALS(t.capacity(), 20zu);
        t.reserve(32zu);
        VTL_EQUALS(t.count(), 0zu);
        VTL_EQUALS(t.capacity(), 32zu);
    }

    void shrink_to_fit()
    {
        vql::toa<size_t, char> t(20zu, 20zu, 'c');
        VTL_EQUALS(t.count(), 20zu);
        VTL_EQUALS(t.capacity(), 32zu);
        t.shrink_to_fit();
        VTL_EQUALS(t.count(), 20zu);
        VTL_EQUALS(t.capacity(), 20zu);
        t.resize(10zu);
        VTL_EQUALS(t.count(), 10zu);
        VTL_EQUALS(t.capacity(), 20zu);
        t.shrink_to_fit();
        VTL_EQUALS(t.count(), 10zu);
        VTL_EQUALS(t.capacity(), 10zu);
        t.clear();
        VTL_EQUALS(t.count(), 0zu);
        VTL_EQUALS(t.capacity(), 10zu);
        t.shrink_to_fit();
        VTL_EQUALS(t.count(), 0zu);
        VTL_EQUALS(t.capacity(), 1zu);
    }

    void iterate()
    {
        vql::toa<size_t, char> t(20zu, 20zu, 'c');
        size_t grow = 0zu;
        for (auto it = t.begin<size_t>(); it != t.end<size_t>(); ++it)
        {
            VTL_EQUALS(*it, 20zu);
            *it = grow++;
        }
        grow = 0zu;
        for (auto it = t.cbegin<size_t>(); it != t.cend<size_t>(); ++it)
            VTL_EQUALS(*it, grow++);
        for (auto it = t.crbegin<size_t>(); it != t.crend<size_t>(); ++it)
            VTL_EQUALS(*it, --grow);
    }

    void multi_iterate()
    {
        vql::toa<size_t, char> t(20zu, 20zu, 'c');
        size_t grow = 0zu;
        for (auto it = t.begin<size_t, char>(); it != t.end<size_t, char>(); ++it)
        {
            VTL_EQUALS(*it.get<size_t>(), 20zu);
            VTL_EQUALS(*it.get<char>(), 'c');
            *it.get<size_t>() = grow;
            *it.get<char>() = 'a' + grow++;
        }
        grow = 0zu;
        for (auto it = t.cbegin<size_t, char>(); it != t.cend<size_t, char>(); ++it)
        {
            VTL_EQUALS(*it.get<size_t>(), grow);
            VTL_EQUALS(*it.get<char>(), static_cast<char>('a' + grow++));
        }
        for (auto it = t.crbegin<size_t, char>(); it != t.crend<size_t, char>(); ++it)
        {
            VTL_EQUALS(*it.get<size_t>(), --grow);
            VTL_EQUALS(*it.get<char>(), static_cast<char>('a' + grow));
        }
    }
};

VTL_REGISTER_SUITE(toa_tests);
