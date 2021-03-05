#pragma once

#include <cstdint>
#include <limits>


namespace ve {
    using u8  =  uint8_t;
    using i8  =   int8_t;
    using u16 = uint16_t;
    using i16 =  int16_t;
    using u32 = uint32_t;
    using i32 =  int32_t;
    using u64 = uint64_t;
    using i64 =  int64_t;
    
    using f32 = float;
    using f64 = double;
    
    static_assert(std::numeric_limits<f32>::is_iec559, "Non-conforming implementation of f32 detected. Floating point types must fulfill IEEE 754.");
    static_assert(std::numeric_limits<f64>::is_iec559, "Non-conforming implementation of f64 detected. Floating point types must fulfill IEEE 754.");
    
    
    constexpr i8  operator"" _b (u64 value) { return i8 (value); }
    constexpr u8  operator"" _ub(u64 value) { return u8 (value); }
    constexpr i16 operator"" _s (u64 value) { return i16(value); }
    constexpr u16 operator"" _us(u64 value) { return u16(value); }
}