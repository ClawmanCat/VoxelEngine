#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/tests/ecs/storage/component_types.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage_iterator.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage_mixin.hpp>


using entity_type = VE_DEFAULT_ENTITY_TYPE;
using ve::testing::component_types;


enum class mixin_callback { MIXIN_ADDED, MIXIN_REMOVED, INSERT, ERASE, EDIT, SWAP, CLEAR };
constexpr std::size_t num_callbacks = magic_enum::enum_count<mixin_callback>();


template <typename Component> struct signalling_mixin :
    ve::ecs::component_storage_mixin<
        signalling_mixin<Component>,
        ve::ecs::default_entity_traits<entity_type>,
        ve::ecs::default_component_traits<Component>
    >
{
    static inline std::array<bool, num_callbacks> history;

    static void reset_history(void) { history = ve::create_filled_array<num_callbacks>(false); }
    static void set_invoked(mixin_callback cb)   { history[(std::size_t) cb] = true; }
    static bool check_invoked(mixin_callback cb) { return std::exchange(history[(std::size_t) cb], false); }


    ~signalling_mixin(void) { set_invoked(mixin_callback::MIXIN_REMOVED); }
    void on_added_to_storage(auto& parent) { set_invoked(mixin_callback::MIXIN_ADDED); }
    void on_insert(entity_type entt, Component& cmp, std::size_t dense_index) { set_invoked(mixin_callback::INSERT); }
    void on_erase(entity_type entt, Component&& cmp, std::size_t dense_index) { set_invoked(mixin_callback::ERASE); }
    void on_swap(entity_type e1, entity_type e2, Component& c1, Component& c2, std::size_t e1_dest, std::size_t e2_dest) { set_invoked(mixin_callback::SWAP); }
    void on_clear(void) { set_invoked(mixin_callback::CLEAR); }
};


// Test Fixture: clear history before each test.
struct sparse_set_mixin : ::testing::Test {
protected:
    void SetUp(void) override {
        component_types::foreach([] <typename Component> {
            signalling_mixin<Component>::reset_history();
        });
    }
};


template <typename Component> auto make_storage(void) {
    return ve::ecs::basic_component_storage<
        ve::ecs::default_entity_traits<entity_type>,
        ve::ecs::default_component_traits<Component>,
        ve::ecs::no_sparse_set_mixin<ve::ecs::default_entity_traits<entity_type>>,
        signalling_mixin<Component>
    > {};
}


TEST_F(sparse_set_mixin, create_destroy) {
    component_types::foreach([] <typename Component> {
        {
            auto c = make_storage<Component>();
            ASSERT_TRUE(signalling_mixin<Component>::check_invoked(mixin_callback::MIXIN_ADDED));
            signalling_mixin<Component>::check_invoked(mixin_callback::MIXIN_REMOVED); // Clear destruction from move initialization.
        }

        ASSERT_FALSE(signalling_mixin<Component>::check_invoked(mixin_callback::MIXIN_ADDED));
        ASSERT_TRUE(signalling_mixin<Component>::check_invoked(mixin_callback::MIXIN_REMOVED));
    });
}


TEST_F(sparse_set_mixin, component_added) {
    component_types::foreach([] <typename Component> {
        auto c = make_storage<Component>();

        ASSERT_FALSE(signalling_mixin<Component>::check_invoked(mixin_callback::INSERT));
        c.emplace(0);
        ASSERT_TRUE(signalling_mixin<Component>::check_invoked(mixin_callback::INSERT));
    });
}


TEST_F(sparse_set_mixin, component_removed) {
    component_types::foreach([] <typename Component> {
        auto c = make_storage<Component>();

        c.emplace(0);
        ASSERT_FALSE(signalling_mixin<Component>::check_invoked(mixin_callback::ERASE));
        c.erase(0);
        ASSERT_TRUE(signalling_mixin<Component>::check_invoked(mixin_callback::ERASE));
    });
}


TEST_F(sparse_set_mixin, component_swap) {
    component_types::foreach([] <typename Component> {
        auto c = make_storage<Component>();
        c.emplace(0);
        c.emplace(1);
        c.emplace(2);

        while (!c.empty()) {
            ASSERT_FALSE(signalling_mixin<Component>::check_invoked(mixin_callback::SWAP));
            c.erase(std::get<0>(*c.begin()));
            ASSERT_TRUE(signalling_mixin<Component>::check_invoked(mixin_callback::SWAP));
        }
    });
}


TEST_F(sparse_set_mixin, storage_clear) {
    component_types::foreach([] <typename Component> {
        auto c = make_storage<Component>();
        c.erase(0);
        c.erase(1);
        c.erase(2);

        ASSERT_FALSE(signalling_mixin<Component>::check_invoked(mixin_callback::CLEAR));
        c.clear();
        ASSERT_TRUE(signalling_mixin<Component>::check_invoked(mixin_callback::CLEAR));
        ASSERT_FALSE(signalling_mixin<Component>::check_invoked(mixin_callback::ERASE));
    });
}