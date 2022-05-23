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
    class pbr_pipeline : public multipass_pipeline {
    public:
        using pbr_pipeline_tag = void;
        using multipass_pipeline::draw;


        // Note: we delay calling setup pipeline until after the constructor so any mixins have time to add event handlers.
        ve_derived_shared_only_then(pbr_pipeline, multipass_pipeline, setup_pipeline, shared<render_target> target, std::string name) :
            multipass_pipeline(&pipeline_category::RASTERIZATION, std::move(target), std::move(name))
        {}

        void draw(const pipeline_draw_data& data, overridable_function_tag) override;

        virtual std::vector<shared<pipeline>>& get_stages(void) override { return pipeline_stages; }
        virtual const std::vector<shared<pipeline>>& get_stages(void) const override { return pipeline_stages; }
    private:
        void setup_pipeline(void);

        shared<single_pass_pipeline> geometry_pass;
        shared<single_pass_pipeline> lighting_pass;
        shared<single_pass_pipeline> postprocessing_pass;
        std::vector<shared<pipeline>> pipeline_stages;

    public:
        VE_GET_SET_VAL(geometry_pass);
        VE_GET_SET_VAL(lighting_pass);
        VE_GET_SET_VAL(postprocessing_pass);
    };


    template <template <typename Pipeline> typename... Mixins>
    class pbr_pipeline_with_mixins : public pbr_pipeline, public pipeline_mixin_base<pbr_pipeline_with_mixins, Mixins...> {
    public:
        ve_derived_shared_only(
            pbr_pipeline_with_mixins,
            // Note: order is important: we want to initialize the mixins before the pipeline is constructed.
            (pipeline_mixin_base, pbr_pipeline),
            shared<render_target> target,
            std::string name
        ) :
            pbr_pipeline(std::move(target), std::move(name))
        {}
    };
}