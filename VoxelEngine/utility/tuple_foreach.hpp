#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/is_template.hpp>
#include <VoxelEngine/utility/traits/is_std_array.hpp>


namespace ve {
    template <typename T> constexpr inline bool is_tuple_indexable_v = std::disjunction_v<
        meta::is_template<std::tuple, T>,
        meta::is_template<std::pair,  T>,
        meta::is_std_array<T>
    >;
    
    
    // Calls the given predicate for every element of a structure that supports indexing through std::get<index> (e.g. tuple, pair).
    template <typename Tpl, typename Pred, std::size_t I = 0>
    constexpr void tuple_foreach(Tpl& tuple, Pred&& pred) {
        using base_t = std::remove_cvref_t<Tpl>;
    
        if constexpr (I < std::tuple_size_v<base_t>) {
            pred(std::get<I>(tuple));
            tuple_foreach<Tpl, Pred, I + 1>(tuple, fwd(pred));
        }
    }
    
    
    template <typename Tpl, typename Pred, std::size_t I = 0>
    constexpr void tuple_foreach(Tpl&& tuple, Pred&& pred) {
        using base_t = std::remove_cvref_t<Tpl>;
        
        if constexpr (I < std::tuple_size_v<base_t>) {
            pred(std::get<I>(fwd(tuple)));
            tuple_foreach<Tpl, Pred, I + 1>(fwd(tuple), fwd(pred));
        }
    }
}