#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::gfx::opengl {
    struct pipeline_category_t {
        std::string_view name;
    };


    namespace pipeline_category {
        constexpr inline pipeline_category_t RASTERIZATION {
            .name = "rasterization"
        };


        constexpr inline pipeline_category_t COMPUTE {
            .name = "compute"
        };
    }
}