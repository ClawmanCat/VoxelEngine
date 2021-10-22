#include <VoxelEngine/platform/graphics/opengl/shader/stage.hpp>


namespace ve::gfx::opengl {
    const std::vector<shader_stage> shader_stages = {
        shader_stage {
            .name           = "vertex shader",
            .file_extension = ".vert.glsl",
            .shaderc_type   = shaderc_vertex_shader,
            .opengl_type    = GL_VERTEX_SHADER,
            .pipeline       = &pipeline_category::RASTERIZATION,
            .pipeline_mode  = shader_stage::pipeline_mode_t::VARIANT,
            .first          = true
        },

        // Alternative identity for vertex shader when used to pass through GBuffer to fragment shader.
        shader_stage {
            .name           = "G buffer input shader",
            .file_extension = ".g_input.glsl",
            .shaderc_type   = shaderc_vertex_shader,
            .opengl_type    = GL_VERTEX_SHADER,
            .pipeline       = &pipeline_category::RASTERIZATION,
            .pipeline_mode  = shader_stage::pipeline_mode_t::VARIANT,
            .first          = true
        },

        shader_stage {
            .name           = "fragment shader",
            .file_extension = ".frag.glsl",
            .shaderc_type   = shaderc_fragment_shader,
            .opengl_type    = GL_FRAGMENT_SHADER,
            .pipeline       = &pipeline_category::RASTERIZATION,
            .pipeline_mode  = shader_stage::pipeline_mode_t::REQUIRED,
            .last           = true
        },

        shader_stage {
            .name           = "geometry shader",
            .file_extension = ".geo.glsl",
            .shaderc_type   = shaderc_geometry_shader,
            .opengl_type    = GL_GEOMETRY_SHADER,
            .pipeline       = &pipeline_category::RASTERIZATION,
            .pipeline_mode  = shader_stage::pipeline_mode_t::OPTIONAL
        },

        shader_stage {
            .name           = "tesselation control shader",
            .file_extension = ".tctrl.glsl",
            .shaderc_type   = shaderc_tess_control_shader,
            .opengl_type    = GL_TESS_CONTROL_SHADER,
            .pipeline       = &pipeline_category::RASTERIZATION,
            .pipeline_mode  = shader_stage::pipeline_mode_t::OPTIONAL
        },

        shader_stage {
            .name           = "tesselation evaluation shader",
            .file_extension = ".teval.glsl",
            .shaderc_type   = shaderc_tess_evaluation_shader,
            .opengl_type    = GL_TESS_EVALUATION_SHADER,
            .pipeline       = &pipeline_category::RASTERIZATION,
            .pipeline_mode  = shader_stage::pipeline_mode_t::OPTIONAL
        },

        shader_stage {
            .name           = "compute shader",
            .file_extension = ".comp.glsl",
            .shaderc_type   = shaderc_compute_shader,
            .opengl_type    = GL_COMPUTE_SHADER,
            .pipeline       = &pipeline_category::COMPUTE,
            .pipeline_mode  = shader_stage::pipeline_mode_t::REQUIRED,
            .first          = true,
            .last           = true
        }
    };
}