#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/system/system_entity_visibility.hpp>
#include <VoxelEngine/clientserver/instance.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message_setup.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_compound.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_set_component.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_del_component.hpp>
#include <VoxelEngine/utility/stack_polymorph.hpp>
#include <VoxelEngine/utility/traits/function_traits.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>


namespace ve {
    template <
        meta::pack_of_types Synchronized,
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<>,
        typename VisibilitySystem = system_entity_visibility<>
    > requires (
        Synchronized::all([] <typename Component> { return serialize::is_serializable<Component>; }) &&
        Synchronized::all([] <typename Component> { return !requires { typename Component::non_synchronizable_tag; }; }) &&
        requires { typename VisibilitySystem::system_entity_visibility_tag; }
    ) class system_synchronizer : public system<
        system_synchronizer<Synchronized, RequiredTags, ExcludedTags, VisibilitySystem>,
        meta::pack<create_empty_view_tag>
    > {
    private:
        template <typename Component> struct sync_cache_component {
            std::vector<u8> data;


        };

        template <typename Component> struct sync_cache_up_to_date_component {};

        // Wrapper around bool types to avoid conflicts with views that also include a bool type from a registry component.
        struct bool_wrapper { bool value; };

    public:
        using system_synchronizer_tag = void;
        using synchronized_types      = Synchronized;
        using vis_status              = std::add_const_t<typename VisibilitySystem::visibility_status>;


        explicit system_synchronizer(VisibilitySystem& visibility_system, nanoseconds default_sync_rate = nanoseconds{1s} / 30, u16 priority = priority::LOWEST) :
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


            const auto now = steady_clock::now();
            auto synchronize_now = create_filled_array<Synchronized::size>([&] (std::size_t i) {
                return (now - last_sync[i]) >= sync_rates[i];
            });


            for (auto& connection : instance.get_connections()) {
                auto vis_for_conn = visibility_system->visibility_for_remote(connection->get_remote_id());


                compound_message msg;


                Synchronized::foreach_indexed([&] <typename Component, std::size_t Index> {
                    if (!synchronize_now[Index]) return;


                    auto perform_update = [&] (auto& view) {
                        update_serialized_values<Component>(owner, view);
                        add_changes_to_message<Component>(connection.get(), msg, owner, view);
                        add_removals_to_message<Component>(connection.get(), msg, owner, view);
                        remove_destroyed_component_data<Component>(owner);
                    };


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
                Synchronized::foreach_indexed([&] <typename Component, std::size_t Index> {
                    if (!synchronize_now[Index]) return;
                    remove_destroyed_component_data<Component>(owner);
                });
            }


            // Clear the cache-up-to-date tags, since they won't be up to date the next time the system is ran.
            Synchronized::foreach([&] <typename Component> {
                owner.template remove_all_components<sync_cache_up_to_date_component<Component>>();
            });


            // Update timestamps for synchronized components.
            for (std::size_t i = 0; i < Synchronized::size; ++i) {
                if (synchronize_now[i]) last_sync[i] = now;
            }
        }


        template <typename Component> requires Synchronized::template contains<Component>
        void set_sync_rate(nanoseconds interval) {
            sync_rates[Synchronized::template find<Component>] = interval;
        }


        // Adds a rule to determine synchronization of the given component on a per-entity and per-instance basis.
        template <
            typename Rule,
            typename Component = std::remove_cvref_t<typename meta::function_traits<Rule>::arguments::template get<3>>,
            meta::pack_of_types RuleRequiredTags = meta::pack<>,
            meta::pack_of_types RuleExcludedTags = meta::pack<>
        > requires (
            std::is_invocable_r_v<bool, Rule, instance_id, registry&, entt::entity, const Component&>
            // If required tags are excluded by the system, the rule would never match anything.
            // !ExcludedTags::any([] <typename E> { return RuleRequiredTags::template contains<E>; }) &&
            // If excluded tags are required by the system, the rule would never match anything.
            // !RequiredTags::any([] <typename R> { return RuleExcludedTags::template contains<R>; })
        ) void add_per_entity_rule(Rule rule) {
            per_entity_rules[Synchronized::template find<Component>].emplace_back(
                rule_storage<Rule, Component, RuleRequiredTags, RuleExcludedTags> { std::move(rule) }
            );
        }
    private:
        u16 priority;

        VisibilitySystem* visibility_system;

        std::array<nanoseconds, Synchronized::size> sync_rates;
        std::array<steady_clock::time_point, Synchronized::size> last_sync;


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
        std::array<small_vector<rule_storage_poly_t, 1>, Synchronized::size> per_entity_rules;


        // Given an entity and a visibility view (possibly with a per-entity-rule bool_wrapper exclusion component),
        // Returns whether or not the given entity is synchronized,
        static bool is_synchronized(const auto& entity, const auto& view) {
            using view_t = std::remove_cvref_t<decltype(view)>;

            // Skip entities excluded by a per-entity-rule.
            if constexpr (view_traits<view_t>::component_types::template contains<bool_wrapper>) {
                if (!view.template get<bool_wrapper>(entity).value) return false;
            }

            // Skip entities that are not visible.
            if (!(view.template get<vis_status>(entity) & VisibilitySystem::VISIBILITY_BIT)) return false;

            return true;
        }


        // Serialize all component values that are visible to the remote and have not yet been serialized.
        template <typename Component> void update_serialized_values(registry& owner, auto visibility_view) {
            // Serialize all components that have not yet been serialized ever.
            auto view_unserialized = visibility_view | owner.template view_pack<
                typename RequiredTags::template append<Component>,
                typename ExcludedTags::template append<sync_cache_component<Component>>
            >();

            for (auto entity : view_unserialized) {
                if (!is_synchronized(entity, view_unserialized)) continue;

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
                if (!is_synchronized(entity, view_out_of_date)) continue;

                const auto& cmp = view_out_of_date.template get<Component>(entity);
                auto& cache     = view_out_of_date.template get<sync_cache_component<Component>>(entity);

                cache.data.clear();
                serialize::to_bytes(cmp, cache.data);

                owner.set_component(entity, sync_cache_up_to_date_component<Component> { });
            }
        }


        // Add the data about which (visible) components were changed to the provided message.
        template <typename Component> void add_changes_to_message(message_handler* connection, compound_message& msg, registry& owner, auto visibility_view) {
            auto view_synchronized = visibility_view | owner.template view_pack<
                typename RequiredTags::template append<sync_cache_component<Component>>,
                ExcludedTags
            >();

            for (auto entity : view_synchronized) {
                if (!is_synchronized(entity, view_synchronized)) continue;

                const static mtr_id id = get_core_mtr_id(core_message_types::MSG_SET_COMPONENT);
                const auto& cache = view_synchronized.template get<sync_cache_component<Component>>(entity);

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
        template <typename Component> void add_removals_to_message(message_handler* connection, compound_message& msg, registry& owner, auto visibility_view) {
            // If the component was synced before and it has been removed since then, it will still have a cache.
            auto view_removed = visibility_view | owner.template view_pack<
                typename RequiredTags::template append<sync_cache_component<Component>>,
                typename ExcludedTags::template append<Component>
            >();

            for (auto entity : view_removed) {
                if (!is_synchronized(entity, view_removed)) continue;

                const static mtr_id id = get_core_mtr_id(core_message_types::MSG_DEL_COMPONENT);
                const auto& cache = view_removed.template get<sync_cache_component<Component>>(entity);

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