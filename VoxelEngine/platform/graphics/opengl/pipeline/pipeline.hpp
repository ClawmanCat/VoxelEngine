#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/utility.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_storage.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader_program.hpp>
#include <VoxelEngine/ecs/system/system.hpp>


namespace ve::graphics {
    class pipeline : public uniform_storage {
    public:
        pipeline(void) = default;
        virtual ~pipeline(void) = default;
        
        ve_move_only(pipeline);
        
        
        virtual void draw(void) = 0;
        
        virtual void add_buffer(shared<shader_program>&& shader, shared<buffer>&& buffer) = 0;
        virtual void clear(void) = 0;
    };
    
    
    class simple_pipeline : public pipeline {
    public:
        void draw(void) override {
            for (auto& [shader, buffers_for_shader] : buffers) {
                shader->bind();
                
                context ctx {
                    .current_program   = shader->get_id(),
                    .next_texture_unit = 0
                };
                
                bind_uniforms(ctx);
                
                for (auto& buffer : buffers_for_shader) {
                    context last_ctx = ctx;
                    buffer->draw(ctx);
                    ctx = last_ctx;
                }
            }
        }
        
        
        void add_buffer(shared<shader_program>&& shader, shared<buffer>&& buffer) override {
            buffers[std::move(shader)].push_back(std::move(buffer));
        }
        
        
        void clear(void) override {
            // Keep existing allocations to reduce new allocations on next iteration.
            for (auto& [shader, shader_buffers] : buffers) shader_buffers.clear();
        }
    protected:
        hash_map<
            shared<shader_program>,
            std::vector<shared<buffer>>
        > buffers;
    };
}