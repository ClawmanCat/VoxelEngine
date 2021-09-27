#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/compiler.hpp>


namespace ve::gfx::opengl {
    class shader {
    public:
        VE_GET_VAL(id);
        VE_GET_CREF(reflection);
    private:
        GLuint id;
        reflect::shader_reflection reflection;
    };


    template <typename Vertex>
    inline shader make_shader(const shader_compilation_data& data) {
        VE_NOT_YET_IMPLEMENTED;
    }
}