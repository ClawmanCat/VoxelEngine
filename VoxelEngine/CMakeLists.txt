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
)

add_dependencies(VoxelEngine COPY_OUT_DIRS)