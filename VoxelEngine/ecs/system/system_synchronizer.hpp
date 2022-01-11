#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/system/system_entity_visibility.hpp>
#include <VoxelEngine/clientserver/instance.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message_setup.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_compound.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_set_component.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_del_component.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>


namespace ve {
    // TODO: Move entity addition / removal logic to separate system.
    template <
        meta::pack_of_types Synchronized,
        typename VisibilitySystem,
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<>
    > requires (
        Synchronized::all([] <typename Component> { return serialize::is_serializable<Component>; }) &&
        Synchronized::all([] <typename Component> { return !requires { typename Component::non_synchronizable_tag; }; }) &&
        requires { typename VisibilitySystem::system_entity_visibility_tag; }
    ) class system_synchronizer : public system<
        system_synchronizer<Synchronized, VisibilitySystem, RequiredTags, ExcludedTags>,
        meta::pack<create_empty_view_tag>
    > {
    private:
        template <typename Component> struct sync_cache_component {
            std::vector<u8> data;
        };

        template <typename Component> struct sync_cache_up_to_date_component {};

    public:
        explicit system_synchronizer(VisibilitySystem& visibility_system, nanoseconds default_sync_rate = 1s / 30, u16 priority = priority::LOWEST) :
            priority(priority),
            visibility_system(&visibility_system),
            sync_rates(create_filled_array<Synchronized::size>(produce(default_sync_rate))),
            last_sync(create_filled_array<Synchronized::size>(produce(epoch_time<steady_clock::time_point>())))
        {
            VE_ASSERT(
                get_priority() < visibility_system.get_priority(),
                "Entity visibility should be updated before components are synchronized."
            );
        }


        u16 get_priority(void) const {
            return priority;
        }


        void update(registry& owner, view_type view, nanoseconds dt) {
            auto& instance = static_cast<class instance&>(owner);
            u64 tick = instance.get_tick_count();


            auto synchronize_now = create_filled_array<Synchronized::size>([&] (std::size_t i) {
                return time_since(last_sync[i]) >= sync_rates[i];
            });


            for (auto& connection : instance.get_connections()) {
                using vis_status   = std::add_const_t<typename VisibilitySystem::visibility_status>;
                auto  vis_for_conn = visibility_system->visibility_for_remote(connection->get_remote_id());


                compound_message msg;


                Synchronized::foreach_indexed([&] <typename Component, std::size_t Index> {
                    if (!synchronize_now[Index]) return;


                    // Serialize all components that have not yet been serialized ever.
                    auto view_unserialized = vis_for_conn | owner.template view_except<
                        typename RequiredTags::template append<Component>,
                        typename ExcludedTags::template append<sync_cache_component<Component>>
                    >();

                    for (auto entity : view_unserialized) {
                        if (!(view_unserialized.template get<vis_status>(entity) & VisibilitySystem::VISIBILITY_BIT)) continue;

                        const auto& cmp = view_unserialized.template get<Component>(entity);

                        owner.set_component(entity, sync_cache_component<Component> { serialize::to_bytes(cmp) });
                        owner.set_component(entity, sync_cache_up_to_date_component<Component> { });
                    }


                    // Serialize all components that have been synchronized before but are out-of-date.
                    // Splitting this from the unserialized entities prevents unnecessary heap allocations from overwriting the cache component.
                    auto view_out_of_date = vis_for_conn | owner.template view_except<
                        typename RequiredTags::template append<Component, sync_cache_component<Component>>,
                        typename ExcludedTags::template append<sync_cache_up_to_date_component<Component>>
                    >();

                    for (auto entity : view_out_of_date) {
                        if (!(view_out_of_date.template get<vis_status>(entity) & VisibilitySystem::VISIBILITY_BIT)) continue;

                        const auto& cmp = view_out_of_date.template get<Component>(entity);
                        auto& cache     = view_out_of_date.template get<sync_cache_component<Component>>(entity);

                        cache.data.clear();
                        serialize::to_bytes(cmp, cache.data);

                        owner.set_component(entity, sync_cache_up_to_date_component<Component> { });
                    }


                    // Add the updated component data to the message.
                    // We don't have to bother checking the constraints here, since the existence of the cache already implies these are met.
                    auto view_synchronized = vis_for_conn | owner.template view<sync_cache_component<Component>>();

                    for (auto entity : view_synchronized) {
                        if (!(view_synchronized.template get<vis_status>(entity) & VisibilitySystem::VISIBILITY_BIT)) continue;

                        const static mtr_id id = get_core_mtr_id(core_message_types::MSG_SET_COMPONENT);
                        const auto& cache = view_synchronized.template get<sync_cache_component<Component>>(entity);

                        msg.push_message(
                            id,
                            set_component_message {
                                .component_data = cache.data, // TODO: Elude this copy!
                                .component_type = type_hash<Component>(),
                                .entity         = entity
                            },
                            connection.get()
                        );
                    }


                    // Add any removed components to the message.
                    // (If the component was synced before and it has been removed since then, it will still have a cache.)
                    auto view_removed = vis_for_conn | owner.template view_except<
                        typename RequiredTags::template append<sync_cache_component<Component>>,
                        typename ExcludedTags::template append<Component>
                    >();

                    for (auto entity : view_removed) {
                        if (!(view_removed.template get<vis_status>(entity) & VisibilitySystem::VISIBILITY_BIT)) continue;

                        const static mtr_id id = get_core_mtr_id(core_message_types::MSG_DEL_COMPONENT);
                        const auto& cache = view_removed.template get<sync_cache_component<Component>>(entity);

                        msg.push_message(
                            id,
                            del_component_message {
                                .component_type = type_hash<Component>(),
                                .entity         = entity
                            },
                            connection.get()
                        );
                    }
                });


                Synchronized::foreach_indexed([&] <typename Component, std::size_t Index> {
                    if (!synchronize_now[Index]) return;


                    // For any components that were just updated, we can remove the cache if the component was deleted,
                    // as we already sent out the del_component_messages.
                    auto view_removed = owner.template view_except<
                        typename RequiredTags::template append<sync_cache_component<Component>>,
                        typename ExcludedTags::template append<Component>
                    >();

                    owner.template remove_all_components<sync_cache_component<Component>>(view_removed);


                    // The sync cache should be updated again the next time the system is ran.
                    owner.template remove_all_components<sync_cache_up_to_date_component<Component>>();
                });


                connection->send_message(core_message_types::MSG_COMPOUND, msg);
            }


            // Clear the cache-up-to-date tags, since they won't be up to date the next time the system is ran.
            Synchronized::foreach([&] <typename Component> {
                owner.template remove_all_components<sync_cache_up_to_date_component<Component>>();
            });
        }


        template <typename Component> requires Synchronized::template contains<Component>
        void set_sync_rate(nanoseconds interval) {
            sync_rates[Synchronized::template find<Component>] = interval;
        }
    private:
        u16 priority;

        VisibilitySystem* visibility_system;

        std::array<nanoseconds, Synchronized::size> sync_rates;
        std::array<steady_clock::time_point, Synchronized::size> last_sync;
    };
}