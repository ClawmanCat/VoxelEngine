#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/dont_deduce.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_bind_state.hpp>


namespace ve::gfx::opengl {
    class uniform_storage {
    public:
        void push(uniform_bind_state& state) const {
            for (const auto& [name, uniform] : uniforms) {
                auto it = state.bound_uniforms.find(name);

                if (it == state.bound_uniforms.end() || it->second.value == nullptr) {
                    std::tie(it, std::ignore) = state.bound_uniforms.insert_or_assign(
                        name,
                        uniform_bind_state::combined_value { .value = nullptr, .type = &(uniform->stored_type) }
                    );
                } else {
                    // Can't capture structured binding in assert.
                    VE_DEBUG_ONLY(
                        const std::string_view uniform_name = name;
                        const bool types_equal = reflect::compare_spirtypes(*(it->second.type), uniform->stored_type) == std::strong_ordering::equal;

                        VE_DEBUG_ASSERT(types_equal, "Cannot merge uniforms for ", uniform_name, ": stored uniform and pushed uniform have different types.");
                    );
                }

                it->second.value = uniform->push(it->second.value);
            }

            state.uniform_stack.push(this);
            is_bound = true;
        }


        void restore(uniform_bind_state& state) const {
            VE_DEBUG_ASSERT(state.uniform_stack.top() == this, "Only the topmost uniform storage object in the uniform stack may be restored.");

            for (const auto& [name, uniform] : uniforms) {
                state.bound_uniforms[name].value = uniform->get();
            }
        }


        void pop(uniform_bind_state& state) const {
            VE_DEBUG_ASSERT(state.uniform_stack.top() == this, "Only the topmost uniform storage object may be popped from the uniform stack.");
            state.uniform_stack.pop();

            for (const auto& [name, uniform] : uniforms) {
                state.bound_uniforms[name].value = nullptr;
            }

            if (!state.uniform_stack.empty()) {
                state.uniform_stack.top()->restore(state);
            }

            is_bound = false;
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
        mutable bool is_bound = false;
    };
}