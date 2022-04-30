#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_manager.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_storage.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/get.hpp>


namespace ve::gfx::opengl {
    void uniform_manager::push_uniforms(const uniform_storage* storage) {
        uniform_sources.push_back(storage);
    }


    void uniform_manager::pop_uniforms(void) {
        uniform_sources.pop_back();
    }


    void uniform_manager::bind_uniforms_for_shader(const shader* shader) const {
        const auto& reflection = shader->get_reflection();
        hash_set<std::string_view> already_stored { };


        // Load values for uniforms.
        uniform_state_dict uniform_state { };
        for (const auto& storage : uniform_sources | views::indirect) {
            storage.bind_uniforms_for_shader(shader, uniform_state);
        }


        // Store values in UBO & bind.
        for (const auto& [stage_info, stage] : reflection.stages) {
            for (const auto& uniform : stage.uniform_buffers) {
                if (already_stored.contains(uniform.name)) continue;


                auto ubo_source = uniform_state.find(uniform.name);
                VE_ASSERT(
                    ubo_source != uniform_state.end(),
                    "Cannot bind uniforms for shader ", shader->get_reflection().name, ": no value for uniform ", uniform.name, " has been provided."
                );


                // If the UBO already exists, it must have a matching reflection to be compatible,
                // otherwise we will have to create a new one.
                bool ubo_valid = false;
                decltype(ubos)::iterator it;
                if (it = ubos.find(uniform.name); it != ubos.end()) {
                    ubo_valid = (it->second.get_reflection().type == uniform.type);
                }

                if (!ubo_valid) {
                    it = ubos.insert_or_assign(it, uniform.name, uniform_buffer { uniform });
                }

                const auto& src = ubo_source->second;
                it->second.store_bytes(src.uniform_source->to_std140(src.current_value));


                it->second.bind(uniform.binding);
                already_stored.insert(uniform.name);
            }
        }


        // Load values for samplers.
        sampler_state_dict sampler_state;
        for (const auto& storage : uniform_sources | views::indirect) {
            storage.bind_samplers_for_shader(shader, sampler_state);
        }


        // Bind samplers.
        std::size_t texture_unit = 0;

        for (const auto& [stage_info, stage] : reflection.stages) {
            for (const auto& sampler : stage.samplers) {
                if (already_stored.contains(sampler.name)) continue;


                auto sampler_source = sampler_state.find(sampler.name);
                VE_ASSERT(
                    sampler_source != sampler_state.end(),
                    "Cannot bind uniforms for shader ", shader->get_reflection().name, ": no value for sampler ", sampler.name, " has been provided."
                );


                auto textures = sampler_source->second->get_uniform_textures();

                for (const auto& [i, texture] : textures | views::enumerate) {
                    const static std::size_t texture_limit = (std::size_t) gl_get<i32>(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
                    VE_ASSERT(texture_unit < texture_limit, "Attempt to bind too many textures. Hardware limits exceeded.");

                    glActiveTexture(GL_TEXTURE0 + texture_unit);
                    glBindTexture(GL_TEXTURE_2D, texture->get_id());


                    // spirv_reflect seems to provide no way to query the locations of all but the first sampler in an array.
                    // While this is fine for Vulkan, which allows the client to set locations, it poses an issue for OpenGL.
                    // For now, just use the built-in OpenGL reflection system to acquire the locations. This means shaders MUST be compiled with debug symbols.
                    // TODO: Figure out a better way to do this.
                    if (sampler.type.array_extents.empty()) {
                        glUniform1i((GLuint) sampler.location, texture_unit);
                    } else {
                        std::string index_name = cat(sampler.name, "[", i, "]");
                        glUniform1i(glGetUniformLocation(shader->get_id(), index_name.c_str()), texture_unit);
                    }

                    ++texture_unit;
                }


                already_stored.insert(sampler.name);
            }
        }
    }
}