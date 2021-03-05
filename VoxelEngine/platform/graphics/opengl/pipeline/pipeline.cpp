#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/buffer_owner_registry.hpp>


namespace ve::graphics {
    void pipeline::registry_add_buffer(const shared<buffer>& vertex_buffer, const shared<shader_program>& shader, actor_id owner) {
        buffer_owner_registry::instance().on_buffer_added_to_pipeline(
            vertex_buffer,
            shader,
            shared_from_this(),
            owner
        );
    }
    
    
    void pipeline::registry_remove_buffer(const shared<buffer>& vertex_buffer, const shared<shader_program>& shader) {
        buffer_owner_registry::instance().on_buffer_removed_from_pipeline(
            vertex_buffer,
            shader,
            shared_from_this()
        );
    }
}