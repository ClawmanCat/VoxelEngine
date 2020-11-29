#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/traits/glm_traits.hpp>

#include <functional>


#define ve_transform(name, change) [&](auto&& name) { return change; }

#define ve_vec_transform(name, change)                      \
[&] <typename T, std::size_t N> (ve::vec<N, T> vec) {       \
    for (std::size_t i = 0; i < N; ++i) {                   \
        auto name = std::move(vec[i]);                      \
        vec[i] = change;                                    \
    }                                                       \
                                                            \
    return vec;                                             \
}


namespace ve {
    const inline auto no_op    = [](auto...) { };
    const inline auto identity = [](auto&& v) -> decltype(auto) { return std::forward<decltype(v)>(v); };
    
    
    const inline auto equal     = [](const auto& a, const auto& b) { return a == b; };
    const inline auto not_equal = [](const auto& a, const auto& b) { return a != b; };
    
    
    // Returns a method for checking if two objects are equal on the given field.
    [[nodiscard]] constexpr inline auto equal_on_field(auto field) {
        return [field](const auto& a, const auto& b) { return a.*field == b.*field; };
    }
    
    
    // Returns a method for checking if two objects return equal values for the given method.
    template <typename... Args>
    [[nodiscard]] constexpr inline auto equal_on_method(auto method, Args&&... args) {
        return [method, ...args = std::forward<Args>(args)](const auto& a, const auto& b) {
            return a.*method(args...) == b.*method(args...);
        };
    }
    
    
    // Return a method that returns the logical negation of whatever the original method returns.
    [[nodiscard]] constexpr inline auto logical_not(auto fn) {
        return [fn]<typename... Args>(Args&&... args) { return !fn(std::forward<Args>(args)...); };
    }
}