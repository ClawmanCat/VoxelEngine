#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/graphics/shader/compiler.hpp>
#include <VoxelEngine/graphics/shader/layout_validator.hpp>
#include <VoxelEngine/platform/graphics/opengl/vertex/layout.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/category.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/render_context.hpp>

#include <gl/glew.h>
#include <ctti/type_id.hpp>


namespace ve::gfx::opengl {
    class shader {
    public:
        ve_shared_only(
            shader,
            GLuint id,
            reflect::shader_reflection reflection,
            const pipeline_category_t* category,
            ctti::type_id_t vertex_type,
            detail::vertex_layout vertex_layout
        ) :
            id(id),
            reflection(std::move(reflection)),
            category(category),
            vertex_type(std::move(vertex_type)),
            vertex_layout(std::move(vertex_layout))
        {}

        ~shader(void) {
            glDeleteShader(id);
        }

        ve_immovable(shader);


        void bind(render_context& ctx) {
            // If the shader used by the pipeline changed, the uniform storage may now contain dead pointers,
            // so clear it before the next render pass.
            // (Also the UBOs themselves may have changed.)
            if (ctx.uniform_state.storage_owner != this) {
                ctx.uniform_state.bound_uniforms.clear();
                ctx.uniform_state.storage_owner = this;
            }

            auto& bound_state = ctx.uniform_state.bound_uniforms;


            for (const auto& stage : reflection.stages) {
                for (const auto& ubo : stage.second.uniform_buffers) {
                    if (auto it = bound_state.find(ubo.name); it != bound_state.end()) {
                        // Uniform was already present. Don't reallocate if the type info matches.
                        if (*it->second.type == ubo.type) {
                            // Note: we don't have to mark the UBO as dirty here:
                            // A new value must be pushed for the uniform before the UBO is synchronized,
                            // at which point the (std140) serialized versions of the values will be compared.
                            // The UBO may be marked as dirty at that point, if the serialized data differs from that in the UBO.
                            it->second.value = nullptr;
                            continue;
                        }
                    }

                    // Note on lifetime: uniforms are bound on this structure while a pipeline is bound with this shader.
                    // The only possible invalidation occurs when the pipeline switches shaders,
                    // which is handled by keeping track of which shader owns the storage of the current uniform state.
                    // (See usage of ctx.uniform_state.storage_owner)
                    ctx.uniform_state.bound_uniforms.insert_or_assign(
                        ubo.name,
                        uniform_bind_state::combined_value {
                            .value        = nullptr,
                            .value_std140 = std::vector<u8> { },
                            .ubo          = uniform_buffer(&ubo),
                            .ubo_dirty    = true,
                            .type         = &ubo.type
                        }
                    );

                    // For UBO structs with one member, allow setting that member directly.
                    // (See uniform_bind_state.hpp)
                    if (ubo.members.size() == 1) {
                        ctx.uniform_state.aliases.emplace(ubo.members[0].name, ubo.name);
                    }
                }
            }


            glUseProgram(id);
            vertex_layout.bind();
        }


        VE_GET_VAL(id);
        VE_GET_VAL(category);
        VE_GET_CREF(reflection);
        VE_GET_CREF(vertex_type);
        VE_GET_CREF(vertex_layout);
    private:
        GLuint id = 0;
        reflect::shader_reflection reflection;
        const pipeline_category_t* category;

        // It is unlikely that two different vertex layouts will be compatible with the same shader,
        // so just assume one vertex type per shader.
        ctti::type_id_t vertex_type;
        detail::vertex_layout vertex_layout;
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
        const auto& [input_stage, input_reflection] = data.reflection.get_input_stage();
        validate_vertex_layout<Vertex>(input_reflection.inputs);


        std::vector<GLuint> stages;

        for (const auto& [stage, spirv] : data.spirv) {
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


        return shader::create(
            program,
            data.reflection,
            input_stage->pipeline,
            ctti::type_id<Vertex>(),
            detail::generate_vertex_layout<Vertex>(input_reflection)
        );
    }
}