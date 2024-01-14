#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/tests/ecs/storage/component_types.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage.hpp>
#include <VoxelEngine/ecs/view/joined_entity_view.hpp>
#include <VoxelEngine/ecs/view/component_storage_view.hpp>
#include <VoxelEngine/ecs/view/sparse_set_view.hpp>


using entity_type   = VE_DEFAULT_ENTITY_TYPE;
using entity_traits = ve::ecs::default_entity_traits<entity_type>;


template <typename View, typename... Sets> inline void test_common(View view, const Sets&... sets) {
    ve::hash_set<entity_type> expected;

    ve::tuple_foreach(std::tuple { sets... }, [&] (const auto& s) {
        expected.insert(s.get_entities().begin(), s.get_entities().end());
    });


    for (const auto [entt] : ve::ecs::joined_entity_view(std::tuple { sets... })) {
        EXPECT_TRUE(expected.contains(entt));
        expected.erase(entt);
    }

    EXPECT_TRUE(expected.empty());
}


TEST(joined_entity_view, join_sparse_set) {
    using set = ve::ecs::basic_sparse_set<entity_traits>;

    set a { 1, 11, 22, 44 };
    set b { 3, 88, 200, 1223 };
    set c { 50, 101, 8 };

    test_common(
        ve::views::all,
        ve::ecs::sparse_set_view { &a },
        ve::ecs::sparse_set_view { &b },
        ve::ecs::sparse_set_view { &c }
    );
}


TEST(joined_entity_view, join_sparse_set_with_duplicates) {
    using set = ve::ecs::basic_sparse_set<entity_traits>;

    set a { 1, 11, 22, 44 };
    set b { 3, 88, 200, 1223, 1, 44 };
    set c { 50, 101, 8, 1, 200 };

    test_common(
        ve::views::all,
        ve::ecs::sparse_set_view { &a },
        ve::ecs::sparse_set_view { &b },
        ve::ecs::sparse_set_view { &c }
    );
}


TEST(joined_entity_view, iterate_reversed) {
    using set = ve::ecs::basic_sparse_set<entity_traits>;

    set a { 1, 11, 22, 44 };
    set b { 3, 88, 200, 1223 };
    set c { 50, 101, 8 };

    test_common(
        ve::views::reverse,
        ve::ecs::sparse_set_view { &a },
        ve::ecs::sparse_set_view { &b },
        ve::ecs::sparse_set_view { &c }
    );
}


TEST(joined_entity_view, iterate_reversed_with_duplicates) {
    using set = ve::ecs::basic_sparse_set<entity_traits>;

    set a { 1, 11, 22, 44 };
    set b { 3, 88, 200, 1223, 1, 44 };
    set c { 50, 101, 8, 1, 200 };

    test_common(
        ve::views::reverse,
        ve::ecs::sparse_set_view { &a },
        ve::ecs::sparse_set_view { &b },
        ve::ecs::sparse_set_view { &c }
    );
}


TEST(joined_entity_view, join_component_storage) {
    using storage_c1 = ve::ecs::component_storage<ve::testing::copyable_component>;
    using storage_c2 = ve::ecs::component_storage<ve::testing::noncopyable_empty_component>;

    storage_c1 a;
    for (const auto entt : std::array<ve::u64, 4> { 1, 11, 22, 44 }) a.emplace(entt, entt);

    storage_c2 b;
    for (const auto entt : std::array<ve::u64, 4> { 3, 88, 200, 1223 }) b.emplace(entt, entt);

    test_common(
        ve::views::all,
        ve::ecs::component_storage_view { &a },
        ve::ecs::component_storage_view { &b }
    );
}


TEST(joined_entity_view, join_heterogeneous) {
    using set     = ve::ecs::basic_sparse_set<entity_traits>;
    using storage = ve::ecs::component_storage<ve::testing::copyable_component>;


    set a { 1, 11, 22, 44 };

    storage b;
    for (const auto entt : std::array<ve::u64, 4> { 3, 88, 200, 1223 }) b.emplace(entt, entt);


    test_common(
        ve::views::all,
        ve::ecs::sparse_set_view { &a },
        ve::ecs::component_storage_view { &b }
    );
}


TEST(joined_entity_view, empty_set) {
    using set = ve::ecs::basic_sparse_set<entity_traits>;

    set a { };
    set b { };
    set c { };

    test_common(
        ve::views::all,
        ve::ecs::sparse_set_view { &a },
        ve::ecs::sparse_set_view { &b },
        ve::ecs::sparse_set_view { &c }
    );
}


TEST(joined_entity_view, empty_subset) {
    using set = ve::ecs::basic_sparse_set<entity_traits>;

    set a { 1, 11, 22, 44 };
    set b { };
    set c { 50, 101, 8 };

    test_common(
        ve::views::all,
        ve::ecs::sparse_set_view { &a },
        ve::ecs::sparse_set_view { &b },
        ve::ecs::sparse_set_view { &c }
    );
}


TEST(joined_entity_view, size_one_set) {
    using set = ve::ecs::basic_sparse_set<entity_traits>;

    set a { 1  };
    set b { 3  };
    set c { 50 };

    test_common(
        ve::views::all,
        ve::ecs::sparse_set_view { &a },
        ve::ecs::sparse_set_view { &b },
        ve::ecs::sparse_set_view { &c }
    );
}