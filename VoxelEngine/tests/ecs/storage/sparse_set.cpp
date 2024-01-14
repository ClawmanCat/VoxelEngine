#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set.hpp>
#include <VoxelEngine/utility/meta/function_traits.hpp>


using unstable_traits = ve::ecs::default_entity_traits<ve::u64>;
using stable_traits   = typename ve::ecs::transform_entity_traits<unstable_traits>::template with_index_stability<true>;


ve::u64 make_entity(ve::u64 index, ve::u64 version, ve::u64 bits = 0b0) {
    return ve::ecs::entity_utils<unstable_traits>::make_entity(index, version, bits);
}


void contains_test_impl(auto& set, auto fn, auto contained_entity, auto equivalent_entity, auto non_contained_entity) {
    set.insert(contained_entity);
    EXPECT_TRUE(std::invoke(fn, set, contained_entity));
    EXPECT_TRUE(std::invoke(fn, set, equivalent_entity));
    EXPECT_FALSE(std::invoke(fn, set, non_contained_entity));

    set.erase(contained_entity);
    EXPECT_FALSE(std::invoke(fn, set, contained_entity));
    EXPECT_FALSE(std::invoke(fn, set, equivalent_entity));
    EXPECT_FALSE(std::invoke(fn, set, non_contained_entity));
}


TEST(sparse_set, contains) {
    ve::ecs::basic_sparse_set<unstable_traits> set;
    
    auto contained_entity     = make_entity(0, 0, 0);
    auto equivalent_entity    = make_entity(0, 0, 1);
    auto non_contained_entity = make_entity(0, 1, 0);

    contains_test_impl(set, &decltype(set)::contains, contained_entity, equivalent_entity, non_contained_entity);
}


TEST(sparse_set, contains_exact) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    auto contained_entity     = make_entity(0, 0, 0);
    auto equivalent_entity    = make_entity(0, 0, 0);
    auto non_contained_entity = make_entity(0, 0, 1);

    contains_test_impl(set, &decltype(set)::contains_exact, contained_entity, equivalent_entity, non_contained_entity);
}


TEST(sparse_set, contains_any_version) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    auto contained_entity     = make_entity(0, 0, 0);
    auto equivalent_entity    = make_entity(0, 1, 0);
    auto non_contained_entity = make_entity(1, 0, 0);

    contains_test_impl(set, &decltype(set)::contains_any_version, contained_entity, equivalent_entity, non_contained_entity);
}


void insert_test_impl(auto& set, auto fn, auto contained_entity, auto equivalent_entity, auto non_contained_entity) {
    EXPECT_TRUE(std::invoke(fn, set, contained_entity).second);
    EXPECT_FALSE(std::invoke(fn, set, equivalent_entity).second);
    EXPECT_TRUE(std::invoke(fn, set, non_contained_entity).second);
}


TEST(sparse_set, insert) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    auto contained_entity     = make_entity(0, 0, 0);
    auto equivalent_entity    = make_entity(0, 1, 0);
    auto non_contained_entity = make_entity(1, 0, 0);

    insert_test_impl(set, &decltype(set)::insert, contained_entity, equivalent_entity, non_contained_entity);
}


void erase_test_impl(auto& set, auto fn, auto contained_entity, auto equivalent_entity, auto non_contained_entity) {
    auto [it, success] = set.insert(contained_entity);
    EXPECT_TRUE(success);

    EXPECT_FALSE(std::invoke(fn, set, non_contained_entity));
    EXPECT_TRUE(std::invoke(fn, set, equivalent_entity));
    EXPECT_FALSE(std::invoke(fn, set, contained_entity));


    std::tie(it, success) = set.insert(contained_entity);
    EXPECT_TRUE(success);

    EXPECT_FALSE(std::invoke(fn, set, non_contained_entity));
    EXPECT_TRUE(std::invoke(fn, set, contained_entity));
    EXPECT_FALSE(std::invoke(fn, set, equivalent_entity));
}


TEST(sparse_set, erase) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    auto contained_entity     = make_entity(0, 0, 0);
    auto equivalent_entity    = make_entity(0, 0, 1);
    auto non_contained_entity = make_entity(0, 1, 0);

    erase_test_impl(set, &decltype(set)::erase, contained_entity, equivalent_entity, non_contained_entity);
}


TEST(sparse_set, erase_exact) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    auto contained_entity     = make_entity(0, 0, 0);
    auto equivalent_entity    = make_entity(0, 0, 0);
    auto non_contained_entity = make_entity(0, 0, 1);

    erase_test_impl(set, &decltype(set)::erase_exact, contained_entity, equivalent_entity, non_contained_entity);
}


TEST(sparse_set, erase_any_version) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    auto contained_entity     = make_entity(0, 0, 0);
    auto equivalent_entity    = make_entity(0, 1, 0);
    auto non_contained_entity = make_entity(1, 0, 0);

    erase_test_impl(set, &decltype(set)::erase_any_version, contained_entity, equivalent_entity, non_contained_entity);
}


void find_test_impl(auto& set, auto fn, auto contained_entity, auto equivalent_entity, auto non_contained_entity) {
    set.insert(contained_entity);
    EXPECT_EQ(*std::invoke(fn, set, contained_entity), contained_entity);
    EXPECT_EQ(*std::invoke(fn, set, equivalent_entity), contained_entity);
    EXPECT_EQ(std::invoke(fn, set, non_contained_entity), set.end());

    set.erase(contained_entity);
    EXPECT_EQ(std::invoke(fn, set, contained_entity), set.end());
    EXPECT_EQ(std::invoke(fn, set, equivalent_entity), set.end());
    EXPECT_EQ(std::invoke(fn, set, non_contained_entity), set.end());
}


TEST(sparse_set, find) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    auto contained_entity     = make_entity(0, 0, 0);
    auto equivalent_entity    = make_entity(0, 0, 1);
    auto non_contained_entity = make_entity(0, 1, 0);

    find_test_impl(set, ve::meta::as_mutable_mem_fn(&decltype(set)::find), contained_entity, equivalent_entity, non_contained_entity);
}


TEST(sparse_set, find_exact) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    auto contained_entity     = make_entity(0, 0, 0);
    auto equivalent_entity    = make_entity(0, 0, 0);
    auto non_contained_entity = make_entity(0, 0, 1);

    find_test_impl(set, ve::meta::as_mutable_mem_fn(&decltype(set)::find_exact), contained_entity, equivalent_entity, non_contained_entity);
}


TEST(sparse_set, find_any_version) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    auto contained_entity     = make_entity(0, 0, 0);
    auto equivalent_entity    = make_entity(0, 1, 0);
    auto non_contained_entity = make_entity(1, 0, 0);

    find_test_impl(set, ve::meta::as_mutable_mem_fn(&decltype(set)::find_any_version), contained_entity, equivalent_entity, non_contained_entity);
}


TEST(sparse_set, iteration_complexity_unstable) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    for (std::size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(set.size(), set.iteration_complexity());
        set.insert(make_entity(i, 0, 0));
    }

    for (std::size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(set.size(), set.iteration_complexity());
        set.erase(make_entity(i, 0, 0));
    }

    ASSERT_EQ(set.iteration_complexity(), 0);
}


TEST(sparse_set, iteration_complexity_stable) {
    ve::ecs::basic_sparse_set<stable_traits> set;

    for (std::size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(set.size(), set.iteration_complexity());
        set.insert(make_entity(i, 0, 0));
    }

    for (std::size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(set.iteration_complexity(), 10);
        set.erase(make_entity(i, 0, 0));
    }

    ASSERT_EQ(set.iteration_complexity(), 9);

    set.clear();
    ASSERT_EQ(set.iteration_complexity(), 0);
}


TEST(sparse_set, size_unstable) {
    ve::ecs::basic_sparse_set<unstable_traits> set;

    for (std::size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(set.size(), i);
        set.insert(make_entity(i, 0, 0));
        ASSERT_EQ(set.size(), i + 1);
    }

    for (std::size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(set.size(), 10 - i);
        set.erase(make_entity(i, 0, 0));
        ASSERT_EQ(set.size(), 10 - i - 1);
    }
}


TEST(sparse_set, size_stable) {
    ve::ecs::basic_sparse_set<stable_traits> set;

    for (std::size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(set.size(), i);
        set.insert(make_entity(i, 0, 0));
        ASSERT_EQ(set.size(), i + 1);
    }

    for (std::size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(set.size(), 10 - i);
        set.erase(make_entity(i, 0, 0));
        ASSERT_EQ(set.size(), 10 - i - 1);
    }
}


TEST(sparse_set, tombstone_exists) {
    ve::ecs::basic_sparse_set<stable_traits> set;

    set.insert(make_entity(0, 0, 0));
    set.insert(make_entity(1, 0, 0));
    set.erase(make_entity(0, 0, 0));

    const auto& raw = set.get_dense();
    ASSERT_EQ(raw.at(0), decltype(set)::entity_utils::tombstone());
    ASSERT_EQ(raw.at(1), make_entity(1, 0, 0));
}


TEST(sparse_set, versioning) {
    ve::ecs::basic_sparse_set<unstable_traits> set;
    using utils = typename decltype(set)::entity_utils;

    for (std::size_t version = 0; version < 10; ++version) {
        for (std::size_t entity = 0; entity < 10; ++entity) {
            auto [it, success] = set.insert(make_entity(entity, version, 0));

            ASSERT_TRUE(success);
            ASSERT_EQ(utils::version_of(*set.find_any_version(make_entity(entity, 0, 0))), version);

            ASSERT_TRUE(set.erase(make_entity(entity, version, 0)));
        }
    }
}


TEST(sparse_set, copy_construct) {
    std::vector<std::size_t> entities { 1, 10, 100, 1'000, 10'000, 100'000 };

    ve::ecs::basic_sparse_set<stable_traits> s1 { };
    for (const auto entt : entities) s1.insert(entt);

    ve::ecs::basic_sparse_set<stable_traits> s2 { s1 };


    for (const auto entt : entities) {
        ASSERT_TRUE(s1.contains(entt));
        ASSERT_TRUE(s2.contains(entt));
    }

    for (const auto entt : s1) ASSERT_TRUE(ranges::contains(entities, entt));
    for (const auto entt : s2) ASSERT_TRUE(ranges::contains(entities, entt));
}


TEST(sparse_set, copy_assign) {
    std::vector<std::size_t> entities { 1, 10, 100, 1'000, 10'000, 100'000 };

    ve::ecs::basic_sparse_set<stable_traits> s1 { };
    for (const auto entt : entities) s1.insert(entt);

    ve::ecs::basic_sparse_set<stable_traits> s2 { };
    s2 = s1;


    for (const auto entt : entities) {
        ASSERT_TRUE(s1.contains(entt));
        ASSERT_TRUE(s2.contains(entt));
    }

    for (const auto entt : s1) ASSERT_TRUE(ranges::contains(entities, entt));
    for (const auto entt : s2) ASSERT_TRUE(ranges::contains(entities, entt));
}


TEST(sparse_set, move_construct) {
    std::vector<std::size_t> entities { 1, 10, 100, 1'000, 10'000, 100'000 };

    ve::ecs::basic_sparse_set<stable_traits> s1 { };
    for (const auto entt : entities) s1.insert(entt);

    ve::ecs::basic_sparse_set<stable_traits> s2 { std::move(s1) };


    for (const auto entt : entities) ASSERT_TRUE(s2.contains(entt));

    ASSERT_TRUE(s1.empty());
    for (const auto entt : s2) ASSERT_TRUE(ranges::contains(entities, entt));
}


TEST(sparse_set, move_assign) {
    std::vector<std::size_t> entities { 1, 10, 100, 1'000, 10'000, 100'000 };

    ve::ecs::basic_sparse_set<stable_traits> s1 { };
    for (const auto entt : entities) s1.insert(entt);

    ve::ecs::basic_sparse_set<stable_traits> s2 { };
    s2 = std::move(s1);


    for (const auto entt : entities) ASSERT_TRUE(s2.contains(entt));

    ASSERT_TRUE(s1.empty());
    for (const auto entt : s2) ASSERT_TRUE(ranges::contains(entities, entt));
}