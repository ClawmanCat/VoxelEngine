#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/traits/maybe_const.hpp>

#include <entt/entt.hpp>


namespace ve {
    namespace detail {
        // ENTT does not support views without any components. Some systems may require these however.
        // To resolve this, this empty component is added to all entities, and can be used instead of an empty view.
        struct common_component {
            using non_removable_tag = void;
            using non_syncable_tag  = void;
        };


        template <typename... Required, typename... Excluded>
        constexpr auto view_type_impl(meta::pack<Required...>, meta::pack<Excluded...>)
            -> std::conditional_t<
                (sizeof...(Required) == 0),
                entt::basic_view<entt::entity, entt::exclude_t<Excluded...>, common_component>,
                entt::basic_view<entt::entity, entt::exclude_t<Excluded...>, Required...>
            >;
    }


    template <meta::pack_of_types RequiredTypes, meta::pack_of_types ExcludedTypes>
    using view_type_for = decltype(detail::view_type_impl(RequiredTypes{}, ExcludedTypes{}));


    template <meta::pack_of_types RequiredTypes, meta::pack_of_types ExcludedTypes>
    inline view_type_for<RequiredTypes, ExcludedTypes> construct_view(meta::maybe_const<entt::registry> auto& registry) {
        return [&] <typename... Required> (meta::pack<Required...>) {
            return [&] <typename... Excluded> (meta::pack<Excluded...>) {
                if constexpr (RequiredTypes::size == 0) {
                    return registry.template view<detail::common_component>(entt::exclude_t<Excluded...> { });
                } else {
                    return registry.template view<Required...>(entt::exclude_t<Excluded...> { });
                }
            } (ExcludedTypes { });
        } (RequiredTypes { });
    }
}