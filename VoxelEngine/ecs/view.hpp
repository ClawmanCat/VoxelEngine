#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/traits/maybe_const.hpp>

#include <VoxelEngine/ecs/entt_include.hpp>


namespace ve {
    namespace detail {
        template <typename... I, typename... E>
        inline auto get_component_view_type(meta::pack<I...>, meta::pack<E...>) -> entt::basic_view<entt::entity, entt::get_t<I...>, entt::get_t<>, entt::exclude_t<E...>> {
            return {};
        }


        template <typename... I, typename... E>
        inline auto get_registry_view_type(meta::pack<I...>, meta::pack<E...>) -> decltype(std::declval<entt::registry>().template view<I...>(entt::exclude<E...>)) {
            return {};
        }
    }


    template <typename Component>
    using storage_type = entt::storage<Component>;


    // The type of a view for the given component types.
    template <meta::pack_of_types Included, meta::pack_of_types Excluded>
    using component_view_type = decltype(detail::get_component_view_type(Included {}, Excluded {}));


    // Equivalent to component_view_type, unless Included is empty, in which case this is an exclude-only view type
    // for the storage type of the registry.
    template <meta::pack_of_types Included, meta::pack_of_types Excluded>
    using registry_view_type = decltype(detail::get_registry_view_type(Included {}, Excluded {}));


    template <meta::pack_of_types Included, meta::pack_of_types Excluded = meta::pack<>>
    inline auto view_from_registry(meta::maybe_const<entt::registry> auto& registry) {
        return [&] <typename... I, typename... E> (meta::pack<I...>, meta::pack<E...>) {
            return registry.template view<I...>(entt::exclude<E...>);
        } (Included {}, Excluded {});
    }


    inline auto view_from_set(meta::maybe_const<entt::sparse_set> auto& set) {
        using set_t = std::remove_reference_t<decltype(set)>;
        return entt::basic_view<entt::entity, entt::get_t<>, entt::get_t<set_t>, entt::exclude_t<>> { set };
    }


    // Can't use meta::maybe_const here since the type wouldn't be deducible.
    template <typename Component> inline auto view_from_storage(storage_type<Component>& storage) {
        return entt::basic_view<entt::entity, entt::get_t<Component>, entt::get_t<>, entt::exclude_t<>> { storage };
    }

    template <typename Component> inline auto view_from_storage(const storage_type<Component>& storage) {
        return entt::basic_view<entt::entity, entt::get_t<const Component>, entt::get_t<>, entt::exclude_t<>> { storage };
    }


    template <typename> struct view_traits {};

    template <typename Entity, typename... Components, typename... Included, typename... Excluded>
    struct view_traits<entt::basic_view<Entity, entt::get_t<Components...>, entt::get_t<Included...>, entt::exclude_t<Excluded...>>> {
        using entity_type     = Entity;
        using component_types = meta::pack<Components...>;
        using included_types  = meta::pack<Included...>;
        using excluded_types  = meta::pack<Excluded...>;
    };
}