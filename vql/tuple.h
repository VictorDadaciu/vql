#pragma once

#include "utils.h"

#include <concepts>
#include <format>
#include <functional>
#include <ranges>
#include <type_traits>

namespace vql
{
template <typename... Ts>
class tuple
{
    struct storage;
    consteval
    {
        std::vector<meta_info> member_specs;

        template for (size_t i = 0zu; meta_info type : { ^^Ts... })
            member_specs.push_back(data_member_spec(type, {
                .name = std::format("item{}", i++)
            }));

        std::meta::define_aggregate(^^storage, member_specs);
    }
    storage m_data;

    static constexpr auto members = std::define_static_array(nonstatic_data_members_of(^^storage, std::meta::access_context::unchecked()));

    static consteval size_t index_of_type(meta_info, size_t=0zu) noexcept;

public:
    static constexpr size_t size = sizeof...(Ts);

    constexpr tuple()
        noexcept((std::is_nothrow_default_constructible_v<Ts> && ...))
        requires(std::is_default_constructible_v<Ts> && ...) = default;

    constexpr tuple(const Ts&... values)
        noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...))
        requires(std::is_copy_constructible_v<Ts> && ...) : m_data{values...} {}

    constexpr tuple(Ts&&... values)
        noexcept((std::is_nothrow_move_constructible_v<Ts> && ...))
        requires(std::is_move_constructible_v<Ts> && ...) : m_data{std::move(values)...} {}

    constexpr tuple(const tuple&)
        noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...))
        requires(std::is_copy_constructible_v<Ts> && ...) = default;

    constexpr tuple(tuple&&)
        noexcept((std::is_nothrow_move_constructible_v<Ts> && ...))
        requires(std::is_move_constructible_v<Ts> && ...) = default;

    constexpr tuple& operator=(const tuple&)
        noexcept((std::is_nothrow_copy_assignable_v<Ts> && ...))
        requires(std::is_copy_assignable_v<Ts> && ...) = default;

    constexpr tuple& operator=(tuple&&)
        noexcept((std::is_nothrow_move_assignable_v<Ts> && ...))
        requires(std::is_move_assignable_v<Ts> && ...) = default;

    template<size_t I> requires(I < sizeof...(Ts))
    constexpr auto& get() noexcept
    {
        return m_data.[:members[I]:];
    }

    template<size_t I> requires(I < sizeof...(Ts))
    constexpr const auto& get() const noexcept
    {
        return m_data.[:members[I]:];
    }

    template<typename T, size_t I=0zu>
    constexpr auto& get() noexcept
    {
        return get<index_of_type(^^T, I)>();
    }

    template<typename T, size_t I=0zu>
    constexpr const auto& get() const noexcept
    {
        return get<index_of_type(^^T, I)>();
    }
    
    void swap(tuple& other) noexcept((std::is_nothrow_swappable_v<Ts> && ...))
    {
        template for (constexpr size_t I : std::views::iota(0zu, size)) std::swap(get<I>(), other.get<I>());
    }
    
    constexpr void swap(const tuple& other) noexcept((std::is_nothrow_swappable_v<const Ts> && ...))
    {
        template for (constexpr size_t I : std::views::iota(0zu, size)) std::swap(get<I>(), other.get<I>());
    }
};

template <class T>
concept Tuple = is_template_of(^^T, ^^tuple);

namespace detail
{
    template <Tuple T, size_t... I>
    constexpr bool _compare_eq(const T& a, const T& b, const std::index_sequence<I...>&)
    {
        return ((a.template get<I>() == b.template get<I>()) && ...);
    }

    template <Tuple T, size_t... I>
    constexpr bool _compare_neq(const T& a, const T& b, const std::index_sequence<I...>&)
    {
        return ((a.template get<I>() != b.template get<I>()) && ...);
    }
}

template <typename... Ts>
inline constexpr bool operator==(const tuple<Ts...>& a, const tuple<Ts...>& b) noexcept
    requires ((requires (const Ts& a, const Ts& b) { a == b; }) && ...)
{
    static constexpr auto indices = std::make_index_sequence<sizeof...(Ts)>();
    return detail::_compare_eq(a, b, indices);
}

template <typename... Ts>
inline constexpr bool operator!=(const tuple<Ts...>& a, const tuple<Ts...>& b) noexcept
    requires ((requires (const Ts& a, const Ts& b) { a != b; }) && ...)
{
    static constexpr auto indices = std::make_index_sequence<sizeof...(Ts)>();
    return detail::_compare_neq(a, b, indices);
}

template <Tuple T>
inline void swap(T& a, T& b) noexcept
{
    a.swap(b);
}

template <Tuple T>
inline constexpr void swap(const T& a, const T& b) noexcept
{
    a.swap(b);
}

template <size_t I, Tuple T>
inline constexpr auto& get(T& t) noexcept
{
    return t.template get<I>();
}

template <size_t I, Tuple T>
inline constexpr const auto& get(const T& t) noexcept
{
    return t.template get<I>();
}

template <typename E, size_t I, Tuple T>
inline constexpr auto& get(const T& t) noexcept
{
    return t.template get<E, I>();
}

template <typename E, size_t I, Tuple T>
inline constexpr const auto& get(const T& t) noexcept
{
    return t.template get<E, I>();
}

template<typename... Ts>
constexpr tuple<Ts...> make_tuple(Ts&&... args)
{
    return tuple<Ts...>(std::forward<Ts...>(args...));
}

template<typename T> requires (is_template_of(^^T, ^^typelist))
constexpr auto make_tuple()
{
    return typename[:std::meta::substitute(^^tuple, template_arguments_of(^^T)):]();
}

namespace detail
{
    template <typename Func, Tuple T, size_t... I>
    inline constexpr decltype(auto) _apply_helper(Func&& func, T&& t, const std::index_sequence<I...>&)
    {
        return func(t.template get<I>()...);
    }
}

template <typename Func, typename... Ts>
inline constexpr decltype(auto) apply(Func&& func, tuple<Ts...>&& t)
{
    static constexpr auto indices = std::make_index_sequence<sizeof...(Ts)>();
    return detail::_apply_helper(std::forward<Func>(func), std::forward<tuple<Ts...>>(t), std::make_index_sequence<sizeof...(Ts)>());
}
}

#include "detail/tuple.inl"