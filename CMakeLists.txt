cmake_minimum_required(VERSION 3.12)
project("VoxelEngine")


# Prevent running CMake concurrently.
file(LOCK ${CMAKE_SOURCE_DIR} DIRECTORY GUARD FILE)

# C++20 features are required.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(/EHsc /clang:-fbracket-depth=1024)
add_compile_options(/clang:-Wall /clang:-Wextra)      # -Wpedantic will trigger ~4000 warnings in library code.
add_compile_options(/clang:-Wno-deprecated-volatile)  # Mute warnings emitted by GLM.
add_compile_options(/clang:-Wno-unknown-attributes)   # Mute warnings due to lack of C++20 attribute support.
add_compile_options(/clang:-Wno-unused-private-field) # Unused members are used to disable move & copy on some objects.
add_compile_options(/clang:-Wno-unused-parameter)

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

# Add targets.
add_subdirectory(VoxelEngine)
add_subdirectory(VEDemoGame)
add_subdirectory(VEDemoPlugin)
add_subdirectory(VELauncher)
