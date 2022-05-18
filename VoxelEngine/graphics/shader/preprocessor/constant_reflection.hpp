#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/preprocessor/shader_preprocessor.hpp>
#include <VoxelEngine/graphics/shader/reflect.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(shader/stage.hpp)

#include <ctre.hpp>


namespace ve::gfx {
    // Perform reflection on any specialization constants in the shader, since SPIRV-Reflect currently doesn't support it.
    class constant_reflection_preprocessor : public shader_preprocessor {
    public:
        using attribute_map = hash_map<const gfxapi::shader_stage*, std::vector<reflect::attribute>>;
        using shader_preprocessor::shader_preprocessor;

        void operator()(std::string& src, arbitrary_storage& context) const override;

        std::size_t hash(void) const override { return type_hash<constant_reflection_preprocessor>(); /* No internal state. */ }
    private:
        static reflect::attribute generate_attribute(std::string_view binding, std::string_view type_string, std::string_view name);
        static reflect::object_type::base_type_t get_base_type(std::string_view type);
    };
}