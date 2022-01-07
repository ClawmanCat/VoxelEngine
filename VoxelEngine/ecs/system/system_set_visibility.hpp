#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/storage_group/group.hpp>
#include <VoxelEngine/clientserver/instance.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/functional.hpp>


namespace ve {
    // Decides which entities are visible to which remotes instance based on the result of pred.
    // Pred must be invocable as pred(registry&, entity, message_handler*) -> bool.
    // Note that this system does not perform any kind of synchronization by itself.
    // Instead other systems like system_synchronizer use this when performing their own synchronization.
    // Note that each registry can have at most one visibility provider.
    template <
        typename Pred = fn<bool, registry&, entt::entity, message_handler*>
    > requires (
        std::is_invocable_r_v<bool, Pred, registry&, entt::entity, message_handler*>
    ) class system_set_visibility :
        public system<
            system_set_visibility<Pred>,
            meta::pack<>,
            meta::pack<>
        >,
        private registry_access_for_visibility_provider<
            system_set_visibility<Pred>
        >
    {
    public:
        using visibility_provider_tag = void;


        // Visible = v & 0b01, Changed = v & 0b10.
        // Note that this makes INVISIBLE the zero-initialized state, which means new entities start off invisible.
        enum visibility_state : u8 {
            INVISIBLE        = 0b00,
            VISIBLE          = 0b01,
            BECAME_INVISIBLE = 0b10,
            BECAME_VISIBLE   = 0b11
        };

        constexpr static inline u8 VISIBILITY_BIT = 0b01;
        constexpr static inline u8 CHANGE_BIT = 0b10;


        explicit system_set_visibility(
            Pred pred    = [] (auto& r, auto e, auto* h) { return true; },
            u16 priority = priority::LOWEST + 1
        ) :
            pred(std::move(pred)),
            priority(priority)
        {}


        u16 get_priority(void) const {
            return priority;
        }


        void init(registry& owner) {
            set_visibility_provider(owner);


            entity_destroyed_handler = owner.add_handler([this](const entity_destroyed_event& e) {
                removed_entities.push_back(e.entity);

                // Entities that are destroyed will appear as going invisible.
                for (auto& [id, storage] : visibility_map) {
                    auto& visibility = storage.get(e.entity);
                    if (visibility != INVISIBLE) visibility = BECAME_INVISIBLE;
                }
            });
        }


        void uninit(registry& owner) {
            clear_visibility_provider(owner);
            owner.template remove_handler<entity_destroyed_event>(entity_destroyed_handler);
        }


        void update(registry& owner, view_type view, nanoseconds dt) {
            instance& i = static_cast<instance&>(owner);
            auto connections = i.get_connections();


            for (auto& connection : connections) {
                auto& states_for_connection = visibility_map.try_emplace(connection->get_remote_id(), owner.get_storage()).first->second;

                for (auto entity : view) {
                    u8& visibility  = (u8&) states_for_connection.try_insert(entity, INVISIBLE);
                    u8  visible_now = (u8)  std::invoke(pred, owner, entity, connection.get());

                    visibility = visibility_state(
                        // Copy visibility bit from new state.
                        (visible_now & VISIBILITY_BIT) |
                        // If the visibility bit changed, the changed bit is true.
                        (u8((visible_now & VISIBILITY_BIT) != (visibility & VISIBILITY_BIT)) << 1)
                    );
                }
            }


            for (auto entity : removed_entities) {
                for (auto& [id, storage] : visibility_map) storage.remove(entity);
            }

            removed_entities.clear();
        }


        auto visibility_for(instance_id remote) {
            return visibility_map.at(remote).view();
        }

        auto visibility_for(instance_id remote) const {
            return visibility_map.at(remote).view();
        }


        bool is_visible(entt::entity entity, instance_id remote) const {
            return visibility_map.at(remote).contains(entity);
        }
    private:
        hash_map<instance_id, storage_group<visibility_state>> visibility_map;

        std::vector<entt::entity> removed_entities;
        event_handler_id_t entity_destroyed_handler;

        Pred pred;
        u16 priority;
    };
}