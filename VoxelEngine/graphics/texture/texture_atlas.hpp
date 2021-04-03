#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/graphics.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/utility/io/io.hpp>


namespace ve::graphics {
    // Represents a section of a texture.
    struct subtexture {
        shared<texture> tex;
        vec2f uv;       // Normalized
        vec2f size;     // Normalized
        u8 index = 0;
    };
    
    
    template <typename Derived> class texture_atlas : public resource_owner<Derived> {
    public:
        expected<subtexture> add_texture(const io::image& img, ve_default_actor(owner)) {
            VE_CRTP_CHECK(Derived, add_texture);
            return static_cast<Derived*>(this)->add_texture(img, owner);
        }
        
        
        void remove_texture(const subtexture& tex) {
            VE_CRTP_CHECK(Derived, remove_texture);
            static_cast<Derived*>(this)->remove_texture(tex);
        }
        
        
        static u32 quantization(void) {
            VE_CRTP_CHECK(Derived, quantization);
            return Derived::quantization();
        }
    
    
        static vec2ui size(void) {
            VE_CRTP_CHECK(Derived, size);
            return Derived::size();
        }
    };
}