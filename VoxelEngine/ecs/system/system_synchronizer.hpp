#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/system/system_set_visibility.hpp>
#include <VoxelEngine/ecs/storage_group/group.hpp>
#include <VoxelEngine/clientserver/instance.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_compound.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_add_del_entity.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_set_component.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_del_component.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>

#include <xxhash.h>


namespace ve {
    namespace detail {
        // For every synchronized component, a hash is stored of the value it last had while being synchronized.
        // If the value is unchanged, synchronization can be skipped.
        template <typename Component> struct component_sync_shadow {
            using non_syncable_tag  = void;

            XXH64_hash_t hash;
        };
    }


    struct dont_synchronize_by_default_tag {
        using non_removable_tag = void;
        using non_syncable_tag  = void;
    };


    // Broadcasts changes to the ECS to remote instances.
    // This system should be coupled to a system_set_visibility.
    // TODO: Handle per-component synchronization rates.
    // TODO: Move entity add/remove messages to a different system.
    template <
        meta::pack_of_types Synchronized,
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<dont_synchronize_by_default_tag>,
        typename VisibilityPred = fn<bool, registry&, entt::entity, message_handler*>
    > requires (
        Synchronized::all([] <typename Component> { return serialize::is_serializable<Component>; }) &&
        Synchronized::all([] <typename Component> { return !requires { typename Component::non_syncable_tag; }; }) &&
        // TODO: Remove below requirements. We need to make sure to add/remove the sync shadow if the inclusion of an entity changes.
        RequiredTags::all([] <typename Component> { return requires { typename Component::non_removable_tag; }; }) &&
        ExcludedTags::all([] <typename Component> { return requires { typename Component::non_removable_tag; }; })
    ) class system_synchronizer : public system<
        system_synchronizer<Synchronized, RequiredTags, ExcludedTags>,
        // Synchronized components are not in the required list, since we still synchronize if the entity only has some of them.
        RequiredTags,
        ExcludedTags
    > {
    public:
        using visibility_system = system_set_visibility<VisibilityPred>;


        explicit system_synchronizer(visibility_system* visibility, u16 priority = priority::LOWEST) :
            visibility(visibility),
            priority(priority)
        {
            VE_ASSERT(visibility->get_priority() > this->get_priority(), "Visibility should be updated before synchronization.");
        }


        u16 get_priority(void) const {
            return priority;
        }


        void init(registry& owner) {
            removed_components = storage_group<removed_component_list> { owner.get_storage() };


            Synchronized::foreach_indexed([&] <typename Component, std::size_t I> () {
                // Whenever a component is added, also add a sync shadow.
                on_component_added_handlers[I] = owner.add_handler([] (const component_created_event<Component>& e) {
                    if (!RequiredTags::all([&] <typename R> () { return e.owner->template has_component<R>(e.entity); })) return;
                    if ( ExcludedTags::any([&] <typename R> () { return e.owner->template has_component<R>(e.entity); })) return;

                    e.owner->set_component(e.entity, detail::component_sync_shadow<Component> { 0 });
                });

                // Whenever a component is removed, delete the sync shadow and add it to the list of deleted components.
                on_component_removed_handlers[I] = owner.add_handler([this] (const component_destroyed_event<Component>& e) {
                    if (!RequiredTags::all([&] <typename R> () { return e.owner->template has_component<R>(e.entity); })) return;
                    if ( ExcludedTags::any([&] <typename R> () { return e.owner->template has_component<R>(e.entity); })) return;

                    e.owner->template remove_component<detail::component_sync_shadow<Component>>(e.entity);

                    auto& removed_components_for_entity = removed_components.try_insert(e.entity);
                    removed_components_for_entity.components.push_back(type_hash<Component>());
                });
            });
        }


        void uninit(registry& owner) {
            // Remove event handlers and all remaining sync shadows.
            Synchronized::foreach_indexed([&] <typename Component, std::size_t I> () {
                owner.template remove_handler<component_created_event<Component>>(on_component_added_handlers[I]);
                owner.template remove_handler<component_destroyed_event<Component>>(on_component_removed_handlers[I]);


                using Included = typename RequiredTags::template append<Component>;
                using Excluded = ExcludedTags;

                for (auto entity : owner.template view_except<Included, Excluded>()) {
                    owner.template remove_component<detail::component_sync_shadow<Component>>(entity);
                }
            });

            removed_components.clear();
        }


        void update(registry& owner, view_type view, nanoseconds dt) {
            using vis_state = typename visibility_system::visibility_state;

            constexpr u8 VISIBILITY_BIT = visibility_system::VISIBILITY_BIT;
            constexpr u8 CHANGE_BIT = visibility_system::CHANGE_BIT;


            instance& i = static_cast<instance&>(owner);
            auto connections = i.get_connections();


            for (const auto& conn : connections) {
                auto visibility_view = visibility->visibility_for(conn->get_remote_id());


                // Handle entities going in and out of visibility. This automatically covers newly added and removed entities.
                add_del_entity_message added, removed;
                for (auto entity : visibility_view) {
                    u8 entity_visibility = u8(visibility_view.template get<vis_state>(entity));

                    if (entity_visibility & CHANGE_BIT) {
                        auto& ctr = (entity_visibility & VISIBILITY_BIT) ? added : removed;
                        ctr.entities.push_back(entity);
                    }
                }


                if (!added.entities.empty()) conn->send_message(core_message_types::MSG_ADD_ENTITY, added);
                if (!removed.entities.empty()) conn->send_message(core_message_types::MSG_DEL_ENTITY, removed);
            }


            // For every synchronized component that was removed, send a message to each remote that can see the entity.
            for (const auto& conn : connections) {
                auto visibility_view =
                    visibility->visibility_for(conn->get_remote_id()) |
                    removed_components.view()                         |
                    view;


                compound_message msg;
                const auto msg_id = conn->get_local_mtr().get_type(core_message_types::MSG_DEL_COMPONENT).id;

                for (auto entity : visibility_view) {
                    if (!(visibility_view.template get<vis_state>(entity) & VISIBILITY_BIT)) continue;


                    for (const auto& component : visibility_view.template get<removed_component_list>(entity).components) {
                        msg.push_message(
                            msg_id,
                            del_component_message { .component_type = component, .entity = entity },
                            conn.get()
                        );
                    }
                }


                if (!msg.empty()) conn->send_message(core_message_types::MSG_COMPOUND, msg);
            }

            removed_components.clear();


            // For every synchronized component, send a message to each remote that can see the entity.
            // TODO: Handle partially synchronized components.
            // TODO: If a component is visible to multiple remotes, it should only be serialized once.
            for (const auto& conn : connections) {
                auto visibility_view = visibility->visibility_for(conn->get_remote_id()) | view;


                compound_message msg;
                const auto msg_id = conn->get_local_mtr().get_type(core_message_types::MSG_SET_COMPONENT).id;

                for (auto entity : view) {
                    if (!(visibility_view.template get<vis_state>(entity) & VISIBILITY_BIT)) continue;


                    Synchronized::foreach([&] <typename Component> () {
                        const auto* component = owner.template try_get_component<Component>(entity);
                        if (!component) return;


                        // Check if component changed since last sync.
                        auto serialized_component = serialize::to_bytes(*component);

                        auto& prev_value_hash = owner.template get_component<detail::component_sync_shadow<Component>>(entity);
                        auto  curr_value_hash = XXH64(serialized_component.data(), serialized_component.size(), 0);
                        if (curr_value_hash == prev_value_hash.hash) return;


                        // Synchronize and update value hash.
                        msg.push_message(
                            msg_id,
                            set_component_message {
                                .data           = std::move(serialized_component),
                                .component_type = type_hash<Component>(),
                                .entity         = entity
                            },
                            conn.get()
                        );

                        prev_value_hash.hash = curr_value_hash;
                    });
                }


                if (!msg.empty()) conn->send_message(core_message_types::MSG_COMPOUND, msg);
            }
        }


        VE_GET_VAL(visibility);
    private:
        // Components that were removed from entities since the last update.
        struct removed_component_list { small_vector<u64, 4> components; };
        storage_group<removed_component_list> removed_components;
        std::array<event_handler_id_t, Synchronized::size> on_component_added_handlers, on_component_removed_handlers;

        // System that manages the visibility for each entity for each remote.
        visibility_system* visibility;

        u16 priority;
    };
}