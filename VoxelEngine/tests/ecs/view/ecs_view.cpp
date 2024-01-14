#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage.hpp>
#include <VoxelEngine/ecs/view/ecs_view.hpp>
#include <VoxelEngine/ecs/view/joined_entity_view.hpp>
#include <VoxelEngine/ecs/view/query/query_view.hpp>
#include <VoxelEngine/ecs/view/sparse_set_view.hpp>
#include <VoxelEngine/ecs/view/component_storage_view.hpp>



namespace E = ve::ecs;
namespace Q = ve::ecs::query;


using entity_type      = VE_DEFAULT_ENTITY_TYPE;
using entity_traits    = E::default_entity_traits<entity_type>;
using component_type   = ve::meta::placeholder_type;
using component_traits = E::default_component_traits<component_type>;


TEST(ecs_view, constraint_checks) {
    using set_view = E::sparse_set_view<E::basic_sparse_set<entity_traits>>;
    using cmp_view = E::component_storage_view<E::basic_component_storage<entity_traits, component_traits>>;

    ASSERT_TRUE(E::ecs_view<set_view>);
    ASSERT_TRUE(E::ecs_view<cmp_view>);
    ASSERT_TRUE(E::direct_ecs_view<set_view>);
    ASSERT_TRUE(E::direct_ecs_view<cmp_view>);


    using joined_view = E::joined_entity_view<ve::meta::pack<set_view, set_view>>;
    ASSERT_TRUE(E::ecs_view<joined_view>);


    constexpr auto query = Q::has<component_type>;
    using query_view = E::query::query_view<decltype(query), set_view, ve::meta::pack<cmp_view>>;
    ASSERT_TRUE(E::ecs_view<query_view>);
}