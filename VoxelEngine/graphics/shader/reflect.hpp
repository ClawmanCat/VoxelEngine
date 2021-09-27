#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/spirtype.hpp>
#include <VoxelEngine/utility/assert.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(shader/stage.hpp)

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_reflect.hpp>

#include <compare>


namespace ve::gfx::reflect {
    using spirv_blob = std::vector<u32>;


    struct attribute {
        std::string name;
        primitive_t type;
        std::size_t location;
        std::size_t binding;

        auto operator<=>(const attribute& other) const {
            if (auto cmp = (name <=> other.name); cmp != 0) return cmp;
            return compare_spirtypes(type, other.type);
        }
    };


    struct stage {
        std::vector<attribute> inputs, outputs;
        std::vector<attribute> uniform_buffers;
        std::vector<attribute> push_constants;
    };


    struct shader_reflection {
        vec_map<const gfxapi::shader_stage*, stage> stages;

        std::string name;
        const gfxapi::pipeline_category* pipeline;


        const auto& get_input_stage(void) const {
            return *ranges::find_if(stages, [](const auto& kv) { return kv.first->first; });
        }

        const auto& get_output_stage(void) const {
            return *ranges::find_if(stages, [](const auto& kv) { return kv.first->last; });
        }
    };


    extern shader_reflection generate_reflection(std::string name, const vec_map<const gfxapi::shader_stage*, spirv_blob>& stages);
}