cmake_minimum_required(VERSION 3.19)
project("VoxelEngine")


# Prevent concurrent CMake runs.
file(LOCK ${CMAKE_SOURCE_DIR}/cmake.lock)


# Allow loading of scripts from cmake folder.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
# Store builds in out folder.
set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/out")
# Allow including from the root directory.
include_directories(${CMAKE_SOURCE_DIR})


# Output to out directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}/bin)


# Force C++20. (Note this must be done manually with MSVC, see cmake/profiles/msvc.cmake)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Set required preprocessor definitions.
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(SDL_MAIN_HANDLED)
# Boost.Asio unfortunately includes Windows.h. While it defines WIN32_LEAN_AND_MEAN, wingdi still defines macros like "ERROR".
add_compile_definitions(NOGDI)
# Not supported on Windows with Clang on Windows as of 14/07/2021
add_compile_definitions(BOOST_ASIO_DISABLE_STD_COROUTINE)
add_compile_definitions(_SILENCE_CLANG_COROUTINE_MESSAGE)


# Install Dependencies
include(run_conan)

select_conan_profile(conan_profile)
message(STATUS ${conan_profile})
run_conan(${conan_profile})


# Enable compiler-specific warnings
include(compiler_profile)
set_compiler_profile()


# Copy required directories to binary folder.
add_custom_target(
    COPY_OUT_DIRS ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/out_dirs
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Copy shared libraries from conan folder to binary folder.
if (EXISTS ${CMAKE_SOURCE_DIR}/out/conan/bin)
    add_custom_target(
        COPY_SHARED_LIBS ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/out/conan/bin
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()


# Enable testing (Each subproject will add its own tests.)
if (${ENABLE_TESTING})
    message(STATUS "Test mode is enabled.")

    enable_testing()
    add_compile_definitions("VE_TESTING=1")
endif()


# Set graphics API.
if (VE_GRAPHICS_API)
    string(TOUPPER ${VE_GRAPHICS_API} gfxapi_upper)

    add_compile_definitions("VE_GRAPHICS_API=${VE_GRAPHICS_API}")
    add_compile_definitions("VE_GRAPHICS_API_${gfxapi_upper}=1")
endif()


# Add subprojects.
add_subdirectory(VoxelEngine)
add_subdirectory(VEDemoGame)
add_subdirectory(VELauncher)
add_subdirectory(VEDemoPlugin)
add_subdirectory(VEEngineSettings)