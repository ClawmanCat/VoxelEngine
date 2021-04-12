#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/side.hpp>
#include <VoxelEngine/ecs/system/system_view_type.hpp>

#include <entt/entt.hpp>


namespace ve {
    template <
        typename Derived,
        side Side,
        meta::type_pack ComponentTypes,
        meta::type_pack ExcludedComponentTypes = meta::pack<>
    > struct system {
    public:
        constexpr static inline side side = Side;
        using components     = ComponentTypes;
        using excluded       = ExcludedComponentTypes;
        using most_derived_t = Derived;
        using view_type      = typename detail::generate_view_type<components, excluded>::type;
        using ecs_system_tag = void;
        
        
        void update(view_type& view, microseconds dt) {
            VE_CRTP_CHECK(Derived, update);
            static_cast<Derived*>(this)->update(view, dt);
        }
    };
}