#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/arbitrary_storage.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_manager.hpp>


namespace ve::gfx::opengl {
    class pipeline;
    class single_pass_pipeline;


    struct render_context : public arbitrary_storage {
        render_context(void) = default;
        ve_rt_move_only(render_context);


        uniform_manager uniform_state;

        // In the case of multipass rendering, each renderpass will be pushed on top of the overarching pipeline.
        std::stack<const pipeline*> pipelines;
        const single_pass_pipeline* renderpass;
    };
}