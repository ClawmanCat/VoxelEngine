#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_storage.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader.hpp>


namespace ve::gfx::opengl {
    void uniform_storage::bind_uniforms_for_shader(const shader* shader, uniform_state_dict& state) const {
        // Already set check is required to prevent setting the uniform twice when using name aliasing.
        hash_set<const uniform*> already_set;

        for (const auto& [stage_info, stage] : shader->get_reflection().stages) {
            for (const auto& uniform : stage.uniform_buffers) {
                auto it = uniforms.find(uniform.name);

                // Allow aliasing of single-member UBOs to their contained value.
                if (it == uniforms.end() && uniform.members.size() == 1) {
                    it = uniforms.find(uniform.members.front().name);
                }

                if (it != uniforms.end() && !already_set.contains(it->second.get())) {
                    auto& target = state[uniform.name];

                    target.uniform_source = it->second.get();
                    target.current_value  = it->second->combine(target.current_value);

                    already_set.insert(it->second.get());
                }
            }
        }
    }


    void uniform_storage::bind_samplers_for_shader(const shader* shader, sampler_state_dict& state) const {
        for (const auto& [stage_info, stage] : shader->get_reflection().stages) {
            for (const auto& sampler : stage.samplers) {
                if (auto it = samplers.find(sampler.name); it != samplers.end()) {
                    state[sampler.name] = it->second;
                }
            }
        }
    }


    void uniform_storage::set_uniform(unique<uniform>&& uniform) {
        // Constructing of the string must occur before moving the uniform,
        // as order of evaluation within the function parameters is undefined.
        std::string name = uniform->get_name();
        uniforms.insert_or_assign(std::move(name), std::move(uniform));
    }


    void uniform_storage::remove_uniform(std::string_view name) {
        uniforms.erase(name);
        samplers.erase(name);
    }



    bool uniform_storage::has_uniform(std::string_view name) const {
        return uniforms.contains(name) || samplers.contains(name);
    }


    unique<uniform> uniform_storage::take_normal_uniform(std::string_view name) {
        return std::move(uniforms.extract(name).mapped());
    }


    shared<uniform_sampler> uniform_storage::take_sampler_uniform(std::string_view name) {
        return std::move(samplers.extract(name).mapped());
    }
}