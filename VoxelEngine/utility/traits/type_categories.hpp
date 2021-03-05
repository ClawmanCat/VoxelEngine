#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack.hpp>


namespace ve::meta {
    using integral_types = pack<i8, u8, i16, u16, i32, u32, i64, u64>;
    using float_types    = pack<f32, f64>;
    using scalar_types   = typename integral_types::template append_pack<float_types>;
}