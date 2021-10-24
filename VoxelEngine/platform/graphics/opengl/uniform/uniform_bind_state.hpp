#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/object_type.hpp>
#include <VoxelEngine/graphics/uniform/uniform_sampler.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/get.hpp>


namespace ve::gfx::opengl {
    class uniform_storage;
    class shader;


    struct uniform_bind_state {
        struct combined_value {
            const void* value;
            std::vector<u8> value_std140;
            const reflect::object_type* type;

            // Updating the UBO is just updating the GPUs view of the data, so allow this as a const action.
            mutable uniform_buffer ubo;


            // Pushes the current value of the uniform to the GPU. Call this before binding the UBO.
            void synchronize_ubo(void) const {
                VE_ASSERT(value, "Attempt to store uninitialized uniform in UBO.");
                ubo.store_bytes(value_std140);
            }
        };

        const shader* storage_owner = nullptr;
        std::stack<const uniform_storage*> uniform_stack;
        hash_map<std::string_view, combined_value> bound_uniforms;

        // When a UBO contains a single element, e.g. uniform U_Transform { mat4f transform; };
        // it is often desirable to set the uniform by the name of its member, rather than by creating a separate
        // struct to wrap it.
        // Keep track of what UBOs only have a single member, and what UBO those member names map to.
        hash_map<std::string_view, std::string_view> aliases;


        struct sampler_info {
            std::size_t location, count;
            texture_list textures;
        };

        hash_map<std::string_view, sampler_info> samplers;


        void bind_state(GLuint current_shader) const {
            for (const auto& [name, ubo] : bound_uniforms) {
                ubo.synchronize_ubo();
                ubo.ubo.bind();
            }


            // TODO: Move this to a separate file, most of the contents of opengl/uniform could be in the common library.
            std::size_t texture_unit = 0;

            for (const auto& [name, info] : samplers) {
                for (const auto& [i, texture] : info.textures | views::enumerate) {
                    const static std::size_t texture_limit = (std::size_t) gl_get<i32>(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
                    VE_ASSERT(texture_unit < texture_limit, "Attempt to bind too many textures. Hardware limits exceeded.");

                    glActiveTexture(GL_TEXTURE0 + texture_unit);
                    glBindTexture(GL_TEXTURE_2D, texture->get_id());


                    GLuint location;

                    if (info.textures.size() == 1) {
                        location = (GLuint) info.location;
                    } else {
                        // This is very unfortunate, but there seems to be no guarantee that locations within the array will be continuous,
                        // and SPIRV Reflect provides no way to query any location other than the start of the array.
                        // TODO: Find a way to do this without querying OpenGL, or at least cache the result.
                        std::string index_name = cat(name, "[", i, "]");
                        location = glGetUniformLocation(current_shader, index_name.c_str());
                    }


                    glUniform1i(location, texture_unit);
                    ++texture_unit;
                }
            }
        }


        bool requires_value(std::string_view name) const {
            return bound_uniforms.contains(name);
        }
    };
}