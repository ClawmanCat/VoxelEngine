CMAKE_MINIMUM_REQUIRED(VERSION 3.19)
PROJECT("VoxelEngine")


# Prevent concurrent CMake runs.
FILE(LOCK ${CMAKE_SOURCE_DIR}/cmake.lock)


# Get compiler name and build type.
STRING(TOLOWER ${CMAKE_CXX_COMPILER_ID} CXX_COMPILER_ID_LOWER)
STRING(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWER)

# RelWithDebInfo -> FastDebug
IF (${BUILD_TYPE_LOWER} STREQUAL "relwithdebinfo")
    SET(BUILD_TYPE_LOWER "fastdebug")
ENDIF()


# Command line options.
SET(VE_ENABLE_TESTING ON CACHE BOOL "Generate test targets.")
SET(VE_GRAPHICS_API "OpenGL" CACHE STRING "Graphics API to use for the engine.")
SET(VE_PROFILE_NAME "${CXX_COMPILER_ID_LOWER}-${BUILD_TYPE_LOWER}" CACHE STRING "Name of the current CMake profile. Will be used as the folder name for build artifacts.")
SET(VE_COMPILER_PROFILE ON CACHE BOOL "Use a preconfigured profile with compiler-specific settings for e.g. warnings.")
SET(VE_WARNINGS_ARE_ERRORS ON CACHE BOOL "Treat all warnings as errors.")

SET_PROPERTY(CACHE VE_GRAPHICS_API PROPERTY STRINGS OpenGL Vulkan)


# Require C++23 mode.
SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)


# Export all symbols on Windows.
# TODO: Replace this with SymbolGenerator later!
SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


# Allow loading of scripts from cmake folder.
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
# Store builds in out folder.
SET(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/out")
# Allow including from the root directory.
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})


# Enable Testing
INCLUDE(CTest)
ENABLE_TESTING()


# Install Dependencies
INCLUDE(install_dependencies)
INSTALL_GIT_DEPENDENCIES()
INSTALL_CONAN_DEPENDENCIES()


# Enable compiler-specific compatibility flags and warnings.
IF (VE_COMPILER_PROFILE)
    INCLUDE(compiler_config)
    CONFIGURE_COMPILER()
ENDIF()


# Add subprojects.
ADD_SUBDIRECTORY(VoxelEngine)
ADD_SUBDIRECTORY(VEDemoGame)
ADD_SUBDIRECTORY(VELauncher)
ADD_SUBDIRECTORY(VEDemoPlugin)