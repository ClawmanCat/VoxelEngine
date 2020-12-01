#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/container_utils.hpp>
#include <VoxelEngine/utils/functional.hpp>
#include <VoxelEngine/utils/meta/glm_comparison.hpp>
#include <VoxelEngine/utils/io/io.hpp>
#include <VoxelEngine/graphics/render/texture/texture.hpp>


namespace ve {
    class texture_atlas {
    public:
        texture_atlas(const vec2ui& size = { 2048, 2048 }) : size(size) {}
        
        ~texture_atlas(void) {
            if (tex.id) glDeleteTextures(1, &tex.id);
        }
        
        
        texture_atlas(const texture_atlas&) = delete;
        texture_atlas& operator=(const texture_atlas&) = delete;
        
        
        texture_atlas(texture_atlas&& o) {
            *this = std::move(o);
        }
        
        texture_atlas& operator=(texture_atlas&& o) {
            std::swap(tex, o.tex);
            size   = o.size;
            mipmap = o.mipmap;
            
            return *this;
        }
        
        texture get_texture(void) const { return tex; }
    protected:
        vec2ui size = { 0, 0 };
        texture tex = { 0 };
        bool mipmap = false;
        
        
        constexpr vec2f coords_to_uv(const vec2ui& position) {
            return ((vec2f) position) / ((vec2f) size);
        }
        
        
        using texture_parameter = std::pair<GLenum, GLenum>;
        
        void reserve_texture_storage(const std::vector<texture_parameter>& params, u32 mipmap_level = 4) {
            VE_ASSERT(tex.id == 0);
            
            glGenTextures(1, &tex.id);
            VE_ASSERT(tex.id);
            glBindTexture(GL_TEXTURE_2D, tex.id);
            
            
            mipmap = requires_mipmaps(params);
            glTexStorage2D(GL_TEXTURE_2D, mipmap ? mipmap_level : 1, GL_RGBA8, size.x, size.y);
            
            for (const auto& [param, value] : params) {
                glTexParameteri(GL_TEXTURE_2D, param, value);
            }
            
            if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);
        }
        
        
        void store_subtexture(const io::image& img, const vec2ui& where) {
            VE_ASSERT(tex.id);
            VE_ASSERT(glm::all(img.size + where < size));
            
            glBindTexture(GL_TEXTURE_2D, tex.id);
            glTexSubImage2D(GL_TEXTURE_2D, 0, where.x, where.y, img.size.x, img.size.y, GL_RGBA, GL_UNSIGNED_BYTE, &img.pixels[0]);
            
            if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);
        }
        
    private:
        static bool requires_mipmaps(const std::vector<texture_parameter>& params) {
            std::array mipmap_modes = {
                GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR,
                GL_LINEAR_MIPMAP_NEAREST,  GL_LINEAR_MIPMAP_LINEAR
            };
            
            return contains_if(
                ve_such_that(kv, kv.first == GL_TEXTURE_MIN_FILTER && contains(kv.second, mipmap_modes)),
                params
            );
        }
    };
}