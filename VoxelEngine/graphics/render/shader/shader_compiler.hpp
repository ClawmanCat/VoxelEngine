#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/io/io.hpp>
#include <VoxelEngine/utils/logger.hpp>
#include <VoxelEngine/utils/container_utils.hpp>
#include <VoxelEngine/utils/meta/if_constexpr.hpp>
#include <VoxelEngine/graphics/render/shader/shader_program.hpp>
#include <VoxelEngine/graphics/render/shader/shader_type.hpp>

#include <magic_enum.hpp>
#include <GL/glew.h>

#include <optional>
#include <string_view>
#include <filesystem>
#include <vector>
#include <numeric>


namespace ve {
    class shader_compiler {
    public:
        shader_compiler(void) = delete;
    
        
        static std::optional<shader_program> compile_program(const fs::path& dir, std::string_view name) {
            const fs::path compute_shader_path = dir / (name + shader_types[(u32) shader_type::COMPUTE].file_ext);
        
            
            auto shaders = fs::exists(compute_shader_path)
                           ? compile_compute_shader(compute_shader_path)
                           : compile_pipeline_shader(dir, name);
        
            if (shaders == std::nullopt) return std::nullopt;
        
        
            GLuint program = glCreateProgram();
            for (GLuint shader : shaders.value()) glAttachShader(program, shader);
            glLinkProgram(program);
            for (GLuint shader : shaders.value()) glDeleteShader(shader);
        
            return check_errors<false>(program, GL_LINK_STATUS, name, "link shaders")
                ? std::optional { shader_program { program } }
                : std::nullopt;
        }
        
    private:
        static std::optional<std::vector<GLuint>> compile_pipeline_shader(const fs::path& dir, std::string_view name) {
            std::vector<GLuint> result;
            
            for (const auto& [ve_type, gl_type, file_ext, pipeline_type] : shader_types) {
                const fs::path path = dir / (name + file_ext);
                bool exists = fs::exists(path);
                
                if (!exists && pipeline_type == shader_pipeline_type::REQUIRED) {
                    VE_LOG_ERROR(
                        "Could not compile shader "s + name +
                        ": it is missing a " + magic_enum::enum_name(ve_type) + " stage."
                    );
                    
                    return std::nullopt;
                }
                
                if (exists) {
                    const auto src = io::read_text(path);
                    if (src == std::nullopt) return std::nullopt;
    
                    std::string src_str = join_strings(src.value());
                    const auto src_ptr = (const GLchar*) src_str.data();
                    
                    
                    GLuint shader = glCreateShader(gl_type);
                    glShaderSource(shader, 1, &src_ptr, nullptr);
                    
                    glCompileShader(shader);
                    if (!check_errors<true>(shader, GL_COMPILE_STATUS, name, "compile shader")) return std::nullopt;
                    
                    result.push_back(shader);
                }
            }
            
            return result;
        }
    
    
        static std::optional<std::vector<GLuint>> compile_compute_shader(const fs::path& file) {
            VE_ASSERT(false);   // Not implemented. (TODO)
            return std::nullopt;
        }
        
        
        template <bool shader> static bool check_errors(GLuint id, GLenum flag, std::string_view name, std::string_view action) {
            auto status_fn  = meta::return_if<shader>(glGetShaderiv,      glGetProgramiv);
            auto logger_fn  = meta::return_if<shader>(glGetShaderInfoLog, glGetProgramInfoLog);
            auto cleanup_fn = meta::return_if<shader>(glDeleteShader,     glDeleteProgram);
            
            GLint result = GL_TRUE;
            status_fn(id, flag, &result);
            
            if (result == GL_FALSE) {
                i32 length = 0;
                status_fn(id, GL_INFO_LOG_LENGTH, &length);
                
                std::string log;
                log.resize(length);
                logger_fn(id, length, &length, log.data());
                VE_LOG_ERROR("Failed to "s + action + " for pipeline " + name + ":\n" + log);
                
                cleanup_fn(id);
                return false;
            }
            
            return true;
        }
    };
}