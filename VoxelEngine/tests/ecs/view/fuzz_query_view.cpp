#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/services/random.hpp>
#include <VoxelEngine/ecs/storage/registry/registry.hpp>


namespace fuzzargs = ve::testing::fuzz_arguments;
using namespace ve::ecs::query;

using entity_type  = VE_DEFAULT_ENTITY_TYPE;


struct Type1 {};
struct Type2 {};
struct Type3 {};
struct Type4 {};
struct Type5 {};
struct Type6 {};
struct AllEntitiesHave {};
struct NoEntitiesHave  {};


struct state {
    struct entity_state {
        bool has_type_1 : 1 = false;
        bool has_type_2 : 1 = false;
        bool has_type_3 : 1 = false;
        bool has_type_4 : 1 = false;
        bool has_type_5 : 1 = false;
        bool has_type_6 : 1 = false;
    };


    ve::hash_map<entity_type, entity_state> state {};
    ve::ecs::registry registry {};
};


void add_entity(state* s, entity_type entt, bool has_1, bool has_2, bool has_3, bool has_4, bool has_5, bool has_6) {
    bool success = s->registry.create_with_id(entt);

    if (success) {
        auto& e_state = s->state[entt];

        if (has_1) { s->registry.template add_component<Type1>(entt); e_state.has_type_1 = true; }
        if (has_2) { s->registry.template add_component<Type2>(entt); e_state.has_type_2 = true; }
        if (has_3) { s->registry.template add_component<Type3>(entt); e_state.has_type_3 = true; }
        if (has_4) { s->registry.template add_component<Type4>(entt); e_state.has_type_4 = true; }
        if (has_5) { s->registry.template add_component<Type5>(entt); e_state.has_type_5 = true; }
        if (has_6) { s->registry.template add_component<Type6>(entt); e_state.has_type_6 = true; }

        s->registry.template add_component<AllEntitiesHave>(entt);
    }
}


template <typename Q> void test_common(Q query, ve::fn<bool, entity_type, const state::entity_state&> should_have) {
    state s {};

    ve::get_service<ve::engine_logger>().info("Building random registry contents.") << std::flush;
    ve::testing::fuzz_single_operation<state*>(
        add_entity,
        std::tuple {
            fuzzargs::random_int<entity_type>(0, 1e6),
            fuzzargs::from_value_set({ true, false }),
            fuzzargs::from_value_set({ true, false }),
            fuzzargs::from_value_set({ true, false }),
            fuzzargs::from_value_set({ true, false }),
            fuzzargs::from_value_set({ true, false }),
            fuzzargs::from_value_set({ true, false })
        },
        &s
    );


    ve::get_service<ve::engine_logger>().info("Building query.") << std::flush;
    auto view_query      = s.registry.query(query);
    auto everything_else = s.registry.get_entities();


    ve::get_service<ve::engine_logger>().info("Checking query contents.") << std::flush;
    for (const auto& tpl : view_query) {
        const auto entt = std::get<0>(tpl);
        EXPECT_TRUE(should_have(entt, s.state.at(entt)));

        everything_else.erase(entt);
        s.state.erase(entt); // Throw if entity shows up twice.
    }


    ve::get_service<ve::engine_logger>().info("Checking entities not matched by query.") << std::flush;
    for (const auto [i, entt] : everything_else | ve::views::enumerate) {
        EXPECT_FALSE(should_have(entt, s.state.at(entt)));
    }
}


TEST(fuzz_query_view, include_one) {
    test_common(
        has<Type1>,
        [] (entity_type entt, const auto& state) { return state.has_type_1; }
    );
}


TEST(fuzz_query_view, include_many) {
    test_common(
        has<Type1> && has<Type2> && has<Type3>,
        [] (entity_type entt, const auto& state) { return state.has_type_1 && state.has_type_2 && state.has_type_3; }
    );
}


TEST(fuzz_query_view, maybe_include_one) {
    test_common(
        has<Type1> || false_query,
        [] (entity_type entt, const auto& state) { return state.has_type_1; }
    );
}


TEST(fuzz_query_view, maybe_include_many) {
    test_common(
        has<Type1> || has<Type2> || has<Type3>,
        [] (entity_type entt, const auto& state) { return state.has_type_1 || state.has_type_2 || state.has_type_3; }
    );
}


TEST(fuzz_query_view, exclude_one) {
    test_common(
        !has<Type1>,
        [] (entity_type entt, const auto& state) { return !state.has_type_1; }
    );
}


TEST(fuzz_query_view, exclude_one_of_many) {
    test_common(
        !has<Type1> || !has<Type2> || !has<Type3>,
        [] (entity_type entt, const auto& state) { return !state.has_type_1 || !state.has_type_2 || !state.has_type_3; }
    );
}


TEST(fuzz_query_view, exclude_many) {
    test_common(
        !has<Type1> && !has<Type2> && !has<Type3>,
        [] (entity_type entt, const auto& state) { return !state.has_type_1 && !state.has_type_2 && !state.has_type_3; }
    );
}


TEST(fuzz_query_view, composite_query) {
    test_common(
        (
            (has<Type1> && has<Type2>) ||
            (has<Type3> && has<Type4>)
        ) &&
        (!has<Type5> || !has<Type6>),
        [] (entity_type entt, const auto& state) {
            return (
                (
                    (state.has_type_1 && state.has_type_2) ||
                    (state.has_type_3 && state.has_type_4)
                ) &&
                (!state.has_type_5 || !state.has_type_6)
            );
        }
    );
}


TEST(fuzz_query_view, include_everything) {
    test_common(has<AllEntitiesHave>,      [] (entity_type entt, const auto& state) { return true; });
    test_common(!has<NoEntitiesHave>,      [] (entity_type entt, const auto& state) { return true; });
    test_common(has<Type1> || !has<Type1>, [] (entity_type entt, const auto& state) { return true; });
    test_common(true_query,                [] (entity_type entt, const auto& state) { return true; });
}


TEST(fuzz_query_view, exclude_everything) {
    test_common(!has<AllEntitiesHave>,     [] (entity_type entt, const auto& state) { return false; });
    test_common(has<NoEntitiesHave>,       [] (entity_type entt, const auto& state) { return false; });
    test_common(has<Type1> && !has<Type1>, [] (entity_type entt, const auto& state) { return false; });
    test_common(false_query,               [] (entity_type entt, const auto& state) { return false; });
}