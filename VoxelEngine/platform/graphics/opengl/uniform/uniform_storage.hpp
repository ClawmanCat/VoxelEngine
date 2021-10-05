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
                auto& storage = state.bound_uniforms.at(name);

                auto& uniform_unbound = uniform; // Can't use structured binding in lambda.
                VE_DEBUG_ASSERT(
                    reflect::compare_spirtypes(uniform_unbound->stored_type, *storage.type) == std::strong_ordering::equal,
                    "Attempt to bind uniform", uniform_unbound->name, "with incorrect type."
                );

                uniform->combine(storage.value);
                storage.value = uniform->get();
                update_std140_value(storage, uniform.get());
            }

            state.uniform_stack.push(this);
            is_bound = true;
        }


        void restore(uniform_bind_state& state) const {
            VE_DEBUG_ASSERT(state.uniform_stack.top() == this, "Only the topmost uniform storage object in the uniform stack may be restored.");

            for (const auto& [name, uniform] : uniforms) {
                auto& storage = state.bound_uniforms.at(name);

                storage.value = uniform->get();
                update_std140_value(storage, uniform.get());
            }
        }


        void pop(uniform_bind_state& state) const {
            VE_DEBUG_ASSERT(state.uniform_stack.top() == this, "Only the topmost uniform storage object may be popped from the uniform stack.");
            state.uniform_stack.pop();

            for (const auto& [name, uniform] : uniforms) {
                auto& storage = state.bound_uniforms.at(name);

                storage.value = nullptr;
                storage.ubo_dirty = true;
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


        template <typename T, typename Combine = decltype(combine_functions::overwrite)>
        void set_uniform_value(std::string name, meta::dont_deduce<T> value, Combine combine = combine_functions::overwrite) {
            set_uniform(make_unique<uniform_value<T, Combine>>(std::move(name), std::move(value), fwd(combine)));
        }


        template <typename T, typename Produce, typename Combine = decltype(combine_functions::overwrite)>
        void set_uniform_producer(std::string name, Produce produce, Combine combine = combine_functions::overwrite) {
            set_uniform(make_unique<uniform_producer<T, Produce, Combine>>(std::move(name), fwd(produce), fwd(combine)));
        }
    private:
        hash_map<std::string_view, unique<uniform>> uniforms;
        mutable bool is_bound = false;


        static void update_std140_value(uniform_bind_state::combined_value& target, const struct uniform* uniform) {
            // Prevent unnecessary heap allocations by keeping some storage to push the std140 object into,
            // so we can memcmp it with the old value.
            static thread_local std::vector<u8> temporary_storage;
            // get_std140 clears the buffer beforehand, no manual clear necessary.
            uniform->get_std140(temporary_storage);


            // Only mark the UBO as dirty if the buffer actually changed.
            target.ubo_dirty =
                temporary_storage.size() == target.value_std140.size() &&
                memcmp(temporary_storage.data(), target.value_std140.data(), temporary_storage.size()) != 0;

            if (!target.ubo_dirty) return;


            // While just using operator= would probably be fine, the standard makes no guarantees about memory reuse in that case.
            target.value_std140.resize(temporary_storage.size());
            memcpy(target.value_std140.data(), temporary_storage.data(), temporary_storage.size());
        }
    };
}