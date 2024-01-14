#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/tests/ecs/storage/component_types.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage_iterator.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage_mixin.hpp>


using entity_type = VE_DEFAULT_ENTITY_TYPE;
using ve::testing::component_types;


TEST(component_storage_iterator, foreach) {
    component_types::foreach([] <typename Component> {
        ve::ecs::component_storage<Component> c;

        for (entity_type entt = 0; entt < 10; ++entt) c.emplace(entt, entt);
        for (auto [entt, component] : c) ASSERT_EQ(component, Component { entt });
    });
}


TEST(component_storage_iterator, increment) {
    component_types::foreach([] <typename Component> {
        ve::ecs::component_storage<Component> c;
        
        std::array vals { 0u, 1u, 3u, 5u, 10u, 12u, 23u };
        for (auto val : vals) c.emplace(entity_type { val }, entity_type { val });


        auto it = c.begin();
        
        for (auto v : vals) {
            ASSERT_EQ(v, std::get<0>(*it));
            ++it;
        }
    });
}


TEST(component_storage_iterator, decrement) {
    component_types::foreach([] <typename Component> {
        ve::ecs::component_storage<Component> c;
        
        std::array vals { 0u, 1u, 3u, 5u, 10u, 12u, 23u };
        for (auto val : vals) c.emplace(entity_type { val }, entity_type { val });


        auto it = c.end();

        for (auto v : vals | ve::views::reverse) {
            --it;
            ASSERT_EQ(v, std::get<0>(*it));
        }
    });
}


TEST(component_storage_iterator, equality) {
    component_types::foreach([] <typename Component> {
        ve::ecs::component_storage<Component> c;

        std::array vals { 0u, 1u, 3u, 5u, 10u, 12u, 23u };
        for (auto val : vals) c.emplace(entity_type { val }, entity_type { val });

        ASSERT_EQ(c.begin(), c.begin());
        ASSERT_EQ(c.end(), c.end());
        ASSERT_EQ(std::next(c.begin()), std::next(c.begin()));
        ASSERT_NE(c.begin(), c.end());
        ASSERT_NE(std::next(c.begin()), c.begin());


        ve::ecs::component_storage<Component> empty;
        ASSERT_EQ(empty.begin(), empty.end());
    });
}


TEST(component_storage_iterator, comparability) {
    component_types::foreach([] <typename Component> {
        ve::ecs::component_storage<Component> c;

        std::array vals { 0u, 1u, 3u, 5u, 10u, 12u, 23u };
        for (auto val : vals) c.emplace(entity_type { val }, entity_type { val });

        ASSERT_LT(c.begin(), c.end());
        ASSERT_GT(std::next(c.begin()), c.begin());
    });
}