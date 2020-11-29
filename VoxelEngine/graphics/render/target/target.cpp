#include <VoxelEngine/graphics/render/target/target.hpp>


namespace ve {
    void render_target::draw(void) {
        bind();
        provider->draw();
    }
    
    void render_target::bind(void) {
        if (current_fbo != fbo) glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        current_fbo = fbo;
    }
}