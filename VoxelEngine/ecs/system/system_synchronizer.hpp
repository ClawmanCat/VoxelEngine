#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/system/system_entity_visibility.hpp>
#include <VoxelEngine/ecs/component/component_tags.hpp>
#include <VoxelEngine/clientserver/instance.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message_setup.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_compound.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_set_component.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_del_component.hpp>
#include <VoxelEngine/utility/stack_polymorph.hpp>
#include <VoxelEngine/utility/traits/function_traits.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>

#include <xxhash.h>


namespace ve {
    namespace detail {
        template <typename T> struct is_partially_synchronizable {
            constexpr static bool value = requires { typename T::partially_synchronizable_tag; };
        };
    }


    // Synchronizes the given components with any remote instances connected to the one managing the system.
    // Synchronized component types must either be serializable, in which case they are synchronized directly,
    // or must implement the partially_synchronized interface, in which case the system will ensure the components'
    // callbacks are invoked when required.
    // The provided visibility system decides what entities have their components synchronized with which remotes.
    // Additionally, per-entity-rules can be provided which decide if a component is synchronized on a per-entity and a
    // per-remote basis.
    // Each component has an associated synchronization interval. The component is also always synchronized when it
    // first becomes visible to a remote.
    template <
        meta::pack_of_types Synchronized,
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<>,
        typename VisibilitySystem = system_entity_visibility<>,
        template <typename System> typename... Mixins
    > requires (
        Synchronized::all([] <typename Component> { return serialize::is_serializable<Component>; }) &&
        Synchronized::all([] <typename Component> { return component_tags::is_synchronizable_v<Component>; }) &&
        requires { typename VisibilitySystem::system_entity_visibility_tag; }
    ) class system_synchronizer : public system<
        system_synchronizer<Synchronized, RequiredTags, ExcludedTags, VisibilitySystem, Mixins...>,
        create_empty_view,
        meta::pack<>,
        // While we should technically declare sync_cache related components here, that would require forward declaring them,
        // which is impossible for nested classes. Fortunately, it does not matter, since they are private, there are no risks
        // of concurrency issues with other systems and two instances of the synchronizer cannot run concurrently,
        // since it is not marked as safe to do so.
        meta::pack_ops::merge_all<Synchronized, RequiredTags, ExcludedTags>,
        Mixins...
    > {
    private:
        // TODO: Sync cache component should persist between ticks so we can skip sending values that have not changed.
        template <typename Component> struct sync_cache_component {
            std::vector<u8> data;
            bool changed = true;
        };

        template <typename Component> struct sync_cache_up_to_date_component {};

        // Wrapper around bool types to avoid conflicts with views that also include a bool type from a registry component.
        struct bool_wrapper { bool value; };

    public:
        using system_synchronizer_tag = void;
        using synchronized_types      = Synchronized;
        using partially_synced_types  = typename Synchronized::template filter_trait<detail::is_partially_synchronizable>;
        using vis_status              = std::add_const_t<typename VisibilitySystem::visibility_status>;


        explicit system_synchronizer(VisibilitySystem& visibility_system, nanoseconds default_sync_rate = nanoseconds{1s} / 30, u16 priority = priority::LOWEST) :
            priority(priority),
            visibility_system(&visibility_system),
            sync_rates(create_filled_array<synchronized_types::size>(produce(default_sync_rate))),
            last_sync(create_filled_array<synchronized_types::size>(produce(epoch_time<steady_clock::time_point>())))
        {
            VE_ASSERT(
                get_priority() < visibility_system.get_priority(),
                "Entity visibility should be updated before components are synchronized."
            );
        }


        template <typename Component> constexpr static u8 access_mode_for_component(void) {
            return (u8) system_access_mode::READ_CMP;
        }


        u16 get_priority(void) const {
            return priority;
        }


        void on_system_added(registry& owner) {
            VE_DEBUG_ASSERT(
                dynamic_cast<class instance*>(&owner),
                "Registry must be part of an instance in order to use a synchronization system."
            );
        }


        void on_system_update(registry& owner, view_type view, nanoseconds dt) {
            auto& instance = static_cast<class instance&>(owner);
            u64 tick = instance.get_tick_count();


            const auto now = steady_clock::now();
            auto synchronize_now = create_filled_array<synchronized_types::size>([&] (std::size_t i) {
                return (now - last_sync[i]) >= sync_rates[i];
            });


            for (auto& connection : instance.get_connections()) {
                auto vis_for_conn = visibility_system->visibility_for_remote(connection->get_remote_id());


                compound_message msg;


                synchronized_types::foreach_indexed([&] <typename Component, std::size_t Index> {
                    auto perform_update = [&] (auto& view) {
                        bool sync_timer_elapsed = synchronize_now[Index];

                        update_serialized_values<Component>(owner, view, sync_timer_elapsed);
                        add_changes_to_message<Component>(connection.get(), msg, owner, view, sync_timer_elapsed);
                        add_removals_to_message<Component>(connection.get(), msg, owner, view, sync_timer_elapsed);
                        remove_destroyed_component_data<Component>(owner);
                    };


                    // Filter out any entities that are excluded by a rule, then serialize and synchronize.
                    if (!per_entity_rules[Index].empty()) {
                        storage_type<bool_wrapper> included_entities { };
                        included_entities.insert(vis_for_conn.begin(), vis_for_conn.end(), bool_wrapper { true });

                        for (const auto& rule : per_entity_rules[Index]) {
                            rule->update_sync_list(owner, connection->get_remote_id(), included_entities);
                        }

                        auto view_with_rules = view_from_storage(included_entities) | vis_for_conn;
                        perform_update(view_with_rules);
                    } else {
                        perform_update(vis_for_conn);
                    }
                });


                if (!msg.empty()) connection->send_message(core_message_types::MSG_COMPOUND, msg);


                // Remove cached values for components that no longer exist.
                synchronized_types::foreach_indexed([&] <typename Component, std::size_t Index> {
                    if (!synchronize_now[Index]) return;
                    remove_destroyed_component_data<Component>(owner);
                });


                partially_synced_types::foreach([&] <typename Component> {
                    invoke_ps_callbacks<Component>(connection.get(), owner, vis_for_conn);
                });
            }


            // Clear the cache-up-to-date tags, since they won't be up to date the next time the system is ran.
            synchronized_types::foreach([&] <typename Component> {
                owner.template remove_all_components<sync_cache_up_to_date_component<Component>>();
            });


            // Update timestamps for synchronized components.
            for (std::size_t i = 0; i < synchronized_types::size; ++i) {
                if (synchronize_now[i]) last_sync[i] = now;
            }
        }


        template <typename Component> requires synchronized_types::template contains<Component>
        void set_sync_rate(nanoseconds interval) {
            sync_rates[synchronized_types::template find<Component>] = interval;
        }


        // Adds a rule to determine synchronization of the given component on a per-entity and per-instance basis.
        template <
            typename Rule,
            typename Component = meta::nth_argument_base<Rule, 3>,
            meta::pack_of_types RuleRequiredTags = meta::pack<>,
            meta::pack_of_types RuleExcludedTags = meta::pack<>
        > requires (
            std::is_invocable_r_v<bool, Rule, instance_id, registry&, entt::entity, const Component&> &&
            // If required tags are excluded by the system, the rule would never match anything.
            meta::pack_ops::intersection<ExcludedTags, RuleRequiredTags>::size == 0 &&
            // If excluded tags are required by the system, the rule would never match anything.
            meta::pack_ops::intersection<RequiredTags, RuleExcludedTags>::size == 0
        ) void add_per_entity_rule(Rule rule) {
            per_entity_rules[synchronized_types::template find<Component>].emplace_back(
                rule_storage<Rule, Component, RuleRequiredTags, RuleExcludedTags> { std::move(rule) }
            );
        }
    private:
        u16 priority;

        VisibilitySystem* visibility_system;

        std::array<nanoseconds, synchronized_types::size> sync_rates;
        std::array<steady_clock::time_point, synchronized_types::size> last_sync;


        struct rule_storage_base {
            virtual ~rule_storage_base(void) = default;
            virtual void update_sync_list(registry& owner, instance_id remote, storage_type<bool_wrapper>& storage) const = 0;
        };

        template <typename Rule, typename Component, meta::pack_of_types Required, meta::pack_of_types Excluded>
        struct rule_storage : rule_storage_base {
            Rule rule;

            explicit rule_storage(Rule rule) : rule(std::move(rule)) {}

            void update_sync_list(registry& owner, instance_id remote, storage_type<bool_wrapper>& storage) const override {
                auto view = view_from_storage(storage) | owner.template view_pack<typename Required::template append<Component>, Excluded>();

                for (auto entity : view) {
                    view.template get<bool_wrapper>(entity).value &= std::invoke(rule, remote, owner, entity, view.template get<Component>(entity));
                }
            }
        };

        using rule_storage_poly_t = stack_polymorph<rule_storage_base, sizeof(rule_storage_base) + 32>;
        std::array<small_vector<rule_storage_poly_t, 1>, synchronized_types::size> per_entity_rules;


        // Given an entity and a visibility view (possibly with a per-entity-rule bool_wrapper exclusion component),
        // Returns whether or not the given entity is synchronized,
        static bool is_synchronized(const auto& entity, const auto& view, bool sync_timer_elapsed) {
            using view_t = std::remove_cvref_t<decltype(view)>;

            // Skip entities excluded by a per-entity-rule.
            if constexpr (view_traits<view_t>::component_types::template contains<bool_wrapper>) {
                if (!view.template get<bool_wrapper>(entity).value) return false;
            }

            // Skip entities that are not visible.
            auto visibility = view.template get<vis_status>(entity);
            if (!(visibility & VisibilitySystem::VISIBILITY_BIT)) return false;

            // If the entity just became visible, always synchronize it, otherwise wait until the timer has elapsed.
            return visibility == VisibilitySystem::BECAME_VISIBLE ? true : sync_timer_elapsed;
        }


        // Construct new partially-synchronized components on remotes and invoke callbacks on the local instance of the component.
        template <typename Component> void invoke_ps_callbacks(message_handler* connection, registry& owner, auto visibility_view) {
            auto view = visibility_view | owner.template view_pack<typename RequiredTags::template append<Component>, ExcludedTags>();

            for (auto entity : view) {
                const auto visibility = view.template get<vis_status>(entity);

                if (visibility == vis_status::BECAME_VISIBLE) {
                    ((typename Component::ps_base_type&) view.template get<Component>(entity)).on_added_to_remote(connection->get_remote_id());
                }

                else if (visibility == vis_status::BECAME_INVISIBLE) {
                    ((typename Component::ps_base_type&) view.template get<Component>(entity)).on_removed_from_remote(connection->get_remote_id());
                }
            }
        }


        // Serialize all component values that are visible to the remote and have not yet been serialized.
        template <typename Component> void update_serialized_values(registry& owner, auto visibility_view, bool sync_timer_elapsed) {
            // Serialize all components that have not yet been serialized ever.
            auto view_unserialized = visibility_view | owner.template view_pack<
                typename RequiredTags::template append<Component>,
                typename ExcludedTags::template append<sync_cache_component<Component>>
            >();

            for (auto entity : view_unserialized) {
                if (!is_synchronized(entity, view_unserialized, sync_timer_elapsed)) continue;

                const auto& cmp = view_unserialized.template get<Component>(entity);

                owner.set_component(entity, sync_cache_component<Component> { serialize::to_bytes(cmp) });
                owner.set_component(entity, sync_cache_up_to_date_component<Component> { });
            }


            // Serialize all components that have been synchronized before but are out-of-date.
            // Splitting this from the unserialized entities prevents unnecessary heap allocations from overwriting the cache component.
            auto view_out_of_date = visibility_view | owner.template view_pack<
                typename RequiredTags::template append<Component, sync_cache_component<Component>>,
                typename ExcludedTags::template append<sync_cache_up_to_date_component<Component>>
            >();

            for (auto entity : view_out_of_date) {
                if (!is_synchronized(entity, view_out_of_date, sync_timer_elapsed)) continue;

                const auto& cmp = view_out_of_date.template get<Component>(entity);
                auto& cache     = view_out_of_date.template get<sync_cache_component<Component>>(entity);
                auto old_hash   = XXH64(cache.data.data(), cache.data.size(), 0);

                cache.data.clear();
                serialize::to_bytes(cmp, cache.data);

                auto new_hash = XXH64(cache.data.data(), cache.data.size(), 0);
                cache.changed = (old_hash != new_hash);

                owner.set_component(entity, sync_cache_up_to_date_component<Component> { });
            }
        }


        // Add the data about which (visible) components were changed to the provided message.
        template <typename Component> void add_changes_to_message(message_handler* connection, compound_message& msg, registry& owner, auto visibility_view, bool sync_timer_elapsed) {
            auto view_synchronized = visibility_view | owner.template view_pack<
                typename RequiredTags::template append<sync_cache_component<Component>>,
                ExcludedTags
            >();

            for (auto entity : view_synchronized) {
                if (!is_synchronized(entity, view_synchronized, sync_timer_elapsed)) continue;

                const static mtr_id id = get_core_mtr_id(core_message_types::MSG_SET_COMPONENT);
                const auto& cache = view_synchronized.template get<sync_cache_component<Component>>(entity);

                // If the component didn't change and the remote already has the most up-to-date value, we don't have to synchronize it again.
                if (!cache.changed && !(view_synchronized.template get<vis_status>(entity) & VisibilitySystem::CHANGED_BIT)) continue;

                msg.push_message(
                    id,
                    set_component_message {
                        .component_data = cache.data, // TODO: Elude this copy!
                        .component_type = type_hash<Component>(),
                        .entity         = entity
                    },
                    connection
                );
            }
        }


        // Add the data about which (visible) components were removed to the provided message.
        template <typename Component> void add_removals_to_message(message_handler* connection, compound_message& msg, registry& owner, auto visibility_view, bool sync_timer_elapsed) {
            // If the component was synced before and it has been removed since then, it will still have a cache.
            auto view_removed = visibility_view | owner.template view_pack<
                typename RequiredTags::template append<sync_cache_component<Component>>,
                typename ExcludedTags::template append<Component>
            >();

            for (auto entity : view_removed) {
                if (!is_synchronized(entity, view_removed, sync_timer_elapsed)) continue;


                const static mtr_id id = get_core_mtr_id(core_message_types::MSG_DEL_COMPONENT);

                msg.push_message(
                    id,
                    del_component_message {
                        .component_type = type_hash<Component>(),
                        .entity         = entity
                    },
                    connection
                );
            }
        }


        // Remove caches from the registry for components that have been removed.
        template <typename Component> void remove_destroyed_component_data(registry& owner) {
            // If the component was synced before and it has been removed since then, it will still have a cache.
            auto view_removed = owner.template view_pack<
                typename RequiredTags::template append<sync_cache_component<Component>>,
                typename ExcludedTags::template append<Component>
            >();

            owner.template remove_all_components<sync_cache_component<Component>>(view_removed);
        }
    };
}