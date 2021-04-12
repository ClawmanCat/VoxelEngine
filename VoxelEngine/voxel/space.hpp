#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/tile/tile_provider.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>
#include <VoxelEngine/voxel/chunk/generator/generator.hpp>
#include <VoxelEngine/voxel/chunk/loader/loader.hpp>
#include <VoxelEngine/voxel/voxel_settings.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/graphics/mesh/voxel_mesher.hpp>
#include <VoxelEngine/graphics/buffer/multi_buffer.hpp>
#include <VoxelEngine/side/side.hpp>


namespace ve {
    class voxel_space :
        public tile_provider<voxel_space>,
        public component<voxel_space, side::BOTH, component_serialization_mode::MANUAL>
    {
    public:
        explicit voxel_space(unique<chunk_generator>&& generator)
            : generator(std::move(generator)), mesh(std::make_shared<graphics::multi_buffer>())
        {}
        
        ve_move_only(voxel_space);
        
        
        [[nodiscard]] const tile_data& operator[](const vec3i& where) const {
            const chunkpos chunk_pos = chunk_position(where);
            
            auto it = chunks.find(chunk_pos);
            VE_ASSERT(it != chunks.end(), "Attempt to access unloaded chunk");
            
            chunk& chnk = *(it->second.chunk);
            return chnk[where - chunk_pos];
        }
    
    
        void on_serialized(void) {
            VE_ASSERT(false, "Not yet implemented.");
        }
    
    
        void on_deserialized(void) {
            VE_ASSERT(false, "Not yet implemented.");
        }
        
        
        shared<graphics::buffer> get_mesh(void) const {
            return mesh;
        }
        
        
        void add_loader(shared<chunk_loader>&& loader) {
            loaders.insert(std::move(loader));
        }
        
        
        void remove_loader(const chunk_loader* loader) {
            loaders.erase(loader);
        }
        
    private:
        friend class chunk_loader;
        
        
        struct per_chunk_data {
            unique<chunk> chunk;
            chunkpos position;
            shared<graphics::buffer> mesh = nullptr;
            u32 load_count = 0;
        };
        
        
        hash_map<chunkpos, per_chunk_data> chunks;
        unique<chunk_generator> generator;
        shared<graphics::multi_buffer> mesh;
        hash_set<shared<chunk_loader>> loaders;
        
        
        constexpr static chunkpos chunk_position(const tilepos& world_position) {
            return chunkpos(world_position / (i32) voxel_settings::chunk_size);
        }
    
    
        void load_chunk(const chunkpos& where) {
            auto it = chunks.find(where);
            
            if (it != chunks.end()) {
                ++(it->second.load_count);
            } else {
                auto [it, changed] = chunks.insert({
                    where,
                    per_chunk_data {
                        .chunk      = generator->generate(where),
                        .position   = where,
                        .mesh       = nullptr,
                        .load_count = 1
                    }
                });
                
                auto& chunk_data = it->second;
                
                if constexpr (voxel_settings::mesh_t::indexed) {
                    using vertex_t = typename voxel_settings::mesh_t::vertex_t;
                    using index_t  = typename voxel_settings::mesh_t::index_t;
                    
                    chunk_data.mesh = std::make_shared<
                        graphics::indexed_vertex_buffer<vertex_t, index_t>
                    >(graphics::mesh_chunk(*chunk_data.chunk));
                    
                    chunk_data.mesh->set_uniform_value(
                        "transform"s,
                        glm::translate(glm::identity<mat4f>(), (vec3f) (where * (i32) voxel_settings::chunk_size))
                    );
                } else {
                    using vertex_t = typename voxel_settings::mesh_t::vertex_t;
    
                    chunk_data.mesh = std::make_shared<
                        graphics::vertex_buffer<vertex_t>
                    >(graphics::mesh_chunk(*chunk_data.chunk));
    
                    chunk_data.mesh->set_uniform_value(
                        "transform"s,
                        glm::translate(glm::identity<mat4f>(), (vec3f) (where * (i32) voxel_settings::chunk_size))
                    );
                }
                
                mesh->insert(copy(chunk_data.mesh));
            }
        }
    
    
        void unload_chunk(const chunkpos& where) {
            auto it = chunks.find(where);
            auto& chunk_data = it->second;
            
            if (chunk_data.load_count == 1) {
                mesh->erase(chunk_data.mesh);
                chunks.erase(it);
            } else {
                --chunk_data.load_count;
            }
        }
    };
}