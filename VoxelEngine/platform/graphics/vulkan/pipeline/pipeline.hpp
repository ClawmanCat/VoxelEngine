#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/pipeline/pipeline_settings.hpp>
#include <VoxelEngine/platform/graphics/vulkan/pipeline/pipeline_attachment.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/render_target.hpp>
#include <VoxelEngine/platform/graphics/vulkan/vertex/vertex_buffer.hpp>
#include <VoxelEngine/platform/graphics/vulkan/shader/shader.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    class pipeline {
    public:
        pipeline(const shared<shader>& shader_program, const shared<render_target>& target, const pipeline_settings& settings = {})
            : target(target), shader_program(shader_program), settings(settings)
        {}

        virtual ~pipeline(void) = default;

        ve_immovable(pipeline);


        virtual void bind(void) = 0;
        virtual void unbind(void) = 0;
        virtual void draw(const vertex_buffer& vbo) = 0;


        VE_GET_VAL(target);
        VE_GET_VAL(shader_program);
        VE_GET_CREF(handle);
        VE_GET_CREF(settings);

    protected:
        vk_resource<VkPipeline> handle;
        shared<render_target> target;
        shared<shader> shader_program;
        pipeline_settings settings;
    };
}