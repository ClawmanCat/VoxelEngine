#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/math.hpp>


test_result test_main(void) {
    constexpr std::size_t limit = 32;
    
    std::size_t i = 0;
    for (std::size_t x = 0; x < limit; ++x) {
        for (std::size_t y = 0; y < limit; ++y) {
            for (std::size_t z = 0; z < limit; ++z) {
                ve::vec3ul pos   = ve::vec3ul { x, y, z };
                auto flattened   = ve::flatten({ x, y, z }, limit);
                auto unflattened = ve::unflatten(flattened, limit);

                if (flattened != i) return VE_TEST_FAIL("Flattening ", pos, " should return ", i, " but returned ", flattened);
                if (unflattened != pos) return VE_TEST_FAIL("Unflattening ", flattened, " should return ", pos, " but returned ", unflattened);

                ++i;
            }
        }    
    }

    return VE_TEST_SUCCESS;
}