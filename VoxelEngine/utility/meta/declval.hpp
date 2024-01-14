#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    /**
     * Equivalent to std::declval, but without the evaluated-context detectors some compilers insert when using that method.
     * This allows this version of declval to be used in evaluated contexts, but actually calling it at runtime is still UB.
     * @tparam T The type this function should return (as a rvalue reference).
     * @return This function has a return type of T&& but invoking it is undefined behaviour.
     */
    template <typename T> inline std::add_rvalue_reference_t<T> declval(void) noexcept {
        // TODO: Insert assert here!
        VE_UNREACHABLE;
    }
}