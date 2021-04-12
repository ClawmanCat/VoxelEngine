#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/platform_include.hpp>
#include VE_GRAPHICS_INCLUDE(buffer/buffer.hpp)
#include VE_GRAPHICS_INCLUDE(context.hpp)


namespace ve::graphics {
    class multi_buffer : public buffer {
    public:
        virtual void draw(context& ctx) const {
            buffer::draw(ctx);
            
            for (auto& buf : buffers) {
                buf->draw(ctx);
            }
        }
    
        
        [[nodiscard]] virtual GLuint get_id(void) const {
            return buffers.size() > 0 ? (*buffers.begin())->get_id() : 0;
        }
        
        
        void insert(shared<buffer>&& buf) {
            buffers.insert(std::move(buf));
        }
        
        
        void erase(const shared<buffer>& buf) {
            buffers.erase(buf);
        }
    private:
        tree_set<shared<buffer>> buffers;
    };
}