#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/platform_include.hpp>
#include VE_GRAPHICS_INCLUDE(buffer/buffer.hpp)
#include VE_GRAPHICS_INCLUDE(context.hpp)


namespace ve::graphics {
    class multi_buffer : public buffer, public tree_set<shared<buffer>> {
    public:
        virtual void draw(context& ctx) const {
            buffer::draw(ctx);
            
            for (auto& buf : *this) {
                buf->draw(ctx);
            }
        }
    
        
        [[nodiscard]] virtual GLuint get_id(void) const {
            return size() > 0 ? (*begin())->get_id() : 0;
        }
    };
}