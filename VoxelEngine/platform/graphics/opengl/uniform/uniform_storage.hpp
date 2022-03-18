#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/uniform/uniform_sampler.hpp>
#include <VoxelEngine/graphics/uniform/uniform_combine_function.hpp>
#include <VoxelEngine/graphics/uniform/uniform_convertible.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_manager.hpp>
#include <VoxelEngine/utility/traits/dont_deduce.hpp>


namespace ve::gfx::opengl {
    class shader;


    class uniform_storage {
    public:
        void bind_uniforms_for_shader(const shader* shader, uniform_state_dict& state) const;
        void bind_samplers_for_shader(const shader* shader, sampler_state_dict& state) const;

        void set_uniform(unique<uniform>&& uniform);
        void remove_uniform(std::string_view name);
        bool has_uniform(std::string_view name) const;

        unique<uniform> take_normal_uniform(std::string_view name);
        shared<uniform_sampler> take_sampler_uniform(std::string_view name);


        template <typename T, typename Combine = decltype(combine_functions::overwrite)>
        void set_uniform_value(std::string name, meta::dont_deduce<T> value, Combine combine = combine_functions::overwrite) {
            set_uniform(make_unique<uniform_value<T, Combine>>(std::move(name), std::move(value), fwd(combine)));
        }

        template <typename T, typename Produce, typename Combine = decltype(combine_functions::overwrite)>
        void set_uniform_producer(std::string name, Produce produce, Combine combine = combine_functions::overwrite) {
            set_uniform(make_unique<uniform_producer<T, Produce, Combine>>(std::move(name), fwd(produce), fwd(combine)));
        }


        // Overload for classes that derive from uniform_convertible.
        template <typename T> requires requires { typename T::uniform_convertible_tag; }
        void set_uniform_value(const T& src) {
            set_uniform_value<typename T::uniform_t>(src.get_uniform_name(), src.get_uniform_value(), src.get_uniform_combine_function());
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


        // Overload for samplers.
        void set_uniform_producer(shared<uniform_sampler> src) {
            samplers[src->get_uniform_name()] = std::move(src);
        }
    private:
        hash_map<std::string, unique<uniform>> uniforms;
        hash_map<std::string, shared<uniform_sampler>> samplers;
    };
}