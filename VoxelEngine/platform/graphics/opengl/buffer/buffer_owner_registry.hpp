#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/dependent/actor.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>

#include <GL/glew.h>

#include <vector>


namespace ve::graphics {
    class buffer_owner_registry : public resource_owner<buffer_owner_registry> {
    public:
        static buffer_owner_registry& instance(void);
    
    
        void on_actor_destroyed(actor_id id) {
            auto it = buffer_info.find(id);
            if (it == buffer_info.end()) return;
            
            for (const auto& info : it->second) {
                auto buffer = info.self.lock();
                if (!buffer) continue;
                
                for (const auto& usage : info.used_by) {
                    auto pipeline = usage.pipeline.lock();
                    if (!pipeline) continue;
                    
                    pipeline->remove_buffer<false>(usage.shader.lock(), buffer);
                }
                
                buffer_owners.erase(info.id);
            }
            
            buffer_info.erase(it);
        }
        
        
        void on_buffer_added_to_pipeline(
            const shared<buffer>& buffer,
            const shared<shader_program>& shader,
            const shared<pipeline>& pipeline,
            actor_id owner
        ) {
            auto [it, inserted] = buffer_info[owner].insert(dummy_info(buffer));
            
            // Only bother setting up the object if it was actually inserted.
            auto usage = buffer_owner_info::pipeline_usage { pipeline, shader };
            
            if (inserted) {
                it->used_by = { std::move(usage) };
                it->self    = buffer;
                it->owner   = owner;
                
                buffer_owners[it->id] = &(*it);
            } else {
                // TODO: Remove this limitation. Maybe create shallow buffer copies and track usage externally?
                
                const bool owners_same = owner == it->owner; // Can't be in assert due to structured binding shenanigans.
                VE_ASSERT(
                    owners_same,
                    "All references to the same buffer must have the same owner. "
                    "Clone the buffer if you wish to use a resource from another actor."
                );
                
                it->used_by.push_back(std::move(usage));
            }
        }
        
        
        void on_buffer_removed_from_pipeline(
            const shared<buffer>& vertex_buffer,
            const shared<shader_program>& shader,
            const shared<pipeline>& pipeline
        ) {
            auto owner = buffer_owners.find(vertex_buffer->get_id());
            if (owner == buffer_owners.end()) return;
            
            auto& info = *(owner->second);
            auto usage_it = std::find_if(
                info.used_by.begin(),
                info.used_by.end(),
                [&](const auto& usage) {
                    return usage.pipeline.lock() == pipeline && usage.shader.lock() == shader;
                }
            );
            
            if (usage_it != info.used_by.end()) {
                info.used_by.erase(usage_it);
                if (info.used_by.empty()) buffer_info[info.id].erase(info);
            }
        }
    private:
        buffer_owner_registry(void) = default;
    
    
        struct buffer_owner_info {
            struct pipeline_usage { weak<pipeline> pipeline; weak<shader_program> shader; };
            
            // Most set implementations will return const references, since modifying the object could change the hash.
            // However, only the id determines the hash, so we can safely change everything else.
            mutable std::vector<pipeline_usage> used_by;
            mutable weak<buffer> self;
            mutable actor_id owner;
            GLuint id;
            
            ve_hashable(id);
            ve_eq_comparable_fields(buffer_owner_info, id);
        };
        
        static buffer_owner_info dummy_info(const shared<buffer>& buf) {
            return buffer_owner_info {
                .id = buf->get_id()
            };
        }
        
        
        // Assumes few pipelines and many buffers per pipeline.
        hash_map<actor_id, stable_hash_set<buffer_owner_info>> buffer_info;
        hash_map<GLuint, const buffer_owner_info*> buffer_owners;
    };
}