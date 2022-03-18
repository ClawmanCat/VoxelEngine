#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/uniform/uniform_sampler.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/target.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>


namespace ve::gfx::opengl {
    // Uniform sampler for sampling from the target of a framebuffer.
    struct active_target_attachment : public uniform_sampler {
        shared<pipeline> source;
        // Corresponds to the name of the attachment in the target.
        std::string input;
        // Corresponds to the name of the uniform to be filled by said attachment.
        std::string output;

        active_target_attachment(shared<pipeline> source, std::string input, std::string output) :
            source(std::move(source)), input(std::move(input)), output(std::move(output))
        {}

        texture_list get_uniform_textures(void) const override {
            return { source->get_target()->get_attachments().at(input).texture };
        }

        std::string get_uniform_name(void) const override {
            return output;
        }
    };
}