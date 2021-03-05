#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/io.hpp>
#include <VoxelEngine/utility/utility.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader_type.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader_program.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader_layout.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader_layout_parser.hpp>

#include <GL/glew.h>
#include <magic_enum.hpp>

#include <algorithm>
#include <string_view>


namespace ve::graphics {
    struct shader_with_layout {
        shader_program shader;
        shader_layout layout;
    };
    
    
    class shader_compiler {
        struct shader_stage_info {
            fs::path path;
            shader_info stage;
        };
        
    public:
        shader_compiler(void) = delete;
        
        
        // Given a shader directory and a file name,
        // constructs a shader program out of all shader stages with that filename.
        static expected<shader_with_layout> compile_program(const fs::path& dir, std::string_view name) {
            small_vector<shader_stage_info> info;
            
            // Get all shaders matching the given filename which can be pipelined.
            for (const auto& stage : shader_stages) {
                fs::path path = dir / (name + stage.file_ext);
                
                if (stage.pipeline_type != shader_pipeline_type::FORBIDDEN && fs::exists(path)) {
                    info.push_back({ path, stage });
                }
            }
            
            // Compile found shaders.
            return compile_program_impl(info, name);
        }
        
        
        // Given a set of shader stage files, constructs a shader program from those files.
        static expected<shader_with_layout> compile_program(const small_vector<fs::path>& files, std::string_view name) {
            small_vector<shader_stage_info> info;
            
            // Find the type of each shader file by checking its file extension.
            for (const auto& file : files) {
                auto full_extension = io::full_extension(file);
                
                const auto& stage = std::find_if(
                    shader_stages.begin(),
                    shader_stages.end(),
                    [&](const shader_info& s) { return s.file_ext == full_extension; }
                );
                
                
                // Make sure all shader stages have a known type.
                if (stage == shader_stages.end()) {
                    std::string error = "Given shader stage file "s + file.string() + " does not have a recognised file extension.";
                    
                    VE_ASSERT(false, error);
                    return make_unexpected(std::move(error));
                }
                
                
                // Prevent compute / utility shaders from being compiled into the pipeline.
                if (stage->pipeline_type == shader_pipeline_type::FORBIDDEN) {
                    std::string error =
                        "Cannot add shader stage "s + file.string() + " to pipeline: "
                        "stages of type " + magic_enum::enum_name(stage->ve_type) + " cannot be used inside shader pipelines.";
    
                    VE_ASSERT(false, error);
                    return make_unexpected(std::move(error));
                }
                
                
                info.push_back({ file, *stage });
            }
    
            // Compile shader files.
            return compile_program_impl(info, name);
        }
        
    private:
        static expected<shader_with_layout> compile_program_impl(const small_vector<shader_stage_info>& stages, std::string_view name) {
            // Check if all required shader stages are present.
            VE_DEBUG_ONLY(
                auto required_stages = collect_if(
                    shader_stages,
                    [](shader_info info) { return info.pipeline_type == shader_pipeline_type::REQUIRED; }
                );
                
                for (const auto& required_stage : required_stages) {
                    VE_ASSERT(
                        contains_if(stages, [&](const auto& s) { return s.stage == required_stage; }),
                        "Shader program is missing required stage "s +
                        magic_enum::enum_name(required_stage.pipeline_type) + "."
                    );
                }
            );
    
            
            // Required later to parse the in- and outputs of the shader.
            io::text_file vs_source, fs_source;
    
            std::vector<GLuint> shader_ids;
            shader_ids.reserve(stages.size());
            
            
            // Compile each shader file.
            for (const auto& [file, info] : stages) {
                auto src = io::read_text(file);
                if (!src.has_value()) return make_unexpected(src.error());
                
                
                // OpenGL requires the source as one single string, rather than an array.
                std::string src_string = join_strings(*src, "\n");
                
                VE_DEBUG_ONLY(
                    const std::string file_string = file.string();  // Structured binding shenanigans.
                    VE_ASSERT(src_string.length() > 0, "Shader stage file "s + file_string + " is empty.");
                );
    
                const GLchar* src_ptr = (const GLchar*) src_string.data();
                
                
                // Store source if this is the vertex or the fragment shader.
                if (info.ve_type == shader_type::VERTEX  ) vs_source = std::move(src.value());
                if (info.ve_type == shader_type::FRAGMENT) fs_source = std::move(src.value());
                
                
                // Compile shader and check for errors.
                GLuint shader_id = glCreateShader(info.gl_type);
                glShaderSource(shader_id, 1, &src_ptr, nullptr);
                glCompileShader(shader_id);
                
                auto error = check_errors<SHADER>(
                    shader_id,
                    GL_COMPILE_STATUS,
                    name,
                    "compile "s + magic_enum::enum_name(info.ve_type) + " shader"
                );
                
                if (error) return make_unexpected(*error);
                else shader_ids.push_back(shader_id);
            }
            
            
            // Get program in- and outputs.
            auto program_layout = get_layout(vs_source, fs_source);
            
            
            // Link shader files into program and check for errors.
            GLuint program = glCreateProgram();
            for (GLuint shader : shader_ids) glAttachShader(program, shader);
            glLinkProgram(program);
            for (GLuint shader : shader_ids) glDeleteShader(shader);
            
            
            auto error = check_errors<PROGRAM>(program, GL_LINK_STATUS, name, "link shaders");
            
            if (error) return make_unexpected(*error);
            else return shader_with_layout { program, std::move(program_layout) };
        }
        
        
        enum shader_compile_stage { SHADER, PROGRAM };
        
        template <shader_compile_stage stage>
        static optional<std::string> check_errors(GLuint id, GLenum flag, std::string_view name, std::string_view action) {
            constexpr bool is_shader = (stage == SHADER);
            
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
    };
}