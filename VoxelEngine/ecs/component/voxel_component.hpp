#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/partially_synchronized.hpp>
#include <VoxelEngine/voxel/utility.hpp>
#include <VoxelEngine/voxel/space/voxel_space.hpp>
#include <VoxelEngine/voxel/space/events.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>
#include <VoxelEngine/voxel/chunk/loader/remote_loader.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/utility/algorithm.hpp>


namespace ve {
    // TODO: Refactor chunk sync code. Merge with voxel_space once pointer components are supported?
    namespace detail {
        struct msg_load_chunk {
            voxel::tilepos where;
            typename voxel::chunk::data_t data;
        };


        struct msg_unload_chunk {
            voxel::tilepos where;
        };


        struct msg_set_voxel {
            voxel::tilepos where;
            voxel::tile_data data;
        };
    }


    struct voxel_component {
        shared<voxel::voxel_space> space;

        void update(instance& owner, nanoseconds dt) {
            space->update(dt);
        }
    };


    // The synchronizable_voxel_component wraps a voxel space and synchronizes itself between client and server.
    // What chunks get synchronized to what remotes is determined by invoking the provided synchronization rule.
    // TODO: Implement rule system for voxel space modifications.
    // TODO: Tile registries should be synchronized. Identical loading order is not guaranteed.
    struct synchronizable_voxel_component : public partially_synchronized<synchronizable_voxel_component> {
        struct synchronization_rule {
            synchronization_rule(instance* local, instance_id remote) : local(local), remote(remote) {}

            virtual ~synchronization_rule(void) = default;
            virtual void update(void) = 0;
            virtual unique<synchronization_rule> clone(instance_id remote) const = 0;

            VE_GET_CREF(synchronized);
        protected:
            hash_set<voxel::tilepos> synchronized;

            VE_GET_VAL(local);
            VE_GET_VAL(remote);
        private:
            friend struct synchronizable_voxel_component;
            hash_set<voxel::tilepos> loaded_on_remote;

            instance* local;
            instance_id remote;
        };


        struct dont_synchronize : public synchronization_rule {
            using synchronization_rule::synchronization_rule;

            void update(void) override {}

            unique<synchronization_rule> clone(instance_id remote) const override {
                return make_unique<dont_synchronize>(get_local(), remote);
            }
        };


        instance* owner;
        shared<voxel::remote_loader> loader;
        shared<voxel::voxel_space> space;

        unique<synchronization_rule> rule_clone_template;
        hash_map<instance_id, unique<synchronization_rule>> sync_rules;

        event_handler_id_t on_voxel_changed;


        // For use with system_remote_init.
        static auto remote_initializer(void) {
            return [] (entt::entity entity, registry& owner) {
                owner.template set_component<synchronizable_voxel_component>(
                    entity,
                    synchronizable_voxel_component(static_cast<client&>(owner))
                );
            };
        }

        struct remote_init_tag {};


        // Server-side constructor: accepts an existing voxel space and a synchronization rule.
        synchronizable_voxel_component(server& s, shared<voxel::voxel_space> space, unique<synchronization_rule> rule) :
            partially_synchronized<synchronizable_voxel_component>(&s),
            owner(&s),
            loader(nullptr),
            space(std::move(space)),
            rule_clone_template(std::move(rule))
        {
            on_voxel_changed = space->add_handler([entity = s.entity_for_component(*this), &s] (const voxel::voxel_changed_event& e) {
                auto& self = s.template get_component<synchronizable_voxel_component>(entity);

                // TODO: it would be better to keep a map of chunk => client[] so we don't have to check every one.
                for (const auto& [remote, rule] : self.sync_rules) {
                    if (rule->synchronized.contains(voxel::to_chunkpos(e.where))) {
                        self.send_message(remote, detail::msg_set_voxel { .where = e.where, .data = e.new_value });
                    }
                }
            });
        }


        // Client-side constructor: rebuilds an existing voxel space from remote data.
        explicit synchronizable_voxel_component(client& c) :
            partially_synchronized<synchronizable_voxel_component>(&c),
            owner(&c),
            loader(make_shared<voxel::remote_loader>()),
            space(voxel::voxel_space::create(loader)),
            rule_clone_template(make_unique<dont_synchronize>(&c, nil_uuid()))
        {
            space->add_chunk_loader(loader);
        }


        ~synchronizable_voxel_component(void) {
            space->remove_chunk_loader(loader);
        }


        ve_rt_move_only(synchronizable_voxel_component);


        void update(nanoseconds dt) {
            space->update(dt);


            for (auto& connection : owner->get_connections()) {
                auto id = connection->get_remote_id();


                // Construct a new sync rule for any new remotes.
                auto it = sync_rules.find(id);
                if (it == sync_rules.end()) {
                    std::tie(it, std::ignore) = sync_rules.emplace(id, rule_clone_template->clone(id));
                }

                it->second->update();


                // For every chunk that needs to be synced but is not loaded on the remote, load it.
                for (const auto& pos : it->second->synchronized) {
                    if (auto [remote_it, success] = it->second->loaded_on_remote.emplace(pos); success) {
                        send_message(id, detail::msg_load_chunk { .where = pos, .data = space->get_chunk(pos)->get_chunk_data() });
                    }
                }


                // For every chunk that is loaded on the remote but does not need to be synced, unload it.
                std::vector<voxel::tilepos> unloaded;

                for (const auto& pos : it->second->loaded_on_remote) {
                    if (!it->second->synchronized.contains(pos)) {
                        send_message(id, detail::msg_unload_chunk { .where = pos });
                        unloaded.push_back(pos);
                    }
                }

                for (const auto& pos : unloaded) it->second->loaded_on_remote.erase(pos);
            }
        }


        template <typename Msg> void on_message_received(instance_id remote, const Msg& msg) {
            if constexpr (std::is_same_v<Msg, detail::msg_load_chunk>) {
                if (owner->get_type() == instance::SERVER) return; // The client is not in charge of chunk generation.
                loader->load_from_remote(msg.where, msg.data);
            }

            else if constexpr (std::is_same_v<Msg, detail::msg_unload_chunk>) {
                if (owner->get_type() == instance::SERVER) return; // The client is not in charge of chunk generation.
                loader->unload_from_remote(msg.where);
            }

            else if constexpr (std::is_same_v<Msg, detail::msg_set_voxel>) {
                // TODO: Perform validation here.
                space->set_data(msg.where, msg.data);
            }
        }
    };
}