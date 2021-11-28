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
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    struct dont_synchronize_by_default_tag {};


    // Broadcasts changes to the ECS to remote instances.
    // This system should be coupled to a system_set_visibility.
    // TODO: Handle per-component synchronization rates.
    template <
        meta::pack_of_types Synchronized,
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<dont_synchronize_by_default_tag>,
        typename VisibilityPred = fn<bool, registry&, entt::entity, message_handler*>
    > class system_synchronizer : public system<
        system_synchronizer<Synchronized, RequiredTags, ExcludedTags>,
        RequiredTags,
        ExcludedTags
    > {
    public:
        using visibility_system = system_set_visibility<VisibilityPred>;


        explicit system_synchronizer(visibility_system* visibility, u16 priority = priority::LOWEST) :
            visibility(visibility),
            priority(priority)
        {}


        u16 get_priority(void) const {
            return priority;
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


            // For every synchronized component, send a message to each remote that can see the entity.
            // TODO: Only send changed components. Use second registry to store value hashes?
            // TODO: Handle partially synchronized components.
            for (const auto& conn : connections) {
                auto visibility_view = visibility->visibility_for(conn->get_remote_id()) | view;


                compound_message msg;
                const auto msg_id = conn->get_local_mtr().get_type(core_message_types::MSG_SET_COMPONENT).id;

                for (auto entity : view) {
                    Synchronized::foreach([&] <typename Component> () {
                        msg.push_message(
                            msg_id,
                            set_component_message {
                                .data           = detail::get_component<Component>(owner, entity),
                                .component_type = type_hash<Component>(),
                                .entity         = entity
                            },
                            conn.get()
                        );
                    });
                }


                conn->send_message(core_message_types::MSG_COMPOUND, msg);
            }


            // TODO: Also send a list of components that have been removed.
        }


        VE_GET_VAL(visibility);
    private:
        visibility_system* visibility;
        registry::system_id visibility_id;

        u16 priority;
    };
}