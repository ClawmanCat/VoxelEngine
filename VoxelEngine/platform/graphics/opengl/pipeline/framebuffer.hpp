#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/format.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/get.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    enum class framebuffer_attachment : GLenum {
        COLOR_BUFFER = GL_COLOR_ATTACHMENT0,
        DEPTH_BUFFER = GL_DEPTH_ATTACHMENT
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

            for (const auto& attachment : attachment_templates) {
                GLenum attachment_type    = (GLenum) attachment;
                const texture_format* fmt = nullptr;

                if (attachment == framebuffer_attachment::COLOR_BUFFER) {
                    const static u32 color_attachment_limit = (u32) gl_get<i32>(GL_MAX_COLOR_ATTACHMENTS);

                    VE_ASSERT(
                        num_color_attachments < color_attachment_limit,
                        "Framebuffer may have at most", color_attachment_limit, "color attachments."
                    );

                    attachment_type += num_color_attachments;
                    fmt = &(get_context()->settings.color_buffer_format);

                    ++num_color_attachments;
                } else {
                    VE_ASSERT(
                        !std::exchange(has_depth_attachment, true),
                        "Framebuffer may have at most one depth attachment."
                    );

                    fmt = &(get_context()->settings.depth_buffer_format);
                }


                auto [it, success] = this->attachments.emplace(
                    attachment_type,
                    texture::create(*fmt, prev_size, 1)
                );


                glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_type, GL_TEXTURE_2D, it->second->get_id(), 0);
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
        }


        VE_GET_VAL(id);
        VE_GET_CREF(attachment_templates);
        VE_GET_CREF(attachments);
        VE_GET_CREF(texture_validator);
    private:
        GLuint id = 0;

        std::vector<framebuffer_attachment> attachment_templates;
        vec_map<GLenum, shared<texture>> attachments;

        // The texture validator returns the size each texture should have before the next draw call.
        std::function<vec2ui(void)> texture_validator;
        vec2ui prev_size;


        void rebuild_attachments(void) {
            glBindFramebuffer(GL_FRAMEBUFFER, id);

            for (auto& [type, old_attachment] : attachments) {
                auto new_attachment = texture::create(
                    old_attachment->get_format(),
                    prev_size,
                    old_attachment->get_mipmap_levels()
                );

                glFramebufferTexture2D(GL_FRAMEBUFFER, type, GL_TEXTURE_2D, new_attachment->get_id(), 0);
                std::swap(old_attachment, new_attachment);
            }
        }
    };
}