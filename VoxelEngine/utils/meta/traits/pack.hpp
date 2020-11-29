#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/traits/null_type.hpp>
#include <VoxelEngine/utils/meta/traits/is_of_template.hpp>


namespace ve::meta {
    namespace detail {
        // Num types = 0
        template <template <typename...> typename Pack, typename...> struct splitter {
            using head = null_type;
            using tail = Pack<>;
        };
        
        // Num types = 1
        template <template <typename...> typename Pack, typename T0, typename... Ts> struct splitter<Pack, T0, Ts...> {
            using head = T0;
            using tail = Pack<>;
        };
        
        // Num types > 1
        template <template <typename...> typename Pack, typename T0, typename T1, typename... Ts> struct splitter<Pack, T0, T1, Ts...> {
            using head = T0;
            using tail = Pack<T1, Ts...>;
        };
    }
    
    
    template <typename... Ts> class pack {
    public:
        constexpr static std::size_t size = sizeof...(Ts);
        
        using head = typename detail::splitter<pack, Ts...>::head;
        using tail = typename detail::splitter<pack, Ts...>::tail;
        
        
        template <template <typename...> typename F> using expand_inside  = F<Ts...>;
        template <template <typename...> typename F> using expand_outside = pack<F<Ts>...>;
    
    
        template <typename... Xs> using append  = pack<Ts..., Xs...>;
        template <typename... Xs> using prepend = pack<Xs..., Ts...>;
        
        template <typename Pack> using append_pack  = typename Pack::template expand_inside<append>;
        template <typename Pack> using prepend_pack = typename Pack::template expand_inside<prepend>;
        
        
        template <typename T> constexpr static bool contains(void) {
            if constexpr (size > 0) {
                return std::is_same_v<head, T> || tail::template contains<T>();
            } else return false;
        }
    };
    
    
    template <typename T> concept type_pack = is_of_template_v<pack, T>;
}