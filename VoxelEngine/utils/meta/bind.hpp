#pragma once

#include <VoxelEngine/core/core.hpp>

#include <utility>
#include <functional>


namespace ve::meta {
    // Not yet implemented by every STL at the time of writing.
    template <typename Fn, typename... Args>
    [[nodiscard]] constexpr inline auto bind_front(Fn&& fn, Args&&... args) noexcept {
        return
            [fn = std::forward<Fn>(fn), ...args = std::forward<Args>(args)]
            <typename... RemainingArgs> (RemainingArgs&&... remaining_args) mutable -> decltype(auto) {
                return std::invoke(fn, std::forward<Args>(args)..., std::forward<RemainingArgs>(remaining_args)...);
            };
    }
    
    
    template <typename Fn, typename... Args>
    [[nodiscard]] constexpr inline auto bind_back(Fn&& fn, Args&&... args) noexcept {
        return
            [fn = std::forward<Fn>(fn), ...args = std::forward<Args>(args)]
            <typename... RemainingArgs> (RemainingArgs&&... remaining_args) mutable -> decltype(auto) {
                return std::invoke(fn, std::forward<RemainingArgs>(remaining_args)..., std::forward<Args>(args)...);
            };
    }
}