#include <VoxelEngine/voxel/space.hpp>
#include <VoxelEngine/graphics/mesh/voxel_mesher.hpp>
#include <VoxelEngine/voxel/tile/tiles.hpp>


namespace ve {
    voxel_space::voxel_space(unique<chunk_generator>&& generator)
        : generator(std::move(generator)), mesh(std::make_shared<graphics::multi_buffer>())
    {}
    
    
    [[nodiscard]] const tile_data& voxel_space::get(const vec3i& where) const {
        const chunkpos chunk_pos = chunk_position(where);
        auto it = chunks.find(chunk_pos);
    
        if (it == chunks.end()) {
            const static tile_data unknown = *tiles::TILE_UNKNOWN;
            return unknown;
        }
    
        chunk& chnk = *(it->second.chunk);
        return chnk.tiles[where - tile_position(chunk_pos)];
    }
    
    
    void voxel_space::set(const vec3i& where, const tile_data& td) {
        const chunkpos chunk_pos    = chunk_position(where);
        const tilepos  in_chunk_pos = where - tile_position(chunk_pos);
        
        auto it = chunks.find(chunk_pos);
        if (it == chunks.end()) {
            VE_LOG_WARN("Attempt to set tile in unloaded chunk. Modification was not recorded.");
            return;
        }
    
        chunk& chnk = *(it->second.chunk);
        chnk.tiles[in_chunk_pos] = td;
        
        // Mark the current chunk as needing a new mesh, and also neighbouring chunks if the tile is on a chunk border.
        // Note: the mesh update method already checks if the marked chunks are actually loaded, so we don't have to do so here.
        unmeshed_chunks.insert(chunk_pos);
        
        for (auto dir : direction{}) {
            auto axis = axis_of_traversal<tilepos>(dir);
            auto neighbour = in_chunk_pos + direction_vector(dir);
            
            if (neighbour.*axis < 0 || neighbour.*axis >= (i32) voxel_settings::chunk_size) {
                unmeshed_chunks.insert(chunk_pos + direction_vector(dir));
            }
        }
    }


    void voxel_space::on_serialized(void) {
        VE_ASSERT(false, "Not yet implemented.");
    }


    void voxel_space::on_deserialized(void) {
        VE_ASSERT(false, "Not yet implemented.");
    }
    
    
    // Allow manually deferring the creation of the chunk meshes for increased performance.
    // TODO: Do this automatically via event-based rendering system.
    void voxel_space::toggle_mesh_updates(bool enabled) {
        if (enabled && !mesh_updates_enabled) update_mesh();
        mesh_updates_enabled = enabled;
    }
    
    
    [[nodiscard]] shared<graphics::buffer> voxel_space::get_mesh(void) {
        return mesh;
    }
    
    
    void voxel_space::add_loader(shared<chunk_loader>&& loader) {
        loaders.insert(std::move(loader));
    }
    
    
    void voxel_space::remove_loader(const chunk_loader* loader) {
        loaders.erase(loader);
    }
    
    
    [[nodiscard]] const chunk* voxel_space::get_chunk(const chunkpos& where) const {
        auto it = chunks.find(where);
        return (it == chunks.end()) ? nullptr : it->second.chunk.get();
    }

    
    void voxel_space::update_mesh(void) {
        if (!unmeshed_chunks.empty()) {
            VE_LOG_DEBUG("Remeshing "s + std::to_string(unmeshed_chunks.size()) + " chunks.");
        }
        
        for (const auto& pos : unmeshed_chunks) {
            auto it = chunks.find(pos);
            if (it == chunks.end()) continue;
            
            auto& chnk = it->second;
            
            
            using buffer_t = std::conditional_t<
                voxel_mesh_t::indexed,
                graphics::indexed_vertex_buffer<
                    typename voxel_mesh_t::vertex_t,
                    typename voxel_mesh_t::index_t
                >,
                graphics::vertex_buffer<
                    typename voxel_mesh_t::vertex_t
                >
            >;
            
            mesh->erase(chnk.mesh);
            chnk.mesh = std::make_shared<buffer_t>(graphics::mesh_chunk(pos, *chnk.chunk, *this));

            chnk.mesh->set_uniform_value(
                "transform"s,
                translation_matrix((vec3f) (pos * (i32) voxel_settings::chunk_size))
            );
            
            mesh->insert(copy(chnk.mesh));
        }
        
        unmeshed_chunks.clear();
    }
    
    
    void voxel_space::invalidate_neighbour_meshes(const chunkpos& where) {
        spatial_iterate(where, vec3i { 1 }, [&](const vec3i& pos) {
            if (chunks.contains(pos)) unmeshed_chunks.insert(pos);
        });
        
        if (mesh_updates_enabled) update_mesh();
    }


    void voxel_space::load_chunk(const chunkpos& where) {
        auto it = chunks.find(where);
        
        if (it != chunks.end()) {
            ++(it->second.load_count);
        } else {
            chunks.insert({
                where,
                per_chunk_data {
                    .chunk      = generator->generate(where),
                    .position   = where,
                    .mesh       = nullptr,
                    .load_count = 1
                }
            });

            invalidate_neighbour_meshes(where);
        }
    }


    void voxel_space::unload_chunk(const chunkpos& where) {
        auto it = chunks.find(where);
        auto& chunk_data = it->second;
        
        if (chunk_data.load_count == 1) {
            mesh->erase(chunk_data.mesh);
            chunks.erase(it);
            unmeshed_chunks.erase(where);

            invalidate_neighbour_meshes(where);
        } else {
            --chunk_data.load_count;
        }
    }
}