#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/graphics/shader/include_handler.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/get.hpp>

#include <shaderc/shaderc.hpp>


namespace ve::gfx::opengl::shader_helpers {
    inline const shaderc::CompileOptions& default_compile_options(void) {
        static shaderc::CompileOptions options = [] {
            shaderc::CompileOptions result;

            result.SetSourceLanguage(shaderc_source_language_glsl);
            result.SetTargetEnvironment(shaderc_target_env_opengl, 430);

            result.AddMacroDefinition("VE_GRAPHICS_API",    "opengl");
            result.AddMacroDefinition("VE_MAX_VS_SAMPLERS", to_string(gl_get<i32>(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS)));
            result.AddMacroDefinition("VE_MAX_FS_SAMPLERS", to_string(gl_get<i32>(GL_MAX_TEXTURE_IMAGE_UNITS)));
            result.AddMacroDefinition("VE_MAX_SAMPLERS",    to_string(gl_get<i32>(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)));
            result.AddMacroDefinition("VE_MAX_UBO_SIZE",    to_string(gl_get<i32>(GL_MAX_UNIFORM_BLOCK_SIZE)));

            result.SetIncluder(make_unique<gfx::detail::include_handler>(io::paths::PATH_SHADERS));

            result.SetAutoBindUniforms(true);
            result.SetAutoMapLocations(true);

            return result;
        }();

        return options;
    }
}