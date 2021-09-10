#include <VoxelEngine/platform/graphics/vulkan/shader/stage.hpp>


namespace ve::gfx::vulkan {
    const std::vector<shader_stage> shader_stages = {
        shader_stage {
            .name           = "vertex shader",
            .file_extension = ".vert.glsl",
            .shaderc_type   = shaderc_vertex_shader,
            .vulkan_type    = VK_SHADER_STAGE_VERTEX_BIT,
            .pipeline       = &pipeline_categories::RASTERIZATION,
            .pipeline_mode  = shader_stage::pipeline_mode_t::REQUIRED,
            .first          = true
        },

        shader_stage {
            .name           = "fragment shader",
            .file_extension = ".frag.glsl",
            .shaderc_type   = shaderc_fragment_shader,
            .vulkan_type    = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pipeline       = &pipeline_categories::RASTERIZATION,
            .pipeline_mode  = shader_stage::pipeline_mode_t::REQUIRED,
            .last           = true
        },

        shader_stage {
            .name           = "geometry shader",
            .file_extension = ".geo.glsl",
            .shaderc_type   = shaderc_geometry_shader,
            .vulkan_type    = VK_SHADER_STAGE_GEOMETRY_BIT,
            .pipeline       = &pipeline_categories::RASTERIZATION,
            .pipeline_mode  = shader_stage::pipeline_mode_t::OPTIONAL
        },

        shader_stage {
            .name           = "tesselation control shader",
            .file_extension = ".tctrl.glsl",
            .shaderc_type   = shaderc_tess_control_shader,
            .vulkan_type    = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            .pipeline       = &pipeline_categories::RASTERIZATION,
            .pipeline_mode  = shader_stage::pipeline_mode_t::OPTIONAL
        },

        shader_stage {
            .name           = "tesselation evaluation shader",
            .file_extension = ".teval.glsl",
            .shaderc_type   = shaderc_tess_evaluation_shader,
            .vulkan_type    = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            .pipeline       = &pipeline_categories::RASTERIZATION,
            .pipeline_mode  = shader_stage::pipeline_mode_t::OPTIONAL
        },

        shader_stage {
            .name           = "compute shader",
            .file_extension = ".comp.glsl",
            .shaderc_type   = shaderc_compute_shader,
            .vulkan_type    = VK_SHADER_STAGE_COMPUTE_BIT,
            .pipeline       = &pipeline_categories::COMPUTE,
            .pipeline_mode  = shader_stage::pipeline_mode_t::REQUIRED,
            .first          = true,
            .last           = true
        }
    };
}