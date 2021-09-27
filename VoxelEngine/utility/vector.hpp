#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>

#include <queue>
#include <stack>


namespace ve {
    template <typename T, typename It>
    inline void swap_erase(std::vector<T>& vec, It iterator) {
        // Swap should be valid, even if the vector only contains one element.
        // (http://eel.is/c++draft/swappable.requirements#2.2)
        std::swap(vec.back(), *iterator);
        vec.pop_back();
    }
    
    
    template <typename T>
    inline void swap_erase_at(std::vector<T>& vec, std::size_t where) {
        swap_erase(vec, vec.begin() + where);
    }
    
    
    template <typename T>
    inline void swap_erase_element(std::vector<T>& vec, const T& elem) {
        std::ptrdiff_t distance = (&elem - &vec[0]);
        
        VE_DEBUG_ASSERT(
            distance >= 0 && distance < (std::ptrdiff_t) vec.size(),
            "Attempt to swap erase element not in vector."
        );
    
        swap_erase_at(vec, distance);
    }
    
    
    template <typename T> inline auto take(std::queue<T>& ctr) {
        auto elem = std::move(ctr.front());
        ctr.pop();
        
        return elem;
    }
    
    
    template <typename T> inline auto take(std::stack<T>& ctr) {
        auto elem = std::move(ctr.top());
        ctr.pop();
        
        return elem;
    }
    
    
    template <typename T> inline auto take(std::vector<T>& ctr) {
        auto elem = std::move(ctr.back());
        ctr.pop_back();
        
        return elem;
    }
    
    
    template <typename T> inline auto take_front(std::deque<T>& ctr) {
        auto elem = std::move(ctr.front());
        ctr.pop_front();
        
        return elem;
    }
    
    
    template <typename T> inline auto take_back(std::deque<T>& ctr) {
        auto elem = std::move(ctr.back());
        ctr.pop_back();
        
        return elem;
    }


    template <typename T> inline auto take_front(std::span<T>& ctr) {
        auto elem = ctr.front();
        ctr = ctr.last(ctr.size() - 1);

        return elem;
    }


    template <typename T> inline auto take_back(std::span<T>& ctr) {
        auto elem = ctr.back();
        ctr = ctr.first(ctr.size() - 1);

        return elem;
    }


    template <typename T> inline auto take_front_n(std::span<T>& ctr, std::size_t n) {
        auto result = ctr.first(n);
        ctr = ctr.last(ctr.size() - n);

        return result;
    }


    template <typename T> inline auto take_back_n(std::span<T>& ctr, std::size_t n) {
        auto result = ctr.last(n);
        ctr = ctr.first(ctr.size() - n);

        return result;
    }


    // Creates a filled array by invoking pred for each element.
    template <std::size_t N, typename Pred> requires std::is_invocable_v<Pred, std::size_t>
    constexpr auto create_filled_array(Pred pred) {
        return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
            return std::array { pred(Is)... };
        } (std::make_index_sequence<N>());
    }


    // Creates an array from the given elements, deducing the size automatically.
    template <typename T0, typename... Ts> requires (std::is_convertible_v<Ts, T0> && ...)
    constexpr auto make_array(T0&& first, Ts&&... rest) {
        return std::array<std::remove_reference_t<T0>, (sizeof...(Ts) + 1)> { fwd(first), fwd(rest)... };
    }


    template <typename A, typename B> struct combine_result {
        std::vector<A*> unmatched_a;
        std::vector<B*> unmatched_b;
        std::vector<std::pair<A*, B*>> matched;
    };

    // Given two containers, A and B, makes pairs of elements from both containers where pred(a, b) returns true,
    // and returns these pairs, together with all unmatched elements in a combine_result.
    template <typename A, typename B, typename Pred>
    inline auto combine_into_pairs(A& a, B& b, Pred pred) {
        using AV = std::conditional_t<std::is_const_v<A>, std::add_const_t<typename A::value_type>, typename A::value_type>;
        using BV = std::conditional_t<std::is_const_v<B>, std::add_const_t<typename B::value_type>, typename B::value_type>;


        combine_result<AV, BV> result;

        // Assume all elements of B to be unmatched, then remove them as they get matched.
        // Note: removed elements are just swapped to the end of the vector, then erased all at once once at the end.
        result.unmatched_b = b | views::addressof | ranges::to<std::vector>;
        auto real_end = result.unmatched_b.end();

        for (auto& ae : a) {
            for (auto& be : b) {
                if (pred(ae, be)) {
                    real_end = std::remove(result.unmatched_b.begin(), real_end, &be);
                    result.matched.emplace_back(&ae, &be);

                    goto matched; // Skip adding ae to unmatched list, since we just matched it.
                }
            }

            result.unmatched_a.push_back(&ae);
            matched:;
        }

        result.unmatched_b.erase(real_end, result.unmatched_b.end());
        return result;
    }
}