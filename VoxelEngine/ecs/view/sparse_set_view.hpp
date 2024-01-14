#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view/ecs_view.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set.hpp>
#include <VoxelEngine/utility/container/fake_pointer.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>
#include <VoxelEngine/utility/meta/is_template.hpp>
#include <VoxelEngine/utility/meta/copy_modifier.hpp>


namespace ve::ecs {
    /** View adapter class for basic_sparse_set. */
    template <meta::template_instantiation_of<basic_sparse_set_common_base> Container>
    class sparse_set_view {
    private:
        template <typename Base, typename Parent> class iterator_adapter : private Base {
        public:
            template <typename B, typename P> friend class iterator_adapter;


            VE_WRAP_TYPEDEFS(Parent, entity_type, entity_traits, entity_utils, component_types, size_type, difference_type);
            using value_type        = std::conditional_t<std::is_const_v<Parent>, typename Parent::const_value_type, typename Parent::value_type>;
            using reference         = std::conditional_t<std::is_const_v<Parent>, typename Parent::const_reference,  typename Parent::reference>;
            using pointer           = std::conditional_t<std::is_const_v<Parent>, typename Parent::const_pointer,    typename Parent::pointer>;
            using iterator_category = std::bidirectional_iterator_tag;


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


            // Note: must be overridden manually since base class has a templated operator.
            template <typename B, typename P> [[nodiscard]] auto operator<=>(const iterator_adapter<B, P>& o) const {
                return Base::operator<=>(o);
            }

            // Note: must be overridden manually since base class has a templated operator.
            template <typename B, typename P> [[nodiscard]] bool operator==(const iterator_adapter<B, P>& o) const {
                return Base::operator==(o);
            }


            [[nodiscard]] entity_type  entity(void)     const { return Base::operator*(); }
            [[nodiscard]] std::tuple<> components(void) const { return std::tuple<>{};    }

            [[nodiscard]] reference operator* (void) const { return std::tuple { entity() }; }
            [[nodiscard]] pointer   operator->(void) const { return pointer    { *this    }; }
        };
    public:
        VE_WRAP_TYPEDEFS(Container, entity_type, entity_traits, entity_utils, size_type, difference_type);
        using component_types = meta::pack<>;

        VE_IMPL_CONTAINER_TYPEDEFS(
            std::tuple<entity_type>,
            std::type_identity_t,
            std::type_identity_t,
            fake_pointer,
            VE_WRAP_TYPENAME(iterator_adapter<typename Container::iterator, sparse_set_view<Container>>),
            VE_WRAP_TYPENAME(iterator_adapter<typename Container::const_iterator, const sparse_set_view<Container>>),
            VE_TRUE,
            VE_TRUE
        );

        VE_CONTAINER_BEGIN_END((meta::const_as(*container, *this).begin()), (meta::const_as(*container, *this).end()));


        sparse_set_view(void) = default;
        explicit sparse_set_view(Container* container) : container(container) {}


        [[nodiscard]] const_reference operator[](entity_type entt) const { return (*container)[entt]; }
        [[nodiscard]] bool contains(entity_type entt) const { return container->contains(entt); }
        [[nodiscard]] const auto& get_entities(void) const { return *container; }
    private:
        Container* container;
    };
}