#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <template <typename...> typename Trait>
    struct negate_trait {
        template <typename... Ts> struct type {
            constexpr static bool value = !Trait<Ts...>::value;
        };
    };
}