#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::gfx::combine_functions {
    constexpr inline auto add       = [](const auto& old_value, const auto& new_value) { return old_value + new_value; };
    constexpr inline auto multiply  = [](const auto& old_value, const auto& new_value) { return old_value * new_value; };
    constexpr inline auto overwrite = [](const auto& old_value, const auto& new_value) { return new_value; };
}