#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_storage.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/settings.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/target.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/category.hpp>
#include <VoxelEngine/utility/assert.hpp>


namespace ve::gfx::opengl {
    class vertex_buffer;
    struct render_context;


    class pipeline : public uniform_storage {
    public:
        explicit pipeline(const pipeline_category_t* type, shared<render_target> target) :
            type(type), target(std::move(target))
        {}

        virtual ~pipeline(void) = default;
        virtual void draw(const std::vector<const vertex_buffer*>& buffers, render_context& ctx) = 0;

    private:
        const pipeline_category_t* type;
        shared<render_target> target;

    public:
        VE_GET_VAL(type);
        VE_GET_SET_CREF(target);
    };


    class single_pass_pipeline : public pipeline {
    public:
        single_pass_pipeline(shared<render_target> target, shared<class shader> shader, pipeline_settings settings = {})
            : pipeline(shader->get_category(), std::move(target)), settings(std::move(settings)), shader(std::move(shader))
        {}

        void draw(const std::vector<const vertex_buffer*>& buffers, render_context& ctx) override;

        VE_GET_MREF(settings);
        VE_GET_CREF(shader);
    private:
        pipeline_settings settings;
        shared<class shader> shader;

        void bind_settings(void);
    };


    class multi_pass_pipeline : public pipeline {
    public:
        struct renderpass {
            shared<pipeline> pipeline;
            shared<render_target> target;
        };


        explicit multi_pass_pipeline(std::vector<renderpass> stages, const pipeline_category_t* category = &pipeline_category::RASTERIZATION)
            : pipeline(category, stages.back().target), stages(std::move(stages))
        {
            VE_ASSERT(
                ranges::all_of(stages, [&](const auto& s) { return s.pipeline->get_type() == category; }),
                "All stages within a multipass rendering pipeline must be of the same category."
            );
        }


        explicit multi_pass_pipeline(shared<render_target> target, const pipeline_category_t* category = &pipeline_category::RASTERIZATION)
            : pipeline(category, std::move(target))
        {}

        void draw(const std::vector<const vertex_buffer*>& buffers, render_context& ctx) override;

        void add_stage(shared<pipeline> stage, std::size_t index);
        void remove_stage(shared<pipeline> stage);
        void remove_stage(std::size_t index);

        VE_GET_CREF(stages);
    private:
        std::vector<renderpass> stages;
    };
}