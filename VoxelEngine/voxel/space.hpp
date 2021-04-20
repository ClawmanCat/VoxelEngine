#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/tile/tile_provider.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>
#include <VoxelEngine/voxel/chunk/generator/generator.hpp>
#include <VoxelEngine/voxel/chunk/loader/loader.hpp>
#include <VoxelEngine/voxel/voxel_settings.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/graphics/buffer/multi_buffer.hpp>
#include <VoxelEngine/side/side.hpp>


namespace ve {
    // TODO: Split for server and client.
    class voxel_space :
        public tile_provider<voxel_space>,
        public component<voxel_space, side::BOTH, component_serialization_mode::MANUAL>
    {
    public:
        explicit voxel_space(unique<chunk_generator>&& generator);
        ve_move_only(voxel_space);
    
        [[nodiscard]] const tile_data& get(const vec3i& where) const;
        void set(const vec3i& where, const tile_data& td);
        
        void on_serialized(void);
        void on_deserialized(void);
        
        // TODO: Should be private when meshing is handled by events.
        void update_mesh(void);
        
        // Allow manually deferring the creation of the chunk meshes for increased performance.
        // TODO: Do this automatically via event-based rendering system.
        void toggle_mesh_updates(bool enabled);
        [[nodiscard]] shared<graphics::buffer> get_mesh(void);
        
        void add_loader(shared<chunk_loader>&& loader);
        void remove_loader(const chunk_loader* loader);
        
        // Returns nullptr if the chunk is not currently loaded.
        [[nodiscard]] const chunk* get_chunk(const chunkpos& where) const;
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
        hash_set<shared<chunk_loader>> loaders;
    
        shared<graphics::multi_buffer> mesh;
        hash_set<chunkpos> unmeshed_chunks;
        bool mesh_updates_enabled = true;
        
        
        constexpr static chunkpos chunk_position(const tilepos& world_position) {
            constexpr u32 exponent = least_significant_bit(voxel_settings::chunk_size);
            return chunkpos(world_position >> (i32) exponent);
        }
        
        
        constexpr static tilepos tile_position(const chunkpos& chunk_position) {
            constexpr u32 exponent = least_significant_bit(voxel_settings::chunk_size);
            return tilepos(chunk_position) << (i32) exponent;
        }
        
        
        void invalidate_neighbour_meshes(const chunkpos& where);
    
        void load_chunk(const chunkpos& where);
        void unload_chunk(const chunkpos& where);
    };
}