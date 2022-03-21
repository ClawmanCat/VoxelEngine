#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/format.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/framebuffer_attachment.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/get.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/reset_texture_bindings.hpp>

#include <gl/glew.h>
#include <magic_enum.hpp>


namespace ve::gfx::opengl {
    struct framebuffer_attachment {
        framebuffer_attachment_template attachment_template;
        shared<texture> texture;
        u32 attachment_index;

        bool is_color_attachment(void) const { return attachment_template.attachment_type == framebuffer_attachment_template::COLOR_BUFFER; }
        bool is_depth_attachment(void) const { return attachment_template.attachment_type == framebuffer_attachment_template::DEPTH_BUFFER; }
    };


    class framebuffer {
    public:
        enum clear_options_t : GLuint { CLEAR_COLOR_BUFFER = GL_COLOR_BUFFER_BIT, CLEAR_DEPTH_BUFFER = GL_DEPTH_BUFFER_BIT };


        framebuffer(void) = default;


        framebuffer(
            const std::vector<framebuffer_attachment_template>& templates,
            std::function<vec2ui(void)> texture_validator
        ) :
            texture_validator(std::move(texture_validator)),
            prev_size(this->texture_validator())
        {
            glGenFramebuffers(1, &id);
            VE_ASSERT(id, "Failed to create framebuffer.");

            glBindFramebuffer(GL_FRAMEBUFFER, id);


            u32 num_color_attachments = 0;
            bool has_depth_attachment = false;

            for (const auto& tmpl : templates) {
                if (tmpl.attachment_type == framebuffer_attachment_template::COLOR_BUFFER) {
                    const static u32 max_color_attachments = (u32) gl_get<i32>(GL_MAX_COLOR_ATTACHMENTS);

                    VE_ASSERT(
                        num_color_attachments < max_color_attachments,
                        "Framebuffer may have at most", max_color_attachments, "color attachments."
                    );

                    attachments.emplace(
                        tmpl.name,
                        framebuffer_attachment { .attachment_template = tmpl, .texture = nullptr, .attachment_index = num_color_attachments }
                    );

                    ++num_color_attachments;
                } else {
                    VE_ASSERT(
                        !std::exchange(has_depth_attachment, true),
                        "Framebuffer may have at most one depth attachment."
                    );

                    attachments.emplace(
                        tmpl.name,
                        framebuffer_attachment { .attachment_template = tmpl, .texture = nullptr, .attachment_index = 0 }
                    );
                }
            }


            build_attachments();
        }


        virtual ~framebuffer(void) {
            if (id) glDeleteFramebuffers(1, &id);
        }


        ve_rt_swap_move_only(framebuffer, id, attachments, texture_validator, prev_size);


        virtual void bind(void) {
            VE_DEBUG_ASSERT(id, "Cannot bind uninitialized framebuffer.");

            if (std::exchange(prev_size, texture_validator()) != prev_size) {
                build_attachments();
            }

            glBindFramebuffer(GL_FRAMEBUFFER, id);
            glViewport(0, 0, (GLsizei) prev_size.x, (GLsizei) prev_size.y);
        }


        virtual void clear(GLuint mask = CLEAR_COLOR_BUFFER | CLEAR_DEPTH_BUFFER) {
            bind();

            for (auto& [name, attachment] : attachments) {
                if (
                    (attachment.is_color_attachment() && (mask & CLEAR_COLOR_BUFFER)) ||
                    (attachment.is_depth_attachment() && (mask & CLEAR_DEPTH_BUFFER))
                ) {
                    if (attachment.attachment_template.should_clear()) attachment.texture->clear_simple(attachment.attachment_template.clear_value);
                }
            }
        }


        VE_GET_VAL(id);
        VE_GET_CREF(attachments);
        VE_GET_CREF(texture_validator);
    private:
        GLuint id = 0;

        hash_map<std::string, framebuffer_attachment> attachments;

        // The texture validator returns the size each texture should have before the next draw call.
        std::function<vec2ui(void)> texture_validator;
        vec2ui prev_size;


        void bind_attachments(void) {
            glBindFramebuffer(GL_FRAMEBUFFER, id);


            std::vector<GLenum> draw_buffers;
            for (const auto& attachment : attachments | views::values | views::filter(&framebuffer_attachment::is_color_attachment)) {
                draw_buffers.push_back(((GLenum) attachment.attachment_template.attachment_type) + attachment.attachment_index);
            }


            if (draw_buffers.empty()) {
                glDrawBuffer(GL_NONE);
            } else {
                // TODO: Re-order to match on name rather than index.
                ranges::sort(draw_buffers);
                glDrawBuffers((GLsizei) draw_buffers.size(), draw_buffers.data());
            }
        }


        void build_attachments(void) {
            glBindFramebuffer(GL_FRAMEBUFFER, id);


            for (auto& [name, attachment] : attachments) {
                const auto& tmpl = attachment.attachment_template;

                auto tex = texture::create(
                    prev_size,
                    *tmpl.tex_format,
                    1,
                    tmpl.tex_filter,
                    tmpl.tex_wrap
                );

                if (tmpl.attachment_type == framebuffer_attachment_template::DEPTH_BUFFER) {
                    tex->set_parameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                }

                glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    (GLenum) tmpl.attachment_type + attachment.attachment_index,
                    GL_TEXTURE_2D,
                    tex->get_id(),
                    0
                );

                std::swap(attachment.texture, tex);
            }


            VE_DEBUG_ASSERT(
                glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
                "Failed to rebuild attachments for framebuffer: code", glCheckFramebufferStatus(GL_FRAMEBUFFER)
            );


            // Reset the currently bound textures to the default texture, since if the attachment texture is not compatible
            // with whatever sampler is used in the shader (e.g. its a depth texture), it will raise an UB warning,
            // even if we rebind it before actually using the sampler.
            bind_attachments();
            reset_texture_bindings((GLint) attachments.size());
        }
    };
}