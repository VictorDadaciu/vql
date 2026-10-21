#pragma once

#include "tuple.h"
#include "utils.h"

#include <cassert>
#include <iterator>

namespace vql
{
template <typename T>
class _iterator_base
{
public:
    using value_type = T;
    using iterator_category = std::contiguous_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    _iterator_base(pointer ptr=nullptr) noexcept : m_it(ptr) {};
    _iterator_base(const _iterator_base& other) noexcept : m_it(other.m_it) {}
    _iterator_base(_iterator_base&& other) noexcept : m_it(other.m_it) { other.m_it = nullptr; }
    _iterator_base& operator=(const _iterator_base& other) noexcept { m_it = other.m_it; return *this; }
    _iterator_base& operator=(_iterator_base&& other) noexcept { m_it = other.m_it; other.m_it = nullptr; return *this; }

    ~_iterator_base() = default;

    reference operator*() const { return *m_it; }
    reference operator&() const { return m_it; }
    pointer operator->() const noexcept { return m_it; }
    explicit operator reference() { return *m_it; }
    explicit operator pointer() noexcept { return m_it; }
    explicit operator const reference() const { return *m_it; }
    explicit operator const pointer() const noexcept { return m_it; }
    explicit operator bool() const noexcept { return m_it; }

    reference operator[](difference_type x) { return m_it[x]; }
    const reference operator[](difference_type x) const { return m_it[x]; }

    friend bool operator== (const _iterator_base& a, const _iterator_base& b) noexcept { return a.m_it ==  b.m_it; }
    friend bool operator!= (const _iterator_base& a, const _iterator_base& b) noexcept { return a.m_it !=  b.m_it; }
    friend bool operator<  (const _iterator_base& a, const _iterator_base& b) noexcept { return a.m_it <   b.m_it; }
    friend bool operator<= (const _iterator_base& a, const _iterator_base& b) noexcept { return a.m_it <=  b.m_it; }
    friend bool operator>  (const _iterator_base& a, const _iterator_base& b) noexcept { return a.m_it >   b.m_it; }
    friend bool operator>= (const _iterator_base& a, const _iterator_base& b) noexcept { return a.m_it >=  b.m_it; }
    friend auto operator<=>(const _iterator_base& a, const _iterator_base& b) noexcept { return a.m_it <=> b.m_it; }

protected:
    pointer m_it;
};

template <typename T>
class _iterator : public _iterator_base<T>
{
public:
    using base = _iterator_base<T>;
    using value_type = base::value_type;
    using iterator_category = base::iterator_category;
    using difference_type = base::difference_type;
    using pointer = base::pointer;
    using reference = base::reference;

    _iterator& operator++() noexcept { ++m_it; return *this; }
    _iterator& operator--() noexcept { --m_it; return *this; }
    _iterator  operator++(int) noexcept { return _iterator(m_it++); }
    _iterator  operator--(int) noexcept { return _iterator(m_it--); }
    _iterator& operator+=(_iterator other) noexcept { m_it += other.m_it; return *this; }
    _iterator& operator-=(_iterator other) noexcept { m_it -= other.m_it; return *this; }
    _iterator& operator+=(pointer x) noexcept { m_it += x; return *this; }
    _iterator& operator-=(pointer x) noexcept { m_it -= x; return *this; }
    _iterator& operator+=(difference_type x) noexcept { m_it += x; return *this; }
    _iterator& operator-=(difference_type x) noexcept { m_it -= x; return *this; }
    friend _iterator operator+ (const _iterator& it, _iterator other) noexcept { return _iterator(it.m_it + other.m_it); }
    friend _iterator operator- (const _iterator& it, _iterator other) noexcept { return _iterator(it.m_it - other.m_it); }
    friend _iterator operator+ (const _iterator& it, pointer x) noexcept { return _iterator(it.m_it + x); }
    friend _iterator operator- (const _iterator& it, pointer x) noexcept { return _iterator(it.m_it - x); }
    friend _iterator operator+ (const _iterator& it, difference_type x) noexcept { return _iterator(it.m_it + x); }
    friend _iterator operator- (const _iterator& it, difference_type x) noexcept { return _iterator(it.m_it - x); }

protected:
    using base::m_it;
};

template <typename T>
class _reverse_iterator : public _iterator_base<T>
{
public:
    using base = _iterator_base<T>;
    using value_type = base::value_type;
    using iterator_category = base::iterator_category;
    using difference_type = base::difference_type;
    using pointer = base::pointer;
    using reference = base::reference;

    _reverse_iterator& operator++() noexcept { --m_it; return *this; }
    _reverse_iterator& operator--() noexcept { ++m_it; return *this; }
    _reverse_iterator  operator++(int) noexcept { return _reverse_iterator(m_it--); }
    _reverse_iterator  operator--(int) noexcept { return _reverse_iterator(m_it++); }
    _reverse_iterator& operator+=(difference_type x) noexcept { m_it -= x; return *this; }
    _reverse_iterator& operator-=(difference_type x) noexcept { m_it += x; return *this; }
    friend _reverse_iterator operator+ (const _reverse_iterator& it, _reverse_iterator other) noexcept { return __reverse_iterator(it.m_it - other.m_it); }
    friend _reverse_iterator operator- (const _reverse_iterator& it, _reverse_iterator other) noexcept { return __reverse_iterator(it.m_it + other.m_it); }
    friend _reverse_iterator operator+ (const _reverse_iterator& it, pointer x) noexcept { return _reverse_iterator(it.m_it - x); }
    friend _reverse_iterator operator- (const _reverse_iterator& it, pointer x) noexcept { return _reverse_iterator(it.m_it + x); }
    friend _reverse_iterator operator+ (const _reverse_iterator& it, difference_type x) noexcept { return _reverse_iterator(it.m_it - x); }
    friend _reverse_iterator operator- (const _reverse_iterator& it, difference_type x) noexcept { return _reverse_iterator(it.m_it + x); }

protected:
    using base::m_it;
};

template <typename T>
using iterator = _iterator<T>;
template <typename T>
using const_iterator = _iterator<const T>;
template <typename T>
using reverse_iterator = _reverse_iterator<T>;
template <typename T>
using const_reverse_iterator = _reverse_iterator<const T>;

template<typename... Ts> requires (sizeof...(Ts) > 0 && (std::movable<Ts> && ...))
class toa
{
    static constexpr bool is_unique = is_set({ ^^Ts... });

    void allocate_and_initialize_views();
    void reallocate(size_t);
    constexpr size_t calculate_new_capacity_for(size_t) const noexcept;
    std::pair<size_t, size_t> reallocate_if_needed(size_t);

public:
    template <template <typename> typename Iterator, typename... Us> requires (is_unique && is_set({ ^^Us... }) && is_subset_of({ ^^Us... }, { ^^Ts... }))
    class _multi_iterator
    {
        using toa_ref_type = std::conditional_t<(is_template_of(^^Iterator, ^^iterator) || is_template_of(^^Iterator, ^^reverse_iterator)), toa<Ts...>&, const toa<Ts...>&>;

    public:
        static constexpr size_t size = sizeof...(Us);

        _multi_iterator(toa_ref_type t, size_t offset=0zu) noexcept : m_its{(t.template begin<Ts>() + offset)...}, m_toa(t) {}
        _multi_iterator(const _multi_iterator&) noexcept = default;
        _multi_iterator(_multi_iterator&&) noexcept = default;
        _multi_iterator& operator=(const _multi_iterator&) noexcept = default;
        _multi_iterator& operator=(_multi_iterator&&) noexcept = default;

        ~_multi_iterator() = default;

        template<size_t I> requires(I < sizeof...(Us)) auto& get() noexcept { return m_its.template get<I>(); }
        template<size_t I> requires(I < sizeof...(Us)) const auto& get() const noexcept { return m_its.template get<I>(); }
        template<typename T> auto& get() noexcept { return m_its.template get<T>(); }
        template<typename T> const auto& get() const noexcept { return m_its.template get<T>(); }

        void operator++() noexcept { template for (constexpr size_t I : std::views::iota(0zu, size)) ++m_its.template get<I>(); }
        void operator++(int) noexcept { template for (constexpr size_t I : std::views::iota(0zu, size)) m_its.template get<I>()++; }
        void operator--() noexcept { template for (constexpr size_t I : std::views::iota(0zu, size)) --m_its.template get<I>(); }
        void operator--(int) noexcept { template for (constexpr size_t I : std::views::iota(0zu, size)) m_its.template get<I>()--; }
        void operator+(std::integral auto x) noexcept { template for (constexpr size_t I : std::views::iota(0zu, size)) m_its.template get<I>() += x; }
        void operator-(std::integral auto x) noexcept { template for (constexpr size_t I : std::views::iota(0zu, size)) m_its.template get<I>() -= x; }

        bool done() const noexcept requires(is_template_of(^^Iterator, ^^iterator)) { return m_its.template get<0>() != m_toa.template end<Us...[0]>(); }
        bool done() const noexcept requires(is_template_of(^^Iterator, ^^const_iterator)) { return m_its.template get<0>() != m_toa.template cend<Us...[0]>(); }
        bool done() const noexcept requires(is_template_of(^^Iterator, ^^reverse_iterator)) { return m_its.template get<0>() != m_toa.template rend<Us...[0]>(); }
        bool done() const noexcept requires(is_template_of(^^Iterator, ^^const_reverse_iterator)) { return m_its.template get<0>() != m_toa.template crend<Us...[0]>(); }

        bool operator==(const _multi_iterator& other) const noexcept { return get<0>() == other.template get<0>(); }
        bool operator!=(const _multi_iterator& other) const noexcept { return get<0>() != other.template get<0>(); }
        bool operator>(const _multi_iterator& other) const noexcept { return get<0>() > other.template get<0>(); }
        bool operator>=(const _multi_iterator& other) const noexcept { return get<0>() >= other.template get<0>(); }
        bool operator<(const _multi_iterator& other) const noexcept { return get<0>() < other.template get<0>(); }
        bool operator<=(const _multi_iterator& other) const noexcept { return get<0>() <= other.template get<0>(); }
        auto operator<=>(const _multi_iterator& other) const noexcept { return get<0>() <=> other.template get<0>(); }

    private:
        tuple<Iterator<Us>...> m_its;
        toa_ref_type m_toa;
    };
    
    template <size_t... Is> requires (((Is < sizeof...(Ts)) && ...) && are_unique({ Is... }))
    using multi_iterator_i = _multi_iterator<iterator, Ts...[Is]...>;
    using multi_iterator = _multi_iterator<iterator, Ts...>;

    template <size_t... Is> requires (((Is < sizeof...(Ts)) && ...) && are_unique({ Is... }))
    using const_multi_iterator_i = _multi_iterator<const_iterator, Ts...[Is]...>;
    using const_multi_iterator = _multi_iterator<const_iterator, Ts...>;

    template <size_t... Is> requires (((Is < sizeof...(Ts)) && ...) && are_unique({ Is... }))
    using reverse_multi_iterator_i = _multi_iterator<reverse_iterator, Ts...[Is]...>;
    using reverse_multi_iterator = _multi_iterator<reverse_iterator, Ts...>;

    template <size_t... Is> requires (((Is < sizeof...(Ts)) && ...) && are_unique({ Is... }))
    using const_reverse_multi_iterator_i = _multi_iterator<const_reverse_iterator, Ts...[Is]...>;
    using const_reverse_multi_iterator = _multi_iterator<const_reverse_iterator, Ts...>;

    static constexpr size_t size = sizeof...(Ts);

    explicit toa(size_t=1zu);
    toa(size_t, const tuple<Ts...>&)
        noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...))
        requires (std::is_copy_constructible_v<Ts> && ...);
    toa(size_t, const Ts&...)
        noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...))
        requires (std::is_copy_constructible_v<Ts> && ...);
    toa(const toa&)
        noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...))
        requires (std::is_copy_constructible_v<Ts> && ...);
    toa(toa&&) noexcept;
    toa& operator=(const toa&)
        noexcept((std::is_nothrow_copy_assignable_v<Ts> && ...))
        requires (std::is_copy_assignable_v<Ts> && ...);
    toa& operator=(toa&&) noexcept;

    size_t capacity() const noexcept { return m_capacity; }
    size_t count() const noexcept { return m_count; }

    template <size_t I> constexpr auto get() noexcept { return m_views.template get<I>(); }
    template <size_t I> constexpr auto get() const noexcept { return m_views.template get<I>(); }
    template <size_t I> constexpr Ts...[I]& cell(size_t i) noexcept { assert(i < m_count); return m_views.template get<I>()[i]; }
    template <size_t I> constexpr const Ts...[I]& cell(size_t i) const noexcept { assert(i < m_count); return m_views.template get<I>()[i]; }
    template <typename T> constexpr auto get() noexcept requires(is_unique) { return m_views.template get<T*>(); }
    template <typename T> constexpr auto get() const noexcept requires(is_unique) { return m_views.template get<T*>(); }
    template <typename T> constexpr T& cell(size_t i) noexcept requires(is_unique) { assert(i < m_count); return m_views.template get<T*>()[i]; }
    template <typename T> constexpr const T& cell(size_t i) const noexcept requires(is_unique) { assert(i < m_count); return m_views.template get<T*>()[i]; }

    template <size_t I> iterator<Ts...[I]> begin() noexcept { return iterator<Ts...[I]>(m_views.template get<I>()); }
    template <size_t I> iterator<Ts...[I]> end() noexcept { return iterator<Ts...[I]>(m_views.template get<I>() + m_count); }
    template <typename T> iterator<T> begin() noexcept requires(is_unique) { return iterator<T>(m_views.template get<T*>()); }
    template <typename T> iterator<T> end() noexcept requires(is_unique) { return iterator<T>(m_views.template get<T*>() + m_count); }

    template <size_t I> const_iterator<Ts...[I]> cbegin() const noexcept { return const_iterator<Ts...[I]>(m_views.template get<I>()); }
    template <size_t I> const_iterator<Ts...[I]> cend() const noexcept { return const_iterator<Ts...[I]>(m_views.template get<I>() + m_count); }
    template <typename T> const_iterator<T> cbegin() const noexcept requires(is_unique) { return const_iterator<T>(m_views.template get<T*>()); }
    template <typename T> const_iterator<T> cend() const noexcept requires(is_unique) { return const_iterator<T>(m_views.template get<T*>() + m_count); }

    template <size_t I> reverse_iterator<Ts...[I]> rbegin() noexcept { return reverse_iterator<Ts...[I]>(m_views.template get<I>() +  + static_cast<int64_t>(m_count) - 1); }
    template <size_t I> reverse_iterator<Ts...[I]> rend() noexcept { return reverse_iterator<Ts...[I]>(m_views.template get<I>() - 1); }
    template <typename T> reverse_iterator<T> rbegin() noexcept requires(is_unique) { return reverse_iterator<T>(m_views.template get<T*>() + static_cast<int64_t>(m_count) - 1); }
    template <typename T> reverse_iterator<T> rend() noexcept requires(is_unique) { return reverse_iterator<T>(m_views.template get<T*>() - 1); }

    template <size_t I> const_reverse_iterator<Ts...[I]> crbegin() const noexcept { return const_reverse_iterator<Ts...[I]>(m_views.template get<I>() + static_cast<int64_t>(m_count) - 1); }
    template <size_t I> const_reverse_iterator<Ts...[I]> crend() const noexcept { return const_reverse_iterator<Ts...[I]>(m_views.template get<I>() - 1); }
    template <typename T> const_reverse_iterator<T> crbegin() const noexcept requires(is_unique) { return const_reverse_iterator<T>(m_views.template get<T*>() + static_cast<int64_t>(m_count) - 1); }
    template <typename T> const_reverse_iterator<T> crend() const noexcept requires(is_unique) { return const_reverse_iterator<T>(m_views.template get<T*>() - 1); }

    void swap(toa& other) noexcept;
    friend void swap(toa& a, toa& b) noexcept { a.swap(b); }

    size_t push_back() noexcept((std::is_nothrow_default_constructible_v<Ts> && ...)) requires (std::is_default_constructible_v<Ts> && ...);
    size_t push_back(const tuple<Ts...>&) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...);
    size_t push_back(tuple<Ts...>&&) noexcept;
    size_t push_back(const Ts&...) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...);
    size_t push_back(Ts&&...) noexcept;

    size_t append(size_t) noexcept((std::is_nothrow_default_constructible_v<Ts> && ...)) requires (std::is_default_constructible_v<Ts> && ...);
    size_t append(size_t, const tuple<Ts...>&) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...);
    size_t append(size_t, const Ts&...) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...);
    size_t append(const toa&) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...);
    size_t append(toa&&) noexcept;

    void resize(size_t) noexcept((std::is_nothrow_default_constructible_v<Ts> && ...)) requires (std::is_default_constructible_v<Ts> && ...);
    void resize(size_t, const tuple<Ts...>&) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...);
    void resize(size_t, const Ts&...) noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...)) requires (std::is_copy_constructible_v<Ts> && ...);

    void clear() noexcept;
    void reserve(size_t);
    void shrink_to_fit();

private:
    tuple<Ts*...> m_views;
    size_t m_capacity{};
    size_t m_count{};

    std::unique_ptr<cache_aligned_bytes[]> m_data{};
};
}

#include "detail/toa.inl"
