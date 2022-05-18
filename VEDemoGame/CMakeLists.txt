include(create_target)


create_target(
    VEDemoGame
    SHARED
    0 0 1
    # Dependencies:
    PUBLIC VoxelEngine
)


# The game should export all symbols from the engine.
create_symbol_filter_impl(
    VEDemoGame
    VEDemoGame
    VoxelEngine
    "${CMAKE_SOURCE_DIR}/VoxelEngine/symgen.filter"
    "${PROJECT_BINARY_DIR}/VoxelEngine/CMakeFiles/VoxelEngine.dir"
)


target_compile_definitions(VEDemoGame PRIVATE "VE_BUILD_GAME=1")