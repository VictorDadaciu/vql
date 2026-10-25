#pragma once

#include <vql/tuple.h>
#include <vql/toa.h>

#include <vtl/suite.h>

#include <sstream>

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

template<typename T>
struct suite_helper<T*>
{
    inline static std::string to_string(const T* v)
    {
        return std::format("address: {}", reinterpret_cast<std::intptr_t>(v));
    }

    inline static bool equals(const T* a, const T* b)
    {
        return a == b;
    }
};

template<typename T>
struct suite_helper<vql::_iterator_base<T>>
{
    inline static std::string to_string(const vql::_iterator_base<T>& v)
    {
        return suite_helper<T*>::to_string(&v);
    }

    inline static bool equals(const vql::_iterator_base<T>& a, const vql::_iterator_base<T>& b)
    {
        return a == b;
    }
};
}