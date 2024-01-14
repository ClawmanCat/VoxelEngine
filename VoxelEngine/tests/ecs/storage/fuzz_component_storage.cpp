#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/tests/ecs/storage/component_types.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/utility/meta/pack/pack_operations.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage.hpp>




namespace fuzzargs    = ve::testing::fuzz_arguments;
using entity_type     = VE_DEFAULT_ENTITY_TYPE;
using component_types = ve::testing::component_types;


using entity_trait_types = ve::meta::pack<
    ve::ecs::default_entity_traits<entity_type>,
    typename ve::ecs::transform_entity_traits<ve::ecs::default_entity_traits<entity_type>>
        ::template with_index_stability<true>
>;

// Note: has storage_elusion = true, since placeholder is an empty type.
using component_trait_types = ve::meta::pack<
    ve::ecs::default_component_traits<ve::meta::placeholder_type>,
    typename ve::ecs::transform_component_traits<ve::ecs::default_component_traits<ve::meta::placeholder_type>>
        ::template with_reference_stability<true>
>;




struct component_state {
    entity_type entity;
    entity_type value;
    void* address;
    bool is_present = false;
};

template <typename C, typename ET, typename CT> struct state {
    ve::ecs::basic_component_storage<ET, CT> storage;
    ve::hash_map<entity_type, component_state> expected_state;

    using component_type   = C;
    using entity_traits    = ET;
    using component_traits = CT;
};




template <typename State> void add_component_to_storage(State& s, entity_type entity, entity_type value) {
    auto [it, success] = s.expected_state.emplace(entity, component_state { entity, value, nullptr });
    auto& cmp_state    = it->second;


    auto [c_it, c_success] = s.storage.emplace(entity, typename State::component_type { value });
    EXPECT_NE(cmp_state.is_present, c_success);


    if (!std::exchange(cmp_state.is_present, true)) {
        cmp_state.address = std::addressof(c_it.component());
    }
}


template <typename State> void remove_component_from_storage(State& s, entity_type entity) {
    auto it = s.expected_state.find(entity);


    if (it == s.expected_state.end()) {
        EXPECT_FALSE(s.storage.contains(entity));
    } else {
        EXPECT_TRUE(s.storage.erase(entity));
        s.expected_state.erase(it);
    }
}


template <typename State> void update_component_in_storage(State& s, entity_type entity, entity_type value) {
    if (!s.expected_state.contains(entity)) return;

    auto it = s.storage.find(entity);
    EXPECT_NE(it, s.storage.end());

    it.component() = typename State::component_type { value };
    s.expected_state.at(entity).value = value;
}


template <typename State> void clear_storage(State& s) {
    s.storage.clear();
    s.expected_state.clear();

    EXPECT_TRUE(s.storage.empty());
}


/** Assert every component in the storage has the expected value. */
template <typename State> void validate_value_unchanged(State& s) {
    for (const auto [entt, component] : s.storage) {
        EXPECT_EQ(
            typename State::component_type { s.expected_state.at(entt).value },
            component
        );
    }
}


/** Assert every component in the storage has its original address (Only if reference_stability is true). */
template <typename State> void validate_address_unchanged(State& s) {
    if constexpr (State::component_traits::reference_stability) {
        for (const auto [entt, component] : s.storage) {
            EXPECT_EQ(s.expected_state.at(entt).address, std::addressof(component));
        }
    }
}




TEST(component_storage, fuzz_test) {
    ve::meta::pack_cartesian_product<component_types, entity_trait_types, component_trait_types>([&] <typename Component, typename ET, typename CT> {
        using component_traits = typename ve::ecs::transform_component_traits<CT>::template with_component_type<Component>;
        using state_type       = state<Component, ET, component_traits>;


        auto random_entity = fuzzargs::multi_generator {
            fuzzargs::random_int<entity_type>(0, 1000),
            fuzzargs::random_int<entity_type>(10'000, 11'000),
            fuzzargs::random_int<entity_type>(100'000, 101'000),
            fuzzargs::random_int<entity_type>(1'000'000, 1'001'000)
        };

        auto random_value = fuzzargs::random_int<entity_type>();


        // Unstable index + Stable Address is not a valid combination.
        if constexpr (!CT::reference_stability || ET::index_stability) {
            ve::get_service<ve::engine_logger>().info(
                "Testing with index stability: {}, reference stability: {}, component type: {}.",
                ET::index_stability,
                component_traits::reference_stability,
                ve::typename_of<Component>()
            ) << std::flush;


            ve::testing::operation_fuzzer<state_type> fuzzer;


            std::tuple operations {
                std::tuple {
                    &add_component_to_storage<state_type>,
                    2.0f,
                    random_entity, random_value
                },
                std::tuple {
                    &remove_component_from_storage<state_type>,
                    1.0f,
                    random_entity
                },
                std::tuple {
                    &update_component_in_storage<state_type>,
                    5.0f,
                    random_entity, random_value
                },
                std::tuple { &clear_storage<state_type>,              1e-4f },
                std::tuple { &validate_address_unchanged<state_type>, 0.1f  },
                std::tuple { &validate_value_unchanged<state_type>,   0.1f  }
            };


            ve::tuple_foreach(operations, [&] (const auto& subtuple) {
                std::apply([&] (auto operation, auto weight, const auto&... args) {
                    fuzzer.add_operation(operation, weight, args...);
                }, subtuple);
            });


            fuzzer.invoke({}, 1e7);
        }
    });
}