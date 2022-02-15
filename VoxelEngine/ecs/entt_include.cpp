#include <VoxelEngine/ecs/entt_include.hpp>
#include <VoxelEngine/utility/assert.hpp>


namespace ve::detail {
    void entt_assert_override(bool cond, const char* msg) {
        VE_DEBUG_ASSERT(cond, msg);
    }
}