#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <template <typename...> typename Tmpl, typename... Xs> struct bind {
        template <typename... Ys> using front = Tmpl<Xs..., Ys...>;
        template <typename... Ys> using back  = Tmpl<Ys..., Xs...>;
    };


    template <typename T, template <typename...> typename... Tmpls> struct apply_all {
        using type = T;
    };

    template <typename T, template <typename...> typename Tmpl0, template <typename...> typename... Tmpls> struct apply_all<T, Tmpl0, Tmpls...> {
        using type = apply_all<Tmpl0<T>, Tmpls...>;
    };

    template <typename T, template <typename...> typename... Tmpls> using apply_all_t = typename apply_all<T, Tmpls...>::type;
}