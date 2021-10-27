#include <VoxelEngine/voxel/space/voxel_space.hpp>
#include <VoxelEngine/voxel/chunk/loader.hpp>
#include <VoxelEngine/voxel/chunk/generator.hpp>
#include <VoxelEngine/voxel/chunk/chunk_mesher.hpp>
#include <VoxelEngine/voxel/tile/tiles.hpp>


namespace ve::voxel {
    voxel_space::voxel_space(unique<chunk_generator>&& generator) :
        generator(std::move(generator)),
        vertex_buffer(detail::buffer_t::create())
    {}


    void voxel_space::update(nanoseconds dt) {
        for (auto& loader : chunk_loaders) loader->update(this);


        VE_DEBUG_ONLY(
            u32 mesh_count = ranges::count_if(chunks | views::values, equal_on(&per_chunk_data::needs_meshing, true));
            if (mesh_count) VE_LOG_DEBUG(cat("Re-meshing ", mesh_count, " chunks."));
        );

        for (auto& [pos, chunk_data] : chunks) {
            if (chunk_data.needs_meshing) {
                chunk_data.subbuffer->store_mesh(mesh_chunk(this, chunk_data.chunk.get(), pos));
                chunk_data.needs_meshing = false;
            }
        }
    }


    const tile_data& voxel_space::get_data(const tilepos& where) const {
        if (auto it = chunks.find(to_chunkpos(where)); it != chunks.end()) {
            return it->second.chunk->get_data(to_localpos(where));
        } else {
            const static auto td_unknown = voxel_settings::get_tile_registry().get_default_state(tiles::TILE_UNKNOWN);
            return td_unknown;
        }
    }


    void voxel_space::set_data(const tilepos& where, const tile_data& td) {
        auto chunkpos = to_chunkpos(where);


        if (auto it = chunks.find(chunkpos); it != chunks.end()) {
            it->second.chunk->set_data(to_localpos(where), td);


            // Re-mesh the current chunk and also its neighbours if the position is on the edge of the chunk.
            it->second.needs_meshing = true;
            for (const auto& dir : directions) {
                auto neighbour_chunkpos = to_chunkpos(where + tilepos { dir });

                if (neighbour_chunkpos != chunkpos) {
                    if (auto neighbour = chunks.find(neighbour_chunkpos); neighbour != chunks.end()) {
                        neighbour->second.needs_meshing = true;
                    }
                }
            }
        } else {
            VE_LOG_WARN("Attempt to set tile in unloaded chunk. Operation will be ignored.");
        }
    }


    bool voxel_space::is_loaded(const tilepos& chunkpos) const {
        return chunks.contains(chunkpos);
    }


    void voxel_space::add_chunk_loader(shared<chunk_loader> loader) {
        loader->start_loading(this);
        chunk_loaders.insert(std::move(loader));
    }


    void voxel_space::remove_chunk_loader(const shared<chunk_loader>& loader) {
        chunk_loaders.erase(loader);
        loader->stop_loading(this);
    }


    void voxel_space::load_chunk(const tilepos& where) {
        if (auto it = chunks.find(where); it != chunks.end()) {
            it->second.load_count++;
        } else {
            // Note: actual meshing is performed during the next tick, so if we load multiple chunks at once,
            // we don't need to mesh them twice.
            auto buffer = detail::subbuffer_t::create();
            auto handle = vertex_buffer->insert(buffer);

            chunks.emplace(
                where,
                per_chunk_data {
                    .chunk         = generator->generate(this, where),
                    .subbuffer     = buffer,
                    .handle        = handle,
                    .needs_meshing = true,
                    .load_count    = 1
                }
            );

            buffer->set_uniform_value<mat4f>(
                "transform",
                glm::translate(glm::identity<mat4f>(), vec3f { where * tilepos { voxel_settings::chunk_size } }),
                gfx::combine_functions::multiply
            );


            // Also re-mesh neighbours since we probably don't have to render most of the shared face with this chunk anymore.
            for (const auto& dir : directions) {
                if (auto neighbour = chunks.find(where + tilepos { dir }); neighbour != chunks.end()) {
                    neighbour->second.needs_meshing = true;
                }
            }
        }
    }


    void voxel_space::unload_chunk(const tilepos& where) {
        if (auto it = chunks.find(where); it != chunks.end()) {
            it->second.load_count--;

            if (it->second.load_count == 0) {
                vertex_buffer->erase(it->second.handle);
                chunks.erase(it);

                // Re-mesh neighbours since we need to start rendering the shared face with this chunk again.
                for (const auto& dir : directions) {
                    if (auto neighbour = chunks.find(where + tilepos { dir }); neighbour != chunks.end()) {
                        neighbour->second.needs_meshing = true;
                    }
                }
            }
        } else {
            VE_LOG_ERROR("Attempt to unload a chunk that was already unloaded. This may indicate an issue with the chunk loader.");
        }
    }
}