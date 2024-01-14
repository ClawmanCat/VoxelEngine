#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view/ecs_view.hpp>
#include <VoxelEngine/ecs/view/ecs_view_utils.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/utility/container/fake_pointer.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>


namespace ve::ecs {
    template <typename Parent> class joined_entity_view_iterator;


    /**
     * Given a set of direct_ecs_views, provides a view over all entities that are at least in one of the views.
     * This view performs deduplication, i.e., an entity contained in multiple views will only be iterated on its first occurrence.
     * @tparam Views A set of direct_ecs_views to iterate over the entities of.
     */
    template <meta::type_pack Views> requires (
        ecs_view<typename Views::head>
        //Views::all([] <typename View> { return ecs_view<View>; })
    ) class joined_entity_view {
    public:
        using self_type       = joined_entity_view<Views>;
        using views_type      = Views;
        using views_tuple     = typename Views::template to<std::tuple>;
        using entity_type     = detail::common_view_projection<Views, detail::projections::entity_type>;
        using entity_traits   = detail::common_view_projection<Views, detail::projections::entity_traits>;
        using entity_utils    = detail::common_view_projection<Views, detail::projections::entity_utils>;
        using component_types = meta::pack<>;
        using common_set_type = basic_sparse_set_common_base<entity_traits>;

        VE_IMPL_CONTAINER_TYPEDEFS(
            std::tuple<entity_type>,
            std::type_identity_t,
            std::type_identity_t,
            fake_pointer,
            joined_entity_view_iterator<self_type>,
            joined_entity_view_iterator<const self_type>,
            VE_TRUE,
            VE_TRUE
        );

        VE_CONTAINER_BEGIN_END(
            (this, Views::empty ? typename common_set_type::const_iterator {} : std::as_const(*entities.front()).begin(), 0),
            (this, Views::empty ? typename common_set_type::const_iterator {} : std::as_const(*entities.back()).end(),    Views::size)
        );


        joined_entity_view(void) = default;

        explicit joined_entity_view(views_tuple views) :
            views(std::move(views)),
            entities(generate_filled_array<Views::size>([&] <std::size_t I> (meta::value<I>) {
                return (common_set_type*) std::addressof(std::get<I>(this->views).get_entities());
            }))
        {}


        [[nodiscard]] bool contains(entity_type entt) const {
            for (const auto* set : entities) {
                if (set->contains(entt)) return true;
            }

            return false;
        }


        [[nodiscard]] value_type operator[](entity_type entt) const {
            for (const auto* set : entities) {
                if (auto it = set->find(entt); it != set->end()) return { (*set)[entt] };
            }

            VE_DEBUG_ASSERT(false, "Entity not present in view.");
            VE_UNREACHABLE;
        }


        [[nodiscard]] iterator find(entity_type entt) {
            for (const auto [i, set] : entities | views::enumerate) {
                if (auto it = set->find(entt); it != set->end()) return iterator { this, it, i };
            }

            return end();
        }


        [[nodiscard]] const_iterator find(entity_type entt) const {
            for (const auto [i, set] : entities | views::enumerate) {
                if (auto it = set->find(entt); it != set->end()) return const_iterator { this, it, i };
            }

            return end();
        }
    private:
        friend class joined_entity_view_iterator<self_type>;
        friend class joined_entity_view_iterator<const self_type>;

        views_tuple views;
        std::array<common_set_type*, Views::size> entities;
    };


    /** Deduction guide: views pack is equal to the tuple passed to the constructor. */
    template <typename... Views> joined_entity_view(std::tuple<Views...>) -> joined_entity_view<meta::pack<Views...>>;


    template <typename Parent> class joined_entity_view_iterator {
    public:
        VE_WRAP_TYPEDEFS(
            Parent,
            entity_type, entity_traits, entity_utils, component_types,
            value_type, reference, pointer, size_type, difference_type
        );

        using self_type             = joined_entity_view_iterator<Parent>;
        using wrapped_iterator_type = typename Parent::common_set_type::const_iterator;
        using iterator_category     = std::bidirectional_iterator_tag;


        joined_entity_view_iterator(void) = default;

        joined_entity_view_iterator(Parent* parent, wrapped_iterator_type iterator, std::size_t view_index) :
            parent(parent),
            iterator(std::move(iterator)),
            view_index(view_index)
        { seek_to_valid(); }


        /** Copy / move operations: allow conversion from iterator to const_iterator. */
        template <iterator_compatible_parent<Parent> OP> joined_entity_view_iterator(const joined_entity_view_iterator<OP>& other) { *this = other; }
        template <iterator_compatible_parent<Parent> OP> joined_entity_view_iterator(joined_entity_view_iterator<OP>&& other) { *this = std::move(other); }

        template <iterator_compatible_parent<Parent> OP> joined_entity_view_iterator& operator=(const joined_entity_view_iterator<OP>& other) {
            VE_COPY_FIELDS(other, parent, iterator, view_index);
            return *this;
        }

        template <iterator_compatible_parent<Parent> OP> joined_entity_view_iterator& operator=(joined_entity_view_iterator<OP>&& other) {
            VE_MOVE_FIELDS(other, parent, iterator, view_index);
            return *this;
        }


        self_type& operator++(void) { next(); return *this; }
        self_type& operator--(void) { prev(); return *this; }
        self_type  operator++(int)  { auto old = *this; next(); return old; }
        self_type  operator--(int)  { auto old = *this; prev(); return old; }


        [[nodiscard]] value_type operator* (void) const { return value_type { *iterator }; }
        [[nodiscard]] pointer    operator->(void) const { return pointer { **this }; }

        [[nodiscard]] entity_type entity(void) const { return *iterator; }
        [[nodiscard]] std::tuple<> components(void) const { return {}; }


        VE_COMPARE_AS(view_index, iterator);
    private:
        template <typename P> friend class joined_entity_view_iterator;

        Parent* parent;
        wrapped_iterator_type iterator;
        std::size_t view_index;


        [[nodiscard]] auto current_begin(void) const { return parent->entities[view_index]->begin(); }
        [[nodiscard]] auto current_end  (void) const { return parent->entities[view_index]->end();   }


        void next(void) {
            bool incremented = false, valid = false;

            while (!incremented || !valid) {
                if (iterator == current_end()) {
                    if (++view_index != parent->entities.size()) [[likely]] {
                        iterator = current_begin();
                        valid    = iterator != current_end() && !seen(*iterator);

                        continue;
                    } else break;
                }

                ++iterator;
                incremented = true;
                valid       = iterator != current_end() && !seen(*iterator);
            }
        }


        void prev(void) {
            bool decremented = false, valid = false;

            while (!decremented && !valid) {
                if (iterator == current_begin()) {
                    --view_index;
                    iterator = current_end();
                    continue;
                }

                --iterator;
                decremented = true;
                valid       = !seen(*iterator);
            }
        }


        void seek_to_valid(void) {
            if (
                view_index < parent->entities.size() &&
                (
                    iterator == parent->entities[view_index]->end() ||
                    seen(*iterator)
                )
            ) next();
        }


        bool seen(entity_type entt) const {
            for (std::size_t i = 0; i < view_index; ++i) {
                if (parent->entities[i]->contains(entt)) return true;
            }

            return false;
        }
    };
}