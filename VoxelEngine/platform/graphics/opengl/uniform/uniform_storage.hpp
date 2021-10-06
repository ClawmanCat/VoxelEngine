#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/dont_deduce.hpp>
#include <VoxelEngine/graphics/uniform/uniform_combine_function.hpp>
#include <VoxelEngine/graphics/uniform/uniform_convertible.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_bind_state.hpp>


namespace ve::gfx::opengl {
    class uniform_storage {
    public:
        void push(uniform_bind_state& state) const {
            for (const auto& [name, uniform] : uniforms) {
                auto& storage = get_storage_for(state, name);

                auto& uniform_unbound = uniform; // Can't use structured binding in lambda.
                std::size_t type_hash = storage.type->hash();

                VE_DEBUG_ASSERT(
                    uniform_unbound->stored_type_hash  == type_hash ||
                    uniform_unbound->wrapped_type_hash == type_hash,
                    "Attempt to bind uniform", uniform_unbound->name, "with incorrect type."
                );

                // Note that this is valid, even in the case of struct aliases, since both the GLSL layouts and the C++
                // layout always have the first struct member at offset 0.
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
                auto& storage = get_storage_for(state, name);

                storage.value = uniform->get();
                update_std140_value(storage, uniform.get());
            }
        }


        void pop(uniform_bind_state& state) const {
            VE_DEBUG_ASSERT(state.uniform_stack.top() == this, "Only the topmost uniform storage object may be popped from the uniform stack.");
            state.uniform_stack.pop();

            for (const auto& [name, uniform] : uniforms) {
                auto& storage = get_storage_for(state, name);

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

        template <typename T> requires requires { typename T::uniform_convertible_tag; }
        void set_uniform_value(const T& src) {
            set_uniform_value<typename T::uniform_t>(src.get_uniform_name(), src.get_uniform_value(), src.get_uniform_combine_function());
        }


        template <typename T, typename Produce, typename Combine = decltype(combine_functions::overwrite)>
        void set_uniform_producer(std::string name, Produce produce, Combine combine = combine_functions::overwrite) {
            set_uniform(make_unique<uniform_producer<T, Produce, Combine>>(std::move(name), fwd(produce), fwd(combine)));
        }

        // Note: pointer is stored and must remain valid until the uniform is removed or the storage is destroyed.
        template <typename T> requires requires { typename T::uniform_convertible_tag; }
        void set_uniform_producer(const T* src) {
            set_uniform_producer<typename T::uniform_t>(
                src->get_uniform_name(),
                [src] { return src->get_uniform_value(); },
                src->get_uniform_combine_function()
            );
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


        static uniform_bind_state::combined_value& get_storage_for(uniform_bind_state& state, std::string_view name) {
            // Get the UBO with the given name, check aliases if it doesn't exist.
            // Assume it exists in one of the two, setting an uniform that doesn't exist should be an error anyway.
            auto storage_it = state.bound_uniforms.find(name);

            return (storage_it == state.bound_uniforms.end())
                ? state.bound_uniforms.at(state.aliases.at(name))
                : storage_it->second;
        }
    };
}