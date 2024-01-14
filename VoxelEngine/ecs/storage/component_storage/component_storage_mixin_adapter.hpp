#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set_mixin.hpp>
#include <VoxelEngine/utility/meta/value.hpp>


#define VE_IMPL_CSMA_METHOD(name, ...)                                                  \
additional_mixin.name(__VA_ARGS__);                                                     \
                                                                                        \
if constexpr (requires { parent->mixin_##name(__VA_ARGS__); }) {                        \
    parent->mixin_##name(__VA_ARGS__);                                                  \
}


namespace ve::ecs::detail {
    /**
     * Adapter class to allow @ref basic_component_storage to have its own @ref sparse_set_mixin,
     * while still allowing an user provided mixin to be provided as well.
     * @tparam Parent Parent @ref basic_component_storage object.
     * @tparam AdditionalMixin Optional additional @ref sparse_set_mixin.
     */
    template <typename Parent, sparse_set_mixin_type<typename Parent::entity_traits> AdditionalMixin>
    class component_storage_mixin_adapter : public sparse_set_mixin<component_storage_mixin_adapter<Parent, AdditionalMixin>, typename Parent::entity_traits> {
    public:
        using wrapped_mixin_type = AdditionalMixin;
        using entity_type        = typename Parent::entity_type;


        component_storage_mixin_adapter(void) = default;

        explicit component_storage_mixin_adapter(Parent* parent, wrapped_mixin_type additional_mixin = wrapped_mixin_type { }) :
            parent(parent),
            additional_mixin(std::move(additional_mixin))
        {}


        template <typename SetParent> void on_added_to_set(SetParent& set) { 
            VE_IMPL_CSMA_METHOD(on_added_to_set, set);
        }

        void on_insert(entity_type entt, std::size_t dense_index) {
            VE_IMPL_CSMA_METHOD(on_insert, entt, dense_index);
        }

        void on_erase(entity_type entt, std::size_t dense_index) {
            VE_IMPL_CSMA_METHOD(on_erase, entt, dense_index);
        }

        void on_edited(entity_type prev, entity_type current, std::size_t dense_index) {
            VE_IMPL_CSMA_METHOD(on_edited, prev, current, dense_index);
        }

        void on_swap(entity_type e1, entity_type e2, std::size_t e1_dest, std::size_t e2_dest) {
            VE_IMPL_CSMA_METHOD(on_swap, e1, e2, e1_dest, e2_dest);
        }

        void on_clear(void) {
            VE_IMPL_CSMA_METHOD(on_clear);
        }


        /**
         * Invoked by the parent @ref basic_component_storage when it is moved.
         * Note: should be private but Clang does not resolve friendship to 'Parent' correctly.
         */
        void on_parent_moved(Parent* new_address) { parent = new_address; }


        VE_GET_VALS(parent);
        VE_GET_MREFS(additional_mixin);
    private:
        Parent* parent;
        VE_NO_UNIQUE_ADDRESS wrapped_mixin_type additional_mixin;
    };
}