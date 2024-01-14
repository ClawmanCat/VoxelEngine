#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/tests/ecs/storage/component_types.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage_iterator.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage_mixin.hpp>


using entity_type = VE_DEFAULT_ENTITY_TYPE;
using ve::testing::component_types;


TEST(component_storage, insertion) {
    component_types::foreach([] <typename Component> {
        ve::ecs::component_storage<Component> c;

        for (entity_type entt = 0; entt < 10; ++entt) {
            auto [it, success] = c.emplace(entt);

            ASSERT_TRUE(success);
            ASSERT_NE(it, c.end());
        }
    });
}


TEST(component_storage, erase) {
    component_types::foreach([] <typename Component> {
        ve::ecs::component_storage<Component> c;

        for (entity_type entt = 0; entt < 10; ++entt) c.emplace(entt);

        for (entity_type entt = 0; entt < 10; ++entt) {
            ASSERT_TRUE(c.erase(entt));
        }

        for (entity_type entt = 0; entt < 10; ++entt) {
            ASSERT_FALSE(c.erase(entt));
        }
    });
}


TEST(component_storage, find) {
    component_types::foreach([] <typename Component> {
        ve::ecs::component_storage<Component> c;

        for (entity_type entt = 0; entt < 10; ++entt) c.emplace(entt, Component { entt });

        for (entity_type entt = 0; entt < 10; ++entt) {
            auto it = c.find(entt);
            auto [found_entt, component] = *it;

            ASSERT_EQ(entt, found_entt);
            ASSERT_EQ(component, Component { entt });
        }
    });
}


TEST(component_storage, copy_construct) {
    std::vector<std::size_t> entities { 1, 10, 100, 1'000, 10'000, 100'000 };


    component_types::foreach([&] <typename Component> {
        ve::ecs::component_storage<Component> s1 {};
        for (const auto entt : entities) s1.emplace(entt, Component { entt });

        ve::ecs::component_storage<Component> s2 { s1 };


        for (const auto entt : entities) {
            ASSERT_TRUE(s1.contains(entt));
            ASSERT_TRUE(s2.contains(entt));
        }

        for (const auto [entt, cmp] : s1) ASSERT_TRUE(cmp == Component { entt } && ranges::contains(entities, entt));
        for (const auto [entt, cmp] : s2) ASSERT_TRUE(cmp == Component { entt } && ranges::contains(entities, entt));
    });
}


TEST(component_storage, copy_assign) {
    std::vector<std::size_t> entities { 1, 10, 100, 1'000, 10'000, 100'000 };


    component_types::foreach([&] <typename Component> {
        ve::ecs::component_storage<Component> s1 {};
        for (const auto entt : entities) s1.emplace(entt, Component { entt });

        ve::ecs::component_storage<Component> s2 { };
        s2 = s1;


        for (const auto entt : entities) {
            ASSERT_TRUE(s1.contains(entt));
            ASSERT_TRUE(s2.contains(entt));
        }

        for (const auto [entt, cmp] : s1) ASSERT_TRUE(cmp == Component { entt } && ranges::contains(entities, entt));
        for (const auto [entt, cmp] : s2) ASSERT_TRUE(cmp == Component { entt } && ranges::contains(entities, entt));
    });
}


TEST(component_storage, move_construct) {
    std::vector<std::size_t> entities { 1, 10, 100, 1'000, 10'000, 100'000 };


    component_types::foreach([&] <typename Component> {
        ve::ecs::component_storage<Component> s1 {};
        for (const auto entt : entities) s1.emplace(entt, Component { entt });

        ve::ecs::component_storage<Component> s2 { std::move(s1) };


        for (const auto entt : entities) {
            ASSERT_TRUE(s2.contains(entt));
        }

        ASSERT_TRUE(s1.empty());
        for (const auto [entt, cmp] : s2) ASSERT_TRUE(cmp == Component { entt } && ranges::contains(entities, entt));
    });
}


TEST(component_storage, move_assign) {
    std::vector<std::size_t> entities { 1, 10, 100, 1'000, 10'000, 100'000 };


    component_types::foreach([&] <typename Component> {
        ve::ecs::component_storage<Component> s1 {};
        for (const auto entt : entities) s1.emplace(entt, Component { entt });

        ve::ecs::component_storage<Component> s2 { };
        s2 = std::move(s1);


        for (const auto entt : entities) {
            ASSERT_TRUE(s2.contains(entt));
        }

        ASSERT_TRUE(s1.empty());
        for (const auto [entt, cmp] : s2) ASSERT_TRUE(cmp == Component { entt } && ranges::contains(entities, entt));
    });
}