#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/traits/is_of_template.hpp>
#include <VoxelEngine/utils/meta/if_constexpr.hpp>

#include <vector>
#include <type_traits>
#include <array>


namespace ve {
    namespace detail {
        template <typename T, std::size_t N, typename Fn, std::size_t... IS>
        constexpr inline std::array<T, N> filled_array(Fn&& fn, std::index_sequence<IS...>) {
            if constexpr (std::is_invocable_v<Fn, std::size_t>) return { fn(IS)... };
            else return { ((void) IS, fn())... };
        }
    }
    
    
    // Constructs a reverse iterator from a normal iterator and vice versa.
    template <typename It>
    constexpr inline auto invert_iterator(It it) {
        if constexpr (requires { it.base(); }) {
            return it.base();
        } else {
            return std::make_reverse_iterator(it);
        }
    }
    
    
    template <typename T, std::size_t N, typename Fn>
    constexpr inline std::array<T, N> create_filled_array(Fn&& fn) {
        return detail::filled_array<T, N>(
            std::forward<Fn>(fn),
            std::make_index_sequence<N>()
        );
    }
    
    
    template <typename Vec, typename It>
    inline void swap_erase(Vec& vector, It where) {
        if (vector.empty()) return;
        if (where + 1 != vector.end()) std::swap(*where, vector.back());
        vector.pop_back();
    }
    
    
    template <typename Vec, typename It>
    inline auto swap_erase_get(Vec& vector, It where) {
        VE_ASSERT(!vector.empty());
        
        auto value = (where + 1 != vector.end())
            ? std::exchange(*where, std::move(vector.back()))
            : std::move(vector.back());
        
        vector.pop_back();
        return value;
    }
    
    
    // TODO: Use universal reference & forward.
    inline std::string join_strings(const std::vector<std::string>& vec, std::string_view separator = "\n") {
        std::size_t size = 0;
        for (const auto& str : vec) size += str.size();
        size += (separator.size() * vec.size());
        
        std::string result;
        result.reserve(size);
        
        for (const auto& str : vec) result += str + separator;
        
        return result;
    }
    
    
    // TODO: Use universal reference & forward.
    inline std::vector<std::string> split_string(const std::string& str, std::string_view separator = "\n") {
        std::vector<std::string> result;
        
        std::size_t last = 0;
        for (std::size_t next = str.find(separator); next != std::string::npos; next = str.find(separator, last)) {
            result.push_back(str.substr(last, next - last));
        }
    
        result.push_back(str.substr(last, std::string::npos));
        return result;
    }
    
    
    template <typename Value, typename Container>
    constexpr inline bool contains(const Value& value, const Container& ctr) {
        return std::find(ctr.begin(), ctr.end(), value) != ctr.end();
    }
    
    
    template <typename Value, typename... Values>
    constexpr inline bool contains(const Value& value, const Values&... values) {
        return contains(value, std::array { values... });
    }
    
    
    template <typename Pred, typename Container>
    constexpr inline bool contains_if(const Pred& pred, const Container& ctr) {
        return std::find_if(ctr.begin(), ctr.end(), pred) != ctr.end();
    }
    
    
    template <typename Container>
    constexpr inline void to_lower(Container& ctr) {
        std::transform(ctr.begin(), ctr.end(), std::tolower);
    }
    
    
    template <typename Container>
    constexpr inline void to_upper(Container& ctr) {
        std::transform(ctr.begin(), ctr.end(), std::toupper);
    }
    
    
    template <typename Ctr, typename CB = std::remove_cvref_t<Ctr>>
    inline CB repeat(Ctr&& ctr, std::size_t count) {
        CB result = std::forward<Ctr>(ctr);
        std::size_t size = result.size();
        
        result.reserve(size * count);
        
        for (std::size_t i = 1; i < count; ++i) {
            std::copy(
                result.begin() + (size * i),
                result.begin() + (size * (i + 1)),
                result.begin() + (size * (i + 1))
            )
        }
        
        return result;
    }
    
    
    template <std::size_t Count, typename T, std::size_t N>
    constexpr inline std::array<T, N * Count> repeat(const std::array<T, N>& ctr) {
        return create_filled_array<T, N * Count>(
            [&, src_idx = 0](std::size_t) constexpr mutable {
                if (src_idx >= N) src_idx = 0;
                return ctr[src_idx++];
            }
        );
    }
    
    
    template <typename It, typename Fn> constexpr It find_nth_if(It first, It last, Fn&& fn, std::size_t n) {
        return std::find_if(
            first,
            last,
            [&](const auto& elem) mutable {
                return fn(elem) && (n-- == 0);
            }
        );
    }
    
    
    template <typename T, std::size_t N> constexpr inline vec<N, T> one_hot_vector(std::size_t index, T&& value = T(1)) {
        vec<N, T> result;
        result[index] = value;
        
        return result;
    }
    
    
    template <typename T>
    constexpr inline void append(std::vector<T>& a, std::vector<T>&& b) {
        a.insert(a.end(), std::make_move_iterator(b.begin()), std::make_move_iterator(b.end()));
    }
    
    
    template <typename T>
    constexpr inline void append(std::vector<T>& a, const std::vector<T>& b) {
        a.insert(a.end(), b.begin(), b.end());
    }
    
    
    template <typename Ctr, typename... Ctrs>
    constexpr inline auto join(Ctr&& ctr, Ctrs&&... ctrs) {
        if constexpr (requires { ctr.reserve(0); }) {
            ctr.reserve(ctr.size() + (ctrs.size() + ...));
        }
        
        ([&](auto&& c) {
            if constexpr (std::is_rvalue_reference_v<decltype(c)>) {
                ctr.insert(ctr.end(), std::make_move_iterator(c.begin()), std::make_move_iterator(c.end()));
            } else {
                ctr.insert(ctr.end(), c.begin(), c.end());
            }
        }(std::forward<Ctrs>(ctrs)), ...);
        
        return std::forward<Ctr>(ctr);
    }
}