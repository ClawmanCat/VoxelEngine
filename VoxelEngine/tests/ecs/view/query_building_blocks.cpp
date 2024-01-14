#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/ecs/view/query/query_building_blocks.hpp>
#include <VoxelEngine/ecs/view/query/query_view_builder.hpp>
#include <VoxelEngine/ecs/view/query/query_utils.hpp>


using ve::meta::pack;
using namespace ve::ecs::query;


template <auto Q> using required = detail::query_included_components<decltype(Q)>;
template <auto Q> using optional = detail::query_optional_components<decltype(Q)>;


struct Type1 {};
struct Type2 {};
struct Type3 {};
struct Type4 {};
struct Type5 {};


TEST(query_building_blocks, base_type) {
    constexpr auto Q1 = has<Type1>;
    ASSERT_TYPE_EQ(required<Q1>, pack<Type1>);
    ASSERT_TYPE_EQ(optional<Q1>, pack<>);

    constexpr auto Q2 = has_all<Type1, Type2, Type3>;
    ASSERT_TYPE_EQ(required<Q2>, pack<Type1, Type2, Type3>);
    ASSERT_TYPE_EQ(optional<Q2>, pack<>);
}


TEST(query_building_blocks, conjunction) {
    constexpr auto Q1 = has<Type1> && has<Type2>;
    ASSERT_TYPE_EQ(required<Q1>, pack<Type1, Type2>);
    ASSERT_TYPE_EQ(optional<Q1>, pack<>);

    constexpr auto Q2 = has<Type1> && has<Type1>;
    ASSERT_TYPE_EQ(required<Q2>, pack<Type1>);
    ASSERT_TYPE_EQ(optional<Q2>, pack<>);

    constexpr auto Q3 = has_all<Type1, Type3> && !has_all<Type2, Type4>;
    ASSERT_TYPE_EQ(required<Q3>, pack<Type1, Type3>);
    ASSERT_TYPE_EQ(optional<Q3>, pack<>);

    constexpr auto Q4 = has<Type1> && !has<Type1>;
    ASSERT_TYPE_EQ(required<Q4>, pack<Type1>);
    ASSERT_TYPE_EQ(optional<Q4>, pack<>);
}


TEST(query_building_blocks, disjunction) {
    constexpr auto Q1 = has<Type1> || has<Type2>;
    ASSERT_TYPE_EQ(required<Q1>, pack<>);
    ASSERT_TYPE_EQ(optional<Q1>, pack<Type1, Type2>);

    constexpr auto Q2 = has<Type1> || has<Type1>;
    ASSERT_TYPE_EQ(required<Q2>, pack<>);
    ASSERT_TYPE_EQ(optional<Q2>, pack<Type1>);

    constexpr auto Q3 = has_all<Type1, Type3> || !has_all<Type2, Type4>;
    ASSERT_TYPE_EQ(required<Q3>, pack<>);
    ASSERT_TYPE_EQ(optional<Q3>, pack<Type1, Type3>);

    constexpr auto Q4 = has<Type1> || !has<Type1>;
    ASSERT_TYPE_EQ(required<Q4>, pack<>);
    ASSERT_TYPE_EQ(optional<Q4>, pack<Type1>);
}


TEST(query_building_blocks, inversion) {
    constexpr auto Q1 = !has<Type1>;
    ASSERT_TYPE_EQ(required<Q1>, pack<>);
    ASSERT_TYPE_EQ(optional<Q1>, pack<>);

    constexpr auto Q2 = !!has<Type1>;
    ASSERT_TYPE_EQ(required<Q2>, pack<Type1>);
    ASSERT_TYPE_EQ(optional<Q2>, pack<>);

    constexpr auto Q3 = has<Type1> && !has<Type2>;
    ASSERT_TYPE_EQ(required<Q3>, pack<Type1>);
    ASSERT_TYPE_EQ(optional<Q3>, pack<>);
}


TEST(query_building_blocks, composite) {
    constexpr auto Q1 = (has<Type1> || has<Type2>) && (has<Type3> && (!has<Type4> || !has<Type5>));
    ASSERT_TYPE_EQ(required<Q1>, pack<Type3>);
    ASSERT_TYPE_EQ(optional<Q1>, pack<Type1, Type2>);
}