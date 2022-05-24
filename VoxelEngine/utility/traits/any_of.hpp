#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <typename T, typename... Ts>
    constexpr static bool is_any_of_v = (std::is_same_v<T, Ts> || ...);
}