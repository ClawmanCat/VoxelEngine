#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/renderpass_transforms.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/target/target_sampler.hpp>


namespace ve::gfx::opengl {
    // Performs a repeated rendering operation. Each pipeline takes the output of the previous stage as input,
    // and the first stage takes its input from the final stage (Except on the first iteration). This process is repeated 'count' times.
    // 'name_transform' can be used to map outputs from one stage to uniform samplers in the next one.
    // After the last render operation completes, uniform values are restored to the values they had before this method was invoked.
    // The uniform 'uint ping_pong_iteration' is provided for shaders to retrieve the current iteration.
    inline void ping_pong_render(
        std::vector<shared<pipeline>> pipelines,
        const pipeline_draw_data& data,
        std::size_t count = 2,
        std::function<std::string(std::string_view)> name_transform = name_transforms::identity { }
    ) {
        VE_DEBUG_ASSERT(pipelines.size() >= 2, "At least 2 pipelines are required to perform a ping-pong render operation.");

        struct old_uniform_state { std::size_t index; shared<uniform_sampler> value; };
        std::vector<old_uniform_state> modified_state;


        // Perform rendering iterations.
        for (std::size_t iteration = 0; iteration < count; ++iteration) {
            if (iteration == 0) {
                // First pass: set all pipelines except the first one to take their inputs from the previous stage.
                for (auto [i, pipeline] : pipelines | views::enumerate | views::drop(1)) {
                    for (const auto& [name, texture] : pipelines[i - 1]->get_target()->get_attachments()) {
                        std::string new_name = name_transform(name);

                        if (pipeline->has_uniform(new_name)) modified_state.push_back(old_uniform_state {
                            .index = i,
                            .value = pipeline->take_sampler_uniform(new_name)
                        });

                        pipeline->set_uniform_producer(make_shared<active_target_attachment>(pipelines[i - 1], name, std::move(new_name)));
                    }
                }
            }

            if (iteration == 1) {
                // 2nd pass: set first pipeline to take its input from the last pipeline.
                for (const auto& [name, texture] : pipelines.back()->get_target()->get_attachments()) {
                    std::string new_name = name_transform(name);

                    if (pipelines.front()->has_uniform(new_name)) modified_state.push_back(old_uniform_state {
                        .index = 0,
                        .value = pipelines.front()->take_sampler_uniform(new_name)
                    });

                    pipelines.front()->set_uniform_producer(make_shared<active_target_attachment>(pipelines.back(), name, std::move(new_name)));
                }
            }

            for (auto& pipeline : pipelines) {
                pipeline->template set_uniform_value<u32>("ping_pong_iteration", (u32) iteration);
                pipeline->draw(data);
            }
        }


        // Restore old state.
        for (auto& [index, value] : modified_state) {
            pipelines[index]->set_uniform_producer(std::move(value));
        }
    }
}