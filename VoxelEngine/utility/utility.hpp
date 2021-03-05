#pragma once

#include <VoxelEngine/core/core.hpp>

#include <vector>
#include <type_traits>
#include <algorithm>
#include <numeric>


namespace ve {
    template <typename T, typename... Ts>
    [[nodiscard]] constexpr inline bool one_of(const T& val, const Ts&... vals) {
        return ((val == vals) || ...);
    }
    
    
    // Erases from a vector without preserving order.
    template <typename Ctr, typename It>
    inline void swap_erase(Ctr& ctr, It it) {
        std::swap(*it, ctr.back());
        ctr.pop_back();
    }
    
    
    // Equivalent to post increment (x++), but for values other than one.
    // i.e. adds to x and returns the value x had before the operation.
    template <typename T> inline T post_add(T& value, T delta) {
        T old = value;
        value += delta;
        return old;
    }
    
    
    template <typename Pred, typename Ctr> inline auto collect_if(Ctr&& ctr, const Pred& pred) {
        std::vector<typename std::remove_cvref_t<Ctr>::value_type> result;
        
        for (auto& elem : ctr) {
            if (pred(elem)) {
                if constexpr (std::is_rvalue_reference_v<Ctr>) result.push_back(std::move(elem));
                else result.push_back(elem);
            }
        }
        
        return result;
    }
    
    
    template <typename Ctr> inline bool contains(const Ctr& ctr, const typename Ctr::value_type& value) {
        return std::find(ctr.begin(), ctr.end(), value) != ctr.end();
    }
    
    
    template <typename Pred, typename Ctr> inline bool contains_if(const Ctr& ctr, const Pred& pred) {
        return std::find_if(ctr.begin(), ctr.end(), pred) != ctr.end();
    }
    
    
    template <typename Ctr> inline std::string join_strings(const Ctr& ctr, const char* separator = "") {
        std::size_t length = 0;
        for (const auto& elem : ctr) length += elem.length();
        
        std::string result;
        result.reserve(length + (strlen(separator) * ctr.size()));
        
        for (const auto& elem : ctr) {
            result.append(elem);
            result.append(separator);
        }
        
        result.resize(result.length() - strlen(separator));
        
        return result;
    }
    
    
    // Given that there are count elements matching pred in the container,
    // removes them by swapping them to the end and shrinking the container once.
    template <typename Elem, typename Pred>
    inline void erase_n_unordered(std::vector<Elem>& ctr, u32 count, Pred pred) {
        u32 removed = 0;
        
        for (auto it = ctr.rbegin(); it != ctr.rend(); ++it) {
            if (pred(*it)) {
                std::swap(
                    *it,
                    *(ctr.begin() + removed)
                );
                
                if (++removed == count) break;
            }
        }
        
        ctr.erase(ctr.end() - removed, ctr.end());
    }
    
    
    template <typename Ctr, typename Elem, typename Default>
    constexpr inline auto find_or(Ctr& ctr, const Elem& elem, Default&& default_value) {
        auto it = std::find(
            ctr.begin(),
            ctr.end(),
            elem
        );
        
        return (it == ctr.end())
            ? std::forward<Default>(default_value)
            : *it;
    }
}