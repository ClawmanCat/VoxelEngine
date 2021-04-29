#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/shader_compiler.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/spirv_to_gl_shader.hpp>

#include <GL/glew.h>


namespace ve::graphics {
    class shader_program {
    public:
        explicit shader_program(const shader_compile_result& shader_binaries)
            : id(detail::spirv_to_gl_shader(shader_binaries))
        {}
        
        void bind(void) { glUseProgram(id); }
        
        [[nodiscard]] GLuint get_id(void) const noexcept { return id; }
    private:
        GLuint id;
    };
}