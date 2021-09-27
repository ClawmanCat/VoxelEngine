#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/paths.hpp>

#include <shaderc/shaderc.hpp>


namespace ve::gfx::opengl::shader_helpers {
    inline const shaderc::CompileOptions& default_compile_options(void) {
        static shaderc::CompileOptions options = [] {
            shaderc::CompileOptions result;

            result.SetSourceLanguage(shaderc_source_language_glsl);
            result.SetTargetEnvironment(shaderc_target_env_opengl, 430);
            result.AddMacroDefinition("VE_GRAPHICS_API", "opengl");

            result.SetIncluder(make_unique<detail::include_handler>(io::paths::PATH_SHADERS));

            result.SetAutoBindUniforms(true);
            result.SetAutoMapLocations(true);

            return result;
        }();

        return options;
    }
}