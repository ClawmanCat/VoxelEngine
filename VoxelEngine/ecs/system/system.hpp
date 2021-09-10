#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/ecs/view_type.hpp>

#include <entt/entt.hpp>


namespace ve {
    template <
        typename Derived,
        meta::pack_of_types RequiredComponents,
        meta::pack_of_types ExcludedComponents = meta::pack<>
    > struct system {
        using required_components = RequiredComponents;
        using excluded_components = ExcludedComponents;
        using expanded_pack_type  = detail::expanded_t<RequiredComponents, ExcludedComponents>;
        using view_type           = view_type<RequiredComponents, ExcludedComponents>;
        
        
        void update(view_type& view, microseconds dt) {
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