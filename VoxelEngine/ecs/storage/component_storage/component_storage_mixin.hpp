#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entity/entity_traits.hpp>
#include <VoxelEngine/ecs/component/component_traits.hpp>


namespace ve::ecs {
    /**
     * CRTP Base class for mixins that can be used to receive callbacks from @ref basic_component_storage.
     * @tparam Derived Derived class which implements this interface.
     * @tparam EntityTraits Entity traits of the sparse set this mixin is used for.
     * @tparam ComponentTraits Component traits of the sparse set this mixin is used for.
     */
    template <typename Derived, entity_traits EntityTraits, component_traits ComponentTraits>
    struct component_storage_mixin {
        using entity_traits    = EntityTraits;
        using component_traits = ComponentTraits;
        using entity_type      = typename EntityTraits::type;
        using component_type   = typename ComponentTraits::type;
        using self_type        = component_storage_mixin<Derived, EntityTraits, ComponentTraits>;


        /**
         * Called when a constructor of the component storage is invoked and when set_mixin is called.
         * Note: the constructors of the mixin may be used as well, but these do not give access to the parent storage.
         * Note: for handling removal from the storage, use the mixin destructor.
         */
        template <typename Parent> void on_added_to_storage(Parent& parent) {
            VE_TRY_TEMPLATE_CRTP_CALL(self_type, Derived, on_added_to_storage<Parent>, (parent));
        }


        /**
         * Called after a component is added to the storage.
         * @param entt The entity to which the inserted component belongs.
         * @param component The component that was inserted for the given entity.
         * @param dense_index The location of the entity in the dense vector of its sparse_set. Also the index of the component in its own internal storage.
         */
        void on_insert(entity_type entt, component_type& component, std::size_t dense_index) {
            VE_TRY_CRTP_CALL(self_type, Derived, on_insert, (entt, component, dense_index));
        }


        /**
         * Called after a component is removed from the storage.
         * @param entt The entity to which the removed component belonged.
         * @param component The component that was removed from the given entity. The mixin may safely take ownership of the component through std::move.
         * @param dense_index The location of the entity in the dense vector of its sparse_set. Also the former index of the component in its own internal storage.
         */
        void on_erase(entity_type entt, component_type&& component, std::size_t dense_index) {
            VE_TRY_CRTP_CALL(self_type, Derived, on_erase, (entt, component, dense_index));
        }


        /**
         * Called after two components swap their position in the component storage.
         * This can happen just before erasure, unless the associated entity set has index stability.
         * In this case, e1/c1 are the entity and component which are about to be erased.
         * Note that an entity/component can swap with itself, in which case both values in each pair of variables are the same.
         * @param e1 The entity which had its position in the dense vector of its set swapped.
         * @param e2 The entity with which e1 has swapped its position.
         * @param c1 The component associated with e1.
         * @param c2 The component associated with e2.
         * @param e1_dest The dense vector index previously occupied by e2 and now occupied by e1 (Also the index of the component within its own internal storage).
         * @param e2_dest The dense vector index previously occupied by e1 and now occupied by e2 (Also the index of the component within its own internal storage).
         */
        void on_swap(entity_type e1, entity_type e2, component_type& c1, component_type& c2, std::size_t e1_dest, std::size_t e2_dest) {
            VE_TRY_CRTP_CALL(self_type, Derived, on_swap, (e1, e2, c1, c2, e1_dest, e2_dest));
        }


        /** Called when the storage is cleared. on_erase is not invoked in this scenario. */
        void on_clear(void) {
            VE_TRY_CRTP_CALL(self_type, Derived, on_clear, ());
        }
    };


    /** Can be used to indicate the absence of a mixin. */
    template <entity_traits ET, component_traits CT>
    struct no_component_storage_mixin : component_storage_mixin<no_component_storage_mixin<ET, CT>, ET, CT> {};

    /** Generates the correct template specialization of @ref no_component_storage_mixin for the given traits. */
    template <entity_traits ET, template <typename> typename CT>
    struct no_component_storage_mixin_for {
        template <typename Component> using type = no_component_storage_mixin<ET, CT<Component>>;
    };


    /**
     * Concept for classes that implement the @ref component_storage_set_mixin CRTP-interface.
     * @tparam ET The @ref entity_traits class to which the mixin's entity traits must compare equivalent.
     * @tparam CT The @ref component_traits class to which the mixin's component traits must compare equivalent.
     */
    template <typename T, typename ET, typename CT> concept component_storage_mixin_type = (
        entity_traits_equivalent<ET, typename T::entity_traits> &&
        component_traits_equivalent<CT, typename T::component_traits> &&
        std::is_base_of_v<component_storage_mixin<T, typename T::entity_traits, typename T::component_traits>, T>
    );
}