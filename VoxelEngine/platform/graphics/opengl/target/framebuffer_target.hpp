#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/target/target.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture.hpp>


namespace ve::graphics {
    struct framebuffer_attachment {
        enum class buffer_type { COLOR, DEPTH, STENCIL };
        
        texture tex;
        buffer_type type = buffer_type::COLOR;
        const char* name = "color";
    };
    
    
    class framebuffer_target : public target {
    public:
        framebuffer_target(shared<pipeline> source) : target(std::move(source)) {
        
        }
        
        ~framebuffer_target(void) {
            if (fbo) glDeleteFramebuffers(1, &fbo);
        }
        
        framebuffer_target(const framebuffer_target&) = delete;
        framebuffer_target& operator=(const framebuffer_target&) = delete;
        
        framebuffer_target(framebuffer_target&& o) : target(std::move(o)) {
            std::swap(fbo, o.fbo);
        }
        
        framebuffer_target& operator=(framebuffer_target&& o) {
            target::operator=(std::move(o));
            std::swap(fbo, o.fbo);
            return *this;
        }
        
        
        virtual void draw(void) override {
        
        }
        
    private:
        GLuint fbo = 0;
    };
}