#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/render/graphics_pipeline.hpp>

#include <GL/glew.h>


namespace ve {
    class render_target {
    public:
        render_target(void) = default;
        explicit render_target(shared<graphics_pipeline>&& provider) : provider(std::move(provider)) {}
        
        render_target(const render_target&) = delete;
        render_target& operator=(const render_target&) = delete;
        
        render_target(render_target&&) = default;
        render_target& operator=(render_target&&) = default;
        
        virtual ~render_target(void) = default;
        
        
        virtual void draw(void);
        
        void set_content_provider(shared<graphics_pipeline>&& provider) {
            this->provider = std::move(provider);
        }
        
    protected:
        shared<graphics_pipeline> provider;
    
        static inline GLuint current_fbo = 0;
        GLuint fbo = 0;
        
    private:
        void bind(void);
    };
}