#pragma once

#include <VoxelEngine/core/core.hpp>

#include <ctti/type_id.hpp>


namespace ve::meta {
    struct null_type { };

    constexpr inline ctti::type_id_t null_type_id = ctti::type_id<null_type>();
}