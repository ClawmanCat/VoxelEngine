#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <typename T> struct glm_traits {
        constexpr static inline bool is_scalar = std::is_scalar_v<T>;
        constexpr static inline bool is_vector = false;
        constexpr static inline bool is_matrix = false;
        constexpr static inline bool is_glm    = false;

        constexpr static inline std::size_t num_rows = 1;
        constexpr static inline std::size_t num_cols = 1;

        using value_type = T;
    };


    template <typename T, std::size_t R> struct glm_traits<vec<R, T>> {
        constexpr static inline bool is_scalar = false;
        constexpr static inline bool is_vector = true;
        constexpr static inline bool is_matrix = false;
        constexpr static inline bool is_glm    = true;

        constexpr static inline std::size_t num_rows = R;
        constexpr static inline std::size_t num_cols = 1;

        using value_type = T;
    };



    template <typename T, std::size_t R, std::size_t C> struct glm_traits<mat<C, R, T>> {
        constexpr static inline bool is_scalar = false;
        constexpr static inline bool is_vector = false;
        constexpr static inline bool is_matrix = true;
        constexpr static inline bool is_glm    = true;

        constexpr static inline std::size_t num_rows = R;
        constexpr static inline std::size_t num_cols = C;

        using value_type = T;
    };
}