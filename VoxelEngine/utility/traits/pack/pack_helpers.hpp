#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>


namespace ve::meta::detail {
    template <template <typename...> typename Pack, typename... Ts>
    struct splitter {
        using head = null_type;
        using tail = Pack<>;
    };
    
    template <template <typename...> typename Pack, typename T0, typename... Ts>
    struct splitter<Pack, T0, Ts...> {
        using head = T0;
        using tail = Pack<Ts...>;
    };
    
    
    #define ve_beptr(...) \
    std::add_pointer_t<__VA_ARGS__>
    
    #define ve_deptr(fn, ...) \
    std::remove_pointer_t<decltype(fn __VA_OPT__(<) __VA_ARGS__ __VA_OPT__(>) ())>
}