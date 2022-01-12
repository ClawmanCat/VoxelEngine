#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/format.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/get.hpp>

#include <gl/glew.h>
#include <magic_enum.hpp>


namespace ve::gfx::opengl {
    class framebuffer;


    struct framebuffer_attachment {
        enum type_t { COLOR_BUFFER = GL_COLOR_ATTACHMENT0, DEPTH_BUFFER = GL_DEPTH_ATTACHMENT };


        // Note: attachment names are only used to fetch the attachment from the framebuffer
        // and need not correspond to the output value names of the shader rendering to them.
        std::string name;
        type_t type;
        const texture_format* format;
        texture_type tex_type;


        explicit framebuffer_attachment(std::string name, type_t type = COLOR_BUFFER, const texture_format* fmt = nullptr, texture_type tex_type = texture_type::TEXTURE_2D) :
            name(std::move(name)), type(type), tex_type(tex_type)
        {
            if (!fmt) {
                fmt = (type == COLOR_BUFFER)
                    ? &get_context()->settings.color_buffer_format
                    : &get_context()->settings.depth_buffer_format;
            }

            format = fmt;
        }

    private:
        friend class framebuffer;
        u8 attachment_index = 0;
    };


    class framebuffer {
    public:
        framebuffer(void) = default;


        framebuffer(
            std::vector<framebuffer_attachment> attachments,
            std::function<vec2ui(void)> texture_validator
        ) :
            attachment_templates(std::move(attachments)),
            texture_validator(std::move(texture_validator)),
            prev_size(this->texture_validator())
        {
            glGenFramebuffers(1, &id);
            glBindFramebuffer(GL_FRAMEBUFFER, id);


            std::size_t num_color_attachments = 0;
            bool has_depth_attachment = false;

            for (auto& attachment : attachment_templates) {
                if (attachment.type == framebuffer_attachment::COLOR_BUFFER) {
                    const static u32 color_attachment_limit = (u32) gl_get<i32>(GL_MAX_COLOR_ATTACHMENTS);

                    VE_ASSERT(
                        num_color_attachments < color_attachment_limit,
                        "Framebuffer may have at most", color_attachment_limit, "color attachments."
                    );

                    attachment.attachment_index = num_color_attachments;
                    ++num_color_attachments;
                } else {
                    VE_ASSERT(
                        !std::exchange(has_depth_attachment, true),
                        "Framebuffer may have at most one depth attachment."
                    );
                }


                auto [it, success] = this->attachments.emplace(
                    attachment.name,
                    texture::create(*attachment.format, prev_size, 1, texture_filter::NEAREST, attachment.tex_type)
                );

                if (attachment.type == framebuffer_attachment::DEPTH_BUFFER) {
                    it->second->set_parameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                }


                glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    (GLenum) attachment.type + attachment.attachment_index,
                    GL_TEXTURE_2D,
                    it->second->get_id(),
                    0
                );
            }
        }


        virtual ~framebuffer(void) {
            if (id) glDeleteFramebuffers(1, &id);
        }


        ve_rt_swap_move_only(framebuffer, id, attachments, texture_validator, prev_size);


        virtual void bind(void) {
            VE_DEBUG_ASSERT(id, "Cannot bind uninitialized framebuffer.");

            if (std::exchange(prev_size, texture_validator()) != prev_size) {
                rebuild_attachments();
            }

            glBindFramebuffer(GL_FRAMEBUFFER, id);


            bool has_color_attachment = ranges::contains(
                attachment_templates | views::transform(&framebuffer_attachment::type),
                framebuffer_attachment::COLOR_BUFFER
            );

            if (has_color_attachment) {
                std::vector<GLenum> draw_buffers;
                for (const auto& attachment : attachment_templates) {
                    if (attachment.type == framebuffer_attachment::COLOR_BUFFER) {
                        draw_buffers.push_back((GLenum) attachment.type + attachment.attachment_index);
                    }
                }

                glDrawBuffers((GLsizei) draw_buffers.size(), draw_buffers.data());
            } else {
                glDrawBuffer(GL_NONE);
            }
        }


        VE_GET_VAL(id);
        VE_GET_CREF(attachment_templates);
        VE_GET_CREF(attachments);
        VE_GET_CREF(texture_validator);
    private:
        GLuint id = 0;

        std::vector<framebuffer_attachment> attachment_templates;
        vec_map<std::string, shared<texture>> attachments;

        // The texture validator returns the size each texture should have before the next draw call.
        std::function<vec2ui(void)> texture_validator;
        vec2ui prev_size;


        void rebuild_attachments(void) {
            glBindFramebuffer(GL_FRAMEBUFFER, id);

            for (auto& [name, old_attachment] : attachments) {
                const auto& attachment_template = *ranges::find(attachment_templates, name, &framebuffer_attachment::name);

                auto new_attachment = texture::create(
                    *attachment_template.format,
                    prev_size,
                    1,
                    texture_filter::NEAREST,
                    attachment_template.tex_type
                );

                if (attachment_template.type == framebuffer_attachment::DEPTH_BUFFER) {
                    new_attachment->set_parameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                }

                glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    (GLenum) attachment_template.type + attachment_template.attachment_index,
                    GL_TEXTURE_2D,
                    new_attachment->get_id(),
                    0
                );

                std::swap(old_attachment, new_attachment);
            }

            if (false) VE_DEBUG_ASSERT(
                glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
                "Failed to rebuild attachments for framebuffer: code", glCheckFramebufferStatus(GL_FRAMEBUFFER)
            );
        }
    };
}