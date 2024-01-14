#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/container/container_typedefs.hpp>
#include <VoxelEngine/ecs/entity/entity_traits.hpp>
#include <VoxelEngine/ecs/component/component_traits.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage_iterator.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage_mixin.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage_mixin_adapter.hpp>
#include <VoxelEngine/ecs/storage/internal/unsafe_paged_storage.hpp>
#include <VoxelEngine/ecs/storage/internal/unsafe_dummy_storage.hpp>
#include <VoxelEngine/ecs/view/ecs_view_utils.hpp>


namespace ve::ecs {
    namespace detail {
        // Can't use std::conditional_t for this since we'd trigger the requires-clause on unsafe_dummy_storage even if it's on the false branch.
        template <component_traits CT>
        struct component_storage_type_impl { using type = detail::unsafe_paged_storage<typename CT::type, CT::page_size>; };

        template <component_traits CT> requires CT::elude_storage
        struct component_storage_type_impl<CT> { using type = detail::unsafe_dummy_storage<typename CT::type>; };

        /**
         * Returns the storage type used internally by @ref basic_component_storage.
         * Either a normal @ref unsafe_paged_storage, or in the case of storage elusion @ref unsafe_dummy_storage.
         */
        template <component_traits CT> using component_storage_type = typename component_storage_type_impl<CT>::type;
    }


    /**
     * ECS Container used to store components of a certain type. Essentially functions as a map<entity, component>. This container is based on that of the EnTT library (https://github.com/skypjack/entt).
     *  - This container provides O(1) lookup of entities while providing a contiguous array of entities & components for fast iteration.
     *  - Entity ID indices should form one or more semi-contiguous value ranges.
     *  - Entity ID indices can and should be re-used by incrementing their version.
     *
     * @warning If ComponentTraits::reference_stability is true, then EntityTraits::index_stability must also be true!
     *
     * @tparam EntityTraits Entity traits for entities stored in this container. Type must fulfil the @ref entity_traits concept.
     * @tparam ComponentTraits Component traits for components stored in this container. Type must fulfil the @ref component_traits concept.
     * @tparam EntityMixin Optional mixin with callbacks for operations that modify the entity set. Type must fulfil the @ref sparse_set_mixin_type concept.
     * @tparam ComponentMixin Optional mixin with callbacks for operations that modify the component set. Type must fulfil the @ref component_storage_mixin_type concept.
     */
    template <
        entity_traits EntityTraits,
        component_traits ComponentTraits,
        sparse_set_mixin_type<EntityTraits> EntityMixin = no_sparse_set_mixin<EntityTraits>,
        component_storage_mixin_type<EntityTraits, ComponentTraits> ComponentMixin = no_component_storage_mixin<EntityTraits, ComponentTraits>
    > requires (
        // Stability if components also requires stability of their associated entities.
        !ComponentTraits::reference_stability || EntityTraits::index_stability
    ) class basic_component_storage {
    public:
        using self_type              = basic_component_storage<EntityTraits, ComponentTraits, EntityMixin, ComponentMixin>;
        using entity_traits          = EntityTraits;
        using component_traits       = ComponentTraits;
        using entity_mixin           = detail::component_storage_mixin_adapter<self_type, EntityMixin>;
        using component_mixin        = ComponentMixin;
        using entity_type            = typename entity_traits::type;
        using component_type         = typename component_traits::type;
        using entity_utils           = entity_utils<entity_traits>;
        using component_utils        = component_utils<component_traits>;
        using entity_storage_type    = basic_sparse_set<entity_traits, entity_mixin>;
        using component_storage_type = detail::component_storage_type<component_traits>;

        VE_IMPL_CONTAINER_TYPEDEFS(
            VE_WRAP_TYPENAME(std::tuple<entity_type, component_type>),
            detail::project_value_type_components<std::add_const_t>::template type,
            detail::project_value_type_components<std::add_lvalue_reference_t>::template type,
            fake_pointer,
            component_storage_iterator<self_type>,
            component_storage_iterator<const self_type>,
            VE_TRUE,
            VE_TRUE
        );

        constexpr static inline bool has_reference_stability = component_traits::reference_stability;
        constexpr static inline bool has_storage_elusion     = component_traits::elude_storage;
        constexpr static inline bool has_entity_mixin        = !std::is_same_v<EntityMixin, no_sparse_set_mixin<entity_traits>>;
        constexpr static inline bool has_component_mixin     = !std::is_same_v<ComponentTraits, no_component_storage_mixin<entity_traits, component_traits>>;


        explicit basic_component_storage(EntityMixin mixin_entity, ComponentMixin mixin_component = ComponentMixin { }) :
            entities(entity_mixin { this, std::move(mixin_entity) }),
            components()
        { set_component_mixin(std::move(mixin_component)); }

        basic_component_storage(void) : basic_component_storage(EntityMixin { }, ComponentMixin {}) {}


        ~basic_component_storage(void) {
            if constexpr (!component_storage_type::trivially_destructible) {
                for (auto it = entities.begin(); it != entities.end(); ++it) components.erase(it.dense_offset());
            }
        }


        basic_component_storage& operator=(const basic_component_storage& other) {
            entities = other.entities;

            if constexpr (component_storage_type::is_copyable) {
                components = other.components;
            } else {
                for (auto it = entities.begin(); it != entities.end(); ++it) {
                    components.emplace(it.dense_offset(), other.components[it.dense_offset()]);
                }
            }

            entities.get_mixin().on_parent_moved(this);
            return *this;
        }

        basic_component_storage& operator=(basic_component_storage&& other) {
            entities   = std::move(other.entities);
            components = std::move(other.components);
            set_component_mixin(std::move(other.mixin));

            entities.get_mixin().on_parent_moved(this);
            return *this;
        }

        basic_component_storage(const basic_component_storage& o) { *this = o; }
        basic_component_storage(basic_component_storage&& o) { *this = std::move(o); }


        VE_CONTAINER_BEGIN_END(
            (this, entities.begin()),
            (this, entities.end())
        );


        /**
         * Constructs a new component in the set for the entity, if one does not already exist.
         * @param entt The entity to construct a component for.
         * @param args The constructor arguments for the component.
         * @return A pair [iterator, success] with 'success' indicating if insertion happened, and 'iterator' an iterator to either the newly inserted or already existing element.
         */
        template <typename... Args> std::pair<iterator, bool> emplace(entity_type entt, Args&&... args) {
            auto [it, success] = entities.insert(entt);

            if (success) {
                auto& component = components.emplace(it.dense_offset(), fwd(args)...);
                mixin.on_insert(entt, component, it.dense_offset());
            }

            return {
                iterator { this, it },
                success
            };
        }


        /**
         * Equivalent to @ref emplace, but replaces any existing element.
         * @param entt The entity to construct or replace a component for.
         * @param args The constructor arguments for the component.
         * @return A pair [iterator, newly_inserted], with 'newly_inserted' true if no component for the entity existed previously, and 'iterator' an iterator to the element.
         */
        template <typename... Args> std::pair<iterator, bool> emplace_or_replace(entity_type entt, Args&&... args) {
            auto [it, success] = emplace(entt, fwd(args)...);

            if (!success) {
                components[it.dense_offset()] = component_type { fwd(args)... };
            }

            return { it, success };
        }


        void clear(void) {
            if constexpr (component_storage_type::trivially_destructible) {
                components.clear();
            } else {
                for (auto it = entities.begin(); it != entities.end(); ++it) components.erase(it.dense_offset());
            }

            entities.clear();
            mixin.on_clear();
        }


        iterator find(entity_type entt) { return iterator { this, entities.find(entt) }; }
        iterator find_exact(entity_type entt) { return iterator { this, entities.find_exact(entt) }; }
        iterator find_any_version(entity_type entt) { return iterator { this, entities.find_any_version(entt) }; }

        const_iterator find(entity_type entt) const { return const_iterator { this, entities.find(entt) }; }
        const_iterator find_exact(entity_type entt) const { return const_iterator { this, entities.find_exact(entt) }; }
        const_iterator find_any_version(entity_type entt) const { return const_iterator { this, entities.find_any_version(entt) }; }


        /** Returns a tuple of the entity and a reference to its component. This method <b>DOES NOT</b> validate the storage contains the component for this entity. */
        [[nodiscard]] reference operator[](entity_type entt) { return { entt, components[entities.get_dense_offset(entt)] }; }
        /** @copydoc operator[] */
        [[nodiscard]] const_reference operator[](entity_type entt) const { return { entt, components[entities.get_dense_offset(entt)] }; }


        void set_entity_mixin(EntityMixin mixin) {
            entities.set_mixin(entity_mixin { this, std::move(mixin) });
        }

        void set_component_mixin(ComponentMixin mixin) {
            this->mixin = std::move(mixin);
            this->mixin.on_added_to_storage(*this);
        }

        [[nodiscard]] EntityMixin&       get_entity_mixin(void)       { return entities.get_mixin().get_additional_mixin(); }
        [[nodiscard]] const EntityMixin& get_entity_mixin(void) const { return entities.get_mixin().get_additional_mixin(); }

        [[nodiscard]] ComponentMixin&       get_component_mixin(void)       { return mixin; }
        [[nodiscard]] const ComponentMixin& get_component_mixin(void) const { return mixin; }


        VE_WRAP_MEM_FNS(entities, erase, erase_any_version, erase_exact);
        VE_WRAP_CONST_MEM_FNS(entities, contains, contains_any_version, contains_exact, size, iteration_complexity, empty);

        VE_GET_CREFS(entities);
    private:
        friend class component_storage_iterator<self_type>;
        friend class component_storage_iterator<const self_type>;
        friend class detail::component_storage_mixin_adapter<self_type, EntityMixin>;

        entity_storage_type entities;
        VE_NO_UNIQUE_ADDRESS component_storage_type components;
        VE_NO_UNIQUE_ADDRESS component_mixin mixin;


        /** Invoked from @ref component_storage_mixin_adapter. Swap components when the entity set swaps two entities. */
        void mixin_on_swap(entity_type e1, entity_type e2, std::size_t e1_dest, std::size_t e2_dest) {
            std::swap(components[e1_dest], components[e2_dest]);
            mixin.on_swap(e1, e2, components[e1_dest], components[e2_dest], e1_dest, e2_dest);
        }

        /** Invoked from @ref component_storage_mixin_adapter. When an entity is erased, also erase its associated component. */
        void mixin_on_erase(entity_type entt, std::size_t dense_index) {
            auto value = std::move(components[dense_index]);
            components.erase(dense_index);
            mixin.on_erase(entt, std::move(value), dense_index);
        }
    };


    template <typename Component> using component_storage = basic_component_storage<
        default_entity_traits<VE_DEFAULT_ENTITY_TYPE>,
        default_component_traits<Component>
    >;
}