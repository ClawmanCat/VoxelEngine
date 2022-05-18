#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/compiler/cache.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/target/target_sampler.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline_builder.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/utility.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/quad.hpp>
#include <VoxelEngine/utility/raii.hpp>
#include <VoxelEngine/utility/algorithm.hpp>
#include <VoxelEngine/utility/then.hpp>
#include <VoxelEngine/utility/io/paths.hpp>


namespace ve::gfx::opengl {
    class pbr_pipeline : public pipeline, public std::enable_shared_from_this<pbr_pipeline> {
    public:
        using pbr_pipeline_tag = void;
        using pipeline::draw;


        ve_shared_only_then(pbr_pipeline, setup_pipeline, shared<render_target> target) :
            pipeline(&pipeline_category::RASTERIZATION, std::move(target))
        {}


        void draw(const pipeline_draw_data& data, overridable_function_tag) override;
    private:
        void setup_pipeline(void) {
            setup_renderpasses();
        }

        void setup_renderpasses(void);


        shared<single_pass_pipeline> geometry_pass;
        shared<single_pass_pipeline> lighting_pass;
        shared<single_pass_pipeline> postprocessing_pass;
        std::vector<single_pass_pipeline*> pipeline_stages;
    };
}