#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


// Constexpr versions of C string operations.
namespace ve {
    consteval bool constexpr_strcmp(const char* a, const char* b) {
        std::size_t i = 0;
        
        // If strings are of non-equal length, last comparison will be between
        // a character from one string against the null-terminator of the other,
        // which will correctly return false.
        while (a[i] != '\0' || b[i] != '\0') {
            if (a[i] != b[i]) return false;
            ++i;
        }
        
        return true;
    }
    
    
    consteval std::size_t constexpr_strlen(const char* str) {
        std::size_t i = 0;
        while (str[i] != '\0') ++i;
        return i;
    }
    
    
    // Disable compiler warning for branches without return statement.
    // Missing return statements are intentional, since this method is consteval,
    // errors should cause compilation failure.
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wreturn-type"
    template <typename T> requires std::is_integral_v<T>
    consteval T constexpr_stoi(std::string_view str) {
        auto is_digit = [](char c) { return c >= '0' && c <= '9'; };
        
        auto pow = [](std::size_t x, std::size_t exp) {
            std::size_t result = 1;
            while (exp--) result *= x;
            return result;
        };
        
        T result = 0;
        bool error = false;
        
        // First character could be - or +.
        std::size_t start = (str[0] == '-' || str[0] == '+');
        
        for (std::size_t i = start; i < str.length(); ++i) {
            if (is_digit(str[i])) {
                std::size_t value = str[i] - '0';
                result += (value * pow(10, i - start));
            } else {
                // String is not a number. Don't return a value and trigger a compile error.
                error = true;
            }
        }
        
        if (!error) return str[0] == '-' ? -result : result;
    }
    #pragma clang diagnostic pop
}