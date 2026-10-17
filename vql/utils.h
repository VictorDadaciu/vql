#pragma once

#include <memory>

namespace vql
{
static constexpr size_t cache_line_size = 64zu;
constexpr size_t next_multiple_of_cache_line_size(size_t x)
{
    return (x + cache_line_size - 1) & -cache_line_size;
}

constexpr size_t next_power_of_2(size_t x)
{
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return ++x;
}

struct alignas(cache_line_size) cache_aligned_bytes
{
    std::array<std::byte, cache_line_size> bytes{};
};

std::byte* allocate_cache_aligned_bytes(size_t byte_size)
{
    return reinterpret_cast<std::byte*>(new cache_aligned_bytes[byte_size / cache_line_size]);
}

std::unique_ptr<cache_aligned_bytes[]> allocate_smart_cache_aligned_bytes(size_t byte_size)
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
