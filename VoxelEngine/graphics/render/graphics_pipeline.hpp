#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/render/buffer/vertex_array.hpp>
#include <VoxelEngine/graphics/render/shader/shader_program.hpp>
#include <VoxelEngine/graphics/render/uniform_storage.hpp>

#include <vector>


namespace ve {
    class graphics_pipeline : public uniform_storage {
    public:
        // Renders the pipeline to the currently bound target.
        void draw(void) {
            for (auto& [shader, vertex_arrays] : buffers) {
                shader->bind();
                GLuint id = shader->get_id();
    
                // Set global uniforms.
                u32 uniform_state = 0;
                bind_uniforms(id, uniform_state);
                
                // Set local uniforms & draw buffers.
                for (auto& array : vertex_arrays) {
                    u32 local_uniform_state = uniform_state;
                    array->draw(id, local_uniform_state);
                }
                
                shader->unbind();
            }
        }
        
        
        void add_buffer(shared<shader_program>&& shader, shared<vertex_array_base>&& buffer) {
            VE_ASSERT(shader != nullptr);
            auto it = buffers.find(shader);
            
            if (it != buffers.end()) {
                it->second.push_back(std::move(buffer));
            } else {
                std::vector<shared<vertex_array_base>> v;
                v.push_back(std::move(buffer));
                
                buffers.insert({ std::move(shader), std::move(v) });
            }
        }
        
    private:
        hash_map<shared<shader_program>, std::vector<shared<vertex_array_base>>> buffers;
    };
}