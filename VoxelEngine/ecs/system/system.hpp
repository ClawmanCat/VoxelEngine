#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/priority.hpp>

#include <entt/entt.hpp>


namespace ve {
    class registry;


    struct create_empty_view_tag {};


    template <
        typename Derived,
        meta::pack_of_types RequiredComponents = meta::pack<>,
        meta::pack_of_types ExcludedComponents = meta::pack<>
    > struct system {
        using required_components = RequiredComponents;
        using excluded_components = ExcludedComponents;
        using view_type           = registry_view_type<required_components, excluded_components>;
        using ecs_system_tag      = void;


        u16 get_priority(void) const {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, get_priority)) {
                return static_cast<const Derived*>(this)->get_priority();
            } else {
                return priority::NORMAL;
            }
        }


        // Called after the system is added to the registry.
        // Note: the engine guarantees the address of the system remains constant between calls to init() and uninit().
        void init(registry& storage) {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, init)) {
                static_cast<Derived*>(this)->init(storage);
            }
        }


        // Called before the system is removed from the registry.
        void uninit(registry& storage) {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, uninit)) {
                static_cast<Derived*>(this)->uninit(storage);
            }
        }


        void update(registry& owner, view_type view, nanoseconds dt) {
            VE_CRTP_CHECK(Derived, update);
            static_cast<Derived*>(this)->update(owner, view, dt);
        }

        
        static view_type make_view(entt::registry& registry) {
            return view_from_registry<required_components, excluded_components>(registry);
        }
    };
}