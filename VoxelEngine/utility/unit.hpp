#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    constexpr inline u64 operator"" _kb(u64 input) { return 1'000ull * input; }
    constexpr inline u64 operator"" _mb(u64 input) { return 1'000'000ull * input; }
    constexpr inline u64 operator"" _gb(u64 input) { return 1'000'000'000ull * input; }
    constexpr inline u64 operator"" _tb(u64 input) { return 1'000'000'000'000ull * input; }

    constexpr inline u64 operator"" _kib(u64 input) { return 1'024ull * input; }
    constexpr inline u64 operator"" _mib(u64 input) { return 1'024ull * 1'024ull * input; }
    constexpr inline u64 operator"" _gib(u64 input) { return 1'024ull * 1'024ull * 1'024ull * input; }
    constexpr inline u64 operator"" _tib(u64 input) { return 1'024ull * 1'024ull * 1'024ull * 1'024ull * input; }
}