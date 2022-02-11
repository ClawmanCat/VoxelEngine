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
    template <typename VisibilityRule = fn<bool, const registry&, entt::entity, const message_handler*>> requires (
        std::is_invocable_r_v<bool, VisibilityRule, const registry&, entt::entity, const message_handler*>
    ) class system_entity_visibility : public system<system_entity_visibility<VisibilityRule>> {
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


        explicit system_entity_visibility(
            VisibilityRule rule = [](const registry&, entt::entity, const message_handler*) { return true; },
            u16 priority = priority::LOWEST + 1
        ) :
            priority(priority),
            rule(std::move(rule))
        {}


        u16 get_priority(void) const {
            return priority;
        }


        void init(registry& owner) {
            VE_DEBUG_ASSERT(
                dynamic_cast<class instance*>(&owner),
                "Registry must be part of an instance in order to use a synchronization system."
            );

            entity_destroyed_handler = owner.add_handler([&] (const entity_destroyed_event& e) {
                destroyed_entities.push_back(e.entity);
            });

            remote_disconnected_handler = owner.add_handler([&] (const instance_disconnected_event& e) {
                storage.erase(e.remote);
            });

            destroyed_entities.clear();
            storage.clear();

            this->owner = static_cast<class instance*>(&owner);
        }


        void uninit(registry& owner) {
            owner.template remove_handler<entity_destroyed_event>(entity_destroyed_handler);
            owner.template remove_handler<instance_disconnected_event>(remote_disconnected_handler);

            this->owner = nullptr;
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


        // Returns a view of the visibility status of every entity for the given remote. Note that this includes invisible entities.
        // Entities and remotes that were added since the last call to update are not accessible this way.
        auto visibility_for_remote(instance_id remote) const {
            return view_from_storage(storage.at(remote));
        }


        // Returns the visibility of the given entity for the given instance.
        // Note that unlike the visibility view, this method is also able to evaluate entities that may have been added
        // since the last call to update.
        bool is_visible(entt::entity entity, instance_id remote) const {
            // TODO: Would it be better to always invoke the rule instead of checking this?
            if (auto remote_it = storage.find(remote); remote_it != storage.end()) {
                if (remote_it->second.contains(entity)) {
                    return remote_it->second.get(entity) & VISIBILITY_BIT;
                }
            }

            return std::invoke(rule, static_cast<const registry&>(*owner), entity, owner->get_connection(remote).get());
        }
    private:
        u16 priority;

        instance* owner = nullptr;
        VisibilityRule rule;
        hash_map<instance_id, storage_type<visibility_status>> storage;

        event_handler_id_t entity_destroyed_handler, remote_disconnected_handler;
        std::vector<entt::entity> destroyed_entities;
    };


    template <typename Rule>
    system_entity_visibility(Rule) -> system_entity_visibility<Rule>;

    template <typename Rule>
    system_entity_visibility(Rule, u16) -> system_entity_visibility<Rule>;
}