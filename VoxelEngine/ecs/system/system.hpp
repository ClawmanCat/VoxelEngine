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
        meta::pack_of_types ExcludedComponents = meta::pack<>,
        u16 Priority = priority::NORMAL
    > struct system {
        constexpr static inline u16 priority = Priority;

        using required_components = RequiredComponents;
        using excluded_components = ExcludedComponents;
        using view_type           = view_type_for<RequiredComponents, ExcludedComponents>;
        using ecs_system_tag      = void;


        void init(registry& storage) {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, init)) {
                static_cast<Derived*>(this)->init(storage);
            }
        }


        void update(view_type view, nanoseconds dt) {
            VE_CRTP_CHECK(Derived, update);
            static_cast<Derived*>(this)->update(view, dt);
        }

        
        static view_type make_view(entt::registry& registry) {
            return [&] <typename... Required, typename... Excluded> (meta::pack<Required...>, meta::pack<Excluded...>) {
                return registry.view<Required...>(entt::exclude_t<Excluded...>{});
            }(RequiredComponents{}, ExcludedComponents{});
        }
    };
}