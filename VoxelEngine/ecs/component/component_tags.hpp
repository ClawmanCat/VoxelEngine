#pragma once

#include <VoxelEngine/core/core.hpp>

#include <VoxelEngine/ecs/entt_include.hpp>


namespace ve {
    class registry;
}


// These traits check for tags that can be added to components to change their behaviour in the ECS.
namespace ve::component_tags {
    // Checks if the component can be synchronized using system_synchronizer, assuming it is serializable.
    template <typename Component> constexpr static bool is_synchronizable_v =
        !requires { typename Component::non_synchronizable_tag; };

    // Checks if a component can be removed from an entity once it is added.
    template <typename Component> constexpr static bool is_removable_v =
        !requires { typename Component::non_removable_tag; };

    // Checks if a component should have a constant address while it is assigned to an entity.
    template <typename Component> constexpr static bool has_constant_address_v =
        requires { typename Component::constant_address_tag; };

    // Checks if the component has a callback to be invoked when it is added to an entity.
    template <typename Component> constexpr static bool has_added_callback_v =
        requires (Component c, registry& r, entt::entity e) { c.on_component_added(r, e); };

    // Checks if the component has a callback to be invoked when it is removed from an entity.
    template <typename Component> constexpr static bool has_removed_callback_v =
        requires (Component c, registry& r, entt::entity e) { c.on_component_removed(r, e); };
}