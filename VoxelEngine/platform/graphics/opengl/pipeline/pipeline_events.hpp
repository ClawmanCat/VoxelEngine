#pragma once

#include <VoxelEngine/core/core.hpp>


#define ve_impl_pipeline_event(name, ...)       \
struct name {                                   \
    const class pipeline* pipeline;             \
    const pipeline_draw_data* draw_data;        \
    __VA_ARGS__                                 \
}


namespace ve::gfx::opengl {
    class pipeline;
    struct renderpass_definition;
    struct pipeline_draw_data;


    // Common Pipeline Events:
    ve_impl_pipeline_event(pipeline_pre_draw_event);
    ve_impl_pipeline_event(pipeline_post_draw_event);


    // PBR Pipeline Events:
    ve_impl_pipeline_event(pbr_pre_geometry_pass_event);
    ve_impl_pipeline_event(pbr_post_geometry_pass_event);

    ve_impl_pipeline_event(pbr_pre_lighting_pass_event);
    ve_impl_pipeline_event(pbr_post_lighting_pass_event);

    ve_impl_pipeline_event(pbr_pre_postprocessing_pass_event);
    ve_impl_pipeline_event(pbr_post_postprocessing_pass_event);


    // Called before and after construction of the sub-pipelines within this pipeline respectively.
    struct pbr_pipeline_pre_build_stages_event {
        const class pipeline* pipeline;
        std::vector<renderpass_definition>* renderpass_defs;
    };

    struct pbr_pipeline_post_build_stages_event {
        const class pipeline* pipeline;
        std::vector<shared<single_pass_pipeline>>* renderpasses;
    };


    // Called before and after construction of the pipeline respectively.
    struct pbr_pipeline_pre_build_event  { const class pipeline* pipeline; };
    struct pbr_pipeline_post_build_event { const class pipeline* pipeline; };
}