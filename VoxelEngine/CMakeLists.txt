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
    PUBLIC CONAN_PKG::fastnoise2
    PUBLIC CONAN_PKG::optick
    PUBLIC CONAN_PKG::magic_enum
    PUBLIC CONAN_PKG::ctti
    PUBLIC CONAN_PKG::ctre
    PUBLIC CONAN_PKG::xxhash
    PUBLIC CONAN_PKG::zlib
    PUBLIC CONAN_PKG::range-v3
    PUBLIC CONAN_PKG::glm
    PUBLIC CONAN_PKG::sdl2
    PUBLIC CONAN_PKG::stb
    PUBLIC CONAN_PKG::spirv-cross
    PUBLIC CONAN_PKG::shaderc
    PUBLIC CONAN_PKG::glew
)


# Generate single-include headers.
generate_include_header(VoxelEngine core core.hpp)
generate_include_header(VoxelEngine clientserver clientserver.hpp)
generate_include_header(VoxelEngine ecs ecs.hpp)
generate_include_header(VoxelEngine event event_system.hpp)
generate_include_header(VoxelEngine input input.hpp)
generate_include_header(VoxelEngine voxel voxel.hpp)
generate_include_header(VoxelEngine utility utility.hpp)
generate_include_header(VoxelEngine utility/traits traits.hpp)
generate_include_header(VoxelEngine graphics/texture/utility utility.hpp)

# For the actual graphics include header, we also want to include the platform graphics header,
# so just autogenerate a different header and include that from the actual graphics header.
generate_include_header(VoxelEngine graphics graphics_includes.hpp graphics.hpp)
generate_include_header(VoxelEngine platform/graphics/opengl graphics.hpp)
generate_include_header(VoxelEngine platform/graphics/vulkan graphics.hpp)


# Copy sources for boost preprocessor so we can use them inside GLSL shaders.
get_target_property(BOOST_INCLUDE_DIRS CONAN_PKG::boost INTERFACE_INCLUDE_DIRECTORIES)
set(SHADER_DIR "${CMAKE_SOURCE_DIR}/out_dirs/assets/shaders")

foreach(dir ${BOOST_INCLUDE_DIRS})
    if (EXISTS "${dir}/boost/preprocessor.hpp")
        message(STATUS "Copying sources from boost preprocessor (${dir}) to shader directory (${SHADER_DIR}).")

        file(
            COPY "${dir}/boost/preprocessor.hpp"
            DESTINATION "${SHADER_DIR}/boost"
        )
    endif()

    if (EXISTS "${dir}/boost/preprocessor")
        file(
            COPY "${dir}/boost/preprocessor"
            DESTINATION "${SHADER_DIR}/boost"
        )
    endif()
endforeach()