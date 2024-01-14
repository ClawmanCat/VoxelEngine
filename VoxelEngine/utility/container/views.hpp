#pragma once

#include <VoxelEngine/core/core.hpp>

#include <range/v3/view.hpp>


namespace ve::views {
    template <typename T> constexpr inline auto cast
        = views::transform([] (auto&& e) { return static_cast<T>(fwd(e)); });
}