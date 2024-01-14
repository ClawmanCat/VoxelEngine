#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/copy_modifier.hpp>
#include <VoxelEngine/utility/container/container_typedefs.hpp>
#include <VoxelEngine/utility/container/fake_pointer.hpp>


namespace ve::ecs {
    /** Iterator class for @ref basic_component_storage. */
    template <typename Parent> class component_storage_iterator {
    public:
        VE_WRAP_TYPEDEFS(
            Parent,
            entity_traits, entity_type, entity_utils, entity_mixin,
            component_traits, component_utils, component_mixin,
            size_type, difference_type
        );

        VE_WRAP_CONSTANTS(Parent, has_reference_stability, has_storage_elusion, has_entity_mixin, has_component_mixin);


        using self_type              = component_storage_iterator<Parent>;
        using component_storage_type = Parent;
        using iterator_category      = std::bidirectional_iterator_tag;
        using entity_iterator_type   = std::conditional_t<std::is_const_v<Parent>, typename Parent::entity_storage_type::const_iterator, typename Parent::entity_storage_type::iterator>;
        using value_type             = std::conditional_t<std::is_const_v<Parent>, typename Parent::const_value_type, typename Parent::value_type>;
        using reference              = std::conditional_t<std::is_const_v<Parent>, typename Parent::const_reference,  typename Parent::reference>;
        using pointer                = std::conditional_t<std::is_const_v<Parent>, typename Parent::const_pointer,    typename Parent::pointer>;
        using component_type         = std::tuple_element_t<1, value_type>;


        component_storage_iterator(void) : entity_iterator(), component_storage(nullptr) {}
        component_storage_iterator(component_storage_type* parent, entity_iterator_type it) : entity_iterator(it), component_storage(parent) {}


        /** Copy / move operations: allow conversion from iterator to const_iterator. */
        template <iterator_compatible_parent<Parent> OP> component_storage_iterator(const component_storage_iterator<OP>& other) { *this = other; }
        template <iterator_compatible_parent<Parent> OP> component_storage_iterator(component_storage_iterator<OP>&& other) { *this = std::move(other); }

        template <iterator_compatible_parent<Parent> OP> component_storage_iterator& operator=(const component_storage_iterator<OP>& other) {
            VE_COPY_FIELDS(other, entity_iterator, component_storage);
            return *this;
        }

        template <iterator_compatible_parent<Parent> OP> component_storage_iterator& operator=(component_storage_iterator<OP>&& other) {
            VE_MOVE_FIELDS(other, entity_iterator, component_storage);
            return *this;
        }


        [[nodiscard]] reference operator* (void) const { return get(); }
        [[nodiscard]] pointer   operator->(void) const { return pointer { get() }; }

        self_type& operator++(void) { ++entity_iterator; return *this; }
        self_type& operator--(void) { --entity_iterator; return *this; }
        self_type  operator++(int)  { auto old = *this; ++entity_iterator; return old; }
        self_type  operator--(int)  { auto old = *this; --entity_iterator; return old; }

        [[nodiscard]] auto operator<=>(const self_type& o) const { return entity_iterator <=> o.entity_iterator; }
        [[nodiscard]] bool operator== (const self_type& o) const { return entity_iterator ==  o.entity_iterator; }


        [[nodiscard]] entity_type     entity   (void) const { return std::get<0>(get()); }
        [[nodiscard]] component_type& component(void) const { return std::get<1>(get()); }


        VE_WRAP_CONST_MEM_FNS(entity_iterator, set_version, set_unassigned_bits, dense_offset);
        VE_GET_VALS(entity_iterator, component_storage);
    private:
        entity_iterator_type entity_iterator;
        component_storage_type* component_storage;


        [[nodiscard]] reference get(void) const {
            return reference { *entity_iterator, component_storage->components[entity_iterator.dense_offset()] };
        }
    };
}