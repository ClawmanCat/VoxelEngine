#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/traits/value.hpp>
#include <VoxelEngine/utils/meta/if_constexpr.hpp>
#include <VoxelEngine/utils/meta/bind.hpp>

#include <tuple>
#include <cstddef>
#include <type_traits>


namespace ve {
    // Iterate over each element of a tuple with the given predicate.
    // The predicate may return void, or a bool controlling whether or not looping should continue.
    // (i.e. return false = break)
    // Additionally, the predicate may take an argument of type value<N>, indicating the index of the element.
    // Returns whether or not the loop completed without breaking.
    template <typename Tpl, typename Pred, std::size_t N = 0>
    constexpr inline bool tuple_foreach(Tpl& tuple, Pred& pred) {
        constexpr bool requires_index = std::is_invocable_v<Pred, std::tuple_element_t<N, Tpl>, meta::value<N>>;
        
        auto pred_bound = meta::return_if<requires_index>(
            meta::bind_front(pred, std::get<N>(tuple), meta::value<N>{}),
            meta::bind_front(pred, std::get<N>(tuple))
        );
        
        constexpr bool returns_bool = std::is_same_v<bool, std::invoke_result_t<decltype(pred_bound)>>;
        
        
        if constexpr (N < std::tuple_size_v<Tpl>) {
            // If pred returns bool, allow breaking from the loop by returning false.
            if constexpr (returns_bool) { if (!pred_bound()) return false; }
            else pred_bound();
            
            return tuple_foreach<Tpl, Pred, N + 1>(tuple, pred);
        } else return true;
    }
}