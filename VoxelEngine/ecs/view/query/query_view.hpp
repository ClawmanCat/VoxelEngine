#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view/ecs_view.hpp>
#include <VoxelEngine/ecs/view/ecs_view_utils.hpp>
#include <VoxelEngine/ecs/view/query/query_utils.hpp>
#include <VoxelEngine/utility/meta/sequence.hpp>
#include <VoxelEngine/utility/meta/copy_modifier.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/utility/container/fake_pointer.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>
#include <VoxelEngine/utility/container/container_typedefs.hpp>


namespace ve::ecs::query {
    namespace detail {
        template <typename T> using add_const_reference_t = std::add_lvalue_reference_t<std::add_const_t<T>>;
        template <typename T> using add_const_pointer_t   = std::add_pointer_t<std::add_const_t<T>>;


        /** Used internally to access the internals of @ref query_view. */
        struct query_view_access_facade {
            static auto& get_controller(auto view) { return view.controller; }
            static auto& get_views(auto view) { return view.views; }
        };
    }


    template <typename Parent> class query_view_iterator;




    /**
     * Given a query and a view controller, views all entities in the view controller that match the given query.
     * Component instances are taken from the provided Views, which should contain all components that are or could be checked by the query.
     * @tparam Q An ecs_query to filter the entities by.
     * @tparam Controller A set of all entities that need to be queried.
     * @tparam Views A pool for every component type that is or could be matched by the query.
     */
    template <ecs_query Q, ecs_view Controller, meta::type_pack Views> requires (
        Views::all([] <typename V> { return ecs_view<V> && V::component_types::size == 1; }) &&
        detail::query_accessed_components<Q>::all([] <typename C> {
            return Views
                ::template map<ecs::detail::projections::component_types>
                ::template flatten_nonrecursive<>
                ::template contains<C>;
        })
    ) class query_view {
    public:
        using self_type                = query_view<Q, Controller, Views>;
        using query_type               = Q;
        using controller_type          = Controller;
        using views_type               = Views;
        using views_tuple              = typename Views::template to<std::tuple>;
        using all_views_type           = typename Views::template prepend<Controller>;
        using entity_type              = ecs::detail::common_view_projection<all_views_type, ecs::detail::projections::entity_type>;
        using entity_traits            = ecs::detail::common_view_projection<all_views_type, ecs::detail::projections::entity_traits>;
        using entity_utils             = ecs::detail::common_view_projection<all_views_type, ecs::detail::projections::entity_utils>;
        using component_types          = detail::query_interface_components<Q>;
        using accessed_component_types = detail::query_accessed_components<Q>;
        using mutable_components_type  = detail::project_query_components<Q, std::add_lvalue_reference_t, std::add_pointer_t>;
        using const_components_type    = detail::project_query_components<Q, detail::add_const_reference_t, detail::add_const_pointer_t>;

        using value_type               = typename mutable_components_type::template prepend<entity_type>::template to<std::tuple>;
        using const_value_type         = typename const_components_type::template prepend<entity_type>::template to<std::tuple>;
        using reference                = value_type;
        using const_reference          = const_value_type;
        using pointer                  = fake_pointer<value_type>;
        using const_pointer            = fake_pointer<const_value_type>;
        using size_type                = std::size_t;
        using difference_type          = std::ptrdiff_t;


        VE_IMPL_ITERATOR_TYPEDEFS(
            query_view_iterator<self_type>,
            query_view_iterator<const self_type>,
            VE_TRUE,
            VE_TRUE
        );

        VE_CONTAINER_BEGIN_END(
            (this, std::as_const(controller).begin()),
            (this, std::as_const(controller).end())
        );


        query_view(void) = default;
        query_view(query_type q, controller_type controller, views_tuple views) : controller(std::move(controller)), views(std::move(views)) {}


        [[nodiscard]] bool contains(entity_type entt) const {
            enum view_state : u8 { UNCHECKED, PRESENT, NOT_PRESENT };
            auto state = create_filled_array<Views::size>(UNCHECKED);


            auto check = [&] <typename C> (meta::type<C>) {
                constexpr std::size_t index = accessed_component_types::template find_first<C>;

                if (state[index] == UNCHECKED) state[index] = view_for<C>(*this).contains(entt) ? PRESENT : NOT_PRESENT;
                return (state[index] == PRESENT);
            };


            auto traverse = [&] <typename QQ> (const auto& self, meta::type<QQ>) {
                using namespace ve::ecs::query::detail;

                if constexpr (is_base<QQ>) {
                    return check(meta::type<typename QQ::component>{});
                }

                else if constexpr (is_const<QQ>) {
                    return QQ::value;
                }

                else if constexpr (is_and<QQ>) {
                    return self(self, meta::type<typename QQ::lhs_type>{}) && self(self, meta::type<typename QQ::rhs_type>{});
                }

                else if constexpr (is_or<QQ>) {
                    return self(self, meta::type<typename QQ::lhs_type>{}) || self(self, meta::type<typename QQ::rhs_type>{});
                }

                else if constexpr (is_not<QQ>) {
                    return !self(self, meta::type<typename QQ::negated_type>{});
                }
            };


            return traverse(traverse, meta::type<Q>{});
        }


        [[nodiscard]] reference       operator[](entity_type entt)       { return std::tuple_cat(controller[entt], get_components_unsafe(*this, entt)); }
        [[nodiscard]] const_reference operator[](entity_type entt) const { return std::tuple_cat(controller[entt], get_components_unsafe(*this, entt)); }
    private:
        friend class query_view_iterator<self_type>;
        friend class query_view_iterator<const self_type>;
        friend struct query_view_access_facade;


        controller_type controller;
        views_tuple views;


        /** Returns the view for the given component, with constness depending on 'Self'. */
        template <typename C, typename Self> static auto& view_for(Self& self) {
            auto* ptr = std::get<0>(meta::foreach_in_sequence(
                std::make_index_sequence<Views::size>(),
                [&] <std::size_t I> {
                    using V  = typename Views::template nth<I>;
                    using CV = meta::copy_const<V, Self>;

                    if constexpr (std::is_same_v<typename V::component_types::template nth<0>, C>) {
                        return meta::stop_token<CV*> { .payload = std::addressof(std::get<I>(self.views)) };
                    }
                }
            ));

            return *ptr;
        }


        /** Returns the components for the given entity, with constness depending on 'Self'. */
        template <typename Self> static auto get_components_unsafe(Self& self, entity_type entt) {
            return component_types::apply([&] <typename... Cs> {
                return std::tuple_cat(
                    [&] <typename C> (meta::type<C>) {
                        auto& view = view_for<C>(self);

                        if constexpr (detail::query_included_components<Q>::template contains<C>) {
                            return tuple_slice<1, 1>(view[entt]);
                        } else if constexpr (detail::query_optional_components<Q>::template contains<C>) {
                            return view.contains(entt)
                                ? std::tuple { std::addressof(std::get<0>(tuple_slice<1, 1>(view[entt]))) }
                                : std::tuple { nullptr };
                        } else return std::tuple {};
                    } (meta::type<Cs>{})...
                );
            });
        }
    };


    /** Deduction guide: query_view(Query, Controller, std::tuple<Views...>) -> query_view<Query, Controller, pack<Views...>>. */
    template <typename Q, typename C, typename... V>
    query_view(Q, C, std::tuple<V...>) -> query_view<Q, C, meta::pack<V...>>;




    /** Iterator class for @ref query_view. */
    template <typename Parent> class query_view_iterator {
    public:
        VE_WRAP_TYPEDEFS(Parent, entity_type, entity_traits, entity_utils, component_types, size_type, difference_type);

        using self_type             = query_view_iterator<Parent>;
        using wrapped_iterator_type = typename Parent::controller_type::const_iterator;
        using iterator_category     = std::bidirectional_iterator_tag;
        using value_type            = std::conditional_t<std::is_const_v<Parent>, typename Parent::const_value_type, typename Parent::value_type>;
        using reference             = std::conditional_t<std::is_const_v<Parent>, typename Parent::const_reference,  typename Parent::reference>;
        using pointer               = std::conditional_t<std::is_const_v<Parent>, typename Parent::const_pointer,    typename Parent::pointer>;


        query_view_iterator(void) = default;
        query_view_iterator(Parent* parent, wrapped_iterator_type iterator) : parent(parent), iterator(std::move(iterator)) { seek_to_valid(); }


        /** Copy / move operations: allow conversion from iterator to const_iterator. */
        template <iterator_compatible_parent<Parent> OP> query_view_iterator(const query_view_iterator<OP>& other) { *this = other; }
        template <iterator_compatible_parent<Parent> OP> query_view_iterator(query_view_iterator<OP>&& other) { *this = std::move(other); }

        template <iterator_compatible_parent<Parent> OP> query_view_iterator& operator=(const query_view_iterator<OP>& other) {
            VE_COPY_FIELDS(other, parent, iterator);
            return *this;
        }

        template <iterator_compatible_parent<Parent> OP> query_view_iterator& operator=(query_view_iterator<OP>&& other) {
            VE_MOVE_FIELDS(other, parent, iterator);
            return *this;
        }


        self_type& operator++(void) { next(); return *this; }
        self_type& operator--(void) { prev(); return *this; }
        self_type  operator++(int)  { auto old = *this; next(); return old; }
        self_type  operator--(int)  { auto old = *this; prev(); return old; }

        reference operator* (void) const { return (*parent)[iterator.entity()]; }
        pointer   operator->(void) const { return pointer { **this }; }


        VE_COMPARE_AS(iterator);
    private:
        template <typename P> friend class query_view_iterator;


        Parent* parent;
        wrapped_iterator_type iterator;


        void next(void) {
            const auto end = parent->controller.end();

            do ++iterator;
            while (iterator != end && !parent->contains(iterator.entity()));
        }


        void prev(void) {
            const auto begin = parent->controller.begin();

            do --iterator;
            while (iterator != begin && !parent->contains(iterator.entity()));
        }


        void seek_to_valid(void) {
            const auto end = parent->controller.end();
            while (iterator != end && !parent->contains(iterator.entity())) ++iterator;
        }
    };
}