#pragma once

#include <VoxelEngine/core/core.hpp>

#include <GL/glew.h>


namespace ve {
    // Cannot be constructed directly. Get one from the shader_library instead.
    class shader_program {
    public:
        void bind(void)   { glUseProgram(id); }
        void unbind(void) { glUseProgram(0);  }
        
        
        [[nodiscard]] GLuint get_id(void) const noexcept { return id; }
    private:
        friend class shader_compiler;
        GLuint id;
        
        shader_program(GLuint id) : id(id) {}
    };
}