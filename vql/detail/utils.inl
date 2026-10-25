#pragma once

namespace vql
{
consteval bool are_unique(const std::vector<size_t>& nums)
{
    for (size_t i = 0zu; i < nums.size() - 1; ++i)
        for (size_t j = i + 1; i < nums.size(); ++j)
            if (nums[i] == nums[j])
                return false;
    return true;
}

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

consteval bool has_physical_size(meta_info r)
{
    if (is_arithmetic_type(r) || is_enum_type(r) || is_scoped_enum_type(r) || nonstatic_data_members_of(r, std::meta::access_context::unchecked()).size() > 0)
        return true;
    do {
        r = base_of(r);
        if (r == ^^null_type)
            return false;
        else if (nonstatic_data_members_of(r, std::meta::access_context::unchecked()).size() > 0)
            return true;
    } while(true);
    return false;
}

consteval bool is_in_types(meta_info type, meta_infos list)
{
    for (meta_info r : list)
        if (type == r)
            return true;
    return false;
}

consteval bool is_template_of(meta_info type, meta_info tmpl)
{
    return has_template_arguments(type) && template_of(type) == tmpl;
}

consteval bool is_descendent_of(meta_info descendent, meta_info ancestor)
{
    do
    {
        if (descendent == ^^null_type)
            return false;
        if (descendent == ancestor)
            return true;
        descendent = base_of(descendent);
    } while(true);
}

consteval bool is_nested_in(meta_info nested, meta_info nest)
{
    return has_parent(nested) && is_template_of(parent_of(nested), nest);
}

consteval bool is_set(meta_infos types)
{
    for (size_t i = 0; i < types.size() - 1; ++i)
        for (size_t j = i + 1; j < types.size(); ++j)
            if (types[i] == types[j])
                return false;
    return true;
}

namespace detail
{
    consteval bool subset_helper(meta_info inner, meta_infos outer, std::vector<size_t> to_exclude)
    {
        for (size_t i = 0; i < outer.size(); ++i)
        {
            bool skip{};
            for (size_t e : to_exclude)
            {
                if (e == i)
                {
                    skip = true;
                    break;
                }
            }
            if (skip) continue;
            else if (outer[i] == inner) return true;
        }
        return false;
    }
}

consteval std::vector<size_t> subset_indices(meta_infos inner, meta_infos outer)
{
    std::vector<size_t> indices;
    for (size_t i = 0zu; i < inner.size(); ++i)
    {
        if (!detail::subset_helper(inner[i], outer, indices))
            return std::vector<size_t>();
        indices.push_back(i);
    }
    return indices;
}

consteval bool is_subset_of(meta_infos inner, meta_infos outer)
{
    return subset_indices(inner, outer).size() > 0zu;
}

consteval meta_info base_of(meta_info r, size_t index)
{
    if (is_fundamental_type(r) || is_enum_type(r) || is_scoped_enum_type(r))
        return ^^null_type;
    auto bases = bases_of(r, std::meta::access_context::unchecked());
    return (index < bases.size() ? type_of(bases[index]) : ^^null_type);
}
}