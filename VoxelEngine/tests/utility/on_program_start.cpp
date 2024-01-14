#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/on_program_start.hpp>


int global_value = 0;


VE_ON_PROGRAM_START(set_global_value, [] {
    global_value = 33;
});


TEST(on_program_start, set_global_value) {
    ASSERT_EQ(global_value, 33);
}