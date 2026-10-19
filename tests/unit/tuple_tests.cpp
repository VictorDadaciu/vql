#include <vtl/suite.h>

#include <vql/tuple.h>

#include <sstream>

namespace vtl
{
template <typename... Ts>
struct suite_helper<vql::tuple<Ts...>>
{
    inline static std::string to_string(const vql::tuple<Ts...>& t)
    {
        if constexpr (sizeof...(Ts) == 0zu)
            return "()";
        else if constexpr (sizeof...(Ts) == 1zu)
            return std::format("({})", t.template get<0>());

        std::stringstream ss;
        ss << std::format("({}", t.template get<0>());
        template for (constexpr size_t I : std::views::iota(1zu, sizeof...(Ts)))
            ss << std::format(", {}", t.template get<I>());
        ss << ")";
        return ss.str();
    }

    inline static bool equals(const vql::tuple<Ts...>& a, const vql::tuple<Ts...>& b)
    {
        return a == b;
    }
};
}

struct nontrivial
{
    inline static size_t constructed = 0zu;
    inline static size_t copy_constructed = 0zu;
    inline static size_t move_constructed = 0zu;
    inline static size_t copy_assigned = 0zu;
    inline static size_t move_assigned = 0zu;
    inline static size_t destructed = 0zu;

    inline static void reset()
    {
        constructed = 0zu;
        copy_constructed = 0zu;
        move_constructed = 0zu;
        copy_assigned = 0zu;
        move_assigned = 0zu;
        destructed = 0zu;
    }

    nontrivial(size_t other = 0zu) : x(other)
    {
        ++constructed;
    }

    nontrivial(const nontrivial& other) : x(other.x)
    {
        ++copy_constructed;
    }

    nontrivial(nontrivial&& other) : x(other.x)
    {
        ++move_constructed;
    }

    nontrivial& operator=(const nontrivial& other)
    {
        x = other.x;
        ++copy_assigned;
        return *this;
    }

    nontrivial& operator=(nontrivial&& other)
    {
        x = other.x;
        ++move_assigned;
        return *this;
    }

    ~nontrivial()
    {
        ++destructed;
    }

    size_t x;
};

inline bool operator==(const nontrivial& a, const nontrivial& b) noexcept
{
    return a.x == b.x;
}

class tuple_tests : public vtl::suite
{
public:
    VTL_TESTS_BEGIN(tuple_tests);
    VTL_TEST(static_vals);
    VTL_TEST(constructor);
    VTL_TEST(make_tuple);
    VTL_TEST(assignment);
    VTL_TEST(get_set_element);
    VTL_TEST(comparison);
    VTL_TESTS_END();

    void tear_down() override
    {
        nontrivial::reset();
    }

    void static_vals()
    {
        VTL_EQUALS(vql::tuple<>::size, 0zu);
        VTL_EQUALS(vql::tuple<int>::size, 1zu);
        VTL_EQUALS(vql::tuple<char>::size, 1zu);
        VTL_EQUALS((vql::tuple<char, int>::size), 2zu);
        VTL_EQUALS((vql::tuple<char, nontrivial>::size), 2zu);
        VTL_EQUALS(decltype(vql::make_tuple(4, 'c', 7zu))::size, 3zu);
        VTL_EQUALS(decltype(vql::make_tuple<vql::typelist<int, char, size_t>>())::size, 3zu);

        VTL_EQUALS(sizeof(vql::tuple<>), 1zu);
        VTL_EQUALS(sizeof(vql::tuple<int>), 4zu);
        VTL_EQUALS(sizeof(vql::tuple<char>), 1zu);
        VTL_EQUALS((sizeof(vql::tuple<char, int>)), 8zu);
        VTL_EQUALS((sizeof(vql::tuple<char, nontrivial>)), 16zu);
        VTL_EQUALS((sizeof(decltype(vql::make_tuple(4, 'c', 7zu)))), 16zu);
        VTL_EQUALS((sizeof(decltype(vql::make_tuple<vql::typelist<int, char, size_t>>()))), 16zu);

        VTL_EQUALS(alignof(vql::tuple<>), 1zu);
        VTL_EQUALS(alignof(vql::tuple<int>), 4zu);
        VTL_EQUALS(alignof(vql::tuple<char>), 1zu);
        VTL_EQUALS((alignof(vql::tuple<char, int>)), 4zu);
        VTL_EQUALS((alignof(vql::tuple<char, nontrivial>)), 8zu);
        VTL_EQUALS((alignof(decltype(vql::make_tuple(4, 'c', 7zu)))), 8zu);
        VTL_EQUALS((alignof(decltype(vql::make_tuple<vql::typelist<int, char, size_t>>()))), 8zu);

    }

    void constructor()
    {
        {
            vql::tuple<int, char, nontrivial> tuple{};
            VTL_EQUALS(tuple.get<0>(), 0);
            VTL_EQUALS(tuple.get<1>(), '\0');
            VTL_EQUALS(nontrivial::constructed, 1zu);
            VTL_EQUALS(nontrivial::destructed, 0zu);
        }
        VTL_EQUALS(nontrivial::destructed, 1zu);
        nontrivial::reset();
        {
            vql::tuple<int, char, nontrivial> tuple(10, 'c', 5zu);
            VTL_EQUALS(tuple.get<0>(), 10);
            VTL_EQUALS(tuple.get<1>(), 'c');
            VTL_EQUALS(nontrivial::constructed, 1zu);
            VTL_EQUALS(nontrivial::destructed, 1zu);
        }
        VTL_EQUALS(nontrivial::destructed, 2zu);
        nontrivial::reset();
        {
            vql::tuple<int, char, nontrivial> tuple1(10, 'c', 5zu);
            vql::tuple<int, char, nontrivial> tuple2(tuple1);
            VTL_EQUALS(tuple2.get<0>(), 10);
            VTL_EQUALS(tuple2.get<1>(), 'c');
            VTL_EQUALS(nontrivial::copy_constructed, 1zu);
            VTL_EQUALS(nontrivial::destructed, 1zu);
        }
        VTL_EQUALS(nontrivial::destructed, 3zu);
        nontrivial::reset();
        {
            vql::tuple<int, char, nontrivial> tuple1(10, 'c', 5zu);
            vql::tuple<int, char, nontrivial> tuple2(std::move(tuple1));
            VTL_EQUALS(tuple2.get<0>(), 10);
            VTL_EQUALS(tuple2.get<1>(), 'c');
            VTL_EQUALS(nontrivial::move_constructed, 2zu);
            VTL_EQUALS(nontrivial::destructed, 1zu);
        }
        VTL_EQUALS(nontrivial::destructed, 3zu);
    }

    void make_tuple()
    {
        {
            auto tuple = vql::make_tuple<vql::typelist<int, char, size_t>>();
            VTL_EQUALS(tuple.get<0>(), 0);
            VTL_EQUALS(tuple.get<1>(), '\0');
            VTL_EQUALS(tuple.get<2>(), 0zu);
        }
        {
            auto tuple = vql::make_tuple(4, 'c', 7zu);
            VTL_EQUALS(tuple.get<0>(), 4);
            VTL_EQUALS(tuple.get<1>(), 'c');
            VTL_EQUALS(tuple.get<2>(), 7zu);
        }
    }

    void assignment()
    {
        {
            vql::tuple<int, char, nontrivial> tuple1(10, 'c', 5zu);
            vql::tuple<int, char, nontrivial> tuple2(20, 'd', 10zu);
            tuple2 = tuple1;
            VTL_EQUALS(tuple2.get<0>(), 10);
            VTL_EQUALS(tuple2.get<1>(), 'c');
            VTL_EQUALS(tuple2.get<2>().x, 5zu);
            VTL_EQUALS(nontrivial::copy_assigned, 1zu);
            VTL_EQUALS(nontrivial::destructed, 2zu);
        }
        VTL_EQUALS(nontrivial::destructed, 4zu);
        nontrivial::reset();
        {
            vql::tuple<int, char, nontrivial> tuple1(10, 'c', 5zu);
            vql::tuple<int, char, nontrivial> tuple2(20, 'd', 10zu);
            tuple2 = std::move(tuple1);
            VTL_EQUALS(tuple2.get<0>(), 10);
            VTL_EQUALS(tuple2.get<1>(), 'c');
            VTL_EQUALS(tuple2.get<2>().x, 5zu);
            VTL_EQUALS(nontrivial::move_assigned, 1zu);
            VTL_EQUALS(nontrivial::destructed, 2zu);
        }
        VTL_EQUALS(nontrivial::destructed, 4zu);
    }

    void get_set_element()
    {
        vql::tuple<int, char, size_t, int, size_t> t{};
        VTL_EQUALS(t.get<0>(), 0);
        VTL_EQUALS(t.get<1>(), '\0');
        VTL_EQUALS(t.get<2>(), 0zu);
        VTL_EQUALS(t.get<3>(), 0);
        VTL_EQUALS(t.get<4>(), 0zu);
        t.get<0>() = 1;
        t.get<1>() = 'c';
        t.get<2>() = 1zu;
        t.get<3>() = 2;
        t.get<4>() = 2zu;
        VTL_EQUALS(t.get<int>(), 1);
        VTL_EQUALS(t.get<char>(), 'c');
        VTL_EQUALS(t.get<size_t>(), 1zu);
        VTL_EQUALS((t.get<int, 1>()), 2);
        VTL_EQUALS((t.get<size_t, 1>()), 2zu);
    }

    void comparison()
    {
        vql::tuple<int, int> t1;
        vql::tuple<int, int> t2;
        VTL_EQUALS(t1, t2);
        t1.get<int, 1>() = 1;
        VTL_ASSERT(t1 != t2);
    }
};

VTL_REGISTER_SUITE(tuple_tests);
