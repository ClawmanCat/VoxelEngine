INCLUDE(create_target)
INCLUDE(create_include_header)


CREATE_TARGET(
    VoxelEngine
    0 0 1
    OBJECT COMPONENT
    # Dependencies
    PUBLIC CONAN_PKG::boost
    PUBLIC CONAN_PKG::abseil
    PUBLIC CONAN_PKG::kangaru
    PUBLIC CONAN_PKG::magic_enum
    PUBLIC CONAN_PKG::xxhash
    PUBLIC CONAN_PKG::crc_cpp
    PUBLIC CONAN_PKG::nlohmann_json
    PUBLIC CONAN_PKG::zlib
    PUBLIC CONAN_PKG::range-v3
    PUBLIC CONAN_PKG::gtest
    PUBLIC CONAN_PKG::doxygen
    PUBLIC CONAN_PKG::sdl
    PUBLIC CONAN_PKG::glm
    PUBLIC CONAN_PKG::glew
    PUBLIC CONAN_PKG::shaderc
    PUBLIC CONAN_PKG::spirv-cross
)


# Generate single-include headers.
CREATE_INCLUDE_HEADER(VoxelEngine core core.hpp)


# Generate a single include header to include everything to prevent CLion resolve contexts from breaking.
CREATE_INCLUDE_HEADER(VoxelEngine "" "utility/debug/ide/clion_resolve_context_fix.hpp")