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


    // A shader object represents a variable in the shader.
    // This is used for attributes, but also for e.g. the contents of a UBO.
    struct shader_object {
        std::string name;
        primitive_t type;

        // Note: fields below this comment are only set if the reflected object is a UBO or SSBO.
        // TODO: it would be nice to have this for all structs, perhaps calculate manually since all uniforms are STD140 anyway?
        std::size_t struct_size = 0;
        std::vector<shader_object> members;
        std::size_t offset_in_parent = 0;
    };


    // An attribute is a shader object that has some external binding,
    // e.g. it is a shader input, output or a uniform.
    struct attribute : shader_object {
        std::size_t location;
        std::size_t binding;


        attribute(const shader_object& obj, std::size_t location, std::size_t binding) :
            shader_object(obj), location(location), binding(binding)
        {}


        auto operator<=>(const attribute& other) const {
            if (auto cmp = (name <=> other.name); cmp != 0) return cmp;
            return compare_spirtypes(type, other.type);
        }
    };


    struct stage {
        std::vector<attribute> inputs, outputs;
        std::vector<attribute> uniform_buffers;
        std::vector<attribute> storage_buffers;
        std::vector<attribute> push_constants;
        std::vector<attribute> samplers;
    };


    struct shader_reflection {
        vec_map<const gfxapi::shader_stage*, stage> stages;

        std::string name;
        const gfxapi::pipeline_category_t* pipeline;


        const auto& get_input_stage(void) const {
            return *ranges::find_if(stages, [](const auto& kv) { return kv.first->first; });
        }

        const auto& get_output_stage(void) const {
            return *ranges::find_if(stages, [](const auto& kv) { return kv.first->last; });
        }
    };


    extern stage generate_stage_reflection(const gfxapi::shader_stage* stage, const spirv_blob& spirv);
    extern shader_reflection generate_reflection(std::string name, const vec_map<const gfxapi::shader_stage*, spirv_blob>& stages);
}