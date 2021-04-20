#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>

#include <cmath>
#include <limits>


namespace ve {
    template <typename T> constexpr T max_value = std::numeric_limits<T>::max();
    template <typename T> constexpr T min_value = std::numeric_limits<T>::lowest();
    template <typename T> constexpr T infinity  = std::numeric_limits<T>::infinity();
    
    
    template <typename T> constexpr inline T radians(const T& degrees) {
        return degrees * (T(pi) / T(180));
    }
    
    
    template <typename T> constexpr inline T degrees(const T& radians) {
        return radians * (T(180) / T(pi));
    }
    
    
    template <typename T> requires std::is_integral_v<T>
    constexpr inline T pow(T v, T exp) {
        T result = 1;
        
        while (true) {
            if (exp & 1) result *= v;
            exp >>= 1;
            
            if (!exp) break;
            v *= v;
        }
        
        return result;
    }
    
    
    template <typename T> constexpr inline bool in(const T& val, const T& min, const T& max) {
        return val >= min && val < max;
    }
    
    
    template <typename T>
    [[nodiscard]] constexpr inline T square(const T& val) {
        return val * val;
    }
    
    
    template <typename T>
    [[nodiscard]] constexpr inline T cube(const T& val) {
        return val * val * val;
    }
    
    
    template <typename T>
    [[nodiscard]] constexpr inline T flatten(const vec3<T>& pos, const T& size) {
        return pos.z + (pos.y * size) + (pos.x * square(size));
    }
    
    
    template <typename T>
    [[nodiscard]] constexpr inline vec3<T> unflatten(const T& pos, const T& size) {
        T x = pos / square(size);
        T y = (pos / size) % size;
        T z = pos % size;
        
        return { x, y, z };
    }
    
    
    template <typename T>
    constexpr inline mat4<T> translation_matrix(const vec3<T>& pos) {
        return glm::translate(glm::identity<mat4<T>>(), pos);
    }
    
    
    template <typename Vec, typename Pred>
    constexpr static void foreach_dimension(Pred pred) {
        constexpr bool pred_requires_index = std::is_invocable_v<Pred, std::size_t>;
        
        
        auto iterate_dimension = [&] <std::size_t Dim> (auto& self) {
            if constexpr (pred_requires_index) pred(Dim);
            else pred();
            
            if constexpr (Dim + 1 < meta::glm_traits<Vec>::rows) {
                self.template operator()<Dim + 1>(self);
            }
        };
        
        
        iterate_dimension.template operator()<0>(iterate_dimension);
    }
    
    
    template <typename Vec, typename Pred>
    constexpr static void spatial_iterate(const Vec& center, const Vec& radius, Pred pred) {
        auto iterate_dim = [&] <std::size_t Dim> (auto& self, Vec& pos) {
            for (typename Vec::value_type i = center[Dim] - radius[Dim]; i <= center[Dim] + radius[Dim]; ++i) {
                pos[Dim] = i;
                
                if constexpr (Dim + 1 < meta::glm_traits<Vec>::size) {
                    self.template operator()<Dim + 1>(self, pos);
                } else {
                    pred(pos);
                }
            }
        };
        
        
        Vec position;
        iterate_dim.template operator()<0>(iterate_dim, position);
    }
    
    
    template <typename T, typename Fn, typename DFn> requires std::is_floating_point_v<T>
    constexpr static T newtons_method(Fn fn, DFn derivative, T guess, std::size_t iterations = 24) {
        for (std::size_t i = 0; i < iterations; ++i) {
            guess -= fn(guess) / derivative(guess);
        }
        
        return guess;
    }
    
    
    template <typename T>
    constexpr static T constexpr_sqrt(T value) {
        if (!std::is_constant_evaluated()) return std::sqrt(value);
        
        return newtons_method(
            [&](T guess) { return guess * guess - value; },
            [&](T guess) { return 2 * guess; },
            value
        );
    }
}