#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/maybe_const.hpp>
#include <VoxelEngine/utils/constexpr_strcmp.hpp>

#include <cstddef>


namespace ve::meta {
    template <typename T> struct glm_traits {
        using self_type = T;
        using data_type = T;
        
        constexpr static std::size_t columns = 1;
        constexpr static std::size_t rows    = 1;
        constexpr static std::size_t size    = 1;
        
        constexpr static bool is_tensor = false;
        constexpr static bool is_vector = false;
        constexpr static bool is_matrix = false;
        
        
        template <maybe_const<T> CT>
        constexpr static CT& get_element(CT& object) { return object; }
    };
    
    
    template <typename T, std::size_t N> struct glm_traits<vec<N, T>> {
        using self_type = vec<N, T>;
        using data_type = T;
        
        constexpr static std::size_t columns = 1;
        constexpr static std::size_t rows    = N;
        constexpr static std::size_t size    = N;
        
        constexpr static bool is_tensor = true;
        constexpr static bool is_vector = true;
        constexpr static bool is_matrix = false;
    
        
        template <maybe_const<self_type> CT>
        constexpr static auto& get_element(CT& object, std::size_t index) { return object[index]; }
    };
    
    
    template <typename T, std::size_t C, std::size_t R> struct glm_traits<mat<C, R, T>> {
        using self_type = mat<C, R, T>;
        using data_type = T;
        
        constexpr static std::size_t columns = C;
        constexpr static std::size_t rows    = R;
        constexpr static std::size_t size    = C * R;
    
        constexpr static bool is_tensor = true;
        constexpr static bool is_vector = false;
        constexpr static bool is_matrix = true;
        
        
        template <maybe_const<self_type> CT>
        constexpr static auto& get_element(CT& object, std::size_t col, std::size_t row) {
            return object[col][row];
        }
        
        
        constexpr static bool is_shape(const char* shape) {
            auto digit_char = [](u8 digit) constexpr { return '0' + digit; };
            
            char buffer[3] = { '0', 'x', '0' };
            
            buffer[0] = digit_char((u8) columns);
            buffer[2] = digit_char((u8) rows);
            if (constexpr_strlen(shape) == 1) buffer[1] = '\0';
            
            return constexpr_strcmp(shape, buffer);
        }
    };
}