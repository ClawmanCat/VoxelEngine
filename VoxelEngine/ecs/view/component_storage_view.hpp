#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view/ecs_view.hpp>
#include <VoxelEngine/ecs/view/ecs_view_utils.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage.hpp>
#include <VoxelEngine/utility/container/fake_pointer.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>
#include <VoxelEngine/utility/meta/is_template.hpp>
#include <VoxelEngine/utility/meta/copy_modifier.hpp>


namespace ve::ecs {
    /** View adapter class for basic_component_storage. */
    template <meta::template_instantiation_of<basic_component_storage> Container>
    class component_storage_view {
    private:
        template <typename Base, typename Parent> class iterator_adapter : private Base {
        public:
            VE_WRAP_TYPEDEFS(Parent, entity_type, entity_traits, entity_utils, component_types, size_type, difference_type);
            using value_type        = std::conditional_t<std::is_const_v<Parent>, typename Parent::const_value_type, typename Parent::value_type>;
            using reference         = std::conditional_t<std::is_const_v<Parent>, typename Parent::const_reference,  typename Parent::reference>;
            using pointer           = std::conditional_t<std::is_const_v<Parent>, typename Parent::const_pointer,    typename Parent::pointer>;
            using iterator_category = std::bidirectional_iterator_tag;
            using component_type    = std::tuple_element_t<1, value_type>;


            VE_COPYABLE(iterator_adapter);
            iterator_adapter(void) = default;
            explicit iterator_adapter(const Base& base) : Base(base) {}
            explicit iterator_adapter(Base&& base) : Base(std::move(base)) {}


            /** Copy / move operations: allow conversion from iterator to const_iterator. */
            template <typename B, iterator_compatible_parent<Parent> OP> iterator_adapter(const iterator_adapter<B, OP>& other) { *this = other; }
            template <typename B, iterator_compatible_parent<Parent> OP> iterator_adapter(iterator_adapter<B, OP>&& other) { *this = std::move(other); }

            template <typename B, iterator_compatible_parent<Parent> OP> iterator_adapter& operator=(const iterator_adapter<B, OP>& other) {
                Base::operator=(other);
                return *this;
            }

            template <typename B, iterator_compatible_parent<Parent> OP> iterator_adapter& operator=(iterator_adapter<B, OP>&& other) {
                Base::operator=(std::move(other));
                return *this;
            }


            using Base::operator++;
            using Base::operator--;
            using Base::operator<=>;
            using Base::operator==;
            using Base::operator*;
            using Base::operator->;


            [[nodiscard]] entity_type entity(void) const { return std::get<0>(*this); }
            [[nodiscard]] std::tuple<component_type&> components(void) const { return { std::get<1>(*this) }; }
        };
    public:
        VE_WRAP_TYPEDEFS(Container, entity_type, entity_traits, entity_utils, size_type, difference_type);
        using component_types = meta::pack<typename Container::component_type>;

        VE_IMPL_CONTAINER_TYPEDEFS(
            VE_WRAP_TYPENAME(std::tuple<entity_type, typename Container::component_type>),
            detail::project_value_type_components<std::add_const_t>::template type,
            detail::project_value_type_components<std::add_lvalue_reference_t>::template type,
            fake_pointer,
            VE_WRAP_TYPENAME(iterator_adapter<typename Container::iterator, component_storage_view<Container>>),
            VE_WRAP_TYPENAME(iterator_adapter<typename Container::const_iterator, const component_storage_view<Container>>),
            VE_TRUE,
            VE_TRUE
        );

        VE_CONTAINER_BEGIN_END((meta::const_as(*container, *this).begin()), (meta::const_as(*container, *this).end()));


        component_storage_view(void) = default;
        explicit component_storage_view(Container* container) : container(container) {}


        [[nodiscard]] reference       operator[](entity_type entt)       { return (*container)[entt]; }
        [[nodiscard]] const_reference operator[](entity_type entt) const { return std::as_const(*container)[entt]; }

        [[nodiscard]] bool contains(entity_type entt) const { return container->contains(entt); }
        [[nodiscard]] const auto& get_entities(void) const { return container->get_entities(); }
    private:
        Container* container;
    };
}