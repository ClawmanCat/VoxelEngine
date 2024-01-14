#pragma once

#include <VoxelEngine/ecs/storage/registry/registry_traits.hpp>
#include <VoxelEngine/ecs/view/ecs_view.hpp>
#include <VoxelEngine/ecs/view/component_storage_view.hpp>
#include <VoxelEngine/ecs/view/query/query_building_blocks.hpp>
#include <VoxelEngine/ecs/view/query/query_view_builder.hpp>
#include <VoxelEngine/utility/meta/is_template.hpp>
#include <VoxelEngine/utility/meta/common_concepts.hpp>


namespace ve::ecs {
    template <registry_traits Traits> class basic_registry :
        public Traits::entity_manager,
        public Traits::component_manager,
        public Traits::system_manager,
        public Traits::query_indexer
    {
    public:
        using self_type           = basic_registry<Traits>;
        using entity_traits       = typename Traits::entity_traits;
        using entity_type         = typename entity_traits::type;
        using entity_utils        = entity_utils<entity_traits>;
        using entity_manager      = typename Traits::entity_manager;
        using component_manager   = typename Traits::component_manager;
        using system_manager      = typename Traits::system_manager;
        using query_index_manager = typename Traits::query_indexer;


        /** Constructs a registry by default constructing all base classes. */
        basic_registry(void) = default;

        /** Constructs a registry from a set of tuples by constructing each base class from the contents of a tuple. */
        basic_registry(
            meta::constructor_tuple_for<entity_manager>      auto entity_manager_args,
            meta::constructor_tuple_for<component_manager>   auto component_manager_args,
            meta::constructor_tuple_for<system_manager>      auto system_manager_args,
            meta::constructor_tuple_for<query_index_manager> auto query_indexer_args
        ) :
            entity_manager     (std::apply([&] (auto&&... args) { return entity_manager      { fwd(args)... }; }, std::move(entity_manager_args))),
            component_manager  (std::apply([&] (auto&&... args) { return component_manager   { fwd(args)... }; }, std::move(component_manager_args))),
            system_manager     (std::apply([&] (auto&&... args) { return system_manager      { fwd(args)... }; }, std::move(system_manager_args))),
            query_index_manager(std::apply([&] (auto&&... args) { return query_index_manager { fwd(args)... }; }, std::move(query_indexer_args)))
        {}


        /**
         * Registries should have a constant address.
         * TODO: This is currently required because the system_invoke_wrapper stores a pointer to the registry, this should be avoidable.
         */
        VE_IMMOVABLE(basic_registry);


        /**
         * Creates a new entity with the given components and returns its ID.
         * @param components Components to add to the newly created entity.
         * @return The ID of the newly created entity.
         */
        template <typename... Components>
        [[nodiscard]] entity_type create(Components... components) {
            const entity_type entt = entity_manager::create();

            query_index_manager::on_create(entt);
            component_manager::template add_components<Components...>(entt, std::move(components)...);

            return entt;
        }


        /**
         * Creates a new entity with the given components and returns its ID.
         * Each element of 'args' should be a tuple that can be used to construct one of the components.
         * @tparam Components The types of the components to add to the newly created entity.
         * @param args One tuple of constructor arguments for each component to construct it with.
         * @return The ID of the newly created entity.
         */
        template <typename... Components, meta::constructor_tuple_for<Components>... Args>
        [[nodiscard]] entity_type create(std::piecewise_construct_t, Args&&... args) {
            const entity_type entt = entity_manager::create();

            query_index_manager::on_create(entt);
            component_manager::template add_components<Components...>(std::piecewise_construct, entt, fwd(args)...);

            return entt;
        }


        /**
         * Creates a new entity with the given ID and the given components, if the given ID is available to use.
         * @param entt The ID to create the new entity with.
         * @param components Components to add to the newly created entity.
         * @return True if the entity was created or false if the entity ID was not available.
         */
        template <typename... Components>
        [[nodiscard]] bool create_with_id(entity_type entt, Components... components) {
            const bool success = entity_manager::create_with_id(entt);

            if (success) [[likely]] {
                query_index_manager::on_create(entt);
                component_manager::template add_components<Components...>(entt, std::move(components)...);
            }

            return success;
        }


        /**
         * Creates a new entity with the given ID and the given components, if the given ID is available to use.
         * Each element of 'args' should be a tuple that can be used to construct one of the components.
         * @tparam Components The types of the components to add to the newly created entity.
         * @param entt The ID to create the new entity with.
         * @param args One tuple of constructor arguments for each component to construct it with.
         * @return True if the entity was created or false if the entity ID was not available.
         */
        template <typename... Components, meta::constructor_tuple_for<Components>... Args>
        [[nodiscard]] bool create_with_id(std::piecewise_construct_t, entity_type entt, Args&&... args) {
            const bool success = entity_manager::create_with_id(entt);

            if (success) [[likely]] {
                query_index_manager::on_create(entt);
                component_manager::template add_components<Components...>(std::piecewise_construct, entt, fwd(args)...);
            }

            return success;
        }


        /**
         * Destroys the entity with the given ID and all of its components, if it exists.
         * @param entt The ID of the entity to destroy.
         * @return True if the entity existed or false if it did not.
         */
        bool destroy(entity_type entt) {
            const bool success = entity_manager::destroy(entt);

            if (success) [[likely]] {
                query_index_manager::on_destroy(entt);
                component_manager::on_destroy(entt);
            }

            return success;
        }


        /**
         * Returns a view of all entities that have the given components.
         * Results in a view with a value type of std::tuple<Entity, [const] Components&...>.
         * @tparam Components The components to create a view of.
         * @return A view of all entities that have the given components and said components.
         */
        template <typename... Components> auto view(void) {
            if constexpr (sizeof...(Components) == 1) return component_storage_view { std::addressof(component_manager::template get_pool<Components...>()) };
            else return this->query((query::has<Components> && ... && query::true_query));
        }


        /** @copydoc view */
        template <typename... Components> auto view(void) const {
            if constexpr (sizeof...(Components) == 1) return component_storage_view { std::addressof(component_manager::template get_pool<Components...>()) };
            else return this->query((query::has<Components> && ... && query::true_query));
        }


        /**
         * Returns a view of all entities matched by the query.
         * Results in a view with a value_type of std::tuple<Entity, [const] IncludedComponents&..., [const] OptionalComponents*...>
         *  where 'IncludedComponents' is each component type that is always included by the query
         *  where 'OptionalComponents' is each component type that is conditionally included by the query
         * E.g. performing a query such as:
         * ~~~
         * using namespace ve::ecs::query;
         * registry.query(has<TypeA> && has<TypeB> && (has<TypeC> || has<TypeD>) && !has<TypeE>);
         * ~~~
         * will result in a view with a value_type std::tuple<Entity, [const] TypeA&, [const] TypeB&, [const] TypeC*, [const] TypeD*>.
         * @param query An ecs_query to create a view from.
         * @return A view over the entities matched by the query and the relevant components.
         */
        template <query::ecs_query Q> auto query(Q query) {
            return query::build_query_view(query, *this);
        }


        /** @copydoc view */
        template <query::ecs_query Q> auto query(Q query) const {
            return query::build_query_view(query, *this);
        }


        // Exposed interface from entity_manager.
        using entity_manager::has_existed;
        using entity_manager::is_alive;
        using entity_manager::is_dead;
        using entity_manager::get_entities;
        using entity_manager::get_tombstones;


        // Exposed interface from component_manager.
        using component_manager::add_component;
        using component_manager::add_components;
        using component_manager::remove_component;
        using component_manager::remove_components;
        using component_manager::take_component;
        using component_manager::take_components;
        using component_manager::has_component;
        using component_manager::has_all;
        using component_manager::has_any;
        using component_manager::has_which;
        using component_manager::get_component;
        using component_manager::try_get_component;


        // Exposed interface from query_index_manager.
        using query_index_manager::declare_superset;
        using query_index_manager::add_index_for_query;
        using query_index_manager::remove_index_for_query;
        using query_index_manager::has_index_for_query;
        using query_index_manager::get_entities_for_indexed_query;


        // Exposed interface from SystemManager.
        using system_manager::update;
        using system_manager::remove_system;
        using system_manager::take_system;
        using system_manager::get_system;
        using system_manager::has_system;
        using system_manager::get_tick;
        using system_manager::get_current_dt;
        using system_manager::get_min_dt;
        using system_manager::get_max_dt;
        using system_manager::set_min_dt;
        using system_manager::set_max_dt;

        template <typename System> std::pair<System&, system_id> add_system(System system) {
            return system_manager::template add_system<self_type, System>(std::move(system));
        }
    };


    using registry = basic_registry<default_registry_traits>;
}