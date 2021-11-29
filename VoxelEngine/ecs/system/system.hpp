#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/ecs/view_type.hpp>
#include <VoxelEngine/utility/priority.hpp>

#include <entt/entt.hpp>


namespace ve {
    class registry;


    template <
        typename Derived,
        meta::pack_of_types RequiredComponents,
        meta::pack_of_types ExcludedComponents = meta::pack<>
    > struct system {
        using required_components = RequiredComponents;
        using excluded_components = ExcludedComponents;
        using view_type           = view_type_for<RequiredComponents, ExcludedComponents>;
        using ecs_system_tag      = void;


        u16 get_priority(void) const {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, get_priority)) {
                return static_cast<const Derived*>(this)->get_priority();
            } else {
                return priority::NORMAL;
            }
        }


        void init(registry& storage) {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, init)) {
                static_cast<Derived*>(this)->init(storage);
            }
        }


        void uninit(registry& storage) {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, init)) {
                static_cast<Derived*>(this)->uninit(storage);
            }
        }


        void update(registry& owner, view_type view, nanoseconds dt) {
            VE_CRTP_CHECK(Derived, update);
            static_cast<Derived*>(this)->update(owner, view, dt);
        }

        
        static view_type make_view(entt::registry& registry) {
            return construct_view<required_components, excluded_components>(registry);
        }
    };
}