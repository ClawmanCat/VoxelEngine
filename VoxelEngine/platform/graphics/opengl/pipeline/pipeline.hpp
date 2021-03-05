#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/utility.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_storage.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader_program.hpp>

#include <memory>


namespace ve::graphics {
    // Note: ownership of contained buffers is managed through buffer_owner_registry.
    // Buffers will still be automatically removed from the pipeline if their owner is destroyed.
    class pipeline : public uniform_storage, public std::enable_shared_from_this<pipeline> {
    public:
        pipeline(void) = default;
        virtual ~pipeline(void) = default;
        
        ve_move_only(pipeline);
        
        
        virtual void draw(void) {
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
        
        
        void add_buffer(
            universal<shared<shader_program>> auto&& shader,
            universal<shared<buffer>> auto&& vertex_buffer,
            ve_default_actor(owner)
        ) {
            registry_add_buffer(vertex_buffer, shader, owner);
            
            buffers[std::forward<shared<shader_program>>(shader)]
                .insert(std::forward<shared<buffer>>(vertex_buffer));
        }
        
        
        void remove_buffer(const shared<shader_program>& shader, const shared<buffer>& buffer) {
            remove_buffer<true>(shader, buffer);
        }
    private:
        // Buffers for each shader.
        // TODO: Optimize storage for iteration.
        hash_map<shared<shader_program>, hash_set<shared<buffer>>> buffers;
        
        
        friend class buffer_owner_registry;
        
        template <bool remove_from_registry>
        void remove_buffer(const shared<shader_program>& shader, const shared<buffer>& buffer) {
            if constexpr (remove_from_registry) registry_remove_buffer(buffer, shader);
            buffers[shader].erase(buffer);
        }
        
        void registry_add_buffer(const shared<buffer>& vertex_buffer, const shared<shader_program>& shader, actor_id owner);
        void registry_remove_buffer(const shared<buffer>& vertex_buffer, const shared<shader_program>& shader);
    };
}