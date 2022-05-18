#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/component/partially_synchronizable.hpp>
#include <VoxelEngine/voxel/space/events.hpp>
#include <VoxelEngine/voxel/space/voxel_space.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>
#include <VoxelEngine/voxel/chunk/loader/remote_loader.hpp>


namespace ve {
    class voxel_component : public partially_synchronizable<voxel_component> {
        struct chunk_load_message {
            voxel::tilepos where;
            typename voxel::chunk::data_t data;
        };

        struct chunk_unload_message {
            voxel::tilepos where;
        };

        struct set_voxel_message {
            voxel::tilepos where;
            voxel::tile_data data;
        };

    public:
        using constant_address_tag = void;


        voxel_component(bool host = false, shared<voxel::chunk_generator> generator = nullptr) : host(host) {
            if (host) {
                VE_DEBUG_ASSERT(generator, "A chunk generator should be provided for the hosting voxel space.");

                loader = std::move(generator);
                space  = voxel::voxel_space::create(loader);
            } else {
                auto remote_generator = make_shared<voxel::remote_loader>();

                loader = remote_generator;
                space  = voxel::voxel_space::create(remote_generator);

                space->add_chunk_loader(std::move(remote_generator));
            }

            // TODO: Find a more robust way to do this, this will break on unified instances.
            space->toggle_meshing(!host);
        }


        void on_component_added_wrapped(registry& owner, entt::entity entity) {
            on_chunk_load = space->add_raw_handler([this] (const voxel::chunk_loaded_event& e) {
                broadcast_message(chunk_load_message { .where = e.chunkpos, .data = e.chunk->get_chunk_data() });
            });

            on_chunk_unload = space->add_raw_handler([this] (const voxel::chunk_unloaded_event& e) {
                broadcast_message(chunk_unload_message { .where = e.chunkpos });
            });

            on_voxel_set = space->add_raw_handler([this] (const voxel::voxel_changed_event& e) {
                broadcast_message(set_voxel_message { .where = e.where, .data = e.new_value });
            });
        }


        void on_component_removed(registry& owner, entt::entity entity) {
            space->remove_handler<voxel::chunk_loaded_event>  (on_chunk_load);
            space->remove_handler<voxel::chunk_unloaded_event>(on_chunk_unload);
            space->remove_handler<voxel::voxel_changed_event> (on_voxel_set);
        }


        void on_added_to_remote(instance_id remote) {
            for (const auto& [pos, data] : space->get_chunks()) {
                send_message(chunk_load_message { .where = pos, .data = data.chunk->get_chunk_data() }, remote);
            }
        }


        template <typename Msg> void on_message_received(const Msg& msg, instance_id remote) {
            if constexpr (std::is_same_v<Msg, set_voxel_message>) {
                // TODO: If host, check if valid.
                space->set_data(msg.where, msg.data);
            }

            else if constexpr (std::is_same_v<Msg, chunk_load_message>) {
                if (host) return; // Only the host may load chunks.
                static_cast<voxel::remote_loader*>(loader.get())->load_from_remote(msg.where, msg.data);
            }

            else if constexpr (std::is_same_v<Msg, chunk_unload_message>) {
                if (host) return; // Only the host may unload chunks.
                static_cast<voxel::remote_loader*>(loader.get())->unload_from_remote(msg.where);
            }
        }


        VE_GET_VAL(space);
        VE_GET_VAL(loader);
        VE_GET_BOOL_IS(host);
    private:
        shared<voxel::voxel_space> space = nullptr;
        shared<voxel::chunk_generator> loader = nullptr;
        event_handler_id_t on_chunk_load, on_chunk_unload, on_voxel_set;

        bool host;
    };
}