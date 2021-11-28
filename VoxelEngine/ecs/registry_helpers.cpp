#include <VoxelEngine/ecs/registry_helpers.hpp>
#include <VoxelEngine/ecs/registry.hpp>


namespace ve::detail {
    entt::registry& get_storage(registry& registry) {
        return registry.storage;
    }
}