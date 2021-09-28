#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/dont_deduce.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_bind_state.hpp>


namespace ve::gfx::opengl {
    class uniform_storage {
    public:
        void push(uniform_bind_state& state) {
            // - If stack empty, initialize keys as self uniform names.
            // - Push self to state.
            // - Combine self uniforms with current uniforms.
        }


        void pop(uniform_bind_state& state) {
            // - Pop self from state.
            // - If state not empty, restore previous state.
            // - Otherwise clear keys.
        }


        void set_uniform(unique<uniform>&& uniform) {
            VE_ASSERT(!is_bound, "Cannot modify uniform storage while it is being used.");
            uniforms.insert_or_assign(uniform->name, std::move(uniform));
        }


        void remove_uniform(std::string_view name) {
            VE_ASSERT(!is_bound, "Cannot modify uniform storage while it is being used.");
            uniforms.erase(name);
        }


        template <typename T, typename Combine>
        void set_uniform_value(std::string name, meta::dont_deduce<T> value, Combine&& combine = combine_functions::overwrite) {
            set_uniform(make_unique<uniform_value<T, Combine>>(std::move(name), std::move(value), fwd(combine)));
        }


        template <typename T, typename Produce, typename Combine>
        void set_uniform_producer(std::string name, Produce&& produce, Combine&& combine = combine_functions::overwrite) {
            set_uniform(make_unique<uniform_producer<T, Produce, Combine>>(std::move(name), fwd(produce), fwd(combine)));
        }
    private:
        hash_map<std::string_view, unique<uniform>> uniforms;
        bool is_bound = false;
    };
}