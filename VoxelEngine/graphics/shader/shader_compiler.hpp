#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/io/io.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/utility.hpp>
#include <VoxelEngine/graphics/shader/shader_stage.hpp>

#include <shaderc/shaderc.hpp>


namespace ve::graphics {
    struct shader_stage_compile_result {
        const shader_stage_data* stage_info;
        std::vector<u32> spirv;
    };
    
    struct shader_compile_result {
        std::string name;
        std::vector<shader_stage_compile_result> stages;
    };
    
    
    class shader_compiler {
    public:
        shader_compiler(void) {
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
            options.SetSourceLanguage(shaderc_source_language_glsl);
            options.SetAutoBindUniforms(true);
            options.SetAutoMapLocations(true);
            options.SetGenerateDebugInfo();
    
            
            // TODO: Move this to platform graphics.
            #if VE_GRAPHICS_API == opengl
                options.SetTargetEnvironment(shaderc_target_env_opengl, 430);
            #elif VE_GRAPHICS_API == vulkan
                options.SetTargetEnvironment(shaderc_target_env_vulkan, 120);
            #else
                #error Unsupported graphics API for shader compiler.
            #endif
        }
        
        
        void define(const std::string& macro, const std::string& value) {
            options.AddMacroDefinition(macro, value);
        }
    
    
        shader_compile_result compile_shader(const std::string& name, const small_vector<fs::path>& paths) const {
            shader_compile_result result { .name = name };
            
            for (const auto& path : paths) {
                auto source = io::read_text(path);
                VE_ASSERT(source, "Failed to read shader file: "s + source.error().what());
                
                auto ext = io::full_extension(path);
                const auto& stage = std::find_if(shader_stage_info.begin(), shader_stage_info.end(), ve_field_equals(file_ext, ext));
                VE_ASSERT(stage != shader_stage_info.end(), "Unkown shader file extension: "s + ext);
                
                result.stages.push_back(compile_stage(name, join_strings(*source, "\n"), stage->stage));
            }
            
            return result;
        }
        
        
        shader_compile_result compile_shader(const std::string& name, const fs::path& folder = io::paths::PATH_SHADERS) const {
            small_vector<fs::path> paths;
            
            for (const auto& entry : fs::directory_iterator(folder)) {
                if (entry.is_regular_file() && io::full_stem(entry.path()) == name) {
                    paths.push_back(entry.path());
                }
            }
            
            return compile_shader(name, paths);
        }
    private:
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
    
    
        shader_stage_compile_result compile_stage(const std::string& name, const std::string& source, shader_stage stage) const {
            auto result = compiler.CompileGlslToSpv(source, shader_stage_info[(u32) stage].shaderc_kind, name.c_str(), options);
        
            VE_ASSERT(
                result.GetCompilationStatus() == shaderc_compilation_status_success,
                "Failed to compile shader: "s + result.GetErrorMessage()
            );
    
            return shader_stage_compile_result {
                .stage_info = &shader_stage_info[(u32) stage],
                .spirv      = { result.begin(), result.end() }
            };
        }
    };
}