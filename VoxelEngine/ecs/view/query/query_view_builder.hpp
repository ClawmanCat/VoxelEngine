#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view/sparse_set_view.hpp>
#include <VoxelEngine/ecs/view/component_storage_view.hpp>
#include <VoxelEngine/ecs/view/joined_entity_view.hpp>
#include <VoxelEngine/ecs/view/query/query_building_blocks.hpp>
#include <VoxelEngine/ecs/view/query/query_utils.hpp>
#include <VoxelEngine/ecs/view/query/query_view.hpp>


namespace ve::ecs::query {
    namespace detail {
        template <typename EntityTraits> const inline basic_sparse_set<EntityTraits> empty_set {};


        /** Façade for accessing registry component pools directly. */
        struct direct_pool_access {
            template <typename C, typename R> static auto& get_pool(R& pool_manager) {
                return pool_manager.template get_pool<C>();
            }
        };


        /** Constructs a view controller for a query view matching the given query. */
        template <typename Q, typename R> inline auto build_query_view_controller(Q query, R& registry) {
            using common_set_type = const basic_sparse_set_common_base<typename R::entity_traits>;


            // If there is an index for this query, use that.
            // Since this is not a constexpr-if branch we have to return the same type as if there was no index,
            // which is joined_entity_view if there are conditionally-included components or sparse_set_view<common_set_type> otherwise.
            if (registry.has_index_for_query(query)) {
                const auto& index = registry.get_entities_for_indexed_query(query);

                if constexpr (query_included_components<Q>::empty && is_closed_set<Q>() && !query_optional_components<Q>::empty) {
                    // Pad the rest of the view with empty subviews so it is the same size.
                    // TODO: Consider de-templatizing joined_entity_view so this isn't necessary.
                    return joined_entity_view { [&] <std::size_t... Is> (std::index_sequence<Is...>) {
                        return std::tuple {
                            sparse_set_view { (common_set_type*) std::addressof(index) },
                            [&] (std::size_t) {
                                return sparse_set_view { (common_set_type*) std::addressof(detail::empty_set<typename R::entity_traits>) };
                            } (Is)...
                        };
                    } (std::make_index_sequence<(query_optional_components<Q>::size - 1)>()) };
                } else {
                    return sparse_set_view { (common_set_type*) std::addressof(index) };
                }
            }


            // If there are components that are always included, find the smallest such component pool.
            if constexpr (!query_included_components<Q>::empty) {
                common_set_type* smallest_set;
                std::size_t smallest_size = max_value<std::size_t>;

                query_included_components<Q>::foreach([&] <typename C> {
                    const auto& pool = detail::direct_pool_access::get_pool<C>(registry);
                    const auto size  = pool.size();

                    if (size < smallest_size) {
                        smallest_size = size;
                        smallest_set  = std::addressof(pool.get_entities());
                    }
                });

                return sparse_set_view { smallest_set };
            }

            // If there are conditionally included components, and at least one must be present,
            // join all conditionally-included components into the view controller.
            else if constexpr (!query_optional_components<Q>::empty && is_closed_set<Q>()) {
                return joined_entity_view {
                    query_optional_components<Q>::apply([&] <typename... Cs> {
                        return std::tuple {
                            sparse_set_view {
                                ((common_set_type*) std::addressof(detail::direct_pool_access::get_pool<Cs>(registry).get_entities()))
                            }...
                        };
                    })
                };
            }

            // Otherwise just use the entire registry as the controller.
            else return sparse_set_view { (common_set_type*) std::addressof(registry.get_entities()) };
        }
    }


    /** Given a registry and a query, constructs a view for the given query over entities in the given registry. */
    template <typename Q, typename R> inline auto build_query_view(Q query, R& registry) {
        return query_view {
            query,
            detail::build_query_view_controller(query, registry),
            detail::query_accessed_components<Q>::apply([&] <typename... Cs> {
                return std::tuple {
                    component_storage_view { std::addressof(detail::direct_pool_access::get_pool<Cs>(registry)) }...
                };
            })
        };
    }
}