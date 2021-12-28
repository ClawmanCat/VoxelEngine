#include <VoxelEngine/voxel/space/voxel_space.hpp>
#include <VoxelEngine/voxel/chunk/loader/loader.hpp>
#include <VoxelEngine/voxel/chunk/generator/generator.hpp>
#include <VoxelEngine/voxel/chunk/chunk_mesher.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/thread/thread_pool.hpp>
#include <VoxelEngine/voxel/tile/tiles.hpp>


namespace ve::voxel {
    voxel_space::chunk_locker::chunk_locker(shared<voxel_space> space, std::vector<tilepos> where) :
        space(std::move(space)),
        loader(make_shared<multi_chunk_loader>(std::move(where)))
    {
        this->space->add_chunk_loader(loader);

        const auto& positions = static_cast<const multi_chunk_loader*>(loader.get())->get_loaded_chunks();
        for (const auto& pos : positions) this->space->chunks[pos].chunk->toggle_write_lock(true);
    }


    voxel_space::chunk_locker::~chunk_locker(void) {
        if (!loader) return;

        const auto& positions = static_cast<const multi_chunk_loader*>(loader.get())->get_loaded_chunks();
        for (const auto& pos : positions) this->space->chunks[pos].chunk->toggle_write_lock(false);

        this->space->remove_chunk_loader(loader);
    }


    void voxel_space::init(unique<chunk_generator>&& generator) {
        this->generator = std::move(generator);
    }


    void voxel_space::update(nanoseconds dt) {
        VE_PROFILE_FN();

        // TODO: Account for loading priority when loading chunks.
        // TODO: Priority should not persist after chunk has been loaded.
        for (auto& loader : chunk_loaders) {
            VE_PROFILE_FN("Updating Chunk Loaders");
            loader->update(this);
        }

        update_meshes();
    }


    void voxel_space::update_meshes(void) {
        VE_PROFILE_FN("Dispatching Mesh Update Tasks");


        // TODO: Convert to coroutine and allow task to be cancelled if a newer mesh task is launched.
        // (E.g. co_yield after every n tiles or something similar.)
        struct mesh_task {
            std::optional<chunk_locker> locker;
            chunk_neighbourhood neighbourhood;
            tilepos chunkpos;
            u64 task_id;

            void operator()(void) {
                VE_PROFILE_WORKER_THREAD("Updating Mesh");
                auto mesh = mesh_chunk(neighbourhood, chunkpos);

                thread_pool::instance().invoke_on_main_and_await([&] {
                    VE_PROFILE_FN("Synchronizing Mesh");
                    auto& chunk_data = locker->get_space()->chunks[chunkpos];

                    // If there was a new mesh task launched after this one we need to discard the result.
                    // TODO: This wouldn't be necessary if we could cancel the task.
                    if (chunk_data.most_recent_mesh_task == task_id) {
                        chunk_data.subbuffer->store_mesh(std::move(mesh));
                        chunk_data.mesh_status = per_chunk_data::MESHED;
                    }

                    locker = std::nullopt;
                    --(*locker->get_space()->ongoing_mesh_tasks);
                });
            }
        };


        // TODO: Account for loading priority when first meshing chunks.
        for (auto& [pos, chunk_data] : chunks) {
            // If the current chunk is locked and has pending changes, don't mesh it yet, as we would mesh the old state.
            if (chunk_data.chunk->has_pending_changes()) continue;


            if (chunk_data.mesh_status == per_chunk_data::NEEDS_MESHING) {
                chunk_data.mesh_status = per_chunk_data::MESHING;

                std::vector<tilepos> positions = { pos };
                std::array<const chunk*, directions.size()> neighbours;

                for (const auto& [i, direction] : directions | views::enumerate) {
                    if (auto it = chunks.find(pos + direction); it != chunks.end()) {
                        positions.push_back(pos + direction);
                        neighbours[i] = it->second.chunk.get();
                    } else {
                        neighbours[i] = nullptr;
                    }
                }


                thread_pool::instance().invoke_on_thread(mesh_task {
                    .locker        = chunk_locker { shared_from_this(), std::move(positions) },
                    .neighbourhood = chunk_neighbourhood { chunk_data.chunk.get(), neighbours },
                    .chunkpos      = pos,
                    .task_id       = ++chunk_data.most_recent_mesh_task
                });
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
            it->second.mesh_status = per_chunk_data::NEEDS_MESHING;

            for (const auto& dir : directions) {
                auto neighbour_chunkpos = to_chunkpos(where + tilepos { dir });

                if (neighbour_chunkpos != chunkpos) {
                    if (auto neighbour = chunks.find(neighbour_chunkpos); neighbour != chunks.end()) {
                        neighbour->second.mesh_status = per_chunk_data::NEEDS_MESHING;
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


    void voxel_space::load_chunk(const tilepos& where, u16 priority) {
        if (auto it = chunks.find(where); it != chunks.end()) {
            it->second.load_count++;
            it->second.load_priority = std::max(it->second.load_priority, priority);
        } else {
            // Note: actual meshing is performed during the next tick, so if we load multiple chunks at once,
            // we don't need to mesh them twice.
            auto buffer = detail::subbuffer_t::create();
            auto handle = vertex_buffer->insert(buffer);

            chunks.emplace(
                where,
                per_chunk_data {
                    .chunk                 = generator->generate(this, where),
                    .subbuffer             = buffer,
                    .handle                = handle,
                    .mesh_status           = per_chunk_data::NEEDS_MESHING,
                    .load_count            = 1,
                    .load_priority         = priority
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
                    neighbour->second.mesh_status = per_chunk_data::NEEDS_MESHING;
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
                        neighbour->second.mesh_status = per_chunk_data::NEEDS_MESHING;
                    }
                }
            }
        } else {
            VE_LOG_ERROR("Attempt to unload a chunk that was already unloaded. This may indicate an issue with the chunk loader.");
        }
    }
}