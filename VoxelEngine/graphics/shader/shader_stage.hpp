#pragma once

#include <VoxelEngine/core/core.hpp>

#include <shaderc/shaderc.hpp>
#include <magic_enum.hpp>


namespace ve::graphics {
    enum class shader_stage {
        VERTEX, FRAGMENT, GEOMETRY, TESS_CTRL, TESS_EVAL, COMPUTE
    };
    
    constexpr inline std::size_t num_shader_stages = magic_enum::enum_count<shader_stage>();
    
    
    struct shader_stage_data {
        shader_stage stage;
        shaderc_shader_kind shaderc_kind;
        std::string file_ext;
        enum pipeline_mode { REQUIRED, ALLOWED, FORBIDDEN } pipeline_mode;
    };
    
    
    const inline std::array<shader_stage_data, num_shader_stages> shader_stage_info {
        shader_stage_data { shader_stage::VERTEX,    shaderc_vertex_shader,          ".vert.glsl",  shader_stage_data::REQUIRED  },
        shader_stage_data { shader_stage::FRAGMENT,  shaderc_fragment_shader,        ".frag.glsl",  shader_stage_data::REQUIRED  },
        shader_stage_data { shader_stage::GEOMETRY,  shaderc_geometry_shader,        ".geo.glsl",   shader_stage_data::ALLOWED   },
        shader_stage_data { shader_stage::TESS_CTRL, shaderc_tess_control_shader,    ".tctrl.glsl", shader_stage_data::ALLOWED   },
        shader_stage_data { shader_stage::TESS_EVAL, shaderc_tess_evaluation_shader, ".teval.glsl", shader_stage_data::ALLOWED   },
        shader_stage_data { shader_stage::COMPUTE,   shaderc_compute_shader,         ".comp.glsl",  shader_stage_data::FORBIDDEN }
    };
}