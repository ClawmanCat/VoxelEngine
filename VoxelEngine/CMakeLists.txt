include(create_target)
include(single_include_generator)

create_target(
    VoxelEngine
    OBJECT
    0 0 1
    # Dependencies:
    PUBLIC CONAN_PKG::boost
    PUBLIC CONAN_PKG::abseil
    PUBLIC CONAN_PKG::magic_enum
    PUBLIC CONAN_PKG::sdl2
    PUBLIC CONAN_PKG::glm
    PUBLIC CONAN_PKG::glew
    PUBLIC CONAN_PKG::ctti
    PUBLIC CONAN_PKG::lodepng
    PUBLIC CONAN_PKG::ctre
    PUBLIC CONAN_PKG::entt
    PUBLIC CONAN_PKG::tl-optional
    PUBLIC CONAN_PKG::tl-expected
    PUBLIC CONAN_PKG::range-v3
    PUBLIC CONAN_PKG::vulkan-sdk
)

# Copies the resource directories (assets, etc.) when VoxelEngine is built.
add_dependencies(VoxelEngine COPY_OUT_DIRS)
# Copies the required shared libraries when VoxelEngine is built.
add_dependencies(VoxelEngine COPY_SHARED_LIBS)

# Indicates that the engine is being built, and not some dependency like a plugin.
target_compile_definitions(VoxelEngine PRIVATE VE_BUILD_ENGINE)


# The graphics API to be used by the engine. Possible values are 'opengl' and 'vulkan'.
target_compile_definitions(VoxelEngine PUBLIC VE_GRAPHICS_API=opengl)


# Generate headers for including subsets of the engine.
generate_si_header(VoxelEngine platform/graphics/opengl graphics.hpp)
generate_si_header(VoxelEngine platform/graphics/vulkan graphics.hpp)
generate_si_header(VoxelEngine graphics graphics_sih.hpp graphics/graphics.hpp)

generate_si_header(VoxelEngine core core.hpp)
generate_si_header(VoxelEngine ecs ecs.hpp)
generate_si_header(VoxelEngine voxel voxel.hpp)
generate_si_header(VoxelEngine utility/serialize serialize.hpp)
generate_si_header(VoxelEngine utility/traits traits.hpp)
generate_si_header(VoxelEngine tests/framework framework.hpp)

