cmake_minimum_required(VERSION 3.12)
project("VoxelEngine")


# For testing purposes, it is often useful to control what flags get passed to the compiler,
# rather than relying on the current CMake profile.
option(CLEAR_FLAGS "Clear compiler flags for the current profile." OFF)

if (${CLEAR_FLAGS})
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
endif()


# This flag is used in ./cmake/create_target.cmake to generate test targets if it is enabled.
option(ENABLE_TESTING "Enable VoxelEngine tests." OFF)


# Prevent running CMake concurrently.
file(LOCK ${CMAKE_SOURCE_DIR} DIRECTORY GUARD FILE)

# C++20 features are required.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output to out directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}/bin)

# Allow including from the root directory.
include_directories(${CMAKE_SOURCE_DIR})

# Allow loading of scripts from cmake folder.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Store builds in out folder.
set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/out")

# Make sure symbols are exported on Windows.
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Load compiler settings profile.
include(clang_profile)
set_compiler_profile()

# Run Conan
include(run_conan)
run_conan()

# Preprocessor Definitions
add_compile_definitions(_CRT_SECURE_NO_WARNINGS SDL_MAIN_HANDLED)

# Copy required directories to binary folder.
add_custom_target(
        COPY_OUT_DIRS ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/out_dirs
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Copy shared libraries from conan folder to binary folder.
add_custom_target(
        COPY_SHARED_LIBS ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/out/conan/bin
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Add targets.
add_subdirectory(VoxelEngine)
add_subdirectory(VEDemoGame)
add_subdirectory(VEDemoPlugin)
add_subdirectory(VELauncher)
