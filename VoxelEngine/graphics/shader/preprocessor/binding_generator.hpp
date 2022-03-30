#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/preprocessor/shader_preprocessor.hpp>


namespace ve::gfx {
    // Automatically generates specialization constant, UBO & SSBO bindings that match across multiple shader stages.
    // Usage:
    // UBO  MyUBO  { ... } => layout (std140, binding = N) uniform MyUBO  { ... }
    // SSBO MySSBO { ... } => layout (std430, binding = N) buffer  MySSBO { ... }
    // CONSTANT uint MySC = ...; => layout (constant_id = N) const uint MySC = ...;
    class autobinding_preprocessor : public shader_preprocessor {
    public:
        struct autobinding_data {
            hash_map<std::string, u32> ubo_bindings, ssbo_bindings, constant_bindings;
            std::size_t uniform_count = 0, constant_count = 0;
        };

        using shader_preprocessor::shader_preprocessor;
        void operator()(std::string& src, arbitrary_storage& context) const override;
    private:
        static void eat_whitespace(std::string_view& sv, std::size_t& counter);
        static std::string_view eat_word(std::string_view& sv, std::size_t& counter);
    };
}