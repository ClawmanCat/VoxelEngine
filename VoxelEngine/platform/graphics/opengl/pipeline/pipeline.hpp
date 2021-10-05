#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_storage.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/settings.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/target.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/category.hpp>


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

    private:
        pipeline_settings settings;
        shared<class shader> shader;

    public:
        VE_GET_SET_CREF(settings);
        VE_GET_SET_CREF(shader);
    };
}