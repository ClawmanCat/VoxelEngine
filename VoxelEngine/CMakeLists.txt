include(create_target)
include(generate_include_header)

# If we use a settings header from the game, we need to link the game as well.
if (DEFINED GAME_PRE_INCLUDE_HEADER)
    set(VoxelEngine_AdditionalTestDependencies ${GAME_TARGET_NAME})
endif()

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
    # spirv-cross and shaderc from the Vulkan SDK have _ITERATOR_DEBUG_LEVEL=0 and cannot be used in debug builds.
    # (Note: setting the separate packages above the vulkan-sdk here means they take priority.)
    PUBLIC CONAN_PKG::spirv-cross
    PUBLIC CONAN_PKG::shaderc
    PUBLIC CONAN_PKG::vulkan-sdk
    PUBLIC CONAN_PKG::glew
)

# Allow automatic selection of the actor when making actor-dependent function calls.
target_compile_definitions(VoxelEngine PRIVATE VE_BUILD_GAME)
target_compile_definitions(VoxelEngine PUBLIC VE_GRAPHICS_API=${VE_GRAPHICS_API})

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

# Define header with overloaded settings from game to be included in the engine.
if (DEFINED GAME_PRE_INCLUDE_HEADER)
    target_compile_definitions(VoxelEngine PUBLIC VE_GAME_PRE_INCLUDE=${GAME_PRE_INCLUDE_HEADER})
endif()


# Copy sources for boost preprocessor so we can use it inside GLSL shaders.
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
            DESTINATION "${SHADER_DIR}/boost/preprocessor"
        )
    endif()
endforeach()