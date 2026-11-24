#pragma once

#include "table.h"

namespace vql::debug
{
template<typename T> requires (is_template_of(^^T, ^^table))
inline void print();
}

#include "detail/debug.inl"
