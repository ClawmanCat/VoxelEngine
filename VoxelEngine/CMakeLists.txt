include(create_target)
include(generate_include_header)


create_target(
    VoxelEngine
    OBJECT
    0 0 1
    # Dependencies:
    PUBLIC CONAN_PKG::boost
    PUBLIC CONAN_PKG::abseil
    PUBLIC CONAN_PKG::entt
    PUBLIC CONAN_PKG::magic_enum
    PUBLIC CONAN_PKG::ctti
    PUBLIC CONAN_PKG::ctre
    PUBLIC CONAN_PKG::range-v3
    PUBLIC CONAN_PKG::glm
    PUBLIC CONAN_PKG::sdl2
    PUBLIC CONAN_PKG::stb
    # spirv-cross from the Vulkan SDK has _ITERATOR_DEBUG_LEVEL=0 and cannot be used in debug builds.
    PUBLIC CONAN_PKG::spirv-cross
    PUBLIC CONAN_PKG::vulkan-sdk
)

# Allow automatic selection of the actor when making actor-dependent function calls.
target_compile_definitions(VoxelEngine PRIVATE VE_BUILD_GAME)

generate_include_header(VoxelEngine core core.hpp)
generate_include_header(VoxelEngine clientserver clientserver.hpp)
generate_include_header(VoxelEngine ecs ecs.hpp)
generate_include_header(VoxelEngine graphics graphics_includes.hpp graphics.hpp)
generate_include_header(VoxelEngine event event_system.hpp)
generate_include_header(VoxelEngine utility utility.hpp)
generate_include_header(VoxelEngine utility/traits traits.hpp)

generate_include_header(VoxelEngine platform/graphics/vulkan graphics.hpp)
