#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    template <typename... Variants>
    struct heterogeneous_hasher {
        using is_transparent = void;

        template <typename T> requires (meta::pack<Variants...>::template contains<T>)
        std::size_t operator()(const T& value) const {
            return default_hash<T>{}(value);
        }
    };


    template <typename... Variants>
    struct heterogeneous_comparator {
        using is_transparent = void;

        template <typename L, typename R> requires (
            meta::pack<Variants...>::template contains<L> &&
            meta::pack<Variants...>::template contains<R>
        ) std::size_t operator()(const L& lhs, const R& rhs) const {
            return lhs == rhs;
        }
    };
}