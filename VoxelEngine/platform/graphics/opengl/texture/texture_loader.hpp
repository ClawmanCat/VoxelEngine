#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/io.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture_utils.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>

#include <GL/glew.h>


namespace ve::graphics {
    inline expected<texture> load_texture(const fs::path& path, ve_default_actor(owner), const texture_parameters& params = {}) {
        auto img = io::read_png(path);
        if (!img) return make_unexpected(img.error());
        
        auto texture = detail::initialize_texture(
            img->size,
            texture_format::RGBA8,
            params
        );
        
        detail::store_texture_data(texture, *img);
        
        return texture;
    }
}