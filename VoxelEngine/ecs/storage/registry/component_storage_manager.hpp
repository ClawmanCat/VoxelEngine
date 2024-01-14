#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entity/entity_traits.hpp>
#include <VoxelEngine/ecs/component/component_traits.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set_mixin.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage_mixin.hpp>
#include <VoxelEngine/utility/meta/value.hpp>
#include <VoxelEngine/utility/meta/common_concepts.hpp>
#include <VoxelEngine/utility/container/type_erased_pointer.hpp>


namespace ve::ecs {
    namespace query::detail { struct direct_pool_access; }


    namespace detail {
        /** Asserts template instantiations of CT fulfil the concept component_traits. */
        template <template <typename> typename CT>
        constexpr inline bool valid_component_traits_template = component_traits<CT<meta::placeholder_type>>;


        /** Asserts template instantiations of CM fulfil the concept component_storage_mixin_type. */
        template <template <typename> typename CM, typename ET, template <typename> typename CT>
        constexpr inline bool valid_component_mixin_template = component_storage_mixin_type<
            CM<meta::placeholder_type>,
            ET,
            CT<meta::placeholder_type>
        >;
    }


    /**
     * The component_storage_manager manages the components associated with entities in a registry.
     * Creation and destruction of entities is not handled by this class, for that see @ref entity_lifetime_manager.
     * @tparam EntityTraits Traits for the entities whose components are stored by this class.
     * @tparam ComponentTraits Traits-template for the components that are stored by this class.
     *  ComponentTraits<X> should be a valid @ref component_traits class for every component type X.
     * @tparam EntityMixin Mixin for sparse sets used within this class.
     * @tparam ComponentMixin Mixin template for component storage objects used within this class.
     *  ComponentMixin<X> should be a valid @ref component_storage_mixin_type for every component type X.
     */
    template <
        entity_traits EntityTraits,
        template <typename> typename ComponentTraits,
        sparse_set_mixin_type<EntityTraits> EntityMixin,
        template <typename> typename ComponentMixin
    > requires (
        detail::valid_component_traits_template<ComponentTraits> &&
        detail::valid_component_mixin_template<ComponentMixin, EntityTraits, ComponentTraits>
    ) class component_storage_manager {
    public:
        using entity_type   = typename EntityTraits::type;
        using entity_traits = EntityTraits;
        using entity_utils  = entity_utils<EntityTraits>;

        template <typename Component> using component_storage_type = basic_component_storage<
            EntityTraits,
            ComponentTraits<Component>,
            EntityMixin,
            ComponentMixin<Component>
        >;


        /** Destroys all components for the given entity. */
        void on_destroy(entity_type entt) {
            for (const auto& [key, pool] : pools) pool.on_entity_destroyed(entt);
        }


        /**
         * Adds the given component to the given entity.
         * @tparam Component The type of the component to add.
         * @param entt The entity to add the given component to.
         * @param args Arguments to construct the component with.
         * @return True if the component was added, or false if the entity already had the component.
         */
        template <typename Component, typename... Args> bool add_component(entity_type entt, Args&&... args) {
            auto [it, success] = get_pool<Component>().emplace(entt, fwd(args)...);
            return success;
        }


        /**
         * Adds the given components to the given entity.
         * @param entt The entity to add the given components to.
         * @param args Instances of the components to add to the given entity.
         * @return An array of booleans indicating if each component was successfully added.
         */
        template <typename... Components> requires meta::all_different<Components...>
        std::array<bool, sizeof...(Components)> add_components(entity_type entt, Components&&... args) {
            return std::array<bool, sizeof...(Components)> { add_component<Components>(fwd(args))... };
        }


        /**
         * Adds the given components to the given entity by constructing each component from one of the provided tuples.
         * This overload is selected by passing std::piecewise_construct as the first function argument.
         *
         * @tparam Components The component types to add to the given entity.
         * @param entt The entity to add the given component types to.
         * @param args A tuple of arguments for each component to construct it with.
         * @return An array of booleans indicating if each component was successfully added.
         */
        template <typename... Components, meta::constructor_tuple_for<Components>... Tuples> requires meta::all_different<Components...>
        std::array<bool, sizeof...(Components)> add_components(std::piecewise_construct_t, entity_type entt, Tuples&&... args) {
            using CS = meta::pack<Components...>;


            return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
                return std::array {
                    std::apply([&] (const auto&... args) {
                        return add_component<typename CS::template nth<Is>>(entt, fwd(args)...);
                    }, std::get<Is>(args))...
                };
            } (std::make_index_sequence<sizeof...(Components)>());
        }


        /**
         * Removes the given component from the given entity.
         * @tparam Component The type of the component to remove.
         * @param entt The entity to remove the component from.
         * @return True if the entity had the component before this function was called, false otherwise.
         */
        template <typename Component> bool remove_component(entity_type entt) {
            get_pool<Component>().erase(entt);
        }


        /**
         * Removes the given components from the given entity.
         * @tparam Components The types of the components to remove.
         * @param entt The entity to remove the components from.
         * @return An array of booleans with true for every component which was present before this function was called, or false otherwise.
         */
        template <typename... Components> std::array<bool, sizeof...(Components)> remove_components(entity_type entt) {
            return std::array { remove_component<Components>()... };
        }


        /**
         * Removes the given component from the given entity and returns it if its existed, or std::nullopt otherwise.
         * @tparam Component The type of the component to remove.
         * @param entt The entity to remove the component from.
         * @return The component as an std::optional if it existed, or std::nullopt otherwise.
         */
        template <typename Component> std::optional<Component> take_component(entity_type entt) {
            if (auto ptr = try_get_component<Component>(entt); ptr) {
                std::optional<Component> result = std::move(*ptr);
                remove_component<Component>(entt);
                return result;
            }

            return std::nullopt;
        }


        /**
         * Removes the given components from the given entity and returns them if they existed, or std::nullopt otherwise.
         * @tparam Components The types of the components to remove.
         * @param entt The entity to remove the component from.
         * @return A tuple with for each component type the component as an std::optional if it existed, or std::nullopt otherwise.
         */
        template <typename... Components> std::tuple<std::optional<Components>...> take_components(entity_type entt) {
            return std::tuple { take_component<Components>(entt)... };
        }


        /** Checks if the given entity has the given component. */
        template <typename Component> [[nodiscard]] bool has_component(entity_type entt) const {
            return has_pool<Component>() && get_pool<Component, true>().contains(entt);
        }


        /** Checks if the given entity has all of the given components. */
        template <typename... Components> [[nodiscard]] bool has_all(entity_type entt) const {
            return (has_component<Components>(entt) && ...);
        }


        /** Checks if the given entity has any of the given components. */
        template <typename... Components> [[nodiscard]] bool has_any(entity_type entt) const {
            return (has_component<Components>(entt) || ...);
        }


        /** Returns an array of booleans indicating which of the given components the entity has. */
        template <typename... Components> [[nodiscard]] std::array<bool, sizeof...(Components)> has_which(entity_type entt) const {
            return std::array { has_component<Components>()... };
        }


        /** Returns the component of the given type of the given entity. The entity must have the given component. */
        template <typename Component> Component& get_component(entity_type entt) {
            VE_DEBUG_ASSERT(
                has_component<Component>(entt),
                "Used get_component with component {} not present on entity {}.",
                typename_of<Component>(),
                entity_utils::entity_string(entt)
            );

            return get_pool<Component>()[entt];
        }


        /** @copydoc get_component */
        template <typename Component> const Component& get_component(entity_type entt) const {
            VE_DEBUG_ASSERT(
                has_component<Component>(entt),
                "Used get_component with component {} not present on entity {}.",
                typename_of<Component>(),
                entity_utils::entity_string(entt)
            );

            return get_pool<Component>()[entt];
        }


        /** Returns a pointer to the given component of the given entity if the entity has such a component, or nullptr otherwise. */
        template <typename Component> Component* try_get_component(entity_type entt) {
            return has_component<Component>()
                ? std::addressof(get_component<Component>(entt))
                : nullptr;
        }


        /** @copydoc try_get_component */
        template <typename Component> const Component* try_get_component(entity_type entt) const {
            return has_component<Component>()
                   ? std::addressof(get_component<Component>(entt))
                   : nullptr;
        }
    private:
        struct component_pool {
            type_erased_pointer storage;
            fn<void, type_erased_pointer&, entity_type> on_entity_destroyed;
        };

        mutable hash_map<type_index_t, component_pool> pools;

    protected:
        friend struct ve::ecs::query::detail::direct_pool_access;


        /** After calling this method, the pool for the given component type is guaranteed to exist, regardless of if it existed already. */
        template <typename Component> void assure_pool_exists(void) const {
            using pool_type = component_storage_type<Component>;


            if (!pools.contains(type_index<Component>())) [[unlikely]] {
                pools.emplace(
                    type_index<Component>(),
                    component_pool {
                        type_erased_pointer { meta::type<pool_type> { }, std::tuple { } },
                        [] (type_erased_pointer& pool, entity_type entt) { pool.as<pool_type>().erase(entt); }
                    }
                );
            }
        }


        /**
         * Returns the pool for the given component type.
         * @tparam Component The type of the component.
         * @tparam Unchecked If false, the pool will be created if it does not exist. If true, the pool is assumed to exist.
         * @return A reference to the pool for the given component type.
         */
        template <typename Component, bool Unchecked = false> [[nodiscard]] component_storage_type<Component>& get_pool(void) {
            if constexpr (!Unchecked) assure_pool_exists<Component>();
            return pools.at(type_index<Component>()).storage.template as<component_storage_type<Component>>();
        }


        /** @copydoc get_pool */
        template <typename Component, bool Unchecked = false> [[nodiscard]] const component_storage_type<Component>& get_pool(void) const {
            if constexpr (!Unchecked) assure_pool_exists<Component>();
            return pools.at(type_index<Component>()).storage.template as<component_storage_type<Component>>();
        }


        /** Returns true if a pool for the given component type exists. */
        template <typename Component> [[nodiscard]] bool has_pool(void) const {
            return pools.contains(type_index<Component>());
        }
    };
}