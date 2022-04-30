#include <VoxelEngine/platform/graphics/opengl/shader/make_shader.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader.hpp>
#include <VoxelEngine/graphics/shader/layout_validator.hpp>
#include <VoxelEngine/graphics/shader/compiler/cache.hpp>

namespace ve::gfx::opengl {
    struct specialization_constants { std::vector<GLuint> ids, values; };

    specialization_constants get_specialization_constants(const shader_compilation_data& data, const shader_stage* stage) {
        specialization_constants result;
        auto& set_constants = data.settings.specialization_settings.constants;

        for (const auto& constant : data.reflection.stages.at(stage).specialization_constants) {
            if (auto it = set_constants.find(constant.name); it != set_constants.end()) {
                // Yes, this is actually how OpenGL expects you to do this...
                GLuint value = 0;
                std::visit([&] <typename T> (const T& v) {
                    static_assert(sizeof(T) <= sizeof(GLuint), "Illegal specialization constant type.");
                    memcpy(&value, &v, sizeof(T));
                }, it->second);

                result.values.push_back(value);
                result.ids.push_back((GLuint) constant.binding);
            }
        }

        return result;
    }


    template <bool Stage> void check_shader_errors(GLuint object, GLenum parameter) {
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


    shared<shader> make_shader(
        const shader_compilation_data& data,
        shader_cache* cache,
        const ctti::type_id_t& vertex_type,
        std::span<const vertex_attribute> vertex_layout,
        const detail::vertex_layout& vertex_layout_reflection
    ) {
        const auto& [input_stage, input_reflection] = data.reflection.get_input_stage();
        validate_vertex_layout(vertex_type.name().cppstring(), vertex_layout, input_reflection.inputs);

        std::vector<GLuint> stages;
        for (const auto& [stage, spirv] : data.spirv_blobs) {
            auto constants     = get_specialization_constants(data, stage);
            bool has_constants = !constants.ids.empty();

            GLuint id = glCreateShader(stage->opengl_type);
            VE_ASSERT(id, "Failed to create OpenGL shader object.");

            glShaderBinary(1, &id, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(u32));
            glSpecializeShader(id, "main", (GLuint) constants.values.size(), has_constants ? constants.ids.data() : nullptr, has_constants ? constants.values.data() : nullptr);
            check_shader_errors<true>(id, GL_COMPILE_STATUS);

            stages.push_back(id);
        }


        GLuint program = glCreateProgram();
        VE_ASSERT(program, "Failed to create OpenGL shader program.");

        for (const auto& stage : stages) glAttachShader(program, stage);

        glLinkProgram(program);
        check_shader_errors<false>(program, GL_LINK_STATUS);

        for (const auto& stage : stages) {
            glDetachShader(program, stage);
            glDeleteShader(stage);
        }


        // Cannot make_shared: only we have friend access.
        auto result = shared<shader>(new shader());

        result->id                       = program;
        result->category                 = input_stage->pipeline;
        result->managing_cache           = cache;
        result->original_compile_data    = data;
        result->vertex_layout            = vertex_layout;
        result->vertex_type              = vertex_type;
        result->vertex_layout_reflection = vertex_layout_reflection;

        return result;
    }
}