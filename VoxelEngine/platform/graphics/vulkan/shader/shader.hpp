#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/compiler.hpp>
#include <VoxelEngine/platform/graphics/vulkan/shader/stage.hpp>
#include <VoxelEngine/platform/graphics/vulkan/shader/layout.hpp>
#include <VoxelEngine/platform/graphics/vulkan/pipeline/pipeline_category.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    struct shader_module_info {
        vk_resource<VkShaderModule> module;
        VkPipelineShaderStageCreateInfo pipeline_info;
    };


    class shader {
    public:
        shader(
            reflect::shader_reflection reflection,
            std::vector<shader_module_info> modules,
            const pipeline_category* pipeline,
            shader_layout layout
        ) :
            reflection(std::move(reflection)),
            modules(std::move(modules)),
            pipeline(pipeline),
            layout(std::move(layout))
        {}


        VE_GET_CREF(reflection);
        VE_GET_CREF(modules);
        VE_GET_CREF(layout);
        VE_GET_VAL(pipeline);
    private:
        reflect::shader_reflection reflection;
        std::vector<shader_module_info> modules;
        const pipeline_category* pipeline;
        shader_layout layout;
    };


    namespace detail {
        extern shader make_shader_impl(const shader_compilation_data& data, shader_layout&& layout);
    }

    template <typename Vertex> inline shader make_shader(const shader_compilation_data& data) {
        // Wrapped since template must be handled in header.
        return detail::make_shader_impl(
            data,
            shader_layout {
                .uniforms = get_uniform_layout(data.reflection),
                .vertex_attributes = get_vertex_layout<Vertex>(data.reflection)
            }
        );
    }
}