#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entity/entity_traits.hpp>


namespace ve::ecs {
    /**
     * CRTP Base class for mixins that can be used to receive callbacks from @ref basic_sparse_set.
     * @tparam Derived Derived class which implements this interface.
     * @tparam Traits Entity traits of the sparse set this mixin is used for.
     */
    template <typename Derived, entity_traits Traits> struct sparse_set_mixin {
        using entity_traits = Traits;
        using entity_type   = typename Traits::type;
        using entity_utils  = entity_utils<Traits>;
        using self_type     = sparse_set_mixin<Derived, Traits>;


        /**
         * Called when a constructor of the sparse set is invoked and when set_mixin is called.
         * Note: the constructors of the mixin may be used as well, but these do not give access to the parent set.
         * Note: for handling removal from the set, use the mixin destructor.
         */
        template <typename Parent> void on_added_to_set(Parent& parent) {
            VE_TRY_TEMPLATE_CRTP_CALL(self_type, Derived, on_added_to_set<Parent>, (parent));
        }


        /**
         * Called after an entity is added to the set.
         * @param entt The entity that was inserted into the set.
         * @param dense_index The index in the dense vector of the set the entity was inserted at.
         */
        void on_insert(entity_type entt, std::size_t dense_index) {
            VE_TRY_CRTP_CALL(self_type, Derived, on_insert, (entt, dense_index));
        }


        /**
         * Called after an entity is removed from the set (Except through set.clear(), see @ref on_clear).
         * @param entt The entity that was removed from the set.
         * @param dense_index The index in the dense vector of the set the entity occupied before its removal.
         */
        void on_erase(entity_type entt, std::size_t dense_index) {
            VE_TRY_CRTP_CALL(self_type, Derived, on_erase, (entt, dense_index));
        }


        /**
         * Called after an entity is modified in place. Note that the index part of the entity always remains constant in this case.
         * @param prev The previous ID of the entity.
         * @param current The new ID of the entity.
         * @param dense_index The index in the dense vector of the set the entity occupies.
         */
        void on_edited(entity_type prev, entity_type current, std::size_t dense_index) {
            VE_TRY_CRTP_CALL(self_type, Derived, on_edited, (prev, current, dense_index));
        }


        /**
         * Called after two entities swap their position in the dense storage of the set.
         * This can happen just before erasure, unless Traits::stable_index is true (Note: e1 is the entity which is about to be erased in this case).
         * Note that an entity can swap with itself, in this case e1 and e2 are the same, and so are e1_dest and e2_dest.
         * @param e1 The entity which had its position in the dense vector of the set swapped.
         * @param e2 The entity with which e1 has swapped its position.
         * @param e1_dest The dense vector index previously occupied by e2 and now occupied by e1.
         * @param e2_dest The dense vector index previously occupied by e1 and now occupied by e2.
         */
        void on_swap(entity_type e1, entity_type e2, std::size_t e1_dest, std::size_t e2_dest) {
            VE_TRY_CRTP_CALL(self_type, Derived, on_swap, (e1, e2, e1_dest, e2_dest));
        }


        /** Called when the set is cleared. on_erase is not invoked in this scenario. */
        void on_clear(void) {
            VE_TRY_CRTP_CALL(self_type, Derived, on_clear, ());
        }
    };


    /** Can be used to indicate the absence of a mixin. */
    template <entity_traits ET> struct no_sparse_set_mixin : sparse_set_mixin<no_sparse_set_mixin<ET>, ET> {};


    /**
     * Concept for classes that implement the @ref sparse_set_mixin CRTP-interface.
     * @tparam ET The @ref entity_traits class to which the mixin's traits must compare equivalent.
     */
    template <typename T, typename ET> concept sparse_set_mixin_type = (
        std::is_base_of_v<sparse_set_mixin<T, typename T::entity_traits>, T> &&
        entity_traits_equivalent<ET, typename T::entity_traits>
    );
}