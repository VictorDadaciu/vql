#pragma once

#include "iterator.h"
#include "utils.h"

#include <ranges>

namespace vql
{
namespace detail
{
    consteval bool do_public_members_of_have_trait(meta_info, auto);
    consteval meta_info next_member(meta_info, meta_info);
    consteval bool has_next_member(meta_info, meta_info);
}

consteval bool is_table_layout_like(meta_info);

template<class T>
concept TableLayoutLike = is_table_layout_like(^^T);

struct flatten_t {};
static constexpr flatten_t flatten = {};

struct handled {};
struct vacuumed {};

template<class T>
concept TableMetadatum = (^^T == ^^handled) || (^^T == ^^vacuumed);

template<TableMetadatum... Specs> requires (is_set({ ^^Specs... }))
struct table_metadata
{
public:
    static constexpr bool is_handled  = is_in_types(^^handled,  { ^^Specs... });
    static constexpr bool is_vacuumed = is_in_types(^^vacuumed, { ^^Specs... });
};

template<class T>
concept TableMetadata = (is_template_of(^^T, ^^table_metadata));

#define _VQL_TRAIT_SUFFIX(_trait_func) \
noexcept((std::meta::is_nothrow_##_trait_func##_type(^^table_layout_wrapper<Layout>))) \
requires (std::meta::is_##_trait_func##_type(^^table_layout_wrapper<Layout>))

#define _VQL_TRAIT_SUFFIX2(_trait_func) \
noexcept((std::meta::is_nothrow_##_trait_func##_type(^^table_layout_wrapper<Layout>))) \
requires (std::meta::is_##_trait_func##_type(^^table_layout_wrapper<Layout>) && table<Layout, Metadata>::template matches<Ts...>())

template<TableLayoutLike Layout, TableMetadata Metadata = table_metadata<>>
class table
{
public:
    using layout = Layout;
    using metadata = Metadata;

    template<typename T, size_t Index>
    class column_view
    {
        static constexpr size_t s_offset = 16zu + Index * sizeof(column_view);

    public:
        using value_t = T;
        using ref_t = value_t&;
        using ptr_t = value_t*;

        size_t count() const noexcept;
        size_t capacity() const noexcept;

        ref_t operator[](size_t i)
        {
            assert(i < count());
            return m_ptr[i];
        }
        const ref_t operator[](size_t i) const
        {
            assert(i < count());
            return m_ptr[i];
        }

        iterator<T> begin() noexcept { return iterator<T>(m_ptr); }
        const_iterator<T> cbegin() const noexcept { return const_iterator<T>(m_ptr); }
        reverse_iterator<T> rbegin() noexcept { return reverse_iterator<T>(m_ptr + count() - 1); }
        const_reverse_iterator<T> crbegin() const noexcept { return const_reverse_iterator<T>(m_ptr + count() - 1); }

        iterator<T> end() noexcept { return iterator<T>(m_ptr + count()); }
        const_iterator<T> cend() const noexcept { return const_iterator<T>(m_ptr + count()); }
        reverse_iterator<T> rend() noexcept { return reverse_iterator<T>(m_ptr - 1); }
        const_reverse_iterator<T> crend() const noexcept { return const_reverse_iterator<T>(m_ptr - 1); }

    private:
        friend table;

        column_view() : m_ptr() {}
        column_view(const ptr_t) = delete;
        column_view(const column_view&) = delete;
        column_view(column_view&&) = delete;

        column_view& operator=(const column_view&) = delete;
        column_view& operator=(column_view&&) = delete;

        ptr_t m_ptr{};
    };

    template<TableLayoutLike L>
    struct table_layout_wrapper;
    using row = table_layout_wrapper<Layout>;

private:
    void allocate_and_initialize_views();
    void reallocate(size_t);
    constexpr size_t calculate_new_capacity_for(size_t) const noexcept;
    std::pair<size_t, size_t> reallocate_if_needed(size_t);
    void swap_views(table&) noexcept;

    // make row
    consteval
    {
        auto ctx = std::meta::access_context::current();
        meta_infos layout_stack;
        meta_infos members_stack;
        std::vector<meta_infos> member_specs_stack;
        meta_info l = ^^Layout;
        meta_info m = ^^null_type;
        meta_infos cm;
        while (detail::has_next_member(l, m))
        {
            m = detail::next_member(l, m);
            if (annotations_of_with_type(m, ^^flatten_t).size() == 0zu || !is_class_type(type_of(m)) || !is_table_layout_like(l))
            {
                // TODO: warning about flattening non-class types or not table-layout-like types
                cm.push_back(data_member_spec(type_of(m), { .name = std::meta::identifier_of(m) }));
            }
            else
            {
                layout_stack.push_back(l);
                members_stack.push_back(m);
                member_specs_stack.push_back(cm);
                l = type_of(m);
                m = ^^null_type;
                cm = meta_infos{};
                continue;
            }
            while (!detail::has_next_member(l, m))
            {
                meta_info inner = substitute(^^table_layout_wrapper, { l });
                define_aggregate(inner, cm);
                if (layout_stack.size() > 0)
                {
                    l = layout_stack.back();
                    layout_stack.pop_back();
                    m = members_stack.back();
                    members_stack.pop_back();
                    cm = member_specs_stack.back();
                    member_specs_stack.pop_back();
                    for (auto inner_m : nonstatic_data_members_of(inner, ctx))
                        cm.push_back(data_member_spec(type_of(inner_m), { .name = std::meta::identifier_of(inner_m) }));
                }
                else break;
            }
        }
    }

    struct column_views;
    // make column_views
    consteval
    {
        // make column_views
        meta_infos member_specs;
        constexpr auto ctx = std::meta::access_context::current();
        constexpr auto row_members = std::define_static_array(nonstatic_data_members_of(dealias(^^row), ctx));
        template for (constexpr auto I : std::views::iota(0zu, row_members.size()))
        {
            constexpr meta_info m = row_members[I];
            constexpr meta_info view_type = substitute(^^column_view, std::vector{type_of(m), std::meta::reflect_constant(I)});
            constexpr meta_info mem_spec = std::meta::data_member_spec(view_type, { .name = std::meta::identifier_of(m) });
            member_specs.push_back(mem_spec);
        }
        define_aggregate(^^column_views, member_specs);
    }
    static constexpr auto view_members = std::define_static_array(nonstatic_data_members_of(^^column_views, std::meta::access_context::current()));
    static constexpr size_t size = view_members.size();

    template<typename... Ts>
    static consteval bool matches();

public:
    explicit table(size_t=1zu);

    table(size_t, const table_layout_wrapper<Layout>&) _VQL_TRAIT_SUFFIX(copy_constructible);
    template<typename... Ts> table(size_t, const Ts&...) _VQL_TRAIT_SUFFIX2(copy_constructible);
    table(const table&) _VQL_TRAIT_SUFFIX(copy_constructible);
    table(table&&) _VQL_TRAIT_SUFFIX(move_constructible);

    table& operator=(const table&) _VQL_TRAIT_SUFFIX(copy_assignable);
    table& operator=(table&&) _VQL_TRAIT_SUFFIX(move_constructible);

    size_t count() const noexcept { return m_count; }
    size_t capacity() const noexcept { return m_capacity; }

    size_t push_back() _VQL_TRAIT_SUFFIX(default_constructible);
    size_t push_back(const table_layout_wrapper<Layout>&) _VQL_TRAIT_SUFFIX(copy_constructible);
    size_t push_back(table_layout_wrapper<Layout>&&)_VQL_TRAIT_SUFFIX(move_constructible);
    template<typename... Ts> size_t push_back(const Ts&...) _VQL_TRAIT_SUFFIX2(copy_constructible);
    template<typename... Ts> size_t push_back(Ts&&...) _VQL_TRAIT_SUFFIX2(move_constructible);

    size_t append(size_t) _VQL_TRAIT_SUFFIX(default_constructible);
    size_t append(size_t, const table_layout_wrapper<Layout>&) _VQL_TRAIT_SUFFIX(copy_constructible);
    template<typename... Ts> size_t append(size_t, const Ts&...) _VQL_TRAIT_SUFFIX2(copy_constructible);
    size_t append(const table&) _VQL_TRAIT_SUFFIX(copy_constructible);
    size_t append(table&&) _VQL_TRAIT_SUFFIX(move_constructible);

    void resize(size_t) _VQL_TRAIT_SUFFIX(default_constructible);
    void resize(size_t, const table_layout_wrapper<Layout>&) _VQL_TRAIT_SUFFIX(copy_constructible);
    template<typename... Ts> void resize(size_t, const Ts&...) _VQL_TRAIT_SUFFIX2(copy_constructible);

    void swap(table& other) noexcept;
    friend void swap(table& a, table& b) noexcept { a.swap(b); }

    void clear() noexcept;
    void reserve(size_t);
    void shrink_to_fit();

// data
private:
    size_t m_capacity;
    size_t m_count;
public:
    column_views views;
private:
    aligned_bytes m_data;
};
}

#include "detail/table.inl"
