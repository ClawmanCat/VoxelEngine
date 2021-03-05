#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/context.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_storage.hpp>

#include <GL/glew.h>


namespace ve::graphics {
    // Base class for other buffer types.
    class buffer : public uniform_storage {
    public:
        virtual ~buffer(void) = default;
        
        virtual void draw(context& ctx) const {
            bind_uniforms(ctx);
        }
        
        [[nodiscard]] virtual GLuint get_id(void) const = 0;
    };
}