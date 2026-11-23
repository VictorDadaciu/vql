#include "vql/table.h"

#include "suite_helpers.h"

#include <algorithm>
#include <numeric>
#include <string>

struct object_metadata
{
    std::string names;
    size_t ids;
};

struct character
{
    [[=vql::flatten]] object_metadata mds;
    int hps;
    float dmgs;
};

struct table_tests : vtl::suite
{
    VTL_TESTS_BEGIN(table_tests);
    VTL_TEST(arena);
    VTL_TESTS_END();

    void arena()
    {
        vql::table<character> chs;
        vql::table<character> chs2(std::move(chs));
        VTL_EQUALS(chs.push_back(), 0zu);
        VTL_EQUALS(chs.views.hps[0], 0);
        chs.views.hps[0] = 1;
        VTL_EQUALS(chs.push_back(), 1zu);
        VTL_EQUALS(chs.count(), 2zu);
        VTL_EQUALS(chs.views.hps.count(), 2zu);
        VTL_EQUALS(chs.views.dmgs.count(), 2zu);
        VTL_EQUALS(chs.push_back(), 2zu);
        VTL_EQUALS(chs.views.hps.count(), 3zu);
        VTL_EQUALS(chs.views.dmgs.count(), 3zu);
        VTL_EQUALS(chs.views.names.count(), 3zu);
        VTL_EQUALS(chs.views.ids.count(), 3zu);
        VTL_EQUALS(chs.views.hps[0], 1);

        for (int i = 0; auto& it : chs.views.hps)
            it = i++;
        for (int i = 0; const auto& it : chs.views.hps)
            VTL_EQUALS(it, i++);

        VTL_EQUALS(std::accumulate(chs.views.hps.cbegin(), chs.views.hps.cend(), 0), 3);

        vql::table<character>::row row;
        VTL_EQUALS(sizeof(row), 48zu);
    }
};

VTL_REGISTER_SUITE(table_tests);

namespace compile_time
{
    class empty{};
    class privates{int x;};
    struct publics{int x;};
    struct both{int x; private: int y;};

    static_assert(!vql::TableLayoutLike<int>);
    static_assert(!vql::TableLayoutLike<empty>);
    static_assert(!vql::TableLayoutLike<privates>);
    static_assert(vql::TableLayoutLike<publics>);
    static_assert(vql::TableLayoutLike<both>);
    static_assert(!vql::TableLayoutLike<std::string>);

    static_assert(vql::TableMetadatum<vql::vacuumed>);
    static_assert(vql::TableMetadatum<vql::handled>);

    static_assert(!vql::table_metadata<>::is_vacuumed);
    static_assert(!vql::table_metadata<>::is_handled);
    static_assert(vql::table_metadata<vql::vacuumed>::is_vacuumed);
    static_assert(!vql::table_metadata<vql::vacuumed>::is_handled);
    static_assert(!vql::table_metadata<vql::handled>::is_vacuumed);
    static_assert(vql::table_metadata<vql::handled>::is_handled);
    static_assert(vql::table_metadata<vql::vacuumed, vql::handled>::is_vacuumed);
    static_assert(vql::table_metadata<vql::vacuumed, vql::handled>::is_handled);
    static_assert(!vql::table<both, vql::table_metadata<>>::metadata::is_vacuumed);
    static_assert(!vql::table<both, vql::table_metadata<>>::metadata::is_handled);
}