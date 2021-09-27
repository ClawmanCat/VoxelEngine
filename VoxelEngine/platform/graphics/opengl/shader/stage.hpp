#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/category.hpp>

#include <gl/glew.h>
#include <shaderc/shaderc.hpp>


namespace ve::gfx::opengl {
    struct shader_stage {
        std::string name;
        std::string file_extension;

        shaderc_shader_kind shaderc_type;
        GLenum opengl_type;


        const pipeline_category_t* pipeline;

        enum pipeline_mode_t {
            REQUIRED,   // The shader stage must be present for all shaders.
            VARIANT,    // At least one variant with the same opengl type must be present for all shaders.
            OPTIONAL    // The shader stage is not required.
        } pipeline_mode;


        bool first = false;
        bool last  = false;
    };


    // Yes, this could be inline. It could be an std::array with deduced size even,
    // were it not for the fact that Clang completely fails to parse the initializer list for some obscure reason if it is declared here.
    const extern std::vector<shader_stage> shader_stages;
}