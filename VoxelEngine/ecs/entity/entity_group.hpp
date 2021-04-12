#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>

#include <entt/entt.hpp>

#include <type_traits>


namespace ve {
    namespace detail {
        template <typename Component> using entity_group_storage_base =
            entt::sigh_storage_mixin<
                entt::storage_adapter_mixin<
                    entt::storage<Component>
                >
            >;
    }
    
    
    // An entity group provides a view over a set of entities and their associated components.
    // This can be used for objects that have some group of entities associated with them,
    // like chunks or the parent of some hierarchy.
    // Unlike with components, new groups can be created at runtime.
    template <typename Derived, typename BackingComponent = meta::null_type>
    struct entity_group : public detail::entity_group_storage_base<BackingComponent> {
        using entity_group_tag = void;
        
        
        auto view(void) {
            return entt::view<
                entt::exclude_t<>,
                BackingComponent
            >{ *this };
        }
    
    
        auto view(void) const {
            return entt::view<
                entt::exclude_t<>,
                BackingComponent
            >{ *this };
        }
    };
}