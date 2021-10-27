#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>

#include <entt/entt.hpp>


namespace ve {
    namespace detail {
        template <typename... Required, typename... Excluded>
        constexpr auto view_type_impl(meta::pack<Required...>, meta::pack<Excluded...>)
        -> entt::basic_view<entt::entity, entt::exclude_t<Excluded...>, Required...>;
    }
    
    template <meta::pack_of_types RequiredTypes, meta::pack_of_types ExcludedTypes>
    using view_type_for = decltype(detail::view_type_impl(RequiredTypes{}, ExcludedTypes{}));
}