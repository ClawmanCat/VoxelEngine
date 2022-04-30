# Just make the settings part of the game to prevent circular dependencies.
file(GLOB_RECURSE sources CONFIGURE DEPENDS LIST_DIRECTORIES false "*.cpp" "*.hpp")
target_sources(VEDemoGame PRIVATE ${sources})

# And provide the include directory for the engine.
target_include_directories(VoxelEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})