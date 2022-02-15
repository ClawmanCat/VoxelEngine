#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>
#include <VoxelEngine/utility/algorithm.hpp>
#include <VoxelEngine/utility/traits/function_traits.hpp>
#include <VoxelEngine/utility/traits/remove_const_pointer.hpp>

#include <VoxelEngine/ecs/entt_include.hpp>


namespace ve {
    class registry;


    enum class change_result : u8 {
        ALLOWED      = 0,   // The remote may make this change.
        FORBIDDEN    = 1,   // The remote may not make this change.
        UNOBSERVABLE = 2    // The remote should not have visibility of the entity that owns this component, or it does not exist.
    };


    // When a change to some component is received from a remote instance, the change validator is queried to see if said change is allowed.
    // The change validator contains rules, which must all return true in order for a change to succeed.
    // When no rules are provided for a given component, the 'allowed_by_default' setting is used to determine if a change should succeed, fail or be unobservable.
    // This setting can be set on both a global and a per-component basis.
    // An optional entity visibility system can be added to automatically reject changes to entities which the remote cannot see.
    class change_validator {
    public:
        using rule_handle = std::size_t;


        change_validator(void) = default;
        explicit change_validator(change_result allow_by_default) : default_behaviour(allow_by_default) {}


        // Checks whether the given change to the component is allowed under the rules of the change validator.
        // old_value and new_value may be null if a component is respectively added or removed.
        template <typename Component> change_result is_allowed(instance_id remote, const registry& registry, entt::entity entity, const Component* old_value, const Component* new_value) const {
            // If we have a visibility system, the entity needs to be visible to the remote for the change to succeed.
            if (check_visibility && !std::invoke(check_visibility, entity, remote)) {
                return change_result::UNOBSERVABLE;
            }


            if (auto it = rules.find(type_hash<Component>()); it != rules.end()) {
                // If there are no rules for the given type, use the default behaviour.
                if (it->second.rules_for_component.empty()) {
                    return (it->second.default_behaviour == per_component_data::NOT_SET)
                        ? default_behaviour
                        : (change_result) it->second.default_behaviour;
                }

                // Otherwise, make sure all rules pass.
                bool allowed = true;
                for (const auto& rule : it->second.rules_for_component) {
                    if (rule) [[likely]] allowed &= std::invoke(rule, remote, registry, entity, old_value, new_value);
                }

                return allowed ? change_result::ALLOWED : change_result::FORBIDDEN;
            } else return default_behaviour;
        }


        // Gets the default behaviour for the given component, that is, the behaviour when no rules have been provided.
        template <typename Component> change_result get_default_behaviour(void) const {
            if (auto it = rules.find(type_hash<Component>()); it != rules.end()) {
                return (it->second.default_behaviour == per_component_data::NOT_SET)
                    ? default_behaviour
                    : it->second.default_behaviour;
            } else return default_behaviour;
        }


        // Add a rule to the change validator. The component type is deduced from the function signature.
        // The rule must be invocable as rule(instance_id, registry&, entt::entity, const Component*, const Component*) and return bool,
        // where the last two parameters are the old value and the new value of the component respectively.
        // If a component is added or removed, the old value and the new value may be null respectively.
        template <typename Fn, typename Component = meta::remove_const_pointer<typename meta::function_traits<Fn>::arguments::template get<2>>>
        requires std::is_invocable_r_v<bool, Fn, instance_id, const registry&, entt::entity, const Component*, const Component*>
        rule_handle add_rule(Fn fn) {
            auto& value = rules[type_hash<Component>()];
            auto& rfc   = value.rules_for_component;

            auto wrapped_fn = [fn = std::move(fn)] (instance_id remote, const registry& r, entt::entity e, const void* old_value, const void* new_value) {
                return std::invoke(fn, remote, r, e, (const Component*) old_value, (const Component*) new_value);
            };

            rule_handle handle;
            if (value.tombstones.empty()) {
                handle = rfc.size();
                rfc.emplace_back(std::move(wrapped_fn));
            } else {
                handle = take(value.tombstones);
                rfc[handle] = std::move(wrapped_fn);
            }

            return handle;
        }


        // Equivalent to add_rule, but the component type must be explicitly provided.
        template <typename Component, typename Fn>
        requires std::is_invocable_r_v<bool, Fn, instance_id, const registry&, entt::entity, const Component*, const Component*>
        rule_handle add_rule_for(Fn fn) {
            return add_rule<Fn, Component>(std::move(fn));
        }


        template <typename Component> void remove_rule(rule_handle handle) {
            rules[type_hash<Component>()].rules_for_component[handle] = nullptr;
        }


        // Sets the default behaviour when no rules and no per-component allow-by-default value is provided.
        void allow_by_default(change_result allowed) {
            default_behaviour = allowed;
        }


        // Sets the behaviour for the given component when no rules are provided.
        // This overrides the global (for all components) value.
        template <typename Component> void allow_by_default(change_result value) {
            rules[type_hash<Component>()].default_behaviour = (per_component_data::default_behaviour_t) value;
        }


        // Clears the default behaviour for the given component, so that the global default will be used.
        template <typename Component> void clear_allow_default(void) {
            rules[type_hash<Component>()].default_behaviour = per_component_data::NOT_SET;
        }


        // Sets the visibility system to be used in conjunction with the change validator.
        // Changes to entities that are not visible to the remote will be automatically denied.
        // The provided value may be nullptr to clear any existing visibility system.
        template <typename System> requires requires { typename System::system_entity_visibility_tag; }
        void set_visibility_system(const System* system) {
            if (system) {
                check_visibility = [system] (entt::entity entity, instance_id remote) { return system->is_visible(entity, remote); };
            } else {
                check_visibility = nullptr;
            }
        }


        // Sets all synchronized components of the given synchronization system to use the given default behaviour.
        // When the global default behaviour is "unobservable", the behaviour for components visible to the remote is usually set
        // to "forbidden" instead, so that when a client makes a change to one of these components, the server can tell it
        // the action is not allowed.
        // Note: the provided system parameter is only used for type deduction and may be null.
        template <typename System> requires requires { typename System::system_synchronizer_tag; }
        void set_default_for_synced_components(const System* system = nullptr, change_result value = change_result::FORBIDDEN) {
            System::synchronized_types::foreach([&] <typename Component> {
                allow_by_default<Component>(value);
            });
        }
    private:
        struct per_component_data {
            std::vector<std::function<bool(instance_id, const registry&, entt::entity, const void*, const void*)>> rules_for_component;
            std::vector<rule_handle> tombstones;

            enum default_behaviour_t : u8 { ALLOWED = 0, FORBIDDEN = 1, UNOBSERVABLE = 2, NOT_SET = 3 } default_behaviour = NOT_SET;
        };

        hash_map<u64, per_component_data> rules;
        change_result default_behaviour = change_result::UNOBSERVABLE;
        std::function<bool(entt::entity, instance_id)> check_visibility = nullptr;
    };
}