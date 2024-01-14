#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/system/system_traits_templates.hpp>
#include <VoxelEngine/ecs/system/scheduling/system_scheduler.hpp>
#include <VoxelEngine/ecs/storage/registry/registry.hpp>


using namespace ve::time;
using namespace ve::ecs;
using namespace ve::ecs::system_traits_templates;

using entity = VE_DEFAULT_ENTITY_TYPE;


struct Type1 {};
struct Type2 {};
struct Type3 { entity value; };
struct Type4 { entity value; };


struct simple_system : ve::ecs::system<
    simple_system,
    read_only_system_for_types<ve::meta::pack<Type1, Type2>>
> {
    bool present_in_registry = false;
    ve::hash_set<entity> seen {};


    template <typename R> void on_added(R& registry) {
        present_in_registry = true;
    }


    template <typename R> void on_removed(R& registry) {
        present_in_registry = false;
    }


    template <typename View> void operator()(View&& view, duration dt, const tick_timestamp& prev, const tick_timestamp& now) {
        for (auto [entt, t1, t2] : view) {
            seen.insert(entt);
        }
    }
};


TEST(system_scheduler, execute_system) {
    registry r;

    auto [system, id] = r.add_system(simple_system {});
    ASSERT_TRUE(system.present_in_registry);


    for (entity entt = 0; entt < 1'024; ++entt) {
        ASSERT_TRUE(r.create_with_id(entt));
        if (entt % 2 == 0) r.template add_component<Type1>(entt);
        if (entt % 3 == 0) r.template add_component<Type2>(entt);
    }

    r.update();


    auto taken_system = *r.template take_system<simple_system>(id);
    ASSERT_FALSE(taken_system.present_in_registry);


    for (entity entt = 0; entt < 1'024; ++entt) {
        ASSERT_EQ(
            (entt % 2 == 0 && entt % 3 == 0),
            taken_system.seen.contains(entt)
        );
    }
}


struct concurrent_system : ve::ecs::system<
    concurrent_system,
    read_only_system_for_types<ve::meta::pack<Type1, Type2>>
> {
    interval executed_when;

    template <typename View> void operator()(View&& view, duration dt, const tick_timestamp& prev, const tick_timestamp& now) {
        auto start = clock::now();
        std::this_thread::sleep_for(5s);
        auto stop  = clock::now();

        executed_when = { start, stop };
        ve::get_service<ve::engine_logger>().info("work work work") << std::flush;
    }
};


TEST(system_scheduler, execute_system_concurrent) {
    ve::ecs::registry r;
    auto [sys_1, id_1] = r.add_system(concurrent_system {});
    auto [sys_2, id_2] = r.add_system(concurrent_system {});
    r.update();

    const bool has_multithreaded   = std::thread::hardware_concurrency() > 1;
    const bool executed_concurrent = has_temporal_intersection(sys_1.executed_when, sys_2.executed_when);

    if (has_multithreaded) ASSERT_TRUE(executed_concurrent);
    else ASSERT_FALSE(executed_concurrent);
}