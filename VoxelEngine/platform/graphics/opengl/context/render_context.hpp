#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_bind_state.hpp>


namespace ve::gfx::opengl {
    class pipeline;
    class single_pass_pipeline;


    struct render_context {
        uniform_bind_state uniform_state;

        // In the case of multipass rendering, each renderpass will be pushed on top of the overarching pipeline.
        std::stack<const pipeline*> pipelines;
        const single_pass_pipeline* renderpass;
    };
}