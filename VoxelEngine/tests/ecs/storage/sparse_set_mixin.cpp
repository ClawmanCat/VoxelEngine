#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set_mixin.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set_iterator.hpp>

#include <magic_enum.hpp>


using traits = ve::ecs::default_entity_traits<ve::u64>;


enum class mixin_callback { MIXIN_ADDED, MIXIN_REMOVED, INSERT, ERASE, EDIT, SWAP, CLEAR };
constexpr std::size_t num_callbacks = magic_enum::enum_count<mixin_callback>();


struct signalling_mixin : ve::ecs::sparse_set_mixin<signalling_mixin, traits> {
    static inline std::array<bool, num_callbacks> history;

    static void reset_history(void) { history = ve::create_filled_array<num_callbacks>(false); }
    static void set_invoked(mixin_callback cb)   { history[(std::size_t) cb] = true; }
    static bool check_invoked(mixin_callback cb) { return std::exchange(history[(std::size_t) cb], false); }
    

    ~signalling_mixin(void) { set_invoked(mixin_callback::MIXIN_REMOVED); }
    void on_added_to_set(auto& parent) { set_invoked(mixin_callback::MIXIN_ADDED); }
    void on_insert(entity_type entt, std::size_t dense_index) { set_invoked(mixin_callback::INSERT); }
    void on_erase(entity_type entt, std::size_t dense_index) { set_invoked(mixin_callback::ERASE); }
    void on_edited(entity_type prev, entity_type current, std::size_t dense_index) { set_invoked(mixin_callback::EDIT); }
    void on_swap(entity_type e1, entity_type e2, std::size_t e1_dest, std::size_t e2_dest) { set_invoked(mixin_callback::SWAP); }
    void on_clear(void) { set_invoked(mixin_callback::CLEAR); }
};


auto make_set(void) {
    return ve::ecs::basic_sparse_set<traits, signalling_mixin> { };
}


// Test Fixture: clear history before each test.
struct sparse_set_mixin : ::testing::Test {
protected:
    void SetUp(void) override { signalling_mixin::reset_history(); }
};


TEST_F(sparse_set_mixin, create_destroy) {
    {
        auto set = make_set();
        ASSERT_TRUE(signalling_mixin::check_invoked(mixin_callback::MIXIN_ADDED));
        signalling_mixin::check_invoked(mixin_callback::MIXIN_REMOVED); // Clear destruction from move initialization.
    }

    ASSERT_FALSE(signalling_mixin::check_invoked(mixin_callback::MIXIN_ADDED));
    ASSERT_TRUE(signalling_mixin::check_invoked(mixin_callback::MIXIN_REMOVED));
}


TEST_F(sparse_set_mixin, entity_added) {
    auto set = make_set();

    ASSERT_FALSE(signalling_mixin::check_invoked(mixin_callback::INSERT));
    set.insert(0);
    ASSERT_TRUE(signalling_mixin::check_invoked(mixin_callback::INSERT));
}


TEST_F(sparse_set_mixin, entity_removed) {
    auto set = make_set();

    set.insert(0);
    ASSERT_FALSE(signalling_mixin::check_invoked(mixin_callback::ERASE));
    set.erase(0);
    ASSERT_TRUE(signalling_mixin::check_invoked(mixin_callback::ERASE));
}


TEST_F(sparse_set_mixin, entity_edited) {
    auto set = make_set();
    set.insert(0);

    ASSERT_FALSE(signalling_mixin::check_invoked(mixin_callback::EDIT));
    set.begin().set_version(1);
    ASSERT_TRUE(signalling_mixin::check_invoked(mixin_callback::EDIT));

    ASSERT_FALSE(signalling_mixin::check_invoked(mixin_callback::EDIT));
    set.begin().set_unassigned_bits(1);
    ASSERT_TRUE(signalling_mixin::check_invoked(mixin_callback::EDIT));
}


TEST_F(sparse_set_mixin, entity_swap) {
    auto set = make_set();
    set.insert(0);
    set.insert(1);
    set.insert(2);

    while (!set.empty()) {
        ASSERT_FALSE(signalling_mixin::check_invoked(mixin_callback::SWAP));
        set.erase(*set.begin());
        ASSERT_TRUE(signalling_mixin::check_invoked(mixin_callback::SWAP));
    }
}


TEST_F(sparse_set_mixin, set_clear) {
    auto set = make_set();
    set.insert(0);
    set.insert(1);
    set.insert(2);

    ASSERT_FALSE(signalling_mixin::check_invoked(mixin_callback::CLEAR));
    set.clear();
    ASSERT_TRUE(signalling_mixin::check_invoked(mixin_callback::CLEAR));
    ASSERT_FALSE(signalling_mixin::check_invoked(mixin_callback::ERASE));
}