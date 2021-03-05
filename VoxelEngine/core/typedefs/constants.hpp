#pragma once

#include <numbers>


namespace ve {
    constexpr inline double pi     = std::numbers::pi;
    constexpr inline double tau    = 2 * std::numbers::pi;
    constexpr inline double sqrt_2 = std::numbers::sqrt2;
    constexpr inline double sqrt_3 = std::numbers::sqrt3;
    
    constexpr inline float pi_f     = std::numbers::pi;
    constexpr inline float tau_f    = 2 * std::numbers::pi;
    constexpr inline float sqrt_2_f = std::numbers::sqrt2;
    constexpr inline float sqrt_3_f = std::numbers::sqrt3;
}