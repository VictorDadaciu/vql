#pragma once

namespace vql
{
template <typename... Ts>
consteval size_t tuple<Ts...>::index_of_type(meta_info r, size_t which) noexcept
{
    for (size_t i = 0; i < members.size(); ++i)
        if (type_of(members[i]) == r && which-- == 0) return i;
    return members.size();
}
}