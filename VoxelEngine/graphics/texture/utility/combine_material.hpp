#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/texture/utility/combine.hpp>


namespace ve::gfx {
    // Combines multiple PBR-textures into a single texture in the format expected by engine shaders.
    template <typename Pixel> inline image<Pixel> make_material_texture(
        const image<Pixel>& roughness,
        const image<Pixel>& metalness,
        const image<Pixel>& ambient_occlusion,
        const image<Pixel>& emissivity,
        vec2ui size = vec2ui { 0 }
    ) {
        // If no size was provided, use the largest component texture size.
        if (size == vec2ui { 0 }) {
            size.x = std::max({ roughness.size.x, metalness.size.x, ambient_occlusion.size.x, emissivity.size.x });
            size.y = std::max({ roughness.size.y, metalness.size.y, ambient_occlusion.size.y, emissivity.size.y });
        }


        return combine_images<Pixel>({
            combine_image_data<Pixel> { .src = &roughness,         .source_channels = { 1, 1, 1, 0 }, .dest_channels = { 1, 0, 0, 0 } },
            combine_image_data<Pixel> { .src = &metalness,         .source_channels = { 1, 1, 1, 0 }, .dest_channels = { 0, 1, 0, 0 } },
            combine_image_data<Pixel> { .src = &ambient_occlusion, .source_channels = { 1, 1, 1, 0 }, .dest_channels = { 0, 0, 1, 0 } },
            combine_image_data<Pixel> { .src = &emissivity,        .source_channels = { 1, 1, 1, 0 }, .dest_channels = { 0, 0, 0, 1 } }
        }, size);
    }
}