#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/framebuffer.hpp>
#include <VoxelEngine/platform/graphics/vulkan/utility/utility.hpp>


namespace ve::gfx::vulkan {
    // TODO: Handle pipelines with multiple subpasses.
    // Probably the easiest way to achieve this would be to define custom shader types for multipass pipelines,
    // e.g. my_shader.1.vert.glsl, etc. then mark those as a custom pipeline category.
    // This would require matching shader extensions by regex rather than by value.
    class rasterization_pipeline : public pipeline {
    public:
        rasterization_pipeline(const shared<shader>& shader_program, const shared<render_target>& target, const pipeline_settings& settings = {})
            : pipeline(shader_program, target, settings)
        {
            target->add_handler([this] (const target_invalidated_event& e) { target_changed = true; });

            recreate_pipeline(shader_program, target, settings);
            recreate_framebuffers();
        }


        void bind(void) override {
            if (!pipeline_settings::dynamic_compatibility_equality{}(settings, creation_settings) || target_changed) {
                recreate_pipeline(shader_program, target, settings);
                recreate_framebuffers();

                target_changed = false;
            }


            current_context = target->bind();

            const auto& fb = framebuffers[target->get_current_swap_index()];
            vec2ui size = fb.get_attachments()[0].image->get_size();


            VkClearValue clear_color { .color = {{
                    settings.clear_color.r,
                    settings.clear_color.g,
                    settings.clear_color.b,
                    settings.clear_color.a
            }}};

            VkClearValue clear_depth { .depthStencil {
                .depth   = 1.0f,
                .stencil = 0
            }};

            std::vector<VkClearValue> clear_values;
            for (const auto& attachment : fb.get_attachments()) {
                switch (attachment.attachment.get_usage()) {
                    case pipeline_attachment::INPUT:
                        // Value doesn't matter, input attachments aren't cleared.
                        [[fallthrough]];
                    case pipeline_attachment::COLOR_BUFFER:
                        [[fallthrough]];
                    case pipeline_attachment::PRESENTABLE:
                        clear_values.push_back(clear_color);
                        break;
                    case pipeline_attachment::DEPTH_BUFFER:
                        clear_values.push_back(clear_depth);
                        break;
                    default:
                        VE_ASSERT(false, "Unhandled attachment type.");
                }
            }


            VkRenderPassBeginInfo info {
                .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass      = renderpass.value,
                .framebuffer     = fb.get_handle(),
                .renderArea      = { .offset = { 0, 0 }, .extent = { size.x, size.y } },
                .clearValueCount = (u32) clear_values.size(),
                .pClearValues    = clear_values.data()
            };

            current_context->cmd_buffer->record(vkCmdBeginRenderPass, &info, VK_SUBPASS_CONTENTS_INLINE);
            current_context->cmd_buffer->record(vkCmdBindPipeline, VK_PIPELINE_BIND_POINT_GRAPHICS, *handle);


            // Set dynamic state.
            VkInstance instance = get_context()->instance;

            current_context->cmd_buffer->record(VE_VKPTR(instance, vkCmdSetPrimitiveTopologyEXT), (VkPrimitiveTopology) settings.topology);
            current_context->cmd_buffer->record(VE_VKPTR(instance, vkCmdSetCullModeEXT),          (VkCullModeFlags) settings.cull_mode);
            current_context->cmd_buffer->record(VE_VKPTR(instance, vkCmdSetFrontFaceEXT),         (VkFrontFace) settings.cull_direction);
            current_context->cmd_buffer->record(VE_VKPTR(instance, vkCmdSetLineWidth),            settings.line_width);
            current_context->cmd_buffer->record(VE_VKPTR(instance, vkCmdSetDepthTestEnableEXT),   (VkBool32) settings.enable_depth);
            current_context->cmd_buffer->record(VE_VKPTR(instance, vkCmdSetDepthWriteEnableEXT),  (VkBool32) settings.enable_depth);
        }


        void unbind(void) override {
            current_context->cmd_buffer->record(vkCmdEndRenderPass);
            current_context = std::nullopt;
        }


        void draw(const vertex_buffer& vbo) override {
            VE_DEBUG_ASSERT(current_context, "Attempt to render with unbound pipeline.");
            vbo.draw(*current_context);
        }

    private:
        std::vector<framebuffer> framebuffers;
        vk_resource<VkRenderPass> renderpass;
        std::optional<render_context> current_context = std::nullopt;

        pipeline_settings creation_settings;
        bool target_changed = false;


        void recreate_pipeline(const shared<shader>& shader_program, const shared<render_target>& target, const pipeline_settings& settings);
        void recreate_framebuffers(void);
    };
}