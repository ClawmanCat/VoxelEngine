include(create_target)

create_target(
    VoxelEngine
    OBJECT
    0 0 1
    # Dependencies:
    PUBLIC CONAN_PKG::boost
    PUBLIC CONAN_PKG::magic_enum
    PUBLIC CONAN_PKG::sdl2
    PUBLIC CONAN_PKG::glm
    PUBLIC CONAN_PKG::glew
    PUBLIC CONAN_PKG::ctti
    PUBLIC CONAN_PKG::range-v3
    PUBLIC CONAN_PKG::lodepng
    PUBLIC CONAN_PKG::ctre
    PUBLIC CONAN_PKG::entt
)

# Copies the resource directories (assets, etc.) when VoxelEngine is built.
add_dependencies(VoxelEngine COPY_OUT_DIRS)
# Copies the required shared libraries when VoxelEngine is built.
add_dependencies(VoxelEngine COPY_SHARED_LIBS)

# Indicates that the engine is being built, and not some dependency like a plugin.
target_compile_definitions(VoxelEngine PRIVATE VE_BUILD_ENGINE)