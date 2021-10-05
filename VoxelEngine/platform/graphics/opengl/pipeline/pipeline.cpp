#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/vertex/vertex_buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/render_context.hpp>


namespace ve::gfx::opengl {
    void single_pass_pipeline::draw(const std::vector<const vertex_buffer*>& buffers, render_context& ctx) {
        ctx.pipelines.push(this);
        ctx.renderpass = this;

        shader->bind(ctx);
        uniform_storage::push(ctx.uniform_state);

        for (const auto& buffer : buffers) {
            buffer->draw(ctx);
        }

        uniform_storage::pop(ctx.uniform_state);
        ctx.renderpass = nullptr;
        ctx.pipelines.pop();
    }
}