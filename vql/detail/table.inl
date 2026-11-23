#pragma once

namespace vql
{
namespace detail
{
    consteval bool do_public_members_of_have_trait(meta_info r, auto f)
    {
        for (meta_info m : nonstatic_data_members_of(r, std::meta::access_context::current()))
            if (!f(type_of(m)))
                return false;
        return true;
    }

    consteval meta_info next_member(meta_info l, meta_info m)
    {
        meta_infos members = nonstatic_data_members_of(l, std::meta::access_context::current());
        if (m == ^^null_type)
            return members[0];
        for (size_t i = 0zu; i < members.size() - 1zu; ++i)
            if (members[i] == m)
                return members[i + 1];
        return ^^null_type;
    }

    consteval bool has_next_member(meta_info l, meta_info m)
    {
        return next_member(l, m) != ^^null_type;
    }
}

consteval bool is_table_layout_like(meta_info r)
{
    return is_class_type(r) &&
        nonstatic_data_members_of(r, std::meta::access_context::current()).size() > 0 &&
        detail::do_public_members_of_have_trait(r, std::meta::is_move_constructible_type) &&
        detail::do_public_members_of_have_trait(r, std::meta::is_move_assignable_type);
}

template<TableLayoutLike Layout, TableMetadata Metadata>
template<typename T, size_t Index>
size_t table<Layout, Metadata>::column_view<T, Index>::count() const noexcept
{
    return reinterpret_cast<const table<Layout, Metadata>*>(reinterpret_cast<const std::byte*>(this) - s_offset)->count();
}

template<TableLayoutLike Layout, TableMetadata Metadata>
template<typename T, size_t Index>
size_t table<Layout, Metadata>::column_view<T, Index>::capacity() const noexcept
{
    return reinterpret_cast<const table<Layout, Metadata>*>(reinterpret_cast<const std::byte*>(this) - s_offset)->capacity();
}

template<TableLayoutLike Layout, TableMetadata Metadata>
template<typename... Ts>
consteval bool table<Layout, Metadata>::matches()
{
    if constexpr (size != sizeof...(Ts)) return false;
    constexpr meta_infos members = std::meta::nonstatic_data_members_of(^^table_layout_wrapper<Layout>, std::meta::access_context::current());
    template for (constexpr auto I : std::views::iota(0zu, size))
        if constexpr (^^Ts...[I] != type_of(members[I]))
            return false;
    return true;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
void table<Layout, Metadata>::allocate_and_initialize_views()
{
    std::array<size_t, size> sizes;
    size_t total{};
    template for (constexpr auto I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        using view_type = [:std::meta::type_of(v):];
        sizes[I] = next_multiple_of_power_of_2<cache_line_size>(sizeof(typename view_type::value_t) * m_capacity);
        total += sizes[I];
    }
    m_data = allocate_aligned_bytes(total);
    std::byte* block = reinterpret_cast<std::byte*>(m_data.get());
    template for (constexpr auto I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        using view_type = [:std::meta::type_of(v):];
        views.[:v:].m_ptr = reinterpret_cast<view_type::ptr_t>(block);
        block += sizes[I];
    }
}

template<TableLayoutLike Layout, TableMetadata Metadata>
void table<Layout, Metadata>::reallocate(size_t new_capacity)
{
    new_capacity = new_capacity > 0zu ? new_capacity : 1zu;
    table t(new_capacity);
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        const auto& view = views.[:v:];
        auto& new_view = t.views.[:v:];
        std::uninitialized_move(view.cbegin(), view.cend(), new_view.begin());
    }
    swap_views(t);
    m_capacity = new_capacity;
    m_data.swap(t.m_data);
}

template<TableLayoutLike Layout, TableMetadata Metadata>
constexpr size_t table<Layout, Metadata>::calculate_new_capacity_for(size_t needed) const noexcept
{
    assert(m_capacity < needed);
    return next_power_of_2(needed);
}

template<TableLayoutLike Layout, TableMetadata Metadata>
std::pair<size_t, size_t> table<Layout, Metadata>::reallocate_if_needed(size_t n)
{
    if (n > m_capacity) reallocate(calculate_new_capacity_for(n));
    return {m_count, n};
}

template<TableLayoutLike Layout, TableMetadata Metadata>
table<Layout, Metadata>::table(size_t initial_capacity) : m_capacity(initial_capacity), m_count()
{
    assert(initial_capacity > 0zu);
    allocate_and_initialize_views();
}

template<TableLayoutLike Layout, TableMetadata Metadata>
table<Layout, Metadata>::table(size_t initial_count, const table_layout_wrapper<Layout>& w) _VQL_TRAIT_SUFFIX(copy_constructible)
    : m_capacity(calculate_new_capacity_for(initial_count)), m_count(0zu)
{
    assert(initial_count > 0zu);
    allocate_and_initialize_views();
    append(initial_count, w);
}

template<TableLayoutLike Layout, TableMetadata Metadata>
template<typename... Ts>
table<Layout, Metadata>::table(size_t initial_count, const Ts&... args) _VQL_TRAIT_SUFFIX2(copy_constructible)
    : m_capacity(calculate_new_capacity_for(initial_count)), m_count(0zu)
{
    assert(initial_count > 0zu);
    allocate_and_initialize_views();
    append(initial_count, args...);
}

template<TableLayoutLike Layout, TableMetadata Metadata>
table<Layout, Metadata>::table(const table& other) _VQL_TRAIT_SUFFIX(copy_constructible) : m_capacity(other.m_capacity), m_count(other.m_count)
{
    allocate_and_initialize_views();
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        auto& view = views.[:v:];
        const auto& other_view = other.views.[:v:];
        std::uninitialized_copy(other_view.cbegin(), other_view.cend(), view.begin());
    }
}

template<TableLayoutLike Layout, TableMetadata Metadata>
table<Layout, Metadata>::table(table&& other) _VQL_TRAIT_SUFFIX(move_constructible)
    : m_capacity(other.m_capacity), m_count(other.m_count), views(), m_data(std::move(other.m_data))
{
    template for (constexpr auto I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        views.[:v:].m_ptr = other.views.[:v:].m_ptr;
    }
}

template<TableLayoutLike Layout, TableMetadata Metadata>
table<Layout, Metadata>& table<Layout, Metadata>::operator=(const table& other) _VQL_TRAIT_SUFFIX(copy_assignable)
{
    m_capacity = other.m_capacity;
    m_count = other.m_count;
    m_data.reset();
    allocate_and_initialize_views();
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        auto& view = views.[:v:];
        const auto& other_view = other.views.[:v:];
        std::uninitialized_copy(other_view.cbegin(), other_view.cend(), view.begin());
    }
    return *this;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
table<Layout, Metadata>& table<Layout, Metadata>::operator=(table&& other) _VQL_TRAIT_SUFFIX(move_constructible)
{
    m_capacity = other.m_capacity;
    m_count = other.m_count;
    template for (constexpr auto I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        views.[:v:].m_ptr = other.views.[:v:].m_ptr;
    }
    m_data = std::move(other.m_data);
    return *this;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
size_t table<Layout, Metadata>::push_back() _VQL_TRAIT_SUFFIX(default_constructible)
{
    auto [old_count, new_count] = reallocate_if_needed(m_count + 1zu);
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        std::construct_at(views.[:v:].m_ptr + m_count);
    }
    m_count = new_count;
    return old_count;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
size_t table<Layout, Metadata>::push_back(const table_layout_wrapper<Layout>& w) _VQL_TRAIT_SUFFIX(copy_constructible)
{
    auto [old_count, new_count] = reallocate_if_needed(m_count + 1zu);
    constexpr meta_infos members = std::meta::nonstatic_data_members_of(^^table_layout_wrapper<Layout>, std::meta::access_context::current());
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        std::construct_at(&views.[:v:].end(), w.[:members[I]:]);
    }
    m_count = new_count;
    return old_count;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
size_t table<Layout, Metadata>::push_back(table_layout_wrapper<Layout>&& w) _VQL_TRAIT_SUFFIX(move_constructible)
{
    auto [old_count, new_count] = reallocate_if_needed(m_count + 1zu);
    constexpr meta_infos members = std::meta::nonstatic_data_members_of(^^table_layout_wrapper<Layout>, std::meta::access_context::current());
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        std::construct_at(&views.[:v:].end(), std::move(w.[:members[I]:]));
    }
    m_count = new_count;
    return old_count;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
template<typename... Ts>
size_t table<Layout, Metadata>::push_back(const Ts&... args) _VQL_TRAIT_SUFFIX2(copy_constructible)
{
    auto [old_count, new_count] = reallocate_if_needed(m_count + 1zu);
    constexpr meta_infos members = std::meta::nonstatic_data_members_of(^^table_layout_wrapper<Layout>, std::meta::access_context::current());
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        std::construct_at(&views.[:v:].end(), args...[I]);
    }
    m_count = new_count;
    return old_count;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
template<typename... Ts>
size_t table<Layout, Metadata>::push_back(Ts&&... args) _VQL_TRAIT_SUFFIX2(move_constructible)
{
    auto [old_count, new_count] = reallocate_if_needed(m_count + 1zu);
    constexpr meta_infos members = std::meta::nonstatic_data_members_of(^^table_layout_wrapper<Layout>, std::meta::access_context::current());
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        std::construct_at(&views.[:v:].end(), std::move(args...[I]));
    }
    m_count = new_count;
    return old_count;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
size_t table<Layout, Metadata>::append(size_t n) _VQL_TRAIT_SUFFIX(default_constructible)
{
    if (n == 0zu) return m_count;
    auto [old_count, new_count] = reallocate_if_needed(m_count + n);
    m_count = new_count;
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        auto& view = views.[:v:];
        auto ite = view.end();
        for (auto it = view.begin() + old_count; it != ite; it++)
            std::construct_at(&it);
    }
    return old_count;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
size_t table<Layout, Metadata>::append(size_t n, const table_layout_wrapper<Layout>& w) _VQL_TRAIT_SUFFIX(copy_constructible)
{
    if (n == 0zu) return m_count;
    auto [old_count, new_count] = reallocate_if_needed(m_count + n);
    m_count = new_count;
    constexpr meta_infos members = std::meta::nonstatic_data_members_of(^^table_layout_wrapper<Layout>, std::meta::access_context::current());
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        auto& view = views.[:v:];
        auto ite = view.end();
        for (auto it = view.begin() + old_count; it != ite; it++)
            std::construct_at(&it, w.[:members[I]:]);
    }
    return old_count;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
template<typename... Ts>
size_t table<Layout, Metadata>::append(size_t n, const Ts&... args) _VQL_TRAIT_SUFFIX2(copy_constructible)
{
    if (n == 0zu) return m_count;
    auto [old_count, new_count] = reallocate_if_needed(m_count + n);
    m_count = new_count;
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        auto& view = views.[:v:];
        auto ite = view.end();
        for (auto it = view.begin() + old_count; it != ite; it++)
            std::construct_at(&it, args...[I]);
    }
    return old_count;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
size_t table<Layout, Metadata>::append(const table<Layout, Metadata>& other) _VQL_TRAIT_SUFFIX(copy_constructible)
{
    if (other.m_count == 0zu) return m_count;
    auto [old_count, new_count] = reallocate_if_needed(m_count + other.m_count);
    m_count = new_count;
    constexpr meta_infos members = std::meta::nonstatic_data_members_of(^^table_layout_wrapper<Layout>, std::meta::access_context::current());
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        auto& view = views.[:v:];
        const auto& other_view = other.views.[:v:];
        std::uninitialized_copy(other_view.cbegin(), other_view.cend(), view.end());
    }
    return old_count;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
size_t table<Layout, Metadata>::append(table<Layout, Metadata>&& other) _VQL_TRAIT_SUFFIX(move_constructible)
{
    if (other.m_count == 0zu) return m_count;
    auto [old_count, new_count] = reallocate_if_needed(m_count + other.m_count);
    m_count = new_count;
    constexpr meta_infos members = std::meta::nonstatic_data_members_of(^^table_layout_wrapper<Layout>, std::meta::access_context::current());
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        auto& view = views.[:v:];
        const auto& other_view = other.views.[:v:];
        std::uninitialized_move(other_view.cbegin(), other_view.cend(), view.end());
    }
    return old_count;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
void table<Layout, Metadata>::resize(size_t n) _VQL_TRAIT_SUFFIX(default_constructible)
{
    if (n == m_count) return;
    else if (n > m_count)
    {
        append(n - m_count);
        return;
    }
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        auto& view = views.[:v:];
        std::destroy(view.cbegin() + n, view.cend());
    }
    m_count = n;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
void table<Layout, Metadata>::resize(size_t n, const table_layout_wrapper<Layout>& w) _VQL_TRAIT_SUFFIX(copy_constructible)
{
    if (n == m_count) return;
    else if (n > m_count)
    {
        append(n - m_count, w);
        return;
    }
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        auto& view = views.[:v:];
        std::destroy(view.cbegin() + n, view.cend());
    }
    m_count = n;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
template<typename... Ts>
void table<Layout, Metadata>::resize(size_t n, const Ts&... args) _VQL_TRAIT_SUFFIX2(copy_constructible)
{
    if (n == m_count) return;
    else if (n > m_count)
    {
        append(n - m_count, args...);
        return;
    }
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        auto& view = views.[:v:];
        std::destroy(view.cbegin() + n, view.cend());
    }
    m_count = n;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
void table<Layout, Metadata>::swap_views(table& other) noexcept
{
    template for (constexpr auto I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        std::swap(views.[:v:].m_ptr, other.views.[:v:].m_ptr);
    }
}

template<TableLayoutLike Layout, TableMetadata Metadata>
void table<Layout, Metadata>::swap(table<Layout, Metadata>& other) noexcept
{
    std::swap(m_capacity, other.m_capacity);
    std::swap(m_count, other.m_count);
    swap_views(other);
    std::swap(m_data, other.m_data);
}

template<TableLayoutLike Layout, TableMetadata Metadata>
void table<Layout, Metadata>::clear() noexcept
{
    template for (constexpr size_t I : std::views::iota(0zu, size))
    {
        constexpr meta_info v = view_members[I];
        if (!is_trivially_destructible_type(type_of(v)))
        {
            const auto& view = views.[:v:];
            std::ranges::destroy(view.cbegin(), view.cend());
        }
    }
    m_count = 0zu;
}

template<TableLayoutLike Layout, TableMetadata Metadata>
void table<Layout, Metadata>::reserve(size_t new_capacity) { if (new_capacity > m_capacity) reallocate(new_capacity); }

template<TableLayoutLike Layout, TableMetadata Metadata>
void table<Layout, Metadata>::shrink_to_fit() { if (m_count < m_capacity) reallocate(m_count); }
}