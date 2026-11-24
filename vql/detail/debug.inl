#pragma once

#include <print>

namespace vql::debug
{
namespace detail
{
    void print_tabs(uint8_t n)
    {
        for (uint8_t i = 0; i < n; ++i)
            std::print("{}", '\t');
    }
}

template<typename T> requires (is_template_of(^^T, ^^table))
void print()
{
    std::println("class {}", std::meta::display_string_of(^^T));
    std::println("{}", "{");
    std::println("\tcolumn_views");
    std::println("\t{}", "{");
    template for (constexpr meta_info m : std::define_static_array(std::meta::nonstatic_data_members_of(^^decltype(T::views), std::meta::access_context::unchecked())))
    {
        std::println("\t\tcolumn_view<{}> {};", std::meta::display_string_of(std::meta::dealias(^^typename [:type_of(m):]::value_t)), std::meta::identifier_of(m));
    }
    std::println("\t{}", "}");
    std::println("{}", "}");
}
}