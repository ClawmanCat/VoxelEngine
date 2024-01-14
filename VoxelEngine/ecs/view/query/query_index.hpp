#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entity/entity_traits.hpp>
#include <VoxelEngine/ecs/view/query/query_utils.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage.hpp>

#include <bitset>


namespace ve::ecs::query {
    /**
     * Adding a query_index for a given query to a registry makes all requests using that query run with the same performance as that of a single-component view,
     * at the cost of extra memory proportional to the number of entities in the registry and slightly affected performance when an entity is created and when one of the accessed components is added/removed from an entity.
     * TODO: In the case where there are guaranteed components, it might be better to only track entities that have at least one of these.
     * @tparam Q The query to index.
     * @tparam EntityTraits The entity traits of the parent registry.
     */
    template <typename Q, entity_traits EntityTraits> class query_index {
    public:
        constexpr static inline bool has_guaranteed_components = !detail::query_included_components<Q>::empty;

        using entity_type         = typename EntityTraits::type;
        using entity_traits       = EntityTraits;
        using entity_utils        = entity_utils<EntityTraits>;
        using accessed_components = detail::query_accessed_components<Q>;


        query_index(void) = default;


        explicit query_index(const auto& src) {
            for (const auto entt : src.get_entities()) {
                entity_state state;

                accessed_components::foreach([&] <typename C> {
                    state.template set<C>(src.template has_component<C>(entt));
                });

                data.emplace(entt, state);


                if (!has_guaranteed_components || state.any()) {
                    if (check_query<Q>(state)) matched.insert(entt);
                }
            }
        }


        void on_entity_created(entity_type entt) {
            auto [it, success] = data.emplace(entt, entity_state { });

            if constexpr (!has_guaranteed_components) {
                if (check_query<Q>(it.component())) matched.insert(entt);
            }
        }


        void on_entity_destroyed(entity_type entt) {
            data.erase(entt);
            matched.erase(entt);
        }


        template <typename Component> void on_component_presence_changed(entity_type entt, bool present) {
            if constexpr (accessed_components::template contains<Component>) {
                auto& state = std::get<1>(data[entt]);
                state.template set<Component>(present);

                if (check_query<Q>(state)) matched.insert(entt);
                else matched.erase(entt);
            }
        }


        [[nodiscard]] std::size_t size(void) const { return matched.size(); }
        [[nodiscard]] const auto& get_matched_entities(void) const { return matched; }
    private:
        struct entity_state {
            std::bitset<accessed_components::size> components;

            template <typename C> bool get(void) const {
                return components[accessed_components::template find_first<C>];
            }

            template <typename C> void set(bool active) {
                components.set(accessed_components::template find_first<C>, active);
            }
        };


        basic_sparse_set<EntityTraits> matched;
        basic_component_storage<EntityTraits, default_component_traits<entity_state>> data;


        template <typename QQ> bool check_query(const entity_state& state) const {
            if constexpr (detail::is_base<QQ>) {
                if constexpr (QQ::include::apply([&] <typename... Cs> { return !(state.template get<Cs>() && ...); })) return false;
                if constexpr (QQ::exclude::apply([&] <typename... Cs> { return  (state.template get<Cs>() || ...); })) return false;
            }

            if constexpr (detail::is_const<QQ>) {
                return QQ::value;
            }

            if constexpr (detail::is_and<QQ>) {
                return check_query<typename QQ::lhs_type>(state) && check_query<typename QQ::rhs_type>(state);
            }

            if constexpr (detail::is_or<QQ>) {
                return check_query<typename QQ::lhs_type>(state) || check_query<typename QQ::rhs_type>(state);
            }

            if constexpr (detail::is_not<QQ>) {
                return !check_query<typename QQ::negated_type>(state);
            }
        }
    };
}