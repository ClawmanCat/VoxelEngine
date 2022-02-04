#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/clientserver/instance.hpp>
#include <VoxelEngine/clientserver/instance_events.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message_setup.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_compound.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_add_del_entity.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    // TODO: It would be more optimal to have the ability to create a view over all entities with the same visibility state.
    // E.g. create a view of all visible entities, or all entities that are going invisible.
    template <typename VisibilityRule> requires std::is_invocable_r_v<bool, VisibilityRule, registry&, entt::entity, message_handler*>
    class system_entity_visibility : public system<system_entity_visibility<VisibilityRule>> {
    public:
        using system_entity_visibility_tag = void;
        using visibility_rule_t            = VisibilityRule;


        enum visibility_status : u8 {
            INVISIBLE        = 0b00,
            VISIBLE          = 0b01,
            BECAME_INVISIBLE = 0b10,
            BECAME_VISIBLE   = 0b11
        };

        constexpr static inline u8 VISIBILITY_BIT = 0b01;
        constexpr static inline u8 CHANGED_BIT    = 0b10;


        explicit system_entity_visibility(VisibilityRule rule, u16 priority = priority::LOWEST + 1) :
            priority(priority),
            rule(std::move(rule))
        {}


        u16 get_priority(void) const {
            return priority;
        }


        void init(registry& owner) {
            entity_destroyed_handler = owner.add_handler([&] (const entity_destroyed_event& e) {
                destroyed_entities.push_back(e.entity);
            });

            remote_disconnected_handler = owner.add_handler([&] (const instance_disconnected_event& e) {
                storage.erase(e.remote);
            });

            destroyed_entities.clear();
            storage.clear();
        }


        void uninit(registry& owner) {
            owner.template remove_handler<entity_destroyed_event>(entity_destroyed_handler);
            owner.template remove_handler<instance_disconnected_event>(remote_disconnected_handler);
        }


        void update(registry& owner, view_type view, nanoseconds dt) {
            auto try_insert = [](auto& ctr, auto k, auto v) -> decltype(auto) {
                return ctr.contains(k) ? ctr.get(k) : ctr.emplace(k, v);
            };


            auto& instance = static_cast<class instance&>(owner);
            add_del_entity_message added, removed;


            for (auto& connection : instance.get_connections()) {
                auto [it, success] = storage.try_emplace(connection->get_remote_id());
                auto& storage_for_conn = it->second;


                // For every entity, update its visibility status based on the provided rule.
                for (auto entity : view) {
                    visibility_status& old_status = try_insert(storage_for_conn, entity, INVISIBLE);
                    visibility_status  new_status = visibility_status((u8) std::invoke(rule, owner, entity, connection.get()));

                    old_status = visibility_status(
                        // The visibility bit is set if the current status is visible.
                        (new_status & VISIBILITY_BIT) |
                        // The change bit is set if the visibility bit changed between the old and new state.
                        (((old_status & VISIBILITY_BIT) != (new_status & VISIBILITY_BIT)) << 1)
                    );


                    if      (old_status == BECAME_VISIBLE  ) added.changed.push_back(entity);
                    else if (old_status == BECAME_INVISIBLE) removed.changed.push_back(entity);
                }


                // Destroyed entities will appear as going invisible on the remote.
                for (auto entity : destroyed_entities) {
                    visibility_status status = storage_for_conn.get(entity);
                    if (status == VISIBLE) removed.changed.push_back(entity);
                }

                storage_for_conn.erase(destroyed_entities.begin(), destroyed_entities.end());


                // Send lists of entities that became visible and invisible to remote.
                if (!added.changed.empty()) {
                    connection->send_message(core_message_types::MSG_ADD_ENTITY, added);
                    added.changed.clear();
                }

                if (!removed.changed.empty()) {
                    connection->send_message(core_message_types::MSG_DEL_ENTITY, removed);
                    removed.changed.clear();
                }
            }


            destroyed_entities.clear();
        }


        auto visibility_for_remote(instance_id remote) const {
            return view_from_storage(storage.at(remote));
        }


        bool is_visible(entt::entity entity, instance_id remote) const {
            const auto& storage_for_conn = storage.at(remote);

            if (storage_for_conn.contains(entity)) {
                return storage_for_conn.get(entity) & VISIBILITY_BIT;
            }

            return false;
        }
    private:
        u16 priority;

        VisibilityRule rule;
        hash_map<instance_id, storage_type<visibility_status>> storage;

        event_handler_id_t entity_destroyed_handler, remote_disconnected_handler;
        std::vector<entt::entity> destroyed_entities;
    };
}