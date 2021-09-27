#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/simple_event_dispatcher.hpp>
#include <VoxelEngine/event/subscribe_only_view.hpp>
#include <VoxelEngine/platform/graphics/vulkan/pipeline/pipeline_attachment.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/render_context.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    class image;


    struct target_invalidated_event {};


    class render_target : public subscribe_only_view<simple_event_dispatcher<false>> {
    public:
        virtual ~render_target(void) = default;

        virtual std::vector<pipeline_attachment> get_attachments(void) const = 0;
        virtual std::size_t get_swap_count(void) const = 0;
        virtual std::size_t get_current_swap_index(void) const = 0;
        virtual std::vector<image*> get_images_for_index(std::size_t index) = 0;

        virtual render_context bind(void) = 0;
    };
}