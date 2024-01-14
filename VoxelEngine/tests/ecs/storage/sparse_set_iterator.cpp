#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set_mixin.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set_iterator.hpp>


using unstable_traits = ve::ecs::default_entity_traits<ve::u64>;
using stable_traits   = typename ve::ecs::transform_entity_traits<unstable_traits>::template with_index_stability<true>;


TEST(sparse_set_iterator, increment) {
    ve::ecs::basic_sparse_set<unstable_traits> set;
    std::array vals { 0, 1, 3, 5, 10, 12, 23 };

    for (auto entt : vals) set.insert(entt);


    auto it = set.begin();

    for (auto v : vals) {
        ASSERT_EQ(v, *it);
        ++it;
    }
}


TEST(sparse_set_iterator, decrement) {
    ve::ecs::basic_sparse_set<unstable_traits> set;
    std::array vals { 0, 1, 3, 5, 10, 12, 23 };

    for (auto entt : vals) set.insert(entt);


    auto it = set.end();

    for (auto v : vals | ve::views::reverse) {
        --it;
        ASSERT_EQ(v, *it);
    }
}


TEST(sparse_set_iterator, increment_tombstone_skipping) {
    ve::ecs::basic_sparse_set<stable_traits> set;
    std::array vals { 0, 1, 3, 5, 10, 12, 23 };

    for (auto entt : vals) set.insert(entt);
    set.erase(3);
    set.erase(12);


    auto it = set.begin();

    for (auto v : vals | ve::views::remove(3) | ve::views::remove(12)) {
        ASSERT_EQ(v, *it);
        ++it;
    }
}


TEST(sparse_set_iterator, decrement_tombstone_skipping) {
    ve::ecs::basic_sparse_set<stable_traits> set;
    std::array vals { 0, 1, 3, 5, 10, 12, 23 };

    for (auto entt : vals) set.insert(entt);
    set.erase(3);
    set.erase(12);


    auto it = set.end();

    for (auto v : vals | ve::views::reverse | ve::views::remove(3) | ve::views::remove(12)) {
        --it;
        ASSERT_EQ(v, *it);
    }
}


TEST(sparse_set_iterator, equality) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    std::array vals { 0, 1, 3, 5, 10, 12, 23 };
    for (auto entt : vals) set.insert(entt);

    ASSERT_EQ(set.begin(), set.begin());
    ASSERT_EQ(set.end(), set.end());
    ASSERT_EQ(std::next(set.begin()), std::next(set.begin()));
    ASSERT_NE(set.begin(), set.end());
    ASSERT_NE(std::next(set.begin()), set.begin());

    ve::ecs::basic_sparse_set<unstable_traits> empty_set;
    ASSERT_EQ(empty_set.begin(), empty_set.end());
}


TEST(sparse_set_iterator, comparability) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    std::array vals { 0, 1, 3, 5, 10, 12, 23 };
    for (auto entt : vals) set.insert(entt);

    ASSERT_LT(set.begin(), set.end());
    ASSERT_GT(std::next(set.begin()), set.begin());
}


TEST(sparse_set_iterator, set_version) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    std::array vals { 0, 1, 3, 5, 10, 12, 23 };
    for (auto entt : vals) set.insert(entt);

    auto it = set.find(12);
    it.set_version(3);

    ASSERT_FALSE(set.contains(decltype(set)::entity_utils::make_entity(12, 0)));
    ASSERT_TRUE(set.contains(decltype(set)::entity_utils::make_entity(12, 3)));
}


TEST(sparse_set_iterator, set_unassigned_bits) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    std::array vals { 0, 1, 3, 5, 10, 12, 23 };
    for (auto entt : vals) set.insert(entt);

    auto it = set.find(12);
    it.set_unassigned_bits(1);

    ASSERT_FALSE(set.contains_exact(decltype(set)::entity_utils::make_entity(12, 0, 0)));
    ASSERT_TRUE(set.contains_exact(decltype(set)::entity_utils::make_entity(12, 0, 1)));
}


TEST(sparse_set_iterator, dense_offset) {
    ve::ecs::basic_sparse_set<stable_traits> set;

    std::array vals    { 0, 1, 3, 5, 10, 12, 23 };
    std::array offsets { 0, 1,    3, 4,      6  };

    for (auto entt : vals) set.insert(entt);
    set.erase(3);
    set.erase(12);

    auto it = set.begin();
    for (auto offset : offsets) {
        ASSERT_EQ(it.dense_offset(), offset);
        ++it;
    }
}