#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>
#include <VoxelEngine/utility/traits/function_traits.hpp>

#include <entt/entt.hpp>


namespace ve {
    class registry;


    enum class change_result : u8 { ALLOWED, DENIED, UNOBSERVABLE };


    // Given a remote instance wishing to make a change to some component, the change_validator checks if the remote
    // is allowed to make said change, based on user defined rules.
    class change_validator {
    public:
        // "allow_by_default" sets the behaviour when there is no rule for a given component.
        explicit change_validator(bool allow_by_default = true) : allow_by_default(allow_by_default) {}


        // Add a rule for the given component type. Type is automatically deduced from the parameters of the rule.
        // Note that either 'old_value' or 'new_value' may be null, indicating the addition or removal of a component from the registry respectively.
        // If there are multiple rules for a given component, they must all return true for an action to be allowed.
        template <typename Rule, typename Component = std::remove_const_t<std::remove_pointer_t<typename meta::function_traits<Rule>::arguments::template get<3>>>>
        requires std::is_invocable_r_v<bool, Rule, instance_id, registry&, entt::entity, const Component*, const Component*>
        void add_rule(Rule&& rule) {
            rules[type_hash<Component>()].push_back([rule = fwd(rule)] (instance_id remote, registry& r, entt::entity e, const void* old_value, const void* new_value) {
                return std::invoke(rule, remote, r, e, (const Component*) old_value, (const Component*) new_value);
            });
        }


        template <typename Component>
        bool is_allowed(instance_id remote, registry& r, entt::entity e, const Component* old_value, const Component* new_value) const {
            auto it = rules.find(type_hash<Component>());
            if (it == rules.end()) return allow_by_default;

            for (const auto& rule : it->second) {
                if (!std::invoke(rule, remote, r, e, old_value, new_value)) return false;
            }

            return true;
        }
    private:
        using type_erased_rule = std::function<bool(instance_id, registry&, entt::entity, const void*, const void*)>;

        hash_map<u64, std::vector<type_erased_rule>> rules;
        bool allow_by_default;
    public:
        VE_GET_SET_VAL(allow_by_default);
    };
}