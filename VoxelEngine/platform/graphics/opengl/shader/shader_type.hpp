#pragma once

#include <VoxelEngine/core/core.hpp>

#include <GL/glew.h>

#include <array>
#include <tuple>
#include <string_view>


namespace ve::graphics {
    enum class shader_type {
        VERTEX, TESS_CONTROL, TESS_EVAL, GEOMETRY, FRAGMENT, COMPUTE, UTILITY
    };
    
    enum class shader_pipeline_type {
        REQUIRED, OPTIONAL, FORBIDDEN
    };
    
    
    struct shader_info {
        shader_type ve_type;
        GLenum gl_type;
        std::string_view file_ext;
        shader_pipeline_type pipeline_type;
        
        ve_eq_comparable(shader_info);
    };
    
    constexpr inline std::array shader_stages {
        shader_info { shader_type::VERTEX,       GL_VERTEX_SHADER,          ".vert.glsl",  shader_pipeline_type::REQUIRED  },
        shader_info { shader_type::TESS_CONTROL, GL_TESS_CONTROL_SHADER,    ".tctrl.glsl", shader_pipeline_type::OPTIONAL  },
        shader_info { shader_type::TESS_EVAL,    GL_TESS_EVALUATION_SHADER, ".teval.glsl", shader_pipeline_type::OPTIONAL  },
        shader_info { shader_type::GEOMETRY,     GL_GEOMETRY_SHADER,        ".geo.glsl",   shader_pipeline_type::OPTIONAL  },
        shader_info { shader_type::FRAGMENT,     GL_FRAGMENT_SHADER,        ".frag.glsl",  shader_pipeline_type::REQUIRED  },
        shader_info { shader_type::COMPUTE,      GL_COMPUTE_SHADER,         ".comp.glsl",  shader_pipeline_type::FORBIDDEN },
        shader_info { shader_type::UTILITY,      GL_INVALID_ENUM,           ".util.glsl",  shader_pipeline_type::FORBIDDEN }
    };
}