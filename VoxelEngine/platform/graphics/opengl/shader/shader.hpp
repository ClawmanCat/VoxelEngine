#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/graphics/shader/compiler.hpp>
#include <VoxelEngine/graphics/shader/layout_validator.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    class shader {
    public:
        ve_shared_only(shader, GLuint id, reflect::shader_reflection reflection)
            : id(id), reflection(std::move(reflection))
        {}

        ~shader(void) {
            glDeleteShader(id);
        }

        ve_immovable(shader);


        void bind(void) {
            glUseProgram(id);
        }


        VE_GET_VAL(id);
        VE_GET_CREF(reflection);
    private:
        GLuint id = 0;
        reflect::shader_reflection reflection;
    };


    namespace detail {
        template <bool Stage>
        inline void check_shader_errors(GLuint object, GLenum parameter) {
            auto status_fn = meta::pick<Stage>(glGetShaderiv, glGetProgramiv);
            auto logger_fn = meta::pick<Stage>(glGetShaderInfoLog, glGetProgramInfoLog);

            GLint success = GL_TRUE;
            status_fn(object, parameter, &success);

            if (success == GL_FALSE) {
                GLint length = 0;
                status_fn(object, GL_INFO_LOG_LENGTH, &length);

                std::string log;
                log.resize(length);
                logger_fn(object, length, &length, log.data());

                VE_ASSERT(false, "Shader compilation error:\n", log);
            }
        }
    }


    template <typename Vertex>
    inline shared<shader> make_shader(const shader_compilation_data& data) {
        std::vector<GLuint> stages;

        for (const auto& [stage, spirv] : data.spirv) {
            // Assure the provided vertex type matches the inputs of the shader.
            if (stage->first) {
                validate_vertex_layout<Vertex>(data.reflection.stages.at(stage).inputs);
            }

            GLuint id = glCreateShader(stage->opengl_type);
            VE_ASSERT(id, "Failed to create OpenGL shader object.");

            glShaderBinary(1, &id, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(u32));
            glSpecializeShader(id, "main", 0, nullptr, nullptr);
            detail::check_shader_errors<true>(id, GL_COMPILE_STATUS);

            stages.push_back(id);
        }


        GLuint program = glCreateProgram();
        VE_ASSERT(program, "Failed to create OpenGL shader program.");

        for (const auto& stage : stages) glAttachShader(program, stage);

        glLinkProgram(program);
        detail::check_shader_errors<false>(program, GL_LINK_STATUS);

        for (const auto& stage : stages) {
            glDetachShader(program, stage);
            glDeleteShader(stage);
        }


        return shader::create(program, data.reflection);
    }
}