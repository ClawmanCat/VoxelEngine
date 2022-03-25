#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/get.hpp>

#include <shaderc/shaderc.hpp>


namespace ve::gfx::opengl::compiler_config {
    inline void prepare_compile_options(shaderc::CompileOptions& options) {
        options.SetSourceLanguage(shaderc_source_language_glsl);
        options.SetTargetEnvironment(shaderc_target_env_opengl, 430);
        options.SetAutoBindUniforms(true);
        options.SetAutoMapLocations(true);
    }


    inline void prepare_default_preprocessor(auto& preprocessor) {
        auto add_macro_kv = [&] (std::string_view name, const auto& value) {
            preprocessor.add_macro(cat(name, "=", value));
        };

        add_macro_kv("VE_GRAPHICS_API",    "opengl");
        add_macro_kv("VE_MAX_VS_SAMPLERS", gl_get<i32>(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS));
        add_macro_kv("VE_MAX_FS_SAMPLERS", gl_get<i32>(GL_MAX_TEXTURE_IMAGE_UNITS));
        add_macro_kv("VE_MAX_SAMPLERS",    gl_get<i32>(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS));
        add_macro_kv("VE_MAX_UBO_SIZE",    gl_get<i32>(GL_MAX_UNIFORM_BLOCK_SIZE));
    }
}