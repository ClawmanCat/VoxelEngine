#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entity/entity_traits.hpp>
#include <VoxelEngine/ecs/storage/registry/entity_lifetime_manager.hpp>
#include <VoxelEngine/ecs/view/query/query_index.hpp>
#include <VoxelEngine/utility/container/type_erased_pointer.hpp>


namespace ve::ecs {
    /**
     * Manages query indices used to speed up queries into a registry.
     * @tparam EntityTraits The traits for entities managed by the registry.
     */
    template <entity_traits EntityTraits> class query_index_manager {
    public:
        using self_type     = query_index_manager<EntityTraits>;
        using entity_type   = typename EntityTraits::type;
        using entity_traits = EntityTraits;
        using entity_utils  = entity_utils<EntityTraits>;


        /**
         * Declares that the set of entities matched by Q1 is a superset of the entities matched by Q2,
         * and that views for Q2 should use the index on Q1 as view controller, if such an index exists.
         * @param q1 A query that always matches a superset of entities matched by q2.
         * @param q2 A query that always matches a subset of entities matched by q1.
         */
        template <typename Q1, typename Q2> void declare_superset(Q1 superset, Q2 subset) {
            supersets[type_index<Q2>()].push_back(type_index<Q1>());
        }


        /**
         * Adds an index on the given query, if one does not already exist.
         * @tparam Derived Derived registry class. This function is wrapped by the registry to bind this parameter.
         * @param query The query to create an index on.
         * @return True if an index was created or false if an index already existed.
         */
        template <typename Derived, typename Q> bool add_index_for_query(Q query) {
            static_assert(std::is_base_of_v<self_type, Derived>, "Provided derived class does not inherit from this class");


            if (!has_index_on_query(query)) {
                using query_index_t = query::query_index<Q, EntityTraits>;


                auto [it, success] = query::detail::query_accessed_components<Q>::apply([&] <typename... Cs> {
                    return indices.emplace(
                        type_index<Q>(),
                        query_index_data {
                            .index           = type_erased_pointer { query_index_t { static_cast<Derived&>(*this) } },
                            .on_create       = [] (void* index, entity_type entt) { ((query_index_t*) index)->on_entity_created(entt);   },
                            .on_destroy      = [] (void* index, entity_type entt) { ((query_index_t*) index)->on_entity_destroyed(entt); },
                            .component_types = { type_index<Cs>()... }
                        }
                    );
                });


                query::detail::query_accessed_components<Q>::foreach([&] <typename C> {
                    on_changed[type_index<C>()].push_back(query_component_change_method {
                        .index    = it->second.get(),
                        .function = [] (void* index, entity_type entt, bool present) {
                            ((query_index_t*) index)->template on_component_presence_changed<C>(entt, present);
                        }
                    });
                });


                return true;
            }


            return false;
        }


        /**
         * Removes the index on the given query, if one exists.
         * @param query The query to remove an index from.
         * @return True if the index was removed or false if there was no index on the query.
         */
        template <typename Q> bool remove_index_for_query(Q query) {
            if (auto it = indices.find(type_index<Q>()); it != indices.end()) {
                query::detail::query_accessed_components<Q>::foreach([&] <typename C> {
                    auto& component_methods = on_changed.at(type_index<C>());

                    auto method_it = ranges::find(
                        component_methods,
                        it->second.index.get(),
                        &query_component_change_method::index
                    );

                    component_methods.erase(method_it);
                });

                indices.erase(it);


                return true;
            }


            return false;
        }


        /** Returns whether or not there is an index on the given query or a declared superset of that query. */
        template <typename Q> [[nodiscard]] bool has_index_for_query(Q query) const {
            if (indices.contains(type_index<Q>())) return true;

            if (auto it = supersets.find(type_index<Q>()); it != supersets.end()) {
                return ranges::any_of(it->second, [&] (const auto& id) { return indices.contains(id); });
            }

            return false;
        }


        /** Callback for when an entity is created in the registry. */
        void on_create(entity_type entt) {
            for (auto& [query, index] : indices) {
                std::invoke(index.on_create, index.index.get(), entt);
            }
        }


        /** Callback for when an entity is removed from the registry. */
        void on_destroy(entity_type entt) {
            for (auto& [query, index] : indices) {
                std::invoke(index.on_destroy, index.index.get(), entt);
            }
        }


        /** Callback for when a component is added to an entity in the registry. */
        template <typename Component> void on_component_added(entity_type entt) {
            for (auto& change_handler : on_changed.at(type_index<Component>())) {
                std::invoke(change_handler.function, change_handler.index, entt, true);
            }
        }


        /** Callback for when a component is removed from an entity in the registry. */
        template <typename Component> void on_component_removed(entity_type entt) {
            for (auto& change_handler : on_changed.at(type_index<Component>())) {
                std::invoke(change_handler.function, change_handler.index, entt, false);
            }
        }


        template <typename Q> [[nodiscard]] const basic_sparse_set<entity_traits>& get_entities_for_indexed_query(Q query) const {
            return indices
                .at(key_for_query<Q>())
                .index
                .template as<query::query_index<Q, EntityTraits>>()
                .get_matched_entities();
        }
    private:
        // Type-erased query index, on_create and on_destroy callbacks and a list of component types.
        struct query_index_data {
            using entity_change_fn = fn<void, void*, entity_type>;
            using view_size_fn     = fn<std::size_t, const void*>;

            type_erased_pointer index;
            entity_change_fn on_create, on_destroy;
            view_size_fn size;
            small_vector<type_index_t, 8> component_types;
        };


        // For a given index and component type, the index callback method to add or remove the component.
        struct query_component_change_method {
            using component_change_fn = fn<void, void*, entity_type, bool>;

            void* index;
            component_change_fn function;
        };


        hash_map<type_index_t, query_index_data> indices;
        hash_map<type_index_t, std::vector<query_component_change_method>> on_changed;
        hash_map<type_index_t, std::vector<type_index_t>> supersets;


        /** If Q has an index, returns the type-ID of Q, otherwise returns the type-ID of the smallest superset of Q. */
        template <typename Q> [[nodiscard]] type_index_t key_for_query(void) const {
            constexpr type_index_t qi = type_index<Q>();

            if (indices.contains(qi)) {
                return qi;
            }

            if (auto it = supersets.find(qi); it != supersets.end()) {
                auto smallest = ranges::min_element(
                    it->second,
                    ranges::less { },
                    [&] (const auto& index) {
                        if (auto it = indices.find(index); it != indices.end()) {
                            return it->second.size(it->second.index.get());
                        }

                        return max_value<std::size_t>;
                    }
                );

                return *smallest;
            }

            VE_DEBUG_ASSERT(false, "No index present for query {}.", typename_of<Q>());
            VE_UNREACHABLE;
        }
    };
}