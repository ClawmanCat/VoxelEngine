#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve::meta {
    template <template <typename...> typename Tmpl, typename... BArgs>
    struct bind_types {
        template <typename... RArgs> using front = Tmpl<BArgs..., RArgs...>;
        template <typename... RArgs> using back  = Tmpl<RArgs..., BArgs...>;

        template <std::size_t Index> struct at {
            template <typename... RArgs> using type = typename pack<RArgs...>
                ::template first<Index>
                ::template append<RArgs...>
                ::template append_pack<
                    typename pack<RArgs...>::template pop_front<Index>
                >;
        };
    };
}