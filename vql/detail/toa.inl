#pragma once

#include <array>

namespace vql
{
template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
void toa<Ts...>::allocate_and_initialize_views()
{
    std::array<size_t, size> sizes;
    size_t total{};
    template for (constexpr auto I : std::views::iota(0zu, size))
    {
        sizes[I] = next_multiple_of_cache_line_size(sizeof(Ts...[I]) * m_capacity);
        total += sizes[I];
    }
    m_data = allocate_smart_cache_aligned_bytes(total);
    std::byte* block = reinterpret_cast<std::byte*>(m_data.get());
    template for (constexpr auto I : std::views::iota(0zu, size))
    {
        m_views.template get<I>() = reinterpret_cast<Ts...[I]*>(block);
        block += sizes[I];
    }
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
toa<Ts...>::toa(size_t initial_capacity) : m_capacity(initial_capacity), m_count(0zu)
{
    assert(initial_capacity > 0zu);
    allocate_and_initialize_views();
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
toa<Ts...>::toa(size_t initial_count, const tuple<Ts...>& t) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...)
    : m_capacity(calculate_new_capacity_for(initial_count)), m_count(0zu)
{
    assert(m_capacity > 0zu);
    allocate_and_initialize_views();
    append(initial_count, t);
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
toa<Ts...>::toa(size_t initial_count, const Ts&... args) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...)
    : m_capacity(calculate_new_capacity_for(initial_count)), m_count(0zu)
{
    assert(m_capacity > 0zu);
    allocate_and_initialize_views();
    append(initial_count, args...);
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
toa<Ts...>::toa(const toa& other) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...)
    : m_capacity(other.m_capacity), m_count(other.m_count)
{
    allocate_and_initialize_views();
    template for (constexpr size_t I : std::views::iota(0zu, size))
        std::uninitialized_copy(other.template cbegin<I>(), other.template cend<I>(), begin<I>());
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
toa<Ts...>::toa(toa&& other) noexcept
    : m_views(std::move(other.m_views)), m_capacity(other.m_capacity), m_count(other.m_count), m_data(std::move(other.m_data)) {}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
toa<Ts...>& toa<Ts...>::operator=(const toa& other) noexcept((std::is_nothrow_copy_assignable_v<Ts> && ...)) requires (std::is_copy_assignable_v<Ts> && ...)
{
    m_capacity = other.m_capacity;
    m_count = other.m_count;
    m_data.reset();
    allocate_and_initialize_views();
    template for (constexpr size_t I : std::views::iota(0zu, size))
        std::uninitialized_copy(other.template cbegin<I>(), other.template cend<I>(), begin<I>());
    return *this;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
toa<Ts...>& toa<Ts...>::operator=(toa&& other) noexcept
{
    m_views = std::move(other.m_views);
    m_capacity = other.m_capacity;
    m_count = other.m_count;
    m_data = std::move(other.m_data);
    return *this;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
void toa<Ts...>::swap(toa& other) noexcept
{
    std::swap(m_views, other.m_views);
    std::swap(m_count, other.m_count);
    std::swap(m_capacity, other.m_capacity);
    std::swap(m_data, other.m_data);
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
void toa<Ts...>::reallocate(size_t new_capacity)
{
    new_capacity = new_capacity > 0zu ? new_capacity : 1zu;
    toa t(new_capacity);
    template for (constexpr size_t I : std::views::iota(0zu, size))
        std::uninitialized_copy(cbegin<I>(), cend<I>(), t.template begin<I>());
    m_views.swap(t.m_views);
    m_capacity = new_capacity;
    m_data.swap(t.m_data);
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
constexpr size_t toa<Ts...>::calculate_new_capacity_for(size_t needed) const noexcept
{
    assert(m_capacity < needed);
    return next_power_of_2(needed);
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
std::pair<size_t, size_t> toa<Ts...>::reallocate_if_needed(size_t n)
{
    if (n > m_capacity) reallocate(calculate_new_capacity_for(n));
    return {m_count, n};
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
size_t toa<Ts...>::push_back()  noexcept((std::is_nothrow_default_constructible_v<Ts> && ...)) requires (std::is_default_constructible_v<Ts> && ...)
{
    auto [old_count, new_count] = reallocate_if_needed(m_count + 1zu);
    template for (constexpr size_t I : std::views::iota(0zu, size)) std::construct_at(&end<I>());
    m_count = new_count;
    return old_count;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
size_t toa<Ts...>::push_back(const tuple<Ts...>& t) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...)
{
    auto [old_count, new_count] = reallocate_if_needed(m_count + 1zu);
    template for (constexpr size_t I : std::views::iota(0zu, size)) std::construct_at(&end<I>(), t.template get<I>());
    m_count = new_count;
    return old_count;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
size_t toa<Ts...>::push_back(tuple<Ts...>&& t) noexcept
{
    auto [old_count, new_count] = reallocate_if_needed(m_count + 1zu);
    template for (constexpr size_t I : std::views::iota(0zu, size)) std::construct_at(&end<I>(), std::move(t.template get<I>()));
    m_count = new_count;
    return old_count;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
size_t toa<Ts...>::push_back(const Ts&... args) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...)
{
    auto [old_count, new_count] = reallocate_if_needed(m_count + 1zu);
    template for (constexpr size_t I : std::views::iota(0zu, size)) std::construct_at(&end<I>(), args...[I]);
    m_count = new_count;
    return old_count;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
size_t toa<Ts...>::push_back(Ts&&... args) noexcept
{
    auto [old_count, new_count] = reallocate_if_needed(m_count + 1zu);
    template for (constexpr size_t I : std::views::iota(0zu, size)) std::construct_at(&end<I>(), std::move(args...[I]));
    m_count = new_count;
    return old_count;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
size_t toa<Ts...>::append(size_t n) noexcept((std::is_nothrow_default_constructible_v<Ts> && ...)) requires (std::is_default_constructible_v<Ts> && ...)
{
    if (n == 0zu) return m_count;
    auto [old_count, new_count] = reallocate_if_needed(m_count + n);
    m_count = new_count;
    template for (constexpr size_t I : std::views::iota(0zu, size))
        for (auto it = begin<I>() + old_count; it != end<I>(); it++)
            std::construct_at(&it);
    return old_count;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
size_t toa<Ts...>::append(size_t n, const tuple<Ts...>& t) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...)
{
    if (n == 0zu) return m_count;
    auto [old_count, new_count] = reallocate_if_needed(m_count + n);
    m_count = new_count;
    template for (constexpr size_t I : std::views::iota(0zu, size))
        for (auto it = begin<I>() + old_count; it != end<I>(); it++)
            std::construct_at(&it, t.template get<I>());
    return old_count;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
size_t toa<Ts...>::append(size_t n, const Ts&... args) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...)
{
    if (n == 0zu) return m_count;
    auto [old_count, new_count] = reallocate_if_needed(m_count + n);
    m_count = new_count;
    template for (constexpr size_t I : std::views::iota(0zu, size))
        for (auto it = begin<I>() + old_count; it != end<I>(); it++)
            std::construct_at(&it, args...[I]);
    return old_count;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
size_t toa<Ts...>::append(const toa& other) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...)
{
    if (other.m_count == 0zu) return m_count;
    auto [old_count, new_count] = reallocate_if_needed(m_count + other.m_count);
    template for (constexpr size_t I : std::views::iota(0zu, size))
        std::uninitialized_copy(other.template cbegin<I>(), other.template cend<I>(), end<I>());
    m_count = new_count;
    return old_count;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
size_t toa<Ts...>::append(toa&& other) noexcept
{
    if (other.m_count == 0zu) return m_count;
    auto [old_count, new_count] = reallocate_if_needed(m_count + other.m_count);
    template for (constexpr size_t I : std::views::iota(0zu, size))
        std::uninitialized_move(other.template cbegin<I>(), other.template cend<I>(), end<I>());
    m_count = new_count;
    return old_count;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
void toa<Ts...>::resize(size_t n) noexcept((std::is_nothrow_default_constructible_v<Ts> && ...)) requires (std::is_default_constructible_v<Ts> && ...)
{
    if (n == m_count) return;
    else if (n > m_count)
    {
        append(n - m_count);
        return;
    }

    template for (constexpr size_t I : std::views::iota(0zu, size))
        std::destroy(cbegin<I>() + n, cend<I>());
    m_count = n;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
void toa<Ts...>::resize(size_t n, const tuple<Ts...>& t) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...)
{
    if (n == m_count) return;
    else if (n > m_count)
    {
        append(n - m_count, t);
        return;
    }

    template for (constexpr size_t I : std::views::iota(0zu, size))
        std::destroy(cbegin<I>() + n, cend<I>());
    m_count = n;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
void toa<Ts...>::resize(size_t n, const Ts&... args) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...)
{
    if (n == m_count) return;
    else if (n > m_count)
    {
        append(n - m_count, args...);
        return;
    }

    template for (constexpr size_t I : std::views::iota(0zu, size))
        std::destroy(cbegin<I>() + n, cend<I>());
    m_count = n;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
void toa<Ts...>::clear() noexcept
{
    template for (constexpr size_t I : std::views::iota(0zu, size))
        if constexpr (!std::is_trivially_destructible_v<Ts...[I]>)
            std::ranges::destroy(cbegin<I>(), cend<I>());
    m_count = 0zu;
}

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
void toa<Ts...>::reserve(size_t new_capacity) { if (new_capacity > m_capacity) reallocate(new_capacity); }

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
void toa<Ts...>::shrink_to_fit() { if (m_count < m_capacity) reallocate(m_count); }
}
