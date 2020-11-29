#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <typename Fn>
    [[nodiscard]] constexpr inline auto remove_return_type(Fn&& fn) noexcept {
        return [fn = std::forward<Fn>(fn)] <typename... Args> (Args&&... args) {
            fn(std::forward<Args>(args)...);
        };
    }
}