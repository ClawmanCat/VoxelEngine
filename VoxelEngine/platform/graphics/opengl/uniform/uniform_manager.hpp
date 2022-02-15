#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform.hpp>
#include <VoxelEngine/graphics/uniform/uniform_sampler.hpp>


namespace ve::gfx::opengl {
    class uniform_storage;
    class shader;


    struct uniform_state {
        const void* current_value = nullptr;
        const uniform* uniform_source = nullptr;
    };

    using uniform_state_dict = hash_map<std::string, uniform_state>;
    using sampler_state_dict = hash_map<std::string, shared<uniform_sampler>>;


    class uniform_manager {
    public:
        void push_uniforms(const uniform_storage* storage);
        void pop_uniforms(void);

        void bind_uniforms_for_shader(const shader* shader) const;
    private:
        std::vector<const uniform_storage*> uniform_sources;
        mutable hash_map<std::string, uniform_buffer> ubos;
    };
}