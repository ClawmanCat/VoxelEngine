#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/shader_compiler.hpp>
#include <VoxelEngine/graphics/shader/shader_stage.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>

#include <GL/glew.h>


namespace ve::graphics::detail {
    const inline flat_map<shader_stage, GLenum> shader_stage_gl_types {
        { shader_stage::VERTEX,    GL_VERTEX_SHADER          },
        { shader_stage::FRAGMENT,  GL_FRAGMENT_SHADER        },
        { shader_stage::GEOMETRY,  GL_GEOMETRY_SHADER        },
        { shader_stage::TESS_CTRL, GL_TESS_CONTROL_SHADER    },
        { shader_stage::TESS_EVAL, GL_TESS_EVALUATION_SHADER },
        { shader_stage::COMPUTE,   GL_COMPUTE_SHADER         }
    };
    
    
    template <bool is_shader> // true for shader errors, false for link errors.
    inline optional<std::string> check_errors(GLuint id, GLenum flag, std::string_view name, std::string_view action) {
        auto status_fn  = meta::pick<is_shader>(glGetShaderiv, glGetProgramiv);
        auto logger_fn  = meta::pick<is_shader>(glGetShaderInfoLog, glGetProgramInfoLog);
        auto cleanup_fn = meta::pick<is_shader>(glDeleteShader, glDeleteProgram);
        
        GLint result = GL_TRUE;
        status_fn(id, flag, &result);
        
        if (result == GL_FALSE) {
            i32 length = 0;
            status_fn(id, GL_INFO_LOG_LENGTH, &length);
            
            std::string log;
            log.resize(length);
            logger_fn(id, length, &length, log.data());
            
            cleanup_fn(id);
            return "Failed to "s + action + " for shader program " + name + ":\n" + log;
        }
        
        return nullopt;
    }
    
    
    inline GLuint spirv_to_gl_shader(const shader_compile_result& shader_binaries) {
        small_vector<GLuint> shader_ids;
        
        for (const auto& stage : shader_binaries.stages) {
            GLuint shader_id = glCreateShader(shader_stage_gl_types.nth((u32) stage.stage_info->stage)->second);
            
            glShaderBinary(
                1,
                &shader_id,
                GL_SHADER_BINARY_FORMAT_SPIR_V,
                (const GLuint*) &stage.spirv[0],
                stage.spirv.size() * sizeof(u32)
            );
            
            glSpecializeShader(
                shader_id,
                "main",
                0,
                nullptr,
                nullptr
            );
            
            auto error = check_errors<true>(shader_id, GL_COMPILE_STATUS, shader_binaries.name, "load shader from binary");
            if (error) VE_ASSERT(false, *error);
            
            shader_ids.push_back(shader_id);
        }
        
        
        GLuint program_id = glCreateProgram();
        for (GLuint shader : shader_ids) glAttachShader(program_id, shader);
        glLinkProgram(program_id);
        for (GLuint shader : shader_ids) glDetachShader(program_id, shader);
    
        auto error = check_errors<false>(program_id, GL_LINK_STATUS, shader_binaries.name, "link shaders");
        if (error) VE_ASSERT(false, *error);
        
        return program_id;
    }
}