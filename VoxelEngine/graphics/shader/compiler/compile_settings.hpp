#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/preprocessor/shader_preprocessor.hpp>
#include <VoxelEngine/utility/arbitrary_storage.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(pipeline/category.hpp)

#include <shaderc/shaderc.hpp>


namespace ve::gfx {
    struct shader_specialize_settings {
        using constant_types = meta::pack<bool, i32, u32, float>;


        template <typename T> requires constant_types::template contains<T>
        void set_constant(std::string name, T value) {
            constants[std::move(name)] = value;
        }


        std::size_t hash(void) const {
            std::size_t result = 0;

            for (const auto& [k, v] : constants) {
                hash_combine(result, k);
                std::visit([&] (const auto& v) { hash_combine(result, v); }, v);
            }

            return result;
        }


        hash_map<std::string, typename constant_types::template expand_inside<std::variant>> constants;
    };


    struct shader_compile_settings {
        const shaderc::CompileOptions* compiler_options;
        arbitrary_storage preprocessor_context;
        std::vector<shared<shader_preprocessor>> additional_preprocessors;
        std::size_t preprocessor_recursion_limit = 32;
        shader_specialize_settings specialization_settings;

        // If this value is nullptr, the pipeline category is deduced automatically from the shader stages.
        const gfxapi::pipeline_category_t* pipeline_type = nullptr;


        std::size_t hash(void) const {
            std::size_t result = 0;

            // shaderc doesn't expose the values of any settings, so the best we can do is compare pointers.
            hash_combine(result, compiler_options);

            hash_combine(result, preprocessor_context);
            for (const auto& pp : additional_preprocessors) hash_combine(result, pp.get());
            hash_combine(result, preprocessor_recursion_limit);

            hash_combine(result, specialization_settings);
            hash_combine(result, pipeline_type);

            return result;
        }
    };
}