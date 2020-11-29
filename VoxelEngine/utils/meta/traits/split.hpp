#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <typename H, typename... T> constexpr decltype(auto) head(H&& h, T&&... t) noexcept {
        return std::forward<H>(h);
    }
}