#pragma once

#include <meta>
#include <memory>

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

constexpr size_t next_multiple_of_cache_line_size(size_t);
constexpr size_t next_power_of_2(size_t);

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
