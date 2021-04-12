#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/side.hpp>
#include <VoxelEngine/utility/traits/pack.hpp>

#include <entt/entt.hpp>


namespace ve::detail {
    template <typename... Components> struct view_type {
        template <typename... Excluded> struct with_exclusions {
            using type = entt::basic_view<
                entt::entity,
                entt::exclude_t<Excluded...>,
                Components...
            >;
            
            
            constexpr static inline auto registry_getter = [](entt::registry& r) {
                return r.view<Components...>(entt::exclude_t<Excluded...>{});
            };
        };
        
        
        template <meta::type_pack Excluded> using generate_with_exclusions =
        typename Excluded::template expand_inside<with_exclusions>;
    };
    
    
    template <meta::type_pack Components, meta::type_pack Excluded>
    using generate_view_type = typename Components
        ::template expand_inside<view_type>
        ::template generate_with_exclusions<Excluded>;
}