#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/graphics/shader/object_type.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(shader/stage.hpp)

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_reflect.hpp>

#include <compare>


namespace ve::gfx::reflect {
    using SPIRV = std::vector<u32>;


    // Represents a variable in the shader.
    // This is used for attributes, but also for e.g. the contents of a UBO.
    struct shader_object {
        std::string name;
        object_type type;

        // Note: fields below this comment are only set if the reflected object is a UBO or SSBO.
        std::vector<shader_object> members;
        std::size_t struct_size = 0;
        std::size_t offset_in_parent = 0;
    };


    // An attribute is a shader object that has some external binding,
    // e.g. it is a shader input, output or a uniform.
    struct attribute : shader_object {
        std::size_t location;
        std::size_t binding;

        attribute(void) = default;

        attribute(shader_object&& obj, std::size_t location, std::size_t binding) :
            shader_object(std::move(obj)), location(location), binding(binding)
        {}
    };


    struct stage {
        std::vector<attribute> inputs, outputs;
        std::vector<attribute> uniform_buffers;
        std::vector<attribute> storage_buffers;
        std::vector<attribute> push_constants;
        std::vector<attribute> specialization_constants;
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


        friend std::ostream& operator<<(std::ostream& stream, const shader_reflection& reflection);
    };


    extern stage generate_stage_reflection(const gfxapi::shader_stage* stage, const SPIRV& spirv);
    extern shader_reflection generate_reflection(std::string name, const vec_map<const gfxapi::shader_stage*, SPIRV>& stages);
}