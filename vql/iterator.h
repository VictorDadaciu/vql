#pragma once

#include <cstddef>
#include <iterator>

namespace vql
{
namespace detail
{
    template<typename T, size_t Stride> requires (Stride >= sizeof(T))
    class iterator_base
    {
    public:
        static constexpr bool contiguous = Stride == sizeof(T);
        using iterator_category = std::conditional_t<contiguous, std::contiguous_iterator_tag, std::random_access_iterator_tag>;
        using difference_type = std::ptrdiff_t;

        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

        iterator_base(pointer ptr=nullptr) noexcept : m_it(ptr) {};
        iterator_base(const iterator_base& other) noexcept : m_it(other.m_it) {}
        iterator_base(iterator_base&& other) noexcept : m_it(other.m_it) { other.m_it = nullptr; }
        iterator_base& operator=(const iterator_base& other) noexcept { m_it = other.m_it; return *this; }
        iterator_base& operator=(iterator_base&& other) noexcept { m_it = other.m_it; other.m_it = nullptr; return *this; }

        ~iterator_base() = default;

        reference operator*() const { return *m_it; }
        pointer operator&() const { return m_it; }
        pointer operator->() const noexcept { return m_it; }
        explicit operator reference() { return *m_it; }
        explicit operator pointer() noexcept { return m_it; }
        explicit operator const reference() const { return *m_it; }
        explicit operator const pointer() const noexcept { return m_it; }
        explicit operator bool() const noexcept { return m_it; }

        friend bool operator== (const iterator_base& a, const iterator_base& b) noexcept { return a.m_it ==  b.m_it; }
        friend bool operator!= (const iterator_base& a, const iterator_base& b) noexcept { return a.m_it !=  b.m_it; }

        pointer m_it;
    };

    template<typename T, size_t Stride> requires (Stride >= sizeof(T))
    class forward_iterator_base : public iterator_base<T, Stride>
    {
    public:
        using base = iterator_base<T, Stride>;
        using base::contiguous;
        using iterator_category = base::iterator_category;
        using difference_type = base::difference_type;

        using value_type = base::value_type;
        using pointer = base::pointer;
        using reference = base::reference;

    protected:
        pointer _add(difference_type i) const
        {
            if constexpr (contiguous) return m_it + i;
            else return reinterpret_cast<pointer>(reinterpret_cast<std::byte*>(m_it) + i * Stride);
        }

    public:
        friend bool operator<  (const forward_iterator_base& a, const forward_iterator_base& b) noexcept { return a.m_it <   b.m_it; }
        friend bool operator<= (const forward_iterator_base& a, const forward_iterator_base& b) noexcept { return a.m_it <=  b.m_it; }
        friend bool operator>  (const forward_iterator_base& a, const forward_iterator_base& b) noexcept { return a.m_it >   b.m_it; }
        friend bool operator>= (const forward_iterator_base& a, const forward_iterator_base& b) noexcept { return a.m_it >=  b.m_it; }
        friend auto operator<=>(const forward_iterator_base& a, const forward_iterator_base& b) noexcept { return a.m_it <=> b.m_it; }

        reference operator[](difference_type i) { return *_add(i); }
        const reference operator[](difference_type i) const { return *_add(i); }

        forward_iterator_base& operator++() noexcept { m_it = _add(1);  return *this; }
        forward_iterator_base& operator--() noexcept { m_it = _add(-1); return *this; }
        forward_iterator_base  operator++(int) noexcept { return forward_iterator_base(_add(1)); }
        forward_iterator_base  operator--(int) noexcept { return forward_iterator_base(_add(-1)); }
        forward_iterator_base& operator+=(difference_type i) noexcept { m_it = _add(i); return *this; }
        forward_iterator_base& operator-=(difference_type i) noexcept { m_it = _add(-i); return *this; }
        friend forward_iterator_base operator+ (const forward_iterator_base& it, difference_type i) noexcept { return forward_iterator_base(_add(i)); }
        friend forward_iterator_base operator- (const forward_iterator_base& it, difference_type i) noexcept { return forward_iterator_base(_add(-i)); }

    protected:
        using base::m_it;
    };

    template<typename T, size_t Stride> requires (Stride >= sizeof(T))
    class reverse_iterator_base : public iterator_base<T, Stride>
    {
    public:
        using base = iterator_base<T, Stride>;
        using base::contiguous;
        using iterator_category = base::iterator_category;
        using difference_type = base::difference_type;

        using value_type = base::value_type;
        using pointer = base::pointer;
        using reference = base::reference;

    protected:
        pointer _add(difference_type i) const
        {
            if constexpr (contiguous) return m_it - i;
            else return reinterpret_cast<pointer>(reinterpret_cast<std::byte*>(m_it) - i * Stride);
        }

    public:
        friend bool operator<  (const reverse_iterator_base& a, const reverse_iterator_base& b) noexcept { return b.m_it <   a.m_it; }
        friend bool operator<= (const reverse_iterator_base& a, const reverse_iterator_base& b) noexcept { return b.m_it <=  a.m_it; }
        friend bool operator>  (const reverse_iterator_base& a, const reverse_iterator_base& b) noexcept { return b.m_it >   a.m_it; }
        friend bool operator>= (const reverse_iterator_base& a, const reverse_iterator_base& b) noexcept { return b.m_it >=  a.m_it; }
        friend auto operator<=>(const reverse_iterator_base& a, const reverse_iterator_base& b) noexcept { return b.m_it <=> a.m_it; }

        reference operator[](difference_type i) { return *_add(i); }
        const reference operator[](difference_type i) const { return *_add(i); }

        reverse_iterator_base& operator++() noexcept { m_it = _add(1);  return *this; }
        reverse_iterator_base& operator--() noexcept { m_it = _add(-1); return *this; }
        reverse_iterator_base  operator++(int) noexcept { return reverse_iterator_base(_add(1)); }
        reverse_iterator_base  operator--(int) noexcept { return reverse_iterator_base(_add(-1)); }
        reverse_iterator_base& operator+=(difference_type i) noexcept { m_it = _add(i); return *this; }
        reverse_iterator_base& operator-=(difference_type i) noexcept { m_it = _add(-i); return *this; }
        friend reverse_iterator_base operator+ (const reverse_iterator_base& it, difference_type i) noexcept { return reverse_iterator_base(_add(i)); }
        friend reverse_iterator_base operator- (const reverse_iterator_base& it, difference_type i) noexcept { return reverse_iterator_base(_add(-i)); }

    protected:
        using base::m_it;
    };
}

template<typename T, size_t Stride = sizeof(T)> requires (Stride >= sizeof(T))
using iterator = detail::forward_iterator_base<T, Stride>;
template<typename T, size_t Stride = sizeof(T)> requires (Stride >= sizeof(T))
using const_iterator = detail::forward_iterator_base<const T, Stride>;
template<typename T, size_t Stride = sizeof(T)> requires (Stride >= sizeof(T))
using reverse_iterator = detail::reverse_iterator_base<T, Stride>;
template<typename T, size_t Stride = sizeof(T)> requires (Stride >= sizeof(T))
using const_reverse_iterator = detail::reverse_iterator_base<const T, Stride>;
}