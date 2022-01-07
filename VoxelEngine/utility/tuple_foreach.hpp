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
    // If the given predicate returns bool, returning false breaks the foreach.
    template <typename Tpl, typename Pred, std::size_t I = 0>
    constexpr void tuple_foreach(Tpl& tuple, Pred&& pred) {
        using base_t = std::remove_cvref_t<Tpl>;
    
        if constexpr (I < std::tuple_size_v<base_t>) {
            if constexpr (std::is_same_v<std::invoke_result_t<Pred, std::tuple_element_t<I, Tpl>&>, bool>) {
                if (!pred(std::get<I>(tuple))) return;
            } else {
                pred(std::get<I>(tuple));
            }

            tuple_foreach<Tpl, Pred, I + 1>(tuple, fwd(pred));
        }
    }
    
    
    template <typename Tpl, typename Pred, std::size_t I = 0>
    constexpr void tuple_foreach(Tpl&& tuple, Pred&& pred) {
        using base_t = std::remove_cvref_t<Tpl>;
        
        if constexpr (I < std::tuple_size_v<base_t>) {
            if constexpr (std::is_same_v<std::invoke_result_t<Pred, std::tuple_element_t<I, Tpl>>, bool>) {
                if (!pred(std::get<I>(fwd(tuple)))) return;
            } else {
                pred(std::get<I>(fwd(tuple)));
            }

            tuple_foreach<Tpl, Pred, I + 1>(fwd(tuple), fwd(pred));
        }
    }
}