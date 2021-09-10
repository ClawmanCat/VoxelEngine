#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/event/event_handler_id.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/memory.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/buffer.hpp>
#include <VoxelEngine/platform/graphics/vulkan/texture/image_formats.hpp>


namespace ve::gfx::vulkan {
    using image_view = vk_resource<VkImageView>;


    class image {
    public:
        constexpr static inline VkImageUsageFlags usage_transferable = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        
        struct arguments {
            const vec2ui& size;
            VkImageUsageFlags usage;
            VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL;
            VkMemoryPropertyFlags properties = device_memory::gpu_memory;
            bool shared = false;

            VkFormat pixel_format;
            // Note: if the pixel format is not in image_formats.hpp, format information should be provided manually.
            const image_format_info* format_info = nullptr;

            u32 mipmap_levels = 1;
            VkFilter mipmap_filter = VK_FILTER_LINEAR;
        };
        

        image(void) = default;
        ve_swap_move_only(image, handle, memory, size, pixel_format, fmt_info, usage, layout, mipmap_levels, mipmap_filter);


        explicit image(const arguments& args) :
            size(args.size),
            pixel_format(args.pixel_format),
            fmt_info(args.format_info ? *(args.format_info) : image_formats.at(args.pixel_format)),
            usage(args.usage),
            layout(args.layout),
            mipmap_levels(args.mipmap_levels),
            mipmap_filter(args.mipmap_filter)
        {
            VkImageCreateInfo info {
                .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .flags         = 0,
                .imageType     = VK_IMAGE_TYPE_2D,
                .format        = args.pixel_format,
                .extent        = VkExtent3D { .width = args.size.x, .height = args.size.y, .depth = 1 },
                .mipLevels     = args.mipmap_levels,
                .arrayLayers   = 1,
                .samples       = VK_SAMPLE_COUNT_1_BIT,
                .tiling        = VK_IMAGE_TILING_OPTIMAL,
                .usage         = args.usage,
                .sharingMode   = args.shared ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
                // Can't initialize with the desired layout directly, so transition after image creation.
                // (https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#resources-image-layouts)
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
            };


            if (vkCreateImage(get_context()->logical_device, &info, nullptr, &handle) != VK_SUCCESS) {
                VE_ASSERT(false, "Failed to create Vulkan image.");
            }


            VkMemoryRequirements requirements;
            vkGetImageMemoryRequirements(get_context()->logical_device, handle, &requirements);

            memory = device_memory(requirements, args.properties);
            vkBindImageMemory(get_context()->logical_device, handle, memory.get_handle(), 0);


            transition_layout(args.layout);
        }


        virtual ~image(void) {
            if (handle != VK_NULL_HANDLE) vkDestroyImage(get_context()->logical_device, handle, nullptr);
        }


        ve::image read(const rect2ui& where) const {
            const auto region_size = where.size();

            buffer staging_buffer {
                region_size.x * region_size.y * fmt_info.stride,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                device_memory::cpu_memory
            };


            VkImageSubresourceLayers subresource  {
                .aspectMask     = fmt_info.aspects,
                .mipLevel       = 0,
                .baseArrayLayer = 0,
                .layerCount     = 1
            };

            VkBufferImageCopy region {
                .bufferOffset      = 0,
                .bufferRowLength   = 0,
                .bufferImageHeight = 0,
                .imageSubresource  = subresource,
                .imageOffset       = VkOffset3D { .x = (i32) where.top_left.x, .y = (i32) where.top_left.y, .z = 0 },
                .imageExtent       = VkExtent3D { .width = region_size.x, .height = region_size.y, .depth = 1 }
            };


            {
                command_buffer cb { queue_family::GRAPHICS_QUEUE };
                cb.record(vkCmdCopyImageToBuffer, handle, layout, staging_buffer.get_handle(), 1, &region);
            }


            return ve::image {
                .data           = staging_buffer.read(),
                .size           = region_size,
                .channels       = fmt_info.channels,
                .stride         = fmt_info.stride,
                .channel_depths = fmt_info.channel_depths
            };
        }


        void write(const ve::image& data, const vec2ui& where) {
            VE_ASSERT(
                data.channels       == fmt_info.channels &&
                data.stride         == fmt_info.stride   &&
                data.channel_depths == fmt_info.channel_depths,
                "Image data written to Vulkan image must match that image in format."
            );


            buffer staging_buffer {
                data.size.x * data.size.y * fmt_info.stride,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                device_memory::cpu_memory
            };

            staging_buffer.write(data.data);


            VkImageSubresourceLayers subresource  {
                .aspectMask     = fmt_info.aspects,
                .mipLevel       = 0,
                .baseArrayLayer = 0,
                .layerCount     = 1
            };

            VkBufferImageCopy region {
                .bufferOffset      = 0,
                .bufferRowLength   = 0,
                .bufferImageHeight = 0,
                .imageSubresource  = subresource,
                .imageOffset       = VkOffset3D { .x = (i32) where.x, .y = (i32) where.y, .z = 0 },
                .imageExtent       = VkExtent3D { .width = data.size.x, .height = data.size.y, .depth = 1 }
            };


            {
                command_buffer cb { queue_family::GRAPHICS_QUEUE };
                cb.record(vkCmdCopyBufferToImage, staging_buffer.get_handle(), handle, layout, 1, &region);
            }

            generate_mipmaps();
        }


        // Copies the region of the image to another image, performing rescaling if necessary using the provided filter.
        void copy_to(image& target, const rect2ui& src, const rect2ui& dst, VkFilter filter = VK_FILTER_LINEAR) const {
            blit_region(
                { *this,  src, 0 },
                { target, dst, 0 },
                filter
            );

            target.generate_mipmaps();
        }


        void transition_layout(
            VkImageLayout new_layout,
            // Stages that need to be synchronized around the layout transition. Defaults to all applicable.
            VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            // Operations in said stages that need to be synchronized around the layout transition. Defaults to all applicable.
            VkAccessFlags src_access = VK_ACCESS_MEMORY_WRITE_BIT,
            VkAccessFlags dst_access = VK_ACCESS_MEMORY_READ_BIT
        ) {
            if (new_layout == layout) return;


            VkImageSubresourceRange subresource {
                .aspectMask     = fmt_info.aspects,
                .baseMipLevel   = 0,
                .levelCount     = mipmap_levels,
                .baseArrayLayer = 0,
                .layerCount     = 1
            };

            VkImageMemoryBarrier barrier {
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask       = src_access,
                .dstAccessMask       = dst_access,
                .oldLayout           = layout,
                .newLayout           = new_layout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image               = handle,
                .subresourceRange    = subresource
            };


            {
                command_buffer cb { queue_family::GRAPHICS_QUEUE };
                cb.record(vkCmdPipelineBarrier, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            }


            layout = new_layout;
        }


        // Call this method instead of the above one if the image layout was transitioned externally,
        // e.g. through a render operation.
        void transition_layout_external(VkImageLayout new_layout) {
            layout = new_layout;
        }


        image_view create_view(void) {
            VkImageViewCreateInfo info {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image            = handle,
                .viewType         = VK_IMAGE_VIEW_TYPE_2D,
                .format           = pixel_format,
                .components       = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY
                },
                .subresourceRange = VkImageSubresourceRange {
                    .aspectMask     = fmt_info.aspects,
                    .baseMipLevel   = 0,
                    .levelCount     = mipmap_levels,
                    .baseArrayLayer = 0,
                    .layerCount     = 1
                },
            };


            VkImageView view;
            if (vkCreateImageView(get_context()->logical_device, &info, nullptr, &view) != VK_SUCCESS) {
                VE_ASSERT(false, "Failed to create Vulkan image view.");
            }


            return { std::move(view), bind<0, 2>(vkDestroyImageView, *(get_context()->logical_device), nullptr) };
        }


        void generate_mipmaps(void) {
            for (u32 i = 1; i < mipmap_levels; ++i) {
                blit_region(
                    { *this, get_extents(), 0 },
                    { *this, get_extents(), i },
                    mipmap_filter
                );
            }
        }


        void update_mipmap_settings(u32 levels, VkFilter filter) {
            mipmap_levels = levels;
            mipmap_filter = filter;
            generate_mipmaps();
        }


        rect2ui get_extents(void) const {
            return rect2ui { .top_left = { 0, 0 }, .bottom_right = size };
        }


        VE_GET_CREF(handle);
        VE_GET_CREF(pixel_format);
        VE_GET_CREF(fmt_info);
        VE_GET_CREF(usage);
        VE_GET_CREF(layout);
        VE_GET_CREF(memory);
        VE_GET_VAL(size);
        VE_GET_VAL(mipmap_levels);
        VE_GET_VAL(mipmap_filter);
    protected:
        VkImage handle = VK_NULL_HANDLE;
        device_memory memory;

        vec2ui size = { 0, 0 };

        VkFormat pixel_format;
        image_format_info fmt_info;

        VkImageUsageFlags usage;
        VkImageLayout layout;

        u32 mipmap_levels = 1;
        VkFilter mipmap_filter = VK_FILTER_LINEAR;


        struct image_region { image& image; rect2ui region; u32 mip_level; };
        struct const_image_region { const image& image; rect2ui region; u32 mip_level; };

        // Performs a copy of the given image region at the given mipmap level, performing conversion using the given filter if required.
        // src and dst may be the same image, as long as the mipmap levels are different.
        static void blit_region(const_image_region src, image_region dst, VkFilter filter) {
            auto to_offset = [](const vec2ui& v) { return VkOffset3D { .x = (i32) v.x, .y = (i32) v.y, .z = 0 }; };

            VkImageSubresourceLayers src_subresource {
                .aspectMask     = src.image.fmt_info.aspects,
                .mipLevel       = src.mip_level,
                .baseArrayLayer = 0,
                .layerCount     = 1
            };

            VkImageSubresourceLayers dst_subresource {
                .aspectMask     = dst.image.fmt_info.aspects,
                .mipLevel       = dst.mip_level,
                .baseArrayLayer = 0,
                .layerCount     = 1
            };

            VkImageBlit region {
                .srcSubresource = src_subresource,
                .srcOffsets     = { to_offset(src.region.top_left), to_offset(src.region.bottom_right) },
                .dstSubresource = dst_subresource,
                .dstOffsets     = { to_offset(dst.region.top_left), to_offset(dst.region.bottom_right) }
            };


            command_buffer cb { queue_family::GRAPHICS_QUEUE };
            cb.record(vkCmdBlitImage, src.image.handle, src.image.layout, dst.image.handle, dst.image.layout, 1, &region, filter);
        }
    };


    // Wrapper for swapchain images, which don't have an associated VkDeviceMemory.
    class non_owning_image : public image {
    public:
        non_owning_image(void) : image() {}


        non_owning_image(VkImage&& img, const image::arguments& args) : image() {
            handle        = std::move(img);
            size          = args.size;
            pixel_format  = args.pixel_format;
            fmt_info      = args.format_info ? *(args.format_info) : image_formats.at(args.pixel_format);
            usage         = args.usage;
            layout        = args.layout;
            mipmap_levels = args.mipmap_levels;
            mipmap_filter = args.mipmap_filter;
        }


        ~non_owning_image(void) {
            // Prevent image destruction.
            handle = VK_NULL_HANDLE;
        }

        ve_inherit_move(non_owning_image, image);
    };
}