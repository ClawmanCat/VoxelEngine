#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>


namespace ve::ecs::query {
    /** Common base class for ECS query building blocks. */
    struct query_building_block {};

    /** Types must derive from query_building_block to be used within ECS query methods. */
    template <typename Q> concept ecs_query = std::is_base_of_v<query_building_block, Q>;




    template <typename Component> struct base_query : query_building_block {
        using component = Component;
    };

    template <typename BoolConstant> struct constant_query : query_building_block {
        constexpr static inline bool value = BoolConstant::value;
    };

    template <typename LHS, typename RHS> struct conjunction_query : query_building_block {
        using lhs_type = LHS;
        using rhs_type = RHS;
    };

    template <typename LHS, typename RHS> struct disjunction_query : query_building_block {
        using lhs_type = LHS;
        using rhs_type = RHS;
    };

    template <typename Q> struct negation_query : query_building_block {
        using negated_type = Q;
    };




    /** Returns the conjunction (logical AND) of two ECS queries. */
    template <ecs_query LHS, ecs_query RHS> constexpr conjunction_query<LHS, RHS> operator&&(LHS, RHS) { return {}; }

    /** Returns the disjunction (logical OR) of two ECS queries. */
    template <ecs_query LHS, ecs_query RHS> constexpr disjunction_query<LHS, RHS> operator||(LHS, RHS) { return {}; }

    /** Returns the negation (logical NOT) of an ECS query. */
    template <ecs_query Q> constexpr negation_query<Q> operator!(Q) { return {}; }


    /** 'True' boolean constants for use within ECS queries. */
    constexpr inline constant_query<std::true_type>  true_query  = {};
    /** 'False' boolean constants for use within ECS queries. */
    constexpr inline constant_query<std::false_type> false_query = {};


    /** Basic ECS query to indicate the presence of a component. */
    template <typename Component> constexpr inline base_query<Component> has = {};
    /** Short for has<Component1> && has<Component2> && ... True if components is empty. */
    template <typename... Components> constexpr inline auto has_all = (has<Components> && ... && true_query);
    /** Short for has<Component1> || has<Component2> || ... False if components is empty. */
    template <typename... Components> constexpr inline auto has_any = (has<Components> || ... || false_query);
}