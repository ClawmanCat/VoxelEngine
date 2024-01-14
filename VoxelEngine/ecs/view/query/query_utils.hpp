#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view/query/query_building_blocks.hpp>
#include <VoxelEngine/utility/meta/is_template.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/utility/meta/pack/create_pack.hpp>
#include <VoxelEngine/utility/meta/pack/pack_operations.hpp>


namespace ve::ecs::query::detail {
    template <typename Q> constexpr inline bool is_base  = meta::is_template_v<base_query,        Q>;
    template <typename Q> constexpr inline bool is_const = meta::is_template_v<constant_query,    Q>;
    template <typename Q> constexpr inline bool is_and   = meta::is_template_v<conjunction_query, Q>;
    template <typename Q> constexpr inline bool is_or    = meta::is_template_v<disjunction_query, Q>;
    template <typename Q> constexpr inline bool is_not   = meta::is_template_v<negation_query,    Q>;


    enum class query_traverse_mode { MUST_INCLUDE, COULD_INCLUDE, MUST_EXCLUDE, COULD_EXCLUDE, EVERYTHING };


    /** Finds components in the given query for the given traverse mode. */
    template <typename Q, query_traverse_mode Mode, bool IsConditional = false, bool IsNegated = false>
    consteval auto find_query_components(void) {
        if constexpr (is_base<Q>) {
            using CT = meta::pack<typename Q::component>;

            if constexpr (Mode == query_traverse_mode::EVERYTHING) return CT{};
            else if constexpr (Mode == query_traverse_mode::MUST_INCLUDE  && !IsConditional && !IsNegated) return CT{};
            else if constexpr (Mode == query_traverse_mode::COULD_INCLUDE &&  IsConditional && !IsNegated) return CT{};
            else if constexpr (Mode == query_traverse_mode::MUST_EXCLUDE  && !IsConditional &&  IsNegated) return CT{};
            else if constexpr (Mode == query_traverse_mode::COULD_EXCLUDE &&  IsConditional &&  IsNegated) return CT{};
            else return meta::pack<>{};
        }

        else if constexpr (is_const<Q>) {
            return meta::pack<>{};
        }

        else if constexpr (is_and<Q>) {
            using lhs = decltype(find_query_components<typename Q::lhs_type, Mode, IsConditional, IsNegated>());
            using rhs = decltype(find_query_components<typename Q::rhs_type, Mode, IsConditional, IsNegated>());

            return typename meta::create_pack::from_many<lhs, rhs>::template unique<>{};
        }

        else if constexpr (is_or<Q>) {
            using lhs = decltype(find_query_components<typename Q::lhs_type, Mode, true, IsNegated>());
            using rhs = decltype(find_query_components<typename Q::rhs_type, Mode, true, IsNegated>());

            return typename meta::create_pack::from_many<lhs, rhs>::template unique<>{};
        }

        else if constexpr (is_not<Q>) {
            return find_query_components<typename Q::negated_type, Mode, IsConditional, !IsNegated>();
        }
    }


    /** Returns true if the union of all accessed components within the query is a superset of all entities accessed by the query. */
    template <typename Q> consteval bool is_closed_set(void) {
        if constexpr (is_base<Q>) {
            return true;
        }

        else if constexpr (is_const<Q>) {
            return !Q::value;
        }

        else if constexpr (is_and<Q>) {
            return is_closed_set<typename Q::lhs_type>() || is_closed_set<typename Q::rhs_type>();
        }

        else if constexpr (is_or<Q>) {
            return is_closed_set<typename Q::lhs_type>() && is_closed_set<typename Q::rhs_type>();
        }

        else if constexpr (is_not<Q>) {
            return !is_closed_set<typename Q::negated_type>();
        }
    }


    /** Components that are always included in the given query. E.g. X in the query has<X>. */
    template <typename Q> using query_included_components  = decltype(find_query_components<Q, query_traverse_mode::MUST_INCLUDE>());
    /** Components that are conditionally included in the given query. E.g. X and Y in the query has<X> || has<Y>. */
    template <typename Q> using query_optional_components  = decltype(find_query_components<Q, query_traverse_mode::COULD_INCLUDE>());
    /** Components that are always excluded from the given query. E.g. X in the query !has<X>. */
    template <typename Q> using query_excluded_components  = decltype(find_query_components<Q, query_traverse_mode::MUST_EXCLUDE>());
    /** Components that are conditionally excluded from the given query. E.g. X and Y in the query !has<X> || !has<Y>. */
    template <typename Q> using query_optional_exclusions  = decltype(find_query_components<Q, query_traverse_mode::COULD_EXCLUDE>());
    /** Components that are accessed by views resulting from this query, including those that are not parts of its interface. */
    template <typename Q> using query_accessed_components  = decltype(find_query_components<Q, query_traverse_mode::EVERYTHING>());
    /** Components that are part of the interface of views resulting from this query (That is, components which are not guaranteed-excluded). */
    template <typename Q> using query_interface_components = meta::create_pack::from_many<query_included_components<Q>, query_optional_components<Q>>;


    /** Performs the given projections to the included and optional components of the given query and joins the resulting types into a single pack. */
    template <
        typename Q,
        template <typename> typename ProjectInclude,
        template <typename> typename ProjectOptional
    > using project_query_components = meta::create_pack::from_many<
        typename query_included_components<Q>
            ::template map<ProjectInclude>,
        typename query_optional_components<Q>
            ::template map<ProjectOptional>
    >;
}