#pragma once

#include <cassert>
#include <meta>
#include <memory>
#include <cstdlib>

namespace vql
{
using meta_info = std::meta::info;
using meta_infos = std::vector<meta_info>;

struct null_type final {};

template<typename T>
struct debug_type;

template<typename... Ts>
struct typelist { static constexpr size_t count = sizeof...(Ts); };

static constexpr size_t cache_line_size = 64zu;

consteval bool are_unique(std::vector<size_t>);

constexpr size_t next_power_of_2(size_t);
constexpr bool is_power_of_2(size_t);

template<size_t Alignment = cache_line_size> requires (is_power_of_2(Alignment))
constexpr size_t next_multiple_of_power_of_2(size_t);

consteval bool has_physical_size(meta_info);
consteval bool is_template_of(meta_info, meta_info);
consteval bool is_in_types(meta_info, meta_infos);
consteval bool is_descendent_of(meta_info, meta_info);
consteval bool is_nested_in(meta_info, meta_info);
consteval bool is_set(meta_infos);
consteval bool is_subset_of(meta_infos, meta_infos);

consteval std::vector<size_t> subset_indices(meta_infos, meta_infos);

consteval meta_info base_of(meta_info, size_t=0zu);

struct alignas(cache_line_size) cache_aligned_bytes
{
    std::array<std::byte, cache_line_size> bytes{};
};

inline std::byte* allocate_cache_aligned_bytes(size_t byte_size)
{
    return reinterpret_cast<std::byte*>(new cache_aligned_bytes[byte_size / cache_line_size]);
}

inline std::unique_ptr<cache_aligned_bytes[]> allocate_smart_cache_aligned_bytes(size_t byte_size)
{
    return std::make_unique<cache_aligned_bytes[]>(byte_size / cache_line_size);
}

namespace detail
{
    struct aligned_deleter final
    {
        void operator()(std::byte* ptr)
        {
            std::free(ptr);
        }
    };
}

using aligned_bytes = std::unique_ptr<std::byte, detail::aligned_deleter>;
template<size_t Alignment = cache_line_size> requires (is_power_of_2(Alignment))
inline aligned_bytes allocate_aligned_bytes(size_t byte_size)
{
    assert(byte_size > 0zu);
    return aligned_bytes(static_cast<std::byte*>(std::aligned_alloc(Alignment, next_multiple_of_power_of_2<Alignment>(byte_size))));
}
}

#define VQL_STRONG_TYPEDEF(underlying, name)                               \
class name final                                                           \
{                                                                          \
private:                                                                   \
    underlying value;                                                      \
public:                                                                    \
    inline name() = default;                                               \
    inline name(const name &x) = default;                                  \
    inline name(name &&x) = default;                                       \
    inline name &operator=(const name &rhs) = default;                     \
    inline name &operator=(name &&rhs) = default;                          \
    inline constexpr name(underlying x) noexcept : value{x} {}             \
    inline constexpr name &operator=(underlying rhs) noexcept              \
    {                                                                      \
        value = rhs;                                                       \
        return *this;                                                      \
    }                                                                      \
    inline constexpr operator const underlying &() const noexcept          \
    {                                                                      \
        return value;                                                      \
    }                                                                      \
    inline constexpr operator underlying &() noexcept { return value; }    \
    inline constexpr decltype(auto) operator==(const name &rhs) noexcept   \
    {                                                                      \
        return value == rhs.value;                                         \
    }                                                                      \
    inline constexpr decltype(auto) operator!=(const name &rhs) noexcept   \
    {                                                                      \
        return value != rhs.value;                                         \
    }                                                                      \
    inline constexpr decltype(auto) operator<(const name &rhs) noexcept    \
    {                                                                      \
        return value < rhs.value;                                          \
    }                                                                      \
    inline constexpr decltype(auto) operator>(const name &rhs) noexcept    \
    {                                                                      \
        return value > rhs.value;                                          \
    }                                                                      \
    inline constexpr decltype(auto) operator<=(const name &rhs) noexcept   \
    {                                                                      \
        return value <= rhs.value;                                         \
    }                                                                      \
    inline constexpr decltype(auto) operator>=(const name &rhs) noexcept   \
    {                                                                      \
        return value >= rhs.value;                                         \
    }                                                                      \
}

#include "detail/utils.inl"
