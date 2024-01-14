#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/utility/meta/pack/pack_operations.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set.hpp>




namespace fuzzargs = ve::testing::fuzz_arguments;
using entity_type  = VE_DEFAULT_ENTITY_TYPE;


using entity_trait_types = ve::meta::pack<
    ve::ecs::default_entity_traits<entity_type>,
    typename ve::ecs::transform_entity_traits<ve::ecs::default_entity_traits<entity_type>>
        ::template with_index_stability<true>
>;

enum entity_match_mode { DEFAULT, EXACT, ANY_VERSION };




struct entity_state {
    entity_type index, version, bits;
    bool is_present = false;

    [[nodiscard]] entity_type entity(void) const {
        return ve::ecs::entity_utils<ve::ecs::default_entity_traits<entity_type>>::make_entity(index, version, bits);
    }
};

template <typename ET> struct state {
    ve::ecs::basic_sparse_set<ET> set;
    ve::hash_map<entity_type, entity_state> expected_state;

    using entity_traits = ET;
    using entity_utils  = ve::ecs::entity_utils<ET>;
};




template <typename State> void add_entity_to_set(State& s, entity_type index, entity_type version, entity_type bits) {
    auto [it, success] = s.expected_state.emplace(index, entity_state { index, version, bits });
    auto& entt_state   = it->second;


    auto [s_it, s_success] = s.set.insert(entt_state.entity());
    EXPECT_NE(entt_state.is_present, s_success);
    EXPECT_EQ(*s_it, entt_state.entity());

    entt_state.is_present = true;
}


template <typename State> void remove_entity_from_set(State& s, entity_type index, entity_type version, entity_type bits, entity_match_mode mode) {
    auto it = s.expected_state.find(index);


    if (it == s.expected_state.end()) {
        EXPECT_FALSE(s.set.contains_any_version(index));
    } else {
        auto& entt_state = it->second;
        auto  rm_entt    = entity_state { index, version, bits };


        bool matched = false;

        switch (mode) {
            case DEFAULT:
                matched = (version == entt_state.version);
                EXPECT_EQ(matched, s.set.erase(rm_entt.entity()));

                break;
            case EXACT:
                matched = (version == entt_state.version && bits == entt_state.bits);
                EXPECT_EQ(matched, s.set.erase_exact(rm_entt.entity()));

                break;
            case ANY_VERSION:
                matched = true;
                EXPECT_EQ(matched, s.set.erase_any_version(rm_entt.entity()));

                break;
        }

        if (matched) s.expected_state.erase(it);
    }
}


template <typename State> void update_entity_in_set(State& s, entity_type index, entity_type new_version, entity_type new_bits) {
    if (auto it = s.expected_state.find(index); it != s.expected_state.end()) {
        auto& entt_state = it->second;

        auto e_it = s.set.find_any_version(index);
        EXPECT_NE(e_it, s.set.end());

        e_it.set_version(new_version);
        e_it.set_unassigned_bits(new_bits);

        entt_state.version = new_version;
        entt_state.bits = new_bits;

        EXPECT_EQ(*e_it, entt_state.entity());
    }
}


template <typename State> void find_entity_in_set(State& s, entity_type index, entity_match_mode mode) {
    if (auto it = s.expected_state.find(index); it != s.expected_state.end()) {
        auto& entt_state = it->second;
        typename decltype(s.set)::iterator e_it;

        switch (mode) {
            case DEFAULT:
                e_it = s.set.find(entt_state.entity() & ~State::entity_utils::unassigned_bits_mask());
                break;
            case EXACT:
                e_it = s.set.find_exact(entt_state.entity());
                break;
            case ANY_VERSION:
                e_it = s.set.find_any_version(entt_state.entity() & State::entity_utils::index_mask());
                break;
        }

        EXPECT_NE(e_it, s.set.end());
        EXPECT_EQ(*e_it, entt_state.entity());
    }
}


template <typename State> void clear_set(State& s) {
    s.set.clear();
    s.expected_state.clear();

    EXPECT_TRUE(s.set.empty());
    EXPECT_EQ(s.set.iteration_complexity(), 0);
}


/** Assert every entity in the expected state is in the set using the given match mode. */
template <typename State> void validate_set_contents(State& s, entity_match_mode mode) {
    EXPECT_EQ(s.set.size(), s.expected_state.size());
    EXPECT_GE(s.set.iteration_complexity(), s.expected_state.size());

    for (const auto& [index, entt_state] : s.expected_state) {
        typename decltype(s.set)::iterator it;

        switch (mode) {
            case DEFAULT:
                it = s.set.find(entt_state.entity() & ~State::entity_utils::unassigned_bits_mask());
                break;
            case EXACT:
                it = s.set.find_exact(entt_state.entity());
                break;
            case ANY_VERSION:
                it = s.set.find_any_version(entt_state.entity() & State::entity_utils::index_mask());
                break;
        }

        EXPECT_NE(it, s.set.end());
        EXPECT_EQ(*it, entt_state.entity());
    }
}


/** Assert every entity in the set is in the expected state. */
template <typename State> void validate_set_contents_reversed(State& s) {
    EXPECT_EQ(s.set.size(), s.expected_state.size());
    EXPECT_GE(s.set.iteration_complexity(), s.expected_state.size());

    for (auto entt : s.set) {
        auto index = State::entity_utils::index_of(entt);
        EXPECT_EQ(entt, s.expected_state.at(index).entity());
    }
}




TEST(sparse_set, fuzz_test) {
    entity_trait_types::foreach([] <typename ET> {
        ve::get_service<ve::engine_logger>().info("Testing with index stability: {}.", ET::index_stability) << std::flush;


        using state_type = state<ET>;


        auto random_index = fuzzargs::multi_generator {
            fuzzargs::random_int<entity_type>(0, 1000),
            fuzzargs::random_int<entity_type>(10'000, 11'000),
            fuzzargs::random_int<entity_type>(100'000, 101'000),
            fuzzargs::random_int<entity_type>(1'000'000, 1'001'000)
        };

        auto random_version = fuzzargs::multi_generator {
            fuzzargs::random_int<entity_type>(0, 4),
            fuzzargs::random_int<entity_type>(1'000'000, 1'000'004)
        };

        auto random_bits = fuzzargs::from_value_set { 0ull, 1ull };
        auto random_mode = fuzzargs::from_value_set ( magic_enum::enum_values<entity_match_mode>() );


        ve::testing::operation_fuzzer<state_type> fuzzer;

        std::tuple operations {
            std::tuple {
                &add_entity_to_set<state_type>,
                2.0f,
                random_index, random_version, random_bits
            },
            std::tuple {
                &remove_entity_from_set<state_type>,
                1.0f,
                random_index, random_version, random_bits, random_mode
            },
            std::tuple {
                &update_entity_in_set<state_type>,
                5.0f,
                random_index, random_version, random_bits
            },
            std::tuple {
                &find_entity_in_set<state_type>,
                5.0f,
                random_index, random_mode
            },
            std::tuple {
                &clear_set<state_type>,
                1e-4f
            },
            std::tuple {
                &validate_set_contents<state_type>,
                0.1f,
                random_mode
            },
            std::tuple {
                &validate_set_contents_reversed<state_type>,
                0.1f
            }
        };


        ve::tuple_foreach(operations, [&] (const auto& subtuple) {
            std::apply([&] (auto operation, auto weight, const auto&... args) {
                fuzzer.add_operation(operation, weight, args...);
            }, subtuple);
        });


        fuzzer.invoke({}, 1e7);
    });
}