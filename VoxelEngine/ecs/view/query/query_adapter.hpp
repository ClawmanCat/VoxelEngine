#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view/query/query_building_blocks.hpp>
#include <VoxelEngine/ecs/view/query/query_view.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>


namespace ve::ecs::query {
    /**
     * Transforms a @ref query_view to have a value type of the given included and optional components.
     * @tparam Include Components that should be included in the value type of the returned query.
     * @tparam Optional Components that should be included as optional pointers in the value type of the returned query.
     * @param query A query to transform.
     * @return A query that iterates the same entities as the provided query, but has a value type defined by 'Include' and 'Optional'.
     */
    template <meta::type_pack Include, meta::type_pack Optional, ecs_query Q> requires (
        detail::query_included_components<Q>::template contains_all<Include> &&
        detail::query_optional_components<Q>::template contains_all<Optional>
    ) inline auto adapt_query(Q query) {
        return [&] <typename... I, typename... O> (meta::pack<I...>, meta::pack<O...>) {
            return query_view {
                (has_all<I...> && (has_any<O...> || true_query)) || true_query,
                query,
                detail::query_view_access_facade::get_views(query)
            };
        } (Include {}, Optional {});
    }
}